/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	aironet.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__ACTION_H__
#define	__ACTION_H__


struct rtmp_adapter;



VOID MlmeQOSAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDLSAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeInvalidAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerRMAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerQOSAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerAddBAReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAddBARspAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerDelBAAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerBAAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerHTAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerVHTAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem) ;

VOID PeerPublicAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

#ifdef CONFIG_STA_SUPPORT
VOID StaPublicAction(
	IN struct rtmp_adapter *pAd,
	IN BSS_2040_COEXIST_IE *pBss2040CoexIE);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID ApPublicAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif /* CONFIG_AP_SUPPORT */


#endif /* __ACTION_H__ */
