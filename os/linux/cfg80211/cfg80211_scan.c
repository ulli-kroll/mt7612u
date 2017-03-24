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

	All related CFG80211 Scan function body.

	History:

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

#ifdef CONFIG_STA_SUPPORT
VOID CFG80211DRV_OpsScanInLinkDownAction(struct rtmp_adapter *pAd)
{
	bool Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("---> CFG80211_MLME Disconnect in Scaning, ORI ==> %d\n",
	  								(int) pAd->Mlme.CntlMachine.CurrState));

	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
	pAd->MlmeAux.Channel = 0;

	pAd->cfg80211_ctrl.FlgCfg80211Scanning = false;
	CFG80211OS_ScanEnd(pAd->pCfg80211_CB, true);

	ScanNextChannel(pAd, OPMODE_STA);
	DBGPRINT(RT_DEBUG_TRACE, ("<--- CFG80211_MLME Disconnect in Scan END, ORI ==> %d\n",
									(int) pAd->Mlme.CntlMachine.CurrState));
}

bool CFG80211DRV_OpsScanRunning(struct rtmp_adapter *pAd)
{
	return pAd->cfg80211_ctrl.FlgCfg80211Scanning;
}
#endif /*CONFIG_STA_SUPPORT*/


/* Refine on 2013/04/30 for two functin into one */
INT CFG80211DRV_OpsScanGetNextChannel(
	struct rtmp_adapter				*pAd)
{
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (cfg80211_ctrl->pCfg80211ChanList != NULL)
	{
		if (cfg80211_ctrl->Cfg80211CurChanIndex < cfg80211_ctrl->Cfg80211ChanListLen)
		{
			return cfg80211_ctrl->pCfg80211ChanList[cfg80211_ctrl->Cfg80211CurChanIndex++];
		}
		else
		{
            kfree(cfg80211_ctrl->pCfg80211ChanList);
            cfg80211_ctrl->pCfg80211ChanList = NULL;
            cfg80211_ctrl->Cfg80211ChanListLen = 0;
			cfg80211_ctrl->Cfg80211CurChanIndex = 0;

			return 0;
		}
	}

	return 0;
}

bool CFG80211DRV_OpsScanSetSpecifyChannel(
	struct rtmp_adapter				*pAd,
	VOID						*pData,
	UINT8						 dataLen)
{
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	uint32_t *pChanList = (uint32_t *) pData;

	if (pChanList != NULL)
	{
		if (cfg80211_ctrl->pCfg80211ChanList != NULL)
			kfree(cfg80211_ctrl->pCfg80211ChanList);

		cfg80211_ctrl->pCfg80211ChanList =
			kmalloc(sizeof(uint32_t *) * dataLen, GFP_ATOMIC);
		if (cfg80211_ctrl->pCfg80211ChanList != NULL) {
			memcpy(cfg80211_ctrl->pCfg80211ChanList, pChanList, sizeof(uint32_t *) * dataLen);
			cfg80211_ctrl->Cfg80211ChanListLen = dataLen;
			cfg80211_ctrl->Cfg80211CurChanIndex = 0 ; /* Start from index 0 */
			return NDIS_STATUS_SUCCESS;
		}
		else
		{
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_FAILURE;
}

bool CFG80211DRV_OpsScanCheckStatus(struct rtmp_adapter *pAd,
	UINT8						 IfType)
{
#ifdef CONFIG_STA_SUPPORT
 	/* CFG_TODO */
	if (CFG80211DRV_OpsScanRunning(pAd))
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("SCAN_FAIL: CFG80211 Internal SCAN Flag On\n"));
		return false;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("SCAN_FAIL: BSS_SCAN_IN_PROGRESS\n"));
		return false;
	}

	/* To avoid the scan cmd come-in during driver init */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SCAN_FAIL: Scan cmd before Startup finish\n"));
		return false;
	}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) &&
	    (pAd->cfg80211_ctrl.FlgCfg80211Connecting == true) &&
        (IfType == RT_CMD_80211_IFTYPE_STATION))
	{
		DBGPRINT(RT_DEBUG_ERROR,("SCAN_FAIL: P2P_CLIENT In Connecting & Canncel Scan with Infra Side\n"));
		return false;
	}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_SUPPORT
	if (pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan == true &&
        (IfType == RT_CMD_80211_IFTYPE_AP))
	{
		DBGPRINT(RT_DEBUG_ERROR,("Disable 20/40 scan!!\n"));
		return false;
	}
#endif /* RT_CFG80211_SUPPORT */

	/* do scan */
	pAd->cfg80211_ctrl.FlgCfg80211Scanning = true;
#endif /*CONFIG_STA_SUPPORT*/
	return true;
}

bool CFG80211DRV_OpsScanExtraIesSet(struct rtmp_adapter *pAd)
{
	CFG80211_CB *pCfg80211_CB = pAd->pCfg80211_CB;
	UINT ie_len = 0;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_CB->pCfg80211_ScanReq)
		ie_len = pCfg80211_CB->pCfg80211_ScanReq->ie_len;

    CFG80211DBG(RT_DEBUG_INFO, ("80211> CFG80211DRV_OpsExtraIesSet ==> %d\n", ie_len));
#ifdef CONFIG_STA_SUPPORT
	CFG80211DBG(RT_DEBUG_INFO, ("80211> is_wpa_supplicant_up ==> %d\n",
									pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP));
#endif /*CONFIG_STA_SUPPORT*/
	if (ie_len == 0)
		return false;

	/* Reset the ExtraIe and Len */
	if (cfg80211_ctrl->pExtraIe)
	{
		kfree(cfg80211_ctrl->pExtraIe);
		cfg80211_ctrl->pExtraIe = NULL;
	}
	cfg80211_ctrl->ExtraIeLen = 0;

	cfg80211_ctrl->pExtraIe = kmalloc(ie_len, GFP_ATOMIC);
	if (cfg80211_ctrl->pExtraIe)
	{
		memcpy(cfg80211_ctrl->pExtraIe, pCfg80211_CB->pCfg80211_ScanReq->ie, ie_len);
		cfg80211_ctrl->ExtraIeLen = ie_len;
		hex_dump("CFG8021_SCAN_EXTRAIE", cfg80211_ctrl->pExtraIe, cfg80211_ctrl->ExtraIeLen);
	}
	else
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211DRV_OpsExtraIesSet ==> allocate fail. \n"));
		return false;
	}

	return true;
}

#ifdef CFG80211_SCAN_SIGNAL_AVG
static void CFG80211_CalBssAvgRssi(
	IN      BSS_ENTRY 				*pBssEntry)
{
        bool bInitial = false;

        if (!(pBssEntry->AvgRssiX8 | pBssEntry->AvgRssi))
        {
                bInitial = true;
        }

        if (bInitial)
        {
                pBssEntry->AvgRssiX8 = pBssEntry->Rssi << 3;
                pBssEntry->AvgRssi  = pBssEntry->Rssi;
        }
        else
        {
        		/* For smooth purpose, oldRssi for 7/8, newRssi for 1/8 */
                pBssEntry->AvgRssiX8 =
					(pBssEntry->AvgRssiX8 - pBssEntry->AvgRssi) + pBssEntry->Rssi;
        }

        pBssEntry->AvgRssi = pBssEntry->AvgRssiX8 >> 3;

}

static void CFG80211_UpdateBssTableRssi(
	IN struct rtmp_adapter			*pAd)
{

	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct ieee80211_channel *chan;
	struct cfg80211_bss *bss;
	BSS_ENTRY *pBssEntry;
	UINT index;
	uint32_t CenFreq;

	for (index = 0; index < pAd->ScanTab.BssNr; index++)
	{
		pBssEntry = &pAd->ScanTab.BssEntry[index];

		if (pAd->ScanTab.BssEntry[index].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel , NL80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel , NL80211_BAND_2GHZ);

		chan = ieee80211_get_channel(pWiphy, CenFreq);
		bss = cfg80211_get_bss(pWiphy, chan, pBssEntry->Bssid, pBssEntry->Ssid, pBssEntry->SsidLen,
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
		if (bss == NULL)
		{
			/* ScanTable Entry not exist in kernel buffer */
		}
		else
		{
			/* HIT */
			CFG80211_CalBssAvgRssi(pBssEntry);
			bss->signal = pBssEntry->AvgRssi * 100; //UNIT: MdBm
			cfg80211_put_bss(pWiphy, bss);
		}
	}
}
#endif /* CFG80211_SCAN_SIGNAL_AVG */

/*
========================================================================
Routine Description:
	Inform us that a scan is got.

Arguments:
	pAdCB				- WLAN control block pointer

Return Value:
	NONE

Note:
	Call RT_CFG80211_SCANNING_INFORM, not CFG80211_Scaning
========================================================================
*/
VOID CFG80211_Scaning(
	IN struct rtmp_adapter					*pAd,
	IN uint32_t 					BssIdx,
	IN uint32_t 					ChanId,
	IN UCHAR						*pFrame,
	IN uint32_t 					FrameLen,
	IN int32_t 					RSSI)
{
#ifdef CONFIG_STA_SUPPORT
	VOID *pCfg80211_CB = pAd->pCfg80211_CB;
	bool FlgIsNMode;
	UINT8 BW;

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Network is down!\n"));
		return;
	}

	/*
		In connect function, we also need to report BSS information to cfg80211;
		Not only scan function.
	*/
	if ((!CFG80211DRV_OpsScanRunning(pAd)) &&
		(pAd->cfg80211_ctrl.FlgCfg80211Connecting == false))
	{
		return; /* no scan is running from wpa_supplicant */
	}


	/* init */
	/* Note: Can not use local variable to do pChan */
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		FlgIsNMode = true;
	else
		FlgIsNMode = false;

	if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20)
		BW = 0;
	else
		BW = 1;

	CFG80211OS_Scaning(pCfg80211_CB,
						ChanId,
						pFrame,
						FrameLen,
						RSSI,
						FlgIsNMode,
						BW);
#endif /* CONFIG_STA_SUPPORT */
}


/*
========================================================================
Routine Description:
	Inform us that scan ends.

Arguments:
	pAdCB			- WLAN control block pointer
	FlgIsAborted	- 1: scan is aborted

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_ScanEnd(
	IN struct rtmp_adapter				*pAd,
	IN bool					FlgIsAborted)
{
#ifdef CONFIG_STA_SUPPORT
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Network is down!\n"));
		return;
	}

	if (!CFG80211DRV_OpsScanRunning(pAd))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("80211> No scan is running!\n"));
		return; /* no scan is running */
	}

	if (FlgIsAborted == true)
		FlgIsAborted = 1;
	else
	{
		FlgIsAborted = 0;
#ifdef CFG80211_SCAN_SIGNAL_AVG
		CFG80211_UpdateBssTableRssi(pAd);
#endif /* CFG80211_SCAN_SIGNAL_AVG */
	}

	CFG80211OS_ScanEnd(CFG80211CB, FlgIsAborted);
	pAd->cfg80211_ctrl.FlgCfg80211Scanning = false;
#endif /* CONFIG_STA_SUPPORT */
}

VOID CFG80211_ScanStatusLockInit(
	IN struct rtmp_adapter	*pAd,
	IN UINT                      init)
{
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;

	if (init)
	{
		spin_lock_init(&pCfg80211_CB->scan_notify_lock);
	}
	else
	{
	}
}

#endif /* RT_CFG80211_SUPPORT */

