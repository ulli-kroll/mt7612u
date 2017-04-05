/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All related CFG80211 function body.

	History:

***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#include "rt_config.h"

static INT CFG80211DRV_UpdateTimIE(struct rtmp_adapter *pAd, UINT mbss_idx, u8 *pBeaconFrame, uint32_t tim_ie_pos)
{
	UCHAR  ID_1B, TimFirst, TimLast, *pTim, *ptr, New_Tim_Len;
	UINT  i;

	ptr = pBeaconFrame + tim_ie_pos; /* TIM LOCATION */
	*ptr = IE_TIM;
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;

	TimFirst = 0; /* record first TIM byte != 0x00 */
	TimLast = 0;  /* record last  TIM byte != 0x00 */

	pTim = pAd->ApCfg.MBSSID[mbss_idx].TimBitmaps;

	for(ID_1B=0; ID_1B < WLAN_MAX_NUM_OF_TIM; ID_1B++)
	{
		/* get the TIM indicating PS packets for 8 stations */
		UCHAR tim_1B = pTim[ID_1B];

		if (ID_1B == 0)
			tim_1B &= 0xfe; /* skip bit0 bc/mc */

		if (tim_1B == 0)
			continue; /* find next 1B */

		if (TimFirst == 0)
			TimFirst = ID_1B;

		TimLast = ID_1B;
	}

	/* fill TIM content to beacon buffer */
	if (TimFirst & 0x01)
		TimFirst --; /* find the even offset byte */

	*(ptr + 1) = 3 + (TimLast - TimFirst + 1); /* TIM IE length */
	*(ptr + 4) = TimFirst;

	for(i=TimFirst; i <= TimLast; i++)
		*(ptr + 5 + i - TimFirst) = pTim[i];

	/* bit0 means backlogged mcast/bcast */
    if (pAd->ApCfg.DtimCount == 0)
		*(ptr + 4) |= (pAd->ApCfg.MBSSID[mbss_idx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & 0x01);

	/* adjust BEACON length according to the new TIM */
	New_Tim_Len = (2 + *(ptr+1));

	return New_Tim_Len;
}

static void CFG80211DRV_UpdateApSettingFromBeacon(struct rtmp_adapter *pAd, UINT mbss_idx, CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon)
{
	PMULTISSID_STRUCT pMbss = &pAd->ApCfg.MBSSID[mbss_idx];
	struct rtmp_wifi_dev *wdev = &pMbss->wdev;

	const UCHAR *ssid_ie = NULL, *wpa_ie = NULL, *rsn_ie = NULL;
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;

	const UCHAR *supp_rates_ie = NULL;
	const UCHAR *ext_supp_rates_ie = NULL, *ht_cap = NULL, *ht_info = NULL;

	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_OPERATION;

	const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;

	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pBeacon->beacon_head+36, pBeacon->beacon_head_len-36);
	supp_rates_ie = cfg80211_find_ie(WLAN_EID_SUPP_RATES, pBeacon->beacon_head+36, pBeacon->beacon_head_len-36);
	/* if it doesn't find WPA_IE in tail first 30 bytes. treat it as is not found */
	wpa_ie = cfg80211_find_ie(CFG_WPA_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	rsn_ie = cfg80211_find_ie(WLAN_EID_RSN, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	wmm_ie = (u8 *) cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_cap = cfg80211_find_ie(WLAN_EID_HT_CAPABILITY, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_info = cfg80211_find_ie(CFG_HT_OP_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);

	/* SSID */
	memset(pMbss->Ssid, 0, pMbss->SsidLen);
	if (ssid_ie == NULL)
	{
		memmove(pMbss->Ssid, "CFG_Linux_GO", 12);
		pMbss->SsidLen = 12;
		DBGPRINT(RT_DEBUG_ERROR,("CFG: SSID Not Found In Packet\n"));
	}
	else
	{
		pMbss->SsidLen = ssid_ie[1];
		memcpy(pMbss->Ssid, ssid_ie+2, pMbss->SsidLen);
		DBGPRINT(RT_DEBUG_TRACE,("CFG : SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen));
	}

	if (pBeacon->hidden_ssid > 0 && pBeacon->hidden_ssid < 3) {
		pMbss->bHideSsid = true;
	}
	else
		pMbss->bHideSsid = false;

	if (pBeacon->hidden_ssid == 1)
		pMbss->SsidLen = 0;

	/* WMM EDCA Paramter */
	CFG80211_SyncPacketWmmIe(pAd, pBeacon->beacon_tail, pBeacon->beacon_tail_len);

	/* Security */
	CFG80211_ParseBeaconIE(pAd, pMbss, wdev, wpa_ie, rsn_ie);

	pMbss->CapabilityInfo =	CAP_GENERATE(1, 0, (wdev->WepStatus != Ndis802_11EncryptionDisabled),
			 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1), pAd->CommonCfg.bUseShortSlotTime, /*SpectrumMgmt*/false);

	/* Disable Driver-Internal Rekey */
	pMbss->WPAREKEY.ReKeyInterval = 0;
	pMbss->WPAREKEY.ReKeyMethod = DISABLE_REKEY;

	if (pBeacon->interval != 0)
	{
		DBGPRINT(RT_DEBUG_TRACE,("CFG_TIM New BI %d\n", pBeacon->interval));
		pAd->CommonCfg.BeaconPeriod = pBeacon->interval;
	}

	if (pBeacon->dtim_period != 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG_TIM New DP %d\n", pBeacon->dtim_period));
		pAd->ApCfg.DtimPeriod = pBeacon->dtim_period;
	}

}

VOID CFG80211DRV_DisableApInterface(struct rtmp_adapter *pAd)
{
	/*CFG_TODO: IT Should be set fRTMP_ADAPTER_HALT_IN_PROGRESS */
	MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct rtmp_wifi_dev *wdev = &pMbss->wdev;

	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = false;
	wdev->Hostapd = Hostapd_Diable;

  	/* For AP - STA switch */
	if (pAd->CommonCfg.BBPCurrentBW != BW_40)
	{
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s, switch to BW_20\n", __FUNCTION__));
		mt7612u_bbp_set_bw(pAd, BW_20);
   	}

    /* Disable pre-TBTT interrupt */
    AsicSetPreTbtt(pAd, false);

    if (!INFRA_ON(pAd))
    {
		/* Disable piggyback */
		RTMPSetPiggyBack(pAd, false);
		AsicUpdateProtect(pAd, 0,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), true, false);
    }

    if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        AsicDisableSync(pAd);
    }

#ifdef RTMP_MAC_USB
    /* For RT2870, we need to clear the beacon sync buffer. */
    RTUSBBssBeaconExit(pAd);
#endif /* RTMP_MAC_USB */

	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
#ifdef CONFIG_STA_SUPPORT
	/* re-assoc to STA's wdev */
	RTMP_OS_NETDEV_SET_WDEV(pAd->net_dev, &pAd->StaCfg.wdev);
#endif /*CONFIG_STA_SUPPORT*/
}

VOID CFG80211_UpdateBeacon(
	struct rtmp_adapter                                *pAd,
	UCHAR 										    *beacon_head_buf,
	uint32_t 										beacon_head_len,
	UCHAR 										    *beacon_tail_buf,
	uint32_t 										beacon_tail_len,
	bool											isAllUpdate)
{
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	HTTRANSMIT_SETTING BeaconTransmit;   /* MGMT frame PHY rate setting when operatin at Ht rate. */
	u8 *pBeaconFrame = (u8 *)pAd->ApCfg.MBSSID[MAIN_MBSSID].BeaconBuf;
	TXWI_STRUC *pTxWI = &pAd->BeaconTxWI;
	UCHAR New_Tim_Len;
	uint32_t beacon_len;

	/* Invoke From CFG80211 OPS For setting Beacon buffer */
	if (isAllUpdate)
	{
		/* 1. Update the Before TIM IE */
		memcpy(pBeaconFrame, beacon_head_buf, beacon_head_len);

		/* 2. Update the TIM IE */
		pAd->ApCfg.MBSSID[MAIN_MBSSID].TimIELocationInBeacon = beacon_head_len;

		/* 3. Store the Tail Part For appending later */
		if (pCfg80211_ctrl->beacon_tail_buf != NULL)
			 kfree(pCfg80211_ctrl->beacon_tail_buf);

		pCfg80211_ctrl->beacon_tail_buf = kmalloc(beacon_tail_len, GFP_ATOMIC);
		if (pCfg80211_ctrl->beacon_tail_buf != NULL) {
			memcpy(pCfg80211_ctrl->beacon_tail_buf, beacon_tail_buf, beacon_tail_len);
			pCfg80211_ctrl->beacon_tail_len = beacon_tail_len;
		}
		else
		{
			pCfg80211_ctrl->beacon_tail_len = 0;
			DBGPRINT(RT_DEBUG_ERROR, ("CFG80211 Beacon: MEM ALLOC ERROR\n"));
		}

		return;
	}
	else /* Invoke From Beacon Timer */
	{
		if (pAd->ApCfg.DtimCount == 0)
			pAd->ApCfg.DtimCount = pAd->ApCfg.DtimPeriod - 1;
		else
			pAd->ApCfg.DtimCount -= 1;
	}

	/* 4. Update the TIM IE */
	New_Tim_Len = CFG80211DRV_UpdateTimIE(pAd, MAIN_MBSSID, pBeaconFrame,
				pAd->ApCfg.MBSSID[MAIN_MBSSID].TimIELocationInBeacon);

	/* 5. Update the AFTER TIM IE */
	if (pCfg80211_ctrl->beacon_tail_buf != NULL)
	{
		memcpy(pAd->ApCfg.MBSSID[MAIN_MBSSID].BeaconBuf +
			       pAd->ApCfg.MBSSID[MAIN_MBSSID].TimIELocationInBeacon + New_Tim_Len,
			       pCfg80211_ctrl->beacon_tail_buf, pCfg80211_ctrl->beacon_tail_len);

		beacon_len = pAd->ApCfg.MBSSID[MAIN_MBSSID].TimIELocationInBeacon + pCfg80211_ctrl->beacon_tail_len
			     + New_Tim_Len;
	}
	else
	{
		 DBGPRINT(RT_DEBUG_ERROR, ("BEACON ====> CFG80211_UpdateBeacon OOPS\n"));
		 return;
	}

    BeaconTransmit.word = 0;
	/* Should be Find the P2P IE Then Set Basic Rate to 6M */
	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
	BeaconTransmit.field.MODE = MODE_OFDM; /* Use 6Mbps */
	else
		BeaconTransmit.field.MODE = MODE_CCK;
	BeaconTransmit.field.MCS = MCS_RATE_6;

	//YF
	RTMPWriteTxWI(pAd, pTxWI, false, false, true, false, false, true, 0, BSS0Mcast_WCID,
                	beacon_len, PID_MGMT, 0, 0, IFS_HTTXOP, &BeaconTransmit);

	/* CFG_TODO */
	RT28xx_UpdateBeaconToAsic(pAd, MAIN_MBSSID, beacon_len,
			pAd->ApCfg.MBSSID[MAIN_MBSSID].TimIELocationInBeacon);

}

void CFG80211DRV_OpsBeaconSet(struct rtmp_adapter *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon;
	pBeacon = (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;

	CFG80211DRV_UpdateApSettingFromBeacon(pAd, MAIN_MBSSID, pBeacon);
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
			  				   pBeacon->beacon_tail, pBeacon->beacon_tail_len,
							   true);
}

bool CFG80211DRV_OpsBeaconAdd(struct rtmp_adapter *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon;
	uint32_t rx_filter_flag;
	bool Cancelled;
	INT i;
	PMULTISSID_STRUCT pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct rtmp_wifi_dev *wdev = &pMbss->wdev;
	UCHAR num_idx;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
#ifdef RTMP_MAC_USB
#ifdef CONFIG_AP_SUPPORT
	RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled);
#endif /*CONFIG_AP_SUPPORT*/
#endif /* RTMP_MAC_USB */

	pBeacon = (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;


	CFG80211DRV_UpdateApSettingFromBeacon(pAd, MAIN_MBSSID, pBeacon);

	rx_filter_flag = APNORMAL;
	mt7612u_write32(pAd, RX_FILTR_CFG, rx_filter_flag);     /* enable RX of DMA block */

	pAd->ApCfg.BssidNum = 1;
	pAd->MacTab.MsduLifeTime = 20; /* default 5 seconds */
	/* CFG_TODO */
	pAd->ApCfg.MBSSID[MAIN_MBSSID].BcnBufIdx = 0 ;
	for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
                pAd->ApCfg.MBSSID[MAIN_MBSSID].TimBitmaps[i] = 0;

	pMbss->bBcnSntReq = true;

	/* For GO Timeout */
	pAd->ApCfg.StaIdleTimeout = 300;
	pMbss->StationKeepAliveTime = 0;

	AsicDisableSync(pAd);

	if (pAd->CommonCfg.Channel > 14)
		pAd->CommonCfg.PhyMode = (WMODE_A | WMODE_AN);
	else
		pAd->CommonCfg.PhyMode = (WMODE_B | WMODE_G |WMODE_GN);

	/* cfg_todo */
	wdev->bWmmCapable = true;

	wdev->wdev_type = WDEV_TYPE_AP;
	wdev->tx_pkt_allowed = ApAllowToSendPacket;
	wdev->allow_data_tx = true;
	wdev->func_dev = (void *)&pAd->ApCfg.MBSSID[MAIN_MBSSID];
	wdev->sys_handle = pAd;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Using netDev ptr from VifList if VifDevList Exist */
	struct net_device *pNetDev = NULL;
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
	   ((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO)) != NULL))
	{
		pMbss->MSSIDDev = pNetDev;
		wdev->if_dev = pNetDev;
		COPY_MAC_ADDR(wdev->bssid, pNetDev->dev_addr);
		COPY_MAC_ADDR(wdev->if_addr, pNetDev->dev_addr);

		RTMP_OS_NETDEV_SET_WDEV(pNetDev, wdev);
	}
	else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
		pMbss->MSSIDDev = pAd->net_dev;
		wdev->if_dev = pAd->net_dev;
		COPY_MAC_ADDR(wdev->bssid, pAd->CurrentAddress);
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);

		/* assoc to MBSSID's wdev */
		RTMP_OS_NETDEV_SET_WDEV(pAd->net_dev, wdev);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("New AP BSSID %02x:%02x:%02x:%02x:%02x:%02x (%d)\n",
		PRINT_MAC(wdev->bssid), pAd->CommonCfg.PhyMode));

	RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && (pAd->Antenna.field.TxPath == 2))
		mt7612u_bbp_set_txdac(pAd, 2);
	else
		mt7612u_bbp_set_txdac(pAd, 0);

	/* Receiver Antenna selection */
	mt7612u_bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) || wdev->bWmmCapable)
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
	}

	AsicSetRDG(pAd, pAd->CommonCfg.bRdg);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);

	AsicSetBssid(pAd, pAd->CurrentAddress);
	mgmt_tb_set_mcast_entry(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s():Reset WCID Table\n", __FUNCTION__));
	AsicDelWcidTab(pAd, WCID_ALL);

	pAd->MacTab.Content[0].Addr[0] = 0x01;
	pAd->MacTab.Content[0].HTPhyMode.field.MODE = MODE_OFDM;
	pAd->MacTab.Content[0].HTPhyMode.field.MCS = 3;
	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;

	AsicBBPAdjust(pAd);
	//MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* P2P_GO */
	MlmeUpdateTxRates(pAd, false, MAIN_MBSSID + MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	/*AP */
		MlmeUpdateTxRates(pAd, false, MIN_NET_DEVICE_FOR_MBSSID);

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		MlmeUpdateHtTxRates(pAd, MIN_NET_DEVICE_FOR_MBSSID);

	/* Disable Protection first. */
	if (!INFRA_ON(pAd))
		AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), true, false);

	APUpdateCapabilityAndErpIe(pAd);
	APUpdateOperationMode(pAd);
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
                                   pBeacon->beacon_tail, pBeacon->beacon_tail_len,
                                   true);

#ifdef RTMP_MAC_USB
	RTUSBBssBeaconInit(pAd);
#endif /* RTMP_MAC_USB */

#ifdef RTMP_MAC_USB
#ifdef CONFIG_STA_SUPPORT
    RTMPInitTimer(pAd, &pAd->CommonCfg.BeaconUpdateTimer, GET_TIMER_FUNCTION(BeaconUpdateExec), pAd, true);
#endif /*CONFIG_STA_SUPPORT*/
	RTUSBBssBeaconStart(pAd);
#endif /* RTMP_MAC_USB */

	/* Enable BSS Sync*/
	AsicEnableApBssSync(pAd);
	//pAd->P2pCfg.bSentProbeRSP = true;

	AsicSetPreTbtt(pAd, true);

#ifdef RTMP_MAC_USB
	/*
	 * Support multiple BulkIn IRP,
	 * the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1.
	 */
	for(num_idx=0; num_idx < pAd->CommonCfg.NumOfBulkInIRP; num_idx++)
	{
		RTUSBBulkReceive(pAd);
	}
#endif /* RTMP_MAC_USB */

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
#ifdef RT_CFG80211_SUPPORT
		wdev->Hostapd=Hostapd_CFG;
#endif /*RT_CFG80211_SUPPORT*/
	return true;
}

bool CFG80211DRV_ApKeyDel(
	struct rtmp_adapter 				*pAd,
	VOID                                            *pData)
{
    CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	MAC_TABLE_ENTRY *pEntry;

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

        if (pKeyInfo->bPairwise == false )
	{
		DBGPRINT(RT_DEBUG_TRACE,("CFG: AsicRemoveSharedKeyEntry %d\n", pKeyInfo->KeyId));
		AsicRemoveSharedKeyEntry(pAd, MAIN_MBSSID, pKeyInfo->KeyId);
	}
	else
	{
		pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

		if (pEntry && (pEntry->Aid != 0))
		{
			memset(&pEntry->PairwiseKey, 0, sizeof(CIPHER_KEY));
			AsicRemovePairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid);
		}
	}

	return true;
}


VOID CFG80211DRV_RtsThresholdAdd(
	struct rtmp_adapter                             *pAd,
	UINT                                            threshold)
{

		if((threshold > 0) && (threshold <= MAX_RTS_THRESHOLD))
			pAd->CommonCfg.RtsThreshold  = (USHORT)threshold;
#ifdef CONFIG_STA_SUPPORT
		else if (threshold== 0)
			pAd->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;
#endif /* CONFIG_STA_SUPPORT */
}


VOID CFG80211DRV_FragThresholdAdd(
	struct rtmp_adapter                            *pAd,
	UINT                                            threshold)
{
		if (threshold > MAX_FRAG_THRESHOLD || threshold < MIN_FRAG_THRESHOLD)
		{
			/*Illegal FragThresh so we set it to default*/
			pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
		}
		else if (threshold % 2 == 1)
		{
			/*
				The length of each fragment shall always be an even number of octets,
				except for the last fragment of an MSDU or MMPDU, which may be either
				an even or an odd number of octets.
			*/
			pAd->CommonCfg.FragmentThreshold = (USHORT)(threshold - 1);
		}
		else
		{
			pAd->CommonCfg.FragmentThreshold = (USHORT)threshold;
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (pAd->CommonCfg.FragmentThreshold == MAX_FRAG_THRESHOLD)
				pAd->CommonCfg.bUseZeroToDisableFragment = true;
			else
				pAd->CommonCfg.bUseZeroToDisableFragment = false;
		}
#endif /* CONFIG_STA_SUPPORT */
}


bool CFG80211DRV_ApKeyAdd(struct rtmp_adapter *pAd, void *pData)
{
#ifdef CONFIG_AP_SUPPORT
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	PMULTISSID_STRUCT pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct rtmp_wifi_dev *pWdev = &pMbss->wdev;
	UINT8 Wcid;
#ifdef RT_CFG80211_SUPPORT
	UINT apidx = MAIN_MBSSID;
#endif

	DBGPRINT(RT_DEBUG_TRACE,("%s =====> \n", __FUNCTION__));
	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 ||
	    pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
		pWdev->WepStatus = Ndis802_11WEPEnabled;
		{
			UCHAR CipherAlg;
			CIPHER_KEY	*pSharedKey;
			struct os_cookie *pObj;

			pObj = pAd->OS_Cookie;

			pSharedKey = &pAd->SharedKey[apidx][pKeyInfo->KeyId];
			memmove(pSharedKey->Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);


			if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_WEP64;
			else
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_WEP128;

			AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId, pSharedKey);
		}
	} else if(pKeyInfo->KeyType == RT_CMD_80211_KEY_WPA) {
		if (pKeyInfo->cipher == Ndis802_11AESEnable) {
		/* AES */
				//pWdev->WepStatus = Ndis802_11Encryption3Enabled;
				//pWdev->GroupKeyWepStatus = Ndis802_11Encryption3Enabled;
		        if (pKeyInfo->bPairwise == false ) {
				if (pWdev->GroupKeyWepStatus == Ndis802_11Encryption3Enabled) {
					DBGPRINT(RT_DEBUG_TRACE, ("CFG: Set AES Security Set. (GROUP) %d\n", pKeyInfo->KeyLen));
					pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].KeyLen= LEN_TK;
					memmove(pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);

					pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].CipherAlg = CIPHER_AES;

					AsicAddSharedKeyEntry(pAd, MAIN_MBSSID, pKeyInfo->KeyId,
							&pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId]);

					GET_GroupKey_WCID(pAd, Wcid, MAIN_MBSSID);
					RTMPSetWcidSecurityInfo(pAd, MAIN_MBSSID, (UINT8)(pKeyInfo->KeyId),
							pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].CipherAlg, Wcid, SHAREDKEYTABLE);

				}
			} else {
				MAC_TABLE_ENTRY *pEntry = NULL;

				if (pKeyInfo->MAC)
					pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

				if (pEntry) {
					DBGPRINT(RT_DEBUG_TRACE, ("CFG: Set AES Security Set. (PAIRWISE) %d\n", pKeyInfo->KeyLen));
					pEntry->PairwiseKey.KeyLen = LEN_TK;
					memcpy(&pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
					memmove(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);
					pEntry->PairwiseKey.CipherAlg = CIPHER_AES;

					AsicAddPairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid, &pEntry->PairwiseKey);
					RTMPSetWcidSecurityInfo(pAd, pEntry->apidx, (UINT8)(pKeyInfo->KeyId & 0x0fff),
						pEntry->PairwiseKey.CipherAlg, pEntry->Aid, PAIRWISEKEYTABLE);

				} else {
					DBGPRINT(RT_DEBUG_ERROR,("CFG: Set AES Security Set. (PAIRWISE) But pEntry NULL\n"));
				}

			}
		} else if (pKeyInfo->cipher == Ndis802_11TKIPEnable) {
		/* TKIP */
				//pWdev->WepStatus = Ndis802_11Encryption2Enabled;
				//pWdev->GroupKeyWepStatus = Ndis802_11Encryption2Enabled;
		        if (pKeyInfo->bPairwise == false ) {
				if (pWdev->GroupKeyWepStatus == Ndis802_11Encryption2Enabled) {
					DBGPRINT(RT_DEBUG_TRACE, ("CFG: Set TKIP Security Set. (GROUP) %d\n", pKeyInfo->KeyLen));
					pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].KeyLen= LEN_TK;
					memmove(pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);

					pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].CipherAlg = CIPHER_TKIP;

					AsicAddSharedKeyEntry(pAd, MAIN_MBSSID, pKeyInfo->KeyId,
							&pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId]);

					GET_GroupKey_WCID(pAd, Wcid, MAIN_MBSSID);
					RTMPSetWcidSecurityInfo(pAd, MAIN_MBSSID, (UINT8)(pKeyInfo->KeyId),
							pAd->SharedKey[MAIN_MBSSID][pKeyInfo->KeyId].CipherAlg, Wcid, SHAREDKEYTABLE);

				}
			} else {
				MAC_TABLE_ENTRY *pEntry = NULL;

				if (pKeyInfo->MAC)
					pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

				if (pEntry) {
					DBGPRINT(RT_DEBUG_TRACE, ("CFG: Set TKIP Security Set. (PAIRWISE) %d\n", pKeyInfo->KeyLen));
					pEntry->PairwiseKey.KeyLen = LEN_TK;
					memcpy(&pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
					memmove(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);
					pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;

					AsicAddPairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid, &pEntry->PairwiseKey);
					RTMPSetWcidSecurityInfo(pAd, pEntry->apidx, (UINT8)(pKeyInfo->KeyId & 0x0fff),
						pEntry->PairwiseKey.CipherAlg, pEntry->Aid, PAIRWISEKEYTABLE);

				} else {
					DBGPRINT(RT_DEBUG_ERROR,("CFG: Set TKIP Security Set. (PAIRWISE) But pEntry NULL\n"));
				}

			}
		}

	}
#endif /* CONFIG_AP_SUPPORT */
	return true;

}

INT CFG80211_StaPortSecured(
	IN struct rtmp_adapter                          *pAd,
	IN UCHAR 					*pMac,
	IN UINT						flag)
{
	MAC_TABLE_ENTRY *pEntry;

	pEntry = MacTableLookup(pAd, pMac);
	if (!pEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can't find pEntry in CFG80211_StaPortSecured\n"));
	}
	else
	{
		if (flag)
		{
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			pEntry->WpaState = AS_PTKINITDONE;
			pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
			DBGPRINT(RT_DEBUG_TRACE,("AID:%d, PortSecured\n", pEntry->Aid));
		}
		else
		{
			pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			DBGPRINT(RT_DEBUG_TRACE,("AID:%d, PortNotSecured\n", pEntry->Aid));
		}
	}

	return 0;
}

void CFG80211_ApStaDel(
	IN struct rtmp_adapter                          *pAd,
	IN UCHAR                                        *pMac)
{
	MAC_TABLE_ENTRY *pEntry;

	if (pMac == NULL)
	{
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		/* From WCID=2 */
		if (INFRA_ON(pAd))
			;//P2PMacTableReset(pAd);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
			MacTableReset(pAd);
	}
	else
	{
		pEntry = MacTableLookup(pAd, pMac);
		if (pEntry)
		{
			MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, false);
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, ("Can't find pEntry in ApStaDel\n"));
	}
}

INT CFG80211_setApDefaultKey(
	IN struct rtmp_adapter                    *pAd,
	IN UINT 					Data
)
{
	DBGPRINT(RT_DEBUG_TRACE, ("Set Ap Default Key: %d\n", Data));
    pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.DefaultKeyId = Data;
	return 0;
}

void CFG80211_ApStaDelSendEvent(struct rtmp_adapter *pAd, const u8 *mac_addr)
{
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	struct net_device *pNetDev = NULL;
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO)) != NULL))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CONCURRENT_DEVICE CFG : GO NOITFY THE CLIENT Disconnected\n"));
		CFG80211OS_DelSta(pNetDev, mac_addr);
	}
	else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SINGLE_DEVICE CFG : GO NOITFY THE CLIENT Disconnected\n"));
		CFG80211OS_DelSta(pAd->net_dev, mac_addr);
	}
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
