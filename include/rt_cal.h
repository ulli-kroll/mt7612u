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
	cmm_rf_cal.c

	Abstract:
	RF calibration and profile related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Arvin Tai     2012/05/02
*/

#define DPD_CAL_PASS_THRES		5
#define DPD_CAL_MAX_RETRY		5

int32_t CalcRCalibrationCode(
	IN Pstruct rtmp_adapter pAd,
	IN int32_t D1,
	IN int32_t D2);

INT Set_TestRxIQCalibration_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

VOID R_Calibration(
	IN Pstruct rtmp_adapter pAd);

VOID RtmpKickOutHwNullFrame(
	IN Pstruct rtmp_adapter pAd,
	IN bool bPrepareContent,
	IN bool bTransmit);

VOID DPD_IQ_Swap_AM_PM_Inversion(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR AntIdx);

VOID DPD_AM_AM_LUT_Scaling(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR AntIdx);

UCHAR DPD_Calibration(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR AntIdx);

VOID DoDPDCalibration(
	IN Pstruct rtmp_adapter pAd);

INT Set_DPDCalPassThres_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

INT Set_TestDPDCalibration_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

INT Set_TestDPDCalibrationTX0_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

INT Set_TestDPDCalibrationTX1_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

VOID LOFT_IQ_Calibration(
	IN struct rtmp_adapter *pAd);

bool BW_Filter_Calibration(
	IN Pstruct rtmp_adapter pAd,
	IN bool bTxCal);

VOID RxDCOC_Calibration(
	IN Pstruct rtmp_adapter pAd);

VOID RXIQ_Calibration(
	IN Pstruct rtmp_adapter pAd);

VOID RF_SELF_TXDC_CAL(
	IN Pstruct rtmp_adapter pAd);

