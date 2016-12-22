/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	gas.c

	Abstract:
	generic advertisement service(GAS)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"


enum DOT11U_ADVERTISMENT_PROTOCOL_ID dot11GASAdvertisementID[] =
{
	ACCESS_NETWORK_QUERY_PROTOCOL,
};

enum GAS_STATE GASPeerCurrentState(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PGAS_CTRL pGASCtrl;
	PGAS_PEER_ENTRY GASPeerEntry;
	PGAS_EVENT_DATA Event = (PGAS_EVENT_DATA)Elem->Msg;

#ifdef CONFIG_AP_SUPPORT
	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;
#endif /* CONFIG_STA_SUPPORT */

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{

			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
			return GASPeerEntry->CurrentState;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	return GAS_UNKNOWN;
}


VOID GASSetPeerCurrentState(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum GAS_STATE State)
{

	PGAS_CTRL pGASCtrl;
	PGAS_PEER_ENTRY GASPeerEntry;
	PGAS_EVENT_DATA Event = (PGAS_EVENT_DATA)Elem->Msg;

#ifdef CONFIG_AP_SUPPORT
	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;
#endif /* CONFIG_STA_SUPPORT */

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			GASPeerEntry->CurrentState = State;
			break;
		}

	}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
}


#ifdef CONFIG_AP_SUPPORT
void wext_send_anqp_req_event(struct net_device *net_dev, const char *peer_mac_addr,
			      		const char *anqp_req, u16 anqp_req_len)
{
	struct anqp_req_data *req_data;
	u16 buflen = 0;
	char *buf;

	buflen = sizeof(*req_data) + anqp_req_len;
	buf = kmalloc(buflen, GFP_ATOMIC);
	memset(buf, buflen);

	req_data = (struct anqp_req_data *)buf;
	req_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	req_data->anqp_req_len	= anqp_req_len;
	memcpy(req_data->anqp_req, anqp_req, anqp_req_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_HS_ANQP_REQ, NULL, (PUCHAR)buf, buflen);

	kfree(buf);

}


void SendAnqpReqEvent(struct net_device *net_dev, const char *peer_mac_addr,
					  const char *anqp_req, u16 anqp_req_len)
{
	wext_send_anqp_req_event(net_dev,
							 peer_mac_addr,
							 anqp_req,
							 anqp_req_len);
}


static VOID SendGASRsp(
    IN struct rtmp_adapter *   pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{

	GAS_EVENT_DATA *Event = (GAS_EVENT_DATA *)Elem->Msg;
	UCHAR *Buf, *Pos;
	GAS_FRAME *GASFrame;
	ULONG FrameLen = 0, VarLen = 0, tmpValue = 0;
	PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	GAS_PEER_ENTRY *GASPeerEntry;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
			break;
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	if (Event->u.GAS_RSP_DATA.AdvertisementProID == ACCESS_NETWORK_QUERY_PROTOCOL)
	{
		/* Advertisement protocol element + Query response length field */
		VarLen += 6;
	}

	if (Event->EventType == GAS_RSP)
	{
		/* Query response field*/
		VarLen += Event->u.GAS_RSP_DATA.QueryRspLen;
	}

	buf = kmalloc(sizeof(*GASFrame) + VarLen, GFP_ATOMIC);
	if (!Buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
        GASSetPeerCurrentState(pAd, Elem, WAIT_PEER_GAS_REQ);

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListDel(&GASPeerEntry->List);
		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		if (GASPeerEntry->PostReplyTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
			GASPeerEntry->PostReplyTimerRunning = FALSE;
		}
		RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);

		if (GASPeerEntry->GASRspBufferingTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
			GASPeerEntry->GASRspBufferingTimerRunning = FALSE;
		}
		RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
		kfree(GASPeerEntry);
		return;
	}

	memset(Buf, sizeof(*GASFrame) + VarLen);

	GASFrame = (GAS_FRAME *)Buf;

	ActHeaderInit(pAd, &GASFrame->Hdr, Event->PeerMACAddr ,pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					   pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);

	FrameLen += sizeof(HEADER_802_11);

	GASFrame->Category = CATEGORY_PUBLIC;
	FrameLen += 1;
	GASFrame->u.GAS_INIT_RSP.Action = ACTION_GAS_INIT_RSP;
	FrameLen += 1;

	if (Event->EventType == GAS_RSP)
	{
		GASFrame->u.GAS_INIT_RSP.DialogToken = Event->u.GAS_RSP_DATA.DialogToken;
		FrameLen += 1;

		GASFrame->u.GAS_INIT_RSP.StatusCode = cpu2le16(Event->u.GAS_RSP_DATA.StatusCode);
		FrameLen += 2;

		GASFrame->u.GAS_INIT_RSP.GASComebackDelay = Event->u.GAS_RSP_DATA.GASComebackDelay;
		FrameLen += 2;

		Pos = GASFrame->u.GAS_INIT_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */

		*Pos++ = Event->u.GAS_RSP_DATA.AdvertisementProID; /* Advertisement Protocol ID field */

		tmpValue = cpu2le16(Event->u.GAS_RSP_DATA.QueryRspLen);
		memmove(Pos, &tmpValue, 2);
		Pos += 2;
		FrameLen +=	6;

		if (Event->u.GAS_RSP_DATA.QueryRspLen > 0)
			memmove(Pos, Event->u.GAS_RSP_DATA.QueryRsp, Event->u.GAS_RSP_DATA.QueryRspLen);

		FrameLen += Event->u.GAS_RSP_DATA.QueryRspLen;

		GASSetPeerCurrentState(pAd, Elem, WAIT_PEER_GAS_REQ);

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListDel(&GASPeerEntry->List);
		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		if (GASPeerEntry->PostReplyTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
			GASPeerEntry->PostReplyTimerRunning = FALSE;
		}
		RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
		GASPeerEntry->FreeResource++;

		if (GASPeerEntry->GASRspBufferingTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
			GASPeerEntry->GASRspBufferingTimerRunning = FALSE;
		}
		RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
		GASPeerEntry->FreeResource++;

		GASPeerEntry->FreeResource++;
		kfree(GASPeerEntry);

	}
	else if (Event->EventType == GAS_RSP_MORE)
	{

		GASFrame->u.GAS_INIT_RSP.DialogToken = Event->u.GAS_RSP_MORE_DATA.DialogToken;
		FrameLen += 1;

		GASFrame->u.GAS_INIT_RSP.StatusCode = cpu2le16(Event->u.GAS_RSP_MORE_DATA.StatusCode);
		FrameLen += 2;

		GASFrame->u.GAS_INIT_RSP.GASComebackDelay = Event->u.GAS_RSP_MORE_DATA.GASComebackDelay;
		FrameLen += 2;

		Pos = GASFrame->u.GAS_INIT_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */

		*Pos++ = Event->u.GAS_RSP_MORE_DATA.AdvertisementProID; /* Advertisement Protocol ID field */

		tmpValue = 0;
		memmove(Pos, &tmpValue, 2);

		Pos += 2;
		FrameLen +=	6;

		GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_CB_REQ);

		/*
 		 * Buffer the query response for a minimun of dotGASResponseBufferingTime
 		 * after the expiry of the GAS comeback delay
 		 */
		if (!GASPeerEntry->GASRspBufferingTimerRunning)
		{
			RTMPSetTimer(&GASPeerEntry->GASRspBufferingTimer, dotGASResponseBufferingTime
								+ GASFrame->u.GAS_INIT_RSP.GASComebackDelay);
			GASPeerEntry->GASRspBufferingTimerRunning = TRUE;
		}

	}

	MiniportMMRequest(pAd, 0, Buf, FrameLen);

	kfree(Buf);
}


VOID ReceiveGASInitReq(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	GAS_EVENT_DATA *Event;
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;
	GAS_PEER_ENTRY *GASPeerEntry, *GASPeerEntryTmp;
	PGAS_CTRL pGASCtrl = NULL;
	UCHAR APIndex, *Pos, *Buf;
	UINT16 VarLen;
	uint32_t Len = 0;
	BOOLEAN IsFound = FALSE, Cancelled;;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		if (MAC_ADDR_EQUAL(GASFrame->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid))
		{
			pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
			break;
		}
	}

	if (!pGASCtrl)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Can not find Peer Control\n", __FUNCTION__));
		return;
	}

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2))
		{
			IsFound = TRUE;
			GASPeerEntry->QueryNum++;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	if (IsFound) {

		//if (GASPeerEntry->QueryNum++ >= 15)
		{
			/* Query too many times, remove the peer address from list */
			//DBGPRINT(RT_DEBUG_ERROR, ("%s Query %d times, remove the peer address from list\n", __FUNCTION__, GASPeerEntry->QueryNum));
			DBGPRINT(RT_DEBUG_OFF, ("%s Q %d, old 0x%x, 0x%x, %02x:%02x:%02x:%02x:%02x:%02x remove peer\n", __FUNCTION__, GASPeerEntry->QueryNum, GASPeerEntry->DialogToken, GASFrame->u.GAS_INIT_REQ.DialogToken, GASPeerEntry->PeerMACAddr[0], GASPeerEntry->PeerMACAddr[1], GASPeerEntry->PeerMACAddr[2], GASPeerEntry->PeerMACAddr[3], GASPeerEntry->PeerMACAddr[4], GASPeerEntry->PeerMACAddr[5]));

			RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
			DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
			{
				if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2))
				{
					DlListDel(&GASPeerEntry->List);
					DlListInit(&GASPeerEntry->GASQueryRspFragList);

					if (GASPeerEntry->PostReplyTimerRunning)
					{
						RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
						GASPeerEntry->PostReplyTimerRunning = FALSE;
					}
					RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);

					if (GASPeerEntry->GASRspBufferingTimerRunning)
					{
						RTMPCancelTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
						GASPeerEntry->GASRspBufferingTimerRunning = FALSE;
					}
					RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
					GASPeerEntry->FreeResource++;

					kfree(GASPeerEntry);
				}
			}
			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		}
		//else
		//	DBGPRINT(RT_DEBUG_ERROR, ("%s Find peer address in GASPeerList already\n", __FUNCTION__));
		//return;
	}

	GASPeerEntry = kmalloc(sizeof(*GASPeerEntry), GFP_ATOMIC);
	if (!GASPeerEntry) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	memset(GASPeerEntry, sizeof(*GASPeerEntry));
	GASPeerEntry->AllocResource++;

	GASPeerEntry->CurrentState = WAIT_PEER_GAS_REQ;
	GASPeerEntry->QueryNum++;
	memmove(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
	GASPeerEntry->DialogToken = GASFrame->u.GAS_INIT_REQ.DialogToken;
	GASPeerEntry->Priv = pAd;
	RTMPInitTimer(pAd, &GASPeerEntry->PostReplyTimer,
					GET_TIMER_FUNCTION(PostReplyTimeout), GASPeerEntry, FALSE);
	GASPeerEntry->AllocResource++;
	RTMPInitTimer(pAd, &GASPeerEntry->GASRspBufferingTimer,
					GET_TIMER_FUNCTION(GASRspBufferingTimeout), GASPeerEntry, FALSE);
	GASPeerEntry->AllocResource++;

	DlListInit(&GASPeerEntry->GASQueryRspFragList);

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListAddTail(&pGASCtrl->GASPeerList, &GASPeerEntry->List);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	memmove(&VarLen, GASFrame->u.GAS_INIT_REQ.Variable + 4, 2);
	VarLen = le2cpu16(VarLen);

	Buf = kmalloc( (UCHAR **)&Buf, sizeof(*Event) + VarLen, GFP_ATOMIC);
	if (!Buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		goto error;
	}

	memset(Buf, sizeof(*Event) + VarLen);

	Event = (GAS_EVENT_DATA *)Buf;

	Event->ControlIndex = APIndex;
	Len += 1;

	memmove(Event->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;

	Event->EventType = PEER_GAS_REQ;
	Len += 2;

	Event->u.PEER_GAS_REQ_DATA.DialogToken = GASFrame->u.GAS_INIT_REQ.DialogToken;
 	Len += 1;

	Pos = GASFrame->u.GAS_INIT_REQ.Variable + 3;
	Event->u.PEER_GAS_REQ_DATA.AdvertisementProID = *Pos;
	GASPeerEntry->AdvertisementProID = *Pos;
	Len += 1;
	Pos++;

	memmove(&Event->u.PEER_GAS_REQ_DATA.QueryReqLen, Pos, 2);
	Event->u.PEER_GAS_REQ_DATA.QueryReqLen = le2cpu16(Event->u.PEER_GAS_REQ_DATA.QueryReqLen);
	Len += 2;
	Pos += 2;

	memmove(Event->u.PEER_GAS_REQ_DATA.QueryReq, Pos, Event->u.PEER_GAS_REQ_DATA.QueryReqLen);
	Len += Event->u.PEER_GAS_REQ_DATA.QueryReqLen;

	MlmeEnqueue(pAd, GAS_STATE_MACHINE, PEER_GAS_REQ, Len, Buf,0);
	RTMP_MLME_HANDLER(pAd);

	kfree(Buf);

	return;

error:
	DlListDel(&GASPeerEntry->List);
	kfree(GASPeerEntry);
}


static VOID SendGASCBRsp(
    IN struct rtmp_adapter *   pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{

	GAS_EVENT_DATA *Event = (GAS_EVENT_DATA *)Elem->Msg;
	UCHAR *Buf, *Pos;
	GAS_FRAME *GASFrame;
	ULONG FrameLen = 0, VarLen = 0, tmpLen = 0;
	GAS_PEER_ENTRY *GASPeerEntry;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag = NULL, *GASQueryRspFragTmp;
	PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	BOOLEAN bGASQueryRspFragFound = FALSE;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			if (GASPeerEntry->AdvertisementProID == ACCESS_NETWORK_QUERY_PROTOCOL)
			{
				/* Advertisement protocol element + Query response length field */
				VarLen += 6;
			}

			DlListForEach(GASQueryRspFrag, &GASPeerEntry->GASQueryRspFragList,
							GAS_QUERY_RSP_FRAGMENT, List)
			{
				if (GASPeerEntry->CurrentGASFragNum ==
								GASQueryRspFrag->GASRspFragID)
				{
					GASPeerEntry->CurrentGASFragNum++;
					VarLen += GASQueryRspFrag->FragQueryRspLen;
					bGASQueryRspFragFound = TRUE;
					break;
				}
			}
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	Buf = kmalloc(sizeof(*GASFrame) + VarLen, GFP_ATOMIC);
	if (!Buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	memset(Buf, sizeof(*GASFrame) + VarLen);

	GASFrame = (GAS_FRAME *)Buf;

	ActHeaderInit(pAd, &GASFrame->Hdr, Event->PeerMACAddr ,pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					   pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);

	FrameLen += sizeof(HEADER_802_11);

	GASFrame->Category = CATEGORY_PUBLIC;
	FrameLen += 1;

	GASFrame->u.GAS_CB_RSP.Action = ACTION_GAS_CB_RSP;
	FrameLen += 1;

	if (Event->EventType == GAS_CB_REQ)
	{
		GASFrame->u.GAS_CB_RSP.DialogToken = Event->u.GAS_CB_REQ_DATA.DialogToken;
		FrameLen += 1;

		GASFrame->u.GAS_CB_RSP.StatusCode = cpu2le16(Event->u.GAS_CB_REQ_DATA.StatusCode);
		FrameLen += 2;

		if (bGASQueryRspFragFound)
			GASFrame->u.GAS_CB_RSP.GASRspFragID = (GASQueryRspFrag->GASRspFragID & 0x7F);
		else
			GASFrame->u.GAS_CB_RSP.GASRspFragID = 0;

		DBGPRINT(RT_DEBUG_OFF, ("GASRspFragID = %d\n", GASFrame->u.GAS_CB_RSP.GASRspFragID));

		FrameLen += 1;

		GASFrame->u.GAS_CB_RSP.GASComebackDelay = 0;
		FrameLen += 2;

		Pos = GASFrame->u.GAS_CB_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */

		*Pos++ = Event->u.GAS_CB_REQ_DATA.AdvertisementProID; /* Advertisement Protocol ID field */

		if (Event->u.GAS_CB_REQ_DATA.StatusCode == 0)
		{

			tmpLen = cpu2le16(GASQueryRspFrag->FragQueryRspLen);

			memmove(Pos, &tmpLen, 2);
			Pos += 2;
			FrameLen +=	6;

			memmove(Pos, GASQueryRspFrag->FragQueryRsp,
								GASQueryRspFrag->FragQueryRspLen);

			FrameLen += GASQueryRspFrag->FragQueryRspLen;
		}
		else
		{
			tmpLen = 0;

			memmove(Pos, &tmpLen, 2);
			Pos += 2;
			FrameLen +=	6;
		}

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListDel(&GASPeerEntry->List);

		DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp,
			&GASPeerEntry->GASQueryRspFragList, GAS_QUERY_RSP_FRAGMENT, List)
		{
			DlListDel(&GASQueryRspFrag->List);
			kfree(GASQueryRspFrag->FragQueryRsp);
			kfree(GASQueryRspFrag);
			GASPeerEntry->FreeResource += 2;
		}

		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		if (GASPeerEntry->PostReplyTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
			GASPeerEntry->PostReplyTimerRunning = FALSE;
		}

		if (GASPeerEntry->GASRspBufferingTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
			GASPeerEntry->GASRspBufferingTimerRunning = FALSE;
		}
		RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
		RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, & Cancelled);

		kfree(GASPeerEntry);
	}
	else if (Event->EventType == GAS_CB_REQ_MORE)
	{

		GASFrame->u.GAS_CB_RSP.DialogToken = Event->u.GAS_CB_REQ_MORE_DATA.DialogToken;
		FrameLen += 1;

		GASFrame->u.GAS_CB_RSP.StatusCode = cpu2le16(Event->u.GAS_CB_REQ_MORE_DATA.StatusCode);
		FrameLen += 2;

		GASFrame->u.GAS_CB_RSP.GASRspFragID = (0x80 | (GASQueryRspFrag->GASRspFragID & 0x7F));
		DBGPRINT(RT_DEBUG_OFF, ("GASRspFragID = %d\n", GASFrame->u.GAS_CB_RSP.GASRspFragID));
		FrameLen += 1;

		GASFrame->u.GAS_CB_RSP.GASComebackDelay = 0;
		FrameLen += 2;

		Pos = GASFrame->u.GAS_CB_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */

		*Pos++ = Event->u.GAS_CB_REQ_MORE_DATA.AdvertisementProID; /* Advertisement Protocol ID field */

		tmpLen = cpu2le16(GASQueryRspFrag->FragQueryRspLen);

		memmove(Pos, &tmpLen, 2);
		Pos += 2;
		FrameLen +=	6;

		memmove(Pos, GASQueryRspFrag->FragQueryRsp,
							GASQueryRspFrag->FragQueryRspLen);

		FrameLen += GASQueryRspFrag->FragQueryRspLen;

		GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_CB_REQ);
	}

	MiniportMMRequest(pAd, 0, Buf, FrameLen);

	kfree(Buf);
}


VOID ReceiveGASCBReq(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	GAS_EVENT_DATA *Event;
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;
	GAS_PEER_ENTRY *GASPeerEntry;
	PGAS_CTRL pGASCtrl = NULL;
	UCHAR APIndex, *Buf;
	uint32_t Len = 0;
	BOOLEAN IsGASCBReqMore = FALSE;
	BOOLEAN IsFound = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		if (MAC_ADDR_EQUAL(GASFrame->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid))
		{
			pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
			break;
		}
	}

	if (!pGASCtrl)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Can not find Peer Control\n", __FUNCTION__));
		return;
	}

	Buf = kmalloc(sizeof(*Event), GFP_ATOMIC);
	if (!Buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	memset(Buf, sizeof(*Event));

	Event = (GAS_EVENT_DATA *)Buf;

	Event->ControlIndex = APIndex;
	Len += 1;

	memmove(Event->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			if ((GASPeerEntry->CurrentGASFragNum + 1) <
				GASPeerEntry->GASRspFragNum)
				IsGASCBReqMore = TRUE;

			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	if (!IsGASCBReqMore)
	{
		if(IsFound)
		{
			Event->EventType = GAS_CB_REQ;
			Len += 2;

			Event->u.GAS_CB_REQ_DATA.DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
 			Len += 1;

			Event->u.GAS_CB_REQ_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
			Len += 1;

			if (pGASCtrl->ExternalANQPServerTest == 2)
				Event->u.GAS_CB_REQ_DATA.StatusCode = SERVER_UNREACHABLE;
			else
				Event->u.GAS_CB_REQ_DATA.StatusCode = 0;

			Len += 2;

			MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_CB_REQ, Len, Buf, 0);
			RTMP_MLME_HANDLER(pAd);
		}
		else
		{
			GASPeerEntry = kmalloc(sizeof(*GASPeerEntry), GFP_ATOMIC);
			if (!GASPeerEntry) {
				DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
				goto error;
			}

			memset(GASPeerEntry, sizeof(*GASPeerEntry));

			GASPeerEntry->CurrentState = WAIT_GAS_CB_REQ;
			GASPeerEntry->ControlIndex = Event->ControlIndex;
			memmove(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
			GASPeerEntry->DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
			GASPeerEntry->AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL; // FIXME
			GASPeerEntry->Priv = pAd;
			DlListInit(&GASPeerEntry->GASQueryRspFragList);

			RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
			DlListAddTail(&pGASCtrl->GASPeerList, &GASPeerEntry->List);
			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

			Event->EventType = GAS_CB_REQ;
			Len += 2;

			Event->u.GAS_CB_REQ_DATA.DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
 			Len += 1;

			Event->u.GAS_CB_REQ_DATA.AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL;
			Len += 1;

			Event->u.GAS_CB_REQ_DATA.StatusCode = UNSPECIFIED_FAILURE;
			Len += 2;

			MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_CB_REQ, Len, Buf, 0);
			RTMP_MLME_HANDLER(pAd);
		}
	}
	else
	{
		Event->EventType = GAS_CB_REQ_MORE;
		Len += 2;

		Event->u.GAS_CB_REQ_MORE_DATA.DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
		Len += 1;

		Event->u.GAS_CB_REQ_MORE_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;

		Event->u.GAS_CB_REQ_MORE_DATA.StatusCode = 0;
		Len += 2;

		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_CB_REQ_MORE, Len, Buf,0);
		RTMP_MLME_HANDLER(pAd);
	}


error:
	kfree(Buf);
}


static BOOLEAN IsAdvertisementProIDValid(
	IN struct rtmp_adapter *pAd,
	IN enum DOT11U_ADVERTISMENT_PROTOCOL_ID AdvertisementProtoID)
{
	INT32 i;

	for (i = 0; i < sizeof(dot11GASAdvertisementID)
					/ sizeof(enum DOT11U_ADVERTISMENT_PROTOCOL_ID); i++)
	{
		if (AdvertisementProtoID == dot11GASAdvertisementID[i])
			return TRUE;
	}

	return FALSE;
}


void PostReplyTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
	GAS_PEER_ENTRY *GASPeerEntry = (GAS_PEER_ENTRY *)FunctionContext;
	PGAS_CTRL pGASCtrl;
   	struct rtmp_adapter *pAd;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	if (!GASPeerEntry)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: GASPeerEntry is NULL\n", __FUNCTION__));
		return;
	}

	pAd = GASPeerEntry->Priv;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __FUNCTION__));
		return;
	}

	pGASCtrl = &pAd->ApCfg.MBSSID[GASPeerEntry->ControlIndex].GASCtrl;

	GASPeerEntry->PostReplyTimerRunning = FALSE;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListDel(&GASPeerEntry->List);

	DlListInit(&GASPeerEntry->GASQueryRspFragList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
	GASPeerEntry->FreeResource += 1;

	RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, & Cancelled);
	GASPeerEntry->FreeResource += 1;

	GASPeerEntry->FreeResource += 1;

	kfree(GASPeerEntry);

}
BUILD_TIMER_FUNCTION(PostReplyTimeout);


VOID GASRspBufferingTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
	GAS_PEER_ENTRY *GASPeerEntry = (GAS_PEER_ENTRY *)FunctionContext;
	struct rtmp_adapter *pAd;
	PGAS_CTRL pGASCtrl;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag = NULL, *GASQueryRspFragTmp;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	if (!GASPeerEntry)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: GASPeerEntry is NULL\n", __FUNCTION__));
		return;
	}

	pAd = GASPeerEntry->Priv;


	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS
						  	| fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __FUNCTION__));
		return;
	}

	pGASCtrl = &pAd->ApCfg.MBSSID[GASPeerEntry->ControlIndex].GASCtrl;

	if (GASPeerEntry->GASRspBufferingTimerRunning)
	{
		GASPeerEntry->GASRspBufferingTimerRunning = FALSE;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListDel(&GASPeerEntry->List);

		DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp,
			&GASPeerEntry->GASQueryRspFragList, GAS_QUERY_RSP_FRAGMENT, List)
		{
			DlListDel(&GASQueryRspFrag->List);
			kfree(GASQueryRspFrag->FragQueryRsp);
			kfree(GASQueryRspFrag);
			GASPeerEntry->FreeResource += 2;
		}

		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
		GASPeerEntry->FreeResource += 1;

		RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, & Cancelled);
		GASPeerEntry->FreeResource += 1;

		GASPeerEntry->FreeResource += 1;

		kfree(GASPeerEntry);
	}
}
BUILD_TIMER_FUNCTION(GASRspBufferingTimeout);


static VOID SendGASIndication(
    IN struct rtmp_adapter *   pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{
	PGAS_PEER_ENTRY GASPeerEntry;
	GAS_EVENT_DATA *Event = (GAS_EVENT_DATA *)Elem->Msg;
	GAS_EVENT_DATA *GASRspEvent;
	PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	struct net_device *NetDev = pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.if_dev;
	UCHAR *Buf;
	uint32_t Len = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	/*
 	 * If advertisement protocol ID equals to the value
 	 * contained in any dot11GASAdvertisementID MID objects
 	 */
	if (IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID) &&
				!pGASCtrl->ExternalANQPServerTest)
	{

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		/* Init PostReply timer */
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
			{
				if (!GASPeerEntry->PostReplyTimerRunning)
				{
					RTMPSetTimer(&GASPeerEntry->PostReplyTimer, dot11GASResponseTimeout);
					GASPeerEntry->PostReplyTimerRunning = TRUE;
				}

				break;
			}
		}

		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP);

		if (Event->u.PEER_GAS_REQ_DATA.AdvertisementProID == ACCESS_NETWORK_QUERY_PROTOCOL)
		{
			/* Send anqp request indication to daemon */
			SendAnqpReqEvent(NetDev,
					 Event->PeerMACAddr,
					 Event->u.PEER_GAS_REQ_DATA.QueryReq,
					 Event->u.PEER_GAS_REQ_DATA.QueryReqLen);
		}
	}
	else if (IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID) &&
				pGASCtrl->ExternalANQPServerTest == 1) /* server not reachable for 2F test */
	{
		Buf = kmalloc(sizeof(*GASRspEvent), GFP_ATOMIC);
		if (!Buf) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
			return;
		}

		memset(Buf, sizeof(*GASRspEvent));

		GASRspEvent = (GAS_EVENT_DATA *)Buf;

		GASRspEvent->ControlIndex = Event->ControlIndex;
		Len += 1;
		memmove(GASRspEvent->PeerMACAddr, Event->PeerMACAddr, MAC_ADDR_LEN);
		Len += MAC_ADDR_LEN;

		GASRspEvent->EventType = GAS_RSP;
		Len += 2;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
			{
				GASRspEvent->u.GAS_RSP_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;
				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		GASRspEvent->u.GAS_RSP_DATA.StatusCode = SERVER_UNREACHABLE;
		Len += 2;

		GASRspEvent->u.GAS_RSP_DATA.GASComebackDelay = 0;
		Len += 2;

		GASRspEvent->u.GAS_RSP_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;

		GASRspEvent->u.GAS_RSP_DATA.QueryRspLen = 0;
		Len += 2;

		GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP);

		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP, Len, Buf, 0);
		RTMP_MLME_HANDLER(pAd);

		kfree(Buf);
	}
	else if (IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID) &&
				pGASCtrl->ExternalANQPServerTest == 2) /* server not reachable for 4F test */
	{
		Buf = kmalloc(sizeof(*GASRspEvent), GFP_ATOMIC);
		if (!Buf) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
			return;
		}

		memset(Buf, sizeof(*GASRspEvent));

		GASRspEvent = (GAS_EVENT_DATA *)Buf;

		GASRspEvent->ControlIndex = Event->ControlIndex;
		Len += 1;
		memmove(GASRspEvent->PeerMACAddr, Event->PeerMACAddr, MAC_ADDR_LEN);
		Len += MAC_ADDR_LEN;

		GASRspEvent->EventType = GAS_RSP_MORE;
		Len += 2;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
			{
				GASRspEvent->u.GAS_RSP_MORE_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;
				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		GASRspEvent->u.GAS_RSP_MORE_DATA.StatusCode = 0;
		Len += 2;

		GASRspEvent->u.GAS_RSP_MORE_DATA.GASComebackDelay = 1;
		Len += 2;

		GASRspEvent->u.GAS_RSP_MORE_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;

		GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP);
		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP_MORE, Len, Buf, 0);
		RTMP_MLME_HANDLER(pAd);

		kfree(Buf);
	}
	else if (!IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID))
	{
		/* Do not support this advertisement protocol, such as MIH */
		buf = kmalloc(sizeof(*GASRspEvent), GFP_ATOMIC);
		if (!Buf) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
			return;
		}

		memset(Buf, sizeof(*GASRspEvent));

		GASRspEvent = (GAS_EVENT_DATA *)Buf;

		GASRspEvent->ControlIndex = Event->ControlIndex;
		Len += 1;
		memmove(GASRspEvent->PeerMACAddr, Event->PeerMACAddr, MAC_ADDR_LEN);
		Len += MAC_ADDR_LEN;

		GASRspEvent->EventType = GAS_RSP;
		Len += 2;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
			{
				GASRspEvent->u.GAS_RSP_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;
				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		GASRspEvent->u.GAS_RSP_DATA.StatusCode = ADVERTISEMENT_PROTOCOL_NOT_SUPPORTED;
		Len += 2;

		GASRspEvent->u.GAS_RSP_DATA.GASComebackDelay = 0;
		Len += 2;

		GASRspEvent->u.GAS_RSP_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;

		GASRspEvent->u.GAS_RSP_DATA.QueryRspLen = 0;
		Len += 2;

		GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP);

		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP, Len, Buf, 0);
		RTMP_MLME_HANDLER(pAd);

		kfree(Buf);
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: should not in this case\n", __FUNCTION__));
}
#endif /* CONFIG_AP_SUPPORT */

static VOID GASCtrlInit(IN struct rtmp_adapter *pAd)
{
	PGAS_CTRL pGASCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif

#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;
	memset(pGASCtrl, sizeof(*pGASCtrl));
	NdisAllocateSpinLock(pAd, &pGASCtrl->GASPeerListLock);
	DlListInit(&pGASCtrl->GASPeerList);
#endif

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
		memset(pGASCtrl, sizeof(*pGASCtrl));
		NdisAllocateSpinLock(pAd, &pGASCtrl->GASPeerListLock);
		DlListInit(&pGASCtrl->GASPeerList);
	}
#endif
}

VOID GASCtrlExit(IN struct rtmp_adapter *pAd)
{
	PGAS_CTRL pGASCtrl;
	GAS_PEER_ENTRY *GASPeerEntry, *GASPeerEntryTmp;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag, *GASQueryRspFragTmp;
	BOOLEAN Cancelled;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif

#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	/* Remove all GAS peer entry */
	DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp,
						&pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{

		DlListDel(&GASPeerEntry->List);

		DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp,
			&GASPeerEntry->GASQueryRspFragList, GAS_QUERY_RSP_FRAGMENT, List)
		{
			DlListDel(&GASQueryRspFrag->List);
			kfree(GASQueryRspFrag->FragQueryRsp);
			kfree(GASQueryRspFrag);
		}

		DlListInit(&GASPeerEntry->GASQueryRspFragList);

		if (GASPeerEntry->GASResponseTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASResponseTimer, &Cancelled);
			GASPeerEntry->GASResponseTimerRunning = FALSE;
		}

		if (GASPeerEntry->GASCBDelayTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASCBDelayTimer, &Cancelled);
			GASPeerEntry->GASCBDelayTimerRunning = FALSE;
		}

		RTMPReleaseTimer(&GASPeerEntry->GASResponseTimer, &Cancelled);
		RTMPReleaseTimer(&GASPeerEntry->GASCBDelayTimer, &Cancelled);
		kfree(GASPeerEntry);

	}

	DlListInit(&pGASCtrl->GASPeerList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	NdisFreeSpinLock(&pGASCtrl->GASPeerListLock);
#endif

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		/* Remove all GAS peer entry */
		DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp,
							&pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{

			DlListDel(&GASPeerEntry->List);

			DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp,
				&GASPeerEntry->GASQueryRspFragList, GAS_QUERY_RSP_FRAGMENT, List)
			{
				DlListDel(&GASQueryRspFrag->List);
				kfree(GASQueryRspFrag->FragQueryRsp);
				kfree(GASQueryRspFrag);
			}

			DlListInit(&GASPeerEntry->GASQueryRspFragList);

			if (GASPeerEntry->PostReplyTimerRunning)
			{
				RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
				GASPeerEntry->PostReplyTimerRunning = FALSE;
			}

			if (GASPeerEntry->GASRspBufferingTimerRunning)
			{
				RTMPCancelTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
				GASPeerEntry->GASRspBufferingTimerRunning = FALSE;
			}

			RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
			RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
			kfree(GASPeerEntry);
		}

		DlListInit(&pGASCtrl->GASPeerList);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		NdisFreeSpinLock(&pGASCtrl->GASPeerListLock);
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID GASStateMachineInit(
			IN struct rtmp_adapter *pAd,
			IN STATE_MACHINE *S,
			OUT STATE_MACHINE_FUNC	Trans[])
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	GASCtrlInit(pAd);

	StateMachineInit(S,	(STATE_MACHINE_FUNC*)Trans, MAX_GAS_STATE, MAX_GAS_MSG, (STATE_MACHINE_FUNC)Drop, GAS_UNKNOWN, GAS_MACHINE_BASE);

#ifdef CONFIG_AP_SUPPORT
	StateMachineSetAction(S, WAIT_PEER_GAS_REQ, PEER_GAS_REQ, (STATE_MACHINE_FUNC)SendGASIndication);
	StateMachineSetAction(S, WAIT_GAS_RSP, GAS_RSP, (STATE_MACHINE_FUNC)SendGASRsp);
	StateMachineSetAction(S, WAIT_GAS_RSP, GAS_RSP_MORE, (STATE_MACHINE_FUNC)SendGASRsp);
	StateMachineSetAction(S, WAIT_GAS_RSP, GAS_CB_REQ, (STATE_MACHINE_FUNC)SendGASCBRsp);
	StateMachineSetAction(S, WAIT_GAS_CB_REQ, GAS_CB_REQ, (STATE_MACHINE_FUNC)SendGASCBRsp);
	StateMachineSetAction(S, WAIT_GAS_CB_REQ, GAS_CB_REQ_MORE, (STATE_MACHINE_FUNC)SendGASCBRsp);
#endif /* CONFIG_AP_SUPPORT */
}
