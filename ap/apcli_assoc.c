/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


	Module Name:
	apcli_assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-6-23		modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


static VOID ApCliMlmeAssocReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliMlmeDisassocReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerAssocRspAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerDisassocAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAssocTimeoutAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenAssoc(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenDisassociate(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAssocPostProc(
	IN struct rtmp_adapter *pAd,
	IN u8 *pAddr2,
	IN unsigned short CapabilityInfo,
	IN unsigned short IfIndex,
	IN u8 SupRate[],
	IN u8 SupRateLen,
	IN u8 ExtRate[],
	IN u8 ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN u8 HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo);

DECLARE_TIMER_FUNCTION(ApCliAssocTimeout);
BUILD_TIMER_FUNCTION(ApCliAssocTimeout);

/*
    ==========================================================================
    Description:
        association state machine init, including state transition and timer init
    Parameters:
        S - pointer to the association state machine
    Note:
        The state machine looks like the following
    ==========================================================================
 */
VOID ApCliAssocStateMachineInit(
	IN	struct rtmp_adapter *pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	u8 i;

	StateMachineInit(S, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_ASSOC_STATE, APCLI_MAX_ASSOC_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_ASSOC_IDLE,
		APCLI_ASSOC_MACHINE_BASE);

	/* first column */
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeAssocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDisassocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);

	/* second column */
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAssoc);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenDisassociate);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliPeerAssocRspAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_ASSOC_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAssocTimeoutAction);

	for (i=0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].AssocCurrState = APCLI_ASSOC_IDLE;

		/* timer init */
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.ApCliAssocTimer,
						GET_TIMER_FUNCTION(ApCliAssocTimeout), pAd, false);

		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.WpaDisassocAndBlockAssocTimer,
							GET_TIMER_FUNCTION(ApCliWpaDisassocApAndBlockAssoc), pAd, false);

	}

	return;
}

/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters
    ==========================================================================
 */
static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT \n"));

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_ASSOC_TIMEOUT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

	return;
}


/*
    ==========================================================================
    Description:
        mlme assoc req handling procedure
    Parameters:
        Adapter - Adapter pointer
        Elem - MLME Queue Element
    Pre:
        the station has been authenticated and the following information is stored in the config
            -# SSID
            -# supported rates and their length
    Post  :
        -# An association request frame is generated and sent to the air
        -# Association timer starts
        -# Association state -> ASSOC_WAIT_RSP

    ==========================================================================
 */
static VOID ApCliMlmeAssocReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	int 	 NStatus;
	bool          Cancelled;
	u8            ApAddr[6];
	HEADER_802_11    AssocHdr;
	u8            WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	unsigned short           ListenIntv;
	ULONG            Timeout;
	unsigned short           CapabilityInfo;
	u8 *          pOutBuffer = NULL;
	ULONG            FrameLen = 0;
	ULONG            tmp;
	u8            SsidIe    = IE_SSID;
	u8            SupRateIe = IE_SUPP_RATES;
	u8            ExtRateIe = IE_EXT_SUPP_RATES;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	unsigned long *pCurrState = NULL;
#ifdef WPA_SUPPLICANT_SUPPORT
	unsigned short 		VarIesOffset = 0;
#endif /* WPA_SUPPLICANT_SUPPORT */
	u8 RSNIe = IE_WPA;
	APCLI_STRUCT *apcli_entry;
	struct rtmp_wifi_dev *wdev;

	if ((ifIndex >= MAX_APCLI_NUM)
		)
		return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &apcli_entry->wdev;

	/* Block all authentication request durning WPA block period */
	if (apcli_entry->bBlockAssoc == true)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Block Auth request durning WPA block period!\n"));
		*pCurrState = APCLI_ASSOC_IDLE;
		ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}
	else if(MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
	{
		//RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, &Cancelled);
		RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, &Cancelled);

		/* allocate and send out AssocRsp frame */
		pOutBuffer = kmalloc( &pOutBuffer);  /*Get an unused nonpaged memory */
		if (pOutBuffer == NULL)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeAssocReqAction() allocate memory failed \n"));
			*pCurrState = APCLI_ASSOC_IDLE;

			ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

			return;
		}


		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Send ASSOC request...\n"));
		ApCliMgtMacHeaderInit(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr, ApAddr, ifIndex);


		/* Build basic frame first */
		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
			sizeof(HEADER_802_11),    &AssocHdr,
			2,                        &CapabilityInfo,
			2,                        &ListenIntv,
			1,                        &SsidIe,
			1,                        &apcli_entry->MlmeAux.SsidLen,
			apcli_entry->MlmeAux.SsidLen,     apcli_entry->MlmeAux.Ssid,
			1,                        &SupRateIe,
			1,                        &apcli_entry->MlmeAux.SupRateLen,
			apcli_entry->MlmeAux.SupRateLen,  apcli_entry->MlmeAux.SupRate,
			END_OF_ARGS);

		if(apcli_entry->MlmeAux.ExtRateLen != 0)
		{
			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
				1,                        &ExtRateIe,
				1,                        &apcli_entry->MlmeAux.ExtRateLen,
				apcli_entry->MlmeAux.ExtRateLen,  apcli_entry->MlmeAux.ExtRate,
				END_OF_ARGS);
			FrameLen += tmp;
		}

		/* HT */
		if ((apcli_entry->MlmeAux.HtCapabilityLen > 0) &&
			WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		{
			ULONG TmpLen;
			HT_CAPABILITY_IE HtCapabilityTmp;

			memset(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			memmove(&HtCapabilityTmp, &apcli_entry->MlmeAux.HtCapability, apcli_entry->MlmeAux.HtCapabilityLen);

#ifdef RT_BIG_ENDIAN
        		*(unsigned short *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(unsigned short *)(&HtCapabilityTmp.HtCapInfo));
        		*(unsigned short *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(unsigned short *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* RT_BIG_ENDINA */
        		MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
        							1,                           &HtCapIe,
        							1,                           &apcli_entry->MlmeAux.HtCapabilityLen,
        							apcli_entry->MlmeAux.HtCapabilityLen, &HtCapabilityTmp,
        							END_OF_ARGS);
			FrameLen += TmpLen;

			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
				(pAd->CommonCfg.Channel > 14) &&
				(apcli_entry->MlmeAux.vht_cap_len))
			{
				FrameLen += build_vht_ies(pAd, (u8 *)(pOutBuffer + FrameLen), SUBTYPE_ASSOC_REQ);
			}
		}

		{
			ULONG TmpLen;
			EXT_CAP_INFO_ELEMENT extCapInfo;
			u8 extInfoLen;

			extInfoLen = sizeof (EXT_CAP_INFO_ELEMENT);
			memset(&extCapInfo, extInfoLen);

#ifdef APCLI_CERT_SUPPORT
			if ((pAd->CommonCfg.bBssCoexEnable == true) &&
			    (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			    && (pAd->CommonCfg.Channel <= 14)
			    && (pAd->bApCliCertTest == true)
			    )
			{
				extCapInfo.BssCoexistMgmtSupport = 1;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: BssCoexistMgmtSupport = 1\n", __FUNCTION__));
			}
#endif /* APCLI_CERT_SUPPORT */
			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
				(pAd->CommonCfg.Channel > 14))
				extCapInfo.operating_mode_notification = 1;

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					  1, &ExtCapIe,
					  1, &extInfoLen,
								extInfoLen,			&extCapInfo,
								END_OF_ARGS);
			FrameLen += TmpLen;
		}


#ifdef AGGREGATION_SUPPORT
		/*
			add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION
			Case I: (Aggregation + Piggy-Back)
				1. user enable aggregation, AND
				2. Mac support piggy-back
				3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON
			Case II: (Aggregation)
				1. user enable aggregation, AND
				2. AP annouces it's AGGREGATION-capable in BEACON
		*/
		if (pAd->CommonCfg.bAggregationCapable)
		{
#ifdef PIGGYBACK_SUPPORT
			if ((pAd->CommonCfg.bPiggyBackCapable) && ((apcli_entry->MlmeAux.APRalinkIe & 0x00000003) == 3))
			{
				ULONG TmpLen;
				u8 RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			} else
#endif /* PIGGYBACK_SUPPORT */
			if (apcli_entry->MlmeAux.APRalinkIe & 0x00000001)
			{
				ULONG TmpLen;
				u8 RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
		else
		{
			ULONG TmpLen;
			u8 RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00};
			MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
							  9,						 RalinkIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif  /* AGGREGATION_SUPPORT */

		if (apcli_entry->MlmeAux.APEdcaParm.bValid)
		{
			if (apcli_entry->UapsdInfo.bAPSDCapable &&
				apcli_entry->MlmeAux.APEdcaParm.bAPSDCapable)
			{
				QBSS_STA_INFO_PARM QosInfo;

				memset(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
				WmeIe[8] |= *(u8 *)&QosInfo;
			}
			else
			{
                /* The Parameter Set Count is set to ¡§0¡¨ in the association request frames */
                /* WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  9,                        &WmeIe[0],
							  END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		apcli_entry->ReqVarIELen = 0;
	        memset(apcli_entry->ReqVarIEs, MAX_VIE_LEN);

                if ((apcli_entry->wpa_supplicant_info.WpaSupplicantUP & 0x7F ) ==  WPA_SUPPLICANT_ENABLE)
                {
                        DBGPRINT(RT_DEBUG_TRACE,("%s:: APCLI WPA_ASSOC_IE FROM SUPPLICANT\n", __FUNCTION__));
                        ULONG TmpWpaAssocIeLen = 0;
                        MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
                                        apcli_entry->wpa_supplicant_info.WpaAssocIeLen, apcli_entry->wpa_supplicant_info.pWpaAssocIe,
                                        END_OF_ARGS);

                        FrameLen += TmpWpaAssocIeLen;

                        VarIesOffset = 0;
                        memmove(apcli_entry->ReqVarIEs + VarIesOffset,
				       apcli_entry->wpa_supplicant_info.pWpaAssocIe, apcli_entry->wpa_supplicant_info.WpaAssocIeLen);
                        VarIesOffset += apcli_entry->wpa_supplicant_info.WpaAssocIeLen;

                        // Set Variable IEs Length
                        apcli_entry->ReqVarIELen = VarIesOffset;
                }
                else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		/* Append RSN_IE when WPAPSK OR WPA2PSK, */
		if (((wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
            		(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
#ifdef WPA_SUPPLICANT_SUPPORT
            		|| (wdev->AuthMode >= Ndis802_11AuthModeWPA)
#endif /* WPA_SUPPLICANT_SUPPORT */
            )
		{
			RSNIe = IE_WPA;

			if ((wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
				||(wdev->AuthMode == Ndis802_11AuthModeWPA2)
#endif /* WPA_SUPPLICANT_SUPPORT */
				)
				RSNIe = IE_WPA2;


#ifdef WPA_SUPPLICANT_SUPPORT
			if (wdev->AuthMode == Ndis802_11AuthModeWPA2)
			{
			INT idx;
                bool FoundPMK = false;
			/* Search chched PMKID, append it if existed */
				for (idx = 0; idx < PMKID_NO; idx++)
				{
					if (NdisEqualMemory(ApAddr, &apcli_entry->SavedPMK[idx].BSSID, 6))
					{
						FoundPMK = true;
						break;
					}
				}

				/*
					When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
					AP would not do PMK cache with STA after STA re-connect to AP again.
					In this case, driver doesn't need to send PMKID to AP and WpaSupplicant.
				*/
				if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) &&
					(NdisEqualMemory(pAd->MlmeAux.Bssid, pAd->CommonCfg.LastBssid, MAC_ADDR_LEN)))
				{
					FoundPMK = false;
				}

				if (FoundPMK)
				{
					// Set PMK number
					*(PUSHORT) &apcli_entry->RSN_IE[apcli_entry->RSNIE_Len] = 1;
					memmove(&apcli_entry->RSN_IE[apcli_entry->RSNIE_Len + 2], &apcli_entry->SavedPMK[idx].PMKID, 16);
                    			apcli_entry->RSNIE_Len += 18;
				}
			}

#ifdef SIOCSIWGENIE
			if ((apcli_entry->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE) &&
				(apcli_entry->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == true))
			{
				;
			}
			else
#endif
#endif /* WPA_SUPPLICANT_SUPPORT */

			MakeOutgoingFrame(pOutBuffer + FrameLen,    				&tmp,
			              	1,                                      	&RSNIe,
	                        1,                                      	&apcli_entry->RSNIE_Len,
	                        apcli_entry->RSNIE_Len,		apcli_entry->RSN_IE,
	                        END_OF_ARGS);

			FrameLen += tmp;
		}
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		kfree(pOutBuffer);

			RTMPSetTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, Timeout);
		*pCurrState = APCLI_ASSOC_WAIT_RSP;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeAssocReqAction() sanity check failed. BUG!!!!!! \n"));
		*pCurrState = APCLI_ASSOC_IDLE;

		ApCliCtrlMsg.Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}

	return;
}

/*
    ==========================================================================
    Description:
        Upper layer issues disassoc request
    Parameters:
        Elem -
    ==========================================================================
 */
static VOID ApCliMlmeDisassocReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_DISASSOC_REQ_STRUCT pDisassocReq;
	HEADER_802_11         DisassocHdr;
	u8 *                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	int           NStatus;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	unsigned long *pCurrState = NULL;

	if ((ifIndex >= MAX_APCLI_NUM)
	)
		return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	/* skip sanity check */
	pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

	/* allocate and send out DeassocReq frame */
	pOutBuffer = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);  /*Get an unused nonpaged memory */
	if (pOutBuffer == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeDisassocReqAction() allocate memory failed\n"));
		*pCurrState = APCLI_ASSOC_IDLE;

		ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Send DISASSOC request [BSSID::%02x:%02x:%02x:%02x:%02x:%02x] \n",
				pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
				pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5]));
	ApCliMgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr, ifIndex);


	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
						sizeof(HEADER_802_11),	&DisassocHdr,
						2,						&pDisassocReq->Reason,
						END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	kfree(pOutBuffer);

    *pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_SUCCESS;


	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

#ifdef WPA_SUPPLICANT_SUPPORT
	if (pAd->ApCfg.ApCliTab[ifIndex].wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
	{
		/*send disassociate event to wpa_supplicant*/
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}
	        RtmpOSWrielessEventSend(pAd->net_dev, SIOCGIWAP, -1, NULL, NULL, 0);
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0);
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	RT_CFG80211_LOST_GO_INFORM(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	return;
}


/*
    ==========================================================================
    Description:
        peer sends assoc rsp back
    Parameters:
        Elme - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerAssocRspAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	bool				Cancelled;
	unsigned short 			CapabilityInfo, Status, Aid;
	u8 			SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	u8 			ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	u8 			Addr2[MAC_ADDR_LEN];
	EDCA_PARM			EdcaParm;
	u8 			CkipFlag;
	APCLI_CTRL_MSG_STRUCT	ApCliCtrlMsg;
	HT_CAPABILITY_IE	HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	/* AP might use this additional ht info IE */
	u8 			HtCapabilityLen;
	u8 			AddHtInfoLen;
	u8 			NewExtChannelOffset = 0xff;
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	ULONG *pCurrState = NULL;
	PAPCLI_STRUCT pApCliEntry = NULL;
	IE_LISTS *ie_list = NULL;

	if ((ifIndex >= MAX_APCLI_NUM)
		)
		return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	ie_list = kmalloc(sizeof(IE_LISTS), GFP_ATOMIC);
	if (ie_list == NULL) {
		DBGPRINT(RT_DEBUG_OFF, ("%s():mem alloc failed!\n", __FUNCTION__));
		return;
	}

	memset((u8 *)ie_list, sizeof(IE_LISTS));

	if (ApCliPeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &CapabilityInfo, &Status, &Aid, SupRate, &SupRateLen, ExtRate, &ExtRateLen,
		&HtCapability, &AddHtInfo, &HtCapabilityLen,&AddHtInfoLen,&NewExtChannelOffset, &EdcaParm, &CkipFlag, ie_list))
	{
		/* The frame is for me ? */
		if(MAC_ADDR_EQUAL(Addr2, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - receive ASSOC_RSP to me (status=%d)\n", Status));
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
			/* Store the AssocRsp Frame to wpa_supplicant via CFG80211 */
            memset(pAd->ApCfg.ApCliTab[ifIndex].ResVarIEs, MAX_VIE_LEN);
            pAd->ApCfg.ApCliTab[ifIndex].ResVarIELen = 0;

            PFRAME_802_11 pFrame =  (PFRAME_802_11) (Elem->Msg);
            pAd->ApCfg.ApCliTab[ifIndex].ResVarIELen = Elem->MsgLen - 6 - sizeof (HEADER_802_11);
            memcpy(pAd->ApCfg.ApCliTab[ifIndex].ResVarIEs, &pFrame->Octet[6], pAd->ApCfg.ApCliTab[ifIndex].ResVarIELen);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.ApCliAssocTimer, &Cancelled);
			if(Status == MLME_SUCCESS)
			{
				/* go to procedure listed on page 376 */
				{
					ApCliAssocPostProc(pAd, Addr2, CapabilityInfo, ifIndex, SupRate, SupRateLen,
						ExtRate, ExtRateLen, &EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);
                    pAd->ApCfg.ApCliTab[0].MlmeAux.Aid = Aid;

					RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
					RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_op, sizeof(VHT_OP_IE));
					pApCliEntry->MlmeAux.vht_cap_len = 0;
					pApCliEntry->MlmeAux.vht_op_len = 0;
					if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && ie_list->vht_cap_len && ie_list->vht_op_len)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("There is vht le at Assoc Rsp ifIndex=%d vht_cap_len=%d\n", ifIndex,ie_list->vht_cap_len));
						memmove(&pApCliEntry->MlmeAux.vht_cap, &(ie_list->vht_cap), ie_list->vht_cap_len);
						pApCliEntry->MlmeAux.vht_cap_len = ie_list->vht_cap_len;
						memmove(&pApCliEntry->MlmeAux.vht_op, &(ie_list->vht_op), ie_list->vht_op_len);
						pApCliEntry->MlmeAux.vht_op_len = ie_list->vht_op_len;
					}
				}

				ApCliCtrlMsg.Status = MLME_SUCCESS;

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}
			else
			{
				ApCliCtrlMsg.Status = Status;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}

			*pCurrState = APCLI_ASSOC_IDLE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliPeerAssocRspAction() sanity check fail\n"));
	}

	if (ie_list)
		kfree(ie_list);

	return;
}

/*
    ==========================================================================
    Description:
        left part of IEEE 802.11/1999 p.374
    Parameters:
        Elem - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerDisassocAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	u8         Addr2[MAC_ADDR_LEN];
	unsigned short        Reason;
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	ULONG *pCurrState = NULL;

	if ((ifIndex >= MAX_APCLI_NUM)
		)
		return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;


	if(PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
	{
		if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, Addr2))
		{
			*pCurrState = APCLI_ASSOC_IDLE;

			{
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PEER_DISCONNECT_REQ, 0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
			}
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliPeerDisassocAction() sanity check fail\n"));
    }

	return;
}

/*
    ==========================================================================
    Description:
        what the state machine will do after assoc timeout
    ==========================================================================
 */
static VOID ApCliAssocTimeoutAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	unsigned long *pCurrState = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliAssocTimeoutAction\n"));

	if ((ifIndex >= MAX_APCLI_NUM)
		)
	return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	*pCurrState = APCLI_ASSOC_IDLE;


	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_REQ_TIMEOUT, 0, NULL, ifIndex);

	return;
}

static VOID ApCliInvalidStateWhenAssoc(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	unsigned long *pCurrState = NULL;

	if ((ifIndex >= MAX_APCLI_NUM)
		)
		return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliInvalidStateWhenAssoc(state=%ld), reset APCLI_ASSOC state machine\n", *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;


	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

static VOID ApCliInvalidStateWhenDisassociate(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	unsigned short ifIndex = (unsigned short)(Elem->Priv);
	ULONG *pCurrState = NULL;

	if ((ifIndex >= MAX_APCLI_NUM)
		)
		return;

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - InvalidStateWhenApCliDisassoc(state=%ld), reset APCLI_ASSOC state machine\n", *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;


	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

/*
    ==========================================================================
    Description:
        procedures on IEEE 802.11/1999 p.376
    Parametrs:
    ==========================================================================
 */
static VOID ApCliAssocPostProc(
	IN struct rtmp_adapter *pAd,
	IN u8 *pAddr2,
	IN unsigned short CapabilityInfo,
	IN unsigned short IfIndex,
	IN u8 SupRate[],
	IN u8 SupRateLen,
	IN u8 ExtRate[],
	IN u8 ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN u8 HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo)
{
	APCLI_STRUCT *pApCliEntry = NULL;

	if (IfIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	pApCliEntry->MlmeAux.BssType = BSS_INFRA;
	pApCliEntry->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	memmove(&pApCliEntry->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.SupRateLen = SupRateLen;
	memmove(pApCliEntry->MlmeAux.SupRate, SupRate, SupRateLen);
    RTMPCheckRates(pAd, pApCliEntry->MlmeAux.SupRate, &(pApCliEntry->MlmeAux.SupRateLen));

	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.ExtRateLen = ExtRateLen;
	memmove(pApCliEntry->MlmeAux.ExtRate, ExtRate, ExtRateLen);
    RTMPCheckRates(pAd, pApCliEntry->MlmeAux.ExtRate, &(pApCliEntry->MlmeAux.ExtRateLen));

	DBGPRINT(RT_DEBUG_TRACE, (HtCapabilityLen ? "%s===> 11n HT STA\n" : "%s===> legacy STA\n", __FUNCTION__));

	if (HtCapabilityLen > 0 && WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		ApCliCheckHt(pAd, IfIndex, pHtCapability, pAddHtInfo);

}

#ifdef WPA_SUPPLICANT_SUPPORT
VOID ApcliSendAssocIEsToWpaSupplicant(
    IN struct rtmp_adapter *pAd,
    IN UINT ifIndex)
{
	STRING custom[IW_CUSTOM_MAX] = {0};

	if ((pAd->ApCfg.ApCliTab[ifIndex].ReqVarIELen + 17) <= IW_CUSTOM_MAX)
	{
		sprintf(custom, "ASSOCINFO_ReqIEs=");
		memmove(custom+17, pAd->ApCfg.ApCliTab[ifIndex].ReqVarIEs, pAd->ApCfg.ApCliTab[ifIndex].ReqVarIELen);
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_REQIE_EVENT_FLAG, NULL, (u8 *)custom, pAd->ApCfg.ApCliTab[ifIndex].ReqVarIELen + 17);

		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_ASSOCINFO_EVENT_FLAG, NULL, NULL, 0);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.ApCliTab[%d].ReqVarIELen + 17 > MAX_CUSTOM_LEN\n",ifIndex));

	return;
}
#endif /*WPA_SUPPLICANT_SUPPORT */
#endif /* APCLI_SUPPORT */

