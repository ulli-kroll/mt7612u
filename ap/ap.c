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
    soft_ap.c

    Abstract:
    Access Point specific routines and MAC table maintenance routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP

 */

#include "rt_config.h"

bool ApCheckLongPreambleSTA(
    IN struct rtmp_adapter *pAd);

char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated" ,
	"active countermeasures",
	"has disassociated with invalid PSK password"};


u8 get_apidx_by_addr(struct rtmp_adapter *pAd, u8 *addr)
{
	u8 apidx;

	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}


/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */
int APInitialize(struct rtmp_adapter *pAd)
{
	int     Status = NDIS_STATUS_SUCCESS;
	INT				i;

	DBGPRINT(RT_DEBUG_TRACE, ("---> APInitialize\n"));

	/* Init Group key update timer, and countermeasures timer */
	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++)
		RTMPInitTimer(pAd, &pAd->ApCfg.MBSSID[i].REKEYTimer, GET_TIMER_FUNCTION(GREKEYPeriodicExec), pAd,  true);

	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, false);
	RTMPInitTimer(pAd, &pAd->CommonCfg.BeaconUpdateTimer, GET_TIMER_FUNCTION(BeaconUpdateExec), pAd, true);






	DBGPRINT(RT_DEBUG_TRACE, ("<--- APInitialize\n"));
	return Status;
}

/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(struct rtmp_adapter *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("---> APShutdown\n"));
/*	if (pAd->OpMode == OPMODE_AP) */

	MlmeRadioOff(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("<--- APShutdown\n"));
}

/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */
VOID APStartUp(struct rtmp_adapter *pAd)
{
	uint32_t i;
	bool bWmmCapable = false;
	u8 idx;
	bool TxPreamble, SpectrumMgmt = false;
	u8 phy_mode = pAd->CommonCfg.cfg_wmode;
#ifdef DOT1X_SUPPORT
	/* bool bDot1xReload = false; */
#endif /* DOT1X_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("===> APStartUp\n"));

	AsicDisableSync(pAd);

	TxPreamble = (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1);

	/* Decide the Capability information field */
	/* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
	if ((pAd->CommonCfg.Channel > 14) && pAd->CommonCfg.bIEEE80211H == true)
		SpectrumMgmt = true;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];
		struct rtmp_wifi_dev *wdev = &pMbss->wdev;

		if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID))
		{
			memmove(pMbss->Ssid, "HT_AP", 5);
			pMbss->Ssid[5] = '0' + idx;
			pMbss->SsidLen = 6;
		}

		/* re-copy the MAC to virtual interface to avoid these MAC = all zero,
		   when re-open the ra0,
		   i.e. ifconfig ra0 down, ifconfig ra0 up, ifconfig ra0 down, ifconfig up ... */
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
		if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
		{
			u8 MacMask = 0;

			if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
				MacMask = 0xFE;
			else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
				MacMask = 0xFC;
			else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
				MacMask = 0xF8;
			else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 16)
				MacMask = 0xF0;

			if (idx > 0)
			{
				wdev->if_addr[0] |= 0x2;
				if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
				{
					/*
						Refer to HW definition -
						Bit1 of MAC address Byte0 is local administration bit
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					 */
					wdev->if_addr[0] += ((idx - 1) << 2);
				}
			}
		}
		else
			wdev->if_addr[5] += idx;

		if (wdev->if_dev != NULL)
		{
			memmove(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
								wdev->if_addr, MAC_ADDR_LEN);
		}
		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);
		wdev->wdev_type = WDEV_TYPE_AP;
		wdev->tx_pkt_allowed = ApAllowToSendPacket;

		if (pMbss->wdev.bWmmCapable)
	        	bWmmCapable = true;

		pMbss->CapabilityInfo = CAP_GENERATE(1, 0, (wdev->WepStatus != Ndis802_11EncryptionDisabled),
											TxPreamble, pAd->CommonCfg.bUseShortSlotTime,
											SpectrumMgmt);


		if (bWmmCapable == true)
		{
			/*
				In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
				bit in the capability field of association, beacon and probe
				management frames.
			*/
/*			pMbss->CapabilityInfo |= 0x0200; */
		}


		/* decide the mixed WPA cipher combination */
		if (wdev->WepStatus == Ndis802_11TKIPAESMix)
		{
			switch ((u8)wdev->AuthMode)
			{
				/* WPA mode */
				case Ndis802_11AuthModeWPA:
				case Ndis802_11AuthModeWPAPSK:
					wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
					break;

				/* WPA2 mode */
				case Ndis802_11AuthModeWPA2:
				case Ndis802_11AuthModeWPA2PSK:
					wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
					break;

				/* WPA and WPA2 both mode */
				case Ndis802_11AuthModeWPA1WPA2:
				case Ndis802_11AuthModeWPA1PSKWPA2PSK:

					/* In WPA-WPA2 and TKIP-AES mixed mode, it shall use the maximum */
					/* cipher capability unless users assign the desired setting. */
					if (wdev->WpaMixPairCipher == MIX_CIPHER_NOTUSE ||
						wdev->WpaMixPairCipher == WPA_TKIPAES_WPA2_NONE ||
						wdev->WpaMixPairCipher == WPA_NONE_WPA2_TKIPAES)
						wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;
					break;
			}
		}
		else
			wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;

		if (wdev->WepStatus == Ndis802_11Encryption2Enabled ||
			wdev->WepStatus == Ndis802_11Encryption3Enabled ||
			wdev->WepStatus == Ndis802_11Encryption4Enabled)
		{
			RT_CfgSetWPAPSKKey(pAd, pMbss->WPAKeyString,
								strlen(pMbss->WPAKeyString),
								(u8 *)pAd->ApCfg.MBSSID[idx].Ssid,
								pAd->ApCfg.MBSSID[idx].SsidLen,
								pAd->ApCfg.MBSSID[idx].PMK);
		}

		/* Generate the corresponding RSNIE */
		RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, idx);

	}

	if (phy_mode != pAd->CommonCfg.PhyMode)
		RTMPSetPhyMode(pAd, phy_mode);

	SetCommonHT(pAd);

	COPY_MAC_ADDR(pAd->CommonCfg.Bssid, pAd->CurrentAddress);

	/* Select DAC according to HT or Legacy, write to BBP R1(bit4:3) */
	/* In HT mode and two stream mode, both DACs are selected. */
	/* In legacy mode or one stream mode, DAC-0 is selected. */
	{
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && (pAd->Antenna.field.TxPath == 2))
			mt7612u_bbp_set_txdac(pAd, 2);
		else
			mt7612u_bbp_set_txdac(pAd, 0);
	}

	/* Receiver Antenna selection */
	mt7612u_bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) || bWmmCapable)
	{
		/* EDCA parameters used for AP's own transmission */
		if (pAd->CommonCfg.APEdcaParm.bValid == false)
			set_default_ap_edca_param(pAd);

		/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
		if (pAd->ApCfg.BssEdcaParm.bValid == false)
			set_default_sta_edca_param(pAd);

		AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
	}
	else
		AsicSetEdcaParm(pAd, NULL);

	if (!WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20; /* Patch UI */

	AsicSetRDG(pAd, pAd->CommonCfg.bRdg);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);

	AsicSetBssid(pAd, pAd->CurrentAddress);
	mgmt_tb_set_mcast_entry(pAd);


	/*@!Release
		Reset whole WCID table

		In AP mode,  First WCID Table in ASIC will never be used.
		To prevent it's 0xff-ff-ff-ff-ff-ff, Write 0 here.

		p.s ASIC use all 0xff as termination of WCID table search.
	*/
	DBGPRINT(RT_DEBUG_TRACE, ("%s():Reset WCID Table\n", __FUNCTION__));
		AsicDelWcidTab(pAd, WCID_ALL);

#ifdef FIFO_EXT_SUPPORT
	AsicFifoExtSet(pAd);
#endif /* FIFO_EXT_SUPPORT */

	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */

	pAd->MacTab.Content[0].Addr[0] = 0x01;
	pAd->MacTab.Content[0].HTPhyMode.field.MODE = MODE_OFDM;
	pAd->MacTab.Content[0].HTPhyMode.field.MCS = 3;
	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;

	AsicBBPAdjust(pAd);

	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
	else
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;
	AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, false);
	AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

//+++Add by shiang for debug
DBGPRINT(RT_DEBUG_OFF, ("%s(): AP Set CentralFreq at %d(Prim=%d, HT-CentCh=%d, VHT-CentCh=%d, BBP_BW=%d)\n",
						__FUNCTION__, pAd->hw_cfg.cent_ch, pAd->CommonCfg.Channel,
						pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch,
						pAd->CommonCfg.BBPCurrentBW));
//---Add by shiang for debug

	MlmeSetTxPreamble(pAd, (unsigned short)pAd->CommonCfg.TxPreamble);
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		MlmeUpdateTxRates(pAd, false, idx);
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			MlmeUpdateHtTxRates(pAd, idx);
	}

	/* Set the RadarDetect Mode as Normal, bc the APUpdateAllBeaconFram() will refer this parameter. */
	RadarStateCheck(pAd);

	/* Disable Protection first. */
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), true, false);

	APUpdateCapabilityAndErpIe(pAd);
	APUpdateOperationMode(pAd);



	/* Initialize security variable per entry,
		1. 	pairwise key table, re-set all WCID entry as NO-security mode.
		2.	access control port status
	*/
	DBGPRINT(RT_DEBUG_TRACE, ("%s():Reset Sec Table\n", __FUNCTION__));
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pAd->MacTab.Content[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		AsicRemovePairwiseKeyEntry(pAd, (u8)i);
	}

	/* Init Security variables */
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		unsigned short Wcid = 0;
		MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];
		struct rtmp_wifi_dev *wdev = &pAd->ApCfg.MBSSID[idx].wdev;

		wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		if (IS_WPA_CAPABILITY(wdev->AuthMode))
			wdev->DefaultKeyId = 1;

		/* Get a specific WCID to record this MBSS key attribute */
		GET_GroupKey_WCID(pAd, Wcid, idx);

		/* When WEP, TKIP or AES is enabled, set group key info to Asic */
		if (wdev->WepStatus == Ndis802_11WEPEnabled)
		{
			u8 CipherAlg, key_idx;

			for (key_idx=0; key_idx < SHARE_KEY_NUM; key_idx++)
			{
				CipherAlg = pAd->SharedKey[idx][key_idx].CipherAlg;

				if (pAd->SharedKey[idx][key_idx].KeyLen > 0)
				{
					/* Set key material to Asic */
					AsicAddSharedKeyEntry(pAd, idx, key_idx, &pAd->SharedKey[idx][key_idx]);

					if (key_idx == wdev->DefaultKeyId)
					{
						/* Generate 3-bytes IV randomly for software encryption using */
					    	for(i = 0; i < LEN_WEP_TSC; i++)
							pAd->SharedKey[idx][key_idx].TxTsc[i] = RandomByte(pAd);

						/* Update WCID attribute table and IVEIV table */
						RTMPSetWcidSecurityInfo(pAd,
												idx,
												key_idx,
												CipherAlg,
												Wcid,
												SHAREDKEYTABLE);
					}
				}
			}
    	}
		else if ((wdev->WepStatus == Ndis802_11TKIPEnable) ||
				 (wdev->WepStatus == Ndis802_11AESEnable) ||
				 (wdev->WepStatus == Ndis802_11TKIPAESMix))
		{
			/* Generate GMK and GNonce randomly per MBSS */
			GenRandom(pAd, wdev->bssid, pMbss->GMK);
			GenRandom(pAd, wdev->bssid, pMbss->GNonce);

			/* Derive GTK per BSSID */
			WpaDeriveGTK(pMbss->GMK,
						(u8 *)pMbss->GNonce,
						wdev->bssid,
						pMbss->GTK,
						LEN_TKIP_GTK);

			/* Install Shared key */
			WPAInstallSharedKey(pAd,
								wdev->GroupKeyWepStatus,
								idx,
								wdev->DefaultKeyId,
								Wcid,
								true,
								pMbss->GTK,
								LEN_TKIP_GTK);

		}

#ifdef DOT1X_SUPPORT
		/* Send singal to daemon to indicate driver had restarted */
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPA2)
        		|| (wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2) || (wdev->IEEE8021X == true))
		{
			;/*bDot1xReload = true; */
    	}
#endif /* DOT1X_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE, ("### BSS(%d) AuthMode(%d)=%s, WepStatus(%d)=%s, AccessControlList.Policy=%ld\n",
					idx, wdev->AuthMode, GetAuthMode(wdev->AuthMode),
					wdev->WepStatus, GetEncryptType(wdev->WepStatus),
					pMbss->AccessControlList.Policy));
	}


#ifdef PIGGYBACK_SUPPORT
	RTMPSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);


	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);



	if (IS_MT76x2(pAd))
		mt76x2_calibration(pAd, pAd->hw_cfg.cent_ch);

	RTUSBBssBeaconInit(pAd);
	/* start sending BEACON out */
	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);

	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
		AsicEnableBssSync(pAd);

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbttInt(pAd, true);


	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	WPA_APSetGroupRekeyAction(pAd);



	/* */
	/* Support multiple BulkIn IRP, */
	/* the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1. */
	/* */
	for(i=0; i<pAd->CommonCfg.NumOfBulkInIRP; i++)
	{
		RTUSBBulkReceive(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
	}






	DBGPRINT(RT_DEBUG_TRACE, ("<=== APStartUp\n"));
}

/*
	==========================================================================
	Description:
		disassociate all STAs and stop AP service.
	Note:
	==========================================================================
 */
VOID APStop(
	IN struct rtmp_adapter *pAd)
{
	bool Cancelled;
	uint32_t Value;
	INT idx;
	MULTISSID_STRUCT *pMbss;

	DBGPRINT(RT_DEBUG_TRACE, ("!!! APStop !!!\n"));


#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */




	MacTableReset(pAd);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	/* Disable pre-tbtt interrupt */
	Value = mt76u_reg_read(pAd, INT_TIMER_EN);
	Value &=0xe;
	mt76u_reg_write(pAd, INT_TIMER_EN, Value);
	/* Disable piggyback */
	RTMPSetPiggyBack(pAd, false);

   	AsicUpdateProtect(pAd, 0,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), true, false);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/*RTMP_ASIC_INTERRUPT_DISABLE(pAd); */
		AsicDisableSync(pAd);

	}

	/* For RT2870, we need to clear the beacon sync buffer. */
	RTUSBBssBeaconExit(pAd);


	for (idx = 0; idx < MAX_MBSSID_NUM(pAd); idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if (pMbss->REKEYTimerRunning == true)
		{
			RTMPCancelTimer(&pMbss->REKEYTimer, &Cancelled);
			pMbss->REKEYTimerRunning = false;
		}
	}

	if (pAd->ApCfg.CMTimerRunning == true)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = false;
	}


	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);




}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(
	IN  struct rtmp_adapter *  pAd,
	IN  PQUEUE_HEADER   pQueue)
{
	PQUEUE_ENTRY pEntry;
	struct sk_buff *pPacket;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): (0x%08lx)...\n", __FUNCTION__, (ULONG)pQueue));

	while (pQueue->Head)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():%d...\n", __FUNCTION__, pQueue->Number));

		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		dev_kfree_skb_any(pPacket);
	}
}

/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/
VOID MacTableMaintenance(struct rtmp_adapter *pAd)
{
	int i, startWcid;
	ULONG MinimumAMPDUSize = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor; /*Default set minimum AMPDU Size to 2, i.e. 32K */
	bool	bRdgActive;
	bool bRalinkBurstMode;
	UINT fAnyStationPortSecured[HW_BEACON_MAX_NUM];
 	UINT bss_index;
	struct mt7612u_mac_table *pMacTable;
#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
	int lastClient=0;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */
	CHAR avgRssi;
	MULTISSID_STRUCT *pMbss;
#ifdef WFA_VHT_PF
	RSSI_SAMPLE *worst_rssi = NULL;
	int worst_rssi_sta_idx = 0;
#endif /* WFA_VHT_PF */


	CHAR rssiIndex = 0, overRssiThresCount = 0;

	memset(fAnyStationPortSecured, 0, sizeof(fAnyStationPortSecured));

	pMacTable = &pAd->MacTab;
	pMacTable->fAnyStationInPsm = false;
	pMacTable->fAnyStationBadAtheros = false;
	pMacTable->fAnyTxOPForceDisable = false;
	pMacTable->fAllStationAsRalink = true;
	pMacTable->fAnyStationNonGF = false;
	pMacTable->fAnyStation20Only = false;
	pMacTable->fAnyStationIsLegacy = false;
	pMacTable->fAnyStationMIMOPSDynamic = false;

	pMacTable->fAnyStaFortyIntolerant = false;
	pMacTable->fAllStationGainGoodMCS = true;


	startWcid = 1;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Skip the Infra Side */
	startWcid = 2;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	for (i = startWcid; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[i];
		bool bDisconnectSta = false;

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->NoDataIdleCount == 0)
			pEntry->StationKeepAliveCount = 0;

		pEntry->NoDataIdleCount ++;
		pEntry->StaConnectTime ++;

		pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %d sec\n",
					PRINT_MAC(pEntry->Addr), MAC_TABLE_ASSOC_TIMEOUT));
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			continue;
		}

		/* 1. check if there's any associated STA in power-save mode. this affects outgoing */
		/*    MCAST/BCAST frames should be stored in PSQ till DtimCount=0 */
		if (pEntry->PsMode == PWR_SAVE)
			pMacTable->fAnyStationInPsm = true;

		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = true;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = true;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD)
			pMacTable->fAnyStationNonGF = true;

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
			pMacTable->fAnyStationIsLegacy = true;

		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = true;

		/* Get minimum AMPDU size from STA */
		if (MinimumAMPDUSize > pEntry->MaxRAmpduFactor)
			MinimumAMPDUSize = pEntry->MaxRAmpduFactor;

		if (pEntry->bIAmBadAtheros)
		{
			pMacTable->fAnyStationBadAtheros = true;
			if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == false)
				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, false, pMacTable->fAnyStationNonGF);
		}

		/* detect the station alive status */
		/* detect the station alive status */
		if ((pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime))
		{
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

		        ......
		        sam> tx ok!
		        sam> count = 1!	 ==> 1 second after the Null Frame is acked
		        sam> count = 2!	 ==> 2 second after the Null Frame is acked
		        sam> count = 3!
		        sam> count = 4!
		        sam> count = 5!
		        sam> count = 6!
		        sam> count = 7!
		        sam> count = 8!
		        sam> count = 9!
		        sam> count = 10!
		        sam> count = 11!
		        sam> count = 12!
		        sam> count = 13!
		        sam> count = 14!
		        sam> count = 15! ==> 15 second after the Null Frame is acked
		        sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
		        sam> count = 1!  ==> 1 second after the Null Frame is acked
		        sam> count = 2!
		        sam> count = 3!
		        sam> count = 4!
		        ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/

			if (pEntry->StationKeepAliveCount++ == 0)
			{
					if (pEntry->PsMode == PWR_SAVE)
					{
						/* use TIM bit to detect the PS station */
						WLAN_MR_TIM_BIT_SET(pAd, pEntry->apidx, pEntry->Aid);
					}
					else
					{
						/* use Null or QoS Null to detect the ACTIVE station */
						bool bQosNull = false;

						if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
							bQosNull = true;

						RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
	    	                           						pEntry->Aid, pEntry->apidx, bQosNull, true, 0);
					}
			}
			else
			{
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime)
					pEntry->StationKeepAliveCount = 0;
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */
		if (pEntry->NoDataIdleCount >= pEntry->StaIdleTimeout)
		{
			bDisconnectSta = true;
			DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
					PRINT_MAC(pEntry->Addr), pEntry->StaIdleTimeout));
			ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
		}
		else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
		{
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE)
			{
				bDisconnectSta = true;
				DBGPRINT(RT_DEBUG_WARN, ("STA-%02x:%02x:%02x:%02x:%02x:%02x had left (%d %lu)\n",
					PRINT_MAC(pEntry->Addr),
					pEntry->ContinueTxFailCnt, pAd->ApCfg.EntryLifeCheck));
			}
		}

		//YF: kickout sta when 3 of 5 exceeds the threshold.

		if (pMbss->RssiLowForStaKickOut != 0)
		{
#define CHECK_DATA_RSSI_UP_BOUND 3
			for (rssiIndex=0; rssiIndex<MAX_LAST_DATA_RSSI_LEN; rssiIndex++)
			{
				if ((pEntry->LastDataRssi[rssiIndex] !=0 ) &&
				    (pEntry->LastDataRssi[rssiIndex] < pMbss->RssiLowForStaKickOut))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%d:[%d] Fail.\n",rssiIndex,pEntry->LastDataRssi[rssiIndex]));
					overRssiThresCount++;
				}
			}

			if (overRssiThresCount >= CHECK_DATA_RSSI_UP_BOUND)
			{
				bDisconnectSta = true;
				DBGPRINT(RT_DEBUG_WARN, ("Dis STA %02x:%02x:%02x:%02x:%02x:%02x , RSSI Kickout Thres[%d]-[%d]Times\n",
							PRINT_MAC(pEntry->Addr), pMbss->RssiLowForStaKickOut, overRssiThresCount));
			}

		}

		if (bDisconnectSta)
		{
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);

			if (pEntry->Sst == SST_ASSOC)
			{
				u8 *pOutBuffer = NULL;
				int NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				unsigned short Reason;

				/*  send out a DISASSOC request frame */
				pOutBuffer  = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);
				if (pOutBuffer == NULL) {
					DBGPRINT(RT_DEBUG_TRACE, (" kmalloc fail  ..\n"));
					/*spin_unlock_bh(&pAd->MacTabLock); */
					continue;
				}
				Reason = REASON_DEAUTH_STA_LEAVING;
				DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame  TO %x %x %x %x %x %x \n",
										Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								pMbss->wdev.if_addr,
								pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								sizeof(HEADER_802_11), &DeAuthHdr,
								2, &Reason,
								END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				kfree(pOutBuffer);

			}

			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			continue;
		}

		/* 3. garbage collect the PsQueue if the STA has being idle for a while */
		if (pEntry->PsQueue.Head)
		{
			pEntry->PsQIdleCount++;
			if (pEntry->PsQIdleCount > 3)
			{
				ULONG irq_flags;
				spin_lock_bh(&pAd->irq_lock);
				APCleanupPsQueue(pAd, &pEntry->PsQueue);
				spin_unlock_bh(&pAd->irq_lock);
				pEntry->PsQIdleCount = 0;
				WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->apidx, pEntry->Aid);
			}
		}
		else
			pEntry->PsQIdleCount = 0;

		/* check if this STA is Ralink-chipset */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
			pMacTable->fAllStationAsRalink = false;

		/* Check if the port is secured */
		if (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
			fAnyStationPortSecured[pEntry->apidx]++;
		if ((pEntry->BSS2040CoexistenceMgmtSupport)
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == true)
		)
		{
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->apidx);
		}

#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
		lastClient = i;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) &&
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
		{
			pMacTable->fAllStationGainGoodMCS = false;
		}

#ifdef WFA_VHT_PF
		if (worst_rssi == NULL) {
			worst_rssi = &pEntry->RssiSample;
			worst_rssi_sta_idx = i;
		} else {
			if (worst_rssi->AvgRssi0 > pEntry->RssiSample.AvgRssi0) {
				worst_rssi = &pEntry->RssiSample;
				worst_rssi_sta_idx = i;
			}
		}
#endif /* WFA_VHT_PF */
	}

#ifdef WFA_VHT_PF
	if (worst_rssi != NULL &&
		((pAd->Mlme.OneSecPeriodicRound % 10) == 5) &&
		(worst_rssi_sta_idx >= 1))
	{
		CHAR gain = 2;
		if (worst_rssi->AvgRssi0 >= -40)
			gain = 1;
		else if (worst_rssi->AvgRssi0 <= -50)
			gain = 2;
		rt85592_lna_gain_adjust(pAd, gain);
		DBGPRINT(RT_DEBUG_TRACE, ("%s():WorstRSSI for STA(%02x:%02x:%02x:%02x:%02x:%02x):%d,%d,%d, Set Gain as %s\n",
					__FUNCTION__,
					PRINT_MAC(pMacTable->Content[worst_rssi_sta_idx].Addr),
					worst_rssi->AvgRssi0, worst_rssi->AvgRssi1, worst_rssi->AvgRssi2,
					(gain == 2 ? "Mid" : "Low")));
	}
#endif /* WFA_VHT_PF */

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
#endif /* CFO_TRACK */

	/* Update the state of port per MBSS */
	for (bss_index = BSS0; bss_index < MAX_MBSSID_NUM(pAd); bss_index++)
	{
		if (fAnyStationPortSecured[bss_index] > 0)
			pAd->ApCfg.MBSSID[bss_index].wdev.PortSecured = WPA_802_1X_PORT_SECURED;
		else
			pAd->ApCfg.MBSSID[bss_index].wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	}


	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg && pMacTable->fAllStationAsRalink)
		bRdgActive = true;
	else
		bRdgActive = false;

	if (pAd->CommonCfg.bRalinkBurstMode && pMacTable->fAllStationGainGoodMCS)
		bRalinkBurstMode = true;
	else
		bRalinkBurstMode = false;

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
		AsicSetRDG(pAd, bRdgActive);

	if (bRalinkBurstMode != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
		AsicSetRalinkBurstMode(pAd, bRalinkBurstMode);


	if ((pMacTable->fAnyStationBadAtheros == false) && (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == true))
	{
		AsicUpdateProtect(pAd, pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, false, pMacTable->fAnyStationNonGF);
	}

	/*
		4.
		garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
		stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	if (pMacTable->McastPsQueue.Head)
	{
		UINT bss_index;

		pMacTable->PsQIdleCount ++;
		if (pMacTable->PsQIdleCount > 1)
		{

			/*spin_lock_bh(&pAd->MacTabLock); */
			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			/*spin_unlock_bh(&pAd->MacTabLock); */
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

		        /* clear MCAST/BCAST backlog bit for all BSS */
			for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
	}
	else
		pMacTable->PsQIdleCount = 0;
}


uint32_t MacTableAssocStaNumGet(
	IN struct rtmp_adapter *pAd)
{
	uint32_t num = 0;
	uint32_t i;


	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num ++;
	}

	return num;
}


/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN struct rtmp_adapter *pAd,
	IN u8 *pAddr,
	OUT SST *Sst,
	OUT unsigned short *Aid,
	OUT u8 *PsMode,
	OUT u8 *Rate)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) /* mcast & broadcast address */
	{
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate;
	}
	else /* unicast address */
	{
		pEntry = MacTableLookup(pAd, pAddr);
		if (pEntry)
		{
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;
			if ((pEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pEntry->GTKState != REKEY_ESTABLISHED))
				*Rate = pAd->CommonCfg.MlmeRate;
			else
				*Rate = pEntry->CurrTxRate;
		}
		else
		{
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate;
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(
	IN struct rtmp_adapter *pAd,
	IN u8 *  pAddr,
	IN unsigned short   Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT)
	{
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];
		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		DBGPRINT_RAW(RT_DEBUG_TRACE,("LOG#%ld %02x:%02x:%02x:%02x:%02x:%02x %s\n",
			pAd->EventTab.Num, pAddr[0], pAddr[1], pAddr[2],
			pAddr[3], pAddr[4], pAddr[5], pEventText[Event]));
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates.
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(
	IN struct rtmp_adapter *pAd)
{
	bool bDisableBGProtect = false, bNonGFExist = false;

	pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) /* non HT BSS exist within 5 sec */
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 1;
		bDisableBGProtect = false;
		bNonGFExist = true;
	}

   	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode == 0) &&
		(pAd->MacTab.fAnyStation20Only) &&
		(pAd->CommonCfg.DesiredHtPhy.ChannelWidth == 1))
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 2;
		bDisableBGProtect = true;
	}

	if (pAd->MacTab.fAnyStationIsLegacy)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 3;
		bDisableBGProtect = true;
	}

	if (bNonGFExist == false)
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;

	AsicUpdateProtect(pAd,
						pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode,
						(ALLN_SETPROTECT),
						bDisableBGProtect,
						bNonGFExist);

	pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = pAd->MacTab.fAnyStationNonGF;
}

/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */
VOID APUpdateCapabilityAndErpIe(
	IN struct rtmp_adapter *pAd)
{
	u8  i, ErpIeContent = 0;
	bool ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime;
	u8 apidx;
	bool	bUseBGProtection;
	bool	LegacyBssExist;


	if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		return;

	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		/* at least one 11b client associated, turn on ERP.NonERPPresent bit */
		/* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
		if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
		{
			ShortSlotCapable = false;
			ErpIeContent |= 0x01;
		}

		/* at least one client can't support short slot */
		if ((pEntry->CapabilityInfo & 0x0400) == 0)
			ShortSlotCapable = false;
	}

	/* legacy BSS exist within 5 sec */
	if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32)
		LegacyBssExist = true;
	else
		LegacyBssExist = false;

	/* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
		AUTO (0): UseProtection = 1 if any 11b STA associated
		ON (1): always USE protection
		OFF (2): always NOT USE protection
	*/
	if (pAd->CommonCfg.UseBGProtection == 0)
	{
		ErpIeContent = (ErpIeContent)? 0x03 : 0x00;
		/*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
		if (LegacyBssExist)
		{
			ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
		}
	}
	else if (pAd->CommonCfg.UseBGProtection == 1)
		ErpIeContent |= 0x02;


	bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
						((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));

	/* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
	if (pAd->CommonCfg.Channel > 14)
		bUseBGProtection = false;

	if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
	{
		unsigned short OperationMode = 0;
		bool	bNonGFExist = 0;

		OperationMode = pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode;
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;
		if (bUseBGProtection)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			AsicUpdateProtect(pAd, OperationMode, (OFDMSETPROTECT), false, bNonGFExist);
		}
		else
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			AsicUpdateProtect(pAd, OperationMode, (OFDMSETPROTECT), true, bNonGFExist);
		}
	}

	/* Decide Barker Preamble bit of ERP IE */
	if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == true))
		pAd->ApCfg.ErpIeContent = (ErpIeContent | 0x04);
	else
		pAd->ApCfg.ErpIeContent = ErpIeContent;

	/* Force to use ShortSlotTime at A-band */
	if (pAd->CommonCfg.Channel > 14)
		ShortSlotCapable = true;

	/* deicide CapabilityInfo.ShortSlotTime bit */
    for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
    {
		unsigned short *pCapInfo = &(pAd->ApCfg.MBSSID[apidx].CapabilityInfo);

		/* In A-band, the ShortSlotTime bit should be ignored. */
		if (ShortSlotCapable
			&& (pAd->CommonCfg.Channel <= 14)
			)
    		(*pCapInfo) |= 0x0400;
		else
    		(*pCapInfo) &= 0xfbff;


   		if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
			(*pCapInfo) &= (~0x020);
		else
			(*pCapInfo) |= 0x020;

	}

	AsicSetSlotTime(pAd, ShortSlotCapable);

}

/*
	==========================================================================
	Description:
        Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
bool ApCheckLongPreambleSTA(
    IN struct rtmp_adapter *pAd)
{
    u8   i;

    for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
    {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

        if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo))
        {
            return true;
        }
    }

    return false;
}

/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
bool ApCheckAccessControlList(
	IN struct rtmp_adapter *pAd,
	IN u8 *       pAddr,
	IN u8         Apidx)
{
	bool Result = true;

    if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
        Result = true;
    else
    {
        ULONG i;
        if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
            Result = false;
        else                                              /* ACL is a negative list */
            Result = true;
        for (i=0; i<pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++)
        {
            if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr))
            {
                Result = !Result;
                break;
            }
        }
    }

    if (Result == false)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x failed in ACL checking\n",
        pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));
    }

    return Result;
}

/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(struct rtmp_adapter *pAd, u8 Apidx)
{
	unsigned short   AclIdx, MacIdx;
	bool  Matched;

	u8 *     pOutBuffer = NULL;
	int NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DisassocHdr;
	unsigned short      Reason;
	MAC_TABLE_ENTRY *pEntry;
	MULTISSID_STRUCT *pMbss;
	bool drop;

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));
	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;
	DBGPRINT(RT_DEBUG_TRACE, ("ApUpdateAccessControlList : Apidx = %d\n", Apidx));

    /* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];
	if (pMbss->AccessControlList.Policy == 0)
		return;

	for (MacIdx=0; MacIdx < MAX_LEN_OF_MAC_TABLE; MacIdx++)
	{
		pEntry = &pAd->MacTab.Content[MacIdx];
		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (pEntry->apidx != Apidx)
			continue;

		drop = false;
		Matched = false;
		 for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++)
		{
			if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr))
			{
				Matched = true;
				break;
			}
		}

		if ((Matched == false) && (pMbss->AccessControlList.Policy == 1))
		{
			drop = true;
			DBGPRINT(RT_DEBUG_TRACE, ("STA not on positive ACL. remove it...\n"));
		}
	       else if ((Matched == true) && (pMbss->AccessControlList.Policy == 2))
			{
			drop = true;
			DBGPRINT(RT_DEBUG_TRACE, ("STA on negative ACL. remove it...\n"));
				}

		if (drop == true) {
			DBGPRINT(RT_DEBUG_TRACE, ("Apidx = %d\n", Apidx));
			DBGPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
				pMbss->AccessControlList.Policy));

			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC)
			{
				/* send out a DISASSOC frame */
				pOutBuffer = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);
				if (pOutBuffer == NULL)
				{
					DBGPRINT(RT_DEBUG_TRACE, (" kmalloc fail  ..\n"));
					return;
				}

				Reason = REASON_DECLINED;
				DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",
							Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
									pEntry->Addr,
									pMbss->wdev.if_addr,
									pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				kfree(pOutBuffer);

				mdelay(5);
			}
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
	}
}


/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is false.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P 	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel
				range given by following equation:
					                                                 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
					                                                      2                          2
					Where
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting
				 Boolean value.
			        =>When the value of OPi is the empty set, then the expression (P == OPi for all values of i)
			        	is defined to be true
			        =>When the value of OTi is the empty set, then the expression (P == OTi for all values of i)
			        	is defined to be true
			        =>When the value of OSi is the empty set, then the expression (S == OSi for all values of i)
			        	is defined to be true
*/


INT GetBssCoexEffectedChRange(
	IN struct rtmp_adapter *pAd,
	IN BSS_COEX_CH_RANGE *pCoexChRange)
{
	INT index, cntrCh = 0;

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));

	/* Build the effected channel list, if something wrong, return directly. */
	if (pAd->CommonCfg.Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = pAd->ChannelList[index].Channel;

			/* Now check about the secondary and central channel */
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			}
			else
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh -4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}

			DBGPRINT(RT_DEBUG_TRACE,("5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
										index,
										((pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel,
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			return true;
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n",
						pAd->CommonCfg.Channel));
		}
	}
	else
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;

			/* Now check about the secondary and central channel */
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				if ((index + 4) < pAd->ChannelListNum)
				{
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			}
			else
			{
				if ((index - 4) >=0)
				{
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh)
			{
				pCoexChRange->effectChStart = (cntrCh - 5) > 0 ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd= (cntrCh + 5) > 0 ? (cntrCh + 5) : 0;
				DBGPRINT(RT_DEBUG_TRACE,("2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PrimaryCh=[Idx:%d, CH:%d], SecondaryCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
										index,
										((pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel,
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			}
			return true;
		}

		/* It should not happened! */
		DBGPRINT(RT_DEBUG_ERROR, ("2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%d, CtrlCh=%d\n",
									index, pAd->ChannelListNum, pAd->CommonCfg.Channel));
	}

	return false;
}


VOID APOverlappingBSSScan(struct rtmp_adapter *pAd)
{
	bool needFallBack = false;
	u8 Channel = pAd->CommonCfg.Channel;
	INT chStartIdx, chEndIdx, index,curPriChIdx, curSecChIdx;


	/* We just care BSS who operating in 40MHz N Mode. */
	if ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) ||
		(pAd->CommonCfg.RegTransmitSetting.field.BW  == BW_20)
		|| (pAd->CommonCfg.Channel > 14)
		)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("The pAd->PhyMode=%d, BW=%d, didn't need channel adjustment!\n",
				pAd->CommonCfg.PhyMode, pAd->CommonCfg.RegTransmitSetting.field.BW));
		return;
	}

	/* Build the effected channel list, if something wrong, return directly. */
	if (pAd->CommonCfg.Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			curPriChIdx = index;
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				chStartIdx = index;
				chEndIdx = chStartIdx + 1;
				curSecChIdx = chEndIdx;
			}
			else
			{
				chStartIdx = index - 1;
				chEndIdx = index;
				curSecChIdx = chStartIdx;
			}
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("5GHz: Cannot found the ControlChannel(%d) in ChannelList, something wrong?\n",
						pAd->CommonCfg.Channel));
			return;
		}
	}
	else
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{

			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				curPriChIdx = index;
				curSecChIdx = ((index + 4) < pAd->ChannelListNum) ? (index + 4) : (pAd->ChannelListNum - 1);

				chStartIdx = (curPriChIdx >= 3) ? (curPriChIdx - 3) : 0;
				chEndIdx = ((curSecChIdx + 3) < pAd->ChannelListNum) ? (curSecChIdx + 3) : (pAd->ChannelListNum - 1);
			}
			else
			{
				curPriChIdx = index;
				curSecChIdx = ((index - 4) >=0 ) ? (index - 4) : 0;
				chStartIdx =(curSecChIdx >= 3) ? (curSecChIdx - 3) : 0;
				chEndIdx =  ((curPriChIdx + 3) < pAd->ChannelListNum) ? (curPriChIdx + 3) : (pAd->ChannelListNum - 1);;
			}
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n",
						pAd->CommonCfg.Channel));
			return;
		}
	}

{
	BSS_COEX_CH_RANGE  coexChRange;
	GetBssCoexEffectedChRange(pAd, &coexChRange);
}

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pAd->ChannelListNum; index++)
		pAd->ChannelList[index].bEffectedChannel = 0;

	pAd->CommonCfg.BssCoexApCnt = 0;

	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Card still not enable Tx/Rx, enable it now!\n"));

		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

		/*
			Support multiple BulkIn IRP,
			the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1.
		*/
		for(index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
	{
			RTUSBBulkReceive(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
	}

		/* Now Enable RxTx */
		RTMPEnableRxTx(pAd);

		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}


	DBGPRINT(RT_DEBUG_TRACE, ("Ready to do passive scanning for Channel[%d] to Channel[%d]!\n",
			pAd->ChannelList[chStartIdx].Channel, pAd->ChannelList[chEndIdx].Channel));

	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = true;
	for (index = chStartIdx; index<=chEndIdx; index++)
	{
		Channel = pAd->ChannelList[index].Channel;
		AsicSetChannel(pAd, Channel, BW_20,  EXTCHA_NONE, true);

		DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));
		/*DBGPRINT(RT_DEBUG_TRACE, ("Passive scanning for Channel %d.....\n", Channel)); */
		msleep_interruptible(300); /* wait for 200 ms at each channel. */
	}
	pAd->CommonCfg.bOverlapScanning = false;

	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n",
					index, pAd->ChannelList[index].Channel, pAd->ChannelList[index].bEffectedChannel));
		if ((pAd->ChannelList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  && (index != curPriChIdx) )
		{
			needFallBack = true;
			DBGPRINT(RT_DEBUG_TRACE, ("needFallBack=true due to OP/OT!\n"));
		}
		if ((pAd->ChannelList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx))
		{
			needFallBack = true;
			DBGPRINT(RT_DEBUG_TRACE, ("needFallBack=true due to OS!\n"));
		}
	}

	/* If need fallback, now do it. */
	if ((needFallBack == true)
		&& (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr)
	)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0;
	}

	return;
}

#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    true - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
bool DOT1X_InternalCmdAction(
    IN  struct rtmp_adapter *pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN	UINT8			cmd)
{
	INT				apidx = MAIN_MBSSID;
	u8 			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	u8 		s_addr[MAC_ADDR_LEN];
	u8 		EAPOL_IE[] = {0x88, 0x8e};
	UINT8			frame_len = LENGTH_802_3 + sizeof(RalinkIe);
	u8 		FrameBuf[frame_len];
	UINT8			offset = 0;

	/* Init the frame buffer */
	memset(FrameBuf, 0, frame_len);

	if (pEntry)
	{
		apidx = pEntry->apidx;
		memmove(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	}
	else
	{
		/* Fake a Source Address for transmission */
		memmove(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;

	/* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf,
					  pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					  s_addr,
					  EAPOL_IE);
	offset += LENGTH_802_3;

	/* Prepare the specific header of internal command */
	memmove(&FrameBuf[offset], RalinkIe, sizeof(RalinkIe));

	/* Report to upper layer */
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == false)
		return false;

	DBGPRINT(RT_DEBUG_TRACE, ("%s done. (cmd=%d)\n", __FUNCTION__, cmd));

	return true;
}

/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    true - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
bool DOT1X_EapTriggerAction(
    IN  struct rtmp_adapter *pAd,
    IN  MAC_TABLE_ENTRY *pEntry)
{
	INT				apidx = MAIN_MBSSID;
	u8 			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	u8 		FrameBuf[frame_len];
	UINT8			offset = 0;

    if((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pAd->ApCfg.MBSSID[apidx].wdev.IEEE8021X == true))
	{
		/* Init the frame buffer */
		memset(FrameBuf, 0, frame_len);

		/* Assign apidx */
		apidx = pEntry->apidx;

		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
		offset += LENGTH_802_3;

		/* Prepare a fake eapol-start body */
		memmove(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == false)
			return false;

		DBGPRINT(RT_DEBUG_TRACE, ("Notify 8021.x daemon to trigger EAP-SM for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));

	}

	return true;
}

#endif /* DOT1X_SUPPORT */

