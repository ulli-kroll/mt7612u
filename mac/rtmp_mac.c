/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		txwi		Pointer to head of each MPDU to HW.
		Ack 		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs 		Setting for IFS gap
		Rate		Setting for transmit rate
		Service 	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003

	Return Value:
		None

	See also : BASmartHardTransmit()    !!!

	========================================================================
*/
VOID RTMPWriteTxWI(
	IN struct rtmp_adapter *pAd,
	IN struct mt7612u_txwi *pOutTxWI,
	IN bool FRAG,
	IN bool CFACK,
	IN bool InsTimestamp,
	IN bool AMPDU,
	IN bool Ack,
	IN bool NSeq,		/* HW new a sequence.*/
	IN u8 BASize,
	IN u8 WCID,
	IN ULONG Length,
	IN u8 PID,
	IN u8 TID,
	IN u8 TxRate,
	IN u8 Txopmode,
	IN HTTRANSMIT_SETTING *pTransmit)
{
	PMAC_TABLE_ENTRY pMac = NULL;
	struct mt7612u_txwi TxWI, *txwi;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT TxEAPId_Cal;
	u8 eTxBf, iTxBf, sounding, ndp_rate, stbc, bw, mcs, sgi, phy_mode, mpdu_density = 0, mimops = 0, ldpc = 0;
	u8 tx_stream_mode = 0;

	if (WCID < MAX_LEN_OF_MAC_TABLE)
		pMac = &pAd->MacTab.Content[WCID];

	/*
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	memset(&TxWI, 0, TXWISize);
	txwi = &TxWI;

	BASize = 0;
	stbc = pTransmit->field.STBC;

	/* If CCK or OFDM, BW must be 20*/
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);

	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	sgi = pTransmit->field.ShortGI;

	if (MT_REV_GTE(pAd, MT76x2, REV_MT76x2E4))
		tx_stream_mode = 0x13;
	else if (MT_REV_ET(pAd, MT76x2, REV_MT76x2E3))
		tx_stream_mode = (pTransmit->field.MODE <= MODE_OFDM) ? 0x93 : 0x0;

	if (pMac) {
		if (pAd->CommonCfg.bMIMOPSEnable) {
			if ((pMac->MmpsMode == MMPS_DYNAMIC) &&
			    (pTransmit->field.MCS > 7)) {
				/* Dynamic MIMO Power Save Mode*/
				mimops = 1;
			} else if (pMac->MmpsMode == MMPS_STATIC) {
				/* Static MIMO Power Save Mode*/
				if (pTransmit->field.MODE >= MODE_HTMIX &&
				    pTransmit->field.MCS > 7) {
					mcs = 7;
					mimops = 0;
				}
			}
		}

		mpdu_density = pMac->MpduDensity;
	}


	eTxBf = pTransmit->field.eTxBF;
	iTxBf = pTransmit->field.iTxBF;

	// Calculate the partial AID
	TxEAPId_Cal = WCID + (UINT)((pAd->CommonCfg.Bssid[5] >> 4) ^ (pAd->CommonCfg.Bssid[5] & 0x0F))*32;
	TxEAPId_Cal -= ((TxEAPId_Cal >> 9) << 9);

	if (pMac && pAd->chipCap.FlgHwTxBfCap) {
		if (pMac->TxSndgType == SNDG_TYPE_NDP  ||
		    pMac->TxSndgType == SNDG_TYPE_SOUNDING) {
			stbc  = false;
			eTxBf = false;
			iTxBf = false;
			sounding = false;
			ndp_rate = 1;
			//phy_mode = 1;
			//mcs = MCS_RATE_24;

			DBGPRINT(RT_DEBUG_TRACE, ("%s : NDP Sounding and NDPSndRate = %d\n", __FUNCTION__, ndp_rate));
			DBGPRINT(RT_DEBUG_TRACE, ("%s : BSSID[5] = %x, txwi->TxEAPId = %d\n", __FUNCTION__, pAd->CommonCfg.Bssid[5], TxEAPId_Cal));
		}
	}

	if (iTxBf || eTxBf)
		stbc = 0; // Force STBC = 0 when TxBf is enabled



	{
		txwi->FRAG = FRAG;
		txwi->CFACK= CFACK;
		txwi->TS = InsTimestamp;
		txwi->AMPDU = AMPDU;
		txwi->ACK = Ack;
		txwi->txop = Txopmode;
		txwi->NSEQ = NSeq;
		txwi->BAWinSize = BASize;
		txwi->ShortGI = sgi;
		txwi->STBC = stbc;
		txwi->LDPC = ldpc;
		txwi->MCS= mcs;
		txwi->BW = bw;
		txwi->PHYMODE= phy_mode;
		txwi->MpduDensity = mpdu_density;
		txwi->MIMOps = mimops;
		txwi->wcid = WCID;
		txwi->MPDUtotalByteCnt = Length;
		txwi->TxPktId = mcs; // PID is not used now!

#ifdef CONFIG_AP_SUPPORT
		txwi->GroupID = true;
		txwi->TxEAPId = TxEAPId_Cal;
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
		txwi->GroupID = false;
		txwi->TxEAPId = pAd->CommonCfg.Bssid[5];
#endif /*CONFIG_STA_SUPPORT*/

		txwi->TxStreamMode = tx_stream_mode;
		txwi->Sounding = sounding;
		txwi->eTxBF = eTxBf;
		txwi->iTxBF = iTxBf;
		txwi->NDPSndRate = ndp_rate;
		txwi->NDPSndBW = bw;
		txwi->TXBF_PT_SCA = (eTxBf | iTxBf) ? true : false;

	}

	memmove(pOutTxWI, &TxWI, TXWISize);
//+++Add by shiang for debug
if (0){
	hex_dump("TxWI", (u8 *)pOutTxWI, TXWISize);
}
//---Add by shiang for debug
}


VOID RTMPWriteTxWI_Data(struct rtmp_adapter *pAd, struct mt7612u_txwi *txwi, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit;
	MAC_TABLE_ENTRY *pMacEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	u8 wcid, pkt_id;
	u8 sgi, mcs, bw, stbc, phy_mode, ldpc;
	u8 basize, ampdu, mimops = 0, mpdu_density = 0;
	u8 iTxBf, eTxBf, sounding, ndp_rate, ndp_bw;
	u8 tx_stream_mode = 0;


	ASSERT(txwi);

	pTransmit = pTxBlk->pTransmit;
	pMacEntry = pTxBlk->pMacEntry;

	/*
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	memset(txwi, 0, TXWISize);

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
		wcid = pTxBlk->Wcid;

	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	/* If CCK or OFDM, BW must be 20 */
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);

	if (MT_REV_GTE(pAd, MT76x2, REV_MT76x2E4))
		tx_stream_mode = 0x13;
	else if (MT_REV_ET(pAd, MT76x2, REV_MT76x2E3))
		tx_stream_mode = (pTransmit->field.MODE <= MODE_OFDM) ? 0x93 : 0x0;

	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);

	ampdu = ((pTxBlk->TxFrameType == TX_AMPDU_FRAME) ? true : false);
	basize = pAd->CommonCfg.TxBASize;
	if(ampdu && pMacEntry) {
		/*
 		 * Under HT20, 2x2 chipset, OPEN, and with some atero chipsets
 		 * reduce BASize to 7 to add one bulk A-MPDU during one TXOP
 		 * to improve throughput
 		 */
		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) &&
		    (pAd->Antenna.field.TxPath == 2) &&
		    (pMacEntry->bIAmBadAtheros) &&
		    (pMacEntry->WepStatus == Ndis802_11EncryptionDisabled)) {
			basize = 7;
		} else {
			u8 RABAOriIdx = pTxBlk->pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];
			basize = pAd->BATable.BAOriEntry[RABAOriIdx].BAWinSize;
		}
	}

	if(pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
		ampdu = false;

	sounding = ndp_bw = ndp_rate = iTxBf = eTxBf = 0;
	if (pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING) {
		sounding = 1;
		iTxBf = false;
		eTxBf = false;
		stbc = false;
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): sending normal sounding, eTxBF=%d\n",
					__FUNCTION__, pTransmit->field.eTxBF));
		iTxBf = 0;
	} else if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP) {
		ndp_bw = pTransmit->field.BW;
		iTxBf = false;
		eTxBf = false;
		stbc = false;
		if (pTxBlk->TxNDPSndgMcs >= 16)
			ndp_rate = 2;
		else if (pTxBlk->TxNDPSndgMcs >= 8)
			ndp_rate = 1;
		else
			ndp_rate = 0;

		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): NDP sounding, eTxBF=%d, ndp_rate = %d\n",
					__FUNCTION__, eTxBf, ndp_rate));
	} else {
		eTxBf = pTransmit->field.eTxBF;
		iTxBf = pTransmit->field.iTxBF;
	}

	if (iTxBf || eTxBf)
		stbc = false; // Force STBC = false when TxBf is enabled

	if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
	{
		mcs = 0;
		ampdu = false;
	}

	if (pMacEntry)
	{
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (mcs > 7))
			mimops = 1;
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			if ((pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) &&
				(mcs > 7))
			{
				mcs = 7;
				mimops = 0;
			}
		}

		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pMacEntry->bIAmBadAtheros))
			mpdu_density = 7;
		else
			mpdu_density = pMacEntry->MpduDensity;
	}

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0) {
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

	/* for rate adapation*/
	pkt_id = mcs;

	{
		txwi->FRAG = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
		txwi->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
		if (RTMP_GET_PACKET_TDLS_WAIT_ACK(pTxBlk->pPacket)) {
			txwi->TxPktId |= 0x80;
			DBGPRINT(RT_DEBUG_INFO,("PktID |= 0x80 : [%x]\n",txwi->TxPktId));
		} else {
			txwi->TxPktId &= 0x7f;
			DBGPRINT(RT_DEBUG_INFO,("PktID : [%x]\n",txwi->TxPktId));
		}
#ifdef WFA_VHT_PF
		if (pAd->force_noack == true)
			txwi->ACK = 0;
#endif /* WFA_VHT_PF */

		if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
			txwi->txop = IFS_BACKOFF; // Reserve larger TXOP to prevent sounding packet from collision
		else
			txwi->txop = pTxBlk->FrameGap;

		txwi->wcid = wcid;
		txwi->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi->CFACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);
		txwi->ShortGI = sgi;
		txwi->STBC = stbc;
		txwi->LDPC = ldpc;
		txwi->TxStreamMode = tx_stream_mode;
		txwi->MCS = mcs;
		txwi->PHYMODE = phy_mode;
		txwi->BW = bw;
		txwi->TxPktId = pkt_id;

		txwi->AMPDU = ampdu;
		txwi->BAWinSize = basize;
		txwi->MIMOps = mimops;
		txwi->MpduDensity = mpdu_density;

		txwi->Sounding = sounding;
		txwi->iTxBF = iTxBf;
		txwi->eTxBF = eTxBf;
		txwi->NDPSndRate = ndp_rate;
		txwi->NDPSndBW = ndp_bw;
		txwi->TXBF_PT_SCA = (eTxBf | iTxBf) ? true : false;
	}
}


VOID RTMPWriteTxWI_Cache(struct rtmp_adapter *pAd, struct mt7612u_txwi *txwi, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
	HTTRANSMIT_SETTING tmpTransmit;
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	u8 pkt_id;
	u8 bw, mcs, stbc, phy_mode, sgi, ldpc;
	u8 ampdu, basize = 0, mimops, mpdu_density = 0;
	u8 sounding, iTxBf, eTxBf, ndp_rate, ndp_bw;
	u8 tx_stream_mode = 0;


	/* If CCK or OFDM, BW must be 20*/
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	pMacEntry->LastTxRate = pTransmit->word;

	if (MT_REV_GTE(pAd, MT76x2, REV_MT76x2E4))
		tx_stream_mode = 0x13;
	else if (MT_REV_ET(pAd, MT76x2, REV_MT76x2E3))
		tx_stream_mode = (pTransmit->field.MODE <= MODE_OFDM) ? 0x93 : 0x0;

	ampdu = ((pMacEntry->NoBADataCountDown == 0) ? true: false);
	if(pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
		ampdu = false;

	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ?
			(BW_20) : (pTransmit->field.BW);

	mimops = 0;
	if (pAd->CommonCfg.bMIMOPSEnable) {
		/* MIMO Power Save Mode*/
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) &&
		    (pTransmit->field.MCS > 7))
			mimops = 1;
		else if (pMacEntry->MmpsMode == MMPS_STATIC) {
			if ((pTransmit->field.MODE >= MODE_HTMIX) &&
			    (pTransmit->field.MCS > 7)) {
				mcs = 7;
				mimops = 0;
			}
		}
	}

	if(ampdu && pMacEntry) {
		/*
 		 * Under HT20, 2x2 chipset, OPEN, and with some atero chipsets
 		 * reduce BASize to 7 to add one bulk A-MPDU during one TXOP
 		 * to improve throughput
 		 */
		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) &&
		    (pMacEntry->bIAmBadAtheros)) {
			mpdu_density = 7;
			if ((pAd->Antenna.field.TxPath == 2) &&
			    (pMacEntry->WepStatus == Ndis802_11EncryptionDisabled)) {
				basize = 7;
			}
		}

	}

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0) {
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

	sounding = eTxBf = iTxBf = ndp_bw = ndp_rate = 0;
	if (pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING) {
		sounding = 1;
		stbc = false;
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): sending normal sounding, eTxBF=%d\n",
					__FUNCTION__, pTransmit->field.eTxBF));
	} else if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP) {
		stbc = false;
		if (pTxBlk->TxNDPSndgMcs>=16)
			ndp_rate = 2;
		else if (pTxBlk->TxNDPSndgMcs>=8)
			ndp_rate = 1;
		else
			ndp_rate = 0;
		ndp_bw = pTransmit->field.BW;

		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): NDP sounding, eTxBF=%d, ndp_rate = %d\n",
					__FUNCTION__, eTxBf, ndp_rate));
	} else {
			eTxBf = pTransmit->field.eTxBF;

		iTxBf = pTransmit->field.iTxBF;
	}

	if (eTxBf || iTxBf)
		stbc = false; //Force STBC = false when TxBf is enabled

	if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE) {
		mcs = 0;
		ampdu = false;
	}



		/* set PID for TxRateSwitching*/
		pkt_id = mcs;

	{
		if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
			txwi->txop = IFS_BACKOFF; // Reserve larger TXOP to prevent sounding packet from collision
		else
		txwi->txop = IFS_HTTXOP;
		txwi->BW = bw;
		txwi->ShortGI = sgi;
		txwi->STBC = stbc;
		txwi->LDPC = ldpc;
		txwi->TxStreamMode = tx_stream_mode;
		txwi->MCS = mcs;
		txwi->PHYMODE = phy_mode;
		txwi->TxPktId = pkt_id;
		txwi->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == true)
			txwi->ACK = 0;
#endif /* WFA_VHT_PF */

		txwi->AMPDU = ampdu;
		if (basize)
			txwi->BAWinSize = basize;
		txwi->MIMOps = mimops;
		if (mpdu_density)
			txwi->MpduDensity = mpdu_density;

		txwi->Sounding = sounding;
		txwi->eTxBF = eTxBf;
		txwi->iTxBF = iTxBf;
		txwi->NDPSndRate = ndp_rate;
		txwi->NDPSndBW = ndp_bw;
		txwi->TXBF_PT_SCA = (eTxBf | iTxBf) ? true : false;

	}

}


INT get_pkt_rssi_by_rxwi(struct rtmp_adapter *pAd, struct mt7612u_rxwi *rxwi, INT size, CHAR *rssi)
{
	INT status = 0;

	status = rlt_get_rxwi_rssi(rxwi, size, rssi);

	return status;
}


INT get_pkt_snr_by_rxwi(struct rtmp_adapter *pAd, struct mt7612u_rxwi *rxwi, INT size, u8 *snr)
{
	INT status = 0;

	status = rlt_get_rxwi_snr(pAd, rxwi, size, snr);

	return status;
}


INT get_pkt_phymode_by_rxwi(struct rtmp_adapter *pAd, struct mt7612u_rxwi *rxwi)
{
	INT status = 0;

	status = rlt_get_rxwi_phymode(rxwi);

	return status;

}

INT rtmp_mac_set_band(struct rtmp_adapter *pAd, int  band)
{
	uint32_t val, band_cfg;

	band_cfg = mt76u_reg_read(pAd, TX_BAND_CFG);
	val = band_cfg & (~0x6);
	switch (band) {
		case BAND_5G:
			val |= 0x02;
			break;
		case BAND_24G:
		default:
			val |= 0x4;
			break;
	}

	if (val != band_cfg)
		mt7612u_write32(pAd, TX_BAND_CFG, val);

	return true;
}


void mt7612u_mac_set_ctrlch(struct rtmp_adapter *pAd, u8 extch)
{
	uint32_t val, band_cfg;

	band_cfg = mt76u_reg_read(pAd, TX_BAND_CFG);
	val = band_cfg & (~0x1);
	switch (extch) {
		case EXTCHA_ABOVE:
			val &= (~0x1);
			break;
		case EXTCHA_BELOW:
			val |= (0x1);
			break;
		case EXTCHA_NONE:
			val &= (~0x1);
			break;
	}

	if (val != band_cfg)
		mt7612u_write32(pAd, TX_BAND_CFG, val);
}


INT rtmp_mac_set_mmps(struct rtmp_adapter *pAd, INT ReduceCorePower)
{
	uint32_t mac_val, org_val;

	org_val = mt76u_reg_read(pAd, 0x1210);
	mac_val = org_val;
	if (ReduceCorePower)
		mac_val |= 0x09;
	else
		mac_val &= ~0x09;

	if (mac_val != org_val)
		mt7612u_write32(pAd, 0x1210, mac_val);

	return true;
}


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(struct rtmp_adapter *pAd)
{
	uint32_t  Offset;
	BCN_TIME_CFG_STRUC csr;

	Offset = (pAd->TbttTickCount) % (BCN_TBTT_OFFSET);
	pAd->TbttTickCount++;

	/*
		The updated BeaconInterval Value will affect Beacon Interval after two TBTT
		beacasue the original BeaconInterval had been loaded into next TBTT_TIMER
	*/
	if (Offset == (BCN_TBTT_OFFSET-2)) {
		csr.word = mt76u_reg_read(pAd, BCN_TIME_CFG);

		/* ASIC register in units of 1/16 TU = 64us*/
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod << 4) - 1 ;
		mt7612u_write32(pAd, BCN_TIME_CFG, csr.word);
	} else if (Offset == (BCN_TBTT_OFFSET-1)) {
		csr.word = mt76u_reg_read(pAd, BCN_TIME_CFG);
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod) << 4;
		mt7612u_write32(pAd, BCN_TIME_CFG, csr.word);
	}
}


VOID rtmp_mac_bcn_buf_init(IN struct rtmp_adapter *pAd)
{
	int idx, tb_size;
	struct rtmp_chip_cap *pChipCap = &pAd->chipCap;


	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++)
		pAd->BeaconOffset[idx] = pChipCap->BcnBase[idx];

	DBGPRINT(RT_DEBUG_TRACE, ("< Beacon Information: >\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tFlgIsSupSpecBcnBuf = %s\n", pChipCap->FlgIsSupSpecBcnBuf ? "true" : "false"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwNum = %d\n", pChipCap->BcnMaxHwNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxNum = %d\n", pChipCap->BcnMaxNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwSize = 0x%x\n", pChipCap->BcnMaxHwSize));
	DBGPRINT(RT_DEBUG_TRACE, ("\tWcidHwRsvNum = %d\n", pChipCap->WcidHwRsvNum));
	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++) {
		DBGPRINT(RT_DEBUG_TRACE, ("\t\tBcnBase[%d] = 0x%x, pAd->BeaconOffset[%d]=0x%x\n",
					idx, pChipCap->BcnBase[idx], idx, pAd->BeaconOffset[idx]));
	}

	{
		struct rtmp_reg_pair bcn_mac_reg_tb[] = {
			{RLT_BCN_OFFSET0, 0x18100800},
			{RLT_BCN_OFFSET1, 0x38302820},
			{RLT_BCN_OFFSET2, 0x58504840},
			{RLT_BCN_OFFSET3, 0x78706860},
		};

		for (idx = 0; idx < ARRAY_SIZE(bcn_mac_reg_tb); idx ++) {
			mt7612u_write32(pAd,
				        (unsigned short)bcn_mac_reg_tb[idx].Register,
				        bcn_mac_reg_tb[idx].Value);
		}
	}
}


INT rtmp_mac_pbf_init(struct rtmp_adapter *pAd)
{
	INT idx;
	struct rtmp_reg_pair *pbf_regs = NULL;

	struct rtmp_reg_pair rlt_pbf_regs[]={
		{TX_MAX_PCNT,		0xefef3f1f},
		{RX_MAX_PCNT,		0xfebf},
	};

	pbf_regs = &rlt_pbf_regs[0];

	if ((pbf_regs != NULL) && (ARRAY_SIZE(rlt_pbf_regs) > 0)) {
		for (idx = 0; idx < ARRAY_SIZE(rlt_pbf_regs); idx++) {
			mt7612u_write32(pAd, pbf_regs->Register,
					pbf_regs->Value);
			pbf_regs++;
		}
	}
	return true;
}


/*
	ASIC register initialization sets
*/
struct rtmp_reg_pair MACRegTable[] = {
	{ LEGACY_BASIC_RATE,	0x0000013f },	/*  Basic rate set bitmap*/
	{ HT_BASIC_RATE,	0x00008003 },	/* Basic HT rate set , 20M, MCS=3, MM. Format is the same as in TXWI.*/
	{ MAC_SYS_CTRL,		0x00 }, 	/* 0x1004, , default Disable RX*/
	{ RX_FILTR_CFG,		0x17f97 }, 	/*0x1400  , RX filter control,  */
	{ BKOFF_SLOT_CFG,	0x209 }, 	/* default set short slot time, CC_DELAY_TIME should be 2	 */
	{ TX_SW_CFG1,		0x80606 }, 	/* Gary,2006-08-23 */
	{ TX_LINK_CFG,		0x1020 },	/* Gary,2006-08-23 */
	/*{TX_TIMEOUT_CFG,	0x00182090 },	 CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT*/
	{ TX_TIMEOUT_CFG,	0x000a2090 },	/* CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT , Modify for 2860E ,2007-08-01*/

	// TODO: shiang-usw, why MT7601 don't need to set this register??
	{ LED_CFG,		0x7f031e46 }, 	/* Gary, 2006-08-23*/

	/*{TX_RTY_CFG,		0x6bb80408},	 Jan, 2006/11/16*/
/* WMM_ACM_SUPPORT*/
/*	{TX_RTY_CFG,		0x6bb80101},	 sample*/
	{ TX_RTY_CFG,		0x47d01f0f},	/* Jan, 2006/11/16, Set TxWI->ACK =0 in Probe Rsp Modify for 2860E ,2007-08-03*/

	{ AUTO_RSP_CFG,		0x00000013},	/* Initial Auto_Responder, because QA will turn off Auto-Responder*/
	{ CCK_PROT_CFG,		0x05740003 /* 0x01740003*/ },	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */
	{ OFDM_PROT_CFG,	0x05740003 /* 0x01740003*/ },	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */

	{ MM40_PROT_CFG,	0x3F44084 },	/* Initial Auto_Responder, because QA will turn off Auto-Responder*/
	// TODO: shiang-usw, why MT7601 don't need to set this register??
	{ WPDMA_GLO_CFG,	0x00000030 },

	{ GF20_PROT_CFG,	0x01744004 },    /* set 19:18 --> Short NAV for MIMO PS*/
	{ GF40_PROT_CFG,	0x03F44084 },
	{ MM20_PROT_CFG,	0x01744004 },


	{ TXOP_CTRL_CFG,	0x0000583f,	/*0x0000243f*/ /*0x000024bf*/},	/*Extension channel backoff.*/
	{ TX_RTS_CFG,		0x00092b20 },

	{ EXP_ACK_TIME,		0x002400ca },	/* default value */
	{ TXOP_HLDR_ET, 	0x00000002 },

	/* Jerry comments 2008/01/16: we use SIFS = 10us in CCK defaultly, but it seems that 10us
		is too small for INTEL 2200bg card, so in MBSS mode, the delta time between beacon0
		and beacon1 is SIFS (10us), so if INTEL 2200bg card connects to BSS0, the ping
		will always lost. So we change the SIFS of CCK from 10us to 16us. */
	{ XIFS_TIME_CFG,	0x33a41010 },
};

#ifdef CONFIG_AP_SUPPORT
struct rtmp_reg_pair APMACRegTable[] = {
	{ WMM_AIFSN_CFG,	0x00001173 },
	{ WMM_CWMIN_CFG,	0x00002344 },
	{ WMM_CWMAX_CFG,	0x000034a6 },
	{ WMM_TXOP0_CFG,	0x00100020 },
	{ WMM_TXOP1_CFG,	0x002F0038 },
	{ TBTT_SYNC_CFG,	0x00012000 },
};
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
struct rtmp_reg_pair STAMACRegTable[] = {
	{ WMM_AIFSN_CFG,	0x00002273 },
	{ WMM_CWMIN_CFG,	0x00002344 },
	{ WMM_CWMAX_CFG,	0x000034aa },
};
#endif /* CONFIG_STA_SUPPORT */

INT rtmp_mac_init(struct rtmp_adapter *pAd)
{
	INT idx;

	for (idx = 0; idx < ARRAY_SIZE(MACRegTable); idx++)
		mt7612u_write32(pAd, MACRegTable[idx].Register,
				MACRegTable[idx].Value);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (idx = 0; idx < ARRAY_SIZE(APMACRegTable); idx++)
			mt7612u_write32(pAd,
				APMACRegTable[idx].Register,
				APMACRegTable[idx].Value);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (idx = 0; idx < ARRAY_SIZE(STAMACRegTable); idx++)
			mt7612u_write32(pAd,
				STAMACRegTable[idx].Register,
				STAMACRegTable[idx].Value);
	}
#endif /* CONFIG_STA_SUPPORT */

	rtmp_mac_pbf_init(pAd);

	mt76x2_init_mac_cr(pAd);

	return true;
}

