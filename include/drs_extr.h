/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
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

	All Dynamic Rate Switch Related Structure & Definition

***************************************************************************/

#ifndef __DRS_EXTR_H__
#define __DRS_EXTR_H__

struct rtmp_adapter;
struct _MAC_TABLE_ENTRY;


typedef struct _RTMP_TX_RATE {
	u8 mode;
	u8 bw;
	u8 mcs;
	u8 nss;
	u8 sgi;
	u8 stbc;
}RTMP_TX_RATE;


typedef struct _RTMP_RA_LEGACY_TB
{
	u8   ItemNo;
#ifdef RT_BIG_ENDIAN
	u8 Rsv2:1;
	u8 Mode:3;
	u8 BW:2;
	u8 ShortGI:1;
	u8 STBC:1;
#else
	u8 STBC:1;
	u8 ShortGI:1;
	u8 BW:2;
	u8 Mode:3;
	u8 Rsv2:1;
#endif
	u8   CurrMCS;
	u8   TrainUp;
	u8   TrainDown;
} RTMP_RA_LEGACY_TB;

#define PTX_RA_LEGACY_ENTRY(pTable, idx)	((RTMP_RA_LEGACY_TB *)&(pTable[(idx+1)*5]))


#ifdef NEW_RATE_ADAPT_SUPPORT
typedef struct  _RTMP_RA_GRP_TB
{
	u8   ItemNo;
#ifdef RT_BIG_ENDIAN
	u8 Rsv2:1;
	u8 Mode:3;
	u8 BW:2;
	u8 ShortGI:1;
	u8 STBC:1;
#else
	u8 STBC:1;
	u8 ShortGI:1;
	u8 BW:2;
	u8 Mode:3;
	u8 Rsv2:1;
#endif
	u8   CurrMCS;
	u8   TrainUp;
	u8   TrainDown;
	u8 downMcs;
	u8 upMcs3;
	u8 upMcs2;
	u8 upMcs1;
	u8 dataRate;
} RTMP_RA_GRP_TB;

#define PTX_RA_GRP_ENTRY(pTable, idx)	((RTMP_RA_GRP_TB *)&(pTable[(idx+1)*10]))
#endif /* NEW_RATE_ADAPT_SUPPORT */

#define RATE_TABLE_SIZE(pTable)			((pTable)[0])		/* Byte 0 is number of rate indices */
#define RATE_TABLE_INIT_INDEX(pTable)	((pTable)[1])		/* Byte 1 is initial rate index */


/* Values of LastSecTxRateChangeAction */
#define RATE_NO_CHANGE	0		/* No change in rate */
#define RATE_UP			1		/* Trying higher rate or same rate with different BF */
#define RATE_DOWN		2		/* Trying lower rate */

enum RATE_ADAPT_ALG{
	RATE_ALG_GRP = 2,
	RATE_ALG_MAX_NUM
};


typedef enum {
	RAL_OLD_DRS,
	RAL_NEW_DRS,
	RAL_QUICK_DRS
}RA_LOG_TYPE;


extern u8 RateSwitchTable11B[];
extern u8 RateSwitchTable11G[];
extern u8 RateSwitchTable11BG[];

extern u8 RateSwitchTable11BGN1S[];
extern u8 RateSwitchTable11BGN2S[];
extern u8 RateSwitchTable11BGN2SForABand[];
extern u8 RateSwitchTable11N1S[];
extern u8 RateSwitchTable11N1SForABand[];
extern u8 RateSwitchTable11N2S[];
extern u8 RateSwitchTable11N2SForABand[];
extern u8 RateSwitchTable11BGN3S[];
extern u8 RateSwitchTable11BGN3SForABand[];

#ifdef NEW_RATE_ADAPT_SUPPORT
extern u8 RateSwitchTableAdapt11N1S[];
extern u8 RateSwitchTableAdapt11N2S[];
extern u8 RateSwitchTableAdapt11N3S[];

#define PER_THRD_ADJ			1

/* ADAPT_RATE_TABLE - true if pTable is one of the Adaptive Rate Switch tables */
extern u8 RateTableVht1S[];
extern u8 RateTableVht1S_MCS9[];
extern u8 RateTableVht2S[];
extern u8 RateTableVht2S_MCS7[];
extern u8 RateTableVht2S_BW20[];
extern u8 RateTableVht2S_BW40[];
extern u8 RateTableVht1S_2G_BW20[];
extern u8 RateTableVht1S_2G_BW40[];
extern u8 RateTableVht2S_2G_BW20[];
extern u8 RateTableVht2S_2G_BW40[];

#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11N1S ||\
									(pTable)==RateSwitchTableAdapt11N2S ||\
									(pTable)==RateSwitchTableAdapt11N3S ||\
									(pTable)==RateTableVht1S ||\
									(pTable)==RateTableVht1S_MCS9 ||\
									(pTable)==RateTableVht2S || \
									(pTable)==RateTableVht2S_MCS7 || \
									(pTable)==RateTableVht2S_BW20 ||\
									(pTable)==RateTableVht2S_BW40 ||\
									(pTable)==RateTableVht1S_2G_BW20 ||\
									(pTable)==RateTableVht1S_2G_BW40 ||\
									(pTable)==RateTableVht2S_2G_BW20 ||\
									(pTable)==RateTableVht2S_2G_BW40)
#else
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11N1S || \
									(pTable)==RateSwitchTableAdapt11N2S || \
									(pTable)==RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */


/* FUNCTION */
VOID MlmeGetSupportedMcs(
	IN struct rtmp_adapter *pAd,
	IN u8 *pTable,
	OUT CHAR mcs[]);

u8 MlmeSelectTxRate(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN CHAR mcs[],
	IN CHAR Rssi,
	IN CHAR RssiOffset);

VOID MlmeClearTxQuality(struct _MAC_TABLE_ENTRY *pEntry);
VOID MlmeClearAllTxQuality(struct _MAC_TABLE_ENTRY *pEntry);
VOID MlmeDecTxQuality(struct _MAC_TABLE_ENTRY *pEntry, u8 rateIndex);
USHORT MlmeGetTxQuality(struct _MAC_TABLE_ENTRY *pEntry, u8 rateIndex);
VOID MlmeSetTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 rateIndex,
	IN USHORT txQuality);



VOID MlmeOldRateAdapt(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 		CurrRateIdx,
	IN u8 		UpRateIdx,
	IN u8 		DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio);

VOID MlmeRestoreLastRate(
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeCheckRDG(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID RTMPSetSupportMCS(
	IN struct rtmp_adapter *pAd,
	IN u8 OpMode,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 SupRate[],
	IN u8 SupRateLen,
	IN u8 ExtRate[],
	IN u8 ExtRateLen,
	IN u8 vht_cap_len,
	IN VHT_CAP_IE *vht_cap,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN u8 HtCapabilityLen);

#ifdef NEW_RATE_ADAPT_SUPPORT
VOID TriggerQuickInitMCSRate(
    IN struct rtmp_adapter *pAd,
    IN struct _MAC_TABLE_ENTRY	*pEntry,
    IN uint32_t CheckInterval);

bool QuickInitMCSRate(
    IN struct rtmp_adapter *pAd,
    IN struct _MAC_TABLE_ENTRY	*pEntry);

VOID MlmeSetMcsGroup(struct rtmp_adapter *pAd, struct _MAC_TABLE_ENTRY *pEnt);

u8 MlmeSelectUpRate(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_GRP_TB *pCurrTxRate);

u8 MlmeSelectDownRate(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 		CurrRateIdx);

VOID MlmeGetSupportedMcsAdapt(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 mcs23GI,
	OUT CHAR 	mcs[]);

u8 MlmeSelectTxRateAdapt(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset);

bool MlmeRAHybridRule(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_GRP_TB *pCurrTxRate,
	IN ULONG			NewTxOkCount,
	IN ULONG			TxErrorRatio);

VOID MlmeNewRateAdapt(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 		UpRateIdx,
	IN u8 		DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio);

#ifdef NEW_RATE_ADAPT_SUPPORT
INT	Set_PerThrdAdj_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

INT	Set_LowTrafficThrd_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT	Set_TrainUpRule_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT	Set_TrainUpRuleRSSI_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT	Set_TrainUpLowThrd_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT	Set_TrainUpHighThrd_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT Set_RateTable_Proc(
	IN  struct rtmp_adapter *pAd,
	IN  char *arg);
#endif /*NEW_RATE_ADAPT_SUPPORT*/

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitchingAdapt(struct rtmp_adapter *pAd, UINT idx);
VOID APQuickResponeForRateUpExecAdapt(struct rtmp_adapter *pAd, UINT idx);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID StaQuickResponeForRateUpExecAdapt(
	IN struct rtmp_adapter *pAd,
	IN ULONG i,
	IN CHAR Rssi);

VOID MlmeDynamicTxRateSwitchingAdapt(
	IN struct rtmp_adapter *pAd,
	IN ULONG i,
	IN ULONG TxSuccess,
	IN ULONG TxRetransmit,
	IN ULONG TxFailCount);
#endif /* CONFIG_STA_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitching(
    IN struct rtmp_adapter *pAd);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID APMlmeSetTxRate(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitching(
	IN struct rtmp_adapter *pAd);

VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeSetTxRate(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate);
#endif /* CONFIG_STA_SUPPORT */

VOID MlmeRAInit(struct rtmp_adapter *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID MlmeNewTxRate(struct rtmp_adapter *pAd, struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeRALog(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RA_LOG_TYPE raLogType,
	IN ULONG TxErrorRatio,
	IN ULONG TxTotalCnt);

VOID MlmeSelectTxRateTable(
	IN struct rtmp_adapter *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN u8 **ppTable,
	IN u8 *pTableSize,
	IN u8 *pInitTxRateIdx);

#endif /* __DRS_EXTR_H__ */

