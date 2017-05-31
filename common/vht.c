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


struct vht_ch_layout{
	u8 ch_low_bnd;
	u8 ch_up_bnd;
	u8 cent_freq_idx;
};

static struct vht_ch_layout vht_ch_80M[]={
	{36, 48, 42},
	{52, 64, 58},
	{100,112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{0, 0 ,0},
};




VOID dump_vht_cap(struct rtmp_adapter *pAd, VHT_CAP_IE *vht_ie)
{
	VHT_CAP_INFO *vht_cap = &vht_ie->vht_cap;
	VHT_MCS_SET *vht_mcs = &vht_ie->mcs_set;

	DBGPRINT(RT_DEBUG_OFF, ("Dump VHT_CAP IE\n"));
	hex_dump("VHT CAP IE Raw Data", (u8 *)vht_ie, sizeof(VHT_CAP_IE));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Capabilities Info Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tMaximum MPDU Length=%d\n", vht_cap->max_mpdu_len));
	DBGPRINT(RT_DEBUG_OFF, ("\tSupported Channel Width=%d\n", vht_cap->ch_width));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxLDPC=%d\n", vht_cap->rx_ldpc));
	DBGPRINT(RT_DEBUG_OFF, ("\tShortGI_80M=%d\n", vht_cap->sgi_80M));
	DBGPRINT(RT_DEBUG_OFF, ("\tShortGI_160M=%d\n", vht_cap->sgi_160M));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxSTBC=%d\n", vht_cap->tx_stbc));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxSTBC=%d\n", vht_cap->rx_stbc));
	DBGPRINT(RT_DEBUG_OFF, ("\tSU BeamformerCap=%d\n", vht_cap->bfer_cap_su));
	DBGPRINT(RT_DEBUG_OFF, ("\tSU BeamformeeCap=%d\n", vht_cap->bfee_cap_su));
	DBGPRINT(RT_DEBUG_OFF, ("\tCompressedSteeringNumOfBeamformerAnt=%d\n", vht_cap->cmp_st_num_bfer));
	DBGPRINT(RT_DEBUG_OFF, ("\tNumber of Sounding Dimensions=%d\n", vht_cap->num_snd_dimension));
	DBGPRINT(RT_DEBUG_OFF, ("\tMU BeamformerCap=%d\n", vht_cap->bfer_cap_mu));
	DBGPRINT(RT_DEBUG_OFF, ("\tMU BeamformeeCap=%d\n", vht_cap->bfee_cap_mu));
	DBGPRINT(RT_DEBUG_OFF, ("\tVHT TXOP PS=%d\n", vht_cap->vht_txop_ps));
	DBGPRINT(RT_DEBUG_OFF, ("\t+HTC-VHT Capable=%d\n", vht_cap->htc_vht_cap));
	DBGPRINT(RT_DEBUG_OFF, ("\tMaximum A-MPDU Length Exponent=%d\n", vht_cap->max_ampdu_exp));
	DBGPRINT(RT_DEBUG_OFF, ("\tVHT LinkAdaptation Capable=%d\n", vht_cap->vht_link_adapt));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Supported MCS Set Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tRx Highest SupDataRate=%d\n", vht_mcs->rx_high_rate));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_1SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss1));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_2SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss2));
	DBGPRINT(RT_DEBUG_OFF, ("\tTx Highest SupDataRate=%d\n", vht_mcs->tx_high_rate));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxMCS Map_1SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss1));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxMCS Map_2SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss2));
}


VOID dump_vht_op(struct rtmp_adapter *pAd, VHT_OP_IE *vht_ie)
{
	VHT_OP_INFO *vht_op = &vht_ie->vht_op_info;
	VHT_MCS_MAP *vht_mcs = &vht_ie->basic_mcs_set;

	DBGPRINT(RT_DEBUG_OFF, ("Dump VHT_OP IE\n"));
	hex_dump("VHT OP IE Raw Data", (u8 *)vht_ie, sizeof(VHT_OP_IE));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Operation Info Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelWidth=%d\n", vht_op->ch_width));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelCenterFrequency Seg 1=%d\n", vht_op->center_freq_1));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelCenterFrequency Seg 1=%d\n", vht_op->center_freq_2));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Basic MCS Set Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_1SS=%d\n", vht_mcs->mcs_ss1));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_2SS=%d\n", vht_mcs->mcs_ss2));
}


VOID trigger_vht_ndpa(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *entry)
{
	u8 *buf;
	VHT_NDPA_FRAME *vht_ndpa;
	struct rtmp_wifi_dev *wdev = entry->wdev;
	UINT frm_len, sta_cnt;
	SNDING_STA_INFO *sta_info;

	buf = kmalloc(MGMT_DMA_BUFFER_SIZE, GFP_ATOMIC);
	if (buf == NULL)
		return;

	memset(buf, 0, MGMT_DMA_BUFFER_SIZE);

	vht_ndpa = (VHT_NDPA_FRAME *)buf;
	frm_len = sizeof(VHT_NDPA_FRAME);
	vht_ndpa->fc.Type = FC_TYPE_CNTL;
	vht_ndpa->fc.SubType = SUBTYPE_VHT_NDPA;
	COPY_MAC_ADDR(vht_ndpa->ra, entry->Addr);
	COPY_MAC_ADDR(vht_ndpa->ta, wdev->if_addr);

	/* Currnetly we only support 1 STA for a VHT DNPA */
	sta_info = vht_ndpa->sta_info;
	for (sta_cnt = 0; sta_cnt < 1; sta_cnt++) {
		sta_info->aid12 = entry->Aid;
		sta_info->fb_type = SNDING_FB_SU;
		sta_info->nc_idx = 0;
		vht_ndpa->token.token_num = entry->snd_dialog_token;
		frm_len += sizeof(SNDING_STA_INFO);
		sta_info++;
		if (frm_len >= (MGMT_DMA_BUFFER_SIZE - sizeof(SNDING_STA_INFO))) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): len(%d) too large!cnt=%d\n",
						__FUNCTION__, frm_len, sta_cnt));
			break;
		}
	}
	if (entry->snd_dialog_token & 0xc0)
		entry->snd_dialog_token = 0;
	else
		entry->snd_dialog_token++;

	vht_ndpa->duration = pAd->CommonCfg.Dsifs +
						RTMPCalcDuration(pAd, pAd->CommonCfg.MlmeRate, frm_len);

	//DBGPRINT(RT_DEBUG_OFF, ("Send VHT NDPA Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
	//			PRINT_MAC(entry->Addr)));
	//hex_dump("VHT NDPA Frame", buf, frm_len);
	MiniportMMRequest(pAd, 0, buf, frm_len);
	kfree(buf);

}


/*
	Get BBP Channel Index by RF channel info
	return value: 0~3
*/
u8 vht_prim_ch_idx(u8 vht_cent_ch, u8 prim_ch)
{
	INT idx = 0;
	u8 bbp_idx = 0;

	if (vht_cent_ch == prim_ch)
		goto done;

	while (vht_ch_80M[idx].ch_up_bnd != 0) {
		if (vht_cent_ch == vht_ch_80M[idx].cent_freq_idx) {
			if (prim_ch == vht_ch_80M[idx].ch_up_bnd)
				bbp_idx = 3;
			else if (prim_ch == vht_ch_80M[idx].ch_low_bnd)
				bbp_idx = 0;
			else
				bbp_idx = prim_ch > vht_cent_ch ? 2 : 1;
			break;
		}
		idx++;
	}

done:
	DBGPRINT(RT_DEBUG_INFO, ("%s():(VhtCentCh=%d, PrimCh=%d) =>BbpChIdx=%d\n",
				__FUNCTION__, vht_cent_ch, prim_ch, bbp_idx));
	return bbp_idx;
}


/*
	Currently we only consider about VHT 80MHz!
*/
u8 vht_cent_ch_freq(struct rtmp_adapter *pAd, u8 prim_ch)
{
	INT idx = 0;


	if (pAd->CommonCfg.vht_bw < VHT_BW_80 || prim_ch < 36) {
		pAd->CommonCfg.vht_cent_ch = 0;
		pAd->CommonCfg.vht_cent_ch2 = 0;
		return prim_ch;
	}

	while (vht_ch_80M[idx].ch_up_bnd != 0) {
		if (prim_ch >= vht_ch_80M[idx].ch_low_bnd &&
		    prim_ch <= vht_ch_80M[idx].ch_up_bnd) {
			pAd->CommonCfg.vht_cent_ch = vht_ch_80M[idx].cent_freq_idx;
			return vht_ch_80M[idx].cent_freq_idx;
		}
		idx++;
	}

	return prim_ch;
}


INT vht_mode_adjust(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, VHT_CAP_IE *cap, VHT_OP_IE *op)
{
	pEntry->MaxHTPhyMode.field.MODE = MODE_VHT;
	pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
	pAd->MacTab.fAnyStationNonGF = true;

	if (op->vht_op_info.ch_width >= 1 &&
	    pEntry->MaxHTPhyMode.field.BW == BW_40) {
		pEntry->MaxHTPhyMode.field.BW= BW_80;
		pEntry->MaxHTPhyMode.field.ShortGI = (cap->vht_cap.sgi_80M);
		pEntry->MaxHTPhyMode.field.STBC = (cap->vht_cap.rx_stbc > 1 ? 1 : 0);
	}

	return true;
}


INT get_vht_op_ch_width(struct rtmp_adapter *pAd)
{

	return true;
}


/********************************************************************
	Procedures for 802.11 AC Information elements
********************************************************************/
/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_quiet_channel(struct rtmp_adapter *pAd, u8 *buf)
{
	INT len = 0;


	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_ext_bss_load(struct rtmp_adapter *pAd, u8 *buf)
{
	INT len = 0;


	return len;
}




/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_ext_pwr_constraint(struct rtmp_adapter *pAd, u8 *buf)
{
	INT len = 0;


	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_vht_txpwr_envelope(struct rtmp_adapter *pAd, u8 *buf)
{
	INT len = 0, pwr_cnt;
	VHT_TXPWR_ENV_IE txpwr_env;

	memset(&txpwr_env, 0, sizeof(txpwr_env));

	if (pAd->CommonCfg.vht_bw == VHT_BW_80) {
		pwr_cnt = 2;
	} else {
		if (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 1)
			pwr_cnt = 1;
		else
			pwr_cnt = 0;
	}
	txpwr_env.tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
	txpwr_env.tx_pwr_info.max_tx_pwr_interpretation = TX_PWR_INTERPRET_EIRP;

// TODO: fixme, we need the real tx_pwr value for each port.
	for (len = 0; len < pwr_cnt; len++)
		txpwr_env.tx_pwr_bw[len] = 15;

	len = 2 + pwr_cnt;
	memmove(buf, &txpwr_env, len);

	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocResp, ProbResp frames
*/
INT build_vht_op_ie(struct rtmp_adapter *pAd, u8 *buf)
{
	VHT_OP_IE vht_op;
	u8 cent_ch;
#ifdef RT_BIG_ENDIAN
	uint16_t tmp;
#endif /* RT_BIG_ENDIAN */

	memset((u8 *)&vht_op, 0, sizeof(VHT_OP_IE));
	vht_op.vht_op_info.ch_width = (pAd->CommonCfg.vht_bw == VHT_BW_80 ? 1: 0);

#ifdef CONFIG_AP_SUPPORT
	if (pAd->CommonCfg.Channel > 14 &&
		(pAd->CommonCfg.bIEEE80211H == 1) &&
		(pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		cent_ch = vht_cent_ch_freq(pAd, pAd->Dot11_H.org_ch);
	else
#endif /* CONFIG_AP_SUPPORT */
		cent_ch = vht_cent_ch_freq(pAd, pAd->CommonCfg.Channel);

	switch (vht_op.vht_op_info.ch_width) {
	case 0:
		vht_op.vht_op_info.center_freq_1 = 0;
		vht_op.vht_op_info.center_freq_2 = 0;
		break;
	case 1:
	case 2:
		vht_op.vht_op_info.center_freq_1 = cent_ch;
		vht_op.vht_op_info.center_freq_2 = 0;
		break;
	case 3:
		vht_op.vht_op_info.center_freq_1 = cent_ch;
		vht_op.vht_op_info.center_freq_2 = pAd->CommonCfg.vht_cent_ch2;
		break;
	}

	vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;
	switch  (pAd->CommonCfg.RxStream) {
	case 2:
		if (IS_MT76x2(pAd)) {
			vht_op.basic_mcs_set.mcs_ss2 = (((pAd->CommonCfg.vht_bw == VHT_BW_2040)
				&& (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20)) ? VHT_MCS_CAP_8 : VHT_MCS_CAP_9);
		} else
			vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_7;
	case 1:
		if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
			vht_op.basic_mcs_set.mcs_ss1 = (((pAd->CommonCfg.vht_bw == VHT_BW_2040)
				&& (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20)) ? VHT_MCS_CAP_8 : VHT_MCS_CAP_9);
		else
			vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_7;
		break;
	}

#ifdef RT_BIG_ENDIAN
	//SWAP16((uint16_t)vht_op.basic_mcs_set);
	memcpy(&tmp,&vht_op.basic_mcs_set, 2);
	tmp=SWAP16(tmp);
	memcpy(&vht_op.basic_mcs_set,&tmp, 2);
#endif /* RT_BIG_ENDIAN */
	memmove((u8 *)buf, (u8 *)&vht_op, sizeof(VHT_OP_IE));

	return sizeof(VHT_OP_IE);
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocReq, (Re)AssocResp, ProbReq/Resp frames
*/
INT build_vht_cap_ie(struct rtmp_adapter *pAd, u8 *buf)
{
	VHT_CAP_IE vht_cap_ie;
	INT rx_nss, tx_nss, mcs_cap;
#ifdef RT_BIG_ENDIAN
	uint32_t tmp_1;
	uint64_t tmp_2;
#endif /*RT_BIG_ENDIAN*/

	memset((u8 *)&vht_cap_ie,  0, sizeof(VHT_CAP_IE));
	vht_cap_ie.vht_cap.max_mpdu_len = 0; // TODO: Ask Jerry about hardware limitation.
	vht_cap_ie.vht_cap.ch_width = 0; /* not support 160 or 80 + 80 MHz */

	if (pAd->CommonCfg.vht_ldpc && (pAd->chipCap.phy_caps & fPHY_CAP_LDPC))
		vht_cap_ie.vht_cap.rx_ldpc = 1;
	else
		vht_cap_ie.vht_cap.rx_ldpc = 0;

	vht_cap_ie.vht_cap.sgi_80M = pAd->CommonCfg.vht_sgi_80;
	vht_cap_ie.vht_cap.htc_vht_cap = 1;
	vht_cap_ie.vht_cap.max_ampdu_exp = 3; // TODO: Ask Jerry about the hardware limitation, currently set as 64K

	vht_cap_ie.vht_cap.tx_stbc = 0;
	vht_cap_ie.vht_cap.rx_stbc = 0;
	if (pAd->CommonCfg.vht_stbc) {
		if (pAd->CommonCfg.TxStream >= 2)
			vht_cap_ie.vht_cap.tx_stbc = 1;
		else
			vht_cap_ie.vht_cap.tx_stbc = 0;

		if (pAd->CommonCfg.RxStream >= 1)
			vht_cap_ie.vht_cap.rx_stbc = 1; // TODO: is it depends on the number of our antennas?
		else
			vht_cap_ie.vht_cap.rx_stbc = 0;
	}

	vht_cap_ie.vht_cap.tx_ant_consistency = 1;
	vht_cap_ie.vht_cap.rx_ant_consistency = 1;

	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss8 = VHT_MCS_CAP_NA;

	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss8 = VHT_MCS_CAP_NA;

	mcs_cap = pAd->chipCap.max_vht_mcs;

	rx_nss = pAd->CommonCfg.RxStream;
	tx_nss = pAd->CommonCfg.TxStream;
#ifdef WFA_VHT_PF
	if ((pAd->CommonCfg.vht_nss_cap > 0) &&
		(pAd->CommonCfg.vht_nss_cap < pAd->CommonCfg.RxStream))
		rx_nss = pAd->CommonCfg.vht_nss_cap;

	if ((pAd->CommonCfg.vht_nss_cap > 0) &&
		(pAd->CommonCfg.vht_nss_cap < pAd->CommonCfg.TxStream))
		tx_nss = pAd->CommonCfg.vht_nss_cap;

	if (pAd->CommonCfg.vht_mcs_cap <pAd->chipCap.max_vht_mcs)
		mcs_cap = pAd->CommonCfg.vht_mcs_cap;
#endif /* WFA_VHT_PF */

	switch  (rx_nss) {
	case 1:
		vht_cap_ie.mcs_set.rx_high_rate = 292;
		vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = mcs_cap;
		break;
	case 2:
		if (mcs_cap == VHT_MCS_CAP_9)
			vht_cap_ie.mcs_set.rx_high_rate = 780;
		else
			vht_cap_ie.mcs_set.rx_high_rate = 585;

		vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = mcs_cap;
		vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = mcs_cap;
		break;
	default:
		vht_cap_ie.mcs_set.rx_high_rate = 0;
		break;
	}

	switch (tx_nss) {
	case 1:
		vht_cap_ie.mcs_set.tx_high_rate = 292;
		vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = mcs_cap;
		break;
	case 2:
		if (mcs_cap == VHT_MCS_CAP_9)
			vht_cap_ie.mcs_set.tx_high_rate = 780;
		else
			vht_cap_ie.mcs_set.tx_high_rate = 585;

		vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = mcs_cap;
		vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = mcs_cap;
		break;
	default:
		vht_cap_ie.mcs_set.tx_high_rate = 0;
		break;
	}

#ifdef RT_BIG_ENDIAN
	memcpy(&tmp_1,&vht_cap_ie.vht_cap, 4);
	tmp_1 = SWAP32(tmp_1);
	memcpy(&vht_cap_ie.vht_cap,&tmp_1, 4);

	memcpy(&tmp_2,&vht_cap_ie.mcs_set, 8);
	tmp_2=SWAP64(tmp_2);
	memcpy(&vht_cap_ie.mcs_set,&tmp_2, 8);

	//hex_dump("&vht_cap_ie", &vht_cap_ie,  sizeof(VHT_CAP_IE));
	//SWAP32((uint32_t)vht_cap_ie.vht_cap);
	//SWAP32((uint32_t)vht_cap_ie.mcs_set);
#endif /* RT_BIG_ENDIAN */


	if ((pAd->chipCap.FlgHwTxBfCap) &&
	    (pAd->BeaconSndDimensionFlag ==0)) {
		vht_cap_ie.vht_cap.num_snd_dimension = pAd->CommonCfg.vht_cap_ie.vht_cap.num_snd_dimension;
   		vht_cap_ie.vht_cap.cmp_st_num_bfer= pAd->CommonCfg.vht_cap_ie.vht_cap.cmp_st_num_bfer;
		vht_cap_ie.vht_cap.bfee_cap_su=pAd->CommonCfg.vht_cap_ie.vht_cap.bfee_cap_su;
		vht_cap_ie.vht_cap.bfer_cap_su=pAd->CommonCfg.vht_cap_ie.vht_cap.bfer_cap_su;
	}
        pAd->BeaconSndDimensionFlag =0;

	memmove(buf, (u8 *)&vht_cap_ie, sizeof(VHT_CAP_IE));

	return sizeof(VHT_CAP_IE);
}


INT build_vht_ies(struct rtmp_adapter *pAd, u8 *buf, u8 frm)
{
	INT len = 0;
	EID_STRUCT eid_hdr;


	eid_hdr.Eid = IE_VHT_CAP;
	eid_hdr.Len = sizeof(VHT_CAP_IE);
	memmove(buf, (u8 *)&eid_hdr, 2);
	len = 2;

	len += build_vht_cap_ie(pAd, (u8 *)(buf + len));
	if (frm == SUBTYPE_BEACON || frm == SUBTYPE_PROBE_RSP ||
	    frm == SUBTYPE_ASSOC_RSP || frm == SUBTYPE_REASSOC_RSP) {
		eid_hdr.Eid = IE_VHT_OP;
		eid_hdr.Len = sizeof(VHT_OP_IE);
		memmove((u8 *)(buf + len), (u8 *)&eid_hdr, 2);
		len +=2;

		len += build_vht_op_ie(pAd, (u8 *)(buf + len));
	}

	return len;
}

bool vht80_channel_group( struct rtmp_adapter *pAd, u8 channel)
{
	INT idx = 0;

	if (channel <= 14)
		return false;

	while (vht_ch_80M[idx].ch_up_bnd != 0) {
		if (channel >= vht_ch_80M[idx].ch_low_bnd &&
		    channel <= vht_ch_80M[idx].ch_up_bnd) {
			if ( (pAd->CommonCfg.RDDurRegion == JAP ||
				pAd->CommonCfg.RDDurRegion == JAP_W53 ||
				pAd->CommonCfg.RDDurRegion == JAP_W56) &&
				vht_ch_80M[idx].cent_freq_idx == 138) {
				idx++;
				continue;
			}

			return true;
		}
		idx++;
	}

	return false;
}

