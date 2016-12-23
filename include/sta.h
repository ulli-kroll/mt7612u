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
    sta.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#ifndef __STA_H__
#define __STA_H__


BOOLEAN RTMPCheckChannel(
	IN struct rtmp_adapter *pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel);

VOID InitChannelRelatedValue(
	IN  struct rtmp_adapter *  pAd);

VOID AdjustChannelRelatedValue(
	IN struct rtmp_adapter *pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN BOOLEAN BandWidth,
	IN UCHAR PriCh,
	IN UCHAR ExtraCh);

VOID RTMPReportMicError(
	IN  struct rtmp_adapter *  pAd,
	IN  PCIPHER_KEY     pWpaKey);

INT RTMPCheckRxError(
	IN struct rtmp_adapter *pAd,
	IN HEADER_802_11 *pHeader,
	IN RX_BLK *pRxBlk,
	IN RXINFO_STRUC *pRxInfo);

VOID WpaMicFailureReportFrame(
	IN  struct rtmp_adapter *   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID WpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(
	IN	struct rtmp_adapter *pAd);

VOID WpaStaGroupKeySetting(
	IN	struct rtmp_adapter *pAd);

VOID WpaSendEapolStart(
	IN	struct rtmp_adapter *pAdapter,
	IN  u8 *         pBssid);


VOID STAHandleRxDataFrame(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);

VOID STARxEAPOLFrameIndicate(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

int STAHardTransmit(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK			*pTxBlk,
	IN  UCHAR			QueIdx);

INT STASendPacket(
	IN struct rtmp_adapter *pAd,
	IN struct sk_buff *pPacket);

INT STAInitialize(struct rtmp_adapter *pAd);

#endif /* __STA_H__ */

