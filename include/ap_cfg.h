#ifndef __AP_CFG_H__
#define __AP_CFG_H__


#include "rt_config.h"

INT RTMPAPPrivIoctlSet(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr);

INT RTMPAPPrivIoctlShow(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr);

VOID RTMPAPGetAssoMacTable(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr);

INT RTMPAPSetInformation(
	IN	struct rtmp_adapter *pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT	*rq,
	IN	INT				cmd);

INT RTMPAPQueryInformation(
	IN	struct rtmp_adapter *      pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT    *rq,
	IN	INT                 cmd);

VOID RTMPIoctlStatistics(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq);

VOID RTMPIoctlGetMacTableStaInfo(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq);

VOID RTMPIoctlGetMacTable(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq);

#if defined(DBG)
VOID RTMPAPIoctlBBP(
    IN  struct rtmp_adapter *  pAdapter,
    IN  RTMP_IOCTL_INPUT_STRUCT    *wrq);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPAPIoctlRF(
	IN	struct rtmp_adapter *pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);
#endif /* RTMP_RF_RW_SUPPORT */

#endif /* DBG */

VOID RtmpDrvRateGet(
	IN	VOID					*pReserved,
/*	IN	PHTTRANSMIT_SETTING		pHtPhyMode, */
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN  UINT8                   Antena,
	OUT	uint32_t 				*pRate);


VOID RTMPIoctlQueryBaTable(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);

#ifdef DOT1X_SUPPORT
VOID RTMPIoctlStaticWepCopy(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);

VOID RTMPIoctlRadiusData(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT		*wrq);

VOID RTMPIoctlAddWPAKey(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);

VOID RTMPIoctlAddPMKIDCache(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);

VOID RTMPIoctlSetIdleTimeout(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);

VOID RTMPIoctlQueryStaAid(
        IN      struct rtmp_adapter *  pAd,
        IN      RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* DOT1X_SUPPORT */

INT Set_AP_Daemon_Status(
	IN struct rtmp_adapter *pAd,
	IN UINT8 WorkSpaceID,
	IN BOOLEAN Status);

INT Set_AP_IE(
	IN struct rtmp_adapter *pAd,
	IN char *IE,
	IN uint32_t IELen);


INT	ApCfg_Set_AuthMode_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	INT				apidx,
	IN	char *		arg);

INT	ApCfg_Set_MaxStaNum_Proc(
	IN struct rtmp_adapter *	pAd,
	IN INT				apidx,
	IN char *			arg);

INT	ApCfg_Set_IdleTimeout_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);


#ifdef APCLI_SUPPORT
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
VOID RTMPApCliAddKey(
	IN	struct rtmp_adapter *    pAd,
	IN 	INT				apidx,
	IN	PNDIS_APCLI_802_11_KEY    pApcliKey);
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* __AP_CFG_H__ */

