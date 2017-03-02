/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All CFG80211 Function Prototype.

***************************************************************************/

#ifndef __CFG80211EXTR_H__
#define __CFG80211EXTR_H__

#ifdef RT_CFG80211_SUPPORT

#define CFG80211CB				    (pAd->pCfg80211_CB)
#define RT_CFG80211_DEBUG 			/* debug use */
#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)		DBGPRINT(__Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif /* RT_CFG80211_DEBUG */

//CFG_TODO
#include "wfa_p2p.h"

#define RT_CFG80211_REGISTER(__pDev, __pNetDev)								\
	CFG80211_Register(__pDev, __pNetDev);

#define RT_CFG80211_BEACON_CR_PARSE(__pAd, __pVIE, __LenVIE)				\
	CFG80211_BeaconCountryRegionParse((VOID *)__pAd, __pVIE, __LenVIE);

#define RT_CFG80211_BEACON_TIM_UPDATE(__pAd)                                \
        CFG80211_UpdateBeacon((VOID *)__pAd, NULL, 0, NULL, 0, FALSE);

#define RT_CFG80211_CRDA_REG_HINT(__pAd, __pCountryIe, __CountryIeLen)		\
	CFG80211_RegHint((VOID *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_HINT11D(__pAd, __pCountryIe, __CountryIeLen)	\
	CFG80211_RegHint11D((VOID *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_RULE_APPLY(__pAd)								\
	CFG80211_RegRuleApply((VOID *)__pAd, NULL, __pAd->cfg80211_ctrl.Cfg80211_Alpha2);

#define RT_CFG80211_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe, 			\
			__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
	CFG80211_ConnectResultInform((VOID *)__pAd, __pBSSID,					\
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_SCANNING_INFORM(__pAd, __BssIdx, __ChanId, __pFrame,	\
			__FrameLen, __RSSI)									\
	CFG80211_Scaning((VOID *)__pAd, __BssIdx, __ChanId, __pFrame,			\
						__FrameLen, __RSSI);

#define RT_CFG80211_SCAN_END(__pAd, __FlgIsAborted)							\
	CFG80211_ScanEnd((VOID *)__pAd, __FlgIsAborted);
#ifdef CONFIG_STA_SUPPORT
#define RT_CFG80211_LOST_AP_INFORM(__pAd) 									\
	CFG80211_LostApInform((VOID *)__pAd);
#endif /*CONFIG_STA_SUPPORT*/
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define RT_CFG80211_LOST_GO_INFORM(__pAd) 									\
	CFG80211_LostP2pGoInform((VOID *)__pAd);
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE*/
#define RT_CFG80211_REINIT(__pAd)											\
	CFG80211_SupBandReInit((VOID *)__pAd);

#define RT_CFG80211_RFKILL_STATUS_UPDATE(_pAd, _active) 					\
	CFG80211_RFKillStatusUpdate(_pAd, _active);

#define RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,   \
			__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
	CFG80211_P2pClientConnectResultInform(__pAd, __pBSSID,				    \
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(_pAd, _PwrMgmt)					\
	CFG80211_P2pClientSendNullFrame(_pAd, _PwrMgmt);


#define CFG80211_BANDINFO_FILL(__pAd, __pBandInfo)							\
{																			\
	(__pBandInfo)->RFICType = __pAd->phy_ctrl.rf_band_cap;								\
	(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
	(__pBandInfo)->TxStream = __pAd->CommonCfg.TxStream;					\
	(__pBandInfo)->RxStream = __pAd->CommonCfg.RxStream;					\
	(__pBandInfo)->MaxTxPwr = 0;											\
	if (WMODE_EQUAL(__pAd->CommonCfg.PhyMode, WMODE_B))				\
		(__pBandInfo)->FlgIsBMode = TRUE;									\
	else																	\
		(__pBandInfo)->FlgIsBMode = FALSE;									\
	(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;						\
	(__pBandInfo)->RtsThreshold = pAd->CommonCfg.RtsThreshold;				\
	(__pBandInfo)->FragmentThreshold = pAd->CommonCfg.FragmentThreshold;	\
	(__pBandInfo)->RetryMaxCnt = 0;											\
	(__pBandInfo)->RetryMaxCnt = mt7612u_read32(__pAd, TX_RTY_CFG);		\
}

/* Scan Releated */
#ifdef CONFIG_STA_SUPPORT
BOOLEAN CFG80211DRV_OpsScanRunning(struct rtmp_adapter *pAdOrg);
#endif /*CONFIG_STA_SUPPORT*/
BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	struct rtmp_adapter *pAdOrg, VOID *pData, UINT8 dataLen);

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	struct rtmp_adapter *pAdOrg, UINT8	IfType);

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(struct rtmp_adapter *pAdOrg);

VOID CFG80211DRV_OpsScanInLinkDownAction(struct rtmp_adapter *pAdOrg);

INT CFG80211DRV_OpsScanGetNextChannel(struct rtmp_adapter *pAdOrg);

VOID CFG80211_ScanStatusLockInit(struct rtmp_adapter *pAdCB, UINT init);

VOID CFG80211_Scaning(
	struct rtmp_adapter *pAdCB, uint32_t BssIdx, uint32_t ChanId, UCHAR *pFrame, uint32_t FrameLen, int32_t RSSI);

VOID CFG80211_ScanEnd(struct rtmp_adapter *pAdCB, BOOLEAN FlgIsAborted);

/* Connect Releated */
BOOLEAN CFG80211DRV_OpsJoinIbss(struct rtmp_adapter *pAdOrg, VOID *pData);
BOOLEAN CFG80211DRV_OpsLeave(struct rtmp_adapter *pAdOrg, UINT8	 IfType);
BOOLEAN CFG80211DRV_Connect(struct rtmp_adapter *pAdOrg, VOID *pData);

VOID CFG80211_P2pClientConnectResultInform(
        IN struct rtmp_adapter                          *pAdCB,
        IN UCHAR                                        *pBSSID,
        IN UCHAR                                        *pReqIe,
        IN uint32_t                                       ReqIeLen,
        IN UCHAR                                        *pRspIe,
        IN uint32_t                                       RspIeLen,
        IN UCHAR                                        FlgIsSuccess);

VOID CFG80211_ConnectResultInform(
	struct rtmp_adapter *pAdCB, UCHAR *pBSSID,	UCHAR *pReqIe, uint32_t ReqIeLen,
	UCHAR *pRspIe, uint32_t RspIeLen,	UCHAR FlgIsSuccess);
VOID CFG80211DRV_PmkidConfig(VOID *pAdOrg, VOID *pData);

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID CFG80211_LostP2pGoInform(struct rtmp_adapter *pAdCB);
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE*/
VOID CFG80211_LostApInform(struct rtmp_adapter *pAdCB);

INT CFG80211_StaPortSecured(
    struct rtmp_adapter          *pAdCB,
    UCHAR                        *pMac,
    UINT    					  flag);

/* AP Related*/
void CFG80211_ApStaDel(struct rtmp_adapter *pAdCB, UCHAR *pMac);

VOID CFG80211_UpdateBeacon(
   struct rtmp_adapter            *pAdOrg,
   UCHAR                          *beacon_head_buf,
   uint32_t                          beacon_head_len,
   UCHAR                          *beacon_tail_buf,
   uint32_t                          beacon_tail_len,
   BOOLEAN                         isAllUpdate);

void CFG80211_ApStaDelSendEvent(struct rtmp_adapter *pAd, const u8 *mac_addr);


/* Information Releated */
BOOLEAN CFG80211DRV_StaGet(struct rtmp_adapter *pAdOrg, void *pData);
VOID CFG80211DRV_SurveyGet(struct rtmp_adapter *pAdOrg, void *pData);
INT CFG80211_reSetToDefault(struct rtmp_adapter *pAdCB);

/* Key Releated */
BOOLEAN CFG80211DRV_StaKeyAdd(struct rtmp_adapter *pAdOrg, void *pData);
BOOLEAN CFG80211DRV_ApKeyAdd(struct rtmp_adapter *pAdOrg, void  *pData);
BOOLEAN CFG80211DRV_ApKeyDel(struct rtmp_adapter *pAdOrg, void *pData);
VOID CFG80211DRV_RtsThresholdAdd(struct rtmp_adapter *pAdOrg, UINT  threshold);
VOID CFG80211DRV_FragThresholdAdd(struct rtmp_adapter *pAdOrg, UINT threshold);
INT CFG80211_setApDefaultKey(struct rtmp_adapter *pAdCB, UINT Data);

#ifdef CONFIG_STA_SUPPORT
INT CFG80211_setStaDefaultKey(struct rtmp_adapter *pAdCB, UINT Data);

#ifdef DOT11W_PMF_SUPPORT
INT CFG80211_setStaMgmtDefaultKey(struct rtmp_adapter *pAdCB, UINT Data);
#endif /* DOT11W_PMF_SUPPORT */

#endif /*CONFIG_STA_SUPPORT*/
/* General Releated */
BOOLEAN CFG80211DRV_OpsSetChannel(struct rtmp_adapter *pAd, VOID *pData);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(struct rtmp_adapter *pAd, VOID *pData);

VOID CFG80211DRV_OpsChangeBssParm(struct rtmp_adapter *pAdOrg, VOID *pData);

VOID CFG80211_UnRegister(struct rtmp_adapter *pAdOrg,	VOID *pNetDev);

INT CFG80211DRV_IoctlHandle(
	struct rtmp_adapter				*pAdSrc,
	RTMP_IOCTL_INPUT_STRUCT		*wrq,
	INT							cmd,
	USHORT						subcmd,
	VOID						*pData,
	ULONG						Data);

UCHAR CFG80211_getCenCh(struct rtmp_adapter *pAd, UCHAR prim_ch);

/* CRDA Releatd */
VOID CFG80211DRV_RegNotify(
	struct rtmp_adapter				*pAdOrg,
	VOID						*pData);

VOID CFG80211_RegHint(
	struct rtmp_adapter				*pAdCB,
	UCHAR						*pCountryIe,
	ULONG						CountryIeLen);

VOID CFG80211_RegHint11D(
	struct rtmp_adapter				*pAdCB,
	UCHAR						*pCountryIe,
	ULONG						CountryIeLen);

VOID CFG80211_RegRuleApply(
	struct rtmp_adapter				*pAdCB,
	VOID						*pWiphy,
	UCHAR						*pAlpha2);

BOOLEAN CFG80211_SupBandReInit(
	struct rtmp_adapter				*pAdCB);

#ifdef RFKILL_HW_SUPPORT
VOID CFG80211_RFKillStatusUpdate(
	struct rtmp_adapter				pAd,
	BOOLEAN						active);
#endif /* RFKILL_HW_SUPPORT */

/* P2P Related */
VOID CFG80211DRV_SetP2pCliAssocIe(
	struct rtmp_adapter				*pAdOrg,
	VOID						*pData,
	UINT                         ie_len);

VOID CFG80211DRV_P2pClientKeyAdd(
	struct rtmp_adapter				*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211DRV_P2pClientConnect(
	struct rtmp_adapter				*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211_checkScanTable(
        IN struct rtmp_adapter                          *pAdCB);

VOID CFG80211_P2pClientSendNullFrame(
	struct rtmp_adapter 				*pAdCB,
	INT							 PwrMgmt);

VOID CFG80211RemainOnChannelTimeout(
	PVOID 						SystemSpecific1,
	PVOID 						FunctionContext,
	PVOID 						SystemSpecific2,
	PVOID 						SystemSpecific3);

BOOLEAN CFG80211DRV_OpsRemainOnChannel(
	struct rtmp_adapter				*pAdOrg,
	VOID						*pData,
	uint32_t 						duration);

void CFG80211DRV_OpsCancelRemainOnChannel(
        struct rtmp_adapter                             *pAdOrg,
        uint32_t                                          cookie);


VOID CFG80211DRV_OpsMgmtFrameProbeRegister(
        struct rtmp_adapter                             *pAdOrg,
        VOID                                            *pData,
		BOOLEAN                                          isReg);

VOID CFG80211DRV_OpsMgmtFrameActionRegister(
        struct rtmp_adapter                             *pAdOrg,
        VOID                                            *pData,
	BOOLEAN                                     isReg);

BOOLEAN CFG80211_CheckActionFrameType(
        IN  struct rtmp_adapter 								 *pAd,
		IN	u8 *									 preStr,
		IN	u8 *									 pData,
		IN	uint32_t                              		 length);


BOOLEAN CFG80211_SyncPacketWmmIe(struct rtmp_adapter *pAd, VOID *pData, ULONG dataLen);
BOOLEAN CFG80211_HandleP2pMgmtFrame(struct rtmp_adapter *pAd, RX_BLK *pRxBlk, UCHAR OpMode);
void CFG80211_SendMgmtFrame(struct rtmp_adapter *pAd, VOID *pData, ULONG Data);


#ifdef CONFIG_AP_SUPPORT

void CFG80211_ParseBeaconIE(struct rtmp_adapter *pAd, MULTISSID_STRUCT *pMbss, struct rtmp_wifi_dev *wdev, const u8 *wpa_ie, const u8 *rsn_ie);

#endif

//--------------------------------
VOID CFG80211_Convert802_3Packet(struct rtmp_adapter *pAd, RX_BLK *pRxBlk, UCHAR *pHeader802_3);
VOID CFG80211_Announce802_3Packet(struct rtmp_adapter *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID);
VOID CFG80211_SendMgmtFrameDone(struct rtmp_adapter *pAd, USHORT Sequence);
VOID CFG80211_SwitchTxChannel(struct rtmp_adapter *pAd, ULONG Data);
void CFG80211DRV_OpsBeaconSet(struct rtmp_adapter *pAd,void  *pData);
BOOLEAN CFG80211DRV_OpsBeaconAdd(struct rtmp_adapter *pAd, void  *pData);
void CFG80211DRV_DisableApInterface(struct rtmp_adapter *pAd);
BOOLEAN CFG80211DRV_OpsVifAdd(struct rtmp_adapter *pA, void  *pData);

#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211EXTR_H__ */
