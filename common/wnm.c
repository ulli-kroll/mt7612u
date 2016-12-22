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
	wnm.c

	Abstract:
	Wireless Network Management(WNM)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

static char SolicitedMulticastAddr[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
									  0x00, 0x00, 0x00, 0x00, 0x01, 0xff};
static char AllNodeLinkLocalMulticastAddr[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00,
											 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											 0x00, 0x00, 0x00, 0x01};

static char link_local[] = {0xfe, 0x80};

#define IS_UNSPECIFIED_IPV6_ADDR(_addr)	\
		(!((_addr).ipv6_addr32[0] | (_addr).ipv6_addr32[1] | (_addr).ipv6_addr32[2] | (_addr).ipv6_addr32[3]))

#ifdef CONFIG_AP_SUPPORT
void wext_send_btm_query_event(struct net_device *net_dev, const char *peer_mac_addr,
							   const char *btm_query, u16 btm_query_len)
{
	struct btm_query_data *query_data;
	u16 buflen = 0;
	char *buf;

	buflen = sizeof(*query_data) + btm_query_len;
	buf = kmalloc(buflen, GFP_ATOMIC);
	memset(buf, buflen);

	query_data = (struct btm_query_data *)buf;
	query_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(query_data->peer_mac_addr, peer_mac_addr, 6);
	query_data->btm_query_len	= btm_query_len;
	memcpy(query_data->btm_query, btm_query, btm_query_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_WNM_BTM_QUERY, NULL, (PUCHAR)buf, buflen);

	kfree(buf);
}

void wext_send_btm_cfm_event(struct net_device *net_dev, const char *peer_mac_addr,
							 const char *btm_rsp, u16 btm_rsp_len)
{

	struct btm_rsp_data *rsp_data;
	u16 buflen = 0;
	char *buf;


	buflen = sizeof(*rsp_data) + btm_rsp_len;
	buf = kmalloc(buflen, GFP_ATOMIC);
	memset(buf, buflen);

	rsp_data = (struct btm_rsp_data *)buf;
	rsp_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(rsp_data->peer_mac_addr, peer_mac_addr, 6);
	rsp_data->btm_rsp_len	= btm_rsp_len;
	memcpy(rsp_data->btm_rsp, btm_rsp, btm_rsp_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
						OID_802_11_WNM_BTM_RSP, NULL, (PUCHAR)buf, buflen);

	kfree(buf);
}

void wext_send_proxy_arp_event(struct net_device *net_dev,
							   const char *source_mac_addr,
							   const char *source_ip_addr,
							   const char *target_mac_addr,
							   const char *target_ip_addr,
							   u8 ip_type,
							   u8 from_ds)
{
	struct proxy_arp_entry *arp_entry;
	u16 varlen = 0, buflen = 0;
	char *buf;

	if (ip_type == IPV4)
		varlen += 8;
	else if (ip_type == IPV6)
		varlen += 32;

	buflen = sizeof(*arp_entry) + varlen;

	buf = kmalloc(buflen, GFP_ATOMIC);
	memset(buf, buflen);

	arp_entry = (struct proxy_arp_entry *)buf;

	arp_entry->ifindex = RtmpOsGetNetIfIndex(net_dev);
	arp_entry->ip_type = ip_type;
	arp_entry->from_ds = from_ds;
	memcpy(arp_entry->source_mac_addr, source_mac_addr, 6);
	memcpy(arp_entry->target_mac_addr, target_mac_addr, 6);

	if (ip_type == IPV4) {
		memcpy(arp_entry->ip_addr, source_ip_addr, 4);
		memcpy(arp_entry->ip_addr + 4, target_ip_addr, 4);
	} else if (ip_type == IPV6) {
		memcpy(arp_entry->ip_addr, source_ip_addr, 16);
		memcpy(arp_entry->ip_addr + 16, target_ip_addr, 16);
	} else
		printk("error not such ip type packet\n");

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
						OID_802_11_WNM_PROXY_ARP, NULL, (PUCHAR)buf, buflen);

	kfree(buf);
}


void SendProxyARPEvent(struct net_device *net_dev,
					   const char *source_mac_addr,
					   const char *source_ip_addr,
					   const char *target_mac_addr,
					   const char *target_ip_addr,
				  	   u8 ip_type,
					   u8 from_ds)
{
	wext_send_proxy_arp_event(net_dev,
							  source_mac_addr,
							  source_ip_addr,
							  target_mac_addr,
							  target_ip_addr,
							  ip_type,
							  from_ds);
}


BOOLEAN IsGratuitousARP(UCHAR *pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP)
	{
		/*
 		 * Check if Gratuitous ARP, Sender IP equal Target IP
 		 */
		SenderIP = Pos + 14;
		TargetIP = Pos + 24;
		if (NdisCmpMemory(SenderIP, TargetIP, 4) == 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The Packet is GratuitousARP\n"));
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN IsUnsolicitedNeighborAdver(struct rtmp_adapter *pAd,
								   PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		Pos += 24;

		if (RTMPEqualMemory(Pos, AllNodeLinkLocalMulticastAddr, 16))
		{
			Pos += 16;

			/* Check if neighbor advertisement type */
			if (*Pos == 0x88)
			{
				Pos += 4;

				/* Check if solicited flag set to 0 */
				if ((*Pos & 0x40) == 0x00)
				{
					DBGPRINT(RT_DEBUG_OFF, ("The Packet is UnsolicitedNeighborAdver\n"));
					Pos += 4;
					return TRUE;
				}
			}
		}

	}

	return FALSE;
}


BOOLEAN IsIPv4ProxyARPCandidate(IN struct rtmp_adapter *pAd,
						   		IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UINT16 ARPOperation;
	UCHAR *SenderIP;
	UCHAR *TargetIP;


	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP)
	{
		Pos += 6;
		memmove(&ARPOperation, Pos, 2);
		ARPOperation = OS_NTOHS(ARPOperation);
		Pos += 2;

		if (ARPOperation == 0x0001)
		{
			SenderIP = Pos + 6;
			TargetIP = Pos + 16;
			/* ARP Request */
			if (NdisCmpMemory(SenderIP, TargetIP, 4) != 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("IPv4ProxyARPCandidate\n"));
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOLEAN IsIpv6DuplicateAddrDetect(struct rtmp_adapter *pAd,
								  PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	RT_IPV6_ADDR *pIPv6Addr;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		Pos += 8;
		pIPv6Addr = (RT_IPV6_ADDR *)Pos;

		if (IS_UNSPECIFIED_IPV6_ADDR(*pIPv6Addr))
		{
			Pos += 16;
			if (RTMPEqualMemory(Pos, SolicitedMulticastAddr, 13))
			{
				Pos += 16;

				/* Check if neighbor solicitation */
				if (*Pos == 0x87)
				{
					DBGPRINT(RT_DEBUG_OFF, ("THe Packet is for Ipv6DuplicateAddrDetect\n"));
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOLEAN IsIPv6ProxyARPCandidate(IN struct rtmp_adapter *pAd,
								IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	RT_IPV6_ADDR *pIPv6Addr;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		Pos += 8;
		pIPv6Addr = (RT_IPV6_ADDR *)Pos;

		//if (!IS_UNSPECIFIED_IPV6_ADDR(*pIPv6Addr))
		//{
			Pos += 16;
			if (RTMPEqualMemory(Pos, SolicitedMulticastAddr, 13))
			{
				Pos+= 16;

				/* Check if neighbor solicitation */
				if (*Pos == 0x87)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("The Packet is IPv6ProxyARPCandidate\n"));
					return TRUE;
				}
			}
		//}
	}

	return FALSE;
}


BOOLEAN IsIPv6RouterSolicitation(IN struct rtmp_adapter *pAd,
								 IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		Pos += 40;

		/* Check if router solicitation */
		if (*Pos == 0x85)
		{
			DBGPRINT(RT_DEBUG_OFF, ("The Packet is IPv6 Router Solicitation\n"));
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN IsIPv6RouterAdvertisement(IN struct rtmp_adapter *pAd,
								  IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		Pos += 40;

		/* Check if router advertisement */
		if (*Pos == 0x86)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The Packet is IPv6 Router Advertisement\n"));
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN IsTDLSPacket(IN struct rtmp_adapter *pAd,
					 IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;

	memmove(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == 0x890d)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("THe Packet is TDLS\n"));
		return TRUE;
	}

	return FALSE;
}


uint32_t IPv4ProxyARPTableLen(IN struct rtmp_adapter *pAd,
							IN struct _MULTISSID_STRUCT *pMbss)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	uint32_t TableLen = 0;
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		TableLen += sizeof(PROXY_ARP_IPV4_UNIT);
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	return TableLen;
}

uint32_t IPv6ProxyARPTableLen(IN struct rtmp_adapter *pAd,
							IN struct _MULTISSID_STRUCT *pMbss)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	uint32_t TableLen = 0;
	int32_t Ret;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		TableLen += sizeof(PROXY_ARP_IPV6_UNIT);
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TableLen;
}

BOOLEAN GetIPv4ProxyARPTable(IN struct rtmp_adapter *pAd,
						 	 IN struct _MULTISSID_STRUCT *pMbss,
						 	 PUCHAR *ProxyARPTable)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	PROXY_ARP_IPV4_UNIT *ProxyARPUnit = (PROXY_ARP_IPV4_UNIT *)(*ProxyARPTable);
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
			memmove(ProxyARPUnit->TargetMACAddr, ProxyARPEntry->TargetMACAddr, MAC_ADDR_LEN);
			memmove(ProxyARPUnit->TargetIPAddr, ProxyARPEntry->TargetIPAddr, 4);
			ProxyARPUnit++;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	return TRUE;
}

BOOLEAN GetIPv6ProxyARPTable(IN struct rtmp_adapter *pAd,
						 	 IN struct _MULTISSID_STRUCT *pMbss,
						 	 PUCHAR *ProxyARPTable)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	PROXY_ARP_IPV6_UNIT *ProxyARPUnit = (PROXY_ARP_IPV6_UNIT *)(*ProxyARPTable);
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
			memmove(ProxyARPUnit->TargetMACAddr, ProxyARPEntry->TargetMACAddr, MAC_ADDR_LEN);
			ProxyARPUnit->TargetIPType = ProxyARPEntry->TargetIPType;
			memmove(ProxyARPUnit->TargetIPAddr, ProxyARPEntry->TargetIPAddr, 16);
			ProxyARPUnit++;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TRUE;
}

uint32_t AddIPv4ProxyARPEntry(IN struct rtmp_adapter *pAd,
					   		IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr,
							PUCHAR pTargetIPAddr)
{

	int i = 0;
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr))
		{
			RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
			return FALSE;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	ProxyARPEntry = kmalloc(sizeof(*ProxyARPEntry), GFP_ATOMIC);
	if (!ProxyARPEntry) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	memmove(ProxyARPEntry->TargetMACAddr, pTargetMACAddr, 6);
	memmove(ProxyARPEntry->TargetIPAddr, pTargetIPAddr, 4);

	for (i = 0; i < 4; i++)
		printk("pTargetIPv4Addr[%i] = %x\n", i, pTargetIPAddr[i]);

	/* Add ProxyARP Entry to list */
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListAddTail(&pWNMCtrl->IPv4ProxyARPList, &ProxyARPEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	return TRUE;
}

VOID RemoveIPv4ProxyARPEntry(IN struct rtmp_adapter *pAd,
					   		IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry, *ProxyARPEntryTmp;
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);

	DlListForEachSafe(ProxyARPEntry, ProxyARPEntryTmp, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		if (!ProxyARPEntry)
			break;

		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr))
		{
			//RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
			//return FALSE;
			DlListDel(&ProxyARPEntry->List);
			kfree(ProxyARPEntry);
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
}

uint32_t AddIPv6ProxyARPEntry(IN struct rtmp_adapter *pAd,
							IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr,
							PUCHAR pTargetIPAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	int32_t Ret;
	UINT8 i;
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr) &&
			IPV6_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, pTargetIPAddr))
		{
			RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
			return FALSE;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	ProxyARPEntry = kmalloc(sizeof(*ProxyARPEntry), GFP_ATOMIC);

	if (!ProxyARPEntry) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	memmove(ProxyARPEntry->TargetMACAddr, pTargetMACAddr, 6);

	if (NdisEqualMemory(link_local, pTargetIPAddr, 2))
		ProxyARPEntry->TargetIPType = IPV6_LINK_LOCAL;
	else
		ProxyARPEntry->TargetIPType = IPV6_GLOBAL;

	memmove(ProxyARPEntry->TargetIPAddr, pTargetIPAddr, 16);

	for (i = 0; i < 6; i++)
		printk("pTargetMACAddr[%i] = %x\n", i, pTargetMACAddr[i]);

	for (i = 0; i < 16; i++)
		printk("pTargetIPv6Addr[%i] = %x\n", i, pTargetIPAddr[i]);

	/* Add ProxyARP Entry to list */
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListAddTail(&pWNMCtrl->IPv6ProxyARPList, &ProxyARPEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TRUE;
}

VOID RemoveIPv6ProxyARPEntry(IN struct rtmp_adapter *pAd,
							IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry, *ProxyARPEntryTmp;
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);

	DlListForEachSafe(ProxyARPEntry, ProxyARPEntryTmp, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		if (!ProxyARPEntry)
			break;

		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr))
		{
			DlListDel(&ProxyARPEntry->List);
			kfree(ProxyARPEntry);
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
}

BOOLEAN IPv4ProxyARP(IN struct rtmp_adapter *pAd,
				 	 IN MULTISSID_STRUCT *pMbss,
				 	 IN PUCHAR pData,
					 IN BOOLEAN FromDS)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	struct net_device *NetDev = pMbss->wdev.if_dev;
	BOOLEAN IsFound = FALSE;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	PUCHAR SourceMACAddr = pData + 10;
	PUCHAR SourceIPAddr = pData + 16;
	PUCHAR TargetIPAddr = pData + 26;
	int32_t Ret;
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		if (IPV4_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, TargetIPAddr))
		{
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	if (IsFound)
	{
		/* Send proxy arp indication to daemon */
		SendProxyARPEvent(NetDev,
						  SourceMACAddr,
						  SourceIPAddr,
						  ProxyARPEntry->TargetMACAddr,
						  ProxyARPEntry->TargetIPAddr,
						  IPV4,
						  FromDS);
	}

	return IsFound;
}

BOOLEAN IPv6ProxyARP(IN struct rtmp_adapter *pAd,
					 IN MULTISSID_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	struct net_device *NetDev = pMbss->wdev.if_dev;
	BOOLEAN IsFound = FALSE;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	PUCHAR SourceMACAddr = pData + 68;
	PUCHAR SourceIPAddr = pData + 10;
	PUCHAR TargetIPAddr = pData + 50;
	int32_t Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		if (IPV6_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, TargetIPAddr))
		{
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	if (IsFound)
	{
		/* Send proxy arp indication to daemon */
		SendProxyARPEvent(NetDev,
						  SourceMACAddr,
						  SourceIPAddr,
						  ProxyARPEntry->TargetMACAddr,
						  ProxyARPEntry->TargetIPAddr,
						  IPV6,
						  FromDS);
	}

	return IsFound;
}


VOID WNMIPv4ProxyARPCheck(
			IN struct rtmp_adapter *pAd,
			struct sk_buff *pPacket,
			USHORT srcPort,
			USHORT dstPort,
			PUCHAR pSrcBuf)
{
	UCHAR apidx = RTMP_GET_PACKET_NET_DEVICE(pPacket);
	MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];

	if (srcPort  == 0x43 && dstPort == 0x44)
	{
		UCHAR *pTargetIPAddr = pSrcBuf + 24;
		/* Client hardware address */
		UCHAR *pTargetMACAddr = pSrcBuf + 36;

		if (pMbss->WNMCtrl.ProxyARPEnable)
		{
			/* Proxy MAC address/IP mapping */
			AddIPv4ProxyARPEntry(pAd, pMbss, pTargetMACAddr, pTargetIPAddr);
		}
	}
}


VOID WNMIPv6ProxyARPCheck(
			IN struct rtmp_adapter *pAd,
			struct sk_buff *pPacket,
			PUCHAR pSrcBuf)
{
	UCHAR apidx = RTMP_GET_PACKET_NET_DEVICE(pPacket);
	MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];

	if (pMbss->WNMCtrl.ProxyARPEnable)
	{
		/* Check if router advertisement, and add proxy entry */
		if (IsIPv6RouterAdvertisement(pAd, pSrcBuf - 2))
		{
			UCHAR *Pos = pSrcBuf + 4;
			UCHAR TargetIPAddr[16];
			INT16 PayloadLen;
			DBGPRINT(RT_DEBUG_OFF, ("This packet is router advertisement\n"));

			memmove(&PayloadLen, Pos, 2);
			PayloadLen = OS_NTOHS(PayloadLen);

			/* IPv6 options */
			Pos += 52;
			PayloadLen -= 16;

			while (PayloadLen > 0)
			{
				UINT8 OptionsLen = (*(Pos + 1)) * 8;

				/* Prefix information */
				if (*Pos == 0x03)
				{
					UCHAR *Prefix;
					int32_t Ret;
					PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
					PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;

					/* Prefix */
					Prefix = (Pos + 16);

					/* Copy global address prefix */
					memmove(TargetIPAddr, Prefix, 8);

					RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
					DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList,
										PROXY_ARP_IPV6_ENTRY, List)
					{
						if (ProxyARPEntry->TargetIPType == IPV6_LINK_LOCAL)
						{

							/* Copy host ipv6 interface identifier */
							memmove(&TargetIPAddr[8],
											&ProxyARPEntry->TargetIPAddr[8], 8);

							/* Proxy MAC address/IPv6 mapping for global address */
							AddIPv6ProxyARPEntry(pAd, pMbss, ProxyARPEntry->TargetMACAddr,
														TargetIPAddr);
						}
					}
					RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
				}

				Pos += OptionsLen;
				PayloadLen -= OptionsLen;
			}
		}
	}
}
#endif /* CONFIG_AP_SUPPORT */

void PeerWNMAction(IN struct rtmp_adapter *pAd,
				   IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	switch(Action)
	{
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("Invalid action field = %d\n", Action));
			break;
	}
}

VOID WNMCtrlInit(IN struct rtmp_adapter *pAd)
{
	PWNM_CTRL pWNMCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
		memset(pWNMCtrl, sizeof(*pWNMCtrl));
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->BTMPeerListLock, &pAd->RscSemMemList);
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->ProxyARPListLock, &pAd->RscSemMemList);
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->ProxyARPIPv6ListLock, &pAd->RscSemMemList);
		DlListInit(&pWNMCtrl->BTMPeerList);
		DlListInit(&pWNMCtrl->IPv4ProxyARPList);
		DlListInit(&pWNMCtrl->IPv6ProxyARPList);
	}
#endif
}


static VOID WNMCtrlRemoveAllIE(PWNM_CTRL pWNMCtrl)
{
	if (pWNMCtrl->TimeadvertisementIELen)
	{
		pWNMCtrl->TimeadvertisementIELen = 0;
		kfree(pWNMCtrl->TimeadvertisementIE);
	}

	if (pWNMCtrl->TimezoneIELen)
	{
		pWNMCtrl->TimezoneIELen = 0;
		kfree(pWNMCtrl->TimezoneIE);
	}
}


VOID WNMCtrlExit(IN struct rtmp_adapter *pAd)
{
	PWNM_CTRL pWNMCtrl;
	uint32_t Ret;
#ifdef CONFIG_AP_SUPPORT
	PROXY_ARP_IPV4_ENTRY *ProxyARPIPv4Entry, *ProxyARPIPv4EntryTmp;
	PROXY_ARP_IPV6_ENTRY *ProxyARPIPv6Entry, *ProxyARPIPv6EntryTmp;
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;

		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);

		DlListInit(&pWNMCtrl->BTMPeerList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->BTMPeerListLock);

		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
		/* Remove all proxy arp entry */
		DlListForEachSafe(ProxyARPIPv4Entry, ProxyARPIPv4EntryTmp,
							&pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
		{
			DlListDel(&ProxyARPIPv4Entry->List);
			kfree(ProxyARPIPv4Entry);
		}
		DlListInit(&pWNMCtrl->IPv4ProxyARPList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
		DlListForEachSafe(ProxyARPIPv6Entry, ProxyARPIPv6EntryTmp,
							&pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
		{
			DlListDel(&ProxyARPIPv6Entry->List);
			kfree(ProxyARPIPv6Entry);
		}
		DlListInit(&pWNMCtrl->IPv6ProxyARPList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->ProxyARPListLock);
		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->ProxyARPIPv6ListLock);

		/* Remove all WNM IEs */
		WNMCtrlRemoveAllIE(pWNMCtrl);
	}
#endif /* CONFIG_AP_SUPPORT */

}


#ifdef CONFIG_AP_SUPPORT
VOID Clear_All_PROXY_TABLE(IN struct rtmp_adapter *pAd)
{
	struct os_cookie *pObj = pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl;
	uint32_t Ret;
	PROXY_ARP_IPV4_ENTRY *ProxyARPIPv4Entry, *ProxyARPIPv4EntryTmp;
	PROXY_ARP_IPV6_ENTRY *ProxyARPIPv6Entry, *ProxyARPIPv6EntryTmp;

	pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	/* Remove all proxy arp entry */
	DlListForEachSafe(ProxyARPIPv4Entry, ProxyARPIPv4EntryTmp,
						&pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		DlListDel(&ProxyARPIPv4Entry->List);
		kfree(ProxyARPIPv4Entry);
	}
	DlListInit(&pWNMCtrl->IPv4ProxyARPList);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEachSafe(ProxyARPIPv6Entry, ProxyARPIPv6EntryTmp,
						&pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		DlListDel(&ProxyARPIPv6Entry->List);
		kfree(ProxyARPIPv6Entry);
	}
	DlListInit(&pWNMCtrl->IPv6ProxyARPList);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
}
#endif
