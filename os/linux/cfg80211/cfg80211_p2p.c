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

	All related CFG80211 P2P function body.

	History:

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

UCHAR CFG_WPS_OUI[4] = {0x00, 0x50, 0xf2, 0x04};
UCHAR CFG_P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9}; /* spec. 1.14 OUI */

BUILD_TIMER_FUNCTION(CFG80211RemainOnChannelTimeout);

static
VOID CFG80211_RemainOnChannelInit(struct rtmp_adapter  *pAd)
{
	if (pAd->cfg80211_ctrl.Cfg80211RocTimerInit == FALSE)
	{
		CFG80211DBG(RT_DEBUG_TRACE, ("CFG80211_ROC : INIT Cfg80211RocTimer\n"));
		RTMPInitTimer(pAd, &pAd->cfg80211_ctrl.Cfg80211RocTimer,
			GET_TIMER_FUNCTION(CFG80211RemainOnChannelTimeout), pAd, FALSE);
		pAd->cfg80211_ctrl.Cfg80211RocTimerInit = true;
	}
}

VOID CFG80211RemainOnChannelTimeout(
	PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *) FunctionContext;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	DBGPRINT(RT_DEBUG_INFO, ("CFG80211_ROC: RemainOnChannelTimeout\n"));

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define RESTORE_COM_CH_TIME 100
	APCLI_STRUCT *pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];

	if (pApCliEntry->Valid &&
	     	RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) &&
            	(pAd->LatchRfRegs.Channel != pApCliEntry->MlmeAux.Channel))
	{
		/* Extend the ROC_TIME for Common Channel When P2P_CLI on */
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC: ROC_Timeout APCLI_ON Channel: %d\n",
								pApCliEntry->MlmeAux.Channel));

        	AsicSwitchChannel(pAd, pApCliEntry->MlmeAux.Channel, FALSE);
        	AsicLockChannel(pAd, pApCliEntry->MlmeAux.Channel);

		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: P2P_CLI PWR_ACTIVE ROC_END\n"));
		CFG80211_P2pClientSendNullFrame(pAd, PWR_ACTIVE);
#ifdef CONFIG_STA_SUPPORT
		if (INFRA_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: CONCURRENT STA PWR_ACTIVE ROC_END\n"));
			RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate,
					  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? true:FALSE),
					  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
		}
#endif /*CONFIG_STA_SUPPORT*/
		RTMPSetTimer(&pCfg80211_ctrl->Cfg80211RocTimer, RESTORE_COM_CH_TIME);
	}
	else if (INFRA_ON(pAd) &&
	   	     (pAd->LatchRfRegs.Channel != pAd->CommonCfg.Channel))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC: ROC_Timeout INFRA_ON Channel: %d\n",
									pAd->CommonCfg.Channel));

        	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
        	AsicLockChannel(pAd, pAd->CommonCfg.Channel);
#ifdef CONFIG_STA_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: INFRA_ON PWR_ACTIVE ROC_END\n"));
		RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate,
				  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? true:FALSE),
				  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
#endif /*CONFIG_STA_SUPPORT*/
		RTMPSetTimer(&pCfg80211_ctrl->Cfg80211RocTimer, RESTORE_COM_CH_TIME);
	}
	else
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
		struct net_device *ndev = CFG80211_GetEventDevice(pAd);

		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC: RemainOnChannelTimeout -- FINISH\n"));

        	cfg80211_remain_on_channel_expired( ndev->ieee80211_ptr,
        		pCfg80211_ctrl->Cfg80211ChanInfo.cookie, pCfg80211_ctrl->Cfg80211ChanInfo.chan,
        		GFP_KERNEL);

		pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	}

}

/* Set a given time on specific channel to listen action Frame */
bool CFG80211DRV_OpsRemainOnChannel(struct rtmp_adapter *pAd, VOID *pData, uint32_t duration)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pChanInfo = (CMD_RTPRIV_IOCTL_80211_CHAN *) pData;
	bool Cancelled;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	UCHAR lock_channel;
        PWIRELESS_DEV pwdev = NULL;
        pwdev = pChanInfo->pWdev;

	CFG80211DBG(RT_DEBUG_INFO, ("%s\n", __FUNCTION__));

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	APCLI_STRUCT *pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
	/* Will be Exit the ApCli Connected Channel so send Null frame on current */
	if (pApCliEntry->Valid &&
	    RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) &&
	        (pApCliEntry->MlmeAux.Channel != pChanInfo->ChanId) &&
                (pApCliEntry->MlmeAux.Channel == pAd->LatchRfRegs.Channel))
	{
        	DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: APCLI PWR_SAVE ROC_START\n"));
        	CFG80211_P2pClientSendNullFrame(pAd, PWR_SAVE);
	}

	if (INFRA_ON(pAd) &&
	       (pAd->CommonCfg.Channel != pChanInfo->ChanId) &&
               (pAd->CommonCfg.Channel == pAd->LatchRfRegs.Channel))
	{
    		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: STA PWR_SAVE ROC_START\n"));
		RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate,
				  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? true:FALSE),
				  PWR_SAVE);
	}
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE */

	/* Channel Switch Case:
	 * 1. P2P_FIND:    [SOCIAL_CH]->[COM_CH]->[ROC_CH]--N_TUs->[ROC_TIMEOUT]
	 *                 Set COM_CH to ROC_CH for merge COM_CH & ROC_CH dwell section.
     	 *
	 * 2. OFF_CH_WAIT: [ROC_CH]--200ms-->[ROC_TIMEOUT]->[COM_CH]
	 *                 Most in GO case.
	 *
	 */
	//lock_channel = CFG80211_getCenCh(pAd, pChanInfo->ChanId);
	lock_channel = pChanInfo->ChanId;
	if (pAd->LatchRfRegs.Channel != lock_channel)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_PKT: ROC CHANNEL_LOCK %d\n", pChanInfo->ChanId));
		//AsicSetChannel(pAd, lock_channel, BW_20, EXTCHA_NONE, FALSE);

		AsicSwitchChannel(pAd, lock_channel, FALSE);
		AsicLockChannel(pAd, lock_channel);
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO, ("80211> ComCH == ROC_CH \n"));
	}
        cfg80211_ready_on_channel(pwdev,  pChanInfo->cookie, pChanInfo->chan, duration, GFP_ATOMIC);

	memcpy(&pCfg80211_ctrl->Cfg80211ChanInfo, pChanInfo, sizeof(CMD_RTPRIV_IOCTL_80211_CHAN));

	CFG80211_RemainOnChannelInit(pAd);

	if (pCfg80211_ctrl->Cfg80211RocTimerRunning == true)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC : CANCEL Cfg80211RocTimer\n"));
		RTMPCancelTimer(&pCfg80211_ctrl->Cfg80211RocTimer, &Cancelled);
		pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	}

	RTMPSetTimer(&pCfg80211_ctrl->Cfg80211RocTimer, duration + 20);
	pCfg80211_ctrl->Cfg80211RocTimerRunning = true;

	return true;
}

void CFG80211DRV_OpsCancelRemainOnChannel(struct rtmp_adapter *pAd, uint32_t cookie)
{
	bool Cancelled;
	CFG80211DBG(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	if (pAd->cfg80211_ctrl.Cfg80211RocTimerRunning == true)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG_ROC : CANCEL Cfg80211RocTimer\n"));
		RTMPCancelTimer(&pAd->cfg80211_ctrl.Cfg80211RocTimer, &Cancelled);
		pAd->cfg80211_ctrl.Cfg80211RocTimerRunning = FALSE;
	}
}

#endif /* RT_CFG80211_SUPPORT */

