/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	rt_ate.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/
#include "rt_config.h"

#define ATE_BBP_REG_NUM	168
UCHAR restore_BBP[ATE_BBP_REG_NUM]={0};

/* 802.11 MAC Header, Type:Data, Length:24bytes + 6 bytes QOS/HTC + 2 bytes padding */
UCHAR TemplateFrame[32] = {0x08,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x00,0xAA,0xBB,0x12,0x34,0x56,0x00,0x11,0x22,0xAA,0xBB,0xCC,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

extern FREQUENCY_ITEM *FreqItems3020;
extern UCHAR NUM_OF_3020_CHNL;

#define TXCONT_TX_PIN_CFG_A 0x041C0050
#define TXCONT_TX_PIN_CFG_G 0x081C00A0

#define ATE_TASK_EXEC_INTV 100
#define ATE_TASK_EXEC_MULTIPLE 10

static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1}; /* CCK Mode. */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1}; /* OFDM Mode. */
#ifdef DOT11N_SS3_SUPPORT
static CHAR HTMIXRateTable3T3R[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1}; /* HT Mix Mode for 3*3. */
#endif /* DOT11N_SS3_SUPPORT */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 32, -1}; /* HT Mix Mode. */
#ifdef DOT11_VHT_AC
static CHAR VHTACRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1}; /* VHT AC Mode. */
#endif /* DOT11_VHT_AC */

#ifdef RTMP_INTERNAL_TX_ALC

/* The desired TSSI over CCK */
extern CHAR desiredTSSIOverCCK[4];

/* The desired TSSI over OFDM */
extern CHAR desiredTSSIOverOFDM[8];

/* The desired TSSI over HT */
extern CHAR desiredTSSIOverHT[8];

/* The desired TSSI over HT using STBC */
extern CHAR desiredTSSIOverHTUsingSTBC[8];

/* The Tx power tuning entry*/
extern TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTable[];

#if defined( RT3352) || defined(RT3350) || defined(MT7601)
/*
==========================================================================
	Description:
		Get the desired TSSI based on ATE setting.

	Arguments:
		pAd

	Return Value:
		The desired TSSI
==========================================================================
 */
CHAR ATEGetDesiredTSSI(
	IN struct rtmp_adapter *	pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR desiredTSSI = 0;
	UCHAR MCS = 0;
	UCHAR MaxMCS = 7;
	UCHAR phy_mode = 0, stbc = 0, bw = 0;


#if defined( RT3352) || defined(RT3350)
	if (phy_mode == MODE_CCK)
	{
		if (MCS > 3) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n",
				__FUNCTION__,
				MCS));

			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverCCK[MCS];
	}
	else if (phy_mode == MODE_OFDM)
	{
		if (MCS > 7) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n",
				__FUNCTION__,
				MCS));

			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((phy_mode == MODE_HTMIX) || (phy_mode == MODE_HTGREENFIELD))
	{
		if (stbc == STBC_NONE)
		{
			if (MCS > MaxMCS) /* boundary verification */
			{
				DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n",
					__FUNCTION__,
					MCS));

				MCS = 0;
			}

			desiredTSSI = desiredTSSIOverHT[MCS];
		}
		else
		{
			if (MCS > MaxMCS) /* boundary verification */
			{
				DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n",
					__FUNCTION__,
					MCS));

				MCS = 0;
			}

			desiredTSSI = desiredTSSIOverHTUsingSTBC[MCS];
		}


		/*
			For HT BW40 MCS 7 with/without STBC configuration,
			the desired TSSI value should subtract one from the formula.
		*/
		if ((bw == BW_40) && (MCS == MCS_7))
		{
			desiredTSSI -= 1;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, Latest Tx setting: MODE = %d, MCS = %d, STBC = %d\n",
		__FUNCTION__,
		desiredTSSI,
		phy_mode,
		MCS,
		stbc));


	return desiredTSSI;
#endif /* defined( RT3352) || defined(RT3350) */
}
#endif /* defined( RT3352) || defined(RT3350) || defined(MT7601) */
#endif /* RTMP_INTERNAL_TX_ALC */


/*
==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in ATE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. TxPowerPercentage
		1. auto calibration based on TSSI feedback
		2. extra 2 db for CCK
		3. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

==========================================================================
*/
VOID ATEAsicAdjustTxPower(
	IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->AdjustTxPower != NULL)
		pATEInfo->pChipStruct->AdjustTxPower(pAd);
	else
		DBGPRINT_ERR(("%s: AdjustTxPower() for this chipset does not exist !\n", __FUNCTION__));

	return;
}


CHAR ATEConvertToRssi(
	IN struct rtmp_adapter *pAd,
	IN	CHAR	Rssi,
	IN  UCHAR   RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;
	CHAR	BaseVal;

	/* Rssi equals to zero should be an invalid value */
	if (Rssi == 0 || (RssiNumber >= 3))
		return -99;

	LNAGain = GET_LNA_GAIN(pAd);
	if (pAd->LatchRfRegs.Channel > 14)
		RssiOffset = pAd->ARssiOffset[RssiNumber];
	else
		RssiOffset = pAd->BGRssiOffset[RssiNumber];

	BaseVal = -12;

#ifdef RT65xx
	if (IS_RT65XX(pAd)) {
		if (IS_MT76x2(pAd)) {
			if (is_external_lna_mode(pAd, pAd->LatchRfRegs.Channel) == TRUE)
				LNAGain = 0;

			if (pAd->LatchRfRegs.Channel > 14)
				return (Rssi + pAd->ARssiOffset[RssiNumber] - (CHAR)LNAGain);
			else
				return (Rssi + pAd->BGRssiOffset[RssiNumber] - (CHAR)LNAGain);
		} else
			return (Rssi - LNAGain - RssiOffset);
	}
#endif /* RT65xx */
		return (BaseVal - RssiOffset - LNAGain - Rssi);
}


VOID ATESampleRssi(
	IN struct rtmp_adapter *pAd,
	IN RXWI_STRUC *pRxWI)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR rssi[3] = {0};
	CHAR snr[3] = {0};

	rssi[0] = pRxWI->RXWI_N.rssi[0];
	rssi[1] = pRxWI->RXWI_N.rssi[1];
	rssi[2] = pRxWI->RXWI_N.rssi[2];

	if ( IS_MT76x2(pAd) ) {
		snr[0] = pRxWI->RXWI_N.bbp_rxinfo[2];
		snr[1] = pRxWI->RXWI_N.bbp_rxinfo[3];
		snr[2] = pRxWI->RXWI_N.bbp_rxinfo[4];
	} else {
		snr[0] = pRxWI->RXWI_N.bbp_rxinfo[0];
		snr[1] = pRxWI->RXWI_N.bbp_rxinfo[1];
		snr[2] = pRxWI->RXWI_N.bbp_rxinfo[2];
		}

	if (rssi[0] != 0)
	{
		pATEInfo->LastRssi0	= ATEConvertToRssi(pAd, rssi[0], RSSI_0);
		pATEInfo->AvgRssi0X8 = (pATEInfo->AvgRssi0X8 - pATEInfo->AvgRssi0) + pATEInfo->LastRssi0;
		pATEInfo->AvgRssi0 = pATEInfo->AvgRssi0X8 >> 3;
	}

	if (rssi[1]!= 0)
	{
		pATEInfo->LastRssi1	= ATEConvertToRssi(pAd, rssi[1], RSSI_1);
		pATEInfo->AvgRssi1X8 = (pATEInfo->AvgRssi1X8 - pATEInfo->AvgRssi1) + pATEInfo->LastRssi1;
		pATEInfo->AvgRssi1 = pATEInfo->AvgRssi1X8 >> 3;
	}

	if (rssi[2]!= 0)
	{
		pATEInfo->LastRssi2	= ATEConvertToRssi(pAd, rssi[2], RSSI_2);
		pATEInfo->AvgRssi2X8 = (pATEInfo->AvgRssi2X8 - pATEInfo->AvgRssi2) + pATEInfo->LastRssi2;
		pATEInfo->AvgRssi2 = pATEInfo->AvgRssi2X8 >> 3;
	}

	pATEInfo->LastSNR0 = snr[0];
	pATEInfo->LastSNR1 = snr[1];
#ifdef DOT11N_SS3_SUPPORT
	pATEInfo->LastSNR2 = snr[2];
#endif /* DOT11N_SS3_SUPPORT */

	pATEInfo->NumOfAvgRssiSample ++;

	return;
}


VOID rt_ee_read_all(struct rtmp_adapter *pAd, USHORT *Data)
{
	USHORT offset = 0;
	USHORT value;

	for (offset = 0; offset < (EEPROM_SIZE >> 1);)
	{
		RT28xx_EEPROM_READ16(pAd, (offset << 1), value);
		Data[offset] = value;
		offset++;
	}

	return;
}


VOID rt_ee_write_all(struct rtmp_adapter *pAd, USHORT *Data)
{
	USHORT offset = 0;
	USHORT value;

#ifdef RTMP_USB_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_USB)
	{
		{
		USHORT offset = 0;
		USHORT length = EEPROM_SIZE;

		RTUSBWriteEEPROM(pAd, offset, (UCHAR *)Data, length);
		return;
	}
	}
#endif /* RTMP_USB_SUPPORT */

	for (offset = 0; offset < (EEPROM_SIZE >> 1);)
	{
		value = Data[offset];
		RT28xx_EEPROM_WRITE16(pAd, (offset << 1), value);
		offset++;
	}

	return;
}


VOID rt_ee_write_bulk(struct rtmp_adapter *pAd, USHORT *Data, USHORT offset, USHORT length)
{
	USHORT pos;
	USHORT value;
	USHORT len = length;

	for (pos = 0; pos < (len >> 1);)
	{
		value = Data[pos];
		RT28xx_EEPROM_WRITE16(pAd, offset+(pos*2), value);
		pos++;
	}

	return;
}

#ifdef RT_RF
VOID RtmpRfIoWrite(
	IN struct rtmp_adapter *pAd)
{
	/* Set RF value 1's set R3[bit2] = [0] */
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RtmpusecDelay(200);

	/* Set RF value 2's set R3[bit2] = [1] */
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RtmpusecDelay(200);

	/* Set RF value 3's set R3[bit2] = [0] */
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	return;
}
#endif /* RT_RF */

VOID ATEAsicSetTxRxPath(
    IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->AsicSetTxRxPath != NULL)
		pATEInfo->pChipStruct->AsicSetTxRxPath(pAd);

	return;
}

VOID BbpSoftReset(
	IN struct rtmp_adapter *pAd)
{
	return;
}

#ifdef MT76x2
VOID ITxBfBbpInit(
	IN struct rtmp_adapter *pAd)
{
	RTMP_IO_WRITE32(pAd, AGC1_R0, 0x00007408);
	RTMP_IO_WRITE32(pAd, TXO_R0,  0x00000020);
	RTMP_IO_WRITE32(pAd, TXBE_R0, 0x00000000);
	RTMP_IO_WRITE32(pAd, TXBE_R4, 0x00000008);
	RTMP_IO_WRITE32(pAd, CAL_R1, 0x00000006);
	RTMP_IO_WRITE32(pAd, CAL_R2, 0x00000005);
	RTMP_IO_WRITE32(pAd, CAL_R3, 0x00000000);
	RTMP_IO_WRITE32(pAd, CAL_R4, 0x00000000);
	RTMP_IO_WRITE32(pAd, CAL_R5, 0x00000400);
	RTMP_IO_WRITE32(pAd, CAL_R6, 0x00000000);
	RTMP_IO_WRITE32(pAd, CAL_R7, 0x00000000);
	RTMP_IO_WRITE32(pAd, MAC_CSR0, 0x00000000);
	return;
}
#endif

static int CheckMCSValid(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Mode,
	IN UCHAR Mcs)
{
	int index;
	PCHAR pRateTab = NULL;

	switch (Mode)
	{
		case MODE_CCK:
			pRateTab = CCKRateTable;
			break;
		case MODE_OFDM:
			pRateTab = OFDMRateTable;
			break;

		case 2: /*MODE_HTMIX*/
		case 3: /*MODE_HTGREENFIELD*/
#ifdef DOT11N_SS3_SUPPORT
			if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
				pRateTab = HTMIXRateTable3T3R;
			else
#endif /* DOT11N_SS3_SUPPORT */
				pRateTab = HTMIXRateTable;
			break;
#ifdef DOT11_VHT_AC
		case MODE_VHT:
			pRateTab = VHTACRateTable;
			break;
#endif /* DOT11_VHT_AC */
		default:
			DBGPRINT_ERR(("unrecognizable Tx Mode %d\n", Mode));
			return -1;
			break;
	}

	index = 0;
	while (pRateTab[index] != -1)
	{
		if (pRateTab[index] == Mcs)
			return 0;
		index++;
	}

	return -1;
}

/*
========================================================================

	Routine Description:
		Set Japan filter coefficients if needed.
	Note:
		This routine should only be called when
		entering TXFRAME mode or TXCONT mode.

========================================================================
*/
static VOID SetJapanFilter(struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR bw = 0, phy_mode = 0;

	bw = pATEInfo->TxWI.TXWI_N.BW;
	phy_mode = pATEInfo->TxWI.TXWI_N.PHYMODE;

	return;
}


/*
========================================================================

	Routine Description:
		Disable protection for ATE.
========================================================================
*/
VOID ATEDisableAsicProtect(
	IN		struct rtmp_adapter *pAd)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	uint32_t Protect[6];
	USHORT			offset;
	UCHAR			step;
	uint32_t MacReg = 0;

	/* Config ASIC RTS threshold register */
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	MacReg |= (0xFFF << 8);
	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	/* Initial common protection settings */
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	/* Handle legacy(B/G) protection */
	ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;
	ProtCfg.field.ProtectCtrl = 0;
	Protect[0] = ProtCfg.word;
	Protect[1] = ProtCfg.word;
	/* CTS-self is not used */
	pAd->FlgCtsEnabled = 0;

	/*
		NO PROTECT
			1.All STAs in the BSS are 20/40 MHz HT
			2. in a 20/40MHz BSS
			3. all STAs are 20MHz in a 20MHz BSS
		Pure HT. no protection.
	*/
	/*
		MM20_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 010111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
	*/
	Protect[2] = 0x01744004;

	/*
		MM40_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 111111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
	*/
	Protect[3] = 0x03f44084;

	/*
		CF20_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 010111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
	*/
	Protect[4] = 0x01744004;

	/*
		CF40_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 111111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
	*/
	Protect[5] = 0x03f44084;

	pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

	offset = CCK_PROT_CFG;
	for (step = 0;step < 6;step++)
		RTMP_IO_WRITE32(pAd, offset + step*4, Protect[step]);

	return;
}


#ifdef CONFIG_AP_SUPPORT
/*
==========================================================================
	Description:
		Used only by ATE to disassociate all STAs and stop AP service.
	Note:
==========================================================================
*/
VOID ATEAPStop(
	IN struct rtmp_adapter *pAd)
{
	BOOLEAN     Cancelled;
	uint32_t 	Value = 0;
	INT         apidx = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("!!! ATEAPStop !!!\n"));

	/* To prevent MCU to modify BBP registers w/o indications from driver. */
#ifdef DFS_SUPPORT
		NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */



#ifdef APCLI_SUPPORT
	ApCliIfDown(pAd);
#endif /* APCLI_SUPPORT */

	MacTableReset(pAd);

	/* Disable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=0xe;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	/* Disable piggyback */
	RTMPSetPiggyBack(pAd, FALSE);

	ATEDisableAsicProtect(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		AsicDisableSync(pAd);

	}

#ifdef RTMP_MAC_USB
	/* For USB, we need to clear the beacon sync buffer. */
	RTUSBBssBeaconExit(pAd);
#endif /* RTMP_MAC_USB */

	for (apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++)
	{
		if (pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning == TRUE)
		{
			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].REKEYTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning = FALSE;
		}
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
	}


	/* Cancel the Timer, to make sure the timer was not queued. */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

	if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE)
		RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);



#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPEnable == TRUE)
	{
		RTMP_CHIP_DISABLE_AP_MIMOPS(pAd);
		pAd->ApCfg.GreenAPLevel=GREENAP_WITHOUT_ANY_STAS_CONNECT;
		pAd->ApCfg.bGreenAPEnable = FALSE;
	}
#endif /* GREENAP_SUPPORT */



}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID RTMPStationStop(
    IN  struct rtmp_adapter *  pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStop\n"));


    /* Do nothing. */

    DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStop\n"));
}


VOID RTMPStationStart(
    IN  struct rtmp_adapter *  pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStart\n"));


	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStart\n"));
}
#endif /* CONFIG_STA_SUPPORT */




#ifdef RLT_BBP
static INT ate_bbp_core_soft_reset(struct rtmp_adapter *pAd, BOOLEAN set_bw, INT bw)
{
	uint32_t bbp_val;

	RTMP_BBP_IO_READ32(pAd, CORE_R4, &bbp_val);
	bbp_val |= 0x1;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R4, bbp_val);
	RtmpusecDelay(1000);

	if (set_bw == TRUE)
	{
		RTMP_BBP_IO_READ32(pAd, CORE_R1, &bbp_val);
		bbp_val &= (~0x18);
		switch (bw)
		{
			case BW_40:
				bbp_val |= 0x10;
				break;
			case BW_80:
				bbp_val |= 0x18;
				break;
			case BW_20:
			default:
					break;
		}
		RTMP_BBP_IO_WRITE32(pAd, CORE_R1, bbp_val);
		RtmpusecDelay(1000);
	}
	RTMP_BBP_IO_READ32(pAd, CORE_R4, &bbp_val);
	bbp_val &= (~0x1);
	RTMP_BBP_IO_WRITE32(pAd, CORE_R4, bbp_val);
	RtmpusecDelay(1000);

	return 0;
}
#endif /* RLT_BBP */

static int TXCARR(
	IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t 		MacData=0;
	int 	Status = NDIS_STATUS_SUCCESS;
#ifdef RLT_BBP
	uint32_t bbp_val;
#endif /* RLT_BBP */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pATEInfo->Mode = ATE_TXCARR;
#ifdef RTMP_MAC_USB
	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
#endif /* RTMP_MAC_USB */

	/* QA has done the following steps if it is used. */
	if (pATEInfo->bQATxStart == FALSE)
	{
		if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd))
			&& (!IS_RT5350(pAd)) && (!IS_RT3593(pAd)) && (!IS_MT7610(pAd)))
			BbpSoftReset(pAd);/* Soft reset BBP. */

		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			if (!IS_RT5592(pAd))
			{
				/* store the original value of TX_PIN_CFG */
				RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));

				/* give TX_PIN_CFG(0x1328) a proper value. */
			if (pATEInfo->Channel <= 14)
			{
				/* G band */
				MacData = TXCONT_TX_PIN_CFG_G;
				if (IS_RT6352(pAd))
					MacData &= 0xFFEFFFFF;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
			}
			else
			{
				/* A band */
				MacData = TXCONT_TX_PIN_CFG_A;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
				}
			}

#ifdef RLT_BBP
#ifdef RT65xx
			if (IS_MT7610(pAd) || IS_MT76x2(pAd))
			{
				RTMP_BBP_IO_READ32(pAd, CORE_R24, &bbp_val);
				bbp_val |= (0x1 << 14);
				bbp_val |= (0x1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, CORE_R24, bbp_val);
			}
#endif /* RT65xx */
#endif /* RLT_BBP */
		}
		else
		{
			// TODO: how to handle this for RLT_BBP?

			/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
			ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static int TXCONT(
	IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t 		MacData=0;
	int 	Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;
	struct ate_chip_struct *pChipStruct = pATEInfo->pChipStruct;
#ifdef RLT_BBP
	uint32_t bbp_val;
#endif /* RLT_BBP */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pATEInfo->Mode = ATE_TXCONT;

	if (pATEInfo->bQATxStart == TRUE)
	{
		/*
			set MAC_SYS_CTRL(0x1004) bit4(Continuous Tx Production Test)
			and bit2(MAC TX enable) back to zero.
		*/
		ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
		ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	}
	else
	{
		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			if (!IS_RT5592(pAd))
			{
				/* store the original value of TX_PIN_CFG */
				RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));
			}
		}
	}

	/* Step 1: send 50 packets first. */
	pATEInfo->TxCount = 50;

	if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)) && (!IS_RT3593(pAd)) && (!IS_RT65XX(pAd)))
		BbpSoftReset(pAd);/* Soft reset BBP. */

	/* Abort Tx, RX DMA. */
	RtmpDmaEnable(pAd, 0);


	/* Do it after Tx/Rx DMA is aborted. */
	pATEInfo->TxDoneCount = 0;

	/* Only needed if we have to send some normal frames. */
	if (pATEInfo->bQAEnabled == FALSE)
		SetJapanFilter(pAd);


#ifdef RTMP_MAC_USB
	/* Setup frame format. */
	ATESetUpFrame(pAd, 0);
#endif /* RTMP_MAC_USB */

	/* Enable Tx */
	ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Start Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);

#ifdef RTMP_MAC_USB
	InterlockedExchange(&pAd->BulkOutRemained, pATEInfo->TxCount);
#endif /* RTMP_MAC_USB */



#ifdef RTMP_MAC_USB
	NdisAcquireSpinLock(&pAd->GenericLock);
	pAd->ContinBulkOut = FALSE;
	NdisReleaseSpinLock(&pAd->GenericLock);

	RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);

	/* Kick bulk out */
	RTUSBKickBulkOut(pAd);

	/* To make sure all the 50 frames have been bulk out before executing step 2 */
	while (atomic_read(&pAd->BulkOutRemained) > 0)
	{
		RtmpOsMsDelay(5);
	}
#endif /* RTMP_MAC_USB */

	if (pATEInfo->TxMethod == TX_METHOD_1)
	{
		if (!IS_RT5592(pAd))
		{
			/* give TX_PIN_CFG(0x1328) a proper value. */
		if (pATEInfo->Channel <= 14)
		{
			/* G band */
			MacData = TXCONT_TX_PIN_CFG_G;
			if (IS_RT6352(pAd))
				MacData &= 0xFFEFFFFF;
				RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
		}
		else
		{
			/* A band */
			MacData = TXCONT_TX_PIN_CFG_A;
				RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
			}
		}
#ifdef RLT_BBP
#ifdef RT65xx
		if (IS_MT7610(pAd) || IS_MT76x2(pAd))
		{
			RTMP_BBP_IO_READ32(pAd, CORE_R24, &bbp_val);
			bbp_val |= (0x1 << 15);
			RTMP_BBP_IO_WRITE32(pAd, CORE_R24, bbp_val);
		}
		if ( IS_MT76x2(pAd) )
			RTMP_IO_WRITE32(pAd, DACCLK_EN_DLY_CFG, 0x80008000);
#endif /* RT65xx */
#endif /* RLT_BBP */

#ifdef RT5592EP_SUPPORT
		/* enable continuous tx production test */
		if (pAd->chipCap.Priv == RT5592_TYPE_EP)
			ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
#endif /* RT5592EP_SUPPORT */
	}
	else
	{
		/* Step 2: send more 50 packets then start Continuous Tx Mode. */
		/* Abort Tx, RX DMA. */
		RtmpDmaEnable(pAd, 0);

		pATEInfo->TxCount = 50;

		pATEInfo->TxDoneCount = 0;
		if (pATEInfo->bQAEnabled == FALSE)
			SetJapanFilter(pAd);


#ifdef RTMP_MAC_USB
		/* Build up Tx frame. */
		ATESetUpFrame(pAd, 0);
#endif /* RTMP_MAC_USB */

		/* Enable Tx */
		ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

		/* Disable Rx */
		ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

		/* Start Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 1);

#ifdef RTMP_MAC_USB
		InterlockedExchange(&pAd->BulkOutRemained, pATEInfo->TxCount);
#endif /* RTMP_MAC_USB */


#ifdef RTMP_MAC_USB
		NdisAcquireSpinLock(&pAd->GenericLock);
		pAd->ContinBulkOut = FALSE;
		NdisReleaseSpinLock(&pAd->GenericLock);

		RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);
		/* Kick bulk out */
		RTUSBKickBulkOut(pAd);

		/* Let pAd->BulkOutRemained be consumed to zero. */
#endif /* RTMP_MAC_USB */
		RtmpusecDelay(500);

		/* enable continuous tx production test */
		ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static int TXCARS(
        IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t 		MacData=0;
	int 	Status = NDIS_STATUS_SUCCESS;
	struct ate_chip_struct *pChipStruct = pATEInfo->pChipStruct;
#ifdef RLT_BBP
	uint32_t bbp_val;
#endif /* RLT_BBP */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pATEInfo->Mode = ATE_TXCARRSUPP;
#ifdef RTMP_MAC_USB
	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
#endif /* RTMP_MAC_USB */

	/* QA has done the following steps if it is used. */
	if (pATEInfo->bQATxStart == FALSE)
	{
#if !defined(RT3883) && !defined(RT3593) && !defined(RT65xx)
		if (!IS_RT3883(pAd) && !IS_RT3593(pAd) && !IS_MT7610(pAd))
		{
			BbpSoftReset(pAd);
		}
#endif /* !defined(RT3883) && !defined(RT3593) && !defined(RT65xx) */

		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			if (!IS_RT5592(pAd))
			{
				/* store the original value of TX_PIN_CFG */
				RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));

				/* give TX_PIN_CFG(0x1328) a proper value. */
			if (pATEInfo->Channel <= 14)
			{
				/* G band */
				MacData = TXCONT_TX_PIN_CFG_G;
				if (IS_RT6352(pAd))
					MacData &= 0xFFEFFFFF;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
			}
			else
			{
				/* A band */
				MacData = TXCONT_TX_PIN_CFG_A;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
				}
			}

#ifdef RLT_BBP
#ifdef RT65xx
			if (IS_MT7610(pAd) || IS_MT76x2(pAd))
			{
				RTMP_BBP_IO_READ32(pAd, CORE_R24, &bbp_val);
				bbp_val |= (0x1 << 15);
				RTMP_BBP_IO_WRITE32(pAd, CORE_R24, bbp_val);

				RTMP_BBP_IO_READ32(pAd, TXC_R1, &bbp_val);
				bbp_val |= (0x1 << 0);
				RTMP_BBP_IO_WRITE32(pAd, TXC_R1, bbp_val);
			}
#endif /* RT65xx */
#endif /* RLT_BBP */


		}
		else
		{
			// TODO: how to handle this for RLT_BBP??

			/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
			ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static int TXFRAME(
	IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t 		MacData=0;
#ifdef RTMP_MAC_USB
	ULONG			IrqFlags = 0;
#endif /* RTMP_MAC_USB */
	int 	Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;
	STRING			IPGStr[8] = {0};
#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	UCHAR		RFValue, BBP49Value;
	CHAR		ChannelPower = pATEInfo->TxPower0;
	CHAR		*TssiRefPerChannel = pATEInfo->TssiRefPerChannel;
	UCHAR		CurrentChannel = pATEInfo->Channel;
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	if (pATEInfo->bTSSICalbrEnableG == TRUE)
	{
		if ((!IS_RT3350(pAd)) && (!IS_RT3352(pAd)))
		{
			DBGPRINT_ERR(("Not support TSSI calibration since not 3350/3352 chip!!!\n"));
			Status = NDIS_STATUS_FAILURE;

			return Status;
		}

		/* Internal TSSI 0 */
		RFValue = (0x3 | 0x0 << 2 | 0x3 << 4);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R27, RFValue);

		RFValue = (0x3 | 0x0 << 2);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R28, RFValue);

		/* set BBP R49[7] = 1 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
		BBP49Value = BbpData;
		BbpData |= 0x80;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpData);
	}
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s(Count=%u)\n", __FUNCTION__, pATEInfo->TxCount));
	//pATEInfo->Mode |= ATE_TXFRAME;

	if (pATEInfo->bQATxStart == FALSE)
	{

		/* set IPG to sync tx power with QA tools */
		/* default value of IPG is 200 */
		snprintf(IPGStr, sizeof(IPGStr), "%u", pATEInfo->IPG);
		DBGPRINT(RT_DEBUG_TRACE, ("IPGstr=%s\n", IPGStr));
		Set_ATE_IPG_Proc(pAd, IPGStr);
	}

	ATE_ASIC_CALIBRATION(pAd, ATE_TXFRAME);


#ifdef RTMP_MAC_USB
	/* Soft reset BBP. */
	BbpSoftReset(pAd);

	/* Clear bit4 to stop continuous Tx production test. */
	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
#endif /* RTMP_MAC_USB */

#ifdef TXBF_SUPPORT
	/* Enable Rx if Sending Sounding. Otherwise Disable */
	if (pATEInfo->txSoundingMode != 0)
	{
		ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
	}
	else
#endif /* TXBF_SUPPORT */
	{
		/* Disable Rx */
		ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
	}


#ifdef RTMP_MAC_USB
	/* Enable Tx */
	ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

	if (pATEInfo->bQAEnabled == FALSE)
		SetJapanFilter(pAd);

	/* Abort Tx, RX DMA. */
	RtmpDmaEnable(pAd, 0);

	pATEInfo->TxDoneCount = 0;

	/* Setup frame format. */
	ATESetUpFrame(pAd, 0);

	/* Start Tx, RX DMA. */
	RtmpDmaEnable(pAd, 1);

	/* Check count is continuous or not yet. */
	if (pATEInfo->TxCount == 0)
	{
		InterlockedExchange(&pAd->BulkOutRemained, 0);
	}
	else
	{
		InterlockedExchange(&pAd->BulkOutRemained, pATEInfo->TxCount);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("bulk out count = %d\n", atomic_read(&pAd->BulkOutRemained)));
	ASSERT((atomic_read(&pAd->BulkOutRemained) >= 0));

	if (atomic_read(&pAd->BulkOutRemained) == 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Send packet continuously\n"));

		/* NdisAcquireSpinLock() == spin_lock_bh() */
		/* NdisAcquireSpinLock only need one argument. */
		NdisAcquireSpinLock(&pAd->GenericLock);
		pAd->ContinBulkOut = TRUE;
		NdisReleaseSpinLock(&pAd->GenericLock);

		/* BULK_OUT_LOCK() == spin_lock_irqsave() */
		BULK_OUT_LOCK(&pAd->BulkOutLock[0], IrqFlags);
		pAd->BulkOutPending[0] = FALSE;
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Send packets depend on counter\n"));

		NdisAcquireSpinLock(&pAd->GenericLock);
		pAd->ContinBulkOut = FALSE;
		NdisReleaseSpinLock(&pAd->GenericLock);

		BULK_OUT_LOCK(&pAd->BulkOutLock[0], IrqFlags);
		pAd->BulkOutPending[0] = FALSE;
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);
	}

	RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);

	/* Kick bulk out */
	RTUSBKickBulkOut(pAd);
#endif /* RTMP_MAC_USB */

#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	if (pATEInfo->bTSSICalbrEnableG == TRUE)
	{
		if ((IS_RT3350(pAd)) || (IS_RT3352(pAd)))
		{
			if ((pATEInfo->TxWI.TXWI_O.MCS == 7)
				&& (pATEInfo->TxWI.TXWI_O.BW == BW_20)	&& (pATEInfo->TxAntennaSel == 1))
			{
				if (pATEInfo->Channel == 7)
				{
					/* step 1: get calibrated channel 7 TSSI reading as reference */
					RtmpOsMsDelay(500);
					DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));

					/* read BBP R49[4:0] and write to EEPROM 0x6E */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
					DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%02x\n", BbpData));
					BbpData &= 0x1f;
					TssiRefPerChannel[CurrentChannel-1] = BbpData;
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));
				}

				/* step 2: calibrate channel 1 and 13 TSSI delta values */
				else if (pATEInfo->Channel == 1)
				{
					/* Channel 1 */
					RtmpOsMsDelay(500);
					DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));

					/* read BBP R49[4:0] */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
					DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%02x\n", BbpData));
					BbpData &= 0x1f;
					TssiRefPerChannel[CurrentChannel-1] = BbpData;
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));
				}
				else if (pATEInfo->Channel == 13)
				{
					/* Channel 13 */
					RtmpOsMsDelay(500);
					DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));

					/* read BBP R49[4:0] */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
					DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%02x\n", BbpData));
					BbpData &= 0x1f;
					TssiRefPerChannel[CurrentChannel-1] = BbpData;
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));
				}
				else
				{
					DBGPRINT(RT_DEBUG_OFF, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));
				}
			}
		}
	}
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */

	pATEInfo->Mode |= ATE_TXFRAME;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static int RXFRAME(
	IN struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t 		MacData=0;
	int 	Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_USB
	uint32_t 		ring_index=0;
#endif /* RTMP_MAC_USB */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	ATE_ASIC_CALIBRATION(pAd, ATE_RXFRAME);

	/* Disable Rx of MAC block */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Clear bit4 to stop continuous Tx production test. */
	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	pATEInfo->Mode |= ATE_RXFRAME;

#ifdef RTMP_MAC_USB
	if (pAd->chipCap.MCUType != ANDES )
	{
		/* Abort Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 0);
	}
#endif /* RTMP_MAC_USB */

	/* Disable Tx of MAC block. */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

#ifdef RTMP_MAC_USB
    /* Reset Rx RING */
	for (ring_index = 0; ring_index < (RX_RING_SIZE); ring_index++)
	{
		PRX_CONTEXT  pRxContext = &(pAd->RxContext[ring_index]);

		pRxContext->InUse = FALSE;
		pRxContext->IRPPending = FALSE;
		pRxContext->Readable = FALSE;

		/* Get the URB from kernel(i.e., host control driver) back to driver. */
		RTUSB_UNLINK_URB(pRxContext->pUrb);

		/* Sleep 200 microsecs to give cancellation time to work. */
		RtmpusecDelay(200);
		pAd->BulkInReq = 0;

		pAd->PendingRx = 0;
		/* Next Rx Read index */
		pAd->NextRxBulkInReadIndex = 0;
		/* Rx Bulk pointer */
		pAd->NextRxBulkInIndex		= RX_RING_SIZE - 1;
		pAd->NextRxBulkInPosition = 0;
	}

	/* read to clear counters */
	RTUSBReadMACRegister(pAd, RX_STA_CNT0, &MacData); /* RX PHY & RX CRC count */
	RTUSBReadMACRegister(pAd, RX_STA_CNT1, &MacData); /* RX PLCP error count & CCA false alarm count */
	RTUSBReadMACRegister(pAd, RX_STA_CNT2, &MacData); /* RX FIFO overflow frame count & RX duplicated filtered frame count */

	pAd->ContinBulkIn = TRUE;

	/* Enable Tx, RX DMA. */
	RtmpDmaEnable(pAd, 1);
#endif /* RTMP_MAC_USB */

	/* Enable Rx of MAC block. */
	ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);


#ifdef RTMP_MAC_USB
	/* Kick bulk in. */
	RTUSBBulkReceive(pAd);
#endif /* RTMP_MAC_USB */


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}

/*
==========================================================================
    Description:
        Set ATE ADDR1=DA for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR3=DA for TxFrame(STA : To DS = 1 ; From DS = 0)

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_DA_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	char *			value;
	INT					octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17)
		return FALSE;

	for (octet = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pATEInfo->Addr1[octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pATEInfo->Addr3[octet++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n",
		pATEInfo->Addr1[0], pATEInfo->Addr1[1], pATEInfo->Addr1[2], pATEInfo->Addr1[3],
		pATEInfo->Addr1[4], pATEInfo->Addr1[5]));

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n",
		pATEInfo->Addr3[0], pATEInfo->Addr3[1], pATEInfo->Addr3[2], pATEInfo->Addr3[3],
		pATEInfo->Addr3[4], pATEInfo->Addr3[5]));
#endif /* CONFIG_STA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_DA_Proc Success\n"));

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE ADDR3=SA for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR2=SA for TxFrame(STA : To DS = 1 ; From DS = 0)

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_SA_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	char *			value;
	INT					octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17)
		return FALSE;

	for (octet=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pATEInfo->Addr3[octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pATEInfo->Addr2[octet++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n",
		pATEInfo->Addr3[0], pATEInfo->Addr3[1], pATEInfo->Addr3[2], pATEInfo->Addr3[3],
		pATEInfo->Addr3[4], pATEInfo->Addr3[5]));
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n",
		pATEInfo->Addr2[0], pATEInfo->Addr2[1], pATEInfo->Addr2[2], pATEInfo->Addr2[3],
		pATEInfo->Addr2[4], pATEInfo->Addr2[5]));
#endif /* CONFIG_STA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_SA_Proc Success\n"));

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE ADDR2=BSSID for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR1=BSSID for TxFrame(STA : To DS = 1 ; From DS = 0)

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_BSSID_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	char *			value;
	INT					octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17)
		return FALSE;

	for (octet=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pATEInfo->Addr2[octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pATEInfo->Addr1[octet++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n",
		pATEInfo->Addr2[0], pATEInfo->Addr2[1], pATEInfo->Addr2[2], pATEInfo->Addr2[3],
		pATEInfo->Addr2[4], pATEInfo->Addr2[5]));

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n",
		pATEInfo->Addr1[0], pATEInfo->Addr1[1], pATEInfo->Addr1[2], pATEInfo->Addr1[3],
		pATEInfo->Addr1[4], pATEInfo->Addr1[5]));
#endif /* CONFIG_STA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_BSSID_Proc Success\n"));

	return TRUE;
}

/*
==========================================================================
    Description:
        Set ATE Tx Power for evaluation

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_POWER_EVALUATION_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{

	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->TxPwrEvaluation!= NULL)
		pATEInfo->pChipStruct->TxPwrEvaluation(pAd);
	else
		return FALSE;

	return TRUE;
}

/*
==========================================================================
    Description:
        Set ATE Tx Antenna

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_Antenna_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR value;
	INT maximun_index = pAd->Antenna.field.TxPath;

	value = simple_strtol(arg, 0, 10);

	if ((value > maximun_index) || (value < 0))
	{
		DBGPRINT_ERR(("Set_ATE_TX_Antenna_Proc::Out of range (Value=%d)\n", value));
		DBGPRINT_ERR(("Set_ATE_TX_Antenna_Proc::The range is 0~%d\n", maximun_index));

		return FALSE;
	}

	pATEInfo->TxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_Antenna_Proc (Antenna = %d)\n", pATEInfo->TxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Rx Antenna

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_RX_Antenna_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR value;
	INT maximun_index = pAd->Antenna.field.RxPath;

	value = simple_strtol(arg, 0, 10);

	if ((value > maximun_index) || (value < 0))
	{
		DBGPRINT_ERR(("Set_ATE_RX_Antenna_Proc::Out of range (Value=%d)\n", value));
		DBGPRINT_ERR(("Set_ATE_RX_Antenna_Proc::The range is 0~%d\n", maximun_index));

		return FALSE;
	}

	pATEInfo->RxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_Antenna_Proc (Antenna = %d)\n", pATEInfo->RxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


VOID DefaultATEAsicExtraPowerOverMAC(
	IN	struct rtmp_adapter *		pAd)
{
	uint32_t ExtraPwrOverMAC = 0;
	uint32_t ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
	RTMP_IO_READ32(pAd, 0x1318, &ExtraPwrOverMAC);
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, 0x131C, &ExtraPwrOverMAC);
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1324, &ExtraPwrOverMAC);
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);

	if (IS_RT5392(pAd))
	{
		/*  For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
		RTMP_IO_READ32(pAd, 0x1320, &ExtraPwrOverMAC);
		ExtraPwrOverTxPwrCfg8 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
		RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, ExtraPwrOverTxPwrCfg8);

		DBGPRINT(RT_DEBUG_TRACE, ("Offset =0x13D8, TxPwr = 0x%08X, ", (UINT)ExtraPwrOverTxPwrCfg8));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n",
		(UINT)ExtraPwrOverTxPwrCfg7,
		(UINT)ExtraPwrOverTxPwrCfg9));
}


VOID ATEAsicExtraPowerOverMAC(
	IN	struct rtmp_adapter *		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->AsicExtraPowerOverMAC!= NULL)
		pATEInfo->pChipStruct->AsicExtraPowerOverMAC(pAd);

	return;
}


VOID ATEAsicTemperCompensation(
	IN	struct rtmp_adapter *		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->TemperCompensation!= NULL)
		pATEInfo->pChipStruct->TemperCompensation(pAd);

	return;
}


#ifdef RT3350
/*
==========================================================================
    Description:
        Set ATE PA bias to improve EVM

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT Set_ATE_PA_Bias_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR PABias = 0;
	UCHAR RFValue;

	if (!IS_RT3350(pAd))
	{
		return FALSE;
	}

	PABias = simple_strtol(arg, 0, 10);

	if (PABias >= 16)
	{
		DBGPRINT_ERR(("Set_ATE_PA_Bias_Proc::Out of range, it should be in range of 0~15.\n"));
		return FALSE;
	}

	pATEInfo->PABias = PABias;

#ifdef RTMP_RF_RW_SUPPORT
#ifndef RLT_RF
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R19, (u8 *)&RFValue);
	RFValue = (((RFValue & 0x0F) | (pATEInfo->PABias << 4)));
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R19, (UCHAR)RFValue);
#endif /* !RLT_RF */
#endif /* RTMP_RF_RW_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_PA_Bias_Proc (PABias = %d)\n", pATEInfo->PABias));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_PA_Bias_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
#endif /* RT3350 */

/*
==========================================================================
    Description:
        Set ATE RF BW

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_BW_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT status = FALSE;
	UCHAR bw = 0;


	if (pATEInfo->pChipStruct->Set_BW_Proc != NULL)
	{
		status = pATEInfo->pChipStruct->Set_BW_Proc(pAd, arg);

	bw = pATEInfo->TxWI.TXWI_N.BW;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", bw));
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc %s\n", status == TRUE ? "success" : "failed"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Tx frame length

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_LENGTH_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	pATEInfo->TxLength = simple_strtol(arg, 0, 10);

	if ((pATEInfo->TxLength < 24) || (pATEInfo->TxLength > (MAX_FRAME_SIZE - 34/* == 2312 */)))
	{
		pATEInfo->TxLength = (MAX_FRAME_SIZE - 34/* == 2312 */);
		DBGPRINT_ERR(("Set_ATE_TX_LENGTH_Proc::Out of range, it should be in range of 24~%d.\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_LENGTH_Proc (TxLength = %d)\n", pATEInfo->TxLength));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_LENGTH_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Tx frame count

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_COUNT_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	pATEInfo->TxCount = simple_strtol(arg, 0, 10);


	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pATEInfo->TxCount));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_COUNT_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Tx frame MCS

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_MCS_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR MCS, phy_mode = 0;
	INT result;

	phy_mode = pATEInfo->TxWI.TXWI_N.PHYMODE;

	MCS = simple_strtol(arg, 0, 10);
	result = CheckMCSValid(pAd, phy_mode, MCS);

	if (result != -1) {
		pATEInfo->TxWI.TXWI_N.MCS = MCS;
	} else {
		DBGPRINT_ERR(("Set_ATE_TX_MCS_Proc::Out of range, refer to rate table.\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MCS_Proc (MCS = %d)\n", MCS));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MCS_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Tx frame STBC

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_STBC_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR stbc = simple_strtol(arg, 0, 10);

	if (stbc > 1)
	{
		DBGPRINT_ERR(("Set_ATE_TX_STBC_Proc::Out of range\n"));
		return FALSE;
	}

	pATEInfo->TxWI.TXWI_N.STBC = stbc;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_STBC_Proc (GI = %d)\n", stbc));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_STBC_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Tx frame Mode
        0: MODE_CCK
        1: MODE_OFDM
        2: MODE_HTMIX
        3: MODE_HTGREENFIELD

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_MODE_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR BbpData = 0;
	UCHAR phy_mode, bw = BW_20;

	phy_mode = simple_strtol(arg, 0, 10);

	if (phy_mode > MODE_VHT)
	{
		phy_mode = MODE_CCK;
		DBGPRINT_ERR(("Set_ATE_TX_MODE_Proc::Out of range.\nIt should be in range of 0~4\n"));
		DBGPRINT(RT_DEBUG_OFF, ("0: CCK, 1: OFDM, 2: HT_MIX, 3: HT_GREEN_FIELD, 4: VHT.\n"));
		return FALSE;
	}

	pATEInfo->TxWI.TXWI_N.PHYMODE = phy_mode;
	bw = pATEInfo->TxWI.TXWI_N.BW;

	if (phy_mode == MODE_CCK) {
		pATEInfo->TxWI.TXWI_N.BW = BW_20;
		bw = BW_20;
	}

#ifdef RT65xx
	/* Turn on BBP 20MHz mode by request here. */
	if (IS_MT7610(pAd))
	{
		return TRUE;
	}
	else
#endif /* RT65xx */
	/* Turn on BBP 20MHz mode by request here. */
	if (phy_mode == MODE_CCK)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Set_ATE_TX_MODE_Proc::CCK Only support 20MHZ. Switch to 20MHZ.\n"));
	}

#ifdef RT3350
	if (IS_RT3350(pAd))
	{
		if (phy_mode == MODE_CCK)
		{
			USHORT value;
			UCHAR  rf_offset;
			UCHAR  rf_value;

			RT28xx_EEPROM_READ16(pAd, 0x126, value);
			rf_value = value & 0x00FF;
			rf_offset = (value & 0xFF00) >> 8;

			if(rf_offset == 0xff)
			    rf_offset = RF_R21;
			if(rf_value == 0xff)
			    rf_value = 0x4F;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);

			RT28xx_EEPROM_READ16(pAd, 0x12a, value);
			rf_value = value & 0x00FF;
			rf_offset = (value & 0xFF00) >> 8;

			if(rf_offset == 0xff)
			    rf_offset = RF_R29;
			if(rf_value == 0xff)
			    rf_value = 0x07;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);


			/* set RF_R24 */
			if (bw == BW_40)
			{
				value = 0x3F;
			}
			else
			{
				value = 0x1F;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
		}
		else
		{
			USHORT value;
			UCHAR  rf_offset;
			UCHAR  rf_value;

			RT28xx_EEPROM_READ16(pAd, 0x124, value);
			rf_value = value & 0x00FF;
			rf_offset = (value & 0xFF00) >> 8;

			if(rf_offset == 0xff)
			    rf_offset = RF_R21;
			if(rf_value == 0xff)
			    rf_value = 0x6F;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);

			RT28xx_EEPROM_READ16(pAd, 0x128, value);
			rf_value = value & 0x00FF;
			rf_offset = (value & 0xFF00) >> 8;

			if(rf_offset == 0xff)
			    rf_offset = RF_R29;
			if(rf_value == 0xff)
			    rf_value = 0x07;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);

			/* set RF_R24 */
			if (bw == BW_40)
			{
				value = 0x28;
			}
			else
			{
				value = 0x18;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
		}
	}
#endif /* RT3350 */

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MODE_Proc (TxMode = %d)\n", phy_mode));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MODE_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE Tx frame GI

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_GI_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR sgi;

	sgi = simple_strtol(arg, 0, 10);

	pATEInfo->TxWI.TXWI_N.ShortGI= (sgi > 1 ? 0 : sgi);

	if (sgi > 1)
	{
		DBGPRINT_ERR(("Set_ATE_TX_GI_Proc::Out of range\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_GI_Proc (GI = %d)\n", sgi));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_GI_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


INT	Set_ATE_RX_FER_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	pATEInfo->bRxFER = simple_strtol(arg, 0, 10);

	if (pATEInfo->bRxFER == 1)
	{
		pATEInfo->RxCntPerSec = 0;
		pATEInfo->RxTotalCnt = 0;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_FER_Proc (bRxFER = %d)\n", pATEInfo->bRxFER));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_FER_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


INT Set_ATE_Read_RF_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue = 0;
	INT index=0;

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		for (index = 0; index < 32; index++)
		{
			RT30xxReadRFRegister(pAd, index, (u8 *)&RFValue);
			DBGPRINT(RT_DEBUG_OFF, ("R%d=%d\n",index,RFValue));
		}
	}
	else
#endif /* RTMP_RF_RW_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_OFF, ("R1 = %x\n", pAd->LatchRfRegs.R1));
		DBGPRINT(RT_DEBUG_OFF, ("R2 = %x\n", pAd->LatchRfRegs.R2));
		DBGPRINT(RT_DEBUG_OFF, ("R3 = %x\n", pAd->LatchRfRegs.R3));
		DBGPRINT(RT_DEBUG_OFF, ("R4 = %x\n", pAd->LatchRfRegs.R4));
	}
	return TRUE;
}


#ifndef RTMP_RF_RW_SUPPORT
INT Set_ATE_Write_RF1_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	uint32_t value = (uint32_t) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R1 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}


INT Set_ATE_Write_RF2_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	uint32_t value = (uint32_t) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R2 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}


INT Set_ATE_Write_RF3_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	uint32_t value = (uint32_t) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R3 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}


INT Set_ATE_Write_RF4_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	uint32_t value = (uint32_t) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R4 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}
#endif /* RTMP_RF_RW_SUPPORT */


/*
==========================================================================
    Description:
        Load and Write EEPROM from a binary file prepared in advance.

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT Set_ATE_Load_E2P_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	BOOLEAN		    	ret = FALSE;
	char *		src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD		srcf;
	int32_t 			retval;
	USHORT 			WriteEEPROM[(EEPROM_SIZE >> 1)];
	INT				FileLength = 0;
	uint32_t 			value = (uint32_t) simple_strtol(arg, 0, 10);
	RTMP_OS_FS_INFO	osFSInfo;

	DBGPRINT(RT_DEBUG_OFF, ("===> %s (value=%d)\n\n", __FUNCTION__, value));

	if (value > 0)
	{
		/* zero the e2p buffer */
		memset((u8 *)WriteEEPROM, 0, EEPROM_SIZE);

		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		do
		{
			/* open the bin file */
			srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(srcf))
			{
				DBGPRINT_ERR(("%s - Error opening file %s\n", __FUNCTION__, src));
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(srcf, (char *)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				DBGPRINT_ERR(("%s : error file length (=%d) in e2p.bin\n",
					   __FUNCTION__, FileLength));
				break;
			}
			else
			{
				/* write the content of .bin file to EEPROM */
				rt_ee_write_all(pAd, WriteEEPROM);
				ret = TRUE;
			}
			break;
		} while(TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(srcf))
		{
			;
		}
		else
		{
			retval = RtmpOSFileClose(srcf);

			if (retval)
			{
				DBGPRINT_ERR(("--> Error %d closing %s\n", -retval, src));

			}
		}

		/* restore */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

    DBGPRINT(RT_DEBUG_OFF, ("<=== %s (ret=%d)\n", __FUNCTION__, ret));

    return ret;
}


#ifdef RTMP_EFUSE_SUPPORT
/*
==========================================================================
    Description:
        Load and Write E-Fuse from pAd->EEPROMImage.

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT Set_ATE_Load_E2P_From_Buf_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	BOOLEAN		    	ret = FALSE;
	uint32_t 			value = (uint32_t) simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("===> %s (value=%d)\n\n", __FUNCTION__, value));

	if (value > 0)
	{
		rt_ee_write_all(pAd, pAd->EEPROMImage);
		ret = TRUE;

	}

    DBGPRINT(RT_DEBUG_OFF, ("<=== %s (ret=%d)\n", __FUNCTION__, ret));

    return ret;
}
#endif /* RTMP_EFUSE_SUPPORT */


INT Set_ATE_Read_E2P_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	USHORT buffer[EEPROM_SIZE >> 1];
	USHORT *p;
	int i;

	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;
	for (i = 0; i < (EEPROM_SIZE >> 1); i++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", *p));
		if (((i+1) % 16) == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		p++;
	}
	return TRUE;
}




/*
==========================================================================
    Description:
        Enable ATE auto Tx alc (Tx auto level control).
        According to the chip temperature, auto adjust the transmit power.

        0: disable
        1: enable

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_AUTO_ALC_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t value = simple_strtol(arg, 0, 10);

	if (value > 0)
	{
#ifdef MT76x0_TSSI_CAL_COMPENSATION
		if (IS_MT76x0(pAd))
		{
			MT76x0ATE_TSSI_DC_Calibration(pAd);
			MT76x0ATE_Enable9BitIchannelADC(pAd, TRUE);
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
#ifdef RTMP_INTERNAL_TX_ALC
#endif /* RTMP_INTERNAL_TX_ALC */
		pATEInfo->bAutoTxAlc = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = TRUE , auto alc enabled!\n"));
	}
	else
	{
		pATEInfo->bAutoTxAlc = FALSE;
#ifdef MT76x0_TSSI_CAL_COMPENSATION
		if (IS_MT76x0(pAd))
		{
			uint32_t MacValue;

			/* clean up MAC 0x13B4 */
			RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MacValue);
			MacValue = MacValue & (~0x3f);
			RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, MacValue);
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = FALSE , auto alc disabled!\n"));
	}

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Enable Tx temperature sensor.

        0: disable
        1: enable

        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TEMP_SENSOR_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	BOOLEAN value = simple_strtol(arg, 0, 10);

	if (value > 0)
	{
		pATEInfo->bLowTemperature = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATETEMPSENSOR = TRUE , temperature sensor enabled!\n"));
	}
	else
	{
		pATEInfo->bLowTemperature = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATETEMPSENSOR = FALSE , temperature sensor disabled!\n"));
	}

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


#ifdef TXBF_SUPPORT

/*
==========================================================================
    Description:
	Sanity check for the channel of Implicit TxBF calibration.

    Return:
	TRUE if all parameters are OK, FALSE otherwise

    Note:
	1. This sanity check function only work for Implicit TxBF calibration.
	2. Currently supported channels are:
        	1, 14, 36, 64, 128, 132, 165               for 11n
        	1, 14, 36, 64, 100, 120, 140, 149, 173 for 11ac
==========================================================================
*/
static BOOLEAN rtmp_ate_txbf_cal_valid_ch(
	IN struct rtmp_adapter *pAd,
	IN UCHAR channel)
{
	BOOLEAN bValidCh;

	/* TODO: shall we check the capability of the chipset here ?? */
	switch (channel)
	{
#ifdef MT76x2
		case 0:
			bValidCh = TRUE;
			break;
#endif
		case 1:
		case 14:
#ifdef A_BAND_SUPPORT
#ifndef MT76x2
		case 36:
		case 64:
		case 100:
		case 128:
		case 132:
		case 165:
#else
		case 36:
		case 64:
		case 100:
		case 120:
		case 140:
		case 149:
		case 173:
#endif
#endif /* A_BAND_SUPPORT */

			bValidCh = TRUE;
			break;
		default:
			bValidCh = FALSE;
			break;
	}

	return bValidCh;
}

#endif /* TXBF_SUPPORT */


/*
==========================================================================
    Description:
        Set ATE Tx frame IPG

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_IPG_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t           data, value;

	pATEInfo->IPG = simple_strtol(arg, 0, 10);
	value = pATEInfo->IPG;

	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &data);

	if (value <= 0)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Set_ATE_IPG_Proc::IPG is disabled(IPG == 0).\n"));
		return TRUE;
	}

	ASSERT(value > 0);

	if ((value > 0) && (value < 256))
	{
	    RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, data);
	}
	else
	{
	    uint32_t aifsn, slottime;

	    RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &slottime);
	    slottime &= 0x000000FF;

	    aifsn = value / slottime;
	    value = value % slottime;

	    RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, data);
	}

	data = (value & 0xFFFF0000) | value | (value << 8);
	RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, data);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_IPG_Proc (IPG = %u)\n", pATEInfo->IPG));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_IPG_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE payload pattern for TxFrame

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_Payload_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	char *			value;

	value = arg;

	/* only one octet acceptable */
	if (strlen(value) != 2)
		return FALSE;

	AtoH(value, &(pATEInfo->Payload), 1);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", pATEInfo->Payload));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Payload_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
==========================================================================
    Description:
        Set ATE fixed/random payload pattern for TxFrame

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_Fixed_Payload_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	uint32_t value = simple_strtol(arg, 0, 10);

	if ( value == 0 )
		pATEInfo->bFixedPayload = FALSE;
	else
		pATEInfo->bFixedPayload = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Fixed_Payload_Proc (Fixed Payload  = %d)\n", pATEInfo->bFixedPayload));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Fixed_Payload_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}





INT	Set_ATE_Show_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	char *Mode_String = NULL;
	char *TxMode_String = NULL;
	UCHAR bw = 0, phy_mode = 0, sgi =0, mcs =0;

	bw = pATEInfo->TxWI.TXWI_N.BW;
	phy_mode = pATEInfo->TxWI.TXWI_N.PHYMODE;
	sgi = pATEInfo->TxWI.TXWI_N.ShortGI;
	mcs = pATEInfo->TxWI.TXWI_N.MCS;

	switch (pATEInfo->Mode)
	{
#ifdef CONFIG_RT2880_ATE_CMD_NEW
		case (fATE_IDLE):
			Mode_String = "ATESTART";
			break;
		case (fATE_EXIT):
			Mode_String = "ATESTOP";
			break;
#else
		case (fATE_IDLE):
			Mode_String = "APSTOP";
			break;
		case (fATE_EXIT):
			Mode_String = "APSTART";
			break;
#endif /* CONFIG_RT2880_ATE_CMD_NEW */
		case ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE)):
			Mode_String = "TXCONT";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE)):
			Mode_String = "TXCARR";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE)):
			Mode_String = "TXCARS";
			break;
		case (fATE_TX_ENABLE):
			Mode_String = "TXFRAME";
			break;
		case (fATE_RX_ENABLE):
			Mode_String = "RXFRAME";
			break;
		default:
		{
			Mode_String = "Unknown ATE mode";
			DBGPRINT(RT_DEBUG_OFF, ("ERROR! Unknown ATE mode!\n"));
			break;
		}
	}
	DBGPRINT(RT_DEBUG_OFF, ("ATE Mode=%s\n", Mode_String));
#ifdef RT3350
	if (IS_RT3350(pAd))
		DBGPRINT(RT_DEBUG_OFF, ("PABias=%u\n", pATEInfo->PABias));
#endif /* RT3350 */
	DBGPRINT(RT_DEBUG_OFF, ("TxPower0=%d\n", pATEInfo->TxPower0));
	DBGPRINT(RT_DEBUG_OFF, ("TxPower1=%d\n", pATEInfo->TxPower1));
#ifdef DOT11N_SS3_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("TxPower2=%d\n", pATEInfo->TxPower2));
#endif /* DOT11N_SS3_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("TxAntennaSel=%d\n", pATEInfo->TxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("RxAntennaSel=%d\n", pATEInfo->RxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("BBPCurrentBW=%u\n", bw));
	DBGPRINT(RT_DEBUG_OFF, ("GI=%u\n", sgi));
	DBGPRINT(RT_DEBUG_OFF, ("MCS=%u\n", mcs));

	switch (phy_mode)
	{
		case 0:
			TxMode_String = "CCK";
			break;
		case 1:
			TxMode_String = "OFDM";
			break;
		case 2:
			TxMode_String = "HT-Mix";
			break;
		case 3:
			TxMode_String = "GreenField";
			break;
		default:
		{
			TxMode_String = "Unknown TxMode";
			DBGPRINT(RT_DEBUG_OFF, ("ERROR! Unknown TxMode!\n"));
			break;
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("TxMode=%s\n", TxMode_String));
	DBGPRINT(RT_DEBUG_OFF, ("Addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pATEInfo->Addr1[0], pATEInfo->Addr1[1], pATEInfo->Addr1[2], pATEInfo->Addr1[3], pATEInfo->Addr1[4], pATEInfo->Addr1[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pATEInfo->Addr2[0], pATEInfo->Addr2[1], pATEInfo->Addr2[2], pATEInfo->Addr2[3], pATEInfo->Addr2[4], pATEInfo->Addr2[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr3=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pATEInfo->Addr3[0], pATEInfo->Addr3[1], pATEInfo->Addr3[2], pATEInfo->Addr3[3], pATEInfo->Addr3[4], pATEInfo->Addr3[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Channel=%u\n", pATEInfo->Channel));
	DBGPRINT(RT_DEBUG_OFF, ("TxLength=%u\n", pATEInfo->TxLength));
	DBGPRINT(RT_DEBUG_OFF, ("TxCount=%u\n", pATEInfo->TxCount));
	DBGPRINT(RT_DEBUG_OFF, ("RFFreqOffset=%u\n", pATEInfo->RFFreqOffset));
	DBGPRINT(RT_DEBUG_OFF, ("bAutoTxAlc=%d\n", pATEInfo->bAutoTxAlc));
	DBGPRINT(RT_DEBUG_OFF, ("IPG=%u\n", pATEInfo->IPG));
	DBGPRINT(RT_DEBUG_OFF, ("Payload=0x%02x\n", pATEInfo->Payload));
#ifdef TXBF_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("bTxBF=%d\n", pATEInfo->bTxBF));
	DBGPRINT(RT_DEBUG_OFF, ("txSoundingMode=%d\n", pATEInfo->txSoundingMode));
#endif /* TXBF_SUPPORT */


	DBGPRINT(RT_DEBUG_OFF, ("Set_ATE_Show_Proc Success\n"));
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

#ifdef RTMP_TEMPERATURE_CALIBRATION
INT Set_ATE_TEMP_CAL_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	INT ret = TRUE;


	return ret;
}


INT Set_ATE_SHOW_TSSI_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	INT ret = TRUE;


	return ret;
}
#endif /* RTMP_TEMPERATURE_CALIBRATION */


#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->ExtendedTssiCalibration != NULL)
	{
		pATEInfo->pChipStruct->ExtendedTssiCalibration(pAd, arg);
	}
	else
	{
		RTMP_CHIP_ATE_TSSI_CALIBRATION_EXTEND(pAd, arg);
	}

	return TRUE;
}
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */


#ifdef RTMP_INTERNAL_TX_ALC


INT Set_ATE_TSSI_CALIBRATION_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->TssiCalibration != NULL)
{
		pATEInfo->pChipStruct->TssiCalibration(pAd, arg);
	}
	else
	{
		RTMP_CHIP_ATE_TSSI_CALIBRATION(pAd, arg);
	}

	return TRUE;
}


#if defined(RT3350) || defined(RT3352)
INT RT335x2_Set_ATE_TSSI_CALIBRATION_ENABLE_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
		{
	BOOLEAN	bTSSICalbrEnableG = FALSE;

	if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)
			{
		DBGPRINT_ERR(("Please set e2p 0x36 as 0x2024!!!\n"));
		return FALSE;
		}

	if ((!IS_RT3350(pAd)) && (!IS_RT3352(pAd)))
	{
		DBGPRINT_ERR(("Not support TSSI calibration since not 3350/3352 chip!!!\n"));
		return FALSE;
	}

	if (strcmp(arg, "0") == 0)
		{
		bTSSICalbrEnableG = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI calibration disabled!\n"));
	}
	else if (strcmp(arg, "1") == 0)
			{
		bTSSICalbrEnableG = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI calibration enabled!\n"));
			}
	else
	{
		return FALSE;
		}

	pAd->ate.bTSSICalbrEnableG = bTSSICalbrEnableG;

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
	}


CHAR InsertTssi(UCHAR InChannel, UCHAR Channel0, UCHAR Channel1,CHAR Tssi0, CHAR Tssi1)
{
	CHAR     InTssi;
	CHAR     ChannelDelta, InChannelDelta;
	CHAR     TssiDelta;

	ChannelDelta = Channel1 - Channel0;
	InChannelDelta = InChannel - Channel0;
	TssiDelta = Tssi1 - Tssi0;

	InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);

	return InTssi;
}

#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */

struct ate_chip_struct RALINKDefault =
{
	/* functions */
	.TxPwrHandler = NULL,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = NULL,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
#ifdef SINGLE_SKU_V2
	.do_ATE_single_sku = NULL,
#endif

	/* command handlers */
	.Set_BW_Proc = NULL,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = FALSE,
};

#ifdef RT28xx
#ifdef RTMP_MAC_USB
extern ATE_CHIP_STRUCT RALINK2870;
#endif /* RTMP_MAC_USB */
#endif /* RT28xx */












#ifdef RT8592
extern struct ate_chip_struct RALINK85592;
#endif /* RT8592 */


#ifdef MT76x2
extern struct ate_chip_struct mt76x2ate;
#endif /* MT76x2 */
/*
==========================================================================
	Description:
		Assign chip structure when initialization.
		This routine is specific for ATE.

==========================================================================
*/
int ChipStructAssign(
	IN	struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	pATEInfo->pChipStruct = &RALINKDefault;

#ifdef RT8592
	if (IS_RT8592(pAd))
	{
		pATEInfo->pChipStruct = &RALINK85592;
		DBGPRINT(RT_DEBUG_OFF, ("%s(): RALINK85592 hook !\n", __FUNCTION__));
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT8592 */


#ifdef MT76x2
	if (IS_MT76x2(pAd))
	{
		pATEInfo->pChipStruct = &mt76x2ate;
		DBGPRINT(RT_DEBUG_OFF, ("%s(): MT76x2 hook !\n", __FUNCTION__));
		return NDIS_STATUS_SUCCESS;
	}
#endif /* MT76x2 */








#ifdef RT28xx
	if (IS_RT2860(pAd))
	{


	}
#endif /* RT28xx */




#ifdef RT8592
	if (IS_RT8592(pAd))
	{
		pATEInfo->pChipStruct = &RALINK85592;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT8592 */


	/* sanity check */
	if (pATEInfo->pChipStruct == &RALINKDefault)
	{
		/* Unknown chipset ! */
		DBGPRINT_ERR(("Warning - Unknown chipset !!!\n"));
		DBGPRINT_ERR(("MAC Version is %u\n", ((pAd->MACVersion & 0xFFFF0000) >> 16)));
		DBGPRINT_ERR(("Interface type is %d\n", pAd->infType));

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
==========================================================================
	Description:
		Initialize ATE_INFO structure.
		This routine is specific for ATE.

==========================================================================
*/
int ATEInit(
	IN	struct rtmp_adapter *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	memset(pATEInfo, 0, sizeof(ATE_INFO));

	if (ChipStructAssign(pAd) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("%s failed !\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	OS_NdisAllocateSpinLock(&(pATEInfo->TssiSemLock));

	pATEInfo->Mode = ATE_STOP;
#ifdef RT3350
	pATEInfo->PABias = 0;
#endif /* RT3350  */
	pATEInfo->TxCount = 0xFFFFFFFF;
	pATEInfo->TxDoneCount = 0;
	pATEInfo->RFFreqOffset = 0;
#if defined(RT5370) || defined(RT5390) || defined(RT6352) || defined(RT65xx)
	if (IS_RT5392(pAd) || IS_RT6352(pAd) || IS_RT65XX(pAd))
		pATEInfo->Payload = 0xAA;
	else
#endif /* defined(RT5370) || defined(RT5390) || defined(RT6352) || defined(RT65xx) */
		pATEInfo->Payload = 0xA5;/* to be backward compatible */
	pATEInfo->bFixedPayload = 1;
	pATEInfo->IPG = 200;/* 200 : sync with QA */
	pATEInfo->TxLength = 1058;

	pATEInfo->TxWI.TXWI_N.BW = BW_20;
	pATEInfo->TxWI.TXWI_N.PHYMODE = MODE_OFDM;
	pATEInfo->TxWI.TXWI_N.MCS = 7;
	pATEInfo->TxWI.TXWI_N.ShortGI = 0;/* LONG GI : 800 ns*/

	pATEInfo->Channel = 1;
	pATEInfo->TxAntennaSel = 1;
	pATEInfo->RxAntennaSel = 0;

	/* please do not change this default channel value */
	pATEInfo->Channel = 1;


	pATEInfo->QID = QID_AC_BE;

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
	{
	/* For 3T/3R ++ */
	/* use broadcast address as default value */
	pATEInfo->Addr1[0] = 0xFF;
	pATEInfo->Addr1[1] = 0xFF;
	pATEInfo->Addr1[2] = 0xFF;
	pATEInfo->Addr1[3] = 0xFF;
	pATEInfo->Addr1[4] = 0xFF;
	pATEInfo->Addr1[5] = 0xFF;

	pATEInfo->Addr2[0] = 0x00;
	pATEInfo->Addr2[1] = 0x11;
	pATEInfo->Addr2[2] = 0x22;
	pATEInfo->Addr2[3] = 0xAA;
	pATEInfo->Addr2[4] = 0xBB;
	pATEInfo->Addr2[5] = 0xCC;

	memmove(pATEInfo->Addr3, pATEInfo->Addr2, MAC_ADDR_LEN);

	{
		uint32_t data;

		data = 0xFFFFFFFF;
    	RTMP_IO_WRITE32(pAd, 0x1044, data);
    	RTMP_IO_READ32(pAd, 0x1048, &data);

    	data = data | 0x0000FFFF;
    	RTMP_IO_WRITE32(pAd, 0x1048, data);
	}
	/* For stream mode in 3T/3R -- */
	}
	else
#endif /* DOT11N_SS3_SUPPORT */
	{
	pATEInfo->Addr1[0] = 0x00;
	pATEInfo->Addr1[1] = 0x11;
	pATEInfo->Addr1[2] = 0x22;
	pATEInfo->Addr1[3] = 0xAA;
	pATEInfo->Addr1[4] = 0xBB;
	pATEInfo->Addr1[5] = 0xCC;

	memmove(pATEInfo->Addr2, pATEInfo->Addr1, MAC_ADDR_LEN);
	memmove(pATEInfo->Addr3, pATEInfo->Addr1, MAC_ADDR_LEN);
	}
	pATEInfo->bRxFER = 0;
	pATEInfo->bQAEnabled = FALSE;
	pATEInfo->bQATxStart = FALSE;
	pATEInfo->bQARxStart = FALSE;
	pATEInfo->bAutoTxAlc = FALSE;
	pATEInfo->bLowTemperature = FALSE;
#ifdef SINGLE_SKU_V2
	pATEInfo->bDoSingleSKU = FALSE;
#endif
	pATEInfo->bAutoVcoCal = FALSE;
#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	pATEInfo->bTSSICalbrEnableG = FALSE;
	memset((u8 *)&(pATEInfo->TssiRefPerChannel), 0, CFG80211_NUM_OF_CHAN_2GHZ);
	memset((u8 *)&(pATEInfo->TssiDeltaPerChannel), 0, CFG80211_NUM_OF_CHAN_2GHZ);
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */

	/* Default TXCONT/TXCARR/TXCARS mechanism is TX_METHOD_1 */
	pATEInfo->TxMethod = TX_METHOD_1;
	if ((IS_RT2070(pAd) || IS_RT2860(pAd) || IS_RT2872(pAd) || IS_RT2883(pAd)))
	{
		/* Early chipsets must be applied original TXCONT/TXCARR/TXCARS mechanism. */
		pATEInfo->TxMethod = TX_METHOD_0;
	}

	/* Power range is 0~31 in A band. */
	pATEInfo->MinTxPowerBandA = 0;
	pATEInfo->MaxTxPowerBandA = 31;
	if ((IS_RT2860(pAd)) || (IS_RT2872(pAd)) || (IS_RT2883(pAd)))
	{
		/* Power range of early chipsets is -7~15 in A band. */
		pATEInfo->MinTxPowerBandA = -7;
		pATEInfo->MaxTxPowerBandA = 15;
	}

#ifdef TXBF_SUPPORT
	pATEInfo->bTxBF = FALSE;
#endif /* TXBF_SUPPORT */


#ifdef RTMP_MAC_USB
#endif /* RTMP_MAC_USB */

	pATEInfo->OneSecPeriodicRound = 0;
	pATEInfo->PeriodicRound = 0;

	return NDIS_STATUS_SUCCESS;
}

#define SMM_BASEADDR                      0x4000
#define PKT_BASEADDR                      0x8000


#ifdef RLT_MAC
// TODO: shiang-6590, fix me, how about this register in RT85592?
#define PBF_CAP_CTRL	0x0440
#endif /* RLT_MAC */
INT Set_ADCDump_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg)
{
	return FALSE;
}


/* 100ms periodic execution */
VOID ATEPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)FunctionContext;
	PATE_INFO pATEInfo = &(pAd->ate);

	if (ATE_ON(pAd))
	{
		pATEInfo->PeriodicRound++;
#ifdef MT76x2
		if ( IS_MT76x2(pAd) )
		{
			if ( (pATEInfo->Mode == ATE_TXFRAME) && (pATEInfo->bAutoTxAlc == TRUE) )			{
				ATEAsicAdjustTxPower(pAd);
			}
		}
#endif /* MT76x2 */

		/* Normal 1 second ATE PeriodicExec.*/
		//if (pATEInfo->PeriodicRound % (ATE_TASK_EXEC_MULTIPLE) == 0)
		{
			pATEInfo->OneSecPeriodicRound++;

			/* for performace enchanement */
			memset(&pAd->RalinkCounters, 0,
							(uint32_t)&pAd->RalinkCounters.OneSecEnd -
							(uint32_t)&pAd->RalinkCounters.OneSecStart);
			NICUpdateRawCounters(pAd);

			if (pATEInfo->bRxFER == 1)
			{
				pATEInfo->RxTotalCnt += pATEInfo->RxCntPerSec;
				ate_print(KERN_EMERG "ATEPeriodicExec: Rx packet cnt = %d/%d\n",
				pATEInfo->RxCntPerSec, pATEInfo->RxTotalCnt);
				pATEInfo->RxCntPerSec = 0;

				if (pATEInfo->RxAntennaSel == 0)
					ate_print(KERN_EMERG "ATEPeriodicExec: Rx AvgRssi0=%d, AvgRssi1=%d, AvgRssi2=%d\n\n",
						pATEInfo->AvgRssi0, pATEInfo->AvgRssi1, pATEInfo->AvgRssi2);
				else
					ate_print(KERN_EMERG "ATEPeriodicExec: Rx AvgRssi=%d\n\n", pATEInfo->AvgRssi0);
			}

			MlmeResetRalinkCounters(pAd);

			/* In QA Mode, QA will handle all registers. */
			if (pATEInfo->bQAEnabled == TRUE)
			{
				return;
			}

			if ((!IS_RT6352(pAd)) && (!IS_MT76x0(pAd)) &&  (!IS_MT76x2(pAd)))
			{
				/* MT7620 and MT7610E have adjusted Tx power above */
				if ((pATEInfo->bAutoTxAlc == TRUE) && (pATEInfo->Mode == ATE_TXFRAME))
					ATEAsicAdjustTxPower(pAd);

				ATEAsicExtraPowerOverMAC(pAd);
			}

			/* only for MT7601 so far */
			ATEAsicTemperCompensation(pAd);

			/* do VCO calibration per four seconds */
			if (pATEInfo->PeriodicRound % (ATE_TASK_EXEC_MULTIPLE * 4) == 0)
			{
				if ((pAd->ate.bAutoVcoCal == TRUE) && (pATEInfo->Mode == ATE_TXFRAME))
				{
					{
					}
				}
			}

		}
	}
	else
	{
		DBGPRINT_ERR(("%s is NOT called in ATE mode.\n", __FUNCTION__));
	}

	return;
}

