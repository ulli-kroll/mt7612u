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


#ifndef __CFG80211CMM_H__
#define __CFG80211CMM_H__

#ifdef RT_CFG80211_SUPPORT

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define CFG80211_GetEventDevice(__pAd) __pAd->cfg80211_ctrl.dummy_p2p_net_dev
#else
#define CFG80211_GetEventDevice(__pAd) __pAd->net_dev
#endif	/* RT_CFG80211_P2P_CONCURRENT_DEVICE */


//yiwei debug for P2P 7.1.3


#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211CMM_H__ */


