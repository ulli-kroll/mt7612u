/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hotspot.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#error "For HOTSPOT2.0 feature, you must define the compile flag -DCONFIG_DOT11V_WNM"

#error "For HOTSPOT2.0 feature, you must define the compile flag -DCONFIG_DOT11U_INTERWORKING"

#include "rtmp.h"

#define HSCTRL_MACHINE_BASE 0

enum HSCTRL_STATE {
	WAIT_HSCTRL_OFF,
	WAIT_HSCTRL_ON,
	HSCTRL_IDLE,
	MAX_HSCTRL_STATE,
};

enum HSCTRL_EVENT {
	HSCTRL_OFF,
	HSCTRL_ON,
	MAX_HSCTRL_MSG,
};

typedef struct GNU_PACKED _HSCTRL_EVENT_DATA {
	u8 ControlIndex;
	u8 EventTrigger;
	u8 EventType;
} HSCTRL_EVENT_DATA, *PHSCTRL_EVENT_DATA;

typedef struct _HOTSPOT_CTRL {
	uint32_t HSIndicationIELen;
	uint32_t P2PIELen;
	uint32_t InterWorkingIELen;
	uint32_t AdvertisementProtoIELen;
	uint32_t QosMapSetIELen;
	uint32_t RoamingConsortiumIELen;
	u8 *InterWorkingIE;
	u8 *AdvertisementProtoIE;
	u8 *QosMapSetIE;
	u8 *RoamingConsortiumIE;
	u8 *HSIndicationIE;
	u8 *P2PIE;
	bool  HSDaemonReady;
	bool HotSpotEnable;
	enum HSCTRL_STATE HSCtrlState;
	bool IsHessid;
	u8 Hessid[MAC_ADDR_LEN];
	UINT8 AccessNetWorkType;
	bool DGAFDisable;
	UINT8 L2Filter;
	bool ICMPv4Deny;
	uint32_t MMPDUSize;
} HOTSPOT_CTRL, *PHOTSPOT_CTRL;

enum {
	L2FilterDisable,
	L2FilterBuiltIn,
	L2FilterExternal,
};

enum {
	PARAM_DGAF_DISABLED,
	PARAM_PROXY_ARP,
	PARAM_L2_FILTER,
	PARAM_ICMPV4_DENY,
	PARAM_MMPDU_SIZE,
	PARAM_EXTERNAL_ANQP_SERVER_TEST,
};

bool L2FilterInspection(
			IN Pstruct rtmp_adapter pAd,
			IN PHOTSPOT_CTRL pHSCtrl,
			IN u8 *pData);

VOID HSCtrlStateMachineInit(
	IN	Pstruct rtmp_adapter 	pAd,
	IN	STATE_MACHINE		*S,
	OUT	STATE_MACHINE_FUNC	Trans[]);

INT Set_STAHotSpot_OnOff(
	IN Pstruct rtmp_adapter pAd,
	IN UINT8 OnOff);

enum HSCTRL_STATE HSCtrlCurrentState(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem);

bool HotSpotEnable(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN INT Type);

VOID HSCtrlExit(
	IN Pstruct rtmp_adapter pAd);

VOID HSCtrlHalt(
	IN Pstruct rtmp_adapter pAd);

INT Set_HotSpot_OnOff(
	IN Pstruct rtmp_adapter pAd,
	IN UINT8 OnOff,
	IN UINT8 EventTrigger,
	IN UINT8 EventType);

struct _PEER_PROBE_REQ_PARAM;

bool ProbeReqforHSAP(
	IN Pstruct rtmp_adapter pAd,
	IN u8 APIndex,
	IN struct _PEER_PROBE_REQ_PARAM *ProbeReqParam);

VOID Clear_Hotspot_All_IE(IN Pstruct rtmp_adapter PAd);

#define isBcastEtherAddr(addr)  ((addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5])== 0xff)

#define GAS_STATE_MESSAGES    0
#define ACTION_STATE_MESSAGES 1

void HotspotOnOffEvent(struct net_device *net_dev, int onoff);
void HotspotAPReload(struct net_device *net_dev);

INT Set_HotSpot_Param(
	IN Pstruct rtmp_adapter pAd,
	uint32_t Param,
	uint32_t Value);

enum {
	HS_ON_OFF_BASE,
	HS_AP_RELOAD,
};

#ifdef CONFIG_AP_SUPPORT
bool HSIPv4Check(
			IN Pstruct rtmp_adapter pAd,
			u8 *pWcid,
			struct sk_buff *pPacket,
			u8 *pSrcBUf,
			uint16_t srcPort,
			uint16_t dscPort);
#endif
#endif /* __HOTSPOT_H__ */
