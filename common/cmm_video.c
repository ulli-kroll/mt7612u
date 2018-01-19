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
	cmm_video.c

    Abstract:
    Ralink WiFi Driver video mode related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

*/

#include "rt_config.h"


#ifdef VIDEO_TURBINE_SUPPORT



bool UpdateFromGlobal = false;

void VideoTurbineUpdate(
	IN struct rtmp_adapter *pAd)
{
	if (UpdateFromGlobal == true)
	{
		pAd->VideoTurbine.Enable = GLOBAL_AP_VIDEO_CONFIG.Enable;
		pAd->VideoTurbine.ClassifierEnable = GLOBAL_AP_VIDEO_CONFIG.ClassifierEnable;
		pAd->VideoTurbine.HighTxMode = GLOBAL_AP_VIDEO_CONFIG.HighTxMode;
		pAd->VideoTurbine.TxPwr = GLOBAL_AP_VIDEO_CONFIG.TxPwr;
		pAd->VideoTurbine.VideoMCSEnable = GLOBAL_AP_VIDEO_CONFIG.VideoMCSEnable;
		pAd->VideoTurbine.VideoMCS = GLOBAL_AP_VIDEO_CONFIG.VideoMCS;
		pAd->VideoTurbine.TxBASize = GLOBAL_AP_VIDEO_CONFIG.TxBASize;
		pAd->VideoTurbine.TxLifeTimeMode = GLOBAL_AP_VIDEO_CONFIG.TxLifeTimeMode;
		pAd->VideoTurbine.TxLifeTime = GLOBAL_AP_VIDEO_CONFIG.TxLifeTime;
		pAd->VideoTurbine.TxRetryLimit = GLOBAL_AP_VIDEO_CONFIG.TxRetryLimit;
	}
}


VOID TxSwQDepthAdjust(IN struct rtmp_adapter *pAd, IN uint32_t qLen)
{
	ULONG IrqFlags;
	INT qIdx;
	QUEUE_HEADER *pTxQ, *pEntry;
	struct sk_buff *pPacket;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	pAd->TxSwQMaxLen = qLen;
	for (qIdx = 0; qIdx < NUM_OF_TX_RING; qIdx++)
	{
		pTxQ = &pAd->TxSwQueue[qIdx];
		while(pTxQ->Number >= pAd->TxSwQMaxLen)
		{
			pEntry = RemoveHeadQueue(pTxQ);
			if (pEntry)
			{
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
				dev_kfree_skb_any(pPacket);
			}
			else
				break;
		}
	}
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	DBGPRINT(RT_DEBUG_OFF, ("%s():Set TxSwQMaxLen as %d\n",
			__FUNCTION__, pAd->TxSwQMaxLen));
}


VOID VideoTurbineDynamicTune(
	IN struct rtmp_adapter *pAd)
{
	if (pAd->VideoTurbine.Enable == true)
	{
			uint32_t MacReg = 0;

		{
			/* Tx retry limit = 2F,1F */
			mt76u_reg_read(pAd, TX_RTY_CFG, &MacReg);
			MacReg &= 0xFFFF0000;
			MacReg |= GetAsicVideoRetry(pAd);
			mt7612u_write32(pAd, TX_RTY_CFG, MacReg);
		}

		pAd->VideoTurbine.TxBASize = GetAsicVideoTxBA(pAd);

		Set_RateAdaptInterval(pAd, "100:50");
		TxSwQDepthAdjust(pAd, 1024);

	}
	else
	{
			uint32_t MacReg = 0;


		/* Default Tx retry limit = 1F,0F */
		mt76u_reg_read(pAd, TX_RTY_CFG, &MacReg);
		MacReg &= 0xFFFF0000;
			MacReg |= GetAsicDefaultRetry(pAd);
		mt7612u_write32(pAd, TX_RTY_CFG, MacReg);

		pAd->VideoTurbine.TxBASize = GetAsicDefaultTxBA(pAd);

		/* reset to default rate adaptation simping interval */
		if ((pAd->ra_interval != DEF_RA_TIME_INTRVAL) ||
			(pAd->ra_fast_interval != DEF_QUICK_RA_TIME_INTERVAL))
			Set_RateAdaptInterval(pAd, "500:100");

		TxSwQDepthAdjust(pAd, MAX_PACKETS_IN_QUEUE);
	}
}

uint32_t GetAsicDefaultRetry(
	IN struct rtmp_adapter *pAd)
{
	uint32_t RetryLimit;

	RetryLimit = 0x1F0F;

	return RetryLimit;
}

u8 GetAsicDefaultTxBA(
	IN struct rtmp_adapter *pAd)
{
        return pAd->CommonCfg.TxBASize;
}

uint32_t GetAsicVideoRetry(
	IN struct rtmp_adapter *pAd)
{
	return pAd->VideoTurbine.TxRetryLimit;
}

u8 GetAsicVideoTxBA(
	IN struct rtmp_adapter *pAd)
{
	return pAd->VideoTurbine.TxBASize;
}

VOID VideoConfigInit(
	IN struct rtmp_adapter *pAd)
{
	pAd->VideoTurbine.Enable = false;
	pAd->VideoTurbine.TxRetryLimit = 0x2F1F;
	pAd->VideoTurbine.TxBASize = pAd->CommonCfg.TxBASize;
}

#endif /* VIDEO_TURBINE_SUPPORT */


