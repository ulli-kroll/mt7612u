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

    Abstract:


 */

#include "ap_autoChSel_cmm.h"

#ifndef __AUTOCHSELECT_H__
#define __AUTOCHSELECT_H__

#define AP_AUTO_CH_SEL(__P, __O)	APAutoSelectChannel((__P), (__O))

ULONG AutoChBssSearchWithSSID(
	IN struct rtmp_adapter *pAd,
	IN u8 *Bssid,
	IN u8 *pSsid,
	IN u8 SsidLen,
	IN u8 Channel);

VOID APAutoChannelInit(
	IN struct rtmp_adapter *pAd);

VOID UpdateChannelInfo(
	IN struct rtmp_adapter *pAd,
	IN int ch,
	IN ChannelSel_Alg Alg);

ULONG AutoChBssInsertEntry(
	IN struct rtmp_adapter *pAd,
	IN u8 *pBssid,
	IN CHAR Ssid[],
	IN u8 SsidLen,
	IN u8 ChannelNo,
	IN u8 ExtChOffset,
	IN CHAR Rssi);

VOID AutoChBssTableInit(
	IN struct rtmp_adapter *pAd);

VOID ChannelInfoInit(
	IN struct rtmp_adapter *pAd);

VOID AutoChBssTableDestroy(
	IN struct rtmp_adapter *pAd);

VOID ChannelInfoDestroy(
	IN struct rtmp_adapter *pAd);

VOID CheckPhyModeIsABand(
	IN struct rtmp_adapter *pAd);

u8 SelectBestChannel(
	IN struct rtmp_adapter *pAd,
	IN ChannelSel_Alg Alg);

u8 APAutoSelectChannel(
	IN struct rtmp_adapter *pAd,
	IN ChannelSel_Alg Alg);

#ifdef AP_SCAN_SUPPORT
VOID AutoChannelSelCheck(
	IN struct rtmp_adapter *pAd);
#endif /* AP_SCAN_SUPPORT */

#endif /* __AUTOCHSELECT_H__ */

