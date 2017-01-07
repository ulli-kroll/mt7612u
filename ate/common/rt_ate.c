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

#ifndef RTMP_RF_RW_SUPPORT
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RTMP_INTERNAL_TX_ALC

#if defined(RT3350) || defined(RT3352)

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
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.AsicExtraPowerOverMAC = NULL,

	/* command handlers */

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

#ifdef MT76x2
	if (IS_MT76x2(pAd))
	{
		pATEInfo->pChipStruct = &mt76x2ate;
		DBGPRINT(RT_DEBUG_OFF, ("%s(): MT76x2 hook !\n", __FUNCTION__));
		return NDIS_STATUS_SUCCESS;
	}
#endif /* MT76x2 */

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

#ifdef RT3350
	pATEInfo->PABias = 0;
#endif /* RT3350  */
	pATEInfo->TxCount = 0xFFFFFFFF;
	pATEInfo->TxDoneCount = 0;
	pATEInfo->RFFreqOffset = 0;
#if defined(RT5370) || defined(RT5390) || defined(RT6352) || defined(RT65xx)
	if (IS_RT65XX(pAd))
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

	/* Power range is 0~31 in A band. */
	pATEInfo->MinTxPowerBandA = 0;
	pATEInfo->MaxTxPowerBandA = 31;

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

	{
		DBGPRINT_ERR(("%s is NOT called in ATE mode.\n", __FUNCTION__));
	}

	return;
}

