/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifndef __WNM_H__
#define __WNM_H__

#include "ipv6.h"

#define BTM_MACHINE_BASE 0
#define WaitPeerBTMRspTimeoutVale 1024

/* BTM states */
enum BTM_STATE {
	WAIT_BTM_QUERY,
	WAIT_PEER_BTM_QUERY,
	WAIT_BTM_REQ,
	WAIT_BTM_RSP,
	WAIT_PEER_BTM_REQ,
	WAIT_PEER_BTM_RSP,
	BTM_UNKNOWN,
	MAX_BTM_STATE,
};


/* BTM events */
enum BTM_EVENT {
	BTM_QUERY,
	PEER_BTM_QUERY,
	BTM_REQ,
	BTM_RSP,
	PEER_BTM_REQ,
	PEER_BTM_RSP,
	MAX_BTM_MSG,
};

#define BTM_FUNC_SIZE (MAX_BTM_STATE * MAX_BTM_MSG)

enum IPV6_TYPE{
	IPV6_LINK_LOCAL,
	IPV6_GLOBAL,
};

typedef struct GNU_PACKED _BTM_EVENT_DATA {
	u8 ControlIndex;
	u8 PeerMACAddr[MAC_ADDR_LEN];
	uint16_t EventType;
	union {
#ifdef CONFIG_STA_SUPPORT
		struct {
			u8 DialogToken;
			uint16_t BTMQueryLen;
			u8 BTMQuery[0];
		} GNU_PACKED BTM_QUERY_DATA;

		struct {
			u8 DialogToken;
			uint16_t BTMRspLen;
			u8 BTMRsp[0];
		} GNU_PACKED BTM_RSP_DATA;

		struct {
			u8 DialogToken;
			uint16_t BTMReqLen;
			u8 BTMReq[0];
		} GNU_PACKED PEER_BTM_REQ_DATA;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		struct {
			u8 DialogToken;
			uint16_t BTMReqLen;
			u8 BTMReq[0];
		} GNU_PACKED BTM_REQ_DATA;

		struct {
			u8 DialogToken;
			uint16_t BTMQueryLen;
			u8 BTMQuery[0];
		} GNU_PACKED PEER_BTM_QUERY_DATA;

		struct {
			u8 DialogToken;
			uint16_t BTMRspLen;
			u8 BTMRsp[0];
		} GNU_PACKED PEER_BTM_RSP_DATA;
#endif /* CONFIG_AP_SUPPORT */
	}u;
} BTM_EVENT_DATA, *PBTM_EVENT_DATA;

typedef struct _BTM_PEER_ENTRY {
	DL_LIST List;
	enum BTM_STATE CurrentState;
	u8 ControlIndex;
	u8 PeerMACAddr[MAC_ADDR_LEN];
	u8 DialogToken;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT WaitPeerBTMRspTimer;
#endif /* CONFIG_AP_SUPPORT */
} BTM_PEER_ENTRY, *PBTM_PEER_ENTRY;

typedef struct _PROXY_ARP_IPV4_ENTRY {
	DL_LIST List;
	u8 TargetMACAddr[MAC_ADDR_LEN];
	u8 TargetIPAddr[4];
} PROXY_ARP_IPV4_ENTRY, *PPROXY_ARP_IPV4_ENTRY;

typedef struct _PROXY_ARP_IPV4_UNIT {
	u8 TargetMACAddr[MAC_ADDR_LEN];
	u8 TargetIPAddr[4];
} PROXY_ARP_IPV4_UNIT, *PPROXY_ARP_IPV4_UNIT;

typedef struct _PROXY_ARP_IPV6_ENTRY {
	DL_LIST List;
	u8 TargetMACAddr[MAC_ADDR_LEN];
	u8 TargetIPType;
	u8 TargetIPAddr[16];
} PROXY_ARP_IPV6_ENTRY, *PPROXY_ARP_IPV6_ENTRY;

typedef struct _PROXY_ARP_IPV6_UNIT {
	u8 TargetMACAddr[MAC_ADDR_LEN];
	u8 TargetIPType;
	u8 TargetIPAddr[16];
} PROXY_ARP_IPV6_UNIT, *PPROXY_ARP_IPV6_UNIT;

typedef struct _WNM_CTRL {
	uint32_t TimeadvertisementIELen;
	uint32_t TimezoneIELen;
	u8 *TimeadvertisementIE;
	u8 *TimezoneIE;
	struct semaphore BTMPeerListLock;
	bool ProxyARPEnable;
	struct semaphore ProxyARPListLock;
	struct semaphore ProxyARPIPv6ListLock;
	DL_LIST IPv4ProxyARPList;
	DL_LIST IPv6ProxyARPList;
	DL_LIST BTMPeerList;
} WNM_CTRL, *PWNM_CTRL;

enum IPTYPE {
	IPV4,
	IPV6
};

bool IsGratuitousARP(IN u8 *pData);

bool IsUnsolicitedNeighborAdver(Pstruct rtmp_adapter pAd,
								   u8 *pData);

bool IsIPv4ProxyARPCandidate(IN Pstruct rtmp_adapter pAd,
								IN u8 *pData);

bool IsIPv6ProxyARPCandidate(IN Pstruct rtmp_adapter pAd,
								IN u8 *pData);

bool IsIPv6RouterSolicitation(IN Pstruct rtmp_adapter pAd,
								 IN u8 *pData);

bool IsIPv6RouterAdvertisement(IN Pstruct rtmp_adapter pAd,
								  IN u8 *pData);

bool IsTDLSPacket(IN Pstruct rtmp_adapter pAd,
					 IN u8 *pData);

struct _MULTISSID_STRUCT;

bool IPv4ProxyARP(IN Pstruct rtmp_adapter pAd,
				 	 IN struct _MULTISSID_STRUCT *pMbss,
				 	 IN u8 *pData,
					 IN bool FromDS);

bool IsIpv6DuplicateAddrDetect(Pstruct rtmp_adapter pAd,
								  u8 *pData);

bool IPv6ProxyARP(IN Pstruct rtmp_adapter pAd,
					 IN struct _MULTISSID_STRUCT *pMbss,
					 IN u8 *pData,
					 IN bool FromDS);

uint32_t AddIPv4ProxyARPEntry(IN Pstruct rtmp_adapter pAd,
					   		IN struct _MULTISSID_STRUCT *pMbss,
							IN u8 *pTargetMACAddr,
							IN u8 *pTargetIPAddr);

uint32_t AddIPv6ProxyARPEntry(IN Pstruct rtmp_adapter pAd,
					   		IN struct _MULTISSID_STRUCT *pMbss,
							IN u8 *pTargetMACAddr,
							IN u8 *pTargetIPAddr);

uint32_t IPv4ProxyARPTableLen(IN Pstruct rtmp_adapter pAd,
							IN struct _MULTISSID_STRUCT *pMbss);

uint32_t IPv6ProxyARPTableLen(IN Pstruct rtmp_adapter pAd,
							IN struct _MULTISSID_STRUCT *pMbss);

bool GetIPv4ProxyARPTable(IN Pstruct rtmp_adapter pAd,
							 IN struct _MULTISSID_STRUCT *pMbss,
							 OUT	u8 **ProxyARPTable);

bool GetIPv6ProxyARPTable(IN Pstruct rtmp_adapter pAd,
							 IN struct _MULTISSID_STRUCT *pMbss,
							 OUT	u8 **ProxyARPTable);

VOID RemoveIPv4ProxyARPEntry(IN Pstruct rtmp_adapter pAd,
					   		IN struct _MULTISSID_STRUCT *pMbss,
							u8 *pTargetMACAddr);

VOID RemoveIPv6ProxyARPEntry(IN Pstruct rtmp_adapter pAd,
							IN struct _MULTISSID_STRUCT *pMbss,
							u8 *pTargetMACAddr);

VOID WNMCtrlInit(IN Pstruct rtmp_adapter pAd);
VOID WNMCtrlExit(IN Pstruct rtmp_adapter pAd);
VOID Clear_All_PROXY_TABLE(IN Pstruct rtmp_adapter pAd);
#ifdef CONFIG_AP_SUPPORT
VOID WNMIPv4ProxyARPCheck(
			IN Pstruct rtmp_adapter pAd,
			struct sk_buff *pPacket,
			unsigned short srcPort,
			unsigned short dstPort,
			u8 *pSrcBuf);

VOID WNMIPv6ProxyARPCheck(
			IN Pstruct rtmp_adapter pAd,
			struct sk_buff *pPacket,
			u8 *pSrcBuf);
#endif /* CONFIG_AP_SUPPORT */


#endif /* __WNM_H__ */

