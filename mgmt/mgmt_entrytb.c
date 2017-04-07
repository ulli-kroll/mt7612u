/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include <rt_config.h>


// TODO: this function not finish yet!!
VOID mgmt_tb_set_mcast_entry(struct rtmp_adapter *pAd)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];

	pEntry->EntryType = ENTRY_WDEV;
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index*/
	pEntry->wcid = MCAST_WCID;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
		pEntry->wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd){
		pEntry->wdev = &pAd->StaCfg.wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID set_entry_phy_cfg(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry)
{

	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	}
	else
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}
}


/*
	==========================================================================
	Description:
		Look up the MAC address in the MAC table. Return NULL if not found.
	Return:
		pEntry - pointer to the MAC entry; NULL is not found
	==========================================================================
*/
MAC_TABLE_ENTRY *MacTableLookup(struct rtmp_adapter *pAd, u8 *pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry))
	{
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;
}


#ifdef CONFIG_STA_SUPPORT
bool StaUpdateMacTableEntry(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN u8 MaxSupportedRateIn500Kbps,
	IN HT_CAPABILITY_IE *ht_cap,
	IN u8 htcap_len,
	IN ADD_HT_INFO_IE *pAddHtInfo,
	IN u8 AddHtInfoLen,
	IN IE_LISTS *ie_list,
	IN USHORT cap_info)
{
	u8 MaxSupportedRate = RATE_11;
	bool bSupportN = false;
	bool supportsETxBf = false;
	struct rtmp_wifi_dev *wdev;

	if (!pEntry)
		return false;

	if (ADHOC_ON(pAd))
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_G)
	    && (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
		return false;

	/* 11n only */
	if (WMODE_HT_ONLY(pAd->CommonCfg.PhyMode)
	    && (htcap_len == 0))
		return false;

	NdisAcquireSpinLock(&pAd->MacTabLock);
	if (pEntry) {
		memset(pEntry->R_Counter, 0, sizeof(pEntry->R_Counter));
		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
		if ((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
		    WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B)) {
			pEntry->RateLen = 4;
			if (MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
				MaxSupportedRate = RATE_11;
		} else
			pEntry->RateLen = 12;

		pEntry->MaxHTPhyMode.word = 0;
		pEntry->MinHTPhyMode.word = 0;
		pEntry->HTPhyMode.word = 0;
		pEntry->MaxSupportedRate = MaxSupportedRate;

		set_entry_phy_cfg(pAd, pEntry);

		pEntry->CapabilityInfo = cap_info;
		pEntry->isCached = false;
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	}

	wdev = &pAd->StaCfg.wdev;
	memset(&pEntry->HTCapability, 0, sizeof (pEntry->HTCapability));
	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if (((wdev->WepStatus != Ndis802_11WEPEnabled)
	     && (wdev->WepStatus != Ndis802_11TKIPEnable))
	    || (pAd->CommonCfg.HT_DisallowTKIP == false)) {
		if ((pAd->StaCfg.BssType == BSS_INFRA) &&
		    (htcap_len != 0) &&
		    WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			bSupportN = true;
		if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
		    (pAd->StaCfg.bAdhocN == true) &&
		    (htcap_len != 0) &&
		    WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			bSupportN = true;
	}

	if (bSupportN) {
		if (ADHOC_ON(pAd))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

		ht_mode_adjust(pAd, pEntry, ht_cap, &pAd->CommonCfg.DesiredHtPhy);

		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
				(pAd->CommonCfg.Channel > 14) &&
				(ie_list->vht_cap_len))
		{
			supportsETxBf = clientSupportsVHTETxBF(pAd, &ie_list->vht_cap.vht_cap);
			DBGPRINT(RT_DEBUG_TRACE, ("%s : VHT mode!\n", __FUNCTION__));
			DBGPRINT(RT_DEBUG_TRACE, ("AP Bfee Cap =%d, AP Bfer Cap =%d!\n",
							ie_list->vht_cap.vht_cap.bfee_cap_su , ie_list->vht_cap.vht_cap.bfer_cap_su));
		}
		else
			supportsETxBf = clientSupportsETxBF(pAd, &ht_cap->TxBFCap);

		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0], &ht_cap->MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
			set_ht_fixed_mcs(pAd, pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);

		pEntry->MaxHTPhyMode.field.STBC = (ht_cap->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
		pEntry->MpduDensity = ht_cap->HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = ht_cap->HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (u8) ht_cap->HtCapInfo.MimoPs;
		pEntry->AMsduSize = (u8) ht_cap->HtCapInfo.AMsduSize;
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

		if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable
		    && (pAd->CommonCfg.REGBACapability.field.AutoBA == false))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);
		if (ht_cap->HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);
		if (ht_cap->HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
		if (ht_cap->HtCapInfo.TxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
		if (ht_cap->HtCapInfo.RxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
		if (ht_cap->ExtHtCapInfo.PlusHTC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);
		if (pAd->CommonCfg.bRdg
		    && ht_cap->ExtHtCapInfo.RDGSupport)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);
		if (ht_cap->ExtHtCapInfo.MCSFeedback == 0x03)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);
		memmove(&pEntry->HTCapability, ht_cap, htcap_len);

		assoc_ht_info_debugshow(pAd, pEntry, htcap_len, ht_cap);
		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
			(ie_list != NULL) && (ie_list->vht_cap_len) && (ie_list->vht_op_len))
		{
			vht_mode_adjust(pAd, pEntry, &ie_list->vht_cap, &ie_list->vht_op);
			assoc_vht_info_debugshow(pAd, pEntry, &ie_list->vht_cap, &ie_list->vht_op);
			memmove(&pEntry->vht_cap_ie, &ie_list->vht_cap, sizeof(VHT_CAP_IE));
		}
	} else {
		pAd->MacTab.fAnyStationIsLegacy = true;
	}

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
	pEntry->CurrTxRate = pEntry->MaxSupportedRate;


	pEntry->freqOffsetValid = false;

	TxBFInit(pAd, pEntry, supportsETxBf);

	RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, false);
	spin_lock_init(&pEntry->TxSndgLock);

	MlmeRAInit(pAd, pEntry);

	/* Set asic auto fall back */
	if (wdev->bAutoTxRateSwitch == true) {
		u8 TableSize = 0;

		MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
		pEntry->bAutoTxRateSwitch = true;
	} else {
		pEntry->HTPhyMode.field.MODE = wdev->HTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
		pEntry->bAutoTxRateSwitch = false;

		/* If the legacy mode is set, overwrite the transmit setting of this entry. */
		RTMPUpdateLegacyTxSetting((u8)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}


	pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->Sst = SST_ASSOC;
	pEntry->AuthState = AS_AUTH_OPEN;
	pEntry->AuthMode = wdev->AuthMode;
	pEntry->WepStatus = wdev->WepStatus;
	pEntry->wdev = wdev;
	if (pEntry->AuthMode < Ndis802_11AuthModeWPA) {
		pEntry->WpaState = AS_NOTUSE;
		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	} else {
		pEntry->WpaState = AS_INITPSK;
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	}

	if (pAd->StaCfg.BssType == BSS_INFRA) {
		u8 HashIdx = 0;
		MAC_TABLE_ENTRY *pCurrEntry = NULL;
		HashIdx = MAC_ADDR_HASH_INDEX(pAd->MlmeAux.Bssid);
		if (pAd->MacTab.Hash[HashIdx] == NULL) {
			pAd->MacTab.Hash[HashIdx] = pEntry;
		} else {
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL) {
				pCurrEntry = pCurrEntry->pNext;
			}
			pCurrEntry->pNext = pEntry;
		}
		RTMPMoveMemory(pEntry->Addr, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
		pEntry->Aid = BSSID_WCID;
		pEntry->wcid = BSSID_WCID;


		pEntry->pAd = pAd;
		SET_ENTRY_CLIENT(pEntry);
		pAd->MacTab.Size ++;
	}

#ifdef DOT11W_PMF_SUPPORT
        RTMPInitTimer(pAd, &pEntry->SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, false);
        RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, false);
#endif /* DOT11W_PMF_SUPPORT */
	NdisReleaseSpinLock(&pAd->MacTabLock);

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
	if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP) {
		SendAssocIEsToWpaSupplicant(pAd->net_dev, pAd->StaCfg.ReqVarIEs,
					    pAd->StaCfg.ReqVarIELen);

		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
					RT_ASSOC_EVENT_FLAG, NULL, NULL, 0);
	}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	{
		wext_notify_event_assoc(pAd->net_dev, pAd->StaCfg.ReqVarIEs,
					pAd->StaCfg.ReqVarIELen);

		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1,
					pAd->MlmeAux.Bssid, NULL, 0);
	}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */


	return true;
}
#endif /* CONFIG_STA_SUPPORT */


MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN struct rtmp_adapter *pAd,
	IN u8 *pAddr,
	IN struct rtmp_wifi_dev *wdev,
	IN u8 apidx,
	IN u8 OpMode,
	IN bool CleanAll)
{
	u8 HashIdx;
	int i, FirstWcid;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	bool Cancelled;

	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

		FirstWcid = 1;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	if (pAd->StaCfg.BssType == BSS_INFRA)
		FirstWcid = 2;
#endif /* CONFIG_STA_SUPPORT */

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	for (i = FirstWcid; i< MAX_LEN_OF_MAC_TABLE; i++)   /* skip entry#0 so that "entry index == AID" for fast lookup*/
	{
		/* pick up the first available vacancy*/
		if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
		{
			pEntry = &pAd->MacTab.Content[i];

			/* ENTRY PREEMPTION: initialize the entry */
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
			RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

			memset(pEntry, 0, sizeof(MAC_TABLE_ENTRY));

			if (CleanAll == true)
			{
				pEntry->MaxSupportedRate = RATE_11;
				pEntry->CurrTxRate = RATE_11;
				memset(pEntry, 0, sizeof(MAC_TABLE_ENTRY));
				pEntry->PairwiseKey.KeyLen = 0;
				pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
			}

			do
			{
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
					if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
					{
						SET_ENTRY_APCLI(pEntry);
						break;
					}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{	/* be a regular-entry*/
						if ((apidx < pAd->ApCfg.BssidNum) &&
							(apidx < MAX_MBSSID_NUM(pAd)) &&
							((apidx < HW_BEACON_MAX_NUM)) &&
							(pAd->ApCfg.MBSSID[apidx].MaxStaNum != 0) &&
							(pAd->ApCfg.MBSSID[apidx].StaCount >= pAd->ApCfg.MBSSID[apidx].MaxStaNum))
						{
							DBGPRINT(RT_DEBUG_WARN, ("%s: The connection table is full in ra%d.\n", __FUNCTION__, apidx));
							NdisReleaseSpinLock(&pAd->MacTabLock);
							return NULL;
						}
					}
#endif /* CONFIG_AP_SUPPORT */
					SET_ENTRY_CLIENT(pEntry);

		} while (false);

			pEntry->wdev = wdev;
			pEntry->wcid = i;
			pEntry->isCached = false;
			pEntry->bIAmBadAtheros = false;

			RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, false);

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				if (IS_ENTRY_CLIENT(pEntry)) /* Only Client entry need the retry timer.*/
				{
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, false);
#ifdef DOT11W_PMF_SUPPORT
					RTMPInitTimer(pAd, &pEntry->SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, false);
					RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, false);
#endif /* DOT11W_PMF_SUPPORT */

	/*				RTMP_OS_Init_Timer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pAd);*/
				}
#ifdef APCLI_SUPPORT
				else if (IS_ENTRY_APCLI(pEntry))
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, false);
#endif /* APCLI_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */

			if (pAd->chipCap.FlgHwTxBfCap)
				RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, false);

			pEntry->pAd = pAd;
			pEntry->CMTimerRunning = false;
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			pEntry->RSNIE_Len = 0;
			memset(pEntry->R_Counter, 0, sizeof(pEntry->R_Counter));
			pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

			if (IS_ENTRY_MESH(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_MESH);
#ifdef CONFIG_AP_SUPPORT
			else if (IS_ENTRY_APCLI(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_APCLI);
			else if (IS_ENTRY_WDS(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_WDS);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			else
				pEntry->apidx = apidx;

#ifdef CONFIG_AP_SUPPORT
			if ((apidx < pAd->ApCfg.BssidNum) &&
				(apidx < MAX_MBSSID_NUM(pAd)) &&
				(apidx < HW_BEACON_MAX_NUM))
				pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];
			else
				pEntry->pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */

			do
			{

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
					if (IS_ENTRY_APCLI(pEntry))
					{
						pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->apidx].wdev.AuthMode;
						pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->apidx].wdev.WepStatus;
						pEntry->wdev_idx = pEntry->apidx;
						if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
						{
							pEntry->WpaState = AS_NOTUSE;
							pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
						}
						else
						{
							pEntry->WpaState = AS_PTKSTART;
							pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
						}
						break;
					}
#endif /* APCLI_SUPPORT */
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						MBSS_MR_APIDX_SANITY_CHECK(pAd, apidx);
						pEntry->AuthMode = pAd->ApCfg.MBSSID[apidx].wdev.AuthMode;
						pEntry->WepStatus = pAd->ApCfg.MBSSID[apidx].wdev.WepStatus;
						pEntry->GroupKeyWepStatus = pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus;
						pEntry->wdev_idx = pEntry->apidx;

						if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
							pEntry->WpaState = AS_NOTUSE;
						else
							pEntry->WpaState = AS_INITIALIZE;

						pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
						pEntry->StaIdleTimeout = pAd->ApCfg.StaIdleTimeout;
						pAd->ApCfg.MBSSID[apidx].StaCount++;
						pAd->ApCfg.EntryClientCount++;
						break;
					}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					pEntry->AuthMode = pAd->StaCfg.wdev.AuthMode;
					pEntry->WepStatus = pAd->StaCfg.wdev.WepStatus;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}
#endif /* CONFIG_STA_SUPPORT */
			} while (false);

			pEntry->GTKState = REKEY_NEGOTIATING;
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
				pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

			pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
			COPY_MAC_ADDR(pEntry->Addr, pAddr);

			if (IS_ENTRY_TDLS(pEntry))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("#########SET_ENTRY_TDLS pEntry->mac %02x:%02x:%02x:%02x:%02x:%02x \n",PRINT_MAC(pEntry->Addr))); //Kyle Debug
			}

			do
			{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
				if (IS_ENTRY_APCLI(pEntry))
				{
					COPY_MAC_ADDR(pEntry->bssid, pAddr);
					break;
				}
#endif // APCLI_SUPPORT //
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
				if (OpMode == OPMODE_AP)
				{
					COPY_MAC_ADDR(pEntry->bssid, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
					break;
				}
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
				if (OpMode == OPMODE_STA)
				{
					COPY_MAC_ADDR(pEntry->bssid, pAddr);
					break;
				}
#endif // CONFIG_STA_SUPPORT //
			} while (false);

			pEntry->Sst = SST_NOT_AUTH;
			pEntry->AuthState = AS_NOT_AUTH;
			pEntry->Aid = (USHORT)i;
			pEntry->CapabilityInfo = 0;
			pEntry->PsMode = PWR_ACTIVE;
			pEntry->PsQIdleCount = 0;
			pEntry->NoDataIdleCount = 0;
			pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
			pEntry->ContinueTxFailCnt = 0;
			pEntry->StatTxRetryOkCount = 0;
			pEntry->StatTxFailCount = 0;
			pEntry->TimeStamp_toTxRing = 0;
			InitializeQueueHeader(&pEntry->PsQueue);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
			}
#endif /* CONFIG_AP_SUPPORT */

			pAd->MacTab.Size ++;

			/* Set the security mode of this entry as OPEN-NONE in ASIC */
			RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, (u8)i);

			/* Add this entry into ASIC RX WCID search table */
			RTMP_STA_ENTRY_ADD(pAd, pEntry);

#ifdef PEER_DELBA_TX_ADAPT
			Peer_DelBA_Tx_Adapt_Init(pAd, pEntry);
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef DROP_MASK_SUPPORT
			drop_mask_init_per_client(pAd, pEntry);
#endif /* DROP_MASK_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
			}
#endif /* CONFIG_AP_SUPPORT */

			if (pAd->chipCap.FlgHwTxBfCap)
				spin_lock_init(&pEntry->TxSndgLock);

			DBGPRINT(RT_DEBUG_TRACE, ("%s(): alloc entry #%d, Total= %d\n",
						__FUNCTION__, i, pAd->MacTab.Size));
			break;
		}
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{

		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
			pAd->MacTab.Hash[HashIdx] = pEntry;
		else
		{
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{


		}
#endif /* CONFIG_AP_SUPPORT */


	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MULTI_CLIENT_SUPPORT
	if (pAd->MacTab.Size < MAX_LEN_OF_MAC_TABLE)
	{
		USHORT size;

		size = pAd->ApCfg.EntryClientCount;
		asic_change_tx_retry(pAd, size);

		if (IS_RT6352(pAd))
		{
			pkt_aggr_num_change(pAd, size);

			if (pAd->CommonCfg.bWmm)
				asic_tune_be_wmm(pAd, size);
		}
	}
#endif /* MULTI_CLIENT_SUPPORT */
#endif // CONFIG_AP_SUPPORT //

	NdisReleaseSpinLock(&pAd->MacTabLock);
	return pEntry;
}


/*
	==========================================================================
	Description:
		Delete a specified client from MAC table
	==========================================================================
 */
bool MacTableDeleteEntry(struct rtmp_adapter *pAd, USHORT wcid, u8 *pAddr)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	bool Cancelled;

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return false;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->MacTab.Content[wcid];

	if (pEntry && !IS_ENTRY_NONE(pEntry))
	{
		/* ENTRY PREEMPTION: Cancel all timers */
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
		RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
                RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
                RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_DOT11V_WNM)
			if (pAd->ApCfg.MBSSID[pEntry->apidx].WNMCtrl.ProxyARPEnable)
			{
				RemoveIPv4ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->apidx], pEntry->Addr);
				RemoveIPv6ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->apidx], pEntry->Addr);
			}
#endif


			/* free resources of BA*/
			BASessionTearDownALL(pAd, pEntry->wcid);

			/* Delete this entry from ASIC on-chip WCID Table*/
			RTMP_STA_ENTRY_MAC_RESET(pAd, wcid);

			if (pAd->chipCap.FlgHwTxBfCap)
				RTMPReleaseTimer(&pEntry->eTxBfProbeTimer, &Cancelled);


#ifdef DOT11W_PMF_SUPPORT
			RTMPReleaseTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPReleaseTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry)
			)
			{
#ifdef DOT1X_SUPPORT
				INT PmkCacheIdx = -1;
#endif /* DOT1X_SUPPORT */

				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);


#ifdef DOT1X_SUPPORT
				/* Notify 802.1x daemon to clear this sta info*/
				if (pEntry->AuthMode == Ndis802_11AuthModeWPA ||
					pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
					pAd->ApCfg.MBSSID[pEntry->apidx].wdev.IEEE8021X)
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

				/* Delete the PMK cache for this entry if it exists.*/
				if ((PmkCacheIdx = RTMPSearchPMKIDCache(pAd, pEntry->apidx, pEntry->Addr)) != -1)
		    		{
					RTMPDeletePMKIDCache(pAd, pEntry->apidx, PmkCacheIdx);
				}
#endif /* DOT1X_SUPPORT */


				pAd->ApCfg.MBSSID[pEntry->apidx].StaCount--;
				pAd->ApCfg.EntryClientCount--;

#ifdef HOSTAPD_SUPPORT
				if(pEntry && pAd->ApCfg.MBSSID[pEntry->apidx].Hostapd == Hostapd_EXT )
				{
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED, -1, pEntry->Addr,
						NULL, 0,pEntry->apidx);
				}
#endif /* HOSTAPD_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
				CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr);
#endif /*RT_CFG80211_SUPPORT*/
			}
#ifdef APCLI_SUPPORT
			else if (IS_ENTRY_APCLI(pEntry))
			{
				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);
			}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			pPrevEntry = NULL;
			pProbeEntry = pAd->MacTab.Hash[HashIdx];
			ASSERT(pProbeEntry);
			if (pProbeEntry != NULL)
			{
				/* update Hash list*/
				do
				{
					if (pProbeEntry == pEntry)
					{
						if (pPrevEntry == NULL)
							pAd->MacTab.Hash[HashIdx] = pEntry->pNext;
						else
							pPrevEntry->pNext = pEntry->pNext;
						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);
			}

			/* not found !!!*/
			ASSERT(pProbeEntry != NULL);

#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &pEntry->PsQueue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */
			/*RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, wcid);*/


		if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
		{
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		}
		RTMPReleaseTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);


#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#ifdef DROP_MASK_SUPPORT
			drop_mask_release_per_client(pAd, pEntry);
#endif /* DROP_MASK_SUPPORT */


//   			memset(pEntry, 0, sizeof(MAC_TABLE_ENTRY));
			memset(pEntry->Addr, 0, MAC_ADDR_LEN);
			/* invalidate the entry */
			SET_ENTRY_NONE(pEntry);
			pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

			pAd->MacTab.Size--;

			DBGPRINT(RT_DEBUG_TRACE, ("MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size));
		}
		else
		{
			DBGPRINT(RT_DEBUG_OFF, ("\n%s: Impossible Wcid = %d !!!!!\n", __FUNCTION__, wcid));
		}
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);

	/*Reset operating mode when no Sta.*/
	if (pAd->MacTab.Size == 0)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
		RTMP_UPDATE_PROTECT(pAd, 0, ALLN_SETPROTECT, true, 0);
	}

#ifdef CONFIG_AP_SUPPORT
	/*APUpdateCapabilityAndErpIe(pAd);*/
	RTMP_AP_UPDATE_CAPABILITY_AND_ERPIE(pAd);  /* edit by johnli, fix "in_interrupt" error when call "MacTableDeleteEntry" in Rx tasklet*/
#endif /* CONFIG_AP_SUPPORT */

	return true;
}


/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID MacTableReset(struct rtmp_adapter *pAd)
{
	int i;
	bool Cancelled;
#ifdef CONFIG_AP_SUPPORT
	u8 *pOutBuffer = NULL;
	int NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	u8 apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/


	for (i=1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pMacEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pMacEntry))
		{
			RTMPReleaseTimer(&pMacEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC)
				{
					/*  send out a De-authentication request frame*/
					pOutBuffer = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);
					if (pOutBuffer == NULL) {
						DBGPRINT(RT_DEBUG_TRACE, (" kmalloc fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					DBGPRINT(RT_DEBUG_WARN, ("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
								Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
										pAd->ApCfg.MBSSID[pMacEntry->apidx].wdev.if_addr,
										pAd->ApCfg.MBSSID[pMacEntry->apidx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
										sizeof(HEADER_802_11), &DeAuthHdr,
										2, &Reason,
										END_OF_ARGS);

					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					kfree(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			pAd->ApCfg.MBSSID[apidx].StaCount = 0;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));
		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);
		DBGPRINT(RT_DEBUG_TRACE, ("2McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));

		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
/*		memset(&pAd->MacTab, 0, sizeof(MAC_TABLE));*/
		memset(&pAd->MacTab.Size, 0,
							sizeof(MAC_TABLE)-
							sizeof(pAd->MacTab.Hash)-
							sizeof(pAd->MacTab.Content));

		InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
		/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}



