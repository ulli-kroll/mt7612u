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
	gas.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __GAS_H__
#define __GAS_H__

#include "link_list.h"

#define GAS_MACHINE_BASE 0

/* gas states */
enum GAS_STATE {
	WAIT_GAS_REQ,
	WAIT_GAS_RSP,
	WAIT_PEER_GAS_REQ,
	WAIT_PEER_GAS_RSP,
	WAIT_GAS_CB_REQ,
	WAIT_GAS_CB_RSP,
	GAS_UNKNOWN,
	MAX_GAS_STATE,
};

/* gas events */
enum GAS_EVENT {
	GAS_REQ,
	GAS_RSP,
	GAS_RSP_MORE,
	PEER_GAS_REQ,
	PEER_GAS_RSP,
	PEER_GAS_RSP_MORE,
	GAS_CB_REQ,
	GAS_CB_REQ_MORE,
	GAS_CB_RSP,
	GAS_CB_RSP_MORE,
	MAX_GAS_MSG,
};

#define GAS_FUNC_SIZE (MAX_GAS_STATE * MAX_GAS_MSG)

typedef struct _GAS_QUERY_RSP_FRAGMENT
{
	DL_LIST List;
	u8 GASRspFragID;
	uint16_t FragQueryRspLen;
	u8 *FragQueryRsp;
}GAS_QUERY_RSP_FRAGMENT, *PGAS_QUERY_RSP_FRAGMENT;

typedef struct _GAS_PEER_ENTRY {
	DL_LIST List;
	enum GAS_STATE CurrentState;
	u8 ControlIndex;
	u8 PeerMACAddr[MAC_ADDR_LEN];
	u8 DialogToken;
	u8 AdvertisementProID;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PostReplyTimer;
	bool PostReplyTimerRunning;
	RALINK_TIMER_STRUCT GASRspBufferingTimer;
	bool GASRspBufferingTimerRunning;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	RALINK_TIMER_STRUCT GASResponseTimer;
	bool GASResponseTimerRunning;
	RALINK_TIMER_STRUCT GASCBDelayTimer;
	bool GASCBDelayTimerRunning;
#endif /* CONFIG_STA_SUPPORT */
	u8 GASRspFragNum;
	u8 CurrentGASFragNum;
	uint32_t AllocResource;
	uint32_t FreeResource;
	u8 QueryNum;
	DL_LIST GASQueryRspFragList;
}GAS_PEER_ENTRY, *PGAS_PEER_ENTRY;

typedef struct _GAS_CTRL {
	DL_LIST GASPeerList;
	NDIS_SPIN_LOCK GASPeerListLock;
	UINT8 ExternalANQPServerTest;
} GAS_CTRL, *PGAS_CTRL;

/*
 * gas events data
 * GASComebackDelay : unit(TU)
 */
typedef struct GNU_PACKED _GAS_EVENT_DATA {
	u8 ControlIndex;
	u8 PeerMACAddr[MAC_ADDR_LEN];
	uint16_t EventType;
	union{
#ifdef CONFIG_STA_SUPPORT
		struct {
			u8 DialogToken;
			u8 AdvertisementProID;
			uint16_t QueryReqLen;
			u8 QueryReq[0];
		} GNU_PACKED GAS_REQ_DATA;
		struct {
			uint16_t StatusCode;
			u8 AdvertisementProID;
			uint16_t QueryRspLen;
			u8 QueryRsp[0];
		} GNU_PACKED PEER_GAS_RSP_DATA;
		struct {
			u8 DialogToken;
		} GNU_PACKED PEER_GAS_RSP_MORE_DATA;
		struct {
			uint16_t StatusCode;
			u8 AdvertisementProID;
			uint16_t QueryRspLen;
			u8 QueryRsp[0];
		} GNU_PACKED GAS_CB_RSP_DATA;
		struct {
			u8 DialogToken;
		} GNU_PACKED GAS_CB_RSP_MORE_DATA;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		struct {
			u8 DialogToken;
			uint16_t StatusCode;
			uint16_t GASComebackDelay;
			u8 AdvertisementProID;
			uint16_t QueryRspLen;
			u8 QueryRsp[0];
		} GNU_PACKED GAS_RSP_DATA;
		struct {
			u8 DialogToken;
			uint16_t StatusCode;
			uint16_t GASComebackDelay;
			u8 AdvertisementProID;
		} GNU_PACKED GAS_RSP_MORE_DATA;
		struct {
			u8 DialogToken;
			u8 AdvertisementProID;
			uint16_t QueryReqLen;
			u8 QueryReq[0];
		} GNU_PACKED PEER_GAS_REQ_DATA;
		struct {
			u8 DialogToken;
			u8 AdvertisementProID;
			uint16_t StatusCode;
		} GNU_PACKED GAS_CB_REQ_DATA;
		struct {
			u8 DialogToken;
			u8 AdvertisementProID;
			uint16_t StatusCode;
		} GNU_PACKED GAS_CB_REQ_MORE_DATA;
#endif /* CONFIG_AP_SUPPORT */
	}u;
}GAS_EVENT_DATA, *PGAS_EVENT_DATA;

VOID GASStateMachineInit(
	IN	Pstruct rtmp_adapter 	pAd,
	IN	STATE_MACHINE		*S,
	OUT	STATE_MACHINE_FUNC	Trans[]);

enum GAS_STATE GASPeerCurrentState(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID GASSetPeerCurrentState(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum GAS_STATE State);

VOID GASCtrlExit(IN Pstruct rtmp_adapter pAd);

#ifdef CONFIG_STA_SUPPORT
VOID ReceiveGASInitRsp(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ReceiveGASCBRsp(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem);

void SendAnqpRspEvent(void *net_dev, const char *peer_mac_addr,
				u16 status, const char *anqp_rsp, u16 anqp_rsp_len);

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(PostReplyTimeout);
DECLARE_TIMER_FUNCTION(GASRspBufferingTimeout);

void SendAnqpReqEvent(struct net_device *net_dev, const char *peer_mac_addr,
				const char *anqp_req, u16 anqp_req_len);

VOID ReceiveGASInitReq(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ReceiveGASCBReq(
	IN Pstruct rtmp_adapter pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif /* CONFIG_AP_SUPPORT */

#endif /* __GAS_H__ */
