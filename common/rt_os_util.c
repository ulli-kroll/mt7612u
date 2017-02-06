/****************************************************************************

    Module Name:
    rt_os_util.c

    Abstract:
	All functions provided from UTIL module are put here (OS independent).

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

/*#include "rt_config.h"
*/
#include "rtmp_comm.h"
#include "rt_os_util.h"



VOID RtmpDrvRateGet(
	IN	VOID					*pReserved,
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN      UINT8                                   Antenna,
	OUT	uint32_t 				*pRate)
{
	uint32_t MCS_1NSS = (uint32_t) MCS;
	*pRate = 0;

	DBGPRINT(RT_DEBUG_TRACE,("<==== %s \nMODE: %x shortGI: %x BW: %x MCS: %x Antenna: %x \n"
		,__FUNCTION__,MODE,ShortGI,BW,MCS,Antenna));
	if((BW >= Rate_BW_MAX) || (ShortGI >= Rate_GI_MAX) || (BW >= Rate_BW_MAX))
	{
		DBGPRINT(RT_DEBUG_ERROR,("<==== %s MODE: %x shortGI: %x BW: %x MCS: %x Antenna: %x , param error\n",__FUNCTION__,MODE,ShortGI,BW,MCS,Antenna));
		return;
	}

#ifdef DOT11_VHT_AC
    if (MODE >= MODE_VHT)
    {
		if(MCS_1NSS > 9)
		{
			Antenna = (MCS / 10)+1;
			MCS_1NSS %= 10;
		}
        *pRate = RalinkRate_VHT_1NSS[BW][ShortGI][MCS_1NSS];
    }
    else
#endif /* DOT11_VHT_AC */

	if ((MODE >= MODE_HTMIX) && (MODE < MODE_VHT))
	{
		if(MCS_1NSS > 7)
		{
			Antenna = (MCS / 8)+1;
			MCS_1NSS %= 8;
		}
		*pRate = RalinkRate_HT_1NSS[BW][ShortGI][MCS_1NSS];
	}
	else
	if (MODE == MODE_OFDM)
		*pRate = RalinkRate_Legacy[MCS_1NSS+4];
	else
		*pRate = RalinkRate_Legacy[MCS_1NSS];



	*pRate *= 500000;
#if defined(DOT11_VHT_AC)
    if (MODE >= MODE_HTMIX)
		*pRate *= Antenna;
#endif /* DOT11_VHT_AC */

	DBGPRINT(RT_DEBUG_TRACE,("=====> %s \nMODE: %x shortGI: %x BW: %x MCS: %x Antenna: %x  Rate = %d\n"
		,__FUNCTION__,MODE,ShortGI,BW,MCS_1NSS,Antenna, (*pRate)/1000000));


}


char *rtstrchr(const char * s, int c)
{
    for(; *s != (char) c; ++s)
        if (*s == '\0')
            return NULL;
    return (char *) s;
}


VOID RtmpMeshDown(
	IN VOID *pDrvCtrlBK,
	IN BOOLEAN WaitFlag,
	IN BOOLEAN (*RtmpMeshLinkCheck)(IN VOID *pAd))
{
}




BOOLEAN RtmpOsCmdDisplayLenCheck(
	IN uint32_t LenSrc,
	IN uint32_t Offset)
{
	if (LenSrc > (IW_PRIV_SIZE_MASK - Offset))
		return FALSE;

	return TRUE;
}


#ifdef WPA_SUPPLICANT_SUPPORT
VOID WpaSendMicFailureToWpaSupplicant(
	IN struct net_device *pNetDev,
	IN const u8 *src_addr,
	IN BOOLEAN bUnicast,
	IN INT key_id,
	IN const u8 *tsc)
{
#ifdef RT_CFG80211_SUPPORT
	CFG80211OS_MICFailReport(pNetDev, src_addr, bUnicast, key_id, tsc);
#else
	char custom[IW_CUSTOM_MAX] = {0};

	snprintf(custom, sizeof(custom), "MLME-MICHAELMICFAILURE.indication");
	if(bUnicast)
		sprintf(custom, "%s unicast", custom);

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, -1, NULL, (u8 *)custom, strlen(custom));
#endif /* RT_CFG80211_SUPPORT */

	return;
}
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
int wext_notify_event_assoc(
	IN struct net_device *pNetDev,
	IN UCHAR *ReqVarIEs,
	IN uint32_t ReqVarIELen)
{
	char custom[IW_CUSTOM_MAX] = {0};

#if WIRELESS_EXT > 17
	if (ReqVarIELen <= IW_CUSTOM_MAX)
	{
		memmove(custom, ReqVarIEs, ReqVarIELen);
		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_ASSOC_REQ_IE, -1, NULL,
								(UCHAR *)custom, ReqVarIELen);
	}
	else
	    DBGPRINT(RT_DEBUG_TRACE, ("pAd->StaCfg.ReqVarIELen > MAX_CUSTOM_LEN\n"));
#else
	int len;

	len = (ReqVarIELen*2) + 17;
	if (len <= IW_CUSTOM_MAX)
	{
		UCHAR   idx;
		snprintf(custom, sizeof(custom), "ASSOCINFO(ReqIEs=");
		for (idx=0; idx<ReqVarIELen; idx++)
		        sprintf(custom, "%s%02x", custom, ReqVarIEs[idx]);
		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, -1, NULL, custom, len);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("len(%d) > MAX_CUSTOM_LEN\n", len));
#endif

	return 0;

}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
VOID SendAssocIEsToWpaSupplicant(
	IN struct net_device *pNetDev,
	IN UCHAR *ReqVarIEs,
	IN uint32_t ReqVarIELen)
{
	STRING custom[IW_CUSTOM_MAX] = {0};

	if ((ReqVarIELen + 17) <= IW_CUSTOM_MAX)
	{
		snprintf(custom, sizeof(custom), "ASSOCINFO_ReqIEs=");
		memmove(custom+17, ReqVarIEs, ReqVarIELen);
		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, RT_REQIE_EVENT_FLAG, NULL, (u8 *)custom, ReqVarIELen + 17);

		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, RT_ASSOCINFO_EVENT_FLAG, NULL, NULL, 0);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("pAd->StaCfg.ReqVarIELen + 17 > MAX_CUSTOM_LEN\n"));

	return;
}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/

int32_t  RtPrivIoctlSetVal(VOID)
{
    return (int32_t)RTPRIV_IOCTL_SET;
}


#ifndef RTMP_ANDES_JAY
#ifdef RTMP_USB_SUPPORT
PVOID RtmpAllocCompletion(VOID)
{
	struct completion *comp = NULL;
	comp = kmalloc(sizeof(struct completion), GFP_ATOMIC);
	if (comp)
		init_completion(comp);

	return comp;
}

VOID RtmpInitCompletion(VOID *comp)
{
	struct completion *complete = (struct completion *)comp;
	init_completion(complete);
}


ULONG RtmpWaitForCompletionTimeout(VOID *Completion, ULONG Expire)
{
	return wait_for_completion_timeout((struct completion *)Completion, Expire);
}


VOID RtmpComplete(VOID *Completion)
{
	complete((struct completion *)Completion);
}


ULONG RtmpMsecsToJiffies(uint32_t msecs)
{
	return msecs_to_jiffies(msecs);
}

#endif /* RTMP_USB_SUPPORT */
#endif /* RTMP_ANDES_JAY */

