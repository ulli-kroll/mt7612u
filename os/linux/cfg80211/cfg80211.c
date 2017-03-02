/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2009, Ralink Technology, Inc.
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

	All related CFG80211 function body.

	History:
		1. 2009/09/17	Sample Lin
			(1) Init version.
		2. 2009/10/27	Sample Lin
			(1) Do not use ieee80211_register_hw() to create virtual interface.
				Use wiphy_register() to register nl80211 command handlers.
			(2) Support iw utility.
		3. 2009/11/03	Sample Lin
			(1) Change name MAC80211 to CFG80211.
			(2) Modify CFG80211_OpsChannelSet().
			(3) Move CFG80211_Register()/CFG80211_UnRegister() to open/close.
		4. 2009/12/16	Sample Lin
			(1) Patch for Linux 2.6.32.
			(2) Add more supported functions in CFG80211_Ops.
		5. 2010/12/10	Sample Lin
			(1) Modify for OS_ABL.
		6. 2011/04/19	Sample Lin
			(1) Add more supported functions in CFG80211_Ops v33 ~ 38.

	Note:
		The feature is supported only in "LINUX" 2.6.28 ~ 2.6.38.

***************************************************************************/


#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"


#ifdef RT_CFG80211_SUPPORT

/* 36 ~ 64, 100 ~ 136, 140 ~ 161 */
#define CFG80211_NUM_OF_CHAN_5GHZ	(sizeof(Cfg80211_Chan)-CFG80211_NUM_OF_CHAN_2GHZ)

#ifdef OS_ABL_FUNC_SUPPORT
/*
	Array of bitrates the hardware can operate with
	in this band. Must be sorted to give a valid "supported
	rates" IE, i.e. CCK rates first, then OFDM.

	For HT, assign MCS in another structure, ieee80211_sta_ht_cap.
*/
const struct ieee80211_rate Cfg80211_SupRate[12] = {
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 10,    /* bitrate in units of 100 Kbps */
		.hw_value = 0,
		.hw_value_short = 0,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 20,
		.hw_value = 1,
		.hw_value_short = 1,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 55,
		.hw_value = 2,
		.hw_value_short = 2,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 110,
		.hw_value = 3,
		.hw_value_short = 3,
	},
	{
		.flags = 0,
		.bitrate = 60,
		.hw_value = 4,
		.hw_value_short = 4,
	},
	{
		.flags = 0,
		.bitrate = 90,
		.hw_value = 5,
		.hw_value_short = 5,
	},
	{
		.flags = 0,
		.bitrate = 120,
		.hw_value = 6,
		.hw_value_short = 6,
	},
	{
		.flags = 0,
		.bitrate = 180,
		.hw_value = 7,
		.hw_value_short = 7,
	},
	{
		.flags = 0,
		.bitrate = 240,
		.hw_value = 8,
		.hw_value_short = 8,
	},
	{
		.flags = 0,
		.bitrate = 360,
		.hw_value = 9,
		.hw_value_short = 9,
	},
	{
		.flags = 0,
		.bitrate = 480,
		.hw_value = 10,
		.hw_value_short = 10,
	},
	{
		.flags = 0,
		.bitrate = 540,
		.hw_value = 11,
		.hw_value_short = 11,
	},
};
#endif /* OS_ABL_FUNC_SUPPORT */

/* all available channels */
static const UCHAR Cfg80211_Chan[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

	/* 802.11 UNI / HyperLan 2 */
	36, 38, 40, 44, 46, 48, 52, 54, 56, 60, 62, 64,

	/* 802.11 HyperLan 2 */
	100, 104, 108, 112, 116, 118, 120, 124, 126, 128, 132, 134, 136,

	/* 802.11 UNII */
	140, 149, 151, 153, 157, 159, 161, 165, 167, 169, 171, 173,

	/* Japan */
	184, 188, 192, 196, 208, 212, 216,
};


static const uint32_t CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef DOT11W_PMF_SUPPORT
	WLAN_CIPHER_SUITE_AES_CMAC,
#endif /* DOT11W_PMF_SUPPORT */
};

/*
	The driver's regulatory notification callback.
*/
static int32_t CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN struct regulatory_request	*pRequest);

/* get RALINK pAd control block in 80211 Ops */
struct rtmp_adapter *MAC80211_PAD_GET(struct wiphy *pWiphy)
{
	struct rtmp_adapter *pAd = wiphy_priv(pWiphy);

	if (pAd == NULL)
		DBGPRINT(RT_DEBUG_ERROR,								\
			("80211> %s but pAd = NULL!", __FUNCTION__));	\

	return pAd;
}							\

/*
========================================================================
Routine Description:
	Set channel.

Arguments:
	pWiphy			- Wireless hardware description
	pChan			- Channel information
	ChannelType		- Channel type

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: set channel, set freq

	enum nl80211_channel_type {
		NL80211_CHAN_NO_HT,
		NL80211_CHAN_HT20,
		NL80211_CHAN_HT40MINUS,
		NL80211_CHAN_HT40PLUS
	};
========================================================================
*/
static int CFG80211_OpsChannelSet(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pDev,
	IN struct ieee80211_channel	*pChan,
	IN enum nl80211_channel_type	ChannelType)

{
	struct rtmp_adapter *pAd;
	CFG80211_CB *p80211CB;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	uint32_t ChanId;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> Channel = %d, Type = %d\n", ChanId, ChannelType));

	/* init */
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;

	p80211CB = NULL;
	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

	if (p80211CB == NULL) {
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> p80211CB == NULL!\n"));
		return 0;
	}

	ChanInfo.IfType = pDev->ieee80211_ptr->iftype;

	if (ChannelType == NL80211_CHAN_NO_HT)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
	else if (ChannelType == NL80211_CHAN_HT20)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
	else if (ChannelType == NL80211_CHAN_HT40MINUS)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;
	else if (ChannelType == NL80211_CHAN_HT40PLUS)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;

	ChanInfo.MonFilterFlag = p80211CB->MonFilterFlag;

	/* set channel */
	RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);

	return 0;
}


/*
========================================================================
Routine Description:
	Change type/configuration of virtual interface.

Arguments:
	pWiphy			- Wireless hardware description
	IfIndex			- Interface index
	Type			- Interface type, managed/adhoc/ap/station, etc.
	pFlags			- Monitor flags
	pParams			- Mesh parameters

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: set type, set monitor
========================================================================
*/
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNetDevIn,
	IN enum nl80211_iftype	Type,
	IN u32			*pFlags,
	struct vif_params	*pParams)
{
	struct rtmp_adapter *pAd;
	CFG80211_CB *pCfg80211_CB;
	struct net_device *pNetDev;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM VifInfo;
	UINT oldType = pNetDevIn->ieee80211_ptr->iftype;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> IfTypeChange %d ==> %d\n", oldType, Type));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	/* sanity check */
#ifdef CONFIG_STA_SUPPORT
	if ((Type != NL80211_IFTYPE_ADHOC) &&
		(Type != NL80211_IFTYPE_STATION) &&
		(Type != NL80211_IFTYPE_MONITOR) &&
		(Type != NL80211_IFTYPE_AP)
	    && (Type != NL80211_IFTYPE_P2P_CLIENT)
	    && (Type != NL80211_IFTYPE_P2P_GO)
	)
#endif /* CONFIG_STA_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Wrong interface type %d!\n", Type));
		return -EINVAL;
	} /* End of if */

	/* update interface type */
	pNetDev = pNetDevIn;

	if (pNetDev == NULL)
		return -ENODEV;

	pNetDev->ieee80211_ptr->iftype = Type;

	VifInfo.net_dev = pNetDev;
	VifInfo.newIfType = Type;
	VifInfo.oldIfType = oldType;

	if (pFlags != NULL) {
		VifInfo.MonFilterFlag = 0;

		if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_FCSFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_FCSFAIL;

		if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_PLCPFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_PLCPFAIL;

		if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_CONTROL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_CONTROL;

		if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_OTHER_BSS)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_OTHER_BSS;
	}

	/* Type transer from linux to driver defined */
	if (Type == NL80211_IFTYPE_STATION) {
		Type = RT_CMD_80211_IFTYPE_STATION;
	} else if (Type == NL80211_IFTYPE_ADHOC) {
		Type = RT_CMD_80211_IFTYPE_ADHOC;
	} else if (Type == NL80211_IFTYPE_MONITOR) {
		Type = RT_CMD_80211_IFTYPE_MONITOR;
	}
#ifdef CONFIG_AP_SUPPORT
	else if (Type == NL80211_IFTYPE_P2P_CLIENT) {
		Type = RT_CMD_80211_IFTYPE_P2P_CLIENT;
	} else if (Type == NL80211_IFTYPE_P2P_GO) {
		Type = RT_CMD_80211_IFTYPE_P2P_GO;
	}
#endif /* CONFIG_AP_SUPPORT */

	RTMP_DRIVER_80211_VIF_CHG(pAd, &VifInfo);

	/*CFG_TODO*/
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pCfg80211_CB->MonFilterFlag = VifInfo.MonFilterFlag;
	return 0;
}

#if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT)
extern int rt_ioctl_siwscan(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wreq, char *extra);
/*
========================================================================
Routine Description:
	Request to do a scan. If returning zero, the scan request is given
	the driver, and will be valid until passed to cfg80211_scan_done().
	For scan results, call cfg80211_inform_bss(); you can call this outside
	the scan/scan_done bracket too.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pRequest		- Scan request

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: scan

	struct cfg80211_scan_request {
		struct cfg80211_ssid *ssids;
		int n_ssids;
		struct ieee80211_channel **channels;
		u32 n_channels;
		const u8 *ie;
		size_t ie_len;

	 * @ssids: SSIDs to scan for (active scan only)
	 * @n_ssids: number of SSIDs
	 * @channels: channels to scan on.
	 * @n_channels: number of channels for each band
	 * @ie: optional information element(s) to add into Probe Request or %NULL
	 * @ie_len: length of ie in octets
========================================================================
*/
static int CFG80211_OpsScan(
	IN struct wiphy		*pWiphy,
	IN struct cfg80211_scan_request *pRequest)
{
#ifdef CONFIG_STA_SUPPORT
	struct rtmp_adapter *pAd;
	CFG80211_CB *pCfg80211_CB;
	struct net_device *pNdev = NULL;

	struct iw_scan_req IwReq;
	union iwreq_data Wreq;

	CFG80211DBG(RT_DEBUG_TRACE, ("========================================================================\n"));
#if 0  /* ULLI : disabled */
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==> %s(%d)\n", __FUNCTION__, pNdev->name, pNdev->ieee80211_ptr->iftype));
#endif
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_NET_DEV_GET(pAd, &pNdev);

	/* YF_TODO: record the scan_req per netdevice */
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */

	if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
			CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
			return 0;
	}
	/* sanity check */
	if ((pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_STATION) &&
	    (pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_AP) &&
	    (pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_ADHOC) &&
	    (pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_P2P_CLIENT)) {
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> DeviceType Not Support Scan ==> %d\n", pNdev->ieee80211_ptr->iftype));
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return -EOPNOTSUPP;
	}

	/* Driver Internal SCAN SM Check */
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Network is down!\n"));
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return -ENETDOWN;
	}

	if (RTMP_DRIVER_80211_SCAN(pAd, pNdev->ieee80211_ptr->iftype) != NDIS_STATUS_SUCCESS) {
		CFG80211DBG(RT_DEBUG_ERROR, ("\n\n\n\n\n80211> BUSY - SCANING \n\n\n\n\n"));
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return 0;
	}


	if (pRequest->ie_len != 0) {
		DBGPRINT(RT_DEBUG_TRACE, ("80211> ExtraIEs Not Null in ProbeRequest from upper layer...\n"));
		/* YF@20120321: Using Cfg80211_CB carry on pAd struct to overwirte the pWpsProbeReqIe. */
		RTMP_DRIVER_80211_SCAN_EXTRA_IE_SET(pAd);
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("80211> ExtraIEs Null in ProbeRequest from upper layer...\n"));
	}

	memset(&Wreq, 0, sizeof(Wreq));
	memset(&IwReq, 0, sizeof(IwReq));

	DBGPRINT(RT_DEBUG_INFO, ("80211> Num %d of SSID & ssidLen %d from upper layer...\n",
		pRequest->n_ssids, pRequest->ssids->ssid_len));

	/* %NULL or zero-length SSID is used to indicate wildcard */
	if ((pRequest->n_ssids == 1) && (pRequest->ssids->ssid_len ==0)) {
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Wildcard SSID In ProbeRequest.\n"));
		Wreq.data.flags |= IW_SCAN_ALL_ESSID;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Named SSID [%s] In ProbeRequest.\n",  pRequest->ssids->ssid));
		Wreq.data.flags |= IW_SCAN_THIS_ESSID;

		/* Fix kernel crash when ssid is null instead of wildcard ssid */
		if (pRequest->ssids->ssid == NULL)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("80211> Scanning failed!!!\n"));
			CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
			return -1;
		}
	}

	/* Set Channel List for this Scan Action */
	DBGPRINT(RT_DEBUG_INFO, ("80211> [%d] Channels In ProbeRequest.\n",  pRequest->n_channels));
	if (pRequest->n_channels > 0) {
		uint32_t *pChanList;
		UINT idx;
		pChanList = kmalloc(sizeof(uint32_t *) * pRequest->n_channels, GFP_ATOMIC);

		if (pChanList == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Alloc memory fail\n", __FUNCTION__));
			return FALSE;
		}

		for (idx = 0; idx < pRequest->n_channels; idx++) {
			pChanList[idx] = ieee80211_frequency_to_channel(pRequest->channels[idx]->center_freq);
			CFG80211DBG(RT_DEBUG_INFO, ("%d,", pChanList[idx]));
		}
		CFG80211DBG(RT_DEBUG_INFO, ("\n"));

		RTMP_DRIVER_80211_SCAN_CHANNEL_LIST_SET(pAd, pChanList, pRequest->n_channels);

		if (pChanList)
			kfree(pChanList);
	}

	/* use 1st SSID in the requested SSID list */
	IwReq.essid_len = pRequest->ssids->ssid_len;
	memcpy(IwReq.essid, pRequest->ssids->ssid, sizeof(IwReq.essid));
	Wreq.data.length = sizeof(struct iw_scan_req);

	rt_ioctl_siwscan(pNdev, NULL, &Wreq, (char *)&IwReq);
	return 0;

#else
	return -EOPNOTSUPP;
#endif /* CONFIG_STA_SUPPORT */
}


#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
	Join the specified IBSS (or create if necessary). Once done, call
	cfg80211_ibss_joined(), also call that function when changing BSSID due
	to a merge.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pParams			- IBSS parameters

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: ibss join

	No fixed-freq and fixed-bssid support.
========================================================================
*/
static int CFG80211_OpsIbssJoin(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNdev,
	IN struct cfg80211_ibss_params	*pParams)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_IBSS IbssInfo;


	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> SSID = %s, BI = %d\n",
				pParams->ssid, pParams->beacon_interval));
	/* init */
	memset(&IbssInfo, 0, sizeof(IbssInfo));
	IbssInfo.BeaconInterval = pParams->beacon_interval;
	IbssInfo.pSsid = (char *) pParams->ssid;

	/* ibss join */
	RTMP_DRIVER_80211_IBSS_JOIN(pAd, &IbssInfo);

	return 0;
}


/*
========================================================================
Routine Description:
	Leave the IBSS.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: ibss leave
========================================================================
*/
static int CFG80211_OpsIbssLeave(struct wiphy *pWiphy,
	struct net_device *pNdev)
{
	struct rtmp_adapter *pAd;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	//CFG_TODO
	RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev->ieee80211_ptr->iftype);
	return 0;
}
#endif /* CONFIG_STA_SUPPORT */


/*
========================================================================
Routine Description:
	Set the transmit power according to the parameters.

Arguments:
	pWiphy			- Wireless hardware description
	Type			-
	dBm				- dBm

Return Value:
	0				- success
	-x				- fail

Note:
	Type -
	enum nl80211_tx_power_setting - TX power adjustment
	 @NL80211_TX_POWER_AUTOMATIC: automatically determine transmit power
	 @NL80211_TX_POWER_LIMITED: limit TX power by the mBm parameter
	 @NL80211_TX_POWER_FIXED: fix TX power to the mBm parameter
========================================================================
*/
static int CFG80211_OpsTxPwrSet(struct wiphy *pWiphy, struct wireless_dev *wdev,
	enum nl80211_tx_power_setting Type, int dBm)
{

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	return -EOPNOTSUPP;
}


/*
========================================================================
Routine Description:
	Store the current TX power into the dbm variable.

Arguments:
	pWiphy			- Wireless hardware description
	pdBm			- dBm

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsTxPwrGet(struct wiphy *pWiphy, struct wireless_dev *wdev,
	int *pdBm)
{
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsTxPwrGet */


/*
========================================================================
Routine Description:
	Power management.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	FlgIsEnabled	-
	Timeout			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsPwrMgmt(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNdev,
	IN bool                 enabled,
	IN int32_t              timeout)
{
	struct rtmp_adapter *pAd;
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==> power save %s\n", __FUNCTION__,(enabled ? "enable" : "disable")));

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_80211_POWER_MGMT_SET(pAd,enabled);
	return 0;
} /* End of CFG80211_OpsPwrMgmt */


/*
========================================================================
Routine Description:
	Get information for a specific station.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	pMac			- STA MAC
	pSinfo			- STA INFO

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsStaGet(struct wiphy *pWiphy,
	struct net_device *pNdev, const u8 *pMac,
	struct station_info *pSinfo)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_STA StaInfo;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	/* init */
	memset(pSinfo, 0, sizeof(*pSinfo));
	memset(&StaInfo, 0, sizeof(StaInfo));

	memcpy(StaInfo.MAC, pMac, 6);

	/* get sta information */
	if (RTMP_DRIVER_80211_STA_GET(pAd, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -ENOENT;

	if (StaInfo.TxRateFlags != RT_CMD_80211_TXRATE_LEGACY) {
		pSinfo->txrate.flags = RATE_INFO_FLAGS_MCS;
		if (StaInfo.TxRateFlags & RT_CMD_80211_TXRATE_BW_40)
			pSinfo->txrate.bw = RATE_INFO_BW_40;

		if (StaInfo.TxRateFlags & RT_CMD_80211_TXRATE_SHORT_GI)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;


		pSinfo->txrate.mcs = StaInfo.TxRateMCS;
	} else {
		pSinfo->txrate.legacy = StaInfo.TxRateMCS;
	}

	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);

	/* fill signal */
	pSinfo->signal = StaInfo.Signal;
	pSinfo->filled |= BIT(NL80211_STA_INFO_SIGNAL);

#ifdef CONFIG_AP_SUPPORT
	/* fill tx count */
	pSinfo->tx_packets = StaInfo.TxPacketCnt;
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);

	/* fill inactive time */
	pSinfo->inactive_time = StaInfo.InactiveTime;
	pSinfo->filled |= BIT(NL80211_STA_INFO_INACTIVE_TIME);
#endif /* CONFIG_AP_SUPPORT */

	return 0;
}


/*
========================================================================
Routine Description:
	List all stations known, e.g. the AP on managed interfaces.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	Idx				-
	pMac			-
	pSinfo			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsStaDump(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNdev,
	IN int			Idx,
	IN UINT8		*pMac,
	IN struct station_info	*pSinfo)
{
	struct rtmp_adapter *pAd;

	if (Idx != 0)
		return -ENOENT;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

#ifdef CONFIG_STA_SUPPORT
	if (RTMP_DRIVER_AP_SSID_GET(pAd, pMac) != NDIS_STATUS_SUCCESS)
		return -EBUSY;
	else
		return CFG80211_OpsStaGet(pWiphy, pNdev, pMac, pSinfo);
#endif /* CONFIG_STA_SUPPORT */

	return -EOPNOTSUPP;
} /* End of CFG80211_OpsStaDump */


/*
========================================================================
Routine Description:
	Notify that wiphy parameters have changed.

Arguments:
	pWiphy			- Wireless hardware description
	Changed			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsWiphyParamsSet(
	IN struct wiphy		*pWiphy,
	IN uint32_t 		Changed)
{
	struct rtmp_adapter *pAd;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	if (Changed & WIPHY_PARAM_RTS_THRESHOLD) {
		RTMP_DRIVER_80211_RTS_THRESHOLD_ADD(pAd, pWiphy->rts_threshold);
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==> rts_threshold(%d)\n", __FUNCTION__,pWiphy->rts_threshold));
		return 0;
	} else if (Changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		RTMP_DRIVER_80211_FRAG_THRESHOLD_ADD(pAd, pWiphy->frag_threshold);
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==> frag_threshold(%d)\n", __FUNCTION__,pWiphy->frag_threshold));
		return 0;
	}

	return -EOPNOTSUPP;
} /* End of CFG80211_OpsWiphyParamsSet */


/*
========================================================================
Routine Description:
	Add a key with the given parameters.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-
	Pairwise		-
	pMacAddr		-
	pParams			-

Return Value:
	0				- success
	-x				- fail

Note:
	pMacAddr will be NULL when adding a group key.
========================================================================
*/
static int CFG80211_OpsKeyAdd(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNdev,
	IN UINT8		KeyIdx,
	IN bool			Pairwise,
	IN const UINT8		*pMacAddr,
	IN struct key_params	*pParams)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;
	p80211CB = NULL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

#ifdef RT_CFG80211_DEBUG
	hex_dump("KeyBuf=", (UINT8 *)pParams->key, pParams->key_len);
#endif /* RT_CFG80211_DEBUG */

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> KeyIdx = %d, pParams->cipher=%x\n", KeyIdx,pParams->cipher));

	if (pParams->key_len >= sizeof(KeyInfo.KeyBuf))
		return -EINVAL;
	/* End of if */


	/* init */
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	memcpy(KeyInfo.KeyBuf, pParams->key, pParams->key_len);
	KeyInfo.KeyBuf[pParams->key_len] = 0x00;
	KeyInfo.KeyId = KeyIdx;
		KeyInfo.bPairwise = Pairwise;
		KeyInfo.KeyLen = pParams->key_len;


	if ((pParams->cipher == WLAN_CIPHER_SUITE_WEP40)) {
		KeyInfo.KeyType = RT_CMD_80211_KEY_WEP40;
	} else if ((pParams->cipher == WLAN_CIPHER_SUITE_WEP104)) {
		KeyInfo.KeyType = RT_CMD_80211_KEY_WEP104;
	} else if ((pParams->cipher == WLAN_CIPHER_SUITE_TKIP) ||
		   (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)) {
		KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
		if (pParams->cipher == WLAN_CIPHER_SUITE_TKIP)
			KeyInfo.cipher = Ndis802_11TKIPEnable;
		else if (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)
			KeyInfo.cipher = Ndis802_11AESEnable;
	}
#ifdef DOT11W_PMF_SUPPORT
//PMF IGTK
	else if (pParams->cipher == WLAN_CIPHER_SUITE_AES_CMAC) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_CMAC;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
	}
#endif /* DOT11W_PMF_SUPPORT */
	else
		return -ENOTSUPP;

	/* add key */
    RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

#ifdef CONFIG_AP_SUPPORT
	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
	    (pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO)) {
		if (pMacAddr) {
			CFG80211DBG(RT_DEBUG_TRACE, ("80211> KeyAdd STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
						PRINT_MAC(pMacAddr)));
			memcpy(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
		}
		CFG80211DBG(RT_DEBUG_OFF, ("80211> AP Key Add\n"));
		RTMP_DRIVER_80211_AP_KEY_ADD(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_CLIENT) {
			CFG80211DBG(RT_DEBUG_TRACE, ("80211> APCLI Key Add\n"));
			RTMP_DRIVER_80211_P2P_CLIENT_KEY_ADD(pAd, &KeyInfo);
		} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		{
#ifdef CONFIG_STA_SUPPORT
			CFG80211DBG(RT_DEBUG_TRACE, ("80211> STA Key Add\n"));
			RTMP_DRIVER_80211_STA_KEY_ADD(pAd, &KeyInfo);
#endif
		}

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
	if (pMacAddr) {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> P2pSendWirelessEvent(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
						PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_SEND_WIRELESS_EVENT(pAd, pMacAddr);
	}
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */

	return 0;


}


/*
========================================================================
Routine Description:
	Get information about the key with the given parameters.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-
	Pairwise		-
	pMacAddr		-
	pCookie			-
	pCallback		-

Return Value:
	0				- success
	-x				- fail

Note:
	pMacAddr will be NULL when requesting information for a group key.

	All pointers given to the pCallback function need not be valid after
	it returns.

	This function should return an error if it is not possible to
	retrieve the key, -ENOENT if it doesn't exist.
========================================================================
*/
static int CFG80211_OpsKeyGet(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	UINT8			KeyIdx,
	bool			Pairwise,
	const UINT8		*pMacAddr,
	void			*pCookie,
	void			(*pCallback)(void *cookie, struct key_params *))
{

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	return -ENOTSUPP;
}


/*
========================================================================
Routine Description:
	Remove a key given the pMacAddr (NULL for a group key) and KeyIdx.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-
	pMacAddr		-

Return Value:
	0				- success
	-x				- fail

Note:
	return -ENOENT if the key doesn't exist.
========================================================================
*/
static int CFG80211_OpsKeyDel(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	UINT8			KeyIdx,
	bool			Pairwise,
	const UINT8		*pMacAddr)
{
    struct rtmp_adapter *pAd;
    CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;
	p80211CB = NULL;

    CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	if (pMacAddr) {
		CFG80211DBG(RT_DEBUG_OFF, ("80211> KeyDel STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
				PRINT_MAC(pMacAddr)));
		memcpy(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
	}

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

	memset(&KeyInfo, 0, sizeof(KeyInfo));
	KeyInfo.KeyId = KeyIdx;
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> KeyDel isPairwise %d\n", Pairwise));
    KeyInfo.bPairwise = Pairwise;

#ifdef CONFIG_AP_SUPPORT
	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
	    (pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO)) {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> AP Key Del\n"));
		RTMP_DRIVER_80211_AP_KEY_DEL(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> STA Key Del\n"));

		if (pMacAddr) {
			CFG80211DBG(RT_DEBUG_OFF, ("80211> STA Key Del -- DISCONNECT\n"));
			RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev->ieee80211_ptr->iftype);
		}
	}

	return 0;
}


/*
========================================================================
Routine Description:
	Set the default key on an interface.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsKeyDefaultSet(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	UINT8			KeyIdx,
	bool			Unicast,
	bool			Multicast)
{
	struct rtmp_adapter *pAd;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> Default KeyIdx = %d\n", KeyIdx));

#ifdef CONFIG_AP_SUPPORT
	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
	    (pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_KEY_DEFAULT_SET(pAd, KeyIdx);
	else
#endif /* CONFIG_AP_SUPPORT */
		RTMP_DRIVER_80211_STA_KEY_DEFAULT_SET(pAd, KeyIdx);

	return 0;
} /* End of CFG80211_OpsKeyDefaultSet */



/*
========================================================================
Routine Description:
	Set the Mgmt default key on an interface.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
#ifdef DOT11W_PMF_SUPPORT
static int CFG80211_OpsMgmtKeyDefaultSet(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	UINT8			KeyIdx)
{
	struct rtmp_adapter *pAd;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> Mgmt Default KeyIdx = %d\n", KeyIdx));

	RTMP_DRIVER_80211_STA_MGMT_KEY_DEFAULT_SET(pAd, KeyIdx);

	return 0;
} /* End of CFG80211_OpsMgmtKeyDefaultSet */
#endif /* DOT11W_PMF_SUPPORT */

/*
========================================================================
Routine Description:
	Connect to the ESS with the specified parameters. When connected,
	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
	If the connection fails for some reason, call cfg80211_connect_result()
	with the status from the AP.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pSme			-

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: connect

	You must use "iw ra0 connect xxx", then "iw ra0 disconnect";
	You can not use "iw ra0 connect xxx" twice without disconnect;
	Or you will suffer "command failed: Operation already in progress (-114)".

	You must support add_key and set_default_key function;
	Or kernel will crash without any error message in linux 2.6.32.


   struct cfg80211_connect_params - Connection parameters

   This structure provides information needed to complete IEEE 802.11
   authentication and association.

   @channel: The channel to use or %NULL if not specified (auto-select based
 	on scan results)
   @bssid: The AP BSSID or %NULL if not specified (auto-select based on scan
 	results)
   @ssid: SSID
   @ssid_len: Length of ssid in octets
   @auth_type: Authentication type (algorithm)

   @ie: IEs for association request
   @ie_len: Length of assoc_ie in octets

   @privacy: indicates whether privacy-enabled APs should be used
   @crypto: crypto settings
   @key_len: length of WEP key for shared key authentication
   @key_idx: index of WEP key for shared key authentication
   @key: WEP key for shared key authentication
========================================================================
*/
static int CFG80211_OpsConnect(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	struct cfg80211_connect_params	*pSme)
{
#ifdef CONFIG_STA_SUPPORT
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_CONNECT ConnInfo;
	struct ieee80211_channel *pChannel = pSme->channel;
	int32_t Pairwise = 0;
	int32_t Groupwise = 0;
	int32_t Keymgmt = 0;
	int32_t WpaVersion = 0;
	int32_t Chan = -1, Idx;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	if (pChannel != NULL)
		Chan = ieee80211_frequency_to_channel(pChannel->center_freq);

	CFG80211DBG(RT_DEBUG_TRACE, ("Groupwise: %x\n", pSme->crypto.cipher_group));
	Groupwise = pSme->crypto.cipher_group;
	//for(Idx=0; Idx<pSme->crypto.n_ciphers_pairwise; Idx++)
	Pairwise |= pSme->crypto.ciphers_pairwise[0];

	CFG80211DBG(RT_DEBUG_TRACE, ("Pairwise %x\n", pSme->crypto.ciphers_pairwise[0]));

	for(Idx=0; Idx<pSme->crypto.n_akm_suites; Idx++)
		Keymgmt |= pSme->crypto.akm_suites[Idx];

	WpaVersion = pSme->crypto.wpa_versions;
	CFG80211DBG(RT_DEBUG_TRACE, ("Wpa_versions %x\n", WpaVersion));

	memset(&ConnInfo, 0, sizeof(ConnInfo));
	ConnInfo.WpaVer = 0;

	if (WpaVersion & NL80211_WPA_VERSION_1) {
		ConnInfo.WpaVer = 1;
	}

	if (WpaVersion & NL80211_WPA_VERSION_2) {
		ConnInfo.WpaVer = 2;
	}

	CFG80211DBG(RT_DEBUG_TRACE, ("Keymgmt %x\n", Keymgmt));
	if (Keymgmt ==  WLAN_AKM_SUITE_8021X)
		ConnInfo.FlgIs8021x = TRUE;
	else
		ConnInfo.FlgIs8021x = FALSE;

	CFG80211DBG(RT_DEBUG_TRACE, ("Auth_type %x\n", pSme->auth_type));
	if (pSme->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
		ConnInfo.AuthType = Ndis802_11AuthModeShared;
	else if (pSme->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
		ConnInfo.AuthType = Ndis802_11AuthModeOpen;
	else
		ConnInfo.AuthType = Ndis802_11AuthModeAutoSwitch;

	if (Pairwise == WLAN_CIPHER_SUITE_CCMP) {
		CFG80211DBG(RT_DEBUG_TRACE, ("WLAN_CIPHER_SUITE_CCMP...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_CCMP;
	} else if (Pairwise == WLAN_CIPHER_SUITE_TKIP) {
		CFG80211DBG(RT_DEBUG_TRACE, ("WLAN_CIPHER_SUITE_TKIP...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_TKIP;
	} else if ((Pairwise == WLAN_CIPHER_SUITE_WEP40) ||
		   (Pairwise & WLAN_CIPHER_SUITE_WEP104)) {
		CFG80211DBG(RT_DEBUG_TRACE, ("WLAN_CIPHER_SUITE_WEP...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_WEP;
	} else {
		CFG80211DBG(RT_DEBUG_TRACE, ("NONE...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_NONE;
	}

	if (Groupwise == WLAN_CIPHER_SUITE_CCMP) {
		ConnInfo.GroupwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_CCMP;
	} else if (Groupwise == WLAN_CIPHER_SUITE_TKIP) {
		ConnInfo.GroupwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_TKIP;
	} else {
		ConnInfo.GroupwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_NONE;
	}

	CFG80211DBG(RT_DEBUG_TRACE, ("ConnInfo.KeyLen ===> %d\n", pSme->key_len));
	CFG80211DBG(RT_DEBUG_TRACE, ("ConnInfo.KeyIdx ===> %d\n", pSme->key_idx));

	ConnInfo.pKey = (UINT8 *)(pSme->key);
	ConnInfo.KeyLen = pSme->key_len;
	ConnInfo.pSsid = (char *) pSme->ssid;
	ConnInfo.SsidLen = pSme->ssid_len;
	ConnInfo.KeyIdx = pSme->key_idx;
	/* YF@20120328: Reset to default */
	ConnInfo.bWpsConnection= FALSE;

	//hex_dump("AssocInfo:", pSme->ie, pSme->ie_len);

	/* YF@20120328: Use SIOCSIWGENIE to make out the WPA/WPS IEs in AssocReq. */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	if(pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_CLIENT) {
		if (pSme->ie_len > 0)
			RTMP_DRIVER_80211_P2PCLI_ASSSOC_IE_SET(pAd, pSme->ie, pSme->ie_len);
		else
			RTMP_DRIVER_80211_P2PCLI_ASSSOC_IE_SET(pAd, NULL, 0);
	} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
		if (pSme->ie_len > 0)
			RTMP_DRIVER_80211_GEN_IE_SET(pAd, pSme->ie, pSme->ie_len);
		else
			RTMP_DRIVER_80211_GEN_IE_SET(pAd, NULL, 0);
	}

#ifdef DOT11W_PMF_SUPPORT
	CFG80211DBG(RT_DEBUG_OFF, ("80211> PMF Connect %d\n", pSme->mfp));
	if (pSme->mfp) {
		ConnInfo.mfp = TRUE;
	} else {
		ConnInfo.mfp = FALSE;
	}
#endif /* DOT11W_PMF_SUPPORT */

	if ((pSme->ie_len > 6) /* EID(1) + LEN(1) + OUI(4) */ &&
	    (pSme->ie[0] == WLAN_EID_VENDOR_SPECIFIC &&
	     pSme->ie[1] >= 4 &&
	     pSme->ie[2] == 0x00 && pSme->ie[3] == 0x50 && pSme->ie[4] == 0xf2 &&
	     pSme->ie[5] == 0x04)) {
		ConnInfo.bWpsConnection= TRUE;
	}

	/* %NULL if not specified (auto-select based on scan)*/
	if (pSme->bssid != NULL) {
		CFG80211DBG(RT_DEBUG_OFF, ("80211> Connect bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(pSme->bssid)));
		ConnInfo.pBssid = (char *) pSme->bssid;
	}

	RTMP_DRIVER_80211_CONNECT(pAd, &ConnInfo, pNdev->ieee80211_ptr->iftype);
#endif /*CONFIG_STA_SUPPORT*/
	return 0;
} /* End of CFG80211_OpsConnect */


/*
========================================================================
Routine Description:
	Disconnect from the BSS/ESS.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	ReasonCode		-

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: connect
========================================================================
*/
static int CFG80211_OpsDisconnect(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	u16			ReasonCode)
{
#ifdef CONFIG_STA_SUPPORT

	struct rtmp_adapter *pAd;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> ReasonCode = %d\n", ReasonCode));

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev->ieee80211_ptr->iftype);
#endif /*CONFIG_STA_SUPPORT*/
	return 0;
}



#ifdef RFKILL_HW_SUPPORT
static int CFG80211_OpsRFKill(
	struct wiphy		*pWiphy)
{
	struct rtmp_adapter		*pAd;
	BOOLEAN		active;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_80211_RFKILL(pAd, &active);
	wiphy_rfkill_set_hw_state(pWiphy, !active);
	return active;
}


VOID CFG80211_RFKillStatusUpdate(
	PVOID			pAd,
	BOOLEAN			active)
{
	struct wiphy *pWiphy;
	CFG80211_CB *pCfg80211_CB;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	wiphy_rfkill_set_hw_state(pWiphy, !active);
	return;
}
#endif /* RFKILL_HW_SUPPORT */


/*
========================================================================
Routine Description:
	Get site survey information.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	Idx				-
	pSurvey			-

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: survey dump
========================================================================
*/
static int CFG80211_OpsSurveyGet(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNdev,
	IN int			Idx,
	IN struct survey_info	*pSurvey)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_SURVEY SurveyInfo;


	if (Idx != 0)
		return -ENOENT;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	/* get information from driver */
	RTMP_DRIVER_80211_SURVEY_GET(pAd, &SurveyInfo);

	/* return the information to upper layer */
	pSurvey->channel = ((CFG80211_CB *)(SurveyInfo.pCfg80211))->pCfg80211_Channels;
	pSurvey->filled = SURVEY_INFO_TIME_BUSY |
			  SURVEY_INFO_TIME_EXT_BUSY;
	pSurvey->time_busy  = SurveyInfo.ChannelTimeBusy; /* unit: us */
	pSurvey->time_ext_busy = SurveyInfo.ChannelTimeExtBusy;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> busy time = %ld %ld\n",
				(ULONG)SurveyInfo.ChannelTimeBusy,
				(ULONG)SurveyInfo.ChannelTimeExtBusy));
	return 0;
} /* End of CFG80211_OpsSurveyGet */


/*
========================================================================
Routine Description:
	Cache a PMKID for a BSSID.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pPmksa			- PMKID information

Return Value:
	0				- success
	-x				- fail

Note:
	This is mostly useful for fullmac devices running firmwares capable of
	generating the (re) association RSN IE.
	It allows for faster roaming between WPA2 BSSIDs.
========================================================================
*/
static int CFG80211_OpsPmksaSet(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	struct cfg80211_pmksa	*pPmksa)
{
#ifdef CONFIG_STA_SUPPORT
	struct rtmp_adapter *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;


	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	if ((pPmksa->bssid == NULL) || (pPmksa->pmkid == NULL))
		return -ENOENT;

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_ADD;
	pIoctlPmaSa->pBssid = (UCHAR *)pPmksa->bssid;
	pIoctlPmaSa->pPmkid = (char *)  pPmksa->pmkid;

	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */

	return 0;
} /* End of CFG80211_OpsPmksaSet */


/*
========================================================================
Routine Description:
	Delete a cached PMKID.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pPmksa			- PMKID information

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsPmksaDel(
	struct wiphy		*pWiphy,
	struct net_device	*pNdev,
	struct cfg80211_pmksa	*pPmksa)
{
#ifdef CONFIG_STA_SUPPORT
	struct rtmp_adapter *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;

	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	if ((pPmksa->bssid == NULL) || (pPmksa->pmkid == NULL))
		return -ENOENT;

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_REMOVE;
	pIoctlPmaSa->pBssid = (UCHAR *)pPmksa->bssid;
	pIoctlPmaSa->pPmkid = (char *) pPmksa->pmkid;

	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */

	return 0;
} /* End of CFG80211_OpsPmksaDel */


/*
========================================================================
Routine Description:
	Flush a cached PMKID.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsPmksaFlush(
	IN struct wiphy		*pWiphy,
	IN struct net_device	*pNdev)
{
#ifdef CONFIG_STA_SUPPORT
	struct rtmp_adapter *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;


	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_FLUSH;
	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */

	return 0;
} /* End of CFG80211_OpsPmksaFlush */

static int CFG80211_OpsRemainOnChannel(
	struct wiphy 		*pWiphy,
	struct wireless_dev	*pWdev,
	struct ieee80211_channel *pChan,
	IN unsigned int duration,
	OUT u64 *cookie)
{
	struct rtmp_adapter *pAd;
	uint32_t ChanId;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	u32 rndCookie;

	struct net_device *dev = NULL;
	INT ChannelType = RT_CMD_80211_CHANTYPE_HT20;
	dev = pWdev->netdev;

	rndCookie = ((RandomByte2(pAd) * 256 * 256* 256) + (RandomByte2(pAd) * 256 * 256) + (RandomByte2(pAd) * 256) + RandomByte2(pAd)) |1;
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	/*CFG_TODO: Shall check channel type*/

	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(RT_DEBUG_TRACE, ("%s: CH = %d, Type = %d, duration = %d, cookie=%d\n", __FUNCTION__,
		ChanId, ChannelType, duration, rndCookie));

	/* init */
	*cookie = rndCookie;
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;
	ChanInfo.IfType = dev->ieee80211_ptr->iftype;
	ChanInfo.ChanType = ChannelType;
	ChanInfo.chan = pChan;
	ChanInfo.cookie = rndCookie;
	ChanInfo.pWdev = pWdev;

	/* set channel */
	RTMP_DRIVER_80211_REMAIN_ON_CHAN_SET(pAd, &ChanInfo, duration);
	return 0;
}

static void CFG80211_OpsMgmtFrameRegister(
	struct wiphy 		*pWiphy,
	struct wireless_dev	*wdev,
	u16 frame_type, bool reg)
{
	struct rtmp_adapter *pAd;
	struct net_device *dev = NULL;

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return;
	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);

	CFG80211DBG(RT_DEBUG_INFO, ("80211> %s ==>\n", __FUNCTION__));
	CFG80211DBG(RT_DEBUG_INFO, ("frame_type = %x, req = %d , (%d)\n", frame_type, reg,  dev->ieee80211_ptr->iftype));

	if (frame_type == IEEE80211_STYPE_PROBE_REQ)
		RTMP_DRIVER_80211_MGMT_FRAME_REG(pAd, dev, reg);
	else if (frame_type == IEEE80211_STYPE_ACTION)
		RTMP_DRIVER_80211_ACTION_FRAME_REG(pAd, dev, reg);
	else
		CFG80211DBG(RT_DEBUG_ERROR, ("Unkown frame_type = %x, req = %d\n", frame_type, reg));


}

//Supplicant_NEW_TDLS

static int CFG80211_OpsMgmtTx(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev,
	struct cfg80211_mgmt_tx_params *params,
	u64 *pCookie)
{
	const u8 *pBuf = params->buf;
	size_t Len = params->len;
	struct ieee80211_channel *pChan = params->chan;
	struct rtmp_adapter *pAd;
	uint32_t ChanId;
	struct net_device *dev = NULL;

	CFG80211DBG(RT_DEBUG_INFO, ("80211> %s ==>\n", __FUNCTION__));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);

	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(RT_DEBUG_INFO, ("80211> Mgmt Channel = %d\n", ChanId));

	/* Send the Frame with basic rate 6 */

	*pCookie = 5678;
	RTMP_DRIVER_80211_CHANNEL_LOCK(pAd, ChanId);
	RTMP_DRIVER_80211_MGMT_FRAME_SEND(pAd, (u8 *) pBuf, Len);

	/* Mark it for using Supplicant-Based off-channel wait
		if (Offchan)
			RTMP_DRIVER_80211_CHANNEL_RESTORE(pAd);
	 */

	return 0;
}


static int CFG80211_OpsTxCancelWait(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev,
	u64 cookie)
{
	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s ==>\n", __FUNCTION__));
    	return 0;
}

static int CFG80211_OpsCancelRemainOnChannel(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev,
	u64 cookie)
{
	struct rtmp_adapter *pAd;
	CFG80211DBG(RT_DEBUG_INFO, ("80211> %s ==>\n", __FUNCTION__));

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

    /* It cause the Supplicant-based OffChannel Hang */
    RTMP_DRIVER_80211_CANCEL_REMAIN_ON_CHAN_SET(pAd, cookie);
    return 0;
}

static int CFG80211_OpsStartAp(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_ap_settings *settings)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	u8 *beacon_head_buf = NULL, *beacon_tail_buf = NULL;

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));

	if (settings->beacon.head_len > 0) {
		beacon_head_buf = kmalloc(settings->beacon.head_len, GFP_ATOMIC);
		memcpy(beacon_head_buf, settings->beacon.head, settings->beacon.head_len);
	}

	if (settings->beacon.tail_len > 0) {
		beacon_tail_buf =  kmalloc(settings->beacon.tail_len, GFP_ATOMIC);
		memcpy(beacon_tail_buf, settings->beacon.tail, settings->beacon.tail_len);
	}

	bcn.beacon_head_len = settings->beacon.head_len;
	bcn.beacon_tail_len = settings->beacon.tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = settings->dtim_period;
        bcn.interval = settings->beacon_interval;


	RTMP_DRIVER_80211_BEACON_ADD(pAd, &bcn);

	if (beacon_head_buf)
		kfree(beacon_head_buf);
	if (beacon_tail_buf)
		kfree(beacon_tail_buf);

	return 0;
}

static int CFG80211_OpsChangeBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_beacon_data *info)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	u8 *beacon_head_buf = NULL, *beacon_tail_buf = NULL;

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));

	if (info->head_len > 0) {
		beacon_head_buf = kmalloc(info->head_len, GFP_ATOMIC);
		memcpy(beacon_head_buf, info->head, info->head_len);
	}

	if (info->tail_len > 0) {
		beacon_tail_buf = kmalloc(info->tail_len, GFP_ATOMIC);
		memcpy(beacon_tail_buf, info->tail, info->tail_len);
	}

	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;

	RTMP_DRIVER_80211_BEACON_SET(pAd, &bcn);

	if (beacon_head_buf)
		kfree(beacon_head_buf);
	if (beacon_tail_buf)
		kfree(beacon_tail_buf);
	return 0;

}

static int CFG80211_OpsStopAp(
	struct wiphy *pWiphy,
	struct net_device *netdev)
{
	struct rtmp_adapter *pAd;
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s ==>\n", __FUNCTION__));

	RTMP_DRIVER_80211_BEACON_DEL(pAd);
	return 0;
}

static int CFG80211_OpsChangeBss(
        struct wiphy *pWiphy,
        struct net_device *netdev,
	struct bss_parameters *params)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_BSS_PARM bssInfo;

	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));

	bssInfo.use_short_preamble = params->use_short_preamble;
	bssInfo.use_short_slot_time = params->use_short_slot_time;
	bssInfo.use_cts_prot = params->use_cts_prot;

	RTMP_DRIVER_80211_CHANGE_BSS_PARM(pAd, &bssInfo);

	return 0;
}

static int CFG80211_OpsStaDel(
	struct wiphy *pWiphy,
	struct net_device *dev,
	struct station_del_parameters *params)
{
	struct rtmp_adapter *pAd;
	UINT8 *pMacAddr = (u8 *) params->mac;
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	if (pMacAddr ==  NULL) {
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, NULL);
	} else {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> Delete STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
					PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, pMacAddr);
	}

	return 0;
}

static int CFG80211_OpsStaAdd(
        struct wiphy *wiphy,
        struct net_device *dev,
        const u8 *mac,
	struct station_parameters *params)
{
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> %s ==>\n", __FUNCTION__));
	return 0;
}

static int CFG80211_OpsStaChg(struct wiphy *pWiphy, struct net_device *dev,
	const u8 *pMacAddr, struct station_parameters *params)
{
	struct rtmp_adapter *pAd;
	CFG80211_CB *p80211CB;

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> Change STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n", PRINT_MAC(pMacAddr)));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	p80211CB = NULL;

	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

	if ((dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP) &&
	    (dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_P2P_GO))
		return -EOPNOTSUPP;

	if(!(params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED))) {
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> %x ==>\n", params->sta_flags_mask));
		return -EOPNOTSUPP;
	}

	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> STA(%02X:%02X:%02X:%02X:%02X:%02X) ==> PortSecured\n",
			PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, pMacAddr, 1);
	} else {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> STA(%02X:%02X:%02X:%02X:%02X:%02X) ==> PortNotSecured\n",
                        PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, pMacAddr, 0);
	}

	return 0;
}


static struct wireless_dev* CFG80211_OpsVirtualInfAdd(
        struct wiphy *pWiphy, const char *name,
        unsigned char name_assign_type, enum nl80211_iftype Type,
        u32 *pFlags, struct vif_params *pParams)
{
	struct rtmp_adapter *pAd;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
	PWIRELESS_DEV pDev = NULL;
	pAd = MAC80211_PAD_GET(pWiphy);

	if (pAd == NULL)
		return NULL;

	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s [%s,%d, %d] ==>\n", __FUNCTION__, name, Type, (int) strlen(name)));

	vifInfo.vifType = Type;
	vifInfo.vifNameLen = strlen(name);
	memset(vifInfo.vifName, 0, sizeof(vifInfo.vifName));
	memcpy(vifInfo.vifName, name, vifInfo.vifNameLen);

	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS)
		return NULL;

	pDev = RTMP_CFG80211_FindVifEntryWdev_ByType(pAd, Type);

	return pDev;
}

static int CFG80211_OpsVirtualInfDel(
	struct wiphy     	*pWiphy,
	struct wireless_dev 	*pwdev)
{
	struct rtmp_adapter *pAd;
	struct net_device *dev = NULL;
	dev = pwdev->netdev;

	if (!dev)
		return 0;

	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s, %s [%d]==>\n", __FUNCTION__, dev->name, dev->ieee80211_ptr->iftype));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	RTMP_DRIVER_80211_VIF_DEL(pAd, dev, dev->ieee80211_ptr->iftype);
	return 0;
}


static int CFG80211_OpsBitrateSet(
        IN struct wiphy                                 *pWiphy,
	IN struct net_device *dev,
	IN const u8 *peer,
	IN const struct cfg80211_bitrate_mask *mask)
{
	return 0;
}

#ifdef CONFIG_NL80211_TESTMODE
static int CFG80211_OpsTestModeCmd(IN struct wiphy *pWiphy, void *Data, int len)
{
}
#endif /* CONFIG_NL80211_TESTMODE */

static int CFG80211_start_p2p_device(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev)
{
	struct rtmp_adapter *pAd;
	struct net_device *dev = wdev->netdev;

	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s, %s [%d]==>\n", __FUNCTION__, dev->name, dev->ieee80211_ptr->iftype));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return -EINVAL;

	return 0;
}

static void CFG80211_stop_p2p_device(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev)
{
	struct rtmp_adapter *pAd;
	struct net_device *dev = wdev->netdev;

	CFG80211DBG(RT_DEBUG_OFF, ("80211> %s, %s [%d]==>\n", __FUNCTION__, dev->name, dev->ieee80211_ptr->iftype));
	pAd = MAC80211_PAD_GET(pWiphy);
	if (pAd == NULL)
		return;

	return;
}

static const struct ieee80211_txrx_stypes
ralink_mgmt_stypes[NUM_NL80211_IFTYPES] = {
		[NL80211_IFTYPE_STATION] = {
				.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
				BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
				.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
				BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
		},
		[NL80211_IFTYPE_P2P_CLIENT] = {
				.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
				BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
				.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
				BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
		},
        [NL80211_IFTYPE_AP] = {
                        .tx = 0xffff,
                        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
                        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
                        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
                        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
                        BIT(IEEE80211_STYPE_AUTH >> 4) |
                        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
                        BIT(IEEE80211_STYPE_ACTION >> 4),
        },
		[NL80211_IFTYPE_P2P_GO] = {
				.tx = 0xffff,
				.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
				BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
				BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
				BIT(IEEE80211_STYPE_DISASSOC >> 4) |
				BIT(IEEE80211_STYPE_AUTH >> 4) |
				BIT(IEEE80211_STYPE_DEAUTH >> 4) |
				BIT(IEEE80211_STYPE_ACTION >> 4),
		},

};

static const struct ieee80211_iface_limit ra_p2p_sta_go_limits[] =
{
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_STATION),
	},
	{
		.max = 1,
		.types = BIT(NL80211_IFTYPE_P2P_GO) |
			 BIT(NL80211_IFTYPE_AP),
	},
        {
                .max = 1,
                .types = BIT(NL80211_IFTYPE_P2P_CLIENT),
        },
};

static const struct ieee80211_iface_combination
ra_iface_combinations_p2p[] = {
	{
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
		.num_different_channels = 2,
#else
		.num_different_channels = 1,
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
		.max_interfaces = 3,
		.beacon_int_infra_match = true,
		.limits = ra_p2p_sta_go_limits,
		.n_limits = ARRAY_SIZE(ra_p2p_sta_go_limits),
	},
};

struct cfg80211_ops CFG80211_Ops = {

	.start_ap	    	= CFG80211_OpsStartAp,
	.change_beacon		= CFG80211_OpsChangeBeacon,
	.stop_ap	    	= CFG80211_OpsStopAp,

	/* set channel for a given wireless interface */
#if 0	/* ULLI : disabled */
	.set_monitor_channel 	= CFG80211_OpsMonitorChannelSet,
#endif

	/* change type/configuration of virtual interface */
	.change_virtual_intf	= CFG80211_OpsVirtualInfChg,
	.add_virtual_intf	= CFG80211_OpsVirtualInfAdd,
	.del_virtual_intf	= CFG80211_OpsVirtualInfDel,


	.start_p2p_device	= CFG80211_start_p2p_device,
	.stop_p2p_device	= CFG80211_stop_p2p_device,

	/* request to do a scan */
	/*
		Note: must exist whatever AP or STA mode; Or your kernel will crash
		in v2.6.38.
	*/
	.scan			= CFG80211_OpsScan,

#ifdef CONFIG_STA_SUPPORT
	/* join the specified IBSS (or create if necessary) */
	.join_ibss		= CFG80211_OpsIbssJoin,
	/* leave the IBSS */
	.leave_ibss		= CFG80211_OpsIbssLeave,
#endif /* CONFIG_STA_SUPPORT */

	/* set the transmit power according to the parameters */
	.set_tx_power		= CFG80211_OpsTxPwrSet,
	/* store the current TX power into the dbm variable */
	.get_tx_power		= CFG80211_OpsTxPwrGet,
	/* configure WLAN power management */
	.set_power_mgmt		= CFG80211_OpsPwrMgmt,
	/* get station information for the station identified by @mac */
	.get_station		= CFG80211_OpsStaGet,
	/* dump station callback */
	.dump_station		= CFG80211_OpsStaDump,
	/* notify that wiphy parameters have changed */
	.set_wiphy_params	= CFG80211_OpsWiphyParamsSet,
	/* add a key with the given parameters */
	.add_key		= CFG80211_OpsKeyAdd,
	/* get information about the key with the given parameters */
	.get_key		= CFG80211_OpsKeyGet,
	/* remove a key given the @mac_addr */
	.del_key		= CFG80211_OpsKeyDel,
	/* set the default key on an interface */
	.set_default_key	= CFG80211_OpsKeyDefaultSet,
#ifdef DOT11W_PMF_SUPPORT
	.set_default_mgmt_key	= CFG80211_OpsMgmtKeyDefaultSet,
#endif /* DOT11W_PMF_SUPPORT */

	/* connect to the ESS with the specified parameters */
	.connect		= CFG80211_OpsConnect,
	/* disconnect from the BSS/ESS */
	.disconnect		= CFG80211_OpsDisconnect,

#ifdef RFKILL_HW_SUPPORT
	/* polls the hw rfkill line */
	.rfkill_poll		= CFG80211_OpsRFKill,
#endif /* RFKILL_HW_SUPPORT */

	/* get site survey information */
	//.dump_survey				= CFG80211_OpsSurveyGet,
	/* cache a PMKID for a BSSID */
	.set_pmksa		= CFG80211_OpsPmksaSet,
	/* delete a cached PMKID */
	.del_pmksa		= CFG80211_OpsPmksaDel,
	/* flush all cached PMKIDs */
	.flush_pmksa		= CFG80211_OpsPmksaFlush,

	/*
		Request the driver to remain awake on the specified
		channel for the specified duration to complete an off-channel
		operation (e.g., public action frame exchange).
	*/
	.remain_on_channel	= CFG80211_OpsRemainOnChannel,
	/* cancel an on-going remain-on-channel operation */
	.cancel_remain_on_channel =  CFG80211_OpsCancelRemainOnChannel,
	.mgmt_tx              	= CFG80211_OpsMgmtTx,

	 .mgmt_tx_cancel_wait   = CFG80211_OpsTxCancelWait,


	/* configure connection quality monitor RSSI threshold */
	.set_cqm_rssi_config	= NULL,

	/* notify driver that a management frame type was registered */
#if 0  /* ULLI : disabled */
	.mgmt_frame_register	= CFG80211_OpsMgmtFrameRegister,
#endif

	/* set antenna configuration (tx_ant, rx_ant) on the device */
	.set_antenna		= NULL,
	/* get current antenna configuration from device (tx_ant, rx_ant) */
	.get_antenna		= NULL,
	.change_bss             = CFG80211_OpsChangeBss,
	.del_station            = CFG80211_OpsStaDel,
	.add_station            = CFG80211_OpsStaAdd,
	.change_station         = CFG80211_OpsStaChg,
//	.set_bitrate_mask                       = CFG80211_OpsBitrateSet,
#ifdef CONFIG_NL80211_TESTMODE
        .testmode_cmd           = CFG80211_OpsTestModeCmd,
#endif /* CONFIG_NL80211_TESTMODE */
};

/* =========================== Global Function ============================== */

static struct wireless_dev *CFG80211_WdevAlloc(
	CFG80211_CB		*pCfg80211_CB,
	CFG80211_BAND		*pBandInfo,
	struct rtmp_adapter	*pAd,
	struct device		*pDev)
{
	struct wireless_dev *pWdev;
	ULONG *pPriv;


	/*
	 * We're trying to have the following memory layout:
	 *
	 * +------------------------+
	 * | struct wiphy			|
	 * +------------------------+
	 * | pAd pointer			|
	 * +------------------------+
	 */

	pWdev = kzalloc(sizeof(struct wireless_dev), GFP_KERNEL);
	if (pWdev == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Wireless device allocation fail!\n"));
		return NULL;
	} /* End of if */

	pWdev->wiphy = wiphy_new(&CFG80211_Ops, sizeof(ULONG *));
	if (pWdev->wiphy == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Wiphy device allocation fail!\n"));
		goto LabelErrWiphyNew;
	} /* End of if */

	/* keep pAd pointer */
	pPriv = (ULONG *)(wiphy_priv(pWdev->wiphy));
	*pPriv = (ULONG)pAd;

	set_wiphy_dev(pWdev->wiphy, pDev);

	pWdev->wiphy->max_scan_ssids = pBandInfo->MaxBssTable;

	/* @NL80211_FEATURE_INACTIVITY_TIMER:
	   This driver takes care of freeingup
	   the connected inactive stations in AP mode.*/

	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* http://www.permalink.gmane.org/gmane.linux.kernel.wireless.general/86454 */
	pWdev->wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;

	pWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_STATION)
									;

#ifdef CONFIG_STA_SUPPORT

	pWdev->wiphy->interface_modes |= BIT(NL80211_IFTYPE_ADHOC);

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	pWdev->wiphy->interface_modes |= (BIT(NL80211_IFTYPE_P2P_CLIENT)
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
#endif /* CONFIG_STA_SUPPORT */

	//pWdev->wiphy->reg_notifier = CFG80211_RegNotifier;

	/* init channel information */
	CFG80211_SupBandInit(pCfg80211_CB, pBandInfo, pWdev->wiphy, NULL, NULL);

	/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
	pWdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	pWdev->wiphy->max_scan_ie_len = IEEE80211_MAX_DATA_LEN;
	pWdev->wiphy->max_num_pmkids = 4;

	pWdev->wiphy->max_remain_on_channel_duration = 5000;

	pWdev->wiphy->mgmt_stypes = ralink_mgmt_stypes;

	pWdev->wiphy->cipher_suites = CipherSuites;
	pWdev->wiphy->n_cipher_suites = ARRAY_SIZE(CipherSuites);

	pWdev->wiphy->flags |= WIPHY_FLAG_AP_UAPSD;

	/*what if you get compile error for below flag, please add the patch into your kernel*/
        /* 018-cfg80211-internal-ap-mlme.patch */
	pWdev->wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;

	/*what if you get compile error for below flag, please add the patch into your kernel*/
     /* 008-cfg80211-offchan-flags.patch */
	pWdev->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
	//CFG_TODO
	//pWdev->wiphy->flags |= WIPHY_FLAG_STRICT_REGULATORY;

	//Driver Report Support TDLS to supplicant
	/* */
	//pWdev->wiphy->iface_combinations = ra_iface_combinations_p2p;
	//pWdev->wiphy->n_iface_combinations = ARRAY_SIZE(ra_iface_combinations_p2p);

	if (wiphy_register(pWdev->wiphy) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Register wiphy device fail!\n"));
		goto LabelErrReg;
	}

	return pWdev;

 LabelErrReg:
	wiphy_free(pWdev->wiphy);

 LabelErrWiphyNew:
	kfree(pWdev);

	return NULL;
} /* End of CFG80211_WdevAlloc */


/*
========================================================================
Routine Description:
	Register MAC80211 Module.

Arguments:
	pAdCB			- WLAN control block pointer
	pDev			- Generic device interface
	pNetDev			- Network device

Return Value:
	NONE

Note:
	pDev != pNetDev
	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))

	Can not use pNetDev to replace pDev; Or kernel panic.
========================================================================
*/
BOOLEAN CFG80211_Register(
	struct rtmp_adapter	*pAd,
	struct device		*pDev,
	struct net_device	*pNetDev)
{
	CFG80211_CB *pCfg80211_CB = NULL;
	CFG80211_BAND BandInfo;
	INT err;

	/* allocate Main Device Info structure */
	pCfg80211_CB = kmalloc(sizeof(CFG80211_CB), GFP_ATOMIC);
	if (pCfg80211_CB == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Allocate MAC80211 CB fail!\n"));
		return FALSE;
	}

	/* allocate wireless device */
	RTMP_DRIVER_80211_BANDINFO_GET(pAd, &BandInfo);

	pCfg80211_CB->pCfg80211_Wdev =
				CFG80211_WdevAlloc(pCfg80211_CB, &BandInfo, pAd, pDev);
	if (pCfg80211_CB->pCfg80211_Wdev == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Allocate Wdev fail!\n"));
		kfree(pCfg80211_CB);
		return FALSE;
	}

	/* bind wireless device with net device */
#ifdef CONFIG_AP_SUPPORT
	/* default we are AP mode */
	pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_AP;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	/* default we are station mode */
	pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_STATION;
#endif /* CONFIG_STA_SUPPORT */

	pNetDev->ieee80211_ptr = pCfg80211_CB->pCfg80211_Wdev;
	SET_NETDEV_DEV(pNetDev, wiphy_dev(pCfg80211_CB->pCfg80211_Wdev->wiphy));
	pCfg80211_CB->pCfg80211_Wdev->netdev = pNetDev;

#ifdef RFKILL_HW_SUPPORT
	wiphy_rfkill_start_polling(pCfg80211_CB->pCfg80211_Wdev->wiphy);
#endif /* RFKILL_HW_SUPPORT */

	RTMP_DRIVER_80211_CB_SET(pAd, pCfg80211_CB);
	RTMP_DRIVER_80211_RESET(pAd);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, TRUE);


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_Register\n"));
	return TRUE;
} /* End of CFG80211_Register */




/* =========================== Local Function =============================== */

/*
========================================================================
Routine Description:
	The driver's regulatory notification callback.

Arguments:
	pWiphy			- Wireless hardware description
	pRequest		- Regulatory request

Return Value:
	0

Note:
========================================================================
*/
static int32_t CFG80211_RegNotifier(
	struct wiphy			*pWiphy,
	struct regulatory_request	*pRequest)
{
	struct rtmp_adapter *pAd;
	ULONG *pPriv;


	/* sanity check */
	pPriv = (ULONG *)(wiphy_priv(pWiphy));
	pAd = (VOID *)(*pPriv);

	if (pAd == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("crda> reg notify but pAd = NULL!"));
		return 0;
	} /* End of if */

	/*
		Change the band settings (PASS scan, IBSS allow, or DFS) in mac80211
		based on EEPROM.

		IEEE80211_CHAN_DISABLED: This channel is disabled.
		IEEE80211_CHAN_PASSIVE_SCAN: Only passive scanning is permitted
					on this channel.
		IEEE80211_CHAN_NO_IBSS: IBSS is not allowed on this channel.
		IEEE80211_CHAN_RADAR: Radar detection is required on this channel.
		IEEE80211_CHAN_NO_FAT_ABOVE: extension channel above this channel
					is not permitted.
		IEEE80211_CHAN_NO_FAT_BELOW: extension channel below this channel
					is not permitted.
	*/

	/*
		Change regulatory rule here.

		struct ieee80211_channel {
			enum ieee80211_band band;
			u16 center_freq;
			u8 max_bandwidth;
			u16 hw_value;
			u32 flags;
			int max_antenna_gain;
			int max_power;
			bool beacon_found;
			u32 orig_flags;
			int orig_mag, orig_mpwr;
		};

		In mac80211 layer, it will change flags, max_antenna_gain,
		max_bandwidth, max_power.
	*/

	switch(pRequest->initiator) {
	case NL80211_REGDOM_SET_BY_CORE:
		/*
			Core queried CRDA for a dynamic world regulatory domain.
		*/
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by core: "));
		break;

	case NL80211_REGDOM_SET_BY_USER:
		/*
			User asked the wireless core to set the regulatory domain.
			(when iw, network manager, wpa supplicant, etc.)
		*/
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by user: "));
		break;

	case NL80211_REGDOM_SET_BY_DRIVER:
		/*
			A wireless drivers has hinted to the wireless core it thinks
			its knows the regulatory domain we should be in.
			(when driver initialization, calling regulatory_hint)
		*/
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by driver: "));
		break;

	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		/*
			The wireless core has received an 802.11 country information
			element with regulatory information it thinks we should consider.
			(when beacon receive, calling regulatory_hint_11d)
		*/
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by country IE: "));
		break;
	}

	CFG80211DBG(RT_DEBUG_OFF, ("%c%c\n", pRequest->alpha2[0], pRequest->alpha2[1]));

	/* only follow rules from user */
	if (pRequest->initiator == NL80211_REGDOM_SET_BY_USER) {
		/* keep Alpha2 and we can re-call the function when interface is up */
		CMD_RTPRIV_IOCTL_80211_REG_NOTIFY RegInfo;

		RegInfo.Alpha2[0] = pRequest->alpha2[0];
		RegInfo.Alpha2[1] = pRequest->alpha2[1];
		RegInfo.pWiphy = pWiphy;

		RTMP_DRIVER_80211_REG_NOTIFY(pAd, &RegInfo);
	} /* End of if */

	return 0;
} /* End of CFG80211_RegNotifier */

#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE */

/* End of crda.c */
