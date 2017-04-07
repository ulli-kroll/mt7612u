/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	chlist.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __CHLIST_H__
#define __CHLIST_H__

#include "rtmp_type.h"
#include "rtmp_def.h"


typedef struct _CH_DESC {
	u8 FirstChannel;
	u8 NumOfCh;
	u8 ChannelProp;
}CH_DESC, *PCH_DESC;

typedef struct _COUNTRY_REGION_CH_DESC {
	u8 RegionIndex;
	PCH_DESC pChDesc;
}COUNTRY_REGION_CH_DESC, *PCOUNTRY_REGION_CH_DESC;

typedef struct _CH_FREQ_MAP_{
	uint16_t 	channel;
	uint16_t 	freqKHz;
}CH_FREQ_MAP;

extern CH_FREQ_MAP CH_HZ_ID_MAP[];
extern int CH_HZ_ID_MAP_NUM;


#define     MAP_CHANNEL_ID_TO_KHZ(_ch, _khz)                 \
			RTMP_MapChannelID2KHZ(_ch, (uint32_t *)&(_khz))
#define     MAP_KHZ_TO_CHANNEL_ID(_khz, _ch)                 \
			RTMP_MapKHZ2ChannelID(_khz, (INT *)&(_ch))

/* Check if it is Japan W53(ch52,56,60,64) channel. */
#define JapanChannelCheck(_ch)  ((_ch == 52) || (_ch == 56) || (_ch == 60) || (_ch == 64))

VOID N_ChannelCheck(struct rtmp_adapter *pAd);
u8 N_SetCenCh(struct rtmp_adapter *pAd, u8 channel);
bool N_ChannelGroupCheck(struct rtmp_adapter *pAd, u8 channel);
INT get_vht_neighbor_index(u8 channel);
bool AC_ChannelGroupCheck(struct rtmp_adapter *pAd, u8 channel);
UINT8 GetCuntryMaxTxPwr(
	IN struct rtmp_adapter *pAd,
	IN UINT8 channel);

VOID RTMP_MapChannelID2KHZ(
	IN u8 Ch,
	OUT uint32_t *pFreq);

VOID RTMP_MapKHZ2ChannelID(
	IN ULONG Freq,
	OUT INT *pCh);

u8 GetChannel_5GHZ(
	IN PCH_DESC pChDesc,
	IN u8 index);

u8 GetChannel_2GHZ(
	IN PCH_DESC pChDesc,
	IN u8 index);

u8 GetChannelFlag(
	IN PCH_DESC pChDesc,
	IN u8 index);

uint16_t TotalChNum(
	IN PCH_DESC pChDesc);

#endif /* __CHLIST_H__ */

