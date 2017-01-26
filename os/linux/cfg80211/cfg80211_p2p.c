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
		pAd->cfg80211_ctrl.Cfg80211RocTimerInit = TRUE;
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
					  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
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
				  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
				  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
#endif /*CONFIG_STA_SUPPORT*/
		RTMPSetTimer(&pCfg80211_ctrl->Cfg80211RocTimer, RESTORE_COM_CH_TIME);
	}
	else
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC: RemainOnChannelTimeout -- FINISH\n"));

        	cfg80211_remain_on_channel_expired( CFG80211_GetEventDevice(pAd),
        		pCfg80211_ctrl->Cfg80211ChanInfo.cookie, pCfg80211_ctrl->Cfg80211ChanInfo.chan,
        		GFP_KERNEL);

		pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	}

}

/* Set a given time on specific channel to listen action Frame */
BOOLEAN CFG80211DRV_OpsRemainOnChannel(struct rtmp_adapter *pAd, VOID *pData, uint32_t duration)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pChanInfo = (CMD_RTPRIV_IOCTL_80211_CHAN *) pData;
	BOOLEAN Cancelled;
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
				  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
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

	if (pCfg80211_ctrl->Cfg80211RocTimerRunning == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC : CANCEL Cfg80211RocTimer\n"));
		RTMPCancelTimer(&pCfg80211_ctrl->Cfg80211RocTimer, &Cancelled);
		pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	}

	RTMPSetTimer(&pCfg80211_ctrl->Cfg80211RocTimer, duration + 20);
	pCfg80211_ctrl->Cfg80211RocTimerRunning = TRUE;

	return TRUE;
}

BOOLEAN CFG80211DRV_OpsCancelRemainOnChannel(struct rtmp_adapter *pAd, uint32_t cookie)
{
	BOOLEAN Cancelled;
	CFG80211DBG(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	if (pAd->cfg80211_ctrl.Cfg80211RocTimerRunning == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG_ROC : CANCEL Cfg80211RocTimer\n"));
		RTMPCancelTimer(&pAd->cfg80211_ctrl.Cfg80211RocTimer, &Cancelled);
		pAd->cfg80211_ctrl.Cfg80211RocTimerRunning = FALSE;
	}
}

INT CFG80211_setPowerMgmt(struct rtmp_adapter *pAd, UINT Enable)
{
	DBGPRINT(RT_DEBUG_TRACE, ("@@@ %s: %d\n", __FUNCTION__, Enable));

#ifdef RT_CFG80211_P2P_SUPPORT
	pAd->cfg80211_ctrl.bP2pCliPmEnable = Enable;
#endif /* RT_CFG80211_P2P_SUPPORT */

	return 0;
}

#ifdef RT_CFG80211_P2P_SUPPORT
/*
	==========================================================================
	Description:
		Make a P2P Fake NoA Attribute to trigger myself to restart NoA.
		The Start time is changed. Duration and Interval and Count is
		the same as GO's beacon

	Parameters:
		 StartTime : A new Start time.
		 pOutBuffer : pointer to buffer that should put data to.
	Note:

	==========================================================================
 */
VOID CFG80211_P2PMakeFakeNoATlv(struct rtmp_adapter *pAd, ULONG StartTime, u8 *pOutBuffer)
{
	u8 *pDest;
	PP2PCLIENT_NOA_SCHEDULE pNoa = &pAd->cfg80211_ctrl.GONoASchedule;
	pDest = pOutBuffer;

	*(pDest) = SUBID_P2P_NOA;
	/* Length is 13*n + 2 = 15 when n = 1 */
	*(pDest+1) = 15;
	/* Lenght 2nd byte */
	*(pDest+2) = 0;
	/* Index. */
	*(pDest+3) = pNoa->Token;
	/* CT Windows and OppPS parm. Don't turn on both. So Set CTWindows = 0 */
	*(pDest+4) = 0;
	/* Count.  Test Plan set to 255. */
	*(pDest+5) = pNoa->Count;
	/* Duration */
	RTMPMoveMemory((pDest+6), pNoa->Duration, 4);
	/* Interval */
	RTMPMoveMemory((pDest+10), pNoa->Interval, 4);
	RTMPMoveMemory((pDest+14), &StartTime, 4);
}


BOOLEAN	CFG80211_P2pAdjustSwNoATimer(struct rtmp_adapter *pAd, ULONG CurrentTimeStamp, ULONG NextTimePoint)
{
	PCFG80211_CTRL pP2PCtrl = &pAd->cfg80211_ctrl;
	ULONG AwakeDuration, NewStartTime;
	UCHAR FakeNoAAttribute[32];

	RTMPZeroMemory(FakeNoAAttribute, 32);
	AwakeDuration = pP2PCtrl->GONoASchedule.Interval - pP2PCtrl->GONoASchedule.Duration;
	if (CurrentTimeStamp < pP2PCtrl->GONoASchedule.CurrentTargetTimePoint)
	{
		/* If offset is more than 1/4 of duration. */
		if ((pP2PCtrl->GONoASchedule.OngoingAwakeTime) >= (AwakeDuration>> 2))
		{
			DBGPRINT(RT_DEBUG_TRACE,("P2pAdjustSwNoATimer HERE HERE!!!! \n"));
			DBGPRINT(RT_DEBUG_TRACE,("OngoingAwakeTime = %ld. CurrentTimeStamp = %ld.!!!! \n",
							pP2PCtrl->GONoASchedule.OngoingAwakeTime, CurrentTimeStamp));

			CFG80211_P2pStopNoA(pAd, &pAd->MacTab.Content[pP2PCtrl->MyGOwcid]);
			FakeNoAAttribute[0] = SUBID_P2P_NOA;
			NewStartTime = pP2PCtrl->GONoASchedule.StartTime +
				       (pP2PCtrl->GONoASchedule.SwTimerTickCounter - 1) * (pP2PCtrl->GONoASchedule.Interval);

			CFG80211_P2PMakeFakeNoATlv(pAd, NewStartTime, &FakeNoAAttribute[0]);

			pAd->MacTab.Content[pP2PCtrl->MyGOwcid].CFGP2pInfo.NoADesc[0].Token--;

			CFG80211_P2pHandleNoAAttri(pAd, &pAd->MacTab.Content[pP2PCtrl->MyGOwcid], &FakeNoAAttribute[0]);
		}

		/* Update expected next Current Target Time Point with NextTimePoint */
		pP2PCtrl->GONoASchedule.CurrentTargetTimePoint = NextTimePoint;
		/* Can immediately dequeue packet because peer already in awake period. */
		return TRUE;
	}
	else
	{
		/* Update expected next Current Target Time Point with NextTimePoint */
		pP2PCtrl->GONoASchedule.CurrentTargetTimePoint = NextTimePoint;
		return FALSE;
	}
}

VOID CFG80211_P2pGPTimeOutHandle(struct rtmp_adapter *pAd)
{
	PCFG80211_CTRL pP2PCtrl = &pAd->cfg80211_ctrl;
	MAC_TABLE_ENTRY *pEntry=NULL;
	ULONG MacValue, Value, GPDiff, NextDiff, SavedNextTargetTimePoint;

	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &= 0xfffffffd;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);

	/* GO operating or Autonomous GO */
	if (CFG_P2PGO_ON(pAd))
	{
		/* Not Yet Ready */
	}
	else if (CFG_P2PCLI_ON(pAd))
	{
		if (pP2PCtrl->NoAIndex >= MAX_LEN_OF_MAC_TABLE)
			return;

		if (pP2PCtrl->NoAIndex != pP2PCtrl->MyGOwcid)
			DBGPRINT(RT_DEBUG_TRACE,("%s: !bug, please check driver %d. \n",
				__FUNCTION__, pP2PCtrl->NoAIndex));

		pEntry = &pAd->MacTab.Content[pP2PCtrl->NoAIndex];
		if (pEntry && pEntry->CFGP2pInfo.NoADesc[0].bValid == TRUE)
		{
			if ((pEntry->CFGP2pInfo.NoADesc[0].Count > 0) && (pEntry->CFGP2pInfo.NoADesc[0].Count < 255))
			{
				/*
					Sometimes go to awake, sometime go to silence. Two state counts One count down.
					so only minus Count when I change from Sleep to Awake
				 */
				if (pEntry->CFGP2pInfo.NoADesc[0].bInAwake == FALSE)
					pEntry->CFGP2pInfo.NoADesc[0].Count--;
			}

			if (pEntry->CFGP2pInfo.NoADesc[0].Count == 0)
			{
				CFG80211_P2pStopNoA(pAd, pEntry);
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS %s: Count down to zero!!StopGP.  return.1 \n", __FUNCTION__));
				return;
			}

			/* To enter absence period, stop transmission a little bit earlier to leave HW to clean the queue. */
			if (pEntry->CFGP2pInfo.NoADesc[0].bInAwake == FALSE)
				NextDiff = pEntry->CFGP2pInfo.NoADesc[0].Duration - 0x200;
			else
				NextDiff = pEntry->CFGP2pInfo.NoADesc[0].Interval - pEntry->CFGP2pInfo.NoADesc[0].Duration + 0x200;

			/* Prepare next time. */


			/*RTMP_IO_READ32(pAd, TSF_TIMER_DW0, &MacValue); */
			MacValue = pAd->cfg80211_ctrl.GONoASchedule.LastBeaconTimeStamp;
			DBGPRINT(RT_DEBUG_TRACE,("P2P_PS 2 Tsf	Timer  = %ld,  NextTargetTimePoint = %ld.\n",
				MacValue, pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint));

			SavedNextTargetTimePoint = pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint;
			if (MacValue <= pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint)
			{
				GPDiff = pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint - MacValue;

				pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint += NextDiff;
				CFG80211_P2pResetNoATimer(pAd, GPDiff);
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS 3	Continue next NOA NextTargetTimePoint = %ld. \n",
						pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint));
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS 3	Value = %ld.  NextDiff = %ld.\n", MacValue, NextDiff));
			}
			else
			{
				CFG80211_P2pStopNoA(pAd, pEntry);
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS 4  NOA NextTargetTimePoint = %ld. \n",
							pEntry->CFGP2pInfo.NoADesc[0].NextTargetTimePoint));
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS 4  Value = %ld = 0x%lx.  NextDiff = %ld.\n", MacValue,  MacValue, NextDiff));
				return;
			}

			if (pEntry->CFGP2pInfo.NoADesc[0].bInAwake == TRUE)
			{
				pEntry->CFGP2pInfo.NoADesc[0].bInAwake = FALSE;
				pP2PCtrl->bKeepSlient = TRUE;
				pP2PCtrl->bPreKeepSlient = TRUE;
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS Enter Absence now ======> %d\n", pP2PCtrl->bKeepSlient));
			}
			else
			{
				pEntry->CFGP2pInfo.NoADesc[0].bInAwake = TRUE;
				pP2PCtrl->bKeepSlient = FALSE;
				pP2PCtrl->bPreKeepSlient = FALSE;
				if (IS_SW_NOA_TIMER(pAd) && (pP2PCtrl->GONoASchedule.Count > 100))
				{
					if (TRUE == CFG80211_P2pAdjustSwNoATimer(pAd, Value, SavedNextTargetTimePoint))
					{
						RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
					}
				}
				else
				{
					RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
				}

				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS Enter Awake now ======= %d\n", pAd->cfg80211_ctrl.bKeepSlient));

			}

		}

	}
}

VOID CFG80211_P2PCTWindowTimer(
	PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)FunctionContext;
	PCFG80211_CTRL pP2pCtrl = &pAd->cfg80211_ctrl;

	if (CFG80211_P2P_TEST_BIT(pP2pCtrl->CTWindows, P2P_OPPS_BIT))
		pP2pCtrl->bKeepSlient = TRUE;
}


/*
	==========================================================================
	Description:
		When I am P2P Client , Handle NoA Attribute.

	Parameters:
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID CFG80211_P2pSwNoATimeOut(
	PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)FunctionContext;
	CFG80211_P2pGPTimeOutHandle(pAd);
}

VOID CFG80211_P2pPreAbsenTimeOut(
	PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)FunctionContext;
	pAd->cfg80211_ctrl.bPreKeepSlient = TRUE;
}


BOOLEAN CFG80211_P2pResetNoATimer(struct rtmp_adapter *pAd, ULONG DiffTimeInUs)
{
	ULONG	GPDiff;
	BOOLEAN	brc = FALSE;

	/*
		Software based timer means don't use GP interrupt to get precise timer calculation.
		So need to check time offset caused by software timer.
	 */
	if (IS_SW_NOA_TIMER(pAd))
	{
		GPDiff = (DiffTimeInUs>>10) & 0xffff;
		printk("P2P_PS ==========> P2pResetNoATimer, %ld ==> %d\n", DiffTimeInUs, GPDiff);
		if (GPDiff > 0)
		{
			GPDiff++;
			RTMPSetTimer(&pAd->cfg80211_ctrl.P2pSwNoATimer, GPDiff);

			/* Increase timer tick counter. */
			pAd->cfg80211_ctrl.GONoASchedule.SwTimerTickCounter++;

			brc = TRUE;
			/* Will go to awake later. Set a pre-enter-absence timer that the time out is smaller the GPDiff. */
			if (pAd->cfg80211_ctrl.GONoASchedule.bInAwake == FALSE)
			{
				if (GPDiff > 10)
				{
					printk("P2P_PS ==========> P2pPreAbsenTimer, %d\n", (GPDiff - 10));
					RTMPSetTimer(&pAd->cfg80211_ctrl.P2pPreAbsenTimer, (GPDiff - 10));
				}
			}
		}
	}
	return brc;

}


VOID CFG80211_P2pGOStartNoA(struct rtmp_adapter *pAd)
{

}

VOID	CFG80211_P2pStopNoA(struct rtmp_adapter *pAd, PMAC_TABLE_ENTRY pMacClient)
{
	ULONG	Value;
	BOOLEAN	Cancelled;

	DBGPRINT(RT_DEBUG_TRACE,("P2P_PS %s .!!!! \n",__FUNCTION__));

	RTMPCancelTimer(&pAd->cfg80211_ctrl.P2pPreAbsenTimer, &Cancelled);
	pAd->cfg80211_ctrl.bKeepSlient = FALSE;
	pAd->cfg80211_ctrl.bPreKeepSlient = FALSE;
	if (pMacClient != NULL)
	{
		pMacClient->CFGP2pInfo.NoADesc[0].Count = 0xf3;
		pMacClient->CFGP2pInfo.NoADesc[0].bValid = FALSE;
		pMacClient->CFGP2pInfo.NoADesc[0].bInAwake = TRUE;
		/*
			Try set Token to a value that has smallest chane the same as the Next Token GO will use.
			So decrease 1
		 */
		pMacClient->CFGP2pInfo.NoADesc[0].Token--;
	}
	RTMPCancelTimer(&pAd->cfg80211_ctrl.P2pSwNoATimer, &Cancelled);
	pAd->cfg80211_ctrl.GONoASchedule.bValid = FALSE;
	pAd->cfg80211_ctrl.GONoASchedule.bInAwake = TRUE;


	/* If need not resume NoA. Can reset all parameters. */
	{
		pAd->cfg80211_ctrl.GONoASchedule.Count = 1;
		pAd->cfg80211_ctrl.GONoASchedule.Duration = 0xc800;
		pAd->cfg80211_ctrl.GONoASchedule.Interval = 0x19000;
	}

	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &= (0xfffffffd);
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);

	pAd->cfg80211_ctrl.GONoASchedule.SwTimerTickCounter = 0;

	/* Set to false again. */
	pAd->cfg80211_ctrl.bPreKeepSlient = FALSE;

}

VOID CFG80211_P2pStartOpPS(struct rtmp_adapter *pAd)
{
	if (pAd->cfg80211_ctrl.GONoASchedule.bValid == TRUE)
		CFG80211_P2pStopNoA(pAd, NULL);

	DBGPRINT(RT_DEBUG_TRACE,("P2P : !! %s \n",__FUNCTION__));
	pAd->cfg80211_ctrl.CTWindows = 0x8a;
	/* Wait next beacon period to really start queue packet. */
	pAd->cfg80211_ctrl.bKeepSlient = FALSE;

}

VOID CFG80211_P2pStopOpPS(struct rtmp_adapter *pAd)
{
	if (pAd->cfg80211_ctrl.GONoASchedule.bValid == FALSE)
		pAd->cfg80211_ctrl.bKeepSlient = FALSE;
	pAd->cfg80211_ctrl.CTWindows = 0;
}

static
ULONG CFG80211_P2pGetTimeStamp(struct rtmp_adapter *pAd)
{
	ULONG Value = 0;
	/* RTMP_IO_FORCE_READ32(pAd, TSF_TIMER_DW0, &Value); */
        Value = pAd->cfg80211_ctrl.GONoASchedule.LastBeaconTimeStamp;

	return Value;
}

BOOLEAN CFG80211_P2pHandleNoAAttri(struct rtmp_adapter *pAd, PMAC_TABLE_ENTRY pMacClient, u8 *pData)
{
	PP2P_NOA_DESC pNoADesc;
	ULONG Value, GPDiff, NoALen, StartTime;
	UCHAR index;

	if (pMacClient == NULL)
		return FALSE;

	if (*pData == SUBID_P2P_NOA)
	{
		NoALen = *(pData+1);
		if (NoALen == 2)
		{
			pMacClient->CFGP2pInfo.CTWindow = *(pData+4);
			if (pMacClient->CFGP2pInfo.NoADesc[0].bValid == TRUE)
				CFG80211_P2pStopNoA(pAd, pMacClient);
			/*
				Copy my GO's CTWindow to P2Pcfg.CTWindow parameters,
				Then As Client, I don't need to search for Client when I want to use CTWindow Value.
			 */
			pAd->cfg80211_ctrl.CTWindows = *(pData+4);
			return TRUE;
		}

		index = *(pData+3);

		pMacClient->CFGP2pInfo.CTWindow = *(pData+4);
		pAd->cfg80211_ctrl.CTWindows = *(pData+4);

		pNoADesc = (PP2P_NOA_DESC)(pData+5);
		pMacClient->CFGP2pInfo.NoADesc[0].Count = pNoADesc->Count;
		pMacClient->CFGP2pInfo.NoADesc[0].Duration = *(uint32_t *)&pNoADesc->Duration[0];
		pMacClient->CFGP2pInfo.NoADesc[0].Interval = *(uint32_t *)&pNoADesc->Interval[0];
		pMacClient->CFGP2pInfo.NoADesc[0].StartTime = *(uint32_t *)&pNoADesc->StartTime[0];
		StartTime = *(uint32_t *)&pNoADesc->StartTime[0];

		if (pMacClient->CFGP2pInfo.NoADesc[0].Token == index)
		{
			/* The same NoA. Doesn't need to set this NoA again. */
			return FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE,("P2P_PS : !!!NEW NOA Here =[%d, %d] Count = %d. Duration =  %ld \n",
					pMacClient->CFGP2pInfo.NoADesc[0].Token, index, pNoADesc->Count,
					pMacClient->CFGP2pInfo.NoADesc[0].Duration));
		DBGPRINT(RT_DEBUG_TRACE,("P2P_PS : CTWindow =  %x \n", pMacClient->CFGP2pInfo.CTWindow));

		pMacClient->CFGP2pInfo.NoADesc[0].Token = index;


		/* RTMP_IO_FORCE_READ32(pAd, TSF_TIMER_DW0, &Value); */
		Value = pAd->cfg80211_ctrl.GONoASchedule.LastBeaconTimeStamp;
		DBGPRINT(RT_DEBUG_TRACE,("P2P_PS Interval = %ld. StartTime = %ld. TSF timer = %ld\n",
			pMacClient->CFGP2pInfo.NoADesc[0].Interval, pMacClient->CFGP2pInfo.NoADesc[0].StartTime, Value));

		if ((pMacClient->CFGP2pInfo.NoADesc[0].Duration <= 0x40) || (pMacClient->CFGP2pInfo.NoADesc[0].Interval <= 0x40))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("P2P_PS !!!!!Interval or Duration too small. ignore.  = %lx return 1\n", Value));
			return FALSE;
		}
		else if ((pMacClient->CFGP2pInfo.NoADesc[0].Duration >= pMacClient->CFGP2pInfo.NoADesc[0].Interval)
			&& (pMacClient->CFGP2pInfo.NoADesc[0].Count > 1))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("P2P_PS !!!!!Duration > Inveral.  return 2\n"));
			return FALSE;
		}

		/* if Start time point is in the future. */
		pAd->cfg80211_ctrl.GONoASchedule.CurrentTargetTimePoint = pMacClient->CFGP2pInfo.NoADesc[0].StartTime;
		if (Value < StartTime)
		{
			GPDiff = pMacClient->CFGP2pInfo.NoADesc[0].StartTime - Value;
			pMacClient->CFGP2pInfo.NoADesc[0].NextTargetTimePoint =
					pMacClient->CFGP2pInfo.NoADesc[0].StartTime + pMacClient->CFGP2pInfo.NoADesc[0].Duration;
			pAd->cfg80211_ctrl.GONoASchedule.OngoingAwakeTime =
					pMacClient->CFGP2pInfo.NoADesc[0].NextTargetTimePoint;


			DBGPRINT(RT_DEBUG_TRACE,("P2P_PS !!!!! GPDiff = %ld. NextTargetTimePoint = %ld\n",
					GPDiff, pMacClient->CFGP2pInfo.NoADesc[0].NextTargetTimePoint));

			/* try to set General Timer. */
			pAd->cfg80211_ctrl.GONoASchedule.LastBeaconTimeStamp += GPDiff;
			if (CFG80211_P2pResetNoATimer(pAd, GPDiff))
			{
				DBGPRINT(RT_DEBUG_TRACE,("P2P_PS !!!!!Start NoA 1  GPDiff = %ld \n", GPDiff));
				pMacClient->CFGP2pInfo.NoADesc[0].bValid = TRUE;
				pMacClient->CFGP2pInfo.NoADesc[0].bInAwake = TRUE;
				pMacClient->CFGP2pInfo.NoADesc[0].Token = index;
				return TRUE;
			}
		}
		else if (Value >= StartTime)
		{
			/* Start time point is in the past. */
			do
			{
				StartTime += pMacClient->CFGP2pInfo.NoADesc[0].Interval;
				if ((StartTime > Value) && ((StartTime-Value) > 0x80))
				{
					GPDiff = StartTime - Value;
					pMacClient->CFGP2pInfo.NoADesc[0].NextTargetTimePoint = StartTime
								/* + pMacClient->P2pInfo.NoADesc[0].Interval */
								  - pMacClient->CFGP2pInfo.NoADesc[0].Duration;

					pAd->cfg80211_ctrl.GONoASchedule.OngoingAwakeTime =
								pMacClient->CFGP2pInfo.NoADesc[0].NextTargetTimePoint;
					pAd->cfg80211_ctrl.GONoASchedule.LastBeaconTimeStamp += GPDiff;

					if (CFG80211_P2pResetNoATimer(pAd, GPDiff))
					{
						DBGPRINT(RT_DEBUG_TRACE,("P2P_PS !!!!!Start NoA 2  GPDiff = %ld\n", GPDiff));
						pMacClient->CFGP2pInfo.NoADesc[0].bValid = TRUE;
						pMacClient->CFGP2pInfo.NoADesc[0].bInAwake = TRUE;
						pMacClient->CFGP2pInfo.NoADesc[0].Token = index;
						return TRUE;
					}
				}

			} while(TRUE);
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Start time out of ctrl ..need Check \n"));
		}
	}

	return FALSE;
}


VOID CFG80211_P2pParseNoASubElmt(struct rtmp_adapter *pAd, VOID *Msg, ULONG MsgLen,
                                 UCHAR wcidindex, uint32_t Sequence)
{
	PCFG80211_CTRL pP2PCtrl = &pAd->cfg80211_ctrl;
	ULONG Length = 0, AttriLen = 0, LeftLength = 0;
	PP2PEID_STRUCT pP2pEid;
	PEID_STRUCT pEid;
	BOOLEAN brc = FALSE, bNoAAttriExist = FALSE;
	u8 *pPtrEid = NULL;

	/* Intel sends multiple P2P IE... So I can't give each input a default value.. */
	if (MsgLen == 0)
		return;

	LeftLength = MsgLen;
	pEid = (PEID_STRUCT)Msg;
	while ((ULONG)(pEid->Len + 2) <= LeftLength)
	{
		/* might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE. */
		if (RTMPEqualMemory(&pEid->Octet[0], CFG_P2POUIBYTE, 4))
		{
			/* Get Request content capability */
			pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
			pPtrEid = (u8 *) pP2pEid;
			AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			Length = 0;

			while ((Length + 3 + AttriLen) <= pEid->Len)
			{
				switch(pP2pEid->Eid)
				{
					case SUBID_P2P_NOA:
					{
						u8 *pData = &pEid->Octet[0];
						DBGPRINT(RT_DEBUG_TRACE, ("P2P_PS Get NoA Attr: %x %x %x %x %x %x %x %x %x \n",
									*(pData+0), *(pData+1), *(pData+2), *(pData+3),
									*(pData+4), *(pData+5), *(pData+6), *(pData+7), *(pData+8)));

						bNoAAttriExist = TRUE;
						brc = CFG80211_P2pHandleNoAAttri(pAd, &pAd->MacTab.Content[wcidindex], pPtrEid);

						/* Got a NoA Attribute from this p2pindex. In fact, This should be GO. */
						if (brc == TRUE)
							pP2PCtrl->NoAIndex = wcidindex;
					}
						break;

					default:
						break;

				}

				Length = Length + 3 + AttriLen;  /* Eid[1] + Len[1]+ content[Len] */
				pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);
				pPtrEid = (u8 *) pP2pEid;
				AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			}
		}
		LeftLength = LeftLength - pEid->Len - 2;
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
	}

	if (bNoAAttriExist == FALSE)
	{
		if (CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.CTWindows, P2P_OPPS_BIT))
		{
			DBGPRINT(RT_DEBUG_TRACE,("Beacon and no NoA Attribute! \n"));
			CFG80211_P2pStopOpPS(pAd);
		}

		if ((pAd->MacTab.Content[wcidindex].CFGP2pInfo.NoADesc[0].bValid == TRUE))
		{
			DBGPRINT(RT_DEBUG_TRACE,("Beacon and no NoA Attribute!Stop active NoA [%d]\n", Sequence));
			CFG80211_P2pStopNoA(pAd, &pAd->MacTab.Content[wcidindex]);
		}
	}
	else
		printk("P2P_PS Debug: %s() %d ===> Get Entry\n", __FUNCTION__, __LINE__);

}


BOOLEAN CFG8211_PeerP2pBeaconSanity(
	struct rtmp_adapter *pAd, VOID *Msg, ULONG MsgLen,
	u8 *pAddr2, CHAR Ssid[], UCHAR *pSsidLen,
	ULONG *Peerip, ULONG *P2PSubelementLen,
	u8 *pP2pSubelement)
{
	PFRAME_802_11 pFrame;
	PEID_STRUCT pEid;
	ULONG Length = 0;
	BOOLEAN	brc = FALSE, bFirstP2pOUI = TRUE;
	u8 *Ptr;

	pFrame = (PFRAME_802_11)Msg;
	Length += LENGTH_802_11;

	*P2PSubelementLen = 0;
	*pSsidLen = 0;
	*Peerip = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

	Ptr = pFrame->Octet;

	/* get timestamp from payload and advance the pointer */
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;

	/* get beacon interval from payload and advance the pointer */
	Ptr += 2;
	Length += 2;

	/* get capability info from payload and advance the pointer */
	Ptr += 2;
	Length += 2;

	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)
	{
		switch(pEid->Eid)
		{
			case IE_SSID:
				if(pEid->Len <= MAX_LEN_OF_SSID)
				{
					RTMPMoveMemory(Ssid, pEid->Octet, pEid->Len);
					*pSsidLen = pEid->Len;
				}
				break;
			case IE_VENDOR_SPECIFIC:
				/* Check the OUI version, filter out non-standard usage */
				if (NdisEqualMemory(pEid->Octet, CFG_WPS_OUI, 4) && (pEid->Len >= 4))
				{
					if (*P2PSubelementLen == 0)
					{
						RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
						*P2PSubelementLen = pEid->Len +2;
					}
					else if (*P2PSubelementLen > 0)
					{
						if (((*P2PSubelementLen) + (pEid->Len+2)) <= MAX_VIE_LEN)
						{
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
							*P2PSubelementLen += (pEid->Len+2);
						}
						else
						{
							DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR!! 111 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n", __FUNCTION__, ((*P2PSubelementLen) + (pEid->Len+2))));
							return FALSE;
						}
					}

				}
				else if (NdisEqualMemory(pEid->Octet, CFG_P2POUIBYTE, 4) && (pEid->Len >= 4))
				{
					/*
						If this is the first P2P OUI. Then also append P2P OUI.
						Beacon 's P2P attribute doesn't exceed 256 bytes. So not use acumulcated form.
					 */
					if (bFirstP2pOUI == TRUE)
					{
						if (*P2PSubelementLen == 0)
						{
							RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
							*P2PSubelementLen = (pEid->Len +2);
							brc = TRUE;
						}
						else if (*P2PSubelementLen > 0)
						{
							if (((*P2PSubelementLen) + (pEid->Len+2)) <= MAX_VIE_LEN)
							{
								RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
								*P2PSubelementLen += (pEid->Len+2);
								brc = TRUE;
							}
							else
							{
								DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR!! 222 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n", __FUNCTION__, ((*P2PSubelementLen) + (pEid->Len+2))));
								return FALSE;
							}
						}
						bFirstP2pOUI = FALSE;
					}
					else
					{
						/*
							If this is not the first P2P OUI. Then don't append P2P OUI.
							because our parse function doesn't need so many P2P OUI.
						 */
						if ((*P2PSubelementLen > 0) && (pEid->Len > 4))
						{
							if (((*P2PSubelementLen) + (pEid->Len+2)) <= MAX_VIE_LEN)
							{
								RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
								*P2PSubelementLen += (pEid->Len+2);
								brc = TRUE;
							}
							else
							{
								DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR!! 333 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n", __FUNCTION__, ((*P2PSubelementLen) + (pEid->Len+2))));
								return FALSE;
							}
						}
					}
				}
				break;
		}
		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);

	}
	return brc;
}


VOID CFG80211_PeerP2pBeacon(struct rtmp_adapter *pAd,
	u8 *pAddr2, MLME_QUEUE_ELEM *Elem, LARGE_INTEGER TimeStamp)
{
	PCFG80211_CTRL pP2PCtrl = &pAd->cfg80211_ctrl;

	UCHAR	Addr2[6], SsidLen, Ssid[32];
	ULONG	PeerIp, P2PSubelementLen;
	u8 *P2pSubelement = NULL;
	PFRAME_802_11		pFrame;
	PMAC_TABLE_ENTRY pMacEntry = NULL;

	pFrame = (PFRAME_802_11)Elem->Msg;
	/* Only check beacon . */
	if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP)
		return;

	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	pMacEntry = &pAd->MacTab.Content[Elem->Wcid];

	/* Init P2pSubelement */
       	if (P2pSubelement)
        {
                kfree(P2pSubelement);
                P2pSubelement = NULL;
        }

	P2pSubelement = kmalloc(MAX_VIE_LENGFP_ATOMIC);
	if (P2pSubelement == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Allocate memory size(=%d) failed\n", __FUNCTION__, MAX_VIE_LEN));
		goto CleanUp;
	}

	if (CFG8211_PeerP2pBeaconSanity(pAd, Elem->Msg, Elem->MsgLen,
					Addr2, Ssid, &SsidLen, &PeerIp,
					&P2PSubelementLen,P2pSubelement))
	{
		/* Parse the power managemenr parameters in here. */
		pP2PCtrl->GONoASchedule.LastBeaconTimeStamp = TimeStamp.u.LowPart;
		CFG80211_P2pParseNoASubElmt(pAd, P2pSubelement, P2PSubelementLen, Elem->Wcid, pFrame->Hdr.Sequence);

		/* Since we get beacon, check if GO enable and OppPS. */
		if (CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.CTWindows, P2P_OPPS_BIT))
		{
			pAd->cfg80211_ctrl.bKeepSlient = FALSE;
			/* TO DO : sync with windows if necessary */
			/*RTMPDeQueueNoAMgmtPacket(pAd);*/

			RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
			if (((pAd->cfg80211_ctrl.CTWindows&0x7f) > 0) && ((pAd->cfg80211_ctrl.CTWindows&0x7f) < 80))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("%s::  set P2P CTWindows timer.\n", __FUNCTION__));
				RTMPSetTimer(&pAd->cfg80211_ctrl.P2pCTWindowTimer, (pAd->cfg80211_ctrl.CTWindows&0x7f));
			}
		}
	}

CleanUp:
	if (P2pSubelement)
	{
		kfree(P2pSubelement);
		P2pSubelement = NULL;
	}

}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID CFG80211_P2pClientSendNullFrame(struct rtmp_adapter *pAd, INT PwrMgmt)
{
        MAC_TABLE_ENTRY *pEntry;

        pEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid);
        if (pEntry == NULL)
        {
                DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_ROC: Can't Find In Table: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                                   PRINT_MAC(pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid)));
        }
        else
        {
                ApCliRTMPSendNullFrame(pAd,
                                       RATE_6,
                                       (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) ? TRUE:FALSE,
                                       pEntry, PwrMgmt);
                OS_WAIT(20);
        }
}

VOID CFG80211DRV_P2pClientKeyAdd(struct rtmp_adapter *pAd, VOID *pData)
{

	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;

    	DBGPRINT(RT_DEBUG_TRACE, ("CFG Debug: CFG80211DRV_P2pClientKeyAdd\n"));
    	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104)
		;
	else
	{
		INT 	BssIdx;
		PAPCLI_STRUCT pApCliEntry;
		MAC_TABLE_ENTRY	*pMacEntry=(MAC_TABLE_ENTRY *)NULL;

		BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAIN_MBSSID;
		pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
        	if (pKeyInfo->bPairwise == FALSE )
		{

			if (pApCliEntry->wdev.WepStatus == Ndis802_11Encryption3Enabled)
			{
				printk("APCLI: Set AES Security Set. [%d] (GROUP) %d\n", BssIdx, pKeyInfo->KeyLen);
				memset(&pApCliEntry->SharedKey[pKeyInfo->KeyId], 0, sizeof(CIPHER_KEY));
				pApCliEntry->SharedKey[pKeyInfo->KeyId].KeyLen = LEN_TK;
				memmove(pApCliEntry->SharedKey[pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);

				pApCliEntry->SharedKey[pKeyInfo->KeyId].CipherAlg = CIPHER_AES;

				AsicAddSharedKeyEntry(pAd, BssIdx, pKeyInfo->KeyId,
						      &pApCliEntry->SharedKey[pKeyInfo->KeyId]);

				RTMPAddWcidAttributeEntry(pAd, BssIdx, pKeyInfo->KeyId,
							  pApCliEntry->SharedKey[pKeyInfo->KeyId].CipherAlg,
							  NULL);

				if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				{
					/* set 802.1x port control */
					pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}
			}
		}
		else
		{
			if(pMacEntry)
			{
				printk("APCLI: Set AES Security Set. [%d] (PAIRWISE) %d\n", BssIdx, pKeyInfo->KeyLen);
				memset(&pMacEntry->PairwiseKey, 0, sizeof(CIPHER_KEY));
				pMacEntry->PairwiseKey.KeyLen = LEN_TK;

				memcpy(&pMacEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
				memmove(pMacEntry->PairwiseKey.Key, &pMacEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);

				pMacEntry->PairwiseKey.CipherAlg = CIPHER_AES;

				AsicAddPairwiseKeyEntry(pAd, (UCHAR)pMacEntry->Aid, &pMacEntry->PairwiseKey);
				RTMPSetWcidSecurityInfo(pAd, BssIdx, 0, pMacEntry->PairwiseKey.CipherAlg, pMacEntry->Aid, PAIRWISEKEYTABLE);
			}
			else
			{
				printk("APCLI: Set AES Security Set. (PAIRWISE) But pMacEntry NULL\n");
			}
		}
	}
}

VOID CFG80211DRV_SetP2pCliAssocIe(struct rtmp_adapter *pAd, VOID *pData, UINT ie_len)
{
	APCLI_STRUCT *apcli_entry;
	hex_dump("P2PCLI=", pData, ie_len);

	apcli_entry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];

	if (ie_len > 0)
	{
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe)
		{
			kfree(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		}

		apcli_entry->wpa_supplicant_info.pWpaAssocIe =
			kmalloc(ie_len, GFP_ATOMIC);
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe)
		{
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = ie_len;
			memmove(apcli_entry->wpa_supplicant_info.pWpaAssocIe, pData, apcli_entry->wpa_supplicant_info.WpaAssocIeLen);
		}
		else
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	}
	else
	{
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe)
		{
			kfree(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		}
		apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	}
}

/* For P2P_CLIENT Connection Setting in AP_CLI SM */
BOOLEAN CFG80211DRV_P2pClientConnect(struct rtmp_adapter *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;
	UCHAR Connect_SSID[NDIS_802_11_LENGTH_SSID];
	uint32_t Connect_SSIDLen;

	APCLI_STRUCT *apcli_entry;
	apcli_entry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];

	struct os_cookie *pObj = pAd->OS_Cookie;
	pObj->ioctl_if_type = INT_APCLI;

	pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *)pData;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI Connection onGoing.....\n"));

	Connect_SSIDLen = pConnInfo->SsidLen;
	if (Connect_SSIDLen > NDIS_802_11_LENGTH_SSID)
		Connect_SSIDLen = NDIS_802_11_LENGTH_SSID;

	memset(&Connect_SSID, 0, sizeof(Connect_SSID));
	memcpy(Connect_SSID, pConnInfo->pSsid, Connect_SSIDLen);

	apcli_entry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;

	/* Check the connection is WPS or not */
	if (pConnInfo->bWpsConnection)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AP_CLI WPS Connection onGoing.....\n"));
		apcli_entry->wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
	}

	/* Set authentication mode */
	if (pConnInfo->WpaVer == 2)
	{
		if (!pConnInfo->FlgIs8021x == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE,("APCLI WPA2PSK\n"));
			Set_ApCli_AuthMode_Proc(pAd, "WPA2PSK");
		}
	}
	else if (pConnInfo->WpaVer == 1)
	{
		if (!pConnInfo->FlgIs8021x)
		{
			DBGPRINT(RT_DEBUG_TRACE,("APCLI WPAPSK\n"));
			Set_ApCli_AuthMode_Proc(pAd, "WPAPSK");
		}
	}
	else
		Set_ApCli_AuthMode_Proc(pAd, "OPEN");

	/* Set PTK Encryption Mode */
	if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP) {
		DBGPRINT(RT_DEBUG_TRACE,("AES\n"));
		Set_ApCli_EncrypType_Proc(pAd, "AES");
	}
	else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP) {
		DBGPRINT(RT_DEBUG_TRACE,("TKIP\n"));
		Set_ApCli_EncrypType_Proc(pAd, "TKIP");
	}
	else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_WEP)
	{
		DBGPRINT(RT_DEBUG_TRACE,("WEP\n"));
		Set_ApCli_EncrypType_Proc(pAd, "WEP");
	}


	if (pConnInfo->pBssid != NULL)
	{
		memset(apcli_entry->CfgApCliBssid, 0, MAC_ADDR_LEN);
		memcpy(apcli_entry->CfgApCliBssid, pConnInfo->pBssid, MAC_ADDR_LEN);
	}

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = TRUE;
	Set_ApCli_Ssid_Proc(pAd, (char *)Connect_SSID);
	Set_ApCli_Enable_Proc(pAd, "1");
	CFG80211DBG(RT_DEBUG_OFF, ("80211> APCLI CONNECTING SSID = %s\n", Connect_SSID));

	return TRUE;
}

VOID CFG80211_P2pClientConnectResultInform(
	struct rtmp_adapter *pAd, UCHAR *pBSSID,
        UCHAR *pReqIe, uint32_t ReqIeLen,
        UCHAR *pRspIe, uint32_t RspIeLen,
        UCHAR FlgIsSuccess)
{
	CFG80211OS_P2pClientConnectResultInform(pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev.if_dev, pBSSID,
					pReqIe, ReqIeLen, pRspIe, RspIeLen, FlgIsSuccess);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
}

VOID CFG80211_LostP2pGoInform(struct rtmp_adapter *pAd)
{
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	struct net_device *pNetDev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("80211> CFG80211_LostGoInform ==> \n"));

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
	((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_CLIENT)) != NULL))
	{
	        if (pNetDev->ieee80211_ptr->sme_state == CFG80211_SME_CONNECTING)
       	 	{
                   cfg80211_connect_result(pNetDev, NULL, NULL, 0, NULL, 0,
                                                                   WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL);
        	}
        	else if (pNetDev->ieee80211_ptr->sme_state == CFG80211_SME_CONNECTED)
        	{
                   cfg80211_disconnected(pNetDev, 0, NULL, 0, GFP_KERNEL);
        	}
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("80211> BUG CFG80211_LostGoInform, BUT NetDevice not exist.\n"));

	Set_ApCli_Enable_Proc(pAd, "0");
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

