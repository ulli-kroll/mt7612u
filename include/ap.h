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
    ap.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_H__
#define __AP_H__




/* ============================================================= */
/*      Common definition */
/* ============================================================= */
#define MBSS_VLAN_INFO_GET(												\
	__pAd, __VLAN_VID, __VLAN_Priority, __FromWhichBSSID) 				\
{																		\
	if ((__FromWhichBSSID < __pAd->ApCfg.BssidNum) &&					\
		(__FromWhichBSSID < HW_BEACON_MAX_NUM) &&						\
		(__pAd->ApCfg.MBSSID[__FromWhichBSSID].wdev.VLAN_VID != 0))			\
	{																	\
		__VLAN_VID = __pAd->ApCfg.MBSSID[__FromWhichBSSID].wdev.VLAN_VID;	\
		__VLAN_Priority = __pAd->ApCfg.MBSSID[__FromWhichBSSID].wdev.VLAN_Priority; \
	}																	\
}

/* ============================================================= */
/*      Function Prototypes */
/* ============================================================= */

BOOLEAN APBridgeToWirelessSta(
    IN  struct rtmp_adapter *  pAd,
    IN  u8 *         pHeader,
    IN  UINT            HdrLen,
    IN  u8 *         pData,
    IN  UINT            DataLen,
    IN  ULONG           fromwdsidx);

INT ApAllowToSendPacket(
	IN struct rtmp_adapter *pAd,
	IN struct rtmp_wifi_dev *wdev,
	IN struct sk_buff *pPacket,
	OUT UCHAR *pWcid);

INT APSendPacket(struct rtmp_adapter *pAd, struct sk_buff *pPacket);

int APInsertPsQueue(
	IN struct rtmp_adapter *pAd,
	IN struct sk_buff *pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx);

int APHardTransmit(struct rtmp_adapter *pAd, TX_BLK *pTxBlk, UCHAR QueIdx);

VOID APRxEAPOLFrameIndicate(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR FromWhichBSSID);

VOID APHandleRxDataFrame(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);

VOID APRxErrorHandle(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);

INT APCheckRxError(struct rtmp_adapter *pAd, RXINFO_STRUC *pRxInfo, RX_BLK *pRxBlk);

BOOLEAN APChkCls2Cls3Err(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Wcid,
	IN HEADER_802_11 *pHeader);

VOID RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescriptorType);

VOID RTMPFrameEndianChange(
    IN  struct rtmp_adapter *pAd,
    IN  UCHAR *pData,
    IN  ULONG Dir,
    IN  BOOLEAN FromRxDoneInt);

/* ap_assoc.c */

VOID APAssocStateMachineInit(
    IN  struct rtmp_adapter *  pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MbssKickOutStas(struct rtmp_adapter *pAd, INT apidx, USHORT Reason);
VOID APMlmeKickOutSta(struct rtmp_adapter *pAd, UCHAR *staAddr, UCHAR Wcid, USHORT Reason);

#ifdef DOT11W_PMF_SUPPORT
VOID APMlmeKickOutAllSta(struct rtmp_adapter *pAd, UCHAR apidx, USHORT Reason);
#endif /* DOT11W_PMF_SUPPORT */

VOID  APCls3errAction(struct rtmp_adapter *pAd, ULONG wcid, HEADER_802_11 *hdr);

/* ap_auth.c */

void APAuthStateMachineInit(
    IN struct rtmp_adapter *pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID APCls2errAction(struct rtmp_adapter *pAd, ULONG wcid, HEADER_802_11 *hdr);

/* ap_connect.c */

#ifdef CONFIG_AP_SUPPORT
BOOLEAN BeaconTransmitRequired(struct rtmp_adapter *pAd, INT apidx, MULTISSID_STRUCT *mbss);
#endif /* CONFIG_AP_SUPPORT */

VOID APMakeBssBeacon(struct rtmp_adapter *pAd, INT apidx);
VOID  APUpdateBeaconFrame(struct rtmp_adapter *pAd, INT apidx);
VOID APMakeAllBssBeacon(struct rtmp_adapter *pAd);
VOID  APUpdateAllBeaconFrame(struct rtmp_adapter *pAd);

/* ap_sync.c */
VOID APSyncStateMachineInit(
    IN struct rtmp_adapter *pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID APScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ApSiteSurvey(
	IN	struct rtmp_adapter * 		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel);

VOID SupportRate(
	IN u8 *SupRate,
	IN UCHAR SupRateLen,
	IN u8 *ExtRate,
	IN UCHAR ExtRateLen,
	OUT u8 **Rates,
	OUT u8 *RatesLen,
	OUT u8 *pMaxSupportRate);


BOOLEAN ApScanRunning(struct rtmp_adapter *pAd);

#ifdef AP_PARTIAL_SCAN_SUPPORT
UCHAR FindPartialScanChannel(
	IN struct rtmp_adapter *pAd);
#endif /* AP_PARTIAL_SCAN_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID APUpdateOperationMode(struct rtmp_adapter *pAd);

#ifdef DOT11N_DRAFT3
VOID APOverlappingBSSScan(struct rtmp_adapter *pAd);

INT GetBssCoexEffectedChRange(
	IN struct rtmp_adapter *pAd,
	IN BSS_COEX_CH_RANGE *pCoexChRange);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


/* ap_mlme.c */
VOID APMlmePeriodicExec(struct rtmp_adapter *pAd);

BOOLEAN APMsgTypeSubst(
    IN struct rtmp_adapter *pAd,
    IN PFRAME_802_11 pFrame,
    OUT INT *Machine,
    OUT INT *MsgType);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID APAsicEvaluateRxAnt(struct rtmp_adapter *pAd);
VOID APAsicRxAntEvalTimeout(struct rtmp_adapter *pAd);

/* ap.c */
UCHAR get_apidx_by_addr(struct rtmp_adapter *pAd, UCHAR *addr);

int APInitialize(struct rtmp_adapter *pAd);
VOID APShutdown(struct rtmp_adapter *pAd);
VOID APStartUp(struct rtmp_adapter *pAd);
VOID APStop(struct rtmp_adapter *pAd);

VOID APCleanupPsQueue(struct rtmp_adapter *pAd, QUEUE_HEADER *pQueue);


VOID MacTableMaintenance(struct rtmp_adapter *pAd);

uint32_t MacTableAssocStaNumGet(struct rtmp_adapter *pAd);

MAC_TABLE_ENTRY *APSsPsInquiry(
    IN  struct rtmp_adapter *  pAd,
    IN  u8 *         pAddr,
    OUT SST             *Sst,
    OUT USHORT          *Aid,
    OUT UCHAR           *PsMode,
    OUT UCHAR           *Rate);

#ifdef SYSTEM_LOG_SUPPORT
VOID ApLogEvent(
    IN struct rtmp_adapter *   pAd,
    IN u8 *          pAddr,
    IN USHORT           Event);
#else
#define ApLogEvent(_pAd, _pAddr, _Event)
#endif /* SYSTEM_LOG_SUPPORT */

VOID APUpdateCapabilityAndErpIe(struct rtmp_adapter *pAd);

BOOLEAN ApCheckAccessControlList(struct rtmp_adapter *pAd, UCHAR *addr, UCHAR apidx);
VOID ApUpdateAccessControlList(struct rtmp_adapter *pAd, UCHAR apidx);


BOOLEAN PeerAssocReqCmmSanity(
    IN struct rtmp_adapter *pAd,
	IN BOOLEAN isRessoc,
    IN VOID *Msg,
    IN INT MsgLen,
    IN IE_LISTS *ie_lists);


BOOLEAN PeerDisassocReqSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT u8 *pAddr2,
    OUT	uint16_t *SeqNum,
    OUT USHORT *Reason);

BOOLEAN PeerDeauthReqSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT u8 *pAddr2,
   	OUT	uint16_t *SeqNum,
    OUT USHORT *Reason);

BOOLEAN APPeerAuthSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
	OUT u8 *pAddr1,
    OUT u8 *pAddr2,
    OUT USHORT *Alg,
    OUT USHORT *Seq,
    OUT USHORT *Status,
    OUT CHAR *ChlgText
	);

#ifdef DOT1X_SUPPORT
INT	Set_OwnIPAddr_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_EAPIfName_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_PreAuthIfName_Proc(struct rtmp_adapter *pAd, char *arg);

/* Define in ap.c */
BOOLEAN DOT1X_InternalCmdAction(
    IN struct rtmp_adapter *pAd,
    IN MAC_TABLE_ENTRY *pEntry,
    IN UINT8 cmd);

BOOLEAN DOT1X_EapTriggerAction(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* DOT1X_SUPPORT */

VOID AP_E2PROM_IOCTL_PostCtrl(RTMP_IOCTL_INPUT_STRUCT *wrq, char *msg);

VOID IAPP_L2_UpdatePostCtrl(struct rtmp_adapter *pAd, UINT8 *mac, INT bssid);

#endif  /* __AP_H__ */

