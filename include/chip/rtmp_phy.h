/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_phy.h

	Abstract:
	Ralink Wireless Chip PHY(BBP/RF) related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RTMP_PHY_H__
#define __RTMP_PHY_H__


#include "mac_ral/rf_ctrl.h"
#ifdef RLT_BBP
#include "phy/rlt_bbp.h"
#endif /* RLT_BBP */



/*
	RF sections
*/
#define RF_R00			0
#define RF_R01			1
#define RF_R02			2
#define RF_R03			3
#define RF_R04			4
#define RF_R05			5
#define RF_R06			6
#define RF_R07			7
#define RF_R08			8
#define RF_R09			9
#define RF_R10			10
#define RF_R11			11
#define RF_R12			12
#define RF_R13			13
#define RF_R14			14
#define RF_R15			15
#define RF_R16			16
#define RF_R17			17
#define RF_R18			18
#define RF_R19			19
#define RF_R20			20
#define RF_R21			21
#define RF_R22			22
#define RF_R23			23
#define RF_R24			24
#define RF_R25			25
#define RF_R26			26
#define RF_R27			27
#define RF_R28			28
#define RF_R29			29
#define RF_R30			30
#define RF_R31			31
#define RF_R32			32
#define RF_R33			33
#define RF_R34			34
#define RF_R35			35
#define RF_R36			36
#define RF_R37			37
#define RF_R38			38
#define RF_R39			39
#define RF_R40			40
#define RF_R41			41
#define RF_R42			42
#define RF_R43			43
#define RF_R44			44
#define RF_R45			45
#define RF_R46			46
#define RF_R47			47
#define RF_R48			48
#define RF_R49			49
#define RF_R50			50
#define RF_R51			51
#define RF_R52			52
#define RF_R53			53
#define RF_R54			54
#define RF_R55			55
#define RF_R56			56
#define RF_R57			57
#define RF_R58			58
#define RF_R59			59
#define RF_R60			60
#define RF_R61			61
#define RF_R62			62
#define RF_R63			63

#define RF_R64			64
#define RF_R65			65
#define RF_R66			66
#define RF_R67			67
#define RF_R68			68
#define RF_R69			69
#define RF_R70			70
#define RF_R71			71
#define RF_R72			72
#define RF_R73			73
#define RF_R74			74
#define RF_R75			75
#define RF_R76			76
#define RF_R77			77
#define RF_R78			78
#define RF_R79			79
#define RF_R126			126
#define RF_R127			127

/* value domain of pAd->RfIcType */
#define RFIC_6352			1	 	/* 2.4G 2T2R */
#define RFIC_7650			1		/* 2.4G/5G 1x1 VHT with BT*/
#define RFIC_7610E			2		/* 5G 1x1 VHT */
#define RFIC_7610U			3
#define RFIC_7630			4		/* 2.4G 1x1 HT with BT */
#define RFIC_7662			5		/* 2.4G/5G 2T2R VHT with BT */
#define RFIC_7612			6		/* 2.4G/5G 2T2R VHT */
#define RFIC_7602			7		/* 2.4G 2T2R VHT */
#define RFIC_UNKNOWN			0xff

#define RFIC_IS_5G_BAND(__pAd)			\
	((__pAd->RfIcType == RFIC_7650) ||	\
	(__pAd->RfIcType == RFIC_7610E) ||	\
	(__pAd->RfIcType == RFIC_7610U) ||	\
	(__pAd->RfIcType == RFIC_7662) ||	\
	(__pAd->RfIcType == RFIC_7612) ||	\
	(__pAd->RfIcType == RFIC_UNKNOWN))


#define BOARD_IS_2G_ONLY(__pAd)         0
#define BOARD_IS_5G_ONLY(__pAd)         0

typedef enum{
	RX_CHAIN_0 = 1<<0,
	RX_CHAIN_1 = 1<<1,
	RX_CHAIN_2 = 1<<2,
	RX_CHAIN_ALL = 0xf
}RX_CHAIN_IDX;


/* */
/* BBP R49 TSSI (Transmit Signal Strength Indicator) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R49_STRUC {
	struct
	{
		u8 adc5_in_sel:1; /* 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390) */
		u8 bypassTSSIAverage:1; /* 0: the average TSSI (the average of the 16 samples), 1: the current TSSI */
		u8 Reserved:1; /* Reserved field */
		u8 TSSI:5; /* TSSI value */
	} field;

	u8 	byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#else
typedef union _BBP_R49_STRUC {
	struct
	{
		u8 TSSI:5; /* TSSI value */
		u8 Reserved:1; /* Reserved field */
		u8 bypassTSSIAverage:1; /* 0: the average TSSI (the average of the 16 samples), 1: the current TSSI */
		u8 adc5_in_sel:1; /* 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390) */
	} field;

	u8 	byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#endif

#define MAX_BBP_MSG_SIZE	4096


/* */
/* BBP & RF are using indirect access. Before write any value into it. */
/* We have to make sure there is no outstanding command pending via checking busy bit. */
/* */
#define MAX_BUSY_COUNT  100         /* Number of retry before failing access BBP & RF indirect register */
#define MAX_BUSY_COUNT_US 2000      /* Number of retry before failing access BBP & RF indirect register */

/*#define PHY_TR_SWITCH_TIME          5  // usec */

#define RSSI_FOR_VERY_LOW_SENSIBILITY   -35
#define RSSI_FOR_LOW_SENSIBILITY		-58
#define RSSI_FOR_MID_LOW_SENSIBILITY	-65 /*-80*/
#define RSSI_FOR_MID_SENSIBILITY		-90

/*****************************************************************************
	RF register Read/Write marco definition
 *****************************************************************************/


/*****************************************************************************
	BBP register Read/Write marco definitions.
	we read/write the bbp value by register's ID.
	Generate PER to test BA
 *****************************************************************************/


struct _RMTP_ADAPTER;

void mt7612u_bbp_set_bw(struct rtmp_adapter *pAd, u8 bw);
void mt7612u_bbp_set_ctrlch(struct rtmp_adapter *pAd, u8 ext_ch);
void mt7612u_bbp_set_rxpath(struct rtmp_adapter *pAd, int rxpath);
INT bbp_tx_comp_init(struct rtmp_adapter *pAd, INT adc_insel, INT tssi_mode);
void mt7612u_bbp_set_txdac(struct rtmp_adapter *pAd, int tx_dac);
INT bbp_set_mmps(struct rtmp_adapter *pAd, bool ReduceCorePower);
INT bbp_set_agc(struct rtmp_adapter *pAd, u8 agc, RX_CHAIN_IDX idx);
INT bbp_get_agc(struct rtmp_adapter *pAd, CHAR *agc, RX_CHAIN_IDX idx);
INT filter_coefficient_ctrl(struct rtmp_adapter *pAd, u8 Channel);
u8 get_random_seed_by_phy(struct rtmp_adapter *pAd);

int NICInitBBP(struct rtmp_adapter *pAd);

typedef struct phy_ops{
	u8 (*get_random_seed_by_phy)(struct rtmp_adapter *pAd);
	INT (*filter_coefficient_ctrl)(struct rtmp_adapter *pAd, u8 Channel);
	INT (*bbp_set_agc)(struct rtmp_adapter *pAd, u8 agc, RX_CHAIN_IDX chain);
	INT (*bbp_get_agc)(struct rtmp_adapter *pAd, CHAR *agc, RX_CHAIN_IDX chain);
	INT (*bbp_set_mmps)(struct rtmp_adapter *pAd, bool ReduceCorePower);
	INT (*bbp_set_ctrlch)(struct rtmp_adapter *pAd, UINT8 ext_ch);
	INT (*bbp_tx_comp_init)(struct rtmp_adapter *pAd, INT adc_insel, INT tssi_mode);
	INT (*bbp_init)(struct rtmp_adapter *pAd);
}PHY_OPS;


#endif /* __RTMP_PHY_H__ */

