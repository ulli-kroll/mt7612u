/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_data.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#include "rt_config.h"

#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))

u8 SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
u8 SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
u8 EAPOL[] = {0x88, 0x8e};
u8   TPID[] = {0x81, 0x00}; /* VLAN related */

u8 IPX[] = {0x81, 0x37};
u8 APPLE_TALK[] = {0x80, 0xf3};


//  UserPriority To AccessCategory mapping
u8 WMM_UP2AC_MAP[8] = {QID_AC_BE, QID_AC_BK,
							QID_AC_BK, QID_AC_BE,
							QID_AC_VI, QID_AC_VI,
							QID_AC_VO, QID_AC_VO};

VOID RTMP_BASetup(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, UINT8 UPriority)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pEntry && (pEntry->NoBADataCountDown == 0) && IS_HT_STA(pEntry))
		{
			bool isRalink = false;
			/* Don't care the status of the portSecured status. */

			if (((pEntry->TXBAbitmap & (1<<UPriority)) == 0) /*&& (pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED)*/
				 && ((IS_ENTRY_CLIENT(pEntry) && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)) ||
					 IS_ENTRY_MESH(pEntry) || IS_ENTRY_WDS(pEntry) ||
				 	 (IS_ENTRY_APCLI(pEntry) && (isRalink == true) && (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)) ||
				 	 (pEntry->WepStatus == Ndis802_11WEPDisabled ||
						pEntry->WepStatus == Ndis802_11AESEnable
						))
				)
			{
				BAOriSessionSetUp(pAd, pEntry, UPriority, 0, 10, false);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if ((pAd->CommonCfg.BACapability.field.AutoBA == true) && (pEntry->NoBADataCountDown == 0) && IS_HT_STA(pEntry))
		{
			if (((pEntry->TXBAbitmap & (1 << UPriority)) == 0) &&
			    ((pEntry->BADeclineBitmap & (1 << UPriority)) == 0) &&
			    (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
			    &&
			    ((IS_ENTRY_CLIENT(pEntry) && pAd->MlmeAux.APRalinkIe != 0x0) || (pEntry->WepStatus != Ndis802_11WEPEnabled && pEntry->WepStatus != Ndis802_11TKIPEnable))
			    &&
			    (!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
			    )
			{
				BAOriSessionSetUp(pAd, pEntry, UPriority, 0, 10, false);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

}


/*
	========================================================================

	Routine Description:
		API for MLME to transmit management frame to AP (BSS Mode)
	or station (IBSS Mode)

	Arguments:
		pAd Pointer to our adapter
		pData		Pointer to the outgoing 802.11 frame
		Length		Size of outgoing management frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
int MiniportMMRequest(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN u8 *pData,
	IN UINT Length)
{
	struct sk_buff *pPacket;
	int Status = NDIS_STATUS_SUCCESS;
	ULONG FreeNum;
	bool bUseDataQ = false, FlgDataQForce = false, FlgIsLocked = false;
	int retryCnt = 0;

	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	if ((QueIdx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG)
	{
		bUseDataQ = true;
		QueIdx &= (~MGMT_USE_QUEUE_FLAG);
	}



	do
	{
		/* Reset is in progress, stop immediately*/
		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST |
								fRTMP_ADAPTER_RADIO_OFF)) ||
			 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
			)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

#ifdef CONFIG_STA_SUPPORT
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
			mt7612u_radio_on(pAd, MLME_RADIO_ON);
#endif /* CONFIG_STA_SUPPORT */

		/* Check Free priority queue*/
		/* Since we use PBF Queue2 for management frame.  Its corresponding DMA ring should be using TxRing.*/
		{
			FreeNum = GET_MGMTRING_FREENO(pAd);
		}

		if ((FreeNum > 0))
		{
			INT hw_len = MT_DMA_HDR_LEN + pAd->chipCap.TXWISize;
			u8 rtmpHwHdr[40];

			ASSERT((sizeof(rtmpHwHdr) > hw_len));

			/* We need to reserve space for rtmp hardware header. i.e., TxWI for RT2860 and TxInfo+TxWI for RT2870*/
			memset(&rtmpHwHdr, 0, hw_len);
			Status = RTMPAllocateNdisPacket(pAd, &pPacket, (u8 *)&rtmpHwHdr[0], hw_len, pData, Length);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
				break;
			}




			Status = MlmeHardTransmit(pAd, QueIdx, pPacket, FlgDataQForce, FlgIsLocked);
			if (Status == NDIS_STATUS_SUCCESS)
				retryCnt = 0;
			else
				dev_kfree_skb_any(pPacket);
		}
		else
		{
			pAd->RalinkCounters.MgmtRingFullCount++;
			DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in MgmtRing, MgmtRingFullCount=%ld!\n",
										QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
		}
	} while (retryCnt > 0);



	return Status;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		FlgIsDeltsFrame 1: the frame is a DELTS frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
void AP_QueuePsActionPacket(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pMacEntry,
	IN	struct sk_buff *pPacket,
	IN	bool			FlgIsDeltsFrame,
	IN	bool			FlgIsLocked,
	IN	u8 		MgmtQid)
{

	/* Note: for original mode of 4 AC are UAPSD, if station want to change
			the mode of a AC to legacy PS, we dont know where to put the
			response;
			1. send the response;
			2. but the station is in ps mode, so queue the response;
			3. we should queue the reponse to UAPSD queue because the station
				is not yet change its mode to legacy ps AC;
			4. so AP should change its mode to legacy ps AC only when the station
				sends a trigger frame and we send out the reponse;
			5. the mechanism is too complicate; */

	{
		/* DuplicatePkt = DuplicatePacket(get_netdev_from_bssid(pAd, pMacEntry->apidx), pPacket, pMacEntry->apidx);*/

#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
Label_Legacy_PS:
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		if (pMacEntry->PsQueue.Number >= MAX_PACKETS_IN_PS_QUEUE)
		{
			dev_kfree_skb_any(pPacket);
			return;
		}
		else
		{
			ULONG IrqFlags=0;

			DBGPRINT(RT_DEBUG_TRACE, ("ps> mgmt to legacy ps queue... (%d)\n", FlgIsDeltsFrame));

			if (FlgIsLocked == false)
				spin_lock_bh(&pAd->irq_lock);
			InsertTailQueue(&pMacEntry->PsQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
			if (FlgIsLocked == false)
				spin_unlock_bh(&pAd->irq_lock);
		}

		/* mark corresponding TIM bit in outgoing BEACON frame*/
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->apidx, pMacEntry->Aid);
	}
}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
int MlmeHardTransmit(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN struct sk_buff *pPacket,
	IN bool FlgDataQForce,
	IN bool FlgIsLocked)
{
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
	HEADER_802_11 *pHeader_802_11;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#endif /* CONFIG_AP_SUPPORT */
	PACKET_INFO PacketInfo;
	u8 *pSrcBufVA;
	UINT SrcBufLen;

	if ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
		)
	{
		return NDIS_STATUS_FAILURE;
	}

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + MT_DMA_HDR_LEN + TXWISize);

	/*
		Section 11.2.1.1 STA Power Management modes of IEEE802.11-2007:
		The Power Managment bit shall not be set in any management frame,
		except an Action frame.

		So in the 'baseline' test plan
		(Wi-Fi 802.11 WPA2, WPA, WEP Interoperability Test Plan),
		Section 2.2.6, the following Requirement:
        APs shall ignore the power save bit in any received Authenticate and
		(Re) Associate, and shall assume that the station is awake for the
		response.
	*/

	/*
		IEEE802.11, 11.2.1.4 AP operation during the contention period f)
		A single buffered MSDU or management frame for a STA in the PS mode shall
		be forwarded to the STA after a PS-Poll has been received from that STA.
		The More Data field shall be set to indicate the presence of further
		buffered MSDUs or "management frames" for the polling STA.
	*/

	/*
		IEEE802.11e, 11.2.1.4 Power management with APSD,
		An unscheduled SP ends after the QAP has attempted to transmit at least
		one MSDU or MMPDU associated with a delivery-enabled AC and destined for
		the non-AP QSTA, but no more than the number indicated in the Max SP
		Length field if the field has a nonzero value.
	*/

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA) ||
		(pHeader_802_11->FC.Type == FC_TYPE_MGMT))
	{
		if (pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL)
			pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}



	if ((pEntry != NULL) &&
		(pEntry->PsMode == PWR_SAVE) &&
		(((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_DATA_NULL) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL)) ||
		((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION)) ||
		((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION_NO_ACK))))
	{
		/* the peer is in PS mode, we need to queue the management frame */
		UINT8 FlgIsDeltsFrame = 0, MgmtQid = QID_AC_VO;

		/*
			1. Data & Not QoS Null, or
			2. Management & Action, or
			3. Management & Action No ACK;
		*/
		DBGPRINT(RT_DEBUG_TRACE, ("STA in ps mode, queue the mgmt frame\n"));
		RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);
		RTMP_SET_PACKET_MGMT_PKT(pPacket, 1); /* is management frame */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 0); /* default to management queue */



		if (FlgDataQForce == true)
			RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 1); /* force to data queue */

		if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
		{
			FRAME_ADDBA_REQ *pFrameBa = (FRAME_ADDBA_REQ *)pHeader_802_11;
			if (pFrameBa->Category == CATEGORY_BA)
				MgmtQid = QueIdx;
		}


		AP_QueuePsActionPacket(pAd, pEntry, pPacket, FlgIsDeltsFrame,
								FlgIsLocked, MgmtQid);
		return NDIS_STATUS_SUCCESS;
	}
    else
#endif /* CONFIG_AP_SUPPORT */
    {
    		return MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);
    }
}


int MlmeHardTransmitMgmtRing(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN struct sk_buff *pPacket)
{
	PACKET_INFO PacketInfo;
	u8 *pSrcBufVA;
	UINT SrcBufLen;
	HEADER_802_11 *pHeader_802_11;
	bool bAckRequired, bInsertTimestamp;
	u8 MlmeRate;
	struct mt7612u_txwi *pFirstTxWI;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	u8 PID, wcid, tx_rate;
	HTTRANSMIT_SETTING *transmit;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
	u8 TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	/* Make sure MGMT ring resource won't be used by other threads*/
	spin_lock_bh(&pAd->MgmtRingLock);
	if (pSrcBufVA == NULL)
	{
		/* The buffer shouldn't be NULL*/
			spin_unlock_bh(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* outgoing frame always wakeup PHY to prevent frame lost*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
			AsicForceWakeup(pAd, true);
	}
#endif /* CONFIG_STA_SUPPORT */

	pFirstTxWI = (struct mt7612u_txwi *)(pSrcBufVA +  MT_DMA_HDR_LEN);
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + MT_DMA_HDR_LEN + TXWISize);

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/* Verify Mlme rate for a / g bands.*/
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Fixed W52 with Activity scan issue in ABG_MIXED and ABGN_MIXED mode.*/
		// TODO: shiang-6590, why we need this condition check here?
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_A | WMODE_B | WMODE_G)
			|| WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_A | WMODE_B | WMODE_G | WMODE_AN | WMODE_GN)
			|| WMODE_CAP(pAd->CommonCfg.PhyMode, WMODE_AC)
		)
		{
			if (pAd->LatchRfRegs.Channel > 14)
			{
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
			}
			else
			{
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
				pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_0;
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

//CFG_TODO YF


	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		{
			/* We are in scan progress, just let the PwrMgmt bit keep as it orginally should be.*/
		}
		else
#endif /* CONFIG_STA_SUPPORT */
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
#ifdef CONFIG_STA_SUPPORT
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	/*
		In WMM-UAPSD, mlme frame should be set psm as power saving but probe
		request frame, Data-Null packets alse pass through MMRequest in RT2860,
		however, we hope control the psm bit to pass APSD
	*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if ((pHeader_802_11->FC.SubType == SUBTYPE_ACTION) ||
			((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
			((pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL) ||
			(pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL))))
		{
			if (RtmpPktPmBitCheck(pAd) == true)
				pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
			else if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
					INFRA_ON(pAd) &&
					RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
			{
				/* We are in scan progress, just let the PwrMgmt bit keep as it orginally should be */
			}
			else
			{
				pHeader_802_11->FC.PwrMgmt = pAd->CommonCfg.bAPSDForcePowerSave;
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(pPacket);
#endif /* CONFIG_AP_SUPPORT */


	bInsertTimestamp = false;
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) /* must be PS-POLL*/
	{
#ifdef CONFIG_STA_SUPPORT
		/*Set PM bit in ps-poll, to fix WLK 1.2  PowerSaveMode_ext failure issue.*/
		if ((pAd->OpMode == OPMODE_STA) && (pHeader_802_11->FC.SubType == SUBTYPE_PS_POLL))
		{
			pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
		}
#endif /* CONFIG_STA_SUPPORT */
		bAckRequired = false;

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
		{
			pHeader_802_11->Duration = 100;
			//DBGPRINT(RT_DEBUG_OFF, ("%s(): VHT_NDPA frame, rate=%d, len=%d, duration=%d\n",
			//			__FUNCTION__, MlmeRate, SrcBufLen, pHeader_802_11->Duration));
			//hex_dump("VHT_NDPA after update Duration", (u8 *)pHeader_802_11, SrcBufLen);

		}
	}
	else /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST*/
		{
			bAckRequired = false;
			pHeader_802_11->Duration = 0;
		}
		else
		{
			{
				bAckRequired = true;
				pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
				if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT))
				{
					bInsertTimestamp = true;
					bAckRequired = false; /* Disable ACK to prevent retry 0x1f for Probe Response*/
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
					/* Find which MBSSID to be send this probeRsp */
					uint32_t apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

					if ( !(apidx >= pAd->ApCfg.BssidNum) &&
					     (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) /* &&
					     (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_CCK) &&
					     (pAd->CommonCfg.MlmeTransmit.field.MCS == RATE_1)*/)
					{
						TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
					}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				}
				else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT))
				{
					bAckRequired = false; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				}
				else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) &&
						 (MacTableLookup(pAd, pHeader_802_11->Addr1) == NULL))
				{
					bAckRequired = false; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence >0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->Dot11_H.RDMode != RD_NORMAL_MODE))
	{
		spin_unlock_bh(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (u8 *)pHeader_802_11, DIR_WRITE, false);
#endif


	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size
	*/


	/*
		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/
	/*pAd->CommonCfg.MlmeTransmit.field.MODE = 1;*/

	/*
		management frame doesn't need encryption.
		so use RESERVED_WCID no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;


	if (pMacEntry == NULL)
	{
		wcid = RESERVED_WCID;
		tx_rate = (u8)pAd->CommonCfg.MlmeTransmit.field.MCS;
		transmit = &pAd->CommonCfg.MlmeTransmit;
		if (pAd->NDPA_Request)
		{
			transmit->field.MODE = MODE_VHT;
			transmit->field.MCS = MCS_RATE_6;
		}
	}
	else
	{
		if(
		   ((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
		   (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s:: Using Low Rate to send QOS NULL!!\n", __FUNCTION__));

			if ((pAd->LatchRfRegs.Channel > 14)
			)
			{
				pMacEntry->HTPhyMode.field.MODE = MODE_OFDM;
				pMacEntry->HTPhyMode.field.MCS = MCS_RATE_6;
			}
			else
			{
				pMacEntry->HTPhyMode.field.MODE = MODE_CCK;
				pMacEntry->HTPhyMode.field.MCS = MCS_0;
			}

			wcid = pMacEntry->Aid;
			tx_rate = (u8)pMacEntry->HTPhyMode.field.MCS;
			transmit = &pMacEntry->HTPhyMode;
		}
		else
		{
			/* dont use low rate to send QoS Null data frame */
			wcid = pMacEntry->Aid;
			tx_rate = (u8)pMacEntry->MaxHTPhyMode.field.MCS;
			transmit = &pMacEntry->MaxHTPhyMode;
		}
	}

	{
		RTMPWriteTxWI(pAd, pFirstTxWI, false, false, bInsertTimestamp, false, bAckRequired, false,
				0, wcid, (SrcBufLen - MT_DMA_HDR_LEN - TXWISize), PID, 0,
				tx_rate, IFS_BACKOFF, transmit);

#ifdef SPECIFIC_TX_POWER_SUPPORT
		if ((IS_RT6352(pAd) || IS_MT76x2U(pAd)) &&
			(pAd->chipCap.hif_type == HIF_RLT) && (pMacEntry == NULL))
			pFirstTxWI->TXWI_N.TxPwrAdj = TxPwrAdj;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	}

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (u8 *)pFirstTxWI, TYPE_TXWI);
#endif


	/* Now do hardware-depened kick out.*/
	HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen);

	/* Make sure to release MGMT ring resource*/
/*	if (!IrqState)*/
		spin_unlock_bh(&pAd->MgmtRingLock);
	return NDIS_STATUS_SUCCESS;
}


/********************************************************************************

	New DeQueue Procedures.

 ********************************************************************************/
#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags) 				\
			do{													\
				if (bIntContext == false)						\
				spin_lock_bh((lock));		\
			}while(0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
			do{													\
				if (bIntContext == false)						\
					spin_unlock_bh((lock));	\
			}while(0)


/*
	========================================================================
	Tx Path design algorithm:
		Basically, we divide the packets into four types, Broadcast/Multicast, 11N Rate(AMPDU, AMSDU, Normal), B/G Rate(ARALINK, Normal),
		Specific Packet Type. Following show the classification rule and policy for each kinds of packets.
				Classification Rule=>
					Multicast: (*addr1 & 0x01) == 0x01
					Specific : bDHCPFrame, bARPFrame, bEAPOLFrame, etc.
					11N Rate : If peer support HT
								(1).AMPDU  -- If TXBA is negotiated.
								(2).AMSDU  -- If AMSDU is capable for both peer and ourself.
											*). AMSDU can embedded in a AMPDU, but now we didn't support it.
								(3).Normal -- Other packets which send as 11n rate.

					B/G Rate : If peer is b/g only.
								(1).ARALINK-- If both of peer/us supprot Ralink proprietary Aggregation and the TxRate is large than RATE_6
								(2).Normal -- Other packets which send as b/g rate.
					Fragment:
								The packet must be unicast, NOT A-RALINK, NOT A-MSDU, NOT 11n, then can consider about fragment.

				Classified Packet Handle Rule=>
					Multicast:
								No ACK, 		pTxBlk->bAckRequired = false;
								No WMM, 		pTxBlk->bWMM = false;
								No piggyback,   pTxBlk->bPiggyBack = false;
								Force LowRate,  pTxBlk->bForceLowRate = true;
					Specific :	Basically, for specific packet, we should handle it specifically, but now all specific packets are use
									the same policy to handle it.
								Force LowRate,  pTxBlk->bForceLowRate = true;

					11N Rate :
								No piggyback,	pTxBlk->bPiggyBack = false;

								(1).AMSDU
									pTxBlk->bWMM = true;
								(2).AMPDU
									pTxBlk->bWMM = true;
								(3).Normal

					B/G Rate :
								(1).ARALINK

								(2).Normal
	========================================================================
*/
static u8 TxPktClassification(struct rtmp_adapter *pAd, struct sk_buff * pPacket, TX_BLK *pTxBlk)
{
	u8 TxFrameType = TX_UNKOWN_FRAME;
	u8 Wcid;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	bool	 bHTRate = false;

	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	if (Wcid == MCAST_WCID) {
		/* Handle for RA is Broadcast/Multicast Address.*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			// TODO: shiang-6590, fix me!
			u8 apidx = RTMP_GET_PACKET_NET_DEVICE_MBSSID(pPacket);
			if (apidx < pAd->ApCfg.BssidNum)
				pTxBlk->wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			else {
				DBGPRINT(RT_DEBUG_ERROR, ("%s(%d): WARN! pTxBlk->wdev not assigned!\n", __FUNCTION__, __LINE__));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		return TX_MCAST_FRAME;
	}

	/* Handle for unicast packets*/
	pMacEntry = &pAd->MacTab.Content[Wcid];
	pTxBlk->wdev = pMacEntry->wdev;
	if (RTMP_GET_PACKET_LOWRATE(pPacket)) {
		/* It's a specific packet need to force low rate, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame*/
		TxFrameType = TX_LEGACY_FRAME;
	} else if (IS_HT_RATE(pMacEntry)) {
		/* it's a 11n capable packet*/

		/* Depends on HTPhyMode to check if the peer support the HTRate transmission.*/
		/* 	Currently didn't support A-MSDU embedded in A-MPDU*/
		bHTRate = true;

		if (pMacEntry->TxSndgType == SNDG_TYPE_NDP &&
		    IS_VHT_RATE(pMacEntry)) {
			TxFrameType = TX_NDPA_FRAME;
		}

		if (RTMP_GET_PACKET_MOREDATA(pPacket) || (pMacEntry->PsMode == PWR_SAVE))
			TxFrameType |= TX_LEGACY_FRAME;
#ifdef WFA_VHT_PF
		else if (pAd->force_amsdu == true)
			return (TxFrameType | TX_AMSDU_FRAME);
#endif /* WFA_VHT_PF */
		else if ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) != 0)
			return (TxFrameType | TX_AMPDU_FRAME);
		else if(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED))
			return (TxFrameType | TX_AMSDU_FRAME);
		else
			TxFrameType |= TX_LEGACY_FRAME;

	} else {
		/* it's a legacy b/g packet.*/

		if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE) &&
		    pAd->CommonCfg.bAggregationCapable) &&
		    (RTMP_GET_PACKET_TXRATE(pPacket) >= RATE_6) &&
		    (!(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) &&
		    CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)))) {
				/* if peer support Ralink Aggregation, we use it.*/
			TxFrameType |= TX_RALINK_FRAME;
		} else {
			TxFrameType |= TX_LEGACY_FRAME;

		}
	}

	/* Currently, our fragment only support when a unicast packet send as NOT-ARALINK, NOT-AMSDU and NOT-AMPDU.*/

	if ((RTMP_GET_PACKET_FRAGMENTS(pPacket) > 1)  &&
	    /* ULLI : we must honour TX_RALINK_FRAME ?? */
	    ((TxFrameType == TX_LEGACY_FRAME) || (TxFrameType == (TX_LEGACY_FRAME | TX_NDPA_FRAME))) &&
	    ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) == 0))
		TxFrameType = TX_FRAG_FRAME;

	return TxFrameType;
}


bool RTMP_FillTxBlkInfo(struct rtmp_adapter *pAd, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	struct sk_buff *pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->apidx = RTMP_GET_PACKET_IF(pPacket);
	pTxBlk->wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);

	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
#ifdef CONFIG_AP_SUPPORT
	pTxBlk->pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);

	/* Default to clear this flag*/
	TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bForceNonQoS);


	if (pTxBlk->Wcid == MCAST_WCID)
	{
		pTxBlk->pMacEntry = NULL;
		{
			pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;
		}

		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);	/* AckRequired = false, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAllowFrag);
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
		if (RTMP_GET_PACKET_MOREDATA(pPacket))
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
		}
	}
	else
	{
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;

		pMacEntry = pTxBlk->pMacEntry;
#ifdef CONFIG_AP_SUPPORT
		pTxBlk->pMbss = pMacEntry->pMbss;
#endif /* CONFIG_AP_SUPPORT */

		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
		if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		else
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				if (IS_ENTRY_CLIENT(pMacEntry))
				{ }
				else
					return false;

				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{


				/* If support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) &&
					CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
#endif /* CONFIG_STA_SUPPORT */
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME)
		{
			if ( ((RTMP_GET_PACKET_LOWRATE(pPacket))
				) ||
				((pAd->OpMode == OPMODE_AP) &&
				 (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			)
			{	/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate.*/
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (IS_HT_STA(pTxBlk->pMacEntry) &&
					(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
					((pAd->CommonCfg.bRdg == true) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bForceNonQoS);
				}
			}

			if ( (IS_HT_RATE(pMacEntry) == false) &&
				(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)))
			{	/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
			}
		}
		else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);
		}

		pMacEntry->DebugTxCount++;
	}


	pAd->LastTxRate = (unsigned short)pTxBlk->pTransmit->word;

	return true;
}


bool CanDoAggregateTransmit(
	IN struct rtmp_adapter *pAd,
	IN struct sk_buff *pPacket,
	IN TX_BLK		*pTxBlk)
{
	int minLen = LENGTH_802_3;

	/*DBGPRINT(RT_DEBUG_TRACE, ("Check if can do aggregation! TxFrameType=%d!\n", pTxBlk->TxFrameType));*/

	if (RTMP_GET_PACKET_WCID(pPacket) == MCAST_WCID)
		return false;

	if (RTMP_GET_PACKET_DHCP(pPacket) ||
		RTMP_GET_PACKET_EAPOL(pPacket) ||
		RTMP_GET_PACKET_WAI(pPacket)
	)
		return false;

	/* Make sure the first packet has non-zero-length data payload */
	if (RTMP_GET_PACKET_VLAN(pPacket))
		minLen += LENGTH_802_1Q; /* VLAN tag */
	else if (RTMP_GET_PACKET_LLCSNAP(pPacket))
		minLen += 8; /* SNAP hdr Len*/
	if (minLen >= pPacket->len)
		return false;

	if ((pTxBlk->TxFrameType == TX_AMSDU_FRAME) &&
		((pTxBlk->TotalFrameLen + pPacket->len) > (RX_BUFFER_AGGRESIZE - 100)))
	{	/* For AMSDU, allow the packets with total length < max-amsdu size*/
		return false;
	}

	if ((pTxBlk->TxFrameType == TX_RALINK_FRAME) &&
		(pTxBlk->TxPacketList.Number == 2))
	{	/* For RALINK-Aggregation, allow two frames in one batch.*/
		return false;
	}

#ifdef CONFIG_STA_SUPPORT
	if ((INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA)) /* must be unicast to AP*/
		return true;
	else
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	/* CFG_TODO */
	if ((MAC_ADDR_EQUAL(pTxBlk->pPacket->data, pPacket->data))
	    && (pAd->OpMode == OPMODE_AP)) /* unicast to same STA*/
		return true;
	else
#endif /* CONFIG_AP_SUPPORT */
		return false;

}


/*
	========================================================================

	Routine Description:
		To do the enqueue operation and extract the first item of waiting
		list. If a number of available shared memory segments could meet
		the request of extracted item, the extracted item will be fragmented
		into shared memory segments.

	Arguments:
		pAd Pointer to our adapter
		pQueue		Pointer to Waiting Queue

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPDeQueuePacket(
	IN struct rtmp_adapter *pAd,
	IN bool bIntContext,
	IN u8 QIdx,
	IN INT Max_Tx_Packets)
{
	PQUEUE_ENTRY pEntry = NULL;
	struct sk_buff *pPacket;
	int Status = NDIS_STATUS_SUCCESS;
	u8 Count=0;
	PQUEUE_HEADER   pQueue;
	ULONG FreeNumber[NUM_OF_TX_RING];
	CHAR QueIdx, sQIdx, eQIdx;
	unsigned long	IrqFlags = 0;
	bool hasTxDesc = false;
	TX_BLK TxBlk, *pTxBlk;

#ifdef DBG_DIAGNOSE
	bool firstRound;
	RtmpDiagStruct *pDiagStruct = &pAd->DiagStruct;
#endif


	if (QIdx == NUM_OF_TX_RING)
	{
		sQIdx = 3;
		eQIdx = 0;	/* 4 ACs, start from 0.*/
	}
	else
	{
		sQIdx = eQIdx = QIdx;
	}

	for (QueIdx=sQIdx; QueIdx >= eQIdx; QueIdx--)
	{
		Count=0;

		RTMP_START_DEQUEUE(pAd, QueIdx, IrqFlags);

#ifdef DBG_DIAGNOSE
		firstRound = ((QueIdx == sQIdx) ? true : false);
#endif /* DBG_DIAGNOSE */

		while (1)
		{
			if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS |
										fRTMP_ADAPTER_RADIO_OFF |
										fRTMP_ADAPTER_RESET_IN_PROGRESS |
										fRTMP_ADAPTER_HALT_IN_PROGRESS |
										fRTMP_ADAPTER_NIC_NOT_EXIST |
										fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET)))

				)
			{
				RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);
				return;
			}

			if (Count >= Max_Tx_Packets)
				break;

			DEQUEUE_LOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			if (&pAd->TxSwQueue[QueIdx] == NULL)
			{
#ifdef DBG_DIAGNOSE
#ifdef DBG_TXQ_DEPTH
				if (firstRound == true)
					pDiagStruct->TxSWQueCnt[pDiagStruct->ArrayCurIdx][0]++;
#endif /* DBG_TXQ_DEPTH */
#endif /* DBG_DIAGNOSE */
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}


			/* probe the Queue Head*/
			pQueue = &pAd->TxSwQueue[QueIdx];
			if ((pEntry = pQueue->Head) == NULL)
			{
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}

			pTxBlk = &TxBlk;
			memset((u8 *)pTxBlk, 0, sizeof(TX_BLK));

			pTxBlk->QueIdx = QueIdx;

			pTxBlk->HeaderBuf = (u8 *)pTxBlk->HeaderBuffer;

			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);

#ifdef CONFIG_AP_SUPPORT
			if (RTMP_GET_PACKET_MGMT_PKT(pPacket) == 1)
			{
				/* this is a management frame */
				int status;

				pEntry = RemoveHeadQueue(pQueue);

					status = MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);

				if (status != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("tx queued mgmt frame error!\n"));
					dev_kfree_skb_any(pPacket);
				}

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				Count++;
				continue;
			}
			else
			{
				/*when WDS Jam happen, drop following 1min to HW TxRing Pkts*/
				MAC_TABLE_ENTRY *pMacEntry = NULL;
				u8 RAWcid;
				RAWcid = RTMP_GET_PACKET_WCID(pPacket);
				pMacEntry = &pAd->MacTab.Content[RAWcid];

				if (!IS_ENTRY_NONE(pMacEntry)
					&& (pMacEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
				{
					/*
						Sample Lin, 20100412

						For non-WDS interface, we need to send packet to detect
						the link periodically; Or when
						pMacEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck,
						no any chance to clear pMacEntry->ContinueTxFailCnt.

						EX: When pMacEntry->ContinueTxFailCnt >=
						pAd->ApCfg.EntryLifeCheck, the condition will not be
						removed and we will drop all packets for the pEntry.
						But maybe the signal becomes better.
						So we try to send a packet periodically and we will
						get the tx status in tx done interrupt.
						If the tx status is success, pMacEntry->ContinueTxFailCnt
						will be cleared to 0.
					*/
#define ENTRY_RETRY_INTERVAL	(100 * OS_HZ / 1000)
					ULONG Now32;

					NdisGetSystemUpTime(&Now32);
					if(RTMP_TIME_BEFORE(Now32, pMacEntry->TimeStamp_toTxRing + ENTRY_RETRY_INTERVAL))
					{
						pEntry = RemoveHeadQueue(pQueue);
						dev_kfree_skb_any(pPacket);
						DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
						Count++;
						continue;
					}
					else
					    NdisGetSystemUpTime(&pMacEntry->TimeStamp_toTxRing);
				}
			}
#endif /* CONFIG_AP_SUPPORT */

			/* Early check to make sure we have enoguh Tx Resource.*/
			hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
			if (!hasTxDesc)
			{
				pAd->PrivateInfo.TxRingFullCnt++;

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);

				break;
			}

			pTxBlk->TxFrameType = TxPktClassification(pAd, pPacket, pTxBlk);
			pEntry = RemoveHeadQueue(pQueue);
			pTxBlk->TotalFrameNum++;
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
			pTxBlk->TotalFrameLen += pPacket->len;
			pTxBlk->pPacket = pPacket;

			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));

			if (pTxBlk->TxFrameType & (TX_RALINK_FRAME | TX_AMSDU_FRAME))
			{
				// Enhance SW Aggregation Mechanism
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}
			}


			if (pTxBlk->TxFrameType == TX_RALINK_FRAME || pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			{
				// Enhance SW Aggregation Mechanism
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}

				do{
					if((pEntry = pQueue->Head) == NULL)
						break;

					/* For TX_AMSDU_FRAME/TX_RALINK_FRAME, Need to check if next pakcet can do aggregation.*/
					pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
					FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
					hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
					if ((hasTxDesc == false) || (CanDoAggregateTransmit(pAd, pPacket, pTxBlk) == false))
						break;

					/*Remove the packet from the TxSwQueue and insert into pTxBlk*/
					pEntry = RemoveHeadQueue(pQueue);
					ASSERT(pEntry);
					pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);


					pTxBlk->TotalFrameNum++;
					pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
					pTxBlk->TotalFrameLen += pPacket->len;
					InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
				}while(1);

				if (pTxBlk->TxPacketList.Number == 1)
					pTxBlk->TxFrameType = TX_LEGACY_FRAME;
			}

			DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);

			Count += pTxBlk->TxPacketList.Number;

			/* Do HardTransmit now.*/
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				Status = APHardTransmit(pAd, pTxBlk, QueIdx);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				Status = STAHardTransmit(pAd, pTxBlk, QueIdx);
			}
#endif /* CONFIG_STA_SUPPORT */


		}

		RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);

		if (!hasTxDesc)
			RTUSBKickBulkOut(pAd);

	}
}


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pAd 	Pointer to our adapter
		Rate			Transmit rate
		Size			Frame size in units of byte

	Return Value:
		Duration number in units of usec

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
unsigned short RTMPCalcDuration(
	IN	struct rtmp_adapter *pAd,
	IN	u8 		Rate,
	IN	ULONG			Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) /* CCK*/
	{
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	/* 72+24 preamble+plcp*/
		else
			Duration = 192; /* 144+48 preamble+plcp*/

		Duration += (unsigned short)((Size << 4) / RateIdTo500Kbps[Rate]);
		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration ++;
	}
	else if (Rate <= RATE_LAST_OFDM_RATE)/* OFDM rates*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
		Duration += 4 * (unsigned short)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	}
	else	/*mimo rate*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
	}

	return (unsigned short)Duration;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPSuspendMsduTransmission(
	IN struct rtmp_adapter *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCANNING, suspend MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	/*
		Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
		use Lowbound as R66 value on ScanNextChannel(...)
	*/
	mt7612u_phy_bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);

	pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	/* abort all TX rings */
	/*mt76u_reg_write(pAd, TX_CNTL_CSR, 0x000f0000);	*/
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPResumeMsduTransmission(
	IN struct rtmp_adapter *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCAN done, resume MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	/*
		After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
		R66 should not be 0
	*/
	if (pAd->BbpTuning.R66CurrentValue == 0)
	{
		pAd->BbpTuning.R66CurrentValue = 0x38;
		DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}
	mt7612u_phy_bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);

	pAd->CommonCfg.BBPCurrentBW = pAd->hw_cfg.bbp_bw;

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
/* sample, for IRQ LOCK to SEM LOCK */
/*
	IrqState = pAd->irq_disabled;
	if (IrqState)
		RTMPDeQueuePacket(pAd, true, NUM_OF_TX_RING, MAX_TX_PROCESS);
	else
*/
	RTMPDeQueuePacket(pAd, false, NUM_OF_TX_RING, MAX_TX_PROCESS);
}


UINT deaggregate_AMSDU_announce(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	struct sk_buff *	pPacket,
	IN	u8 *		pData,
	IN	ULONG			DataSize,
	IN	u8 		OpMode)
{
	unsigned short 			PayloadSize;
	unsigned short 			SubFrameSize;
	PHEADER_802_3 	pAMSDUsubheader;
	UINT			nMSDU;
    u8 		Header802_3[14];

	u8 *		pPayload, *pDA, *pSA, *pRemovedLLCSNAP;
	struct sk_buff *pClonePacket;

#ifdef CONFIG_AP_SUPPORT
	u8 FromWhichBSSID = RTMP_GET_PACKET_IF(pPacket);
	u8 VLAN_Size;
	unsigned short VLAN_VID = 0, VLAN_Priority = 0;


	if ((FromWhichBSSID < pAd->ApCfg.BssidNum)
		)
		VLAN_Size = (pAd->ApCfg.MBSSID[FromWhichBSSID].wdev.VLAN_VID != 0) ? LENGTH_802_1Q : 0;
#ifdef WDS_VLAN_SUPPORT
	else if ((FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS) &&
			(FromWhichBSSID < (MIN_NET_DEVICE_FOR_WDS + MAX_WDS_ENTRY)))
	{
		VLAN_Size = (pAd->WdsTab.WdsEntry[FromWhichBSSID - MIN_NET_DEVICE_FOR_WDS].wdev.VLAN_VID != 0) ? LENGTH_802_1Q : 0;
	}
#endif /* WDS_VLAN_SUPPORT */
	else /* only MBssid support VLAN.*/
		VLAN_Size = 0;
#endif /* CONFIG_AP_SUPPORT */

	nMSDU = 0;

	while (DataSize > LENGTH_802_3)
	{
		nMSDU++;

		/*hex_dump("subheader", pData, 64);*/
		pAMSDUsubheader = (PHEADER_802_3)pData;
		/*pData += LENGTH_802_3;*/
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0]<<8);
		SubFrameSize = PayloadSize + LENGTH_802_3;

		if ((DataSize < SubFrameSize) || (PayloadSize > 1518 ))
			break;

		/*DBGPRINT(RT_DEBUG_TRACE,("%d subframe: Size = %d\n",  nMSDU, PayloadSize));*/
		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;

		/* convert to 802.3 header*/
        CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);

#ifdef CONFIG_STA_SUPPORT
		if ((Header802_3[12] == 0x88) && (Header802_3[13] == 0x8E)
			)
		{
			MLME_QUEUE_ELEM *Elem;

			Elem = kmalloc(sizeof(MLME_QUEUE_ELEM), GFP_ATOMIC);
			if (Elem != NULL) {
				memmove(Elem->Msg+(LENGTH_802_11 + LENGTH_802_1_H), pPayload, PayloadSize);
				Elem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + PayloadSize;
				REPORT_MGMT_FRAME_TO_MLME(pAd, BSSID_WCID, Elem->Msg, Elem->MsgLen, 0, 0, 0, 0, OPMODE_STA);
				kfree(Elem);
				Elem = NULL;
			}
		}
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			u8 WhichBSSID = FromWhichBSSID;
			if (pRemovedLLCSNAP)
			{
				pPayload -= (LENGTH_802_3 + VLAN_Size);
				PayloadSize += (LENGTH_802_3 + VLAN_Size);
				/*memmove(pPayload, &Header802_3, LENGTH_802_3);*/
			}
			else
			{
				pPayload -= VLAN_Size;
				PayloadSize += VLAN_Size;
			}

			MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, WhichBSSID);

#ifdef WDS_VLAN_SUPPORT
			if (VLAN_VID == 0) /* maybe WDS packet */
				WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */

			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									Header802_3, LENGTH_802_3, pPayload,
									FromWhichBSSID, TPID);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
	        	if (pRemovedLLCSNAP)
	        	{
	    			pPayload -= LENGTH_802_3;
	    			PayloadSize += LENGTH_802_3;
	    			memmove(pPayload, &Header802_3[0], LENGTH_802_3);
	        	}
		}
#endif /* CONFIG_STA_SUPPORT */

		pClonePacket = ClonePacket(pPacket, pPayload, PayloadSize);
		if (pClonePacket)
		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pClonePacket, RTMP_GET_PACKET_IF(pPacket));
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pClonePacket, RTMP_GET_PACKET_IF(pPacket));
#endif /* CONFIG_STA_SUPPORT */
		}


		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize+3)&(~0x3);


		if (SubFrameSize > 1528 || SubFrameSize < 32)
			break;

		if (DataSize > SubFrameSize)
		{
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		}
		else
		{
			/* end of A-MSDU*/
			DataSize = 0;
		}
	}

	/* finally release original rx packet*/
	dev_kfree_skb_any(pPacket);

	return nMSDU;
}


UINT BA_Reorder_AMSDU_Annnounce(
	IN	struct rtmp_adapter *pAd,
	IN	struct sk_buff *pPacket,
	IN	u8 		OpMode)
{
	u8 *		pData;
	unsigned short 		DataSize;
	UINT			nMSDU = 0;

	pData = pPacket->data;
	DataSize = pPacket->len;

	nMSDU = deaggregate_AMSDU_announce(pAd, NULL, pPacket, pData, DataSize, OpMode);

	return nMSDU;
}

VOID Indicate_AMSDU_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	u8 		FromWhichBSSID)
{
	UINT			nMSDU;

	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, FromWhichBSSID);
	RTMP_SET_PACKET_IF(pRxBlk->pRxPacket, FromWhichBSSID);
	nMSDU = deaggregate_AMSDU_announce(pAd, pRxBlk, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID AssocParmFill(
	IN struct rtmp_adapter *pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN u8 *                    pAddr,
	IN unsigned short                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN unsigned short                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	/* Add mask to support 802.11b mode only */
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; /* not cf-pollable, not cf-poll-request*/
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID DisassocParmFill(
	IN struct rtmp_adapter *pAd,
	IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN u8 *pAddr,
	IN unsigned short Reason)
{
	COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
	DisassocReq->Reason = Reason;
}


/*
	NOTE: we do have an assumption here, that Byte0 and Byte1
		always reasid at the same scatter gather buffer
 */
static inline VOID Sniff2BytesFromNdisBuffer(
	IN char *buf,
	IN u8 offset,
	OUT u8 *p0,
	OUT u8 *p1)
{
	u8 *ptr = (u8 *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}


#define ETH_TYPE_VLAN	0x8100
#define ETH_TYPE_IPv4	0x0800
#define ETH_TYPE_IPv6	0x86dd
#define ETH_TYPE_ARP	0x0806
#define ETH_TYPE_EAPOL	0x888e
#define ETH_TYPE_WAI	0x88b4

#define IP_VER_CODE_V4	0x40
#define IP_VER_CODE_V6	0x60
#define IP_PROTO_UDP	0x11
#define IP_HDR_LEN		20
#define ETH_HDR_LEN		14

bool RTMPCheckEtherType(
	IN struct rtmp_adapter *pAd,
	IN struct sk_buff *pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN struct rtmp_wifi_dev *wdev,
	OUT u8 *pUserPriority,
	OUT u8 *pQueIdx)
{
	uint16_t TypeLen;
	u8 Byte0, Byte1, *pSrcBuf, up = 0;
	bool isMcast = false;

	pSrcBuf = pPacket->data;
	ASSERT(pSrcBuf);

	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);

#ifdef CONFIG_AP_SUPPORT
	if(IS_MULTICAST_MAC_ADDR(pSrcBuf))
		isMcast = true;
#endif /* CONFIG_AP_SUPPORT */

	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;
	if (TypeLen <= 1500)
	{	/* 802.3, 802.3 LLC*/
		/*
			DestMAC(6) + SrcMAC(6) + Lenght(2) +
			DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
		{
			Sniff2BytesFromNdisBuffer((char *)pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (unsigned short)((Byte0 << 8) + Byte1);
			pSrcBuf += 8; /* Skip this LLC/SNAP header*/
		} else {
			return false;
		}
	}

	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (TypeLen == ETH_TYPE_VLAN)
	{
#ifdef CONFIG_AP_SUPPORT
		unsigned short VLAN_VID = 0;

		/*
			802.3 VLAN packets format:

			DstMAC(6B) + SrcMAC(6B)
			+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes)
			+ Length/Type(2B)
			+ data payload (42-1500 bytes)
			+ FCS(4B)

			VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* No matter unicast or multicast, discard it if not my VLAN packet. */
		VLAN_VID = wdev->VLAN_VID;
		if (VLAN_VID != 0)
		{
			/* check if the packet is my VLAN */
			/* VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID */
			unsigned short vlan_id = *(unsigned short *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & 0x0FFF; /* 12 bit */
			if (vlan_id != VLAN_VID)	/* not my VLAN packet, discard it */
				return false;
		}
#endif /* CONFIG_AP_SUPPORT */

		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer((char *)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (unsigned short)((Byte0 << 8) + Byte1);

		/* only use VLAN tag */
		up = (*pSrcBuf & 0xe0) >> 5;

		pSrcBuf += 4; /* Skip the VLAN Header.*/
	}
	else if (TypeLen == ETH_TYPE_IPv4)
	{
		/*
			0       4       8          14  15                      31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen           |
			+---+----+-----+----+---------------+
			Ver    - 4bit Internet Protocol version number.
			IHL    - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN   - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
		*/
		up = (*(pSrcBuf + 1) & 0xe0) >> 5;
	}
	else if (TypeLen == ETH_TYPE_IPv6)
	{
		/*
			0       4       8        12     16                      31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label                   |
			+---+----+----+--------------------+
			Ver           - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
		*/
		up = ((*pSrcBuf) & 0x0e) >> 1;
	}


#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP)
	{
		MULTISSID_STRUCT *pMbss = (MULTISSID_STRUCT *)wdev->func_dev;
		if ((pMbss->FilterUnusedPacket & FILTER_IPV6_MC) == FILTER_IPV6_MC)
		{
			if (isMcast && (TypeLen == ETH_TYPE_IPv6))
				return false;
		}

		if ((pMbss->FilterUnusedPacket & FILTER_IPV4_MC) == FILTER_IPV4_MC)
		{
			if (isMcast && (TypeLen == ETH_TYPE_IPv4))
				return false;
		}

		if ((pMbss->FilterUnusedPacket & FILTER_IPV6_ALL) == FILTER_IPV6_ALL)
		{
			if (TypeLen == ETH_TYPE_IPv6)
				return false;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	switch (TypeLen)
	{
		case ETH_TYPE_IPv4:
			{
				uint32_t pktLen = pPacket->len;

				ASSERT((pktLen > (ETH_HDR_LEN + IP_HDR_LEN)));	/* 14 for ethernet header, 20 for IP header*/
				RTMP_SET_PACKET_IPV4(pPacket, 1);
				if (*(pSrcBuf + 9) == IP_PROTO_UDP)
				{
					uint16_t srcPort, dstPort;

					pSrcBuf += IP_HDR_LEN;
					srcPort = OS_NTOHS(get_unaligned((uint16_t *)(pSrcBuf)));
					dstPort = OS_NTOHS(get_unaligned((uint16_t *)(pSrcBuf+2)));

					if ((srcPort==0x44 && dstPort==0x43) || (srcPort==0x43 && dstPort==0x44))
					{	/*It's a BOOTP/DHCP packet*/
						RTMP_SET_PACKET_DHCP(pPacket, 1);
					}

#ifdef CONFIG_AP_SUPPORT
					if (wdev->wdev_type == WDEV_TYPE_AP)
					{
					}
#endif /* CONFIG_AP_SUPPORT */
				}
				else if (*(pSrcBuf + 9) == 0x01)
				{
					ASSERT((pktLen > 34));  /* 14 for ethernet header, 20 for IP header*/
					pSrcBuf += 20;  /* Skip the IP header*/

					if (*pSrcBuf == 0x08)
					{
						pSrcBuf++;
						if ((*pSrcBuf == 0x00) && (pktLen < 2000))
						{
							DBGPRINT(RT_DEBUG_TRACE, ("PS DEBUG: %s[%d]==PING PACKET (%d)\n",
								__FUNCTION__, __LINE__, pktLen));
						}
					}
				}
			}
			break;
		case ETH_TYPE_ARP:
			{
#ifdef CONFIG_AP_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_AP)
				{
				}
#endif /* CONFIG_AP_SUPPORT */
				RTMP_SET_PACKET_DHCP(pPacket, 1);
			}
			break;
		case ETH_P_IPV6:
			{
#ifdef CONFIG_AP_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_AP)
				{
				}
				/*
					Check if DHCPv6 Packet, and Convert group-address DHCP
					packets to individually-addressed 802.11 frames
 				 */
#endif /* CONFIG_AP_SUPPORT */
				/* return AC_BE if packet is not IPv6 */
				if ((*pSrcBuf & 0xf0) != 0x60)
					up = 0;
			}
			break;
		case ETH_TYPE_EAPOL:
			RTMP_SET_PACKET_EAPOL(pPacket, 1);
			break;


		default:
			break;
	}

	RTMP_SET_PACKET_PROTOCOL(pPacket, TypeLen);

	/* have to check ACM bit. downgrade UP & QueIdx before passing ACM*/
	/* NOTE: AP doesn't have to negotiate TSPEC. ACM is controlled purely via user setup, not protocol handshaking*/
	/*
		Under WMM ACM control, we dont need to check the bit;
		Or when a TSPEC is built for VO but we will change priority to
		BE here and when we issue a BA session, the BA session will
		be BE session, not VO session.
	*/
	if (pAd->CommonCfg.APEdcaParm.bACM[WMM_UP2AC_MAP[up]])
		up = 0;


	/*
		Set WMM when
		1. wdev->bWmmCapable == true
		2. Receiver's capability
			a). bc/mc packets
				->Need to get UP for IGMP use
			b). unicast packets
				-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
		3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr
	*/
	if ((wdev->bWmmCapable == true) && (up <= 7))
	{
		*pUserPriority = up;
		*pQueIdx = WMM_UP2AC_MAP[up];
	}

	return true;
}


VOID Update_Rssi_Sample(
	IN struct rtmp_adapter *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN struct mt7612u_rxwi *pRxWI)
{
	CHAR rssi[3];
	u8 snr[3];
	bool bInitial = false;
	CHAR Phymode = get_pkt_phymode_by_rxwi(pAd, pRxWI);

	if (!(pRssi->AvgRssi0 | pRssi->AvgRssi0X8 | pRssi->LastRssi0))
		bInitial = true;

	get_pkt_rssi_by_rxwi(pAd, pRxWI, 3, &rssi[0]);
	get_pkt_snr_by_rxwi(pAd, pRxWI, 3, &snr[0]);

	if (snr[0] != 0)
	{
		pRssi->LastSnr0 = ConvertToSnr(pAd, (u8)snr[0]);
		if (bInitial)
		{
			pRssi->AvgSnr0X8 = pRssi->LastSnr0 << 3;
			pRssi->AvgSnr0  = pRssi->LastSnr0;
		}
		else
			pRssi->AvgSnr0X8 = (pRssi->AvgSnr0X8 - pRssi->AvgSnr0) + pRssi->LastSnr0;

		pRssi->AvgSnr0 = pRssi->AvgSnr0X8 >> 3;
	}

	if (rssi[0] != 0)
	{
		pRssi->LastRssi0 = ConvertToRssi(pAd, (CHAR)rssi[0], RSSI_0);

		if (IS_MT76x2U(pAd)) {
			if ((Phymode == MODE_CCK)) {
				pRssi->LastRssi0 -= 2;
			} else if ((pRxWI->bw  == BW_80) && (pRssi->LastRssi0 < -75)
						&& (is_external_lna_mode(pAd, pAd->CommonCfg.Channel) == false)) {
				pRssi->LastRssi0 = (-92 + pRssi->LastSnr0);
			}
		}

		if (bInitial)
		{
			pRssi->AvgRssi0X8 = pRssi->LastRssi0 << 3;
			pRssi->AvgRssi0  = pRssi->LastRssi0;
		}
		else
			pRssi->AvgRssi0X8 = (pRssi->AvgRssi0X8 - pRssi->AvgRssi0) + pRssi->LastRssi0;

		pRssi->AvgRssi0 = pRssi->AvgRssi0X8 >> 3;
	}

	if (snr[1] != 0)
	{
		pRssi->LastSnr1 = ConvertToSnr(pAd, (u8)snr[1]);
		if (bInitial)
		{
			pRssi->AvgSnr1X8 = pRssi->LastSnr1 << 3;
			pRssi->AvgSnr1  = pRssi->LastSnr1;
		}
		else
			pRssi->AvgSnr1X8 = (pRssi->AvgSnr1X8 - pRssi->AvgSnr1) + pRssi->LastSnr1;

		pRssi->AvgSnr1 = pRssi->AvgSnr1X8 >> 3;
	}

	if (rssi[1] != 0)
	{
		pRssi->LastRssi1 = ConvertToRssi(pAd, (CHAR)rssi[1], RSSI_1);

		if (IS_MT76x2U(pAd)) {
			if ((Phymode == MODE_CCK)) {
				pRssi->LastRssi1 -= 2;
			}
				else if ((pRxWI->bw == BW_80) && (pRssi->LastRssi1 < -75)
						&& (is_external_lna_mode(pAd, pAd->CommonCfg.Channel) == false)) {
				pRssi->LastRssi1 = (-92 + pRssi->LastSnr1);
			}
		}

		if (bInitial)
		{
			pRssi->AvgRssi1X8 = pRssi->LastRssi1 << 3;
			pRssi->AvgRssi1  = pRssi->LastRssi1;
		}
		else
			pRssi->AvgRssi1X8 = (pRssi->AvgRssi1X8 - pRssi->AvgRssi1) + pRssi->LastRssi1;

		pRssi->AvgRssi1 = pRssi->AvgRssi1X8 >> 3;
	}

	if (rssi[2] != 0)
	{
		pRssi->LastRssi2 = ConvertToRssi(pAd, (CHAR)rssi[2], RSSI_2);

		if (bInitial)
		{
			pRssi->AvgRssi2X8 = pRssi->LastRssi2 << 3;
			pRssi->AvgRssi2  = pRssi->LastRssi2;
		}
		else
			pRssi->AvgRssi2X8 = (pRssi->AvgRssi2X8 - pRssi->AvgRssi2) + pRssi->LastRssi2;

		pRssi->AvgRssi2 = pRssi->AvgRssi2X8 >> 3;
	}
}



/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet(struct rtmp_adapter *pAd, RX_BLK *pRxBlk, u8 FromWhichBSSID)
{
	struct sk_buff *pRxPacket = pRxBlk->pRxPacket;
	u8 Header802_3[LENGTH_802_3];
	unsigned short VLAN_VID = 0, VLAN_Priority = 0;

//+++Add by shiang for debug
if (0) {
	hex_dump("Indicate_Legacy_Packet", pRxBlk->pData, pRxBlk->DataSize);
	hex_dump("802_11_hdr", (u8 *)pRxBlk->pHeader, LENGTH_802_11);
}
//---Add by shiang for debug

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef RT_CFG80211_SUPPORT
	CFG80211_Convert802_3Packet(pAd, pRxBlk, Header802_3);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

	if (pRxBlk->DataSize > MAX_RX_PKT_LEN)
	{
		dev_kfree_skb_any(pRxPacket);
		return;
	}

	STATS_INC_RX_PACKETS(pAd, FromWhichBSSID);

	if (pAd->CommonCfg.bDisableReordering == 0)
	{
		PBA_REC_ENTRY		pBAEntry;
		ULONG				Now32;
		u8 			Wcid = pRxBlk->wcid;
		u8 			TID = pRxBlk->TID;
		unsigned short 			Idx;

#define REORDERING_PACKET_TIMEOUT		((100 * OS_HZ)/1000)	/* system ticks -- 100 ms*/

		if (Wcid < MAX_LEN_OF_MAC_TABLE)
		{
			Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
			if (Idx != 0)
			{
				pBAEntry = &pAd->BATable.BARecEntry[Idx];
				/* update last rx time*/
				NdisGetSystemUpTime(&Now32);
				if ((pBAEntry->list.qlen > 0) &&
					 RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(REORDERING_PACKET_TIMEOUT)))
	   				)
				{
					DBGPRINT(RT_DEBUG_OFF, ("Indicate_Legacy_Packet():flush reordering_timeout_mpdus! RxWI->Flags=%d, pRxWI.TID=%d, RxD->AMPDU=%d!\n",
												pRxBlk->Flags, pRxBlk->TID, pRxBlk->pRxInfo->AMPDU));
					hex_dump("Dump the legacy Packet:", pRxBlk->pRxPacket->data, 64);
					ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, Now32);
				}
			}
		}
	}

#ifdef CONFIG_AP_SUPPORT
	MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pRxBlk->pData, pRxBlk->DataSize);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, FromWhichBSSID, TPID);
//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", pRxBlk->pRxPacket->data, pRxBlk->pRxPacket->len);
}
//---Add by shiang for debug


	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef RT_CFG80211_SUPPORT
	CFG80211_Announce802_3Packet(pAd, pRxBlk, FromWhichBSSID);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
}




/* Normal, AMPDU or AMSDU*/
VOID CmmRxnonRalinkFrameIndicate(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	u8 		FromWhichBSSID)
{
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
	}
	else
	{
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
			Indicate_AMSDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		else
		{
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
	}
}


/* Normal, AMPDU or AMSDU*/


VOID CmmRxRalinkFrameIndicate(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY *pEntry,
	IN	RX_BLK *pRxBlk,
	IN	u8 FromWhichBSSID)
{
	u8 		Header802_3[LENGTH_802_3];
	uint16_t 		Msdu2Size;
	uint16_t 			Payload1Size, Payload2Size;
	u8 *			pData2;
	struct sk_buff *pPacket2 = NULL;
	unsigned short 		VLAN_VID = 0, VLAN_Priority = 0;


	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData+1) << 8);
	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize))
	{
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	else
	{
		dev_kfree_skb_any(pRxBlk->pRxPacket);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_STA_SUPPORT */


	ASSERT(pRxBlk->pRxPacket);

	/* Ralink Aggregation frame*/
	pAd->RalinkCounters.OneSecRxAggregationCount ++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;

	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		unsigned short VLAN_VID = 0, VLAN_Priority = 0;

		MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */

		RT_VLAN_PKT_DUPLICATE(pPacket2, pAd, VLAN_VID, VLAN_Priority,
							(pData2-LENGTH_802_3), LENGTH_802_3, pData2,
							Payload2Size, FromWhichBSSID, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pPacket2 = duplicate_pkt(get_netdev_from_bssid(pAd, FromWhichBSSID),
								(pData2-LENGTH_802_3), LENGTH_802_3, pData2,
								Payload2Size, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */

	if (!pPacket2)
	{
		dev_kfree_skb_any(pRxBlk->pRxPacket);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
		if (VLAN_VID == 0) /* maybe WDS packet */
			WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, FromWhichBSSID, TPID);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxBlk->pRxPacket, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxBlk->pRxPacket, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */

	if (pPacket2)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pPacket2, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pPacket2, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */
	}
}


#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}


struct sk_buff *RTMPDeFragmentDataFrame(struct rtmp_adapter *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	struct sk_buff *pRxPacket = pRxBlk->pRxPacket;
	u8 *pData = pRxBlk->pData;
	unsigned short DataSize = pRxBlk->DataSize;
	struct sk_buff *pRetPacket = NULL;
	u8 *pFragBuffer = NULL;
	bool bReassDone = false;
	u8 HeaderRoom = 0;
	struct mt7612u_rxwi *pRxWI = pRxBlk->pRxWI;
	UINT8 RXWISize = pAd->chipCap.RXWISize;

	ASSERT(pHeader);

	HeaderRoom = pData - (u8 *)pHeader;

	/* Re-assemble the fragmented packets*/
	if (pHeader->Frag == 0)
	{	/* Frag. Number is 0 : First frag or only one pkt*/
		/* the first pkt of fragment, record it.*/
		if (pHeader->FC.MoreFrag)
		{
			ASSERT(pAd->FragFrame.pFragPacket);
			pFragBuffer = pAd->FragFrame.pFragPacket->data;
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items.
			    Copy RxWI content to pFragBuffer.
			*/
			//pAd->FragFrame.RxSize = DataSize + HeaderRoom;
			//memmove(pFragBuffer, pHeader, pAd->FragFrame.RxSize);
			pAd->FragFrame.RxSize = DataSize + HeaderRoom + RXWISize;
			memmove(pFragBuffer, pRxWI, RXWISize);
			memmove(pFragBuffer + RXWISize,	 pHeader, pAd->FragFrame.RxSize - RXWISize);
			pAd->FragFrame.Sequence = pHeader->Sequence;
			pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Should be 0*/
			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		}
	}
	else
	{	/*Middle & End of fragment*/
		if ((pHeader->Sequence != pAd->FragFrame.Sequence) ||
			(pHeader->Frag != (pAd->FragFrame.LastFrag + 1)))
		{
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done;
		}
		/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
		//else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE + RXWISize)
		{
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done;
		}


		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will drop it.*/
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pHeader->Sequence, pHeader->Frag));
			goto done;
		}

		pFragBuffer = pAd->FragFrame.pFragPacket->data;

		/* concatenate this fragment into the re-assembly buffer*/
		memmove((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Update fragment number*/

		/* Last fragment*/
		if (pHeader->FC.MoreFrag == false)
			bReassDone = true;
	}

done:
	/* always release rx fragmented packet*/
	dev_kfree_skb_any(pRxPacket);

	/* return defragmented packet if packet is reassembled completely*/
	/* otherwise return NULL*/
	if (bReassDone)
	{
		struct sk_buff *pNewFragPacket;

		/* allocate a new packet buffer for fragment*/
		pNewFragPacket = dev_alloc_skb(RX_BUFFER_NORMSIZE);
		if (pNewFragPacket)
		{
			/* update RxBlk*/
			pRetPacket = pAd->FragFrame.pFragPacket;
			pAd->FragFrame.pFragPacket = pNewFragPacket;
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
			//pRxBlk->pHeader = (PHEADER_802_11) pRetPacket->data;
			//pRxBlk->pData = (u8 *)pRxBlk->pHeader + HeaderRoom;
			//pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom;
			//pRxBlk->pRxPacket = pRetPacket;
			pRxBlk->pRxWI = (struct mt7612u_rxwi *) pRetPacket->data;
			pRxBlk->pHeader = (PHEADER_802_11) ((u8 *)pRxBlk->pRxWI + RXWISize);
			pRxBlk->pData = (u8 *)pRxBlk->pHeader + HeaderRoom;
			pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom - RXWISize;
			pRxBlk->pRxPacket = pRetPacket;
		}
		else
		{
			RESET_FRAGFRAME(pAd->FragFrame);
		}
	}

	return pRetPacket;
}


VOID Indicate_EAPOL_Packet(
	IN struct rtmp_adapter *pAd,
	IN RX_BLK *pRxBlk,
	IN u8 FromWhichBSSID)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pRxBlk->wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: invalid wcid.\n"));
		dev_kfree_skb_any(pRxBlk->pRxPacket);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: drop and release the invalid packet.\n"));
		dev_kfree_skb_any(pRxBlk->pRxPacket);
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APRxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		return;
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		{
			ASSERT((pRxBlk->wcid == BSSID_WCID));
			STARxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
			return;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

}


#ifdef SOFT_ENCRYPT
bool RTMPExpandPacketForSwEncrypt(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	uint32_t ex_head = 0, ex_tail = 0;
	u8 	NumberOfFrag = RTMP_GET_PACKET_FRAGMENTS(pTxBlk->pPacket);

	if (pTxBlk->CipherAlg == CIPHER_AES)
		ex_tail = LEN_CCMP_MIC;

	ex_tail = (NumberOfFrag * ex_tail);

	pTxBlk->pPacket = ExpandPacket(pAd, pTxBlk->pPacket, ex_head, ex_tail);
	if (pTxBlk->pPacket == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: out of resource.\n", __FUNCTION__));
		return false;
	}
	RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	return true;
}


VOID RTMPUpdateSwCacheCipherInfo(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN u8 *pHdr)
{
	HEADER_802_11 *pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader_802_11 = (HEADER_802_11 *) pHdr;
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry && pHeader_802_11->FC.Wep &&
		CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))
	{
		PCIPHER_KEY pKey = &pMacEntry->PairwiseKey;

		TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

		pTxBlk->CipherAlg = pKey->CipherAlg;
		pTxBlk->pKey = pKey;
		if ((pKey->CipherAlg == CIPHER_WEP64) || (pKey->CipherAlg == CIPHER_WEP128))
			inc_iv_byte(pKey->TxTsc, LEN_WEP_TSC, 1);
		else if ((pKey->CipherAlg == CIPHER_TKIP) || (pKey->CipherAlg == CIPHER_AES))
			inc_iv_byte(pKey->TxTsc, LEN_WPA_TSC, 1);

	}

}

#endif /* SOFT_ENCRYPT */

/*
	==========================================================================
	Description:
		Send out a NULL frame to a specified STA at a higher TX rate. The
		purpose is to ensure the designated client is okay to received at this
		rate.
	==========================================================================
 */
VOID RtmpEnqueueNullFrame(
	IN struct rtmp_adapter *pAd,
	IN u8 *pAddr,
	IN u8 TxRate,
	IN u8 AID,
	IN u8 apidx,
	IN bool bQosNull,
	IN bool bEOSP,
	IN u8 OldUP)
{
	int NState;
	HEADER_802_11 *pNullFr;
	u8 *pFrame;
	UINT frm_len;
	MAC_TABLE_ENTRY *pEntry;

	pFrame = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);
	pNullFr = (PHEADER_802_11) pFrame;

	if (pFrame != NULL) {
		frm_len = sizeof(HEADER_802_11);

		pEntry = MacTableLookup(pAd, pAddr);
#ifdef CONFIG_AP_SUPPORT
		if ((pEntry != NULL) && (IS_ENTRY_CLIENT(pEntry)))
		{
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr,
							pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.FrDs = 1;
			pNullFr->FC.ToDs = 0;

			goto body;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pNullFr->FC.FrDs = 0;
			pNullFr->FC.ToDs = 1;

		}
#endif /* CONFIG_STA_SUPPORT */

body:
		pNullFr->FC.Type = FC_TYPE_DATA;

		if (bQosNull)
		{
			u8 *qos_p = ((u8 *)pNullFr) + frm_len;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			frm_len += 2;
		} else
			pNullFr->FC.SubType = SUBTYPE_DATA_NULL;

		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);

		DBGPRINT(RT_DEBUG_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate], AID & 0x3f));
		MiniportMMRequest(pAd, WMM_UP2AC_MAP[7], (u8 *)pNullFr, frm_len);

		kfree(pFrame);
	}
}


VOID RtmpPrepareHwNullFrame(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN bool bQosNull,
	IN bool bEOSP,
	IN u8 OldUP,
	IN u8 OpMode,
	IN u8 PwrMgmt,
	IN bool bWaitACK,
	IN CHAR Index)
{
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	struct mt7612u_txwi *txwi = &pAd->NullTxWI;
	u8 *pNullFrame;
	HEADER_802_11 *pNullFr;
	ULONG Length;
	u8 *ptr;
	UINT i;
	uint32_t longValue;

	pNullFrame = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);

	memset(pNullFrame, 0, 48);
	memset(txwi, 0, TXWISize);

	pNullFr = (PHEADER_802_11) pNullFrame;
	Length = sizeof(HEADER_802_11);

	pNullFr->FC.Type = FC_TYPE_DATA;
	pNullFr->FC.SubType = SUBTYPE_DATA_NULL;
	if (Index == 1)
		pNullFr->FC.ToDs = 0;
	else
	pNullFr->FC.ToDs = 1;
	pNullFr->FC.FrDs = 0;

	COPY_MAC_ADDR(pNullFr->Addr1, pEntry->Addr);
	COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pNullFr->Addr3, pAd->CommonCfg.Bssid);

	pNullFr->FC.PwrMgmt = PwrMgmt;

	pNullFr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, pAd->CommonCfg.TxRate, 14);

	/* sequence is increased in MlmeHardTx */
	pNullFr->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; /* next sequence  */

	if (bQosNull) {
		u8 *qos_p = ((u8 *)pNullFr) + Length;

		pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

		/* copy QOS control bytes */
		qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
		qos_p[1] = 0;
		Length += 2;
	}

	if (Index == 1) {
		HTTRANSMIT_SETTING Transmit;

		Transmit.word = pEntry->MaxHTPhyMode.word;
		Transmit.field.BW = 0;
		if (Transmit.field.MCS > 7)
			Transmit.field.MCS = 7;

		RTMPWriteTxWI(pAd, txwi, false, false, false,
			      false, true, true, 0, pEntry->Aid,
			      Length, (u8)Transmit.field.MCS,
			      0, (u8)Transmit.field.MCS,
			      IFS_HTTXOP, &Transmit);
	} else {
		RTMPWriteTxWI(pAd, txwi, false, false, false,
			      false, true, 0, 0, pEntry->Aid,
			      Length, (u8)pAd->CommonCfg.MlmeTransmit.field.MCS,
			      0, (u8)pAd->CommonCfg.MlmeTransmit.field.MCS,
			      IFS_HTTXOP, &pAd->CommonCfg.MlmeTransmit);
	}

	if (bWaitACK) {

	}

	ptr = (u8 *)&pAd->NullTxWI;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */
	for (i=0; i < TXWISize; i+=4) 	{
		longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
		if (Index == 0)
			mt76u_reg_write(pAd, pAd->chipCap.BcnBase[14] + i, longValue);
		else if (Index == 1)
			mt76u_reg_write(pAd, pAd->chipCap.BcnBase[15] + i, longValue);

		ptr += 4;
	}

	ptr = pNullFrame;
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, false);
#endif /* RT_BIG_ENDIAN */
	for (i= 0; i< Length; i+=4) {
		longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
		if (Index == 0) //for ra0
			mt76u_reg_write(pAd, pAd->chipCap.BcnBase[14] + TXWISize+ i, longValue);
		else if (Index == 1) //for p2p0
			mt76u_reg_write(pAd, pAd->chipCap.BcnBase[15] + TXWISize+ i, longValue);

		ptr += 4;
	}

	if (pNullFrame)
		kfree(pNullFrame);

}


// TODO: shiang-usw, modify the op_mode assignment for this function!!!
VOID dev_rx_mgmt_frm(struct rtmp_adapter *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	struct sk_buff *pRxPacket = pRxBlk->pRxPacket;
	INT op_mode = pRxBlk->OpMode;
	bool 	bPassTheBcastPkt = false;
	INT			i;

#ifdef RT_CFG80211_SUPPORT
	if (CFG80211_HandleP2pMgmtFrame(pAd, pRxBlk, op_mode))
		goto done;
#endif /* RT_CFG80211_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		struct mt7612u_rxinfo *pRxInfo = pRxBlk->pRxInfo;

		op_mode = OPMODE_AP;

		if (!pRxInfo->U2M)
		{
			if ((pHeader->FC.SubType != SUBTYPE_BEACON) && (pHeader->FC.SubType != SUBTYPE_PROBE_REQ))
			{

				if (pHeader->FC.SubType == SUBTYPE_ACTION)
				{

					if (!bPassTheBcastPkt)
						goto done; /* Skip this packet */
				}
				else
					goto done; /* Skip this packet */
			}
		}

		if (pAd->ApCfg.BANClass3Data == true)
		{
			/* disallow new association */
			if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pHeader->FC.SubType == SUBTYPE_AUTH))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Disallow new Association\n"));
				goto done;
			}
		}

		/* Software decrypts WEP data during shared WEP negotiation */
		if ((pHeader->FC.SubType == SUBTYPE_AUTH) &&
			(pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0))
		{
			u8 *pMgmt = (u8 *)pHeader;
			uint16_t mgmt_len = pRxBlk->MPDUtotalByteCnt;
			MAC_TABLE_ENTRY *pEntry = NULL;

			/* Skip 802.11 headre */
			pMgmt += LENGTH_802_11;
			mgmt_len -= LENGTH_802_11;

			if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: SW decrypt WEP data fails - the Entry is empty.\n"));
				goto done;
			}

			/* handle WEP decryption */
			if (RTMPSoftDecryptWEP(pAd,
								   &pAd->SharedKey[pEntry->apidx][pRxBlk->key_idx],
								   pMgmt,
								   &mgmt_len) == false)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: SW decrypt WEP data fails.\n"));
				goto done;
			}
#ifdef RT_BIG_ENDIAN
			/* swap 16 bit fields - Auth Alg No. field */
			*(unsigned short *)pMgmt = SWAP16(*(unsigned short *)pMgmt);

			/* swap 16 bit fields - Auth Seq No. field */
			*(unsigned short *)(pMgmt + 2) = SWAP16(*(unsigned short *)(pMgmt + 2));

			/* swap 16 bit fields - Status Code field */
			*(unsigned short *)(pMgmt + 4) = SWAP16(*(unsigned short *)(pMgmt + 4));
#endif /* RT_BIG_ENDIAN */

			DBGPRINT(RT_DEBUG_TRACE, ("Decrypt AUTH seq#3 successfully\n"));

			/* Update the total length */
			pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);
		}
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		op_mode = OPMODE_STA;


		/* check if need to resend PS Poll when received packet with MoreData = 1 */
		if ((RtmpPktPmBitCheck(pAd) == true) && (pHeader->FC.MoreData == 1)) {
			/* for UAPSD, all management frames will be VO priority */
			if (pAd->CommonCfg.bAPSDAC_VO == 0) {
				/* non-UAPSD delivery-enabled AC */
				RTMP_PS_POLL_ENQUEUE(pAd);
			}
		}

		/* TODO: if MoreData == 0, station can go to sleep */

		/* We should collect RSSI not only U2M data but also my beacon */
		if ((pHeader->FC.SubType == SUBTYPE_BEACON)
		    && (MAC_ADDR_EQUAL(&pAd->CommonCfg.Bssid, &pHeader->Addr2))
		    && (pAd->RxAnt.EvaluatePeriod == 0)) {
			Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, pRxBlk->pRxWI);

			pAd->StaCfg.LastSNR0 = (u8) (pRxBlk->snr[0]);
			pAd->StaCfg.LastSNR1 = (u8) (pRxBlk->snr[1]);

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */
		}

		if ((pHeader->FC.SubType == SUBTYPE_BEACON) &&
		    (ADHOC_ON(pAd)) &&
		    (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)) {
			MAC_TABLE_ENTRY *pEntry = NULL;
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			if (pEntry)
				Update_Rssi_Sample(pAd, &pEntry->RssiSample, pRxBlk->pRxWI);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (pRxBlk->DataSize > MAX_RX_PKT_LEN) {
		DBGPRINT(RT_DEBUG_TRACE, ("DataSize=%d\n", pRxBlk->DataSize));
		hex_dump("MGMT ???", (u8 *)pHeader, pRxBlk->pData - (u8 *) pHeader);
		goto done;
	}

#if defined(CONFIG_AP_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
	if (pHeader->FC.SubType == SUBTYPE_ACTION)
	{
		MAC_TABLE_ENTRY *pMacEntry = NULL;

		/* only PM bit of ACTION frame can be set */
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			pMacEntry = &pAd->MacTab.Content[pRxBlk->wcid];

		if (pMacEntry && ((op_mode == OPMODE_AP && IS_ENTRY_CLIENT(pMacEntry))
		))
		   	RtmpPsIndicate(pAd, pHeader->Addr2, pRxBlk->wcid, pHeader->FC.PwrMgmt);

		/*
			In IEEE802.11, 11.2.1.1 STA Power Management modes,
			The Power Managment bit shall not be set in any management
			frame, except an Action frame.

			In IEEE802.11e, 11.2.1.4 Power management with APSD,
			If there is no unscheduled SP in progress, the unscheduled SP
			begins when the QAP receives a trigger frame from a non-AP QSTA,
			which is a QoS data or QoS Null frame associated with an AC the
			STA has configured to be trigger-enabled.
			So a management action frame is not trigger frame.
		*/
	}
#endif /* defined(CONFIG_AP_SUPPORT) defined(DOT11Z_TDLS_SUPPORT) */

	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
	REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid, pHeader,
					pRxBlk->DataSize,
					pRxBlk->rssi[0], pRxBlk->rssi[1], pRxBlk->rssi[2],
					min((CHAR)pRxBlk->snr[0], (CHAR)pRxBlk->snr[1]),
					op_mode);

	if (pAd->chipCap.FlgHwTxBfCap)
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
		if (pHeader->FC.Order)
		{
			handleHtcField(pAd, pRxBlk);
			pRxBlk->pData += 4;
			pRxBlk->DataSize -= 4;
		}

		/* Check for compressed or non-compressed Sounding Response */
		if (((pHeader->FC.SubType == SUBTYPE_ACTION) || (pHeader->FC.SubType == SUBTYPE_ACTION_NO_ACK))
			&& (pRxBlk->pData[0] == CATEGORY_HT)
			&& ((pRxBlk->pData[1] == MIMO_N_BEACONFORM) || (pRxBlk ->pData[1] == MIMO_BEACONFORM))
		)
		{
			handleBfFb(pAd, pRxBlk);
		}
	}

done:
	dev_kfree_skb_any(pRxPacket);
}


VOID dev_rx_ctrl_frm(struct rtmp_adapter *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	struct sk_buff *pRxPacket = pRxBlk->pRxPacket;

	switch (pHeader->FC.SubType)
	{
		case SUBTYPE_BLOCK_ACK_REQ:
			CntlEnqueueForRecv(pAd, pRxBlk->wcid, (pRxBlk->MPDUtotalByteCnt), (PFRAME_BA_REQ)pHeader);
			{
				FRAME_BA_REQ *bar = (FRAME_BA_REQ *)pHeader;
				if (bar->BARControl.Compressed == 0) {
					u8 tid = bar->BARControl.TID;
					BARecSessionTearDown(pAd, pRxBlk->wcid, tid, false);
				}
			}
			break;

#ifdef CONFIG_AP_SUPPORT
		case SUBTYPE_PS_POLL:
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				unsigned short Aid = pHeader->Duration & 0x3fff;
				u8 *pAddr = pHeader->Addr2;
				MAC_TABLE_ENTRY *pEntry;

				if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) {
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
					if (Aid == 0)
					{
						Aid = pEntry->Aid;
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Aid == 0 (pkt:%d, But Entry:%d)!\n",
                                                                        __FUNCTION__, Aid, pEntry->Aid));
					}

					if (pEntry->Aid == Aid)
						RtmpHandleRxPsPoll(pAd, pAddr, pRxBlk->wcid, false);
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Aid mismatch(pkt:%d, Entry:%d)!\n",
									__FUNCTION__, Aid, pEntry->Aid));
					}
				}
			}
			break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef WFA_VHT_PF
		case SUBTYPE_RTS:
			if (pAd->CommonCfg.vht_bw_signal && pRxBlk->wcid <= MAX_LEN_OF_MAC_TABLE)
			{
				PLCP_SERVICE_FIELD *srv_field;
				RTS_FRAME *rts = (RTS_FRAME *)pRxBlk->pHeader;

				if ((rts->Addr1[0] & 0x1) == 0x1) {
					srv_field = (PLCP_SERVICE_FIELD *)&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[15];
					if (srv_field->dyn_bw == 1) {
						DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x, WCID:%d, DYN,BW=%d\n",
									PRINT_MAC(rts->Addr1), pRxBlk->wcid, srv_field->cbw_in_non_ht));
					}
				}
			}
			break;

		case SUBTYPE_CTS:
			break;
#endif /* WFA_VHT_PF */

		case SUBTYPE_BLOCK_ACK:
		case SUBTYPE_ACK:
		default:
			break;
	}

	dev_kfree_skb_any(pRxPacket);
}


/*
		========================================================================
		Routine Description:
			Process RxDone interrupt, running in DPC level

		Arguments:
			pAd    Pointer to our adapter

		Return Value:
			None

		Note:
			This routine has to maintain Rx ring read pointer.
	========================================================================
*/


#undef MAX_RX_PROCESS_CNT
#define MAX_RX_PROCESS_CNT	(256)

bool rtmp_rx_done_handle(struct rtmp_adapter *pAd)
{
	uint32_t RxProcessed, RxPending;
	bool bReschedule = false;
	RXD_STRUC *pRxD;
	struct mt7612u_rxinfo *pRxInfo;
	u8 *pData;
	struct mt7612u_rxwi *pRxWI;
	struct sk_buff *pRxPacket;
	HEADER_802_11 *pHeader;
	RX_BLK rxblk, *pRxBlk;
	bool bCmdRspPacket = false;

#ifdef LINUX
#endif /* LINUX */

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1)
	{
		if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
				(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
			)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
		{
			break;
		}


		/*
			1. allocate a new data packet into rx ring to replace received packet
				then processing the received packet
			2. the callee must take charge of release of packet
			3. As far as driver is concerned, the rx packet must
				a. be indicated to upper layer or
				b. be released if it is discarded
		*/

		pRxBlk = &rxblk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, 0);
		bCmdRspPacket = false;
		if ((pRxPacket == NULL) && !bCmdRspPacket)
			break;

		if ((pRxPacket == NULL) && bCmdRspPacket)
			continue;

		/* get rx descriptor and data buffer */
		pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];
		pRxInfo = rxblk.pRxInfo;
		pData = pRxPacket->data;
		pRxWI = (struct mt7612u_rxwi *)pData;
		pHeader = rxblk.pHeader;// (PHEADER_802_11)(pData + RXWISize);



#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (u8 *)pHeader, DIR_READ, true);
		RTMPWIEndianChange(pAd , (u8 *)pRxWI, TYPE_RXWI);
#endif

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RXWI)
			dbQueueEnqueueRxFrame(pData, (u8 *)pHeader, pAd->CommonCfg.DebugFlags);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		/* build RX_BLK */
		rxblk.pRxWI = pRxWI;
		rxblk.pHeader = pHeader;
		rxblk.pRxPacket = pRxPacket;
		rxblk.pData = (u8 *)pHeader;
		rxblk.DataSize = pRxBlk->MPDUtotalByteCnt;
		rxblk.Flags = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			SET_PKT_OPMODE_AP(&rxblk);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			SET_PKT_OPMODE_STA(&rxblk);
		}
#endif /* CONFIG_STA_SUPPORT */



		/* Increase Total receive byte counter after real data received no mater any error or not */
		pAd->RalinkCounters.ReceivedByteCount += rxblk.DataSize;
		pAd->RalinkCounters.OneSecReceivedByteCount += rxblk.DataSize;
		pAd->RalinkCounters.RxCount++;
		pAd->RalinkCounters.OneSecRxCount++;

#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);
#endif /* STATS_COUNT_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Check for all RxD errors */
			if (APCheckRxError(pAd, pRxInfo, pRxBlk) != NDIS_STATUS_SUCCESS)
			{
				APRxErrorHandle(pAd, &rxblk);

				/* Increase received error packet counter per BSS */
				if (pHeader->FC.FrDs == 0 &&
					pRxInfo->U2M &&
					pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
				{
					MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pRxBlk->bss_idx];
					pMbss->RxDropCount ++;
					pMbss->RxErrorCount ++;
				}


#ifdef DBG_DIAGNOSE
				if (!pRxInfo->Crc)
#endif
					DBGPRINT(RT_DEBUG_TRACE, ("%s(): CheckRxError!\n", __FUNCTION__));

				dev_kfree_skb_any(pRxPacket);

				continue;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Check for all RxD errors */
			if (RTMPCheckRxError(pAd, pHeader, &rxblk, pRxInfo) != NDIS_STATUS_SUCCESS)
			{
				pAd->Counters8023.RxErrors++;
				/* discard this frame */
				dev_kfree_skb_any(pRxPacket);
				continue;
			}

			if (pAd->chipCap.FreqCalibrationSupport)
			{
				if ((pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == true)
				    && (INFRA_ON(pAd)) && (pRxInfo->Crc == 0)
			    	&& (pHeader->FC.Type == FC_TYPE_MGMT)
			    	&& (pHeader->FC.SubType == SUBTYPE_BEACON)
			    	&& (MAC_ADDR_EQUAL(&pAd->CommonCfg.Bssid, &pHeader->Addr2)))
				{
					pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon = GetFrequencyOffset(pAd, pRxWI);
					pAd->FreqCalibrationCtrl.BeaconPhyMode = (u8)(pRxBlk->rx_rate.field.MODE);

					DBGPRINT(RT_DEBUG_INFO,
						 ("%s: Beacon, CRC=%d, pHdr->Seq=%d, SA=%02x:%02x:%02x:%02x:%02x:%02x, freqOffset=%d, MCS=%d, BW=%d, PHYMODE=%d\n",
						  __FUNCTION__, pRxInfo->Crc,
						  pHeader->Sequence, PRINT_MAC(pHeader->Addr2),
					  	((CHAR) (pRxBlk->freq_offset)),
					  	rxblk.rx_rate.field.MCS,
					  	rxblk.rx_rate.field.BW,
					  	rxblk.rx_rate.field.MODE));
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */


		// TODO: shiang-usw, for P2P, we original has following code, need to check it and merge to correct place!!!

		switch (pHeader->FC.Type)
		{
			case FC_TYPE_DATA:
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
				}
#endif /* CONFIG_STA_SUPPORT */

					{

#ifdef CONFIG_STA_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
						{
							STAHandleRxDataFrame(pAd, &rxblk);
							break;
						}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
						{
							APHandleRxDataFrame(pAd, &rxblk);
							break;
						}
#endif /* CONFIG_AP_SUPPORT */
					}
					break;

			case FC_TYPE_MGMT:
					dev_rx_mgmt_frm(pAd, &rxblk);
					break;

			case FC_TYPE_CNTL:
					dev_rx_ctrl_frm(pAd, &rxblk);
					break;

			default:
				dev_kfree_skb_any(pRxPacket);
				break;
		}
	}



	return bReschedule;
}

#ifdef DROP_MASK_SUPPORT
VOID drop_mask_init_per_client(
	struct rtmp_adapter *ad,
	PMAC_TABLE_ENTRY entry)
{
	bool cancelled = 0;

	if (entry->dropmask_timer.Valid)
			RTMPCancelTimer(&entry->dropmask_timer, &cancelled);
	RTMPInitTimer(ad, &entry->dropmask_timer, GET_TIMER_FUNCTION(drop_mask_timer_action), entry, false);

	spin_lock_init(ad, &entry->drop_mask_lock);

	entry->tx_fail_drop_mask_enabled = 0;
	entry->ps_drop_mask_enabled = 0;
	asic_set_drop_mask(ad, entry->Aid, 0);

	if ((ad->ApCfg.EntryClientCount >= 2) && IS_ENTRY_CLIENT(entry))
		entry->pMbss->WPAREKEY.ReKeyMethod |= MAX_REKEY;
}

VOID drop_mask_release_per_client(
	struct rtmp_adapter *ad,
	PMAC_TABLE_ENTRY entry)
{
	bool cancelled = 0;

	RTMPCancelTimer(&entry->dropmask_timer, &cancelled);
	RTMPReleaseTimer(&entry->dropmask_timer, &cancelled);

	entry->tx_fail_drop_mask_enabled = 0;
	entry->ps_drop_mask_enabled = 0;
	asic_set_drop_mask(ad, entry->Aid, 0);

	if (ad->ApCfg.EntryClientCount == 3)
	{
		/* clear drop mask before client number fall below to threshold */
		drop_mask_per_client_reset(ad);
	}
}

VOID drop_mask_per_client_reset(
	struct rtmp_adapter *ad)
{
	INT i;
	uint32_t max_wcid_num = MAX_LEN_OF_MAC_TABLE;

	for ( i = 0; i < max_wcid_num; i++)
	{
		PMAC_TABLE_ENTRY entry = &ad->MacTab.Content[i];
		if (entry && (!IS_ENTRY_NONE(entry)))
		{
			spin_lock_bh(&entry->drop_mask_lock);
			entry->tx_fail_drop_mask_enabled = 0;
			entry->ps_drop_mask_enabled = 0;
			spin_unlock_bh(&entry->drop_mask_lock);

			if (IS_ENTRY_CLIENT(entry))
				entry->pMbss->WPAREKEY.ReKeyMethod &= (~MAX_REKEY);
		}
	}

	asic_drop_mask_reset(ad);

}

VOID set_drop_mask_per_client(
	struct rtmp_adapter *	ad,
	PMAC_TABLE_ENTRY 	entry,
	UINT8				type,
	bool				enable)
{
	bool cancelled = 0;
	bool write_to_mac = 0;
	uint32_t timeout = 0;

	if (ad->ApCfg.EntryClientCount < 3)
		return;


	switch (type)
	{
		case 1: /* set drop mask due to tx_fail too high */
		{
			write_to_mac = (enable ^ entry->tx_fail_drop_mask_enabled);
			spin_lock_bh(&entry->drop_mask_lock);
			entry->tx_fail_drop_mask_enabled = (enable ? 1:0);
			spin_unlock_bh(&entry->drop_mask_lock);
			timeout = 10;
			break;
		}
		case 2: /* set drop mask due to client is in power saving */
		{
			write_to_mac = (enable ^ entry->ps_drop_mask_enabled);
			spin_lock_bh(&entry->drop_mask_lock);
			entry->ps_drop_mask_enabled = (enable ? 1:0);
			spin_unlock_bh(&entry->drop_mask_lock);
			timeout = 1000;
			break;
		}
		default:
			break;
	}

	RTMPCancelTimer(&entry->dropmask_timer, &cancelled);

	if (enable)
	{
		RTMPSetTimer(&entry->dropmask_timer, timeout /* ms */);
	}

	/* if we don't need to change mac reg, just return */
	if (!write_to_mac)
		return;

	if (enable) {
		asic_set_drop_mask(ad, entry->Aid, enable);
	} else {
		if (!entry->tx_fail_drop_mask_enabled &&
			!entry->ps_drop_mask_enabled)
		{
			asic_set_drop_mask(ad, entry->Aid, enable);
		}
	}
}

VOID  drop_mask_timer_action(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PMAC_TABLE_ENTRY     entry = (MAC_TABLE_ENTRY *)FunctionContext;
	struct rtmp_adapter *ad = (struct rtmp_adapter *)entry->pAd;

	/* Disable drop mask */
	if (entry->tx_fail_drop_mask_enabled)
		set_drop_mask_per_client(ad, entry, 1, 0);

	if (entry->ps_drop_mask_enabled)
		set_drop_mask_per_client(ad, entry, 2, 0);

}
#endif /* DROP_MASK_SUPPORT */

