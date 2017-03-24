/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_apcli.h

    Abstract:
    Support AP-Client function.

    Revision History:
    Who               When            What
    --------------    ----------      ----------------------------------------------
    Shiang, Fonchi    02-13-2007      created
*/

#ifndef _AP_APCLI_H_
#define _AP_APCLI_H_

#ifdef APCLI_SUPPORT

#include "rtmp.h"

#define AUTH_TIMEOUT	300         /* unit: msec */
#define ASSOC_TIMEOUT	300         /* unit: msec */
/*#define JOIN_TIMEOUT	2000        // unit: msec // not used in Ap-client mode, remove it */
#define PROBE_TIMEOUT	1000        /* unit: msec */
#ifdef APCLI_CONNECTION_TRIAL
#define TRIAL_TIMEOUT	400	/* unit: msec */
#endif /* APCLI_CONNECTION_TRIAL */

#define APCLI_ROOT_BSSID_GET(pAd, wcid) ((pAd)->MacTab.Content[(wcid)].Addr)

/* sanity check for apidx */
#define APCLI_MR_APIDX_SANITY_CHECK(idx) \
{ \
	if ((idx) >= MAX_APCLI_NUM) \
	{ \
		(idx) = 0; \
		DBGPRINT(RT_DEBUG_ERROR, ("%s> Error! apcli-idx > MAX_APCLI_NUM!\n", __FUNCTION__)); \
	} \
}

typedef struct _APCLI_MLME_JOIN_REQ_STRUCT {
	UCHAR	Bssid[MAC_ADDR_LEN];
	UCHAR	SsidLen;
	UCHAR	Ssid[MAX_LEN_OF_SSID];
} APCLI_MLME_JOIN_REQ_STRUCT;

typedef struct _APCLI_CTRL_MSG_STRUCT {
	USHORT Status;
	UCHAR SrcAddr[MAC_ADDR_LEN];
} APCLI_CTRL_MSG_STRUCT, *PSTA_CTRL_MSG_STRUCT;

bool isValidApCliIf(
	SHORT ifIndex);

VOID ApCliCtrlStateMachineInit(
	IN Pstruct rtmp_adapter pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliSyncStateMachineInit(
    IN Pstruct rtmp_adapter pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliAuthStateMachineInit(
    IN Pstruct rtmp_adapter pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliAssocStateMachineInit(
    IN Pstruct rtmp_adapter pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(
	IN struct rtmp_adapter *pAd,
	IN UCHAR wcid,
	IN UCHAR *pAddrs);

bool 	ApCliValidateRSNIE(
	IN struct rtmp_adapter *pAd,
	IN 		PEID_STRUCT    	pEid_ptr,
	IN		USHORT			eid_len,
	IN		USHORT			idx);


VOID ApCli_Remove(
	IN Pstruct rtmp_adapter 	pAd);

VOID RT28xx_ApCli_Close(
	IN Pstruct rtmp_adapter 	pAd);



INT ApCliIfLookUp(
	IN Pstruct rtmp_adapter pAd,
	IN u8 *pAddr);


VOID ApCliMgtMacHeaderInit(
    IN	Pstruct rtmp_adapter pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN u8 *pDA,
    IN u8 *pBssid,
    IN USHORT ifIndex);

bool ApCliCheckHt(
	IN		Pstruct rtmp_adapter 		pAd,
	IN		USHORT 				IfIndex,
	IN OUT	HT_CAPABILITY_IE 	*pHtCapability,
	IN OUT	ADD_HT_INFO_IE 		*pAddHtInfo);

bool ApCliCheckVht(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR Wcid,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE *vht_op);

bool ApCliLinkUp(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR ifIndex);

VOID ApCliLinkDown(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR ifIndex);

VOID ApCliIfUp(
	IN Pstruct rtmp_adapter pAd);

VOID ApCliIfDown(
	IN Pstruct rtmp_adapter pAd);

VOID ApCliIfMonitor(
	IN Pstruct rtmp_adapter pAd);

bool ApCliMsgTypeSubst(
	IN Pstruct rtmp_adapter  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

bool preCheckMsgTypeSubset(
	IN Pstruct rtmp_adapter  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

bool ApCliPeerAssocRspSanity(
    IN Pstruct rtmp_adapter pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
    OUT u8 *pAddr2,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pStatus,
    OUT USHORT *pAid,
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
    OUT UCHAR *pHtCapabilityLen,
    OUT UCHAR *pAddHtInfoLen,
    OUT UCHAR *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag,
    OUT IE_LISTS *ie_list);

VOID	ApCliPeerPairMsg1Action(
	IN Pstruct rtmp_adapter    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID	ApCliPeerPairMsg3Action(
	IN Pstruct rtmp_adapter    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID	ApCliPeerGroupMsg1Action(
	IN Pstruct rtmp_adapter    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

bool ApCliCheckRSNIE(
	IN  Pstruct rtmp_adapter   pAd,
	IN  u8 *         pData,
	IN  UCHAR           DataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	OUT	UCHAR			*Offset);

bool ApCliParseKeyData(
	IN  Pstruct rtmp_adapter   pAd,
	IN  u8 *         pKeyData,
	IN  UCHAR           KeyDataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			IfIdx,
	IN	UCHAR			bPairewise);

bool  ApCliHandleRxBroadcastFrame(
	IN  Pstruct rtmp_adapter   pAd,
	IN	RX_BLK			*pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			FromWhichBSSID);

VOID APCliInstallPairwiseKey(
	IN  Pstruct rtmp_adapter   pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

bool APCliInstallSharedKey(
	IN  Pstruct rtmp_adapter   pAd,
	IN  u8 *         pKey,
	IN  UCHAR           KeyLen,
	IN	UCHAR			DefaultKeyIdx,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID ApCliUpdateMlmeRate(struct rtmp_adapter *pAd, USHORT ifIndex);

VOID APCli_Init(struct rtmp_adapter *pAd,struct RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);

bool ApCli_Open(struct rtmp_adapter *pAd, struct net_device *dev_p);
bool ApCli_Close(struct rtmp_adapter *pAd, struct net_device *dev_p);

bool ApCliWaitProbRsp(struct rtmp_adapter *pAd, USHORT ifIndex);
VOID ApCliSimulateRecvBeacon(struct rtmp_adapter *pAd);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
extern INT Set_ApCli_Enable_Proc(
    IN  Pstruct rtmp_adapter pAd,
    IN  char *arg);

extern INT Set_ApCli_Bssid_Proc(
    IN  Pstruct rtmp_adapter pAd,
    IN  char *arg);

bool ApCliAutoConnectExec(
	IN  Pstruct rtmp_adapter   pAd);

bool ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT					pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE	AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE	AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE						WPA);

VOID ApCliSwitchCandidateAP(
	IN Pstruct rtmp_adapter pAd);

VOID RTMPApCliReconnectionCheck(
	IN Pstruct rtmp_adapter pAd);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#endif /* APCLI_SUPPORT */

#endif /* _AP_APCLI_H_ */

