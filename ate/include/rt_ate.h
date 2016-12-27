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
	rt_ate.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __RT_ATE_H__
#define __RT_ATE_H__

#ifdef RALINK_ATE
#ifndef STATS_COUNT_SUPPORT
#error "For ATE support, please set HAS_ATE=y and HAS_STATS_COUNT=y."
#endif /* !STATS_COUNT_SUPPORT */
#endif /* RALINK_ATE */

#include "mac_ral/rtmp_mac.h"
#include "chip/rtmp_phy.h"

// TODO: shiang-6590, remove it after ATE code is re-organized!!
#define BBP_R1			1
#define BBP_R3			3
#define BBP_R4			4
#define BBP_R21			21
#define BBP_R22			22
#define BBP_R24			24
#define BBP_R27			27
#define BBP_R50			50
#define BBP_R51			51
#define BBP_R52			52
#define BBP_R60			60
#define BBP_R66			66
#define BBP_R69			69
#define BBP_R70			70
#define BBP_R174			174
#define BBP_R182			182
// TODO: ---End

struct ate_chip_struct {
	/* functions */
	INT		(*TssiCalibration)(struct rtmp_adapter *pAd, char *arg);
	INT		(*ExtendedTssiCalibration)(struct rtmp_adapter *pAd, char *arg);
	VOID	(*AsicSetTxRxPath)(struct rtmp_adapter *pAd);
	VOID	(*AdjustTxPower)(struct rtmp_adapter *pAd);
	VOID	(*AsicExtraPowerOverMAC)(struct rtmp_adapter *pAd);
	VOID 	(*TemperCompensation)(struct rtmp_adapter *pAd);
#ifdef SINGLE_SKU_V2
	VOID 	(*do_ATE_single_sku)(struct rtmp_adapter *pAd, BOOLEAN value);
#endif

	/* command handlers */

	/* variables */
	INT maxTxPwrCnt;
	BOOLEAN bBBPStoreTXCARR;
	BOOLEAN bBBPStoreTXCARRSUPP;
	BOOLEAN bBBPStoreTXCONT;
	BOOLEAN bBBPLoadATESTOP;

	/* TSSI related */
	int32_t tssi_slope[2];
	int32_t tssi_offset[3][2];
	int32_t pwr_diff_pre[2];
	int32_t ant_pwr_offset[2];
	int32_t curr_temperature;
};

typedef union _CAPTURE_MODE_SHARE_MEMORY {
	struct
	{
		uint32_t       LOW_BYTE0:8;
		uint32_t       LOW_BYTE1:8;
		uint32_t       HIGH_BYTE0:8;
		uint32_t       HIGH_BYTE1:8;
	} field;
	uint32_t                   Value;
}CAPTURE_MODE_SHARE_MEMORY, *PCAPTURE_MODE_SHARE_MEMORY;

typedef struct _ATE_INFO {
	struct ate_chip_struct *pChipStruct;
	UCHAR Mode;
	BOOLEAN PassiveMode;
#ifdef RT3350
	UCHAR   PABias;
#endif /* RT3350 */
	CHAR TxPower0;
	CHAR TxPower1;
#ifdef DOT11N_SS3_SUPPORT
	CHAR TxPower2;
#endif /* DOT11N_SS3_SUPPORT */
	CHAR MinTxPowerBandA; /* Power range of early chipsets is -7~15 in A band */
	CHAR MaxTxPowerBandA; /* Power range of early chipsets is -7~15 in A band */
	CHAR TxAntennaSel;
	CHAR RxAntennaSel;
	USHORT TxInfoLen;
	USHORT TxWILen;
	TXINFO_STRUC TxInfo;	/* TxInfo */
	TXWI_STRUC TxWI;	/* TXWI */
	USHORT QID;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR Channel;
	UCHAR Payload;		/* Payload pattern */
	BOOLEAN bFixedPayload;
	UCHAR TxMethod; /* Early chipsets must be applied old TXCONT/TXCARR/TXCARS mechanism. */
	uint32_t TxLength;
	uint32_t TxCount;
	uint32_t TxDoneCount;	/* Tx DMA Done */
	uint32_t RFFreqOffset;
	uint32_t IPG;
	BOOLEAN bRxFER;		/* Show Rx Frame Error Rate */
	BOOLEAN	bQAEnabled;	/* QA is used. */
	BOOLEAN bQATxStart;	/* Have compiled QA in and use it to ATE tx. */
	BOOLEAN bQARxStart;	/* Have compiled QA in and use it to ATE rx. */
	BOOLEAN bAutoTxAlc;	/* Set Auto Tx Alc */
#ifdef SINGLE_SKU_V2
	BOOLEAN bDoSingleSKU; /*Do Single SKU in ATE*/
#endif
	BOOLEAN bAutoVcoCal; /* Set Auto VCO periodic calibration. */
	BOOLEAN bLowTemperature; /* Trigger Temperature Sensor */
#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	BOOLEAN bTSSICalbrEnableG; /* Enable TSSI calibration */
	CHAR	TssiRefPerChannel[CFG80211_NUM_OF_CHAN_2GHZ];
	CHAR	TssiDeltaPerChannel[CFG80211_NUM_OF_CHAN_2GHZ];
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef TXBF_SUPPORT
	BOOLEAN bTxBF;		/* Enable Tx Bean Forming */
	SHORT	txSoundingMode;	/* Sounding mode for non-QA ATE. 0=none, 1=Data Sounding, 2=NDP */
	UCHAR	calParams[2];
#endif				/* TXBF_SUPPORT */
	uint32_t RxTotalCnt;
	uint32_t RxCntPerSec;
	UCHAR	forceBBPReg;	/* force to not update the specific BBP register, now used for ATE TxBF */

	CHAR LastSNR0;		/* last received SNR */
	CHAR LastSNR1;		/* last received SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	CHAR LastSNR2;
#endif				/* DOT11N_SS3_SUPPORT */
	CHAR LastRssi0;		/* last received RSSI */
	CHAR LastRssi1;		/* last received RSSI for 2nd  antenna */
	CHAR LastRssi2;		/* last received RSSI for 3rd  antenna */
	CHAR AvgRssi0;		/* last 8 frames' average RSSI */
	CHAR AvgRssi1;		/* last 8 frames' average RSSI */
	CHAR AvgRssi2;		/* last 8 frames' average RSSI */
	SHORT AvgRssi0X8;	/* sum of last 8 frames' RSSI */
	SHORT AvgRssi1X8;	/* sum of last 8 frames' RSSI */
	SHORT AvgRssi2X8;	/* sum of last 8 frames' RSSI */
	uint32_t NumOfAvgRssiSample;
	uint32_t Default_TX_PIN_CFG;
	USHORT HLen;		/* Header Length */

#ifdef TXBF_SUPPORT
#define MAX_SOUNDING_RESPONSE_SIZE	(57*2*2*9+3+2+6)	/* Assume 114 carriers (40MHz), 3x3, 8bits/coeff, + SNR + HT HEADER + MIMO CONTROL FIELD */
	UCHAR sounding;
	uint32_t sounding_jiffies;
	CHAR soundingSNR[3];
	uint32_t LastRxRate;
	uint32_t LastTxRate;
	uint32_t soundingRespSize;	/* Size of Sounding response */
	UCHAR soundingResp[MAX_SOUNDING_RESPONSE_SIZE];	/* Entire Sounding response */
#endif /* TXBF_SUPPORT */
	RALINK_TIMER_STRUCT PeriodicTimer;
	ULONG OneSecPeriodicRound;
	ULONG PeriodicRound;
	OS_NDIS_SPIN_LOCK TssiSemLock;
} ATE_INFO, *PATE_INFO;

/*
	Use bitmap to allow coexist of ATE_TXFRAME
	and ATE_RXFRAME(i.e.,to support LoopBack mode).
*/
#define fATE_IDLE					0x00
#define fATE_TX_ENABLE				0x01
#define fATE_RX_ENABLE				0x02
#define fATE_TXCONT_ENABLE			0x04
#define fATE_TXCARR_ENABLE			0x08
#define fATE_TXCARRSUPP_ENABLE		0x10
#define fATE_RESERVED_1				0x20
#define fATE_RESERVED_2				0x40
#define fATE_EXIT					0x80

/* Enter/Reset ATE */
#define	ATE_START                   (fATE_IDLE)
/* Stop/Exit ATE */
#define	ATE_STOP                    (fATE_EXIT)
/* Continuous Transmit Frames (without time gap) */
#define	ATE_TXCONT                  ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE))
/* Transmit Carrier */
#define	ATE_TXCARR                  ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE))
/* Transmit Carrier Suppression (information without carrier) */
#define	ATE_TXCARRSUPP              ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE))
/* Transmit Frames */
#define	ATE_TXFRAME                 (fATE_TX_ENABLE)
/* Receive Frames */
#define	ATE_RXFRAME                 (fATE_RX_ENABLE)


#ifdef RTMP_INTERNAL_TX_ALC
#define EEPROM_TSSI_ENABLE 0x36
#define EEPROM_TSSI_MODE_EXTEND 0x76

#define ATE_MDSM_NORMAL_TX_POWER						0x00
#define ATE_MDSM_DROP_TX_POWER_BY_6dBm					0x01
#define ATE_MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define ATE_MDSM_ADD_TX_POWER_BY_6dBm					0x03
#define ATE_MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK	0x03
#endif /* RTMP_INTERNAL_TX_ALC */

#define	LEN_OF_ARG 16
#define ATE_ON(_p)              (((_p)->ate.Mode) != ATE_STOP)
#define TX_METHOD_0 0 /* Early chipsets must be applied this original TXCONT/TXCARR/TXCARS mechanism. */
#define TX_METHOD_1 1 /* Default TXCONT/TXCARR/TXCARS mechanism is TX_METHOD_1 */
#define ANT_ALL 0
#define ANT_0 1
#define ANT_1 2
#ifdef DOT11N_SS3_SUPPORT
#define ANT_2 3
#endif /* DOT11N_SS3_SUPPORT */

#define ATE_MAC_TX_ENABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 2);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_TX_DISABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 2);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_RX_ENABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 3);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_RX_DISABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 3);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
#define ATE_MAC_TX_CTS_ENABLE(_A, _I, _pV)	\
{											\
	RTMP_IO_READ32(_A, _I, _pV);			\
	(*(_pV)) |= (1 << 4);					\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));		\
}

/* Clear MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 0. */
#define ATE_MAC_TX_CTS_DISABLE(_A, _I, _pV)	\
{											\
	RTMP_IO_READ32(_A, _I, _pV);			\
	(*(_pV)) &= ~(1 << 4);					\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));		\
}

/* Clear BBP R22 to reset Tx Mode (bit7~bit0) = 0. */

/* Set BBP R22 to start Continuous Tx Mode (bit7) = 1. */
#define ATE_BBP_START_CTS_TX_MODE(_A, _I, _pV)		\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 7);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Clear BBP R22 to stop Continuous Tx Mode (bit7) = 0. */
#define ATE_BBP_STOP_CTS_TX_MODE(_A, _I, _pV)		\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 7);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Set BBP R24 to send out Continuous Tx sin wave (bit0) = 1. */
#define ATE_BBP_CTS_TX_SIN_WAVE_ENABLE(_A, _I, _pV)	\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 0);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Clear BBP R24 to stop Continuous Tx sin wave (bit0) = 0. */
#define ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(_A, _I, _pV)	\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 0);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/*
==========================================================================
	Description:
		This routine sets initial value of VGA in the RX chain.
		AGC is the abbreviation of "Automatic Gain Controller",
		while VGA (Variable Gain Amplifier) is a part of AGC loop.
		(i.e., VGA + level detector + feedback loop = AGC)

    Return:
        VOID
==========================================================================
*/

#define ATE_CHIP_SET_TX_RX_PATH(__pAd)								\
	if (__pAd->ate.pChipStruct->AsicSetTxRxPath != NULL)	\
		__pAd->ate.pChipStruct->AsicSetTxRxPath(__pAd)



#ifdef RTMP_MAC_USB
#define ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)    RTMP_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)
#define ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)    RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)

#define BULK_OUT_LOCK(pLock, IrqFlags)	\
		if(1 /*!(in_interrupt() & 0xffff0000)*/)	\
			RTMP_IRQ_LOCK((pLock), IrqFlags);

#define BULK_OUT_UNLOCK(pLock, IrqFlags)	\
		if(1 /*!(in_interrupt() & 0xffff0000)*/)	\
			RTMP_IRQ_UNLOCK((pLock), IrqFlags);

VOID ATE_RTUSBBulkOutDataPacket(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			BulkOutPipeId);

VOID ATE_RTUSBCancelPendingBulkInIRP(
	IN	struct rtmp_adapter *pAd);

VOID ATEResetBulkIn(
	IN struct rtmp_adapter *pAd);

INT ATEResetBulkOut(
	IN struct rtmp_adapter *pAd);
#endif /* RTMP_MAC_USB */

#if defined(RT28xx) || defined(RT2880)
VOID RT28xxATEAsicSwitchChannel(
    IN struct rtmp_adapter *pAd);
#endif /* defined(RT28xx) || defined(RT2880) */




#ifdef RTMP_RF_RW_SUPPORT
#ifdef RLT_RF
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _B, _I, _pV)     rlt_rf_read(_A, _B, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _B, _I, _V)     rlt_rf_write(_A, _B, _I, _V)
#endif /* RLT_RF */
#ifndef RLT_RF
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _I, _pV)     RT30xxReadRFRegister(_A, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _I, _V)     RT30xxWriteRFRegister(_A, _I, _V)
#endif /* !RLT_RF */
#endif /* RTMP_RF_RW_SUPPORT */

#define SYNC_CHANNEL_WITH_QA(_A, _pV)\
	*_pV = _A->Channel

VOID rt_ee_read_all(
	IN  struct rtmp_adapter *  pAd,
	OUT USHORT *Data);

VOID rt_ee_write_all(
	IN  struct rtmp_adapter *  pAd,
	IN  USHORT *Data);

VOID rt_ee_write_bulk(
	IN  struct rtmp_adapter *pAd,
	IN  USHORT *Data,
	IN  USHORT offset,
	IN  USHORT length);

INT	Set_ATE_DA_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_SA_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_BSSID_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ADCDump_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN      struct rtmp_adapter *  pAd,
	IN      char *                arg);
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

#ifdef RTMP_INTERNAL_TX_ALC
INT Set_ATE_TSSI_CALIBRATION_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN      struct rtmp_adapter *  pAd,
	IN      char *                arg);


#if defined(RT3350) || defined(RT3352)
INT RT335x_Set_ATE_TSSI_CALIBRATION_ENABLE_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

CHAR InsertTssi(
	IN UCHAR InChannel,
	IN UCHAR Channel0,
	IN UCHAR Channel1,
	IN CHAR Tssi0,
	IN CHAR Tssi1);

#endif /* defined(RT3350) || defined(RT3352) */

CHAR ATEGetDesiredTSSI(
	IN struct rtmp_adapter *	pAd);

#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_CALIBRATION
INT Set_ATE_TEMP_CAL_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ATE_SHOW_TSSI_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);
#endif /* RTMP_TEMPERATURE_CALIBRATION */

INT	Set_ATE_TX_Antenna_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_RX_Antenna_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

VOID DefaultATEAsicExtraPowerOverMAC(
	IN	struct rtmp_adapter *		pAd);

VOID ATEAsicExtraPowerOverMAC(
	IN	struct rtmp_adapter *pAd);

INT	Default_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);





#if defined(RT28xx) || defined(RT2880)
INT	RT28xx_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);
#endif /* defined(RT28xx) || defined(RT2880) */

INT	Default_Set_ATE_TX_BW_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);





#if defined(RT28xx) || defined(RT2880)
INT	RT28xx_Set_ATE_TX_BW_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);
#endif /* defined(RT28xx) || defined(RT2880) */

INT	Set_ATE_TX_LENGTH_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_TX_COUNT_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_TX_MCS_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_TX_STBC_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	Set_ATE_TX_GI_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);


INT	Set_ATE_RX_FER_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ATE_Read_RF_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

#if (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF))
INT Set_ATE_Write_RF1_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ATE_Write_RF2_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ATE_Write_RF3_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT Set_ATE_Write_RF4_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);
#endif /* (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF)) */

INT Set_ATE_Payload_Proc(
    IN  struct rtmp_adapter *  pAd,
    IN  char *        arg);

INT Set_ATE_Fixed_Payload_Proc(
    IN  struct rtmp_adapter *  pAd,
    IN  char *        arg);

VOID DefaultATEAsicAdjustTxPower(
	IN struct rtmp_adapter *pAd);

VOID ATEAsicAdjustTxPower(
	IN struct rtmp_adapter *pAd);

VOID ATESampleRssi(
	IN struct rtmp_adapter *pAd,
	IN RXWI_STRUC *pRxWI);


#ifdef RTMP_MAC_USB
INT TxDmaBusy(
	IN struct rtmp_adapter *pAd);

INT RxDmaBusy(
	IN struct rtmp_adapter *pAd);

VOID RtmpDmaEnable(
	IN struct rtmp_adapter *pAd,
	IN INT Enable);

INT ATESetUpFrame(
	IN struct rtmp_adapter *pAd,
	IN uint32_t TxIdx);

VOID RTUSBRejectPendingPackets(
	IN	struct rtmp_adapter *pAd);
#endif /* RTMP_MAC_USB */


int ChipStructAssign(
 IN	struct rtmp_adapter *pAd);

int ATEInit(
 IN struct rtmp_adapter *pAd);



#if defined(RT28xx) || defined(RT2880)
VOID RT28xxATERxVGAInit(
 IN struct rtmp_adapter *	pAd);
#endif /* defined(RT28xx) || defined(RT2880) */



VOID  ATEPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID RtmpRfIoWrite(struct rtmp_adapter *pAd);

VOID BbpSoftReset(struct rtmp_adapter *pAd);


#endif /* __RT_ATE_H__ */

