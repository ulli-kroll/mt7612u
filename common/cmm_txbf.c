/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf.c

	Abstract:
	Tx Beamforming related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Shiang     2009/11/04
*/

#include	"rt_config.h"


#define ETXBF_PROBE_TIME (RA_INTERVAL-100)	/* Wait for Sounding Response will time out 100msec before end of RA interval */




#ifdef ETXBF_EN_COND3_SUPPORT
UCHAR groupShift[] = {4, 4, 4};
UCHAR groupMethod[] = {0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 1, 0, 1, 1, 1, 1,
						0, 0, 1, 0, 1, 1, 1, 1};
SHORT groupThrd[] = {-8, 4, 20, 32, 52, 68, 80, 88,
					  -16, 8, 12, 64, 40, 60, 80, 88,
					  -24, 12, 12, 96, 40, 60, 80, 88};
UINT dataRate[] = {65, 130, 195, 260, 390, 520, 585, 650,
				130, 260, 390, 520, 780, 1040, 1170, 1300,
				190, 390, 585, 780, 1170, 1560, 1755, 1950};
#endif /* ETXBF_EN_COND3_SUPPORT */


VOID rtmp_asic_set_bf(
	IN struct rtmp_adapter *pAd)
{
	UINT8 byteValue = 0;
	UINT Value32;



#ifdef MT76x2
	Value32 = mt7612u_read32(pAd, PFMU_R1);
	Value32 &= ~0x330;

	if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
		Value32 |= 0x120;
	else
		Value32 &= (~0x120);

	if (pAd->CommonCfg.ETxBfEnCond > 0)
		Value32 |= 0x210;
	else
		Value32 &= ~0x210;

	RTMP_IO_WRITE32(pAd, PFMU_R1, Value32);

	Value32 = mt7612u_read32(pAd, PFMU_R0);
	Value32 &= ~((0x1 << 6) | 0x3);

	if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
		Value32 |= ((0x1 << 6) | 0x1);
	else
		Value32 &= ~((0x1 << 6) | 0x1);

	if (pAd->CommonCfg.ETxBfEnCond > 0)
	{
		Value32 |= (0x1 << 6) | 0x2;
		RTMP_IO_WRITE32(pAd, TX_TXBF_CFG_2, 0xFFFFFFFF);
	}
	else
		Value32 &= ~((0x1 << 6) | 0x2);

	RTMP_IO_WRITE32(pAd, PFMU_R0, Value32);
#endif
}

/*
	TxBFInit - Intialize TxBF fields in pEntry
		supportsETxBF - TRUE if client supports ETxBF
*/
VOID TxBFInit(
	IN struct rtmp_adapter *	pAd,
	IN MAC_TABLE_ENTRY	*pEntry,
	IN BOOLEAN			supportsETxBF)
{
	pEntry->bfState = READY_FOR_SNDG0;
	pEntry->sndgMcs = 0;
	pEntry->sndg0Snr0 = 0;
	pEntry->sndg0Snr1 = 0;
	pEntry->sndg0Snr2 = 0;
	pEntry->sndg0Mcs = 0;
#ifdef ETXBF_EN_COND3_SUPPORT
	pEntry->sndgRateIdx = 0;
	pEntry->sndg0RateIdx = 0;
	pEntry->sndg1Mcs = 0;
	pEntry->sndg1RateIdx = 0;
	pEntry->sndg1Snr0 = 0;
	pEntry->sndg1Snr1 = 0;
	pEntry->sndg1Snr2 = 0;
	pEntry->bf0Mcs = 0;
	pEntry->bf0RateIdx = 0;
	pEntry->bf1Mcs = 0;
	pEntry->bf1RateIdx = 0;
#endif /* EXTBF_EN_COND3_SUPPORT */
	pEntry->noSndgCnt = 0;
	pEntry->eTxBfEnCond = supportsETxBF? pAd->CommonCfg.ETxBfEnCond: 0;
	pEntry->noSndgCntThrd = NO_SNDG_CNT_THRD;
	pEntry->ndpSndgStreams = pAd->Antenna.field.TxPath;

	/* If client supports ETxBf and ITxBF then give ETxBF priority over ITxBF */
	pEntry->iTxBfEn = pEntry->eTxBfEnCond> 0 ? 0 : pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;

}

BOOLEAN rtmp_chk_itxbf_calibration(
	IN struct rtmp_adapter *pAd)
{
	INT calIdx, calCnt;
	USHORT offset, eeVal, *calptr;
#ifndef MT76x2
	USHORT g_caladdr[] = {0x1a0, 0x1a2, 0x1b0, 0x1b2, 0x1b6, 0x1b8};
	USHORT a_caladdr[] = {0x1a4, 0x1a6, 0x1a8, 0x1aa, 0x1ac, 0x1ae, 0x1b4, 0x1ba, 0x1bc, 0x1be, 0x1c0, 0x1c2, 0x1c4, 0x1c6, 0x1c8};
#else
	USHORT g_caladdr[] = {0xc0, 0xc2, 0xd4, 0xd6, 0xd8};
	USHORT a_caladdr[] = {0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0};
#endif
	uint32_t ee_sum;
	BOOLEAN bCalibrated = TRUE;


	if (pAd->CommonCfg.Channel <= 14)
	{
		calCnt = sizeof(g_caladdr) / sizeof(USHORT);
		calptr = &g_caladdr[0] ;
	}
	else
	{
		calCnt = sizeof(a_caladdr) / sizeof(USHORT);
		calptr = &a_caladdr[0];
	}

	ee_sum = 0;
	for (calIdx = 0; calIdx < calCnt; calIdx++)
	{
		offset = *(calptr + calIdx);
		RT28xx_EEPROM_READ16(pAd, offset, eeVal);
		ee_sum += eeVal;
		DBGPRINT(RT_DEBUG_INFO, ("Check EEPROM(offset=0x%x, eeVal=0x%x, ee_sum=0x%x)!\n",
					offset, eeVal, ee_sum));
		if (eeVal!=0xffff && eeVal!=0)
			return TRUE;
	}

	if ((ee_sum == (0xffff * calCnt)) || (ee_sum == 0x0))
	{
		bCalibrated = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("EEPROM all 0xffff(cnt =%d, sum=0x%x), not valid calibration value!\n",
					calCnt, ee_sum));
	}

	return bCalibrated;
}


VOID Trigger_Sounding_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			SndgType,
	IN	UCHAR			SndgBW,
	IN	UCHAR			SndgMcs,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	/*
		SngType
			0: disable
			1 : sounding
			2: NDP sounding
	*/
	NdisAcquireSpinLock(&pEntry->TxSndgLock);
	pEntry->TxSndgType = SndgType;
	NdisReleaseSpinLock(&pEntry->TxSndgLock);

	RTMPSetTimer(&pEntry->eTxBfProbeTimer, ETXBF_PROBE_TIME);
	DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in Trigger_Sounding_Packet(): sndgType=%d, bw=%d, mcs=%d\n", SndgType, SndgBW, SndgMcs));
}


/*
	eTxBFProbing - called by Rate Adaptation routine each interval.
		Initiates a sounding packet if enabled.
*/
VOID eTxBFProbing(
 	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY	*pEntry)
{
	if (pEntry->eTxBfEnCond == 0)
	{
		pEntry->bfState = READY_FOR_SNDG0;
	}
	else if (pEntry->bfState==READY_FOR_SNDG0 && pEntry->noSndgCnt>=pEntry->noSndgCntThrd)
	{
		/* Select NDP sounding, maximum streams */
		pEntry->sndgMcs = (pEntry->ndpSndgStreams==3) ? 16 : 8;
		Trigger_Sounding_Packet(pAd, SNDG_TYPE_NDP, 0, pEntry->sndgMcs, pEntry);

		pEntry->bfState = WAIT_SNDG_FB0;
		pEntry->noSndgCnt = 0;
	}
	else if (pEntry->bfState == READY_FOR_SNDG0)
	{
		pEntry->noSndgCnt++;
	}
	else
		pEntry->noSndgCnt = 0;
}


/*
	clientSupportsETxBF - returns true if client supports compatible Sounding
*/
BOOLEAN clientSupportsETxBF(
	IN	struct rtmp_adapter * pAd,
	IN	HT_BF_CAP *pTxBFCap)
{
	BOOLEAN compCompat, noncompCompat;

	compCompat = (pTxBFCap->ExpComBF > 0) &&
			 /*(pTxBFCap->ComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath) && */
			 (pAd->CommonCfg.ETxBfNoncompress == 0);

	noncompCompat = (pTxBFCap->ExpNoComBF > 0)
			/* && (pTxBFCap->NoComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath)*/;

	return pTxBFCap->RxNDPCapable==1 && (compCompat || noncompCompat);
}


/*
	clientSupportsETxBF - returns true if client supports compatible Sounding
*/
BOOLEAN clientSupportsVHTETxBF(
	IN	struct rtmp_adapter * pAd,
	IN	VHT_CAP_INFO    *pTxBFCap)
{
	return pTxBFCap->bfee_cap_su;
}


/*
	setETxBFCap - sets our ETxBF capabilities
*/
void setETxBFCap(struct rtmp_adapter *pAd, HT_BF_CAP *pTxBFCap)
{
	if (pAd->CommonCfg.ETxBfIncapable) {
		memset(pTxBFCap, 0, sizeof(*pTxBFCap));
	}
	else
	{
		pTxBFCap->RxNDPCapable =  TRUE;
		pTxBFCap->TxNDPCapable =  TRUE;
		pTxBFCap->ExpNoComSteerCapable =  TRUE;
		pTxBFCap->ExpComSteerCapable = !pAd->CommonCfg.ETxBfNoncompress;
		pTxBFCap->ExpNoComBF = HT_ExBF_FB_CAP_IMMEDIATE;
		pTxBFCap->ExpComBF = pAd->CommonCfg.ETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE;
		pTxBFCap->MinGrouping = 3;
#ifndef MT76x2
		pTxBFCap->NoComSteerBFAntSup = 2;
		pTxBFCap->ComSteerBFAntSup = 2;
#else
		pTxBFCap->NoComSteerBFAntSup = 1; // 2 Tx antenna sounding
		pTxBFCap->ComSteerBFAntSup = 1;   // 2 Tx antenna sounding

		pTxBFCap->TxSoundCapable = TRUE;  // Support staggered sounding frames
#endif
		pTxBFCap->ChanEstimation = pAd->Antenna.field.RxPath-1;
	}
}

void setVHTETxBFCap(struct rtmp_adapter *pAd, VHT_CAP_INFO *pTxBFCap)
{
	if (pAd->CommonCfg.ETxBfIncapable) {
		pTxBFCap->num_snd_dimension = 0;
		pTxBFCap->bfee_cap_mu = FALSE;
		pTxBFCap->bfee_cap_su = FALSE;
		pTxBFCap->bfer_cap_mu = FALSE;
		pTxBFCap->bfer_cap_su = FALSE;
		pTxBFCap->cmp_st_num_bfer = 0;
	}
	else
	{
		pTxBFCap->bfee_cap_su = TRUE;
		pTxBFCap->bfer_cap_su = TRUE;
		pTxBFCap->num_snd_dimension = 1;
		pTxBFCap->cmp_st_num_bfer = 1;
	}
}


#ifdef ETXBF_EN_COND3_SUPPORT
/*
	4. determine the best method among  mfb0, mfb1, snrComb0, snrComb1
 	5. use the best method. if necessary, sndg with the mcs which resulting in the best snrComb.
*/
/*if mcs is not in group 1 */
VOID txSndgSameMcs(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	UCHAR smoothMfb)/*smoothMfb should be the current mcs */
{
	u8 *	pTable;
	UCHAR 		TableSize = 0;
	UCHAR		InitTxRateIdx, i, step;
	BOOLEAN		bWriteEnable;
	UCHAR		SndgType = SNDG_TYPE_SOUNDING;

	if (pEntry->eTxBfEnCond == 0)
	{
		pEntry->bfState = READY_FOR_SNDG0;
		return;
	}

	/*0. write down the current mfb0 */
	pEntry->mfb0 = smoothMfb;

	/* 1. sndg with current mcs, get snrComb0 */
	if (smoothMfb >> 3 > 0 )
	{
		pEntry->sndgMcs = smoothMfb;
	}
	else
	{
		pEntry->sndgMcs = 8;
		SndgType = SNDG_TYPE_NDP;
	}

	/* if ndp sndg is forced by iwpriv command */
	if (pEntry->ndpSndgStreams == 2 ||pEntry->ndpSndgStreams == 3)
	{
		SndgType = SNDG_TYPE_NDP;
		if (pEntry->ndpSndgStreams == 3)
			pEntry->sndgMcs = 16;
		else
			pEntry->sndgMcs = 8;
	}

	/*
		smoothMfb is guaranteed included in the current pTable because
		it is converted from received MFB in handleHtcField()
	*/
	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		step = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		step = 5;
	for (i=1; i<=TableSize; i++)
	{
		if (pTable[i*step+2] >= pEntry->sndgMcs)
			break;
	}

	if (i > TableSize)
		i = TableSize - 1;

/*
	DBGPRINT(RT_DEBUG_TRACE, ("txSndgSameMcs i = %x, step = %x, sndgMcs = %x CurrentMCS = %x \n",
				i, step, pEntry->sndgMcs, pTable[i*step+2]));
*/

	pEntry->sndgRateIdx = pTable[i*step];
	if (pEntry->sndgMcs != pTable[i*step+2])
	{
		/*pEntry->sndgMcs = pTable[i*step+2];*/

		if (pTable[i*step+2] > 16)
			pEntry->sndgMcs = 16;
		else if (pTable[i*step+2] > 8)
			pEntry->sndgMcs = 8;
		else
			pEntry->sndgMcs = 0;

		SndgType = SNDG_TYPE_NDP;
	}

	/* Enable/disable BF matrix writing */
	if  (pEntry->eTxBfEnCond == 1 || pEntry->eTxBfEnCond == 2)
	{
		bWriteEnable = TRUE;
		pEntry->HTPhyMode.field.eTxBF = 1;
	}
	else
	{
		bWriteEnable = FALSE;
	}
	rtmp_asic_etxbf_write_change(pAd, bWriteEnable);

	/* send a sounding packet*/
	Trigger_Sounding_Packet(pAd, SndgType, 0, pEntry->sndgMcs, pEntry);

	pEntry->bfState = WAIT_SNDG_FB0;
	pEntry->noSndgCnt = 0;
}


/*
	txSndgOtherGroup - NOTE: currently unused.
		Only called when ETxBfEnCond==3
*/
VOID txSndgOtherGroup(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry)
{
	u8 *	pTable;
	UCHAR 		TableSize = 0;
	UCHAR		InitTxRateIdx, i, step;
	UCHAR		byteValue = 0;
	UCHAR		SndgType = SNDG_TYPE_SOUNDING;


	/* tx sndg with mcs in the other group */
	if ((pEntry->sndgMcs)>>3 == 2)
	{
		pEntry->sndgMcs = 8;
		SndgType = SNDG_TYPE_NDP;
	}
	else
	{
		pEntry->sndgMcs = 16;
		SndgType = SNDG_TYPE_NDP;
	}
	/* copied from txSndgSameMcs() */
	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		step = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		step = 5;
	for (i=1; i<=TableSize; i++)
	{
		if (pTable[i*step+2] >= pEntry->sndgMcs) break;
	}

	if (i > TableSize)
		i = TableSize - 1;

	pEntry->sndgRateIdx = pTable[i*step];

	if (pEntry->sndgMcs != pTable[i*step+2])
	{
		pEntry->sndgMcs = pTable[i*step+2];
		SndgType = SNDG_TYPE_NDP;
	}
	/*---copied from txSndgSameMcs() end */
	/* disable BF matrix writing */
	rtmp_asic_etxbf_write_change(pAd, FALSE);
	Trigger_Sounding_Packet(pAd, SndgType, 0, pEntry->sndgMcs, pEntry);

	DBGPRINT(RT_DEBUG_TRACE,("ETxBF in txSndgOtherGroup(): tx the second SNDG, enter state WAIT_SNDG_FB1\n" ));

	pEntry->bfState = WAIT_SNDG_FB1;
}

VOID txMrqInvTxBF(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry)

{
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = MSI_TOGGLE_BF;

/*	pEntry->HTPhyMode.field.TxBF = ~pEntry->HTPhyMode.field.TxBF;done in another function call*/

	RTMPSetTimer(&pEntry->eTxBfProbeTimer, ETXBF_PROBE_TIME);
	DBGPRINT(RT_DEBUG_TRACE,("ETxBF in txMrqInvTxBF(): tx the second MRQ, enter state WAIT_MFB\n" ));
	pEntry->bfState = WAIT_MFB;
}

UINT convertSnrToThroughput(
	IN UCHAR streamsIn,
	IN INT snr0,
	IN INT snr1,
	IN INT snr2,
	IN u8 *pTable,
	OUT UCHAR *bestMcsPtr,
	OUT UCHAR *bestRateIdxPtr

	)
{

	UCHAR streams;
	INT	snrTemp;
	UCHAR 	i, j;
	SHORT idx;
	SHORT group;
 	INT	snrTemp1[3];
	INT snr[] = {snr0, snr1, snr2};
	INT snrSum, tpTemp, bestTp=0;
	SHORT thrdTemp;
	BOOLEAN isMcsValid[24];
	UCHAR	rateIdx[24], step, tableSize;
	UCHAR mcs;

#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		step = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		step = 5;
	tableSize = RATE_TABLE_SIZE(pTable);
	for (i=0; i<24; i++)
	{
		isMcsValid[i] = FALSE;
		rateIdx[i] = 0;
	}
	for (i=1; i<=tableSize; i++)
	{
		isMcsValid[pTable[i*step+2]] = TRUE;
		rateIdx[pTable[i*step+2]] = pTable[i*step];
	}

	if (streamsIn > 3)
	{
		DBGPRINT(RT_DEBUG_TRACE,("convertSnrToThroughput(): %d streams are not supported!!!", streamsIn));
		streams = 3;
	}
	else
		streams = streamsIn;

	for (i=0; i<streams; i++){
		for (j=i+1; j<streams; j++){
			if (snr[i] < snr[j]){
				snrTemp = snr[i];
				snr[i] = snr[j];
				snr[j] = snrTemp;
			}
		}
	}
	(*bestMcsPtr) = 0;
	(*bestRateIdxPtr) = rateIdx[0];
	for (group=streams-1; group >=0; group--)
	{
		snrTemp1[0] = snr[0];
		snrTemp1[1] = snr[1];
		snrTemp1[2] = snr[2];
		/*SNR processing for each group according to the baseband implementation, for example MRC*/
		switch (group)
		{
			case 0:
				snrTemp1[1] = 0;
				snrTemp1[2] = 0;
				break;
			case 1:
				snrTemp1[2] = 0;
				break;
			case 2:
				break;
			default:
				break;
		}
		snrSum = snr[0] + snr[1] + snr[2];
		for (idx=7; idx>=0; idx--){
			mcs = group*8+idx;
			thrdTemp = groupThrd[mcs];
			tpTemp = 0;
			if (groupMethod[mcs] == 0)
			{
				if (snrSum > thrdTemp)
					tpTemp =  ((snrSum - thrdTemp) * dataRate[mcs])>>groupShift[group];
			}
			else
			{
				if (group == 1)
					snrTemp1[2] = thrdTemp + 1;
				if (snrTemp1[0] > thrdTemp && snrTemp1[1] > thrdTemp && snrTemp1[2] > thrdTemp)
					tpTemp = ((snrTemp1[0] - thrdTemp)*(snrTemp1[1] - thrdTemp)*(snrTemp1[2] - thrdTemp) * dataRate[mcs])>>groupShift[group];/* have to be revised!!!*/
			}
			if (tpTemp > dataRate[mcs])
				tpTemp = dataRate[mcs];
			if (tpTemp > bestTp && isMcsValid[mcs] == TRUE)
			{
				bestTp = tpTemp;
				(*bestMcsPtr) = mcs;
				(*bestRateIdxPtr) = rateIdx[mcs];
				DBGPRINT(RT_DEBUG_TRACE,("convertSnrToThroughput(): new candidate snr0=%d, snr1=%d, snr2=%d, tp=%d, best MCS=%d\n", snrTemp1[0], snrTemp1[1], snrTemp1[2], tpTemp, *bestMcsPtr));
			}
		}
	}
	DBGPRINT(RT_DEBUG_TRACE,("convertSnrToThroughput(): snr0=%d, snr1=%d, snr2=%d, tp=%d, best MCS=%d\n", snr0, snr1, snr2, bestTp, *bestMcsPtr));
	return bestTp;
}


/*
	NOTE: currently unused. Only called when ETxBfEnCond==3
*/
VOID chooseBestMethod(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	UCHAR			mfb)
{
/*	UCHAR bestMethod;0:original, 1:inverted TxBF, 2:first sndg, 3:second sndg*/
	UINT tp[4], bestTp;
	UCHAR streams, i;
	u8 *	pTable;
	UCHAR 		TableSize = 0;
	UCHAR		InitTxRateIdx;
	UCHAR		byteValue = 0;

	pEntry->mfb1 = mfb;
	DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): received the second MFB %d, noted as mfb1\n", pEntry->mfb1 ));

	if ((pEntry->HTCapability.MCSSet[2] == 0xff) && (pAd->CommonCfg.TxStream == 3))
	{
		streams = 3;
	}
	else if (pEntry->HTCapability.MCSSet[0] == 0xff && pEntry->HTCapability.MCSSet[1] == 0xff && pAd->CommonCfg.TxStream > 1
		 	      && (pAd->CommonCfg.TxStream == 2 || pEntry->HTCapability.MCSSet[2] == 0x0))
	{
		streams = 2;
	}
	else
	{
		streams = 1;
	}

	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

	 tp[2] = convertSnrToThroughput(streams, pEntry->sndg0Snr0,  pEntry->sndg0Snr1, pEntry->sndg0Snr2, pTable, &(pEntry->bf0Mcs), &(pEntry->bf0RateIdx));
	 tp[3] = convertSnrToThroughput(streams, pEntry->sndg1Snr0,  pEntry->sndg1Snr1, pEntry->sndg1Snr2, pTable, &(pEntry->bf1Mcs), &(pEntry->bf1RateIdx));
	 tp[0] = dataRate[pEntry->mfb0];
	 tp[1] = dataRate[pEntry->mfb1];
	 bestTp = 0;
	 for (i=0; i<4; i++)
	 {
		DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): predicted throughput of method %d = %d\n", i, tp[i] ));
	 	if (tp[i] > bestTp)
		{
			bestTp = tp[i];
			pEntry->bestMethod = i;
	 	}
	 }
	 DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): method %d is chosen\n", pEntry->bestMethod ));
	 switch (pEntry->bestMethod)
	 {
	 	case 0:/*do nothing*/
			pEntry->bfState = READY_FOR_SNDG0;
			DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): do nothing, and enter state READY_FOR_SNDG0\n" ));
			break;
	 	case 1:
			pEntry->HTPhyMode.field.eTxBF = ~pEntry->HTPhyMode.field.eTxBF;
			pEntry->bfState = READY_FOR_SNDG0;
			DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): invert the ETxBF status, and enter state READY_FOR_SNDG0\n" ));
			break;
	 	case 2:
			pEntry->sndgMcs = pEntry->sndg0Mcs;
			pEntry->sndgRateIdx = pEntry->sndg0RateIdx;
			/* enable BF matrix writing */
			rtmp_asic_etxbf_write_change(pAd, TRUE);
			if (pEntry->sndgRateIdx == pEntry->CurrTxRateIndex)
				Trigger_Sounding_Packet(pAd, SNDG_TYPE_SOUNDING, 0, pEntry->sndgMcs, pEntry);
			else
				Trigger_Sounding_Packet(pAd, SNDG_TYPE_NDP, 0, pEntry->sndgMcs, pEntry);
			DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): tx the SNDG of the best method, enter state WAIT_BEST_SNDG\n" ));
			pEntry->bfState = WAIT_BEST_SNDG;
			break;
	 	case 3:
			/* tx sndg with mcs in the other group */
			pEntry->sndgMcs = pEntry->sndg1Mcs;
			pEntry->sndgRateIdx = pEntry->sndg1RateIdx;
			/* enable BF matrix writing */
			rtmp_asic_etxbf_write_change(pAd, TRUE);
			if (pEntry->sndgRateIdx == pEntry->CurrTxRateIndex)
				Trigger_Sounding_Packet(pAd, SNDG_TYPE_SOUNDING, 0, pEntry->sndgMcs, pEntry);
			else
				Trigger_Sounding_Packet(pAd, SNDG_TYPE_NDP, 0, pEntry->sndgMcs, pEntry);
			DBGPRINT(RT_DEBUG_TRACE,("ETxBF in chooseBestMethod(): tx the SNDG of the best method, enter state WAIT_BEST_SNDG\n" ));
			pEntry->bfState = WAIT_BEST_SNDG;
			break;
	 }
}


/*
	NOTE: currently unused. Only called when ETxBfEnCond==3
*/
VOID rxBestSndg(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry)
{
	/*set the best mcs of this BF matrix*/
	if (pEntry->bestMethod == 2)
	{
		pEntry->CurrTxRate = pEntry->bf0Mcs;
		pEntry->CurrTxRateIndex = pEntry->bf0RateIdx;
	}
	else if (pEntry->bestMethod == 3)
	{
		pEntry->CurrTxRate = pEntry->bf1Mcs;
		pEntry->CurrTxRateIndex = pEntry->bf1RateIdx;
	}
	pEntry->HTPhyMode.field.eTxBF = 1;

	/*must sync the timing of using new BF matrix and its bfRateIdx!!!*/
	/*need to reset counter for rateAdapt and may have to skip one adaptation when the new BF matrix is applied!!!*/

	DBGPRINT(RT_DEBUG_TRACE,("ETxBF in rxBestSndg(): received the feedback of the best SNDG, and enter state READY_FOR_SNDG0\n" ));

	pEntry->bfState = READY_FOR_SNDG0;
}
#endif	/* ETXBF_EN_COND3_SUPPORT */

VOID handleBfFb(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk)
{
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pRxBlk->wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		return;
	}
	pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);

	/*
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF :(%02x:%02x:%02x:%02x:%02x:%02x)\n",
							pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],
							pEntry->Addr[3],pEntry->Addr[4], pEntry->Addr[5]));
	*/

	if (pEntry->bfState == WAIT_SNDG_FB0)
	{
		int Nc = ((pRxBlk ->pData)[2] & 0x3) + 1;
		/*record the snr comb*/
		pEntry->sndg0Snr0 = 88+(CHAR)(pRxBlk ->pData[8]);
		pEntry->sndg0Snr1 = (Nc<2)? 0: 88+(CHAR)(pRxBlk ->pData[9]);
		pEntry->sndg0Snr2 = (Nc<3)? 0: 88+(CHAR)(pRxBlk ->pData[10]);
		pEntry->sndg0Mcs = pEntry->sndgMcs;

		DBGPRINT(RT_DEBUG_INFO,("   ETxBF: aid=%d  snr %d.%02d %d.%02d %d.%02d\n",
					pRxBlk->wcid,
					pEntry->sndg0Snr0/4,  25*(pEntry->sndg0Snr0 & 0x3),
					pEntry->sndg0Snr1/4,  25*(pEntry->sndg0Snr1 & 0x3),
					pEntry->sndg0Snr2/4,  25*(pEntry->sndg0Snr2 & 0x3)) );
#ifdef ETXBF_EN_COND3_SUPPORT
			if (pEntry->eTxBfEnCond == 1 ||pEntry->eTxBfEnCond == 2)
				pEntry->bfState = READY_FOR_SNDG0;
	     		/* 2. sndg with current mcs+8 or -8, get snrComb1*/
	     		else if (pEntry->eTxBfEnCond == 3)
				txSndgOtherGroup(pAd, pEntry);
#else
		pEntry->bfState = READY_FOR_SNDG0;
#endif
	}
#ifdef ETXBF_EN_COND3_SUPPORT
	else if (pEntry->bfState == WAIT_SNDG_FB1)
	{
		/* 3. mrq with inverted TxBF status, get mfb1*/
		if (TRUE)
		{
			int Nc = ((pRxBlk ->pData)[2] & 0x3) + 1;
			/* record the snr comb */
			pEntry->sndg1Snr0 = 88+(CHAR)(pRxBlk ->pData[8]);
			pEntry->sndg1Snr1 = (Nc<2)? 0: 88+(CHAR)(pRxBlk ->pData[9]);
			pEntry->sndg1Snr2 = (Nc<3)? 0: 88+(CHAR)(pRxBlk ->pData[10]);
			pEntry->sndg1Mcs = pEntry->sndgMcs;

			DBGPRINT(RT_DEBUG_INFO,("   ETxBF: mcs%d, snr %d  %d %d\n",  pEntry->sndg1Mcs, pEntry->sndg1Snr0, pEntry->sndg1Snr1, pEntry->sndg1Snr2 ));
			txMrqInvTxBF(pAd,  pEntry);
	     	}
		else
			chooseBestMethod(pAd, pEntry, 0);
	}
	else if (pEntry->bfState == WAIT_USELESS_RSP)
	{
		int Nc = ((pRxBlk ->pData)[2] & 0x3) + 1;
		pEntry->sndg0Snr0 = 88+(CHAR)(pRxBlk ->pData[8]);
		pEntry->sndg0Snr1 = (Nc<2)? 0: 88+(CHAR)(pRxBlk ->pData[9]);
		pEntry->sndg0Snr2 = (Nc<3)? 0: 88+(CHAR)(pRxBlk ->pData[10]);
		DBGPRINT(RT_DEBUG_INFO,("   ETxBF: mcs%d, snr %d  %d %d\n",  pEntry->sndg1Mcs, pEntry->sndg1Snr0, pEntry->sndg1Snr1, pEntry->sndg1Snr2 ));
		txSndgSameMcs(pAd, pEntry, /*pRxBlk,*/ pEntry->lastLegalMfb);
	}
	else if (pEntry->bfState == WAIT_BEST_SNDG)
	{
		rxBestSndg(pAd, pEntry);
	}
#endif	/* ETXBF_EN_COND3_SUPPORT */
}


VOID handleHtcField(struct rtmp_adapter *pAd, RX_BLK *pRxBlk)
{
}


void eTxBfProbeTimerExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY     *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;
#ifdef ETXBF_EN_COND3_SUPPORT
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)pEntry->pAd;
#endif

	if (pEntry->bfState == WAIT_SNDG_FB0)
	{
		/*record the snr comb*/
		pEntry->sndg0Snr0 = -128;
		pEntry->sndg0Snr1 = -128;
		pEntry->sndg0Snr2 = -128;
		pEntry->sndg0Mcs = pEntry->sndgMcs;
		//DBGPRINT(RT_DEBUG_TRACE,("   ETxBF: timer of WAIT_SNDG_FB0 expires\n" ));
#ifdef ETXBF_EN_COND3_SUPPORT
			if (pEntry->eTxBfEnCond == 1 || pEntry->eTxBfEnCond == 2)
				pEntry->bfState =READY_FOR_SNDG0;
			else if (pEntry->eTxBfEnCond == 3)
				txSndgOtherGroup(pAd, pEntry);
#else
		pEntry->bfState =READY_FOR_SNDG0;
#endif
	}
#ifdef ETXBF_EN_COND3_SUPPORT
	else if (pEntry->bfState == WAIT_SNDG_FB1)
	{
		/*record the snr comb*/
		pEntry->sndg1Snr0 = -128;
		pEntry->sndg1Snr1 = -128;
		pEntry->sndg1Snr2 = -128;
		pEntry->sndg1Mcs = pEntry->sndgMcs;
		//DBGPRINT(RT_DEBUG_TRACE,("   ETxBF: timer of WAIT_SNDG_FB1 expires, run txMrqInvTxBF()\n" ));
		txMrqInvTxBF(pAd, pEntry);
	}
	else if (pEntry->bfState == WAIT_MFB)
	{
		DBGPRINT(RT_DEBUG_TRACE,("   ETxBF: timer of WAIT_MFB expires, run chooseBestMethod()\n" ));
		chooseBestMethod(pAd, pEntry, 0);
	}
	else if (pEntry->bfState == WAIT_BEST_SNDG)
	{
		DBGPRINT(RT_DEBUG_TRACE,("   ETxBF: timer of WAIT_BEST_SNDG expires, run rxBestSndg()\n" ));
		rxBestSndg(pAd, pEntry);
	}
#endif /* ETXBF_EN_COND3_SUPPORT */
}

/* MlmeTxBfAllowed - returns true if ETxBF or ITxBF is supported and pTxRate is a valid BF mode */
BOOLEAN MlmeTxBfAllowed(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate)
{
	/* ETxBF */
	if ((pEntry->eTxBfEnCond > 0) &&
		(pTxRate->Mode == MODE_HTMIX || pTxRate->Mode == MODE_HTGREENFIELD)
#ifdef DBG_CTRL_SUPPORT
		&& (!((pAd->CommonCfg.DebugFlags & DBF_NO_TXBF_3SS) && pTxRate->CurrMCS>20))
#endif /* DBG_CTRL_SUPPORT */
	)
		return TRUE;

	/* ITxBF */
	if (pEntry->iTxBfEn && pTxRate->CurrMCS<16 && pTxRate->Mode!=MODE_CCK)
		return TRUE;

	return FALSE;
}

