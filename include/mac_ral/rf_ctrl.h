/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rf_ctrl.h

	Abstract:
	Ralink wireless chip RF related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RF_CTRL_H__
#define __RF_CTRL_H__

/* ================================================================================= */
/* Register format  for RFCTRL                                                                                                                                               */
/* ================================================================================= */

#define	RF_CSR_CFG	0x500

typedef	union _RF_CSR_CFG_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		uint32_t Rsvd18:14;				/* Reserved */
		uint32_t RF_CSR_KICK:1;			/* kick RF register read/write */
		uint32_t RF_CSR_WR:1;			/* 0: read  1: write */
		uint32_t TESTCSR_RFACC_REGNUM:8;	/* RF register ID */
		uint32_t RF_CSR_DATA:8;			/* DATA */
	} non_bank;
#else
	struct {
		uint32_t RF_CSR_DATA:8;
		uint32_t TESTCSR_RFACC_REGNUM:8;
		uint32_t RF_CSR_WR:1;
		uint32_t RF_CSR_KICK:1;
		uint32_t Rsvd18:14;
	} non_bank;
#endif /* RT_BIG_ENDIAN */
	uint32_t word;
}RF_CSR_CFG_STRUC;


#define	RF_PA_MODE_CCK_1M				(pAd->chipCap.PAModeCCK[0])
#define	RF_PA_MODE_CCK_2M				(pAd->chipCap.PAModeCCK[1])
#define	RF_PA_MODE_CCK_5M				(pAd->chipCap.PAModeCCK[2])
#define	RF_PA_MODE_CCK_11M				(pAd->chipCap.PAModeCCK[3])

#define	RF_PA_MODE_OFDM_6M				(pAd->chipCap.PAModeOFDM[0])
#define	RF_PA_MODE_OFDM_9M				(pAd->chipCap.PAModeOFDM[1])
#define	RF_PA_MODE_OFDM_12M				(pAd->chipCap.PAModeOFDM[2])
#define	RF_PA_MODE_OFDM_18M				(pAd->chipCap.PAModeOFDM[3])
#define	RF_PA_MODE_OFDM_24M				(pAd->chipCap.PAModeOFDM[4])
#define	RF_PA_MODE_OFDM_36M				(pAd->chipCap.PAModeOFDM[5])
#define	RF_PA_MODE_OFDM_48M				(pAd->chipCap.PAModeOFDM[6])
#define	RF_PA_MODE_OFDM_54M				(pAd->chipCap.PAModeOFDM[7])

#define	RF_PA_MODE_HT_MCS0				(pAd->chipCap.PAModeHT[0])
#define	RF_PA_MODE_HT_MCS1				(pAd->chipCap.PAModeHT[1])
#define	RF_PA_MODE_HT_MCS2				(pAd->chipCap.PAModeHT[2])
#define	RF_PA_MODE_HT_MCS3				(pAd->chipCap.PAModeHT[3])
#define	RF_PA_MODE_HT_MCS4				(pAd->chipCap.PAModeHT[4])
#define	RF_PA_MODE_HT_MCS5				(pAd->chipCap.PAModeHT[5])
#define	RF_PA_MODE_HT_MCS6				(pAd->chipCap.PAModeHT[6])
#define	RF_PA_MODE_HT_MCS7				(pAd->chipCap.PAModeHT[7])
#define	RF_PA_MODE_HT_MCS8				(pAd->chipCap.PAModeHT[8])
#define	RF_PA_MODE_HT_MCS9				(pAd->chipCap.PAModeHT[9])
#define	RF_PA_MODE_HT_MCS10				(pAd->chipCap.PAModeHT[10])
#define	RF_PA_MODE_HT_MCS11				(pAd->chipCap.PAModeHT[11])
#define	RF_PA_MODE_HT_MCS12				(pAd->chipCap.PAModeHT[12])
#define	RF_PA_MODE_HT_MCS13				(pAd->chipCap.PAModeHT[13])
#define	RF_PA_MODE_HT_MCS14				(pAd->chipCap.PAModeHT[14])
#define	RF_PA_MODE_HT_MCS15				(pAd->chipCap.PAModeHT[15])

#ifdef MT_RF
#define W_RFDATA 0x0524
#define R_RFDATA 0x052C
#define RF_CTRL 0x0528
#define RF_ADDR_MASK 0xfff
#define RF_ADDR(p) ((p) & 0xfff)
#define RF_R_W_CTRL (1 << 12)
#define RF_READY_MASK (1<< 13)
#define RF_READY(p) (((p) & RF_READY_MASK) == (0x0 << 13))
#define RF_IDX_MASK (1 << 16)
#define RF_IDX(p) (((p) & 0x1) << 16)
#endif /* MT_RF */


typedef struct _BANK_RF_REG_PAIR {
	UCHAR Bank;
	UCHAR Register;
	UCHAR Value;
} BANK_RF_REG_PAIR, *PBANK_RF_REG_PAIR;

typedef struct _R_M_W_REG{
	uint32_t Register;
	uint32_t ClearBitMask;
	uint32_t Value;
} R_M_W_REG, *PR_M_W_REG;

typedef struct _RF_R_M_W_REG{
	UCHAR Bank;
	UCHAR Register;
	UCHAR ClearBitMask;
	UCHAR Value;
} RF_R_M_W_REG, *PRF_R_M_W_REG;

#endif /* __RF_CTRL_H__ */

