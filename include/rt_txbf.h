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
	Tx Beamforming related constants and data structures

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Shiang     2010/06/29
*/


#ifndef _RT_TXBF_H_
#define _RT_TXBF_H_


// Divider phase calibration closed loop definition
#define RX0TX0     0
#define RX1TX1     5

#define ADC0_RX0_2R   8
#define ADC1_RX1_2R   8

//#define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station


// TxSndgPkt Sounding type definitions
#define SNDG_TYPE_DISABLE	0
#define SNDG_TYPE_SOUNDING	1
#define SNDG_TYPE_NDP		2

// Explicit TxBF feedback mechanism
#define ETXBF_FB_DISABLE	0
#define ETXBF_FB_CSI		1
#define ETXBF_FB_NONCOMP	2
#define ETXBF_FB_COMP		4


//#define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station

/*
	eTxBfEnCond values:
	 0:no etxbf,
	 1:etxbf update periodically,
	 2:etxbf updated if mcs changes in RateSwitchingAdapt() or APQuickResponeForRateUpExecAdapt().
	 3:auto-selection: if mfb changes or timer expires, then send sounding packets <------not finished yet!!!
	 note:
		when = 1 or 3, NO_SNDG_CNT_THRD controls the frequency to update the
		matrix(ETXBF_EN_COND=1) or activate the whole bf evaluation process(not defined)
*/

// Defines to include optional code.
//	NOTE: Do not define these options. ETxBfEnCond==3 and
//		MCS Feedback are not fully implemented
//#define ETXBF_EN_COND3_SUPPORT	// Include ETxBfEnCond==3 code
//#define MFB_SUPPORT				// Include MCS Feedback code

// MCS FB definitions
#define MSI_TOGGLE_BF		6
#define TOGGLE_BF_PKTS		5// the number of packets with inverted BF status

// TXBF State definitions
#define READY_FOR_SNDG0		0//jump to WAIT_SNDG_FB0 when channel change or periodically
#define WAIT_SNDG_FB0		1//jump to WAIT_SNDG_FB1 when bf report0 is received
#define WAIT_SNDG_FB1		2
#define WAIT_MFB			3
#define WAIT_USELESS_RSP	4
#define WAIT_BEST_SNDG		5

#define NO_SNDG_CNT_THRD	0//send sndg packet if there is no sounding for (NO_SNDG_CNT_THRD+1)*500msec. If this =0, bf matrix is updated at each call of APMlmeDynamicTxRateSwitchingAdapt()


// ------------ BEAMFORMING PROFILE HANDLING ------------

#define IMP_MAX_BYTES		14		// Implicit: 14 bytes per subcarrier
#define IMP_MAX_BYTES_ONE_COL	7	// Implicit: 7 bytes per subcarrier, when reading first column
#define EXP_MAX_BYTES		18		// Explicit: 18 bytes per subcarrier
#define MAX_BYTES            2      // 2 bytes per subcarrier for implicit and explicit TxBf
#define IMP_COEFF_SIZE		 9		// 9 bits/coeff
#define IMP_COEFF_MASK		0x1FF

#define PROFILE_MAX_CARRIERS_20		56		// Number of subcarriers in 20 MHz mode
#define PROFILE_MAX_CARRIERS_40		114		// Number of subcarriers in 40 MHz mode
#define PROFILE_MAX_CARRIERS_80		242		// Number of subcarriers in 80 MHz mode

#define NUM_CHAIN			 3

// Indices of valid rows in Implicit and Explicit profiles for 20 and 40 MHz
typedef struct {
	int lwb1, upb1;
	int lwb2, upb2;
} SC_TABLE_ENTRY;


typedef struct {
	bool impProfile;
	bool fortyMHz;
	int rows, columns;
	int grouping;
	u8 tag[EXP_MAX_BYTES];
	u8 data[PROFILE_MAX_CARRIERS_40][EXP_MAX_BYTES];
} PROFILE_DATA;

extern PROFILE_DATA profData;

typedef struct {
	u8  ng, cw, nrow, ncol, LM;
	u8  LD, EO, IO, I_E;
	u8  DMAC[6];
	u8  Tx1_scale_2ss, Tx0_scale_2ss, Tx1_scale_1ss, Tx0_scale_1ss;
	u8  STS1_SNR, STS0_SNR;
	unsigned short timeout;
	u8  validFlg;
	u8  CMDInIdx;
} PFMU_PROFILE;

typedef struct {
	u8  dCMDInIdx;
	u8  psi21, phill;
	u8  data[PROFILE_MAX_CARRIERS_80][MAX_BYTES];
} PFMU_DATA;


typedef
struct {
	u8 E1gBeg;
	u8 E1gEnd;
	u8 E1aHighBeg;
	u8 E1aHighEnd;
	u8 E1aLowBeg;
	u8 E1aLowEnd;
	u8 E1aMidBeg;
	u8 E1aMidMid;
	u8 E1aMidEnd;
} ITXBF_PHASE_PARAMS;			// ITxBF BBP reg phase calibration parameters

typedef
struct {
	u8 E1gBeg[3];
	u8 E1gEnd[3];
	u8 E1aHighBeg[3];
	u8 E1aHighEnd[3];
	u8 E1aLowBeg[3];
	u8 E1aLowEnd[3];
	u8 E1aMidBeg[3];
	u8 E1aMidMid[3];
	u8 E1aMidEnd[3];
} ITXBF_LNA_PARAMS;			// ITxBF BBP reg LNA calibration parameters

typedef
struct {
	u8 E1gBeg;
	u8 E1gEnd;
	u8 E1aHighBeg;
	u8 E1aHighEnd;
	u8 E1aLowBeg;
	u8 E1aLowEnd;
	u8 E1aMidBeg;
	u8 E1aMidMid;
	u8 E1aMidEnd;
} ITXBF_DIV_PARAMS;				// ITxBF Divider Calibration parameters

INT ITxBFDividerCalibration(
	IN struct rtmp_adapter *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT u8 *divPhase);

VOID ITxBFLoadLNAComp(
	IN struct rtmp_adapter *pAd);

int ITxBFLNACalibration(
	IN struct rtmp_adapter *pAd,
	IN int calFunction,
	IN int calMethod,
	IN bool gBand);

void Read_TxBfProfile(
	IN	struct rtmp_adapter *pAd,
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum,
	IN	bool			implicitProfile);

void Write_TxBfProfile(
	IN	struct rtmp_adapter *pAd,
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum);

void Read_TagField(
	IN	struct rtmp_adapter *pAd,
	IN  u8 *row,
	IN  int		profileNum);

// Write_TagField - write a profile tagfield
void Write_TagField(
	IN	struct rtmp_adapter *pAd,
	IN  u8 *row,
	IN  int		profileNum);


// displayTagfield - display one tagfield
void displayTagfield(
	IN	struct rtmp_adapter *pAd,
	IN	int		profileNum,
	IN	bool implicitProfile);

// Unpack an ITxBF matrix element from a row of bytes
int Unpack_IBFValue(
	IN u8 *row,
	IN int elemNum);



#endif // _RT_TXBF_H_
