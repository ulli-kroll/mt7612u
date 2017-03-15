#include "rt_config.h"

#ifdef RTMP_EFUSE_SUPPORT
#include "eeprom/mt76x2_e2p.h"
#endif /* RTMP_EFUSE_SUPPORT */

#define MT7662_EFUSE_CTRL	0x0024
static RTMP_REG_PAIR mt76x2_mac_cr_table[] = {
	{PBF_SYS_CTRL, 0x80c00},
	{RLT_PBF_CFG, 0x1efebcff},
	{FCE_PSE_CTRL, 0x1},
	{MAC_SYS_CTRL, 0x0C},
	{MAX_LEN_CFG, 0x003E3FFF},
	{AMPDU_MAX_LEN_20M1S_MCS0_7, 0xAAA99887},
	{AMPDU_MAX_LEN_20M1S_MCS8_9, 0xAA},
	{XIFS_TIME_CFG, 0x33A40D0A},
	{BKOFF_SLOT_CFG, 0x209},
	{TBTT_SYNC_CFG, 0x422010},
	{PWR_PIN_CFG, 0x0},
	{0x1238, 0x001700C8},
	{TX_SW_CFG0, 0x00101001},
	{TX_SW_CFG1, 0x00010000},
	{TX_SW_CFG2, 0x0},
	{TXOP_CTRL_CFG, 0x0000583F},
	{TX_RTS_CFG, 0x00092B20},
	{TX_TIMEOUT_CFG, 0x000A2290},
	{TX_RTY_CFG, 0x47D01F0F},
	{EXP_ACK_TIME, 0x002C00DC},
	{TX_PROT_CFG6, 0xE3F42004},
	{TX_PROT_CFG7, 0xE3F42084},
	{TX_PROT_CFG8, 0xE3F42104},
	{PIFS_TX_CFG, 0x00060FFF},
	{RX_FILTR_CFG, 0x00015F97},
	{LEGACY_BASIC_RATE, 0x0000017F},
	{HT_BASIC_RATE, 0x00008003},
	{0x150C, 0x00000002}, /* Enable TX length > 4095 bytes */
	{0x1608, 0x00000002},
	{0xa44,	0x0},
	{HEADER_TRANS_CTRL_REG, 0x0},
	{TSO_CTRL, 0x0},
	{AUX_CLK_CFG, 0x0},
	{DACCLK_EN_DLY_CFG, 0x0}, /* MAC dynamic control TX 960MHZ */
	{TX_ALC_CFG_4, 0x00000000},
	{TX_ALC_VGA3, 0x0},
	{TX_PWR_CFG_0, 0x3A3A3A3A},
	{TX_PWR_CFG_1, 0x3A3A3A3A},
	{TX_PWR_CFG_2, 0x3A3A3A3A},
	{TX_PWR_CFG_3, 0x3A3A3A3A},
	{TX_PWR_CFG_4, 0x3A3A3A3A},
	{TX_PWR_CFG_7, 0x3A3A3A3A},
	{TX_PWR_CFG_8, 0x3A},
	{TX_PWR_CFG_9, 0x3A},
	{MT7662_EFUSE_CTRL, 0xD000},
	{PER_PORT_PAUSE_ENABLE_CONTROL1, 0xA},
	{0x824, 0x60401c18}, //change the FCE threshold from 0x60402c28 to 0x60401c18
	{TX_TXBF_CFG_0,		0x4002FC21},	/* Force MCS2 for sounding response*/
	{TX_TXBF_CFG_1,		0xFE23727F},
	{TX_TXBF_CFG_2,		0xFFFFFFFF},	/* The explicit TxBF feedback is applied only when the value of (local TSF timer) -
	                                                               (TSF timestamp of the feedback frame) is greater then or equal to 0xFFFFFFFF */
	{0x210, 0x94ff0000},
	{0x1478, 0x00000004},
	{0x1384, 0x00001818},
	{0x1358, 0xEDCBA980},
};
static UCHAR mt76x2_mac_cr_nums = (sizeof(mt76x2_mac_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_g_band_cr_table[] = {
	{BB_PA_MODE_CFG0, 0x010055FF},
	{BB_PA_MODE_CFG1, 0x00550055},
	{RF_PA_MODE_CFG0, 0x010055FF},
	{RF_PA_MODE_CFG1, 0x00550055},
	{TX_ALC_CFG_2, 0x35160A00},
	{TX_ALC_CFG_3, 0x35160A06},
	{TX_ALC_CFG_4, 0x00000606},
	{0x1648, 0x00000000},
};
UCHAR mt76x2_mac_g_band_cr_nums = (sizeof(mt76x2_mac_g_band_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_g_band_internal_pa_cr_table[] = {
	{TX0_RF_GAIN_CORR, 0x0F3C3C3C},
	{TX1_RF_GAIN_CORR, 0x0F3C3C3C},
	{TX0_BB_GAIN_ATTEN, 0x00000606},
	{PAMODE_PWR_ADJ0, 0xF4000200},
	{PAMODE_PWR_ADJ1, 0xFA000200},
};
UCHAR mt76x2_mac_g_band_internal_pa_cr_nums = (sizeof(mt76x2_mac_g_band_internal_pa_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_g_band_external_pa_cr_table[] = {
	{TX0_RF_GAIN_CORR, 0x3C3C023C},
	{TX1_RF_GAIN_CORR, 0x3C3C023C},
	{TX0_BB_GAIN_ATTEN, 0x00001818},
	{PAMODE_PWR_ADJ0, 0x0000EC00},
	{PAMODE_PWR_ADJ1, 0x0000EC00},
};
UCHAR mt76x2_mac_g_band_external_pa_cr_nums = (sizeof(mt76x2_mac_g_band_external_pa_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_g_band_external_pa_low_temp_cr_table[] = {
	{TX0_RF_GAIN_CORR, 0x3C3C023C},
	{TX1_RF_GAIN_CORR, 0x3C3C023C},
	{TX0_BB_GAIN_ATTEN, 0x00001F1F},
};
UCHAR mt76x2_mac_g_band_external_pa_low_temp_cr_nums = (sizeof(mt76x2_mac_g_band_external_pa_low_temp_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_a_band_cr_table[] = {
	{BB_PA_MODE_CFG0, 0x0000FFFF},
	{BB_PA_MODE_CFG1, 0x00FF00FF},
	{RF_PA_MODE_CFG0, 0x0000FFFF},
	{RF_PA_MODE_CFG1, 0x00FF00FF},
	{TX_ALC_CFG_2, 0x1B0F0400},
	{TX_ALC_CFG_3, 0x1B0F0476},
	{TX_ALC_CFG_4, 0x00000000},
};
UCHAR mt76x2_mac_a_band_cr_nums = (sizeof(mt76x2_mac_a_band_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_a_band_internal_pa_cr_table[] = {
	{TX0_RF_GAIN_CORR, 0x383C023C},
	{TX1_RF_GAIN_CORR, 0x24282E28},
	{TX0_BB_GAIN_ATTEN, 0x00000000},
	{PAMODE_PWR_ADJ0, 0x00000000},
	{PAMODE_PWR_ADJ1, 0x00000000},
	{0x1648, 0x00000000},
};
UCHAR mt76x2_mac_a_band_internal_pa_cr_nums = (sizeof(mt76x2_mac_a_band_internal_pa_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_a_band_external_pa_cr_table[] = {
	{TX0_RF_GAIN_CORR, 0x3C3C023C},
	{TX1_RF_GAIN_CORR, 0x3C3C023C},
	{TX0_BB_GAIN_ATTEN, 0x00001818},
	{PAMODE_PWR_ADJ0, 0x04000000},
	{PAMODE_PWR_ADJ1, 0x04000000},
	{0x1648, 0x00830083},
};
UCHAR mt76x2_mac_a_band_external_pa_cr_nums = (sizeof(mt76x2_mac_a_band_external_pa_cr_table) / sizeof(RTMP_REG_PAIR));

RTMP_REG_PAIR mt76x2_mac_a_band_external_pa_low_temp_cr_table[] = {
	{TX0_RF_GAIN_CORR, 0x3C3C023C},
	{TX1_RF_GAIN_CORR, 0x3C3C023C},
	{TX0_BB_GAIN_ATTEN, 0x00001F1F},
};
UCHAR mt76x2_mac_a_band_external_pa_low_temp_cr_nums = (sizeof(mt76x2_mac_a_band_external_pa_low_temp_cr_table) / sizeof(RTMP_REG_PAIR));

struct RF_INDEX_OFFSET mt76x2_rf_index_offset[] = {
	{0, 0x0000, 0x033c},
	{1, 0x0000, 0x033c},
};


static VOID mt76x2_bbp_adjust(struct rtmp_adapter *pAd)
{
	static char *ext_str[]={"extNone", "extAbove", "", "extBelow"};
	UCHAR rf_bw, ext_ch;

	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch) == FALSE) {
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}

	if (WMODE_CAP(pAd->CommonCfg.PhyMode, WMODE_AC) &&
	    (pAd->CommonCfg.Channel > 14) &&
	    (rf_bw == BW_40) &&
	    (pAd->CommonCfg.vht_bw == VHT_BW_80) &&
	    (pAd->CommonCfg.vht_cent_ch != pAd->CommonCfg.CentralChannel)) {
		rf_bw = BW_80;
		pAd->CommonCfg.vht_cent_ch =
			vht_cent_ch_freq(pAd, pAd->CommonCfg.Channel);
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s():rf_bw=%d, ext_ch=%d, PrimCh=%d, HT-CentCh=%d, VHT-CentCh=%d\n",
				__FUNCTION__, rf_bw, ext_ch, pAd->CommonCfg.Channel,
				pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch));

	bbp_set_bw(pAd, rf_bw);

	/* TX/Rx : control channel setting */
	rtmp_mac_set_ctrlch(pAd, ext_ch);
	bbp_set_ctrlch(pAd, ext_ch);

	DBGPRINT(RT_DEBUG_TRACE, ("%s() : %s, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
					__FUNCTION__, ext_str[ext_ch],
					pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
					pAd->CommonCfg.Channel,
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
					pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
}

static int get_chl_grp(u8 channel)
{
	char chl_grp = A_BAND_GRP0_CHL;

	if (channel >= 184 && channel <= 196)
		chl_grp = A_BAND_GRP0_CHL;
	else if (channel >= 36 && channel <= 48)
		chl_grp = A_BAND_GRP1_CHL;
	else if (channel >= 52 && channel <= 64)
		chl_grp = A_BAND_GRP2_CHL;
	else if (channel >= 98 && channel <= 114)
		chl_grp = A_BAND_GRP3_CHL;
	else if (channel >= 116 && channel <= 144)
		chl_grp = A_BAND_GRP4_CHL;
	else if (channel >= 149 && channel <= 165)
		chl_grp = A_BAND_GRP5_CHL;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel (%d)\n", __FUNCTION__, channel));

	return chl_grp;
}

static int get_low_mid_hi_index(u8 channel)
{
	char index = G_BAND_LOW;

	if (channel <= 14) {
		if (channel >= 1 && channel <= 5)
			index = G_BAND_LOW;
		else if (channel >= 6 && channel <= 10)
			index = G_BAND_MID;
		else if (channel >= 11 && channel <= 14)
			index = G_BAND_HI;
		else
			DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel(%d)\n", __FUNCTION__, channel));
	} else {
		if (channel >= 184 && channel <= 188)
			index = A_BAND_LOW;
		else if (channel >= 192 && channel <= 196)
			index = A_BAND_HI;
		else if (channel >= 36 && channel <= 42)
			index = A_BAND_LOW;
		else if (channel >= 44 && channel <= 48)
			index = A_BAND_HI;
		else if (channel >= 52 && channel <= 56)
			index = A_BAND_LOW;
		else if (channel >= 58 && channel <= 64)
			index = A_BAND_HI;
		else if (channel >= 98 && channel <= 104)
			index = A_BAND_LOW;
		else if (channel >= 106 && channel <= 114)
			index = A_BAND_HI;
		else if (channel >= 116 && channel <= 128)
			index = A_BAND_LOW;
		else if (channel >= 130 && channel <= 144)
			index = A_BAND_HI;
		else if (channel >= 149 && channel <= 156)
			index = A_BAND_LOW;
		else if (channel >= 157 && channel <= 165)
			index = A_BAND_HI;
		else
			DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel(%d)\n", __FUNCTION__, channel));
	}

	return index;
}

void mt76x2_adjust_per_rate_pwr_delta(struct rtmp_adapter *ad, u8 channel, char delta_pwr)
{
	u32 value;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	unsigned int band;

	if (channel > 14)
		band = _A_BAND;
	else
		band = _G_BAND;

	value = mt7612u_read32(ad, TX_PWR_CFG_0);
	value &= ~TX_PWR_CCK_1_2_MASK;

	value |= TX_PWR_CCK_1_2(cap->tx_pwr_cck_1_2 + delta_pwr);

	value &= ~TX_PWR_CCK_5_11_MASK;
	value |= TX_PWR_CCK_5_11(cap->tx_pwr_cck_5_11 + delta_pwr);

	value &= ~TX_PWR_OFDM_6_9_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_OFDM_6_9(cap->tx_pwr_g_band_ofdm_6_9 + delta_pwr);
	else
		value |= TX_PWR_OFDM_6_9(cap->tx_pwr_a_band_ofdm_6_9 + delta_pwr);

	value &= ~TX_PWR_OFDM_12_18_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_OFDM_12_18(cap->tx_pwr_g_band_ofdm_12_18 + delta_pwr);
	else
		value |= TX_PWR_OFDM_12_18(cap->tx_pwr_a_band_ofdm_12_18 + delta_pwr);
	mt7612u_write32(ad, TX_PWR_CFG_0, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_1);
	value &= ~TX_PWR_OFDM_24_36_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_OFDM_24_36(cap->tx_pwr_g_band_ofdm_24_36 + delta_pwr);
	else
		value |= TX_PWR_OFDM_24_36(cap->tx_pwr_a_band_ofdm_24_36 + delta_pwr);

	value &= ~TX_PWR_OFDM_48_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_OFDM_48(cap->tx_pwr_g_band_ofdm_48_54 + delta_pwr);
	else
		value |= TX_PWR_OFDM_48(cap->tx_pwr_a_band_ofdm_48_54 + delta_pwr);

	value &= ~TX_PWR_HT_VHT_1SS_MCS_0_1_MASK;
	value |= TX_PWR_HT_VHT_1SS_MCS_0_1(cap->tx_pwr_ht_mcs_0_1 + delta_pwr);

	value &= ~TX_PWR_HT_VHT_1SS_MCS_2_3_MASK;
	value |= TX_PWR_HT_VHT_1SS_MCS_2_3(cap->tx_pwr_ht_mcs_2_3 + delta_pwr);
	mt7612u_write32(ad, TX_PWR_CFG_1, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_2);
	value &= ~TX_PWR_HT_VHT_1SS_MCS_4_5_MASK;
	value |= TX_PWR_HT_VHT_1SS_MCS_4_5(cap->tx_pwr_ht_mcs_4_5 + delta_pwr);

	value &= ~TX_PWR_HT_VHT_1SS_MCS_6_MASK;
	value |= TX_PWR_HT_VHT_1SS_MCS_6(cap->tx_pwr_ht_mcs_6_7 + delta_pwr);

	value &= ~TX_PWR_HT_MCS_8_9_VHT_2SS_0_1_MASK;
	value |= TX_PWR_HT_MCS_8_9_VHT_2SS_0_1(cap->tx_pwr_ht_mcs_8_9 + delta_pwr);

	value &= ~TX_PWR_HT_MCS_10_11_VHT_2SS_MCS_2_3_MASK;
	value |= TX_PWR_HT_MCS_10_11_VHT_2SS_MCS_2_3(cap->tx_pwr_ht_mcs_10_11 + delta_pwr);
	mt7612u_write32(ad, TX_PWR_CFG_2, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_3);
	value &= ~TX_PWR_HT_MCS_12_13_VHT_2SS_MCS_4_5_MASK;
	value |= TX_PWR_HT_MCS_12_13_VHT_2SS_MCS_4_5(cap->tx_pwr_ht_mcs_12_13 + delta_pwr);

	value &= ~TX_PWR_HT_MCS_14_VHT_2SS_MCS_6_MASK;
	value |= TX_PWR_HT_MCS_14_VHT_2SS_MCS_6(cap->tx_pwr_ht_mcs_14_15 + delta_pwr);

	value &= ~TX_PWR_HT_VHT_STBC_MCS_0_1_MASK;
	value |= TX_PWR_HT_VHT_STBC_MCS_0_1(cap->tx_pwr_ht_mcs_0_1 + delta_pwr);

	value &= ~TX_PWR_HT_VHT_STBC_MCS_2_3_MASK;
	value |= TX_PWR_HT_VHT_STBC_MCS_2_3(cap->tx_pwr_ht_mcs_2_3 + delta_pwr);
	mt7612u_write32(ad, TX_PWR_CFG_3, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_4);
	value &= ~TX_PWR_HT_VHT_STBC_MCS_4_5_MASK;
	value |= TX_PWR_HT_VHT_STBC_MCS_4_5(cap->tx_pwr_ht_mcs_4_5 + delta_pwr);

	value &= ~TX_PWR_HT_VHT_STBC_MCS_6_MASK;
	value |= TX_PWR_HT_VHT_STBC_MCS_6(cap->tx_pwr_ht_mcs_6_7 + delta_pwr);
	mt7612u_write32(ad, TX_PWR_CFG_4, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_7);
	value &= ~TX_PWR_OFDM_54_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_OFDM_54(cap->tx_pwr_g_band_ofdm_48_54 + delta_pwr);
	else
		value |= TX_PWR_OFDM_54(cap->tx_pwr_a_band_ofdm_48_54 + delta_pwr);

	value &= ~TX_PWR_VHT_2SS_MCS_8_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_VHT_2SS_MCS_8(cap->tx_pwr_2g_vht_mcs_8_9 + delta_pwr);
	else
		value |= TX_PWR_VHT_2SS_MCS_8(cap->tx_pwr_5g_vht_mcs_8_9 + delta_pwr);

	value &= ~TX_PWR_HT_MCS_7_VHT_1SS_MCS_7_MASK;
	value |= TX_PWR_HT_MCS_7_VHT_1SS_MCS_7(cap->tx_pwr_ht_mcs_6_7 + delta_pwr);

	value &= ~TX_PWR_VHT_2SS_MCS_9_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_VHT_2SS_MCS_9(cap->tx_pwr_2g_vht_mcs_8_9 + delta_pwr);
	else
		value |= TX_PWR_VHT_2SS_MCS_9(cap->tx_pwr_5g_vht_mcs_8_9 + delta_pwr);

	mt7612u_write32(ad, TX_PWR_CFG_7, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_8);
	value &= ~TX_PWR_HT_MCS_15_VHT_2SS_MCS7_MASK;
	value |= TX_PWR_HT_MCS_15_VHT_2SS_MCS7(cap->tx_pwr_ht_mcs_14_15 + delta_pwr);

	value &= ~TX_PWR_VHT_1SS_MCS_8_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_VHT_1SS_MCS_8(cap->tx_pwr_2g_vht_mcs_8_9 + delta_pwr);
	else
		value |= TX_PWR_VHT_1SS_MCS_8(cap->tx_pwr_5g_vht_mcs_8_9 + delta_pwr);

	value &= ~TX_PWR_VHT_1SS_MCS_9_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_VHT_1SS_MCS_9(cap->tx_pwr_2g_vht_mcs_8_9 + delta_pwr);
	else
		value |= TX_PWR_VHT_1SS_MCS_9(cap->tx_pwr_5g_vht_mcs_8_9 + delta_pwr);

	mt7612u_write32(ad, TX_PWR_CFG_8, value);

	value = mt7612u_read32(ad, TX_PWR_CFG_9);
	value &= ~TX_PWR_HT_VHT_STBC_MCS_7_MASK;
	value |= TX_PWR_HT_VHT_STBC_MCS_7(cap->tx_pwr_ht_mcs_6_7 + delta_pwr);

	value &= ~TX_PWR_VHT_STBC_MCS_8_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_VHT_STBC_MCS_8(cap->tx_pwr_2g_vht_mcs_8_9 + delta_pwr);
	else
		value |= TX_PWR_VHT_STBC_MCS_8(cap->tx_pwr_5g_vht_mcs_8_9 + delta_pwr);

	value &= ~TX_PWR_VHT_STBC_MCS_9_MASK;
	if (band == _G_BAND)
		value |= TX_PWR_VHT_STBC_MCS_9(cap->tx_pwr_2g_vht_mcs_8_9 + delta_pwr);
	else
		value |= TX_PWR_VHT_STBC_MCS_9(cap->tx_pwr_5g_vht_mcs_8_9 + delta_pwr);

	mt7612u_write32(ad, TX_PWR_CFG_9, value);
}

static void mt76x2_tx_pwr_gain(struct rtmp_adapter *ad, u8 channel, u8 bw)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	CHAR tx_0_pwr, tx_1_pwr;
	uint32_t value;

	/* set 54Mbps target power */
	if (channel <= 14) {
		tx_0_pwr = cap->tx_0_target_pwr_g_band;
		tx_0_pwr += cap->tx_0_chl_pwr_delta_g_band[get_low_mid_hi_index(channel)];
		if (bw == BW_40)
			tx_0_pwr += cap->delta_tx_pwr_bw40_g_band;

		tx_1_pwr = cap->tx_1_target_pwr_g_band;
		tx_1_pwr += cap->tx_1_chl_pwr_delta_g_band[get_low_mid_hi_index(channel)];
		if (bw == BW_40)
			tx_1_pwr += cap->delta_tx_pwr_bw40_g_band;

	} else {
		tx_0_pwr = cap->tx_0_target_pwr_a_band[get_chl_grp(channel)];
		tx_0_pwr += cap->tx_0_chl_pwr_delta_a_band[get_chl_grp(channel)][get_low_mid_hi_index(channel)];

		if (bw == BW_40)
			tx_0_pwr += cap->delta_tx_pwr_bw40_a_band;
		else if (bw == BW_80)
			tx_0_pwr += cap->delta_tx_pwr_bw80;

		tx_1_pwr = cap->tx_1_target_pwr_a_band[get_chl_grp(channel)];
		tx_1_pwr += cap->tx_1_chl_pwr_delta_a_band[get_chl_grp(channel)][get_low_mid_hi_index(channel)];

		if (bw == BW_40)
			tx_1_pwr += cap->delta_tx_pwr_bw40_a_band;
		else if (bw == BW_80)
			tx_1_pwr += cap->delta_tx_pwr_bw80;
	}

	/* range 0~23.5 db */
	if (tx_0_pwr >= 0x2f)
		tx_0_pwr = 0x2f;

	if (tx_0_pwr < 0)
		tx_0_pwr = 0;

	if (tx_1_pwr >= 0x2f)
		tx_1_pwr = 0x2f;

	if (tx_1_pwr < 0)
		tx_1_pwr = 0;

	/* TX0 channel initial transmission gain setting */
	value = mt7612u_read32(ad, TX_ALC_CFG_0);
	value = value & (~TX_ALC_CFG_0_CH_INT_0_MASK);
	value |= TX_ALC_CFG_0_CH_INT_0(tx_0_pwr);
	//value |= TX_ALC_CFG_0_CH_INT_0(0x7);
	DBGPRINT(RT_DEBUG_INFO, ("tx_0_pwr = %d\n", tx_0_pwr));
	mt7612u_write32(ad, TX_ALC_CFG_0, value);

	/* TX1 channel initial transmission gain setting */
	value = mt7612u_read32(ad, TX_ALC_CFG_0);
	value = value & (~TX_ALC_CFG_0_CH_INT_1_MASK);
	value |= TX_ALC_CFG_0_CH_INT_1(tx_1_pwr);
	//value |= TX_ALC_CFG_0_CH_INT_1(0x7);
	DBGPRINT(RT_DEBUG_INFO, ("tx_1_pwr = %d\n", tx_1_pwr));
	mt7612u_write32(ad, TX_ALC_CFG_0, value);
}

#define EXT_CH_NONE  0x00
#define EXT_CH_ABOVE 0X01
#define EXT_CH_BELOW 0x03

static void mt76x2_switch_channel(struct rtmp_adapter *ad, u8 channel, BOOLEAN scan)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	unsigned int latch_band, band, bw, tx_rx_setting;
	uint32_t ret, i, value, value1, restore_value, loop = 0;
	uint16_t e2p_value;
	UCHAR bbp_ch_idx;
	BOOLEAN band_change = FALSE;
	uint32_t RegValue = 0;
	uint32_t eLNA_gain_from_e2p = 0;
	uint32_t mac_val = 0;

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(ad)) {
		RTMP_SEM_EVENT_WAIT(&ad->hw_atomic, ret);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return;
		}
	}
#endif /* RTMP_MAC_USB */

	if (ad->CommonCfg.BBPCurrentBW == BW_80) {
		bbp_ch_idx = vht_prim_ch_idx(channel, ad->CommonCfg.Channel);
	} else if (ad->CommonCfg.BBPCurrentBW == BW_40) {
		if (ad->CommonCfg.CentralChannel > ad->CommonCfg.Channel)
			bbp_ch_idx = EXT_CH_ABOVE;
		else
			bbp_ch_idx = EXT_CH_BELOW;
	} else {
		bbp_ch_idx = EXT_CH_NONE;
	}

	RegValue = mt7612u_read32(ad, EXT_CCA_CFG);
	RegValue &= ~(0xFFF);
	if (ad->CommonCfg.BBPCurrentBW == BW_80) {
		if (bbp_ch_idx == 0)
			RegValue |= 0x1e4;
		else if (bbp_ch_idx == 1)
			RegValue |= 0x2e1;
		else if (bbp_ch_idx == 2)
			RegValue |= 0x41e;
		else if (bbp_ch_idx == 3)
			RegValue |= 0x81b;
	} else {
		if (ad->CommonCfg.BBPCurrentBW == BW_40) {
			if (ad->CommonCfg.CentralChannel > ad->CommonCfg.Channel)
				RegValue |= 0x1e4;
			else
				RegValue |= 0x2e1;
		} else {
			RegValue |= 0x1e4;
		}
	}

	mt7612u_write32(ad, EXT_CCA_CFG, RegValue);

	/* determine channel flags */
	if (channel > 14)
		band = _A_BAND;
	else
		band = _G_BAND;

	if (!ad->MCUCtrl.power_on) {
		band_change = TRUE;
	} else {
		if (ad->LatchRfRegs.Channel > 14)
			latch_band = _A_BAND;
		else
			latch_band = _G_BAND;

		if (band != latch_band)
			band_change = TRUE;
		else
			band_change = FALSE;
	}

	if (ad->CommonCfg.BBPCurrentBW == BW_80)
		bw = 2;
	else if (ad->CommonCfg.BBPCurrentBW == BW_40)
		bw = 1;
	else
		bw = 0;

	/* Change MAC OFDM SIFS according to BW */
	RegValue = mt7612u_read32(ad, XIFS_TIME_CFG);
	RegValue = RegValue & (~XIFS_TIME_OFDM_SIFS_MASK);
	if (bw == 0)
		RegValue |= XIFS_TIME_OFDM_SIFS(0x0D);
	else /* 40/80Mhz */
		RegValue |= XIFS_TIME_OFDM_SIFS(0x0E);

	mt7612u_write32(ad, XIFS_TIME_CFG, RegValue);


	if ((ad->CommonCfg.TxStream == 1) && (ad->CommonCfg.RxStream == 1))
		tx_rx_setting = 0x101;
	else if ((ad->CommonCfg.TxStream == 2) && (ad->CommonCfg.RxStream == 1))
		tx_rx_setting = 0x201;
	else if ((ad->CommonCfg.TxStream == 1) && (ad->CommonCfg.RxStream == 2))
		tx_rx_setting = 0x102;
	else if ((ad->CommonCfg.TxStream == 2) && (ad->CommonCfg.RxStream == 2))
		tx_rx_setting = 0x202;
	else
		tx_rx_setting = 0x202;



#ifdef RTMP_USB_SUPPORT
	if (IS_USB_INF(ad)) {
		if (band_change) {
			if (band == _G_BAND) {
				RANDOM_WRITE(ad,
					     mt76x2_mac_g_band_cr_table,
					     mt76x2_mac_g_band_cr_nums);

				if (ad->chipCap.PAType & INT_PA_2G)
					RANDOM_WRITE(ad,
						     mt76x2_mac_g_band_internal_pa_cr_table,
						     mt76x2_mac_g_band_internal_pa_cr_nums);
				else
					RANDOM_WRITE(ad,
						     mt76x2_mac_g_band_external_pa_cr_table,
						     mt76x2_mac_g_band_external_pa_cr_nums);

			} else {
				RANDOM_WRITE(ad,
					     mt76x2_mac_a_band_cr_table,
					     mt76x2_mac_a_band_cr_nums);
				if (ad->chipCap.PAType & INT_PA_5G)
					RANDOM_WRITE(ad,
						     mt76x2_mac_a_band_internal_pa_cr_table,
						     mt76x2_mac_a_band_internal_pa_cr_nums);
				else
					RANDOM_WRITE(ad,
						     mt76x2_mac_a_band_external_pa_cr_table,
						     mt76x2_mac_a_band_external_pa_cr_nums);

			}
		}
	}
#endif

	if (IS_MT7602(ad)) {
		/* default use 2 stream to Tx HW auto gened packets */
		mt7612u_write32(ad, 0x1648, 0x00830083);
	}

	/* Fine tune tx power ramp on time based on BBP Tx delay */
	if (isExternalPAMode(ad, channel)) {
		if (bw == 0)
			mt7612u_write32(ad, TX_SW_CFG0, 0x00101101);
		else
			mt7612u_write32(ad, TX_SW_CFG0, 0x000B0C01);

		mt7612u_write32(ad, TX_SW_CFG1, 0x00010200);
	} else {
		if (bw == 0)
			mt7612u_write32(ad, TX_SW_CFG0, 0x00101001);
		else
			mt7612u_write32(ad, TX_SW_CFG0, 0x000B0B01);

		mt7612u_write32(ad, TX_SW_CFG1, 0x00020000);
	}

	/* tx pwr gain setting */
	mt76x2_tx_pwr_gain(ad, channel, bw);

	/* per-rate power delta */
	mt76x2_adjust_per_rate_pwr_delta(ad, channel, 0);

	mt7612u_mcu_switch_channel(ad, channel, scan, bw, tx_rx_setting, bbp_ch_idx);

	eLNA_gain_from_e2p = ((ad->ALNAGain2 & 0xFF) << 24) |
			     ((ad->ALNAGain1 & 0xFF) << 16) |
			     ((ad->ALNAGain0 & 0xFF) << 8) |
			      (ad->BLNAGain & 0xFF);
	mt7612u_mcu_init_gain(ad, channel, TRUE, eLNA_gain_from_e2p);

	value = RTMP_BBP_IO_READ32(ad, AGC1_R8);
	DBGPRINT(RT_DEBUG_INFO, ("%s::BBP 0x2320=0x%08x\n", __FUNCTION__, value));
	value = RTMP_BBP_IO_READ32(ad, AGC1_R9);
	DBGPRINT(RT_DEBUG_INFO, ("%s::BBP 0x2324=0x%08x\n", __FUNCTION__, value));
	value = RTMP_BBP_IO_READ32(ad, AGC1_R4);
	DBGPRINT(RT_DEBUG_INFO, ("%s::BBP 0x2310=0x%08x\n", __FUNCTION__, value));
	value = RTMP_BBP_IO_READ32(ad, AGC1_R5);
	DBGPRINT(RT_DEBUG_INFO, ("%s::BBP 0x2314=0x%08x\n", __FUNCTION__, value));

	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		/* LDPC RX */
		value = RTMP_BBP_IO_READ32(ad, 0x2934);
		value |= (1 << 10);
		RTMP_BBP_IO_WRITE32(ad, 0x2934, value);
	}


#ifdef RTMP_USB_SUPPORT
	mac_val = mt7612u_read32(ad, TXOP_CTRL_CFG);
	if ((mac_val & 0x100000) == 0x100000) {
		ad->chipCap.ed_cca_enable = TRUE;
		mac_val &= ~(1<<20);
		mt7612u_write32(ad, TXOP_CTRL_CFG, mac_val);

		mac_val = mt7612u_read32(ad, TXOP_HLDR_ET);
		mac_val &= ~2;
		mt7612u_write32(ad, TXOP_HLDR_ET, mac_val);
	}
#endif /* RTMP_USB_SUPPORT */

	/* backup mac 1004 value */
	restore_value = mt7612u_read32(ad, 0x1004);

	/* Backup the original RTS retry count and then set to 0 */
	ad->rts_tx_retry_num = mt7612u_read32(ad, 0x1344);

	/* disable mac tx/rx */
	value = restore_value;
	value &= ~0xC;
	mt7612u_write32(ad, 0x1004, value);

	/* set RTS retry count = 0 */
	mt7612u_write32(ad, 0x1344, 0x00092B00);

	/* wait mac 0x1200, bbp 0x2130 idle */
	do {
		value = mt7612u_read32(ad, 0x1200);
		value &= 0x1;
		value1 = RTMP_BBP_IO_READ32(ad, 0x2130);
		DBGPRINT(RT_DEBUG_INFO,("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n", __FUNCTION__));
		RtmpusecDelay(1);
		loop++;
	} while (((value != 0) || (value1 != 0)) && (loop < 300));

	if (loop >= 300) {
		DBGPRINT(RT_DEBUG_OFF, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __FUNCTION__));

		if ((value == 0) && (value1 != 0)) {
			DBGPRINT(RT_DEBUG_OFF, ("%s:: doing IBI and core soft progress\n", __FUNCTION__));

			value1 = RTMP_BBP_IO_READ32(ad, CORE_R4);
			value1 |= 0x2;
			RTMP_BBP_IO_WRITE32(ad, CORE_R4, value1);

			value1 = RTMP_BBP_IO_READ32(ad, CORE_R4);
			value1 &= ~0x2;
			RTMP_BBP_IO_WRITE32(ad, CORE_R4, value1);

			value1 = RTMP_BBP_IO_READ32(ad, CORE_R4);
			value1 |= 0x1;
			RTMP_BBP_IO_WRITE32(ad, CORE_R4, value1);

			value1 = RTMP_BBP_IO_READ32(ad, CORE_R4);
			value1 &= ~0x1;
			RTMP_BBP_IO_WRITE32(ad, CORE_R4, value1);
		}
	}

	if (!ad->MCUCtrl.power_on) {
		 e2p_value = mt7612u_read_eeprom16(ad, BT_RCAL_RESULT);

		if ((e2p_value & 0xff) != 0xff) {
			DBGPRINT(RT_DEBUG_OFF, ("r-cal result = %d\n", e2p_value & 0xff));
			CHIP_CALIBRATION(ad, R_CALIBRATION_7662, 0x00);
		}
	}

	/* RXDCOC calibration */
	CHIP_CALIBRATION(ad, RXDCOC_CALIBRATION_7662, channel);

	if (!ad->MCUCtrl.power_on) {
		/* RX LPF calibration */
		CHIP_CALIBRATION(ad, RC_CALIBRATION_7662, 0x00);
	}

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(ad)) {
		RTMP_SEM_EVENT_WAIT(&ad->tssi_lock, ret);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("tssi_lock get failed(ret=%d)\n", ret));
			return;
		}
	}
#endif /* RTMP_MAC_USB */


	/* TSSI Clibration */
	if (!IS_DOT11_H_RADAR_STATE(ad, RD_SILENCE_MODE) && !ad->chipCap.temp_tx_alc_enable)
		mt76x2_tssi_calibration(ad, channel);

	/* enable TX/RX */
	mt7612u_write32(ad, 0x1004, 0xc);

#ifdef RTMP_USB_SUPPORT
        if (ad->chipCap.ed_cca_enable) {
		mac_val = 0;

		mac_val = mt7612u_read32(ad, TXOP_CTRL_CFG);
                mac_val |= (1<<20);
                mt7612u_write32(ad, TXOP_CTRL_CFG, mac_val);

                mac_val = mt7612u_read32(ad, TXOP_HLDR_ET);
                mac_val |= 2;
                mt7612u_write32(ad, TXOP_HLDR_ET, mac_val);
        }
#endif /* RTMP_USB_SUPPORT */

	/* Restore RTS retry count */
	mt7612u_write32(ad, 0x1344, ad->rts_tx_retry_num);

	if (!ad->MCUCtrl.power_on && ad->chipCap.tssi_enable && !ad->chipCap.temp_tx_alc_enable) {
		value = mt7612u_read32(ad, TX_ALC_CFG_1);
		value = value & (~TX_ALC_CFG_1_TX0_TEMP_COMP_MASK);
		value |= TX_ALC_CFG_1_TX0_TEMP_COMP(0x38);
		mt7612u_write32(ad, TX_ALC_CFG_1, value);
		DBGPRINT(RT_DEBUG_OFF, ("TX0 power compensation = 0x%x\n", value & 0x3f));
		value = mt7612u_read32(ad, TX_ALC_CFG_2);
		value = value & (~TX_ALC_CFG_2_TX1_TEMP_COMP_MASK);
		value |= TX_ALC_CFG_2_TX1_TEMP_COMP(0x38);
		mt7612u_write32(ad, TX_ALC_CFG_2, value);
		DBGPRINT(RT_DEBUG_OFF, ("TX1 power compensation = 0x%x\n", value & 0x3f));
	}


#ifdef RTMP_MAC_USB
	if (IS_USB_INF(ad))
		RTMP_SEM_EVENT_UP(&ad->tssi_lock);
#endif


	/* Channel latch */
	ad->LatchRfRegs.Channel = channel;

	if (!ad->MCUCtrl.power_on)
		ad->MCUCtrl.power_on = TRUE;

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(ad))
		RTMP_SEM_EVENT_UP(&ad->hw_atomic);
#endif

	//mt76x2_set_ed_cca(ad, TRUE);

	percentage_delta_pwr(ad);

#ifdef DYNAMIC_VGA_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	ad->chipCap.agc1_r35_backup = RTMP_BBP_IO_READ32(ad, AGC1_R35);
	ad->chipCap.agc1_r39_backup = RTMP_BBP_IO_READ32(ad, AGC1_R39);
	ad->chipCap.agc1_r41_backup = RTMP_BBP_IO_READ32(ad, AGC1_R41);
#endif /* CONFIG_AP_SUPPORT */

	RTMP_BBP_IO_WRITE32(ad, AGC1_R61, 0xFF64A4E2); /* microwave's function initial gain */
	RTMP_BBP_IO_WRITE32(ad, AGC1_R7, 0x08081010); /* microwave's ED CCA threshold */
	RTMP_BBP_IO_WRITE32(ad, AGC1_R11, 0x00000404); /* microwave's ED CCA threshold */
	RTMP_BBP_IO_WRITE32(ad, AGC1_R2, 0x00007070); /* initial ED CCA threshold */
	mt7612u_write32(ad, TXOP_CTRL_CFG, 0x04101B3F);
#endif /* DYNAMIC_VGA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE,
			("%s(): Switch to Ch#%d(%dT%dR), BBP_BW=%d, bbp_ch_idx=%d)\n",
			__FUNCTION__,
			channel,
			ad->CommonCfg.TxStream,
			ad->CommonCfg.RxStream,
			ad->CommonCfg.BBPCurrentBW,
			bbp_ch_idx));
}

void mt76x2_external_pa_rf_dac_control(struct rtmp_adapter *ad, u8 channel)
{
	if (isExternalPAMode(ad, channel)) {
		if (!strncmp(ad->CommonCfg.CountryCode, "US", 2)
			&& (channel >= 36 && channel <= 48)) {
				mt_rf_write(ad, 0, 0x058, 0x226C6000);
				mt_rf_write(ad, 1, 0x058, 0x226C6000);
				DBGPRINT(RT_DEBUG_OFF,("%s::change MT7612E's RF DAC control range since in US band 1 frequency range\n",
					__FUNCTION__));
		}
	}
}

void mt76x2_tssi_calibration(struct rtmp_adapter *ad, u8 channel)
{
	/* TSSI Clibration */
	if (ad->chipCap.tssi_enable) {
		ad->chipCap.tssi_stage = TSSI_CAL_STAGE;
		if (channel > 14) {
			if (ad->chipCap.PAType == EXT_PA_2G_5G)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0101);
			else if (ad->chipCap.PAType == EXT_PA_5G_ONLY)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0101);
			else
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0001);
		} else {
			if (ad->chipCap.PAType == EXT_PA_2G_5G)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0100);
			else if ((ad->chipCap.PAType == EXT_PA_5G_ONLY) ||
					(ad->chipCap.PAType == INT_PA_2G_5G))
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0000);
			else if (ad->chipCap.PAType == EXT_PA_2G_ONLY)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0100);
			 else
				DBGPRINT(RT_DEBUG_ERROR, ("illegal PA Type(%d)\n", ad->chipCap.PAType));
		}
		ad->chipCap.tssi_stage = TSSI_TRIGGER_STAGE;
	}
}

void mt76x2_tssi_compensation(struct rtmp_adapter *ad, u8 channel)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	ANDES_CALIBRATION_PARAM param;
	uint32_t value = 0;
	uint32_t ret = 0;

#ifdef RTMP_USB_SUPPORT
	if (IS_USB_INF(ad)) {
		RTMP_SEM_EVENT_WAIT(&ad->tssi_lock, ret);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("tssi_lock get failed(ret=%d)\n", ret));
			return;
		}
	}
#endif


	if (ad->chipCap.tssi_stage <= TSSI_CAL_STAGE)
		goto done;

	if (cap->tssi_stage == TSSI_TRIGGER_STAGE) {
		DBGPRINT(RT_DEBUG_INFO, ("%s:TSS_TRIGGER(channel = %d)\n", __FUNCTION__, channel));
		param.mt76x2_tssi_comp_param.pa_mode = (1 << 8);
		param.mt76x2_tssi_comp_param.tssi_slope_offset = 0;

		/* TSSI Trigger */
		if(ad->chipOps.Calibration != NULL)
			ad->chipOps.Calibration(ad, TSSI_COMPENSATION_7662, &param);
		else
			goto done;

		cap->tssi_stage = TSSI_COMP_STAGE;

		goto done;
	}

	/* Check 0x2088[4] = 0 */
	value = RTMP_BBP_IO_READ32(ad, CORE_R34);

	if ((value & (1 << 4)) == 0) {
		uint32_t pa_mode = 0, tssi_slope_offset = 0;

		DBGPRINT(RT_DEBUG_INFO, ("%s:TSSI_COMP(channel = %d)\n", __FUNCTION__, channel));

		if (channel > 14) {
			if (ad->chipCap.PAType == EXT_PA_2G_5G)
				pa_mode = 1;
			else if (ad->chipCap.PAType == EXT_PA_5G_ONLY)
				pa_mode = 1;
			else
				pa_mode = 0;
		} else {
			if (ad->chipCap.PAType == EXT_PA_2G_5G)
				pa_mode = 1;
			else if ((ad->chipCap.PAType == EXT_PA_5G_ONLY) ||
					(ad->chipCap.PAType == INT_PA_2G_5G))
				pa_mode = 0;
			else if (ad->chipCap.PAType == EXT_PA_2G_ONLY)
				pa_mode = 1;
		}

		if (channel <= 14) {
			tssi_slope_offset = TSSI_PARAM2_SLOPE0(cap->tssi_0_slope_g_band)
					| TSSI_PARAM2_SLOPE1(cap->tssi_1_slope_g_band)
					| TSSI_PARAM2_OFFSET0(cap->tssi_0_offset_g_band)
					| TSSI_PARAM2_OFFSET1(cap->tssi_1_offset_g_band);
		} else {
			tssi_slope_offset = TSSI_PARAM2_SLOPE0(cap->tssi_0_slope_a_band[get_chl_grp(channel)])
					| TSSI_PARAM2_SLOPE1(cap->tssi_1_slope_a_band[get_chl_grp(channel)])
					| TSSI_PARAM2_OFFSET0(cap->tssi_0_offset_a_band[get_chl_grp(channel)])
					| TSSI_PARAM2_OFFSET1(cap->tssi_1_offset_a_band[get_chl_grp(channel)]);
		}

		param.mt76x2_tssi_comp_param.pa_mode = (pa_mode | ((0x1) << 9));
		param.mt76x2_tssi_comp_param.tssi_slope_offset = tssi_slope_offset;

		/* TSSI Compensation */
		if(ad->chipOps.Calibration != NULL)
			ad->chipOps.Calibration(ad, TSSI_COMPENSATION_7662, &param);

		cap->tssi_stage = TSSI_TRIGGER_STAGE;

		if (!ad->MCUCtrl.dpd_on
		) {
			/* DPD Calibration */
			if ( (ad->chipCap.PAType== INT_PA_2G_5G)
				|| ((ad->chipCap.PAType == INT_PA_5G) && ( channel > 14 ) )
				|| ((ad->chipCap.PAType == INT_PA_2G) && ( channel <= 14 ) )
			)
				CHIP_CALIBRATION(ad, DPD_CALIBRATION_7662, channel);
				ad->MCUCtrl.dpd_on = TRUE;
		}
	}

done:
;

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(ad))
		RTMP_SEM_EVENT_UP(&ad->tssi_lock);
#endif
}

void mt76x2_calibration(struct rtmp_adapter *ad, u8 channel)
{
	uint32_t value, value1, restore_value, loop = 0;
        uint32_t mac_val = 0;

	if (IS_DOT11_H_RADAR_STATE(ad, RD_SILENCE_MODE)) {
		DBGPRINT(RT_DEBUG_OFF,
			("%s():RDMode  is in Silent State, do not calibration.\n", __FUNCTION__));
		return;
	}

#ifdef RTMP_USB_SUPPORT
        mac_val = mt7612u_read32(ad, TXOP_CTRL_CFG);
        if ((mac_val & 0x100000) == 0x100000) {
                ad->chipCap.ed_cca_enable = TRUE;
                mac_val &= ~(1<<20);
                mt7612u_write32(ad, TXOP_CTRL_CFG, mac_val);

                mac_val = mt7612u_read32(ad, TXOP_HLDR_ET);
                mac_val &= ~2;
                mt7612u_write32(ad, TXOP_HLDR_ET, mac_val);
        }
#endif /* RTMP_USB_SUPPORT */

	DBGPRINT(RT_DEBUG_OFF, ("%s(channel = %d)\n", __FUNCTION__, channel));

	/* backup mac 1004 value */
	restore_value = mt7612u_read32(ad, 0x1004);

	/* Backup the original RTS retry count and then set to 0 */
	ad->rts_tx_retry_num = mt7612u_read32(ad, 0x1344);

	/* disable mac tx/rx */
	value = restore_value;
	value &= ~0xC;
	mt7612u_write32(ad, 0x1004, value);

	/* set RTS retry count = 0 */
	mt7612u_write32(ad, 0x1344, 0x00092B00);

	/* wait mac 0x1200, bbp 0x2130 idle */
	do {
		value = mt7612u_read32(ad, 0x1200);
		value &= 0x1;
		value1 = RTMP_BBP_IO_READ32(ad, 0x2130);
		DBGPRINT(RT_DEBUG_INFO, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n", __FUNCTION__));
		RtmpusecDelay(1);
		loop++;
	} while (((value != 0) || (value1 != 0)) && (loop < 300));

	if (loop >= 300)
		DBGPRINT(RT_DEBUG_OFF, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __FUNCTION__));

	/* LC Calibration */
	if (channel > 14)
		CHIP_CALIBRATION(ad, LC_CALIBRATION_7662, 0x00);

	/* TX LOFT */
	if (channel > 14)
		CHIP_CALIBRATION(ad, TX_LOFT_CALIBRATION_7662, 0x1);
	else
		CHIP_CALIBRATION(ad, TX_LOFT_CALIBRATION_7662, 0x0);

	/* TXIQ Clibration */
	if (channel > 14)
		CHIP_CALIBRATION(ad, TXIQ_CALIBRATION_7662, 0x1);
	else
		CHIP_CALIBRATION(ad, TXIQ_CALIBRATION_7662, 0x0);

	/* RXIQC-FI */
	if (channel > 14)
		CHIP_CALIBRATION(ad, RXIQC_FI_CALIBRATION_7662, 0x1);
	else
		CHIP_CALIBRATION(ad, RXIQC_FI_CALIBRATION_7662, 0x0);

	/* TEMP SENSOR */
	CHIP_CALIBRATION(ad, TEMP_SENSOR_CALIBRATION_7662, 0x00);

	/* enable TX/RX */
	mt7612u_write32(ad, 0x1004, restore_value);

	/* Restore RTS retry count */
	mt7612u_write32(ad, 0x1344, ad->rts_tx_retry_num);

}

static void mt76x2_cal_test(struct rtmp_adapter *ad, uint32_t type)
{
	UCHAR cent_ch;

	if(ad->CommonCfg.BBPCurrentBW == BW_80)
		cent_ch = ad->CommonCfg.vht_cent_ch;
	else
		cent_ch = ad->CommonCfg.CentralChannel;

}

/*
 * Initialize FCE
 */
VOID init_fce(struct rtmp_adapter *ad)
{
	L2_STUFFING_STRUC reg;

	reg.word = 0;


#ifdef RTMP_USB_SUPPORT
	reg.word = mt7612u_read32(ad, FCE_L2_STUFF);
	reg.field.FS_WR_MPDU_LEN_EN = 0;
	mt7612u_write32(ad, FCE_L2_STUFF, reg.word);
#endif
}

static void mt76x2_init_mac_cr(struct rtmp_adapter *ad)
{
	u32 i;
	u32 value = 0;
	u16 e2p_value;
	char xtal_freq_offset = 0;

	/*
		Enable PBF and MAC clock
		SYS_CTRL[11:10] = 0x3
	*/

#ifdef RTMP_USB_SUPPORT
	if (IS_USB_INF(ad))
		RANDOM_WRITE(ad, mt76x2_mac_cr_table, mt76x2_mac_cr_nums);
#endif

#ifdef HDR_TRANS_TX_SUPPORT
	/*
 	 * Enable Header Translation TX
 	 */
	value = mt7612u_read32(ad, HEADER_TRANS_CTRL_REG);
	value |= 0x1; /* 0x1: TX, 0x2: RX */
	mt7612u_write32(ad, HEADER_TRANS_CTRL_REG, value);
#endif /* HDR_TRANS_TX_SUPPORT */

	/*
 	 * Release BBP and MAC reset
 	 * MAC_SYS_CTRL[1:0] = 0x0
 	 */
	value = mt7612u_read32(ad, MAC_SYS_CTRL);
	value &= ~(0x3);
	mt7612u_write32(ad, MAC_SYS_CTRL, value);

	if (IS_MT7612(ad)) {
		/*
 		 * Disable COEX_EN
 		 */
		value = mt7612u_read32(ad, COEXCFG0);
		value &= 0xFFFFFFFE;
		mt7612u_write32(ad, COEXCFG0, value);
	}

	/*
		Set 0x141C[15:12]=0xF
	*/
	value = mt7612u_read32(ad, EXT_CCA_CFG);
	value |= (0x0000F000);
	mt7612u_write32(ad, EXT_CCA_CFG, value);


	/*
 	 * Set 0x13C0[31] = 0x0
 	 */
	value = mt7612u_read32(ad, TX_ALC_CFG_4);
	value &= ~WL_LOWGAIN_CH_EN;
	mt7612u_write32(ad, TX_ALC_CFG_4, value);

	/*
 	 * Check crystal trim2 first
 	 */
	e2p_value = mt7612u_read_eeprom16(ad, G_BAND_BANDEDGE_PWR_BACK_OFF);

	if (((e2p_value & 0xff) == 0x00) || ((e2p_value & 0xff) == 0xff))
		xtal_freq_offset = 0;
	else if ((e2p_value & 0x80) == 0x80)
		xtal_freq_offset = 0 - (e2p_value & 0x7f);
	else if ((e2p_value & 0x80) == 0x00)
		xtal_freq_offset = (e2p_value & 0x7f);

	if ((((e2p_value >> 8) & 0xff) == 0x00) ||
	    (((e2p_value >> 8) & 0xff) == 0xff)) {
		/*
 		 * Compesate crystal trim1
 		 */
		e2p_value = mt7612u_read_eeprom16(ad, XTAL_TRIM1);

		/* crystal trim default value set to 0x14 */
		if (((e2p_value & 0xff) == 0x00) || ((e2p_value & 0xff) == 0xff))
			e2p_value = 0x14;

		/* Set crystal trim1 */
		mt7612u_read_reg(ad, XO_CTRL5, &value);
		value &= 0xffff80ff;
		value |= ((((e2p_value & XTAL_TRIM1_MASK) + xtal_freq_offset) & XTAL_TRIM1_MASK) << 8);
		mt7612u_write_reg(ad, XO_CTRL5, value);

		/* Enable */
		mt7612u_read_reg(ad, XO_CTRL6, &value);
		value &= 0xffff80ff;
		value |= (0x7f << 8);
		mt7612u_write_reg(ad, XO_CTRL6, value);
	} else {
		/* Set crystal trim2 */
		mt7612u_read_reg(ad, XO_CTRL5, &value);
		value &= 0xffff80ff;
		value |= (((e2p_value & XTAL_TRIM2_MASK) + (xtal_freq_offset << 8)) & XTAL_TRIM2_MASK);
		mt7612u_write_reg(ad, XO_CTRL5, value);

		/* Enable */
		mt7612u_read_reg(ad, XO_CTRL6, &value);
		value &= 0xffff80ff;
		value |= (0x7f << 8);
		mt7612u_write_reg(ad, XO_CTRL6, value);
	}

	/*
 	 * add 504, 50c value per ben kao suggestion for rx receivce packet, need to revise this bit
     * only mt7662u do not this setting
	 */
	if (IS_MT76x2U(ad)) {
		mt7612u_write32(ad, 0x504, 0x06000000);
		mt7612u_write32(ad, 0x50c, 0x08800000);
		RtmpOsMsDelay(5);
		mt7612u_write32(ad, 0x504, 0x0);
	}

	/* Decrease MAC OFDM SIFS from 16 to 13us */
	value = mt7612u_read32(ad, XIFS_TIME_CFG);
	value = value & (~XIFS_TIME_OFDM_SIFS_MASK);
	value |= XIFS_TIME_OFDM_SIFS(0x0D);
	mt7612u_write32(ad, XIFS_TIME_CFG, value);

	value = mt7612u_read32(ad, BKOFF_SLOT_CFG);
	value &= ~(BKOFF_SLOT_CFG_CC_DELAY_TIME_MASK);
	value |= BKOFF_SLOT_CFG_CC_DELAY_TIME(0x01);
	mt7612u_write32(ad, BKOFF_SLOT_CFG, value);

	init_fce(ad);

#ifdef MCS_LUT_SUPPORT
	value = mt7612u_read32(ad, TX_FBK_LIMIT);
	if (RTMP_TEST_MORE_FLAG(ad, fASIC_CAP_MCS_LUT))
		value |= 0x40000;
	else
		value &= (~0x40000);
	mt7612u_write32(ad, TX_FBK_LIMIT, value);
#endif /* MCS_LUT_SUPPORT */

	/*
		For co-clock image
		00 : one crystal , disable co-clock out
		01 : One crystal, enable co-clock out
		10 : Two crystal (Default)
	*/
	value = mt7612u_read32(ad, 0x11C);
	if (ad->NicConfig3.field.XtalOption == 0x0)
		value = 0x5C1FEE80;
	else if (ad->NicConfig3.field.XtalOption == 0x1)
		value = 0x5C1FEED0;
	mt7612u_write32(ad, 0x11C, value);


}

static void mt76x2_init_rf_cr(struct rtmp_adapter *ad)
{
	uint16_t value;

	mt7612u_mcu_load_cr(ad, RF_BBP_CR, 0, 0);
}

void mt76x2_get_external_lna_gain(struct rtmp_adapter *ad)
{
	USHORT e2p_val = 0;
	UINT8 lna_type = 0;

	/* b'00: 2.4G+5G external LNA, b'01: 5G external LNA, b'10: 2.4G external LNA, b'11: Internal LNA */
	e2p_val = mt7612u_read_eeprom16(ad, 0x36);
	lna_type = e2p_val & 0xC;
	if (lna_type == 0xC)
		ad->chipCap.LNA_type = 0x0;
	else if (lna_type == 0x8)
		ad->chipCap.LNA_type = 0x1;
	else if (lna_type == 0x4)
		ad->chipCap.LNA_type = 0x10;
	else if (lna_type == 0x0)
		ad->chipCap.LNA_type = 0x11;

	e2p_val = mt7612u_read_eeprom16(ad, 0x44);
	ad->BLNAGain = (e2p_val & 0xFF); /* store external LNA gain for 2.4G on EEPROM 0x44h */
	ad->ALNAGain0 = (e2p_val & 0xFF00) >> 8; /* store external LNA gain for 5G ch#36 ~ ch#64 on EEPROM 0x45h */

	e2p_val = mt7612u_read_eeprom16(ad, 0x48);
	ad->ALNAGain1 = (e2p_val & 0xFF00) >> 8; /* store external LNA gain for 5G ch#100 ~ ch#128 on EEPROM 0x49h */

	e2p_val = mt7612u_read_eeprom16(ad, 0x4C);
	ad->ALNAGain2 = (e2p_val & 0xFF00) >> 8; /* store external LNA gain for 5G ch#132 ~ ch#165 on EEPROM 0x4Dh */

	DBGPRINT(RT_DEBUG_OFF, ("%s::LNA type=0x%x, BLNAGain=0x%x, ALNAGain0=0x%x, ALNAGain1=0x%x, ALNAGain2=0x%x\n",
		__FUNCTION__, ad->chipCap.LNA_type, ad->BLNAGain, ad->ALNAGain0, ad->ALNAGain1, ad->ALNAGain2));
}

void mt76x2_get_agc_gain(struct rtmp_adapter *ad, BOOLEAN init_phase)
{
	UCHAR val;
	USHORT val16;
	uint32_t bbp_val;

	bbp_val = RTMP_BBP_IO_READ32(ad, AGC1_R8);
	val = ((bbp_val & (0x00007f00)) >> 8) & 0x7f;
	ad->CommonCfg.lna_vga_ctl.agc_vga_init_0 = val;
	ad->CommonCfg.lna_vga_ctl.agc1_r8_backup = bbp_val;
	if (init_phase == TRUE) {
		ad->CommonCfg.lna_vga_ctl.agc_vga_ori_0 =
			ad->CommonCfg.lna_vga_ctl.agc_vga_init_0;
		DBGPRINT(RT_DEBUG_OFF, ("original vga value(chain0) = %x\n",  ad->CommonCfg.lna_vga_ctl.agc_vga_ori_0));
	}

	val16 = ((bbp_val & (0xffff0000)) >> 16) & (0xffff);
	ad->CommonCfg.lna_vga_ctl.agc_0_vga_set1_2 = val16;
	DBGPRINT(RT_DEBUG_TRACE, ("initial vga value(chain0) = %x\n",  ad->CommonCfg.lna_vga_ctl.agc_vga_init_0));

	if (ad->CommonCfg.RxStream >= 2) {
		bbp_val = RTMP_BBP_IO_READ32(ad, AGC1_R9);
		val = ((bbp_val & (0x00007f00)) >> 8) & 0x7f;
		ad->CommonCfg.lna_vga_ctl.agc_vga_init_1 = val;
		ad->CommonCfg.lna_vga_ctl.agc1_r9_backup = bbp_val;
		if (init_phase == TRUE) {
			ad->CommonCfg.lna_vga_ctl.agc_vga_ori_1 =
				ad->CommonCfg.lna_vga_ctl.agc_vga_init_1;
			DBGPRINT(RT_DEBUG_OFF, ("original vga value(chain1) = %x\n",  ad->CommonCfg.lna_vga_ctl.agc_vga_ori_1));
		}

		val16 = ((bbp_val & (0xffff0000)) >> 16) & (0xffff);
		ad->CommonCfg.lna_vga_ctl.agc_1_vga_set1_2 = val16;

		DBGPRINT(RT_DEBUG_TRACE, ("initial vga value(chain1) = %x\n",  ad->CommonCfg.lna_vga_ctl.agc_vga_init_1));
	}

	ad->CommonCfg.lna_vga_ctl.bDyncVgaEnable = TRUE;
}

int mt76x2_reinit_agc_gain(struct rtmp_adapter *ad, u8 channel)
{
	uint32_t value0, value1;
	CHAR agc_vga0, agc_vga1;
	UINT8 chl_grp;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	value0 = RTMP_BBP_IO_READ32(ad, AGC1_R8);
	agc_vga0 = ((value0 & (0x00007f00)) >> 8) & 0x7f;

	value1 = RTMP_BBP_IO_READ32(ad, AGC1_R9);
	agc_vga1 = ((value1 & (0x00007f00)) >> 8) & 0x7f;

	DBGPRINT(RT_DEBUG_OFF, ("%s:original agc_vga0 = 0x%x, agc_vga1 = 0x%x\n", __FUNCTION__, agc_vga0, agc_vga1));

	if (channel > 14) {
		chl_grp = get_chl_grp(channel);
		switch (chl_grp) {
			case A_BAND_GRP0_CHL:
				agc_vga0 += cap->rf0_5g_grp0_rx_high_gain;
				agc_vga1 += cap->rf1_5g_grp0_rx_high_gain;
				break;
			case A_BAND_GRP1_CHL:
				agc_vga0 += cap->rf0_5g_grp1_rx_high_gain;
				agc_vga1 += cap->rf1_5g_grp1_rx_high_gain;
				break;
			case A_BAND_GRP2_CHL:
				agc_vga0 += cap->rf0_5g_grp2_rx_high_gain;
				agc_vga1 += cap->rf1_5g_grp2_rx_high_gain;
				break;
			case A_BAND_GRP3_CHL:
				agc_vga0 += cap->rf0_5g_grp3_rx_high_gain;
				agc_vga1 += cap->rf1_5g_grp3_rx_high_gain;
				break;
			case A_BAND_GRP4_CHL:
				agc_vga0 += cap->rf0_5g_grp4_rx_high_gain;
				agc_vga1 += cap->rf1_5g_grp4_rx_high_gain;
				break;
			case A_BAND_GRP5_CHL:
				agc_vga0 += cap->rf0_5g_grp5_rx_high_gain;
				agc_vga1 += cap->rf1_5g_grp5_rx_high_gain;
				break;
			default:
				DBGPRINT(RT_DEBUG_OFF, ("illegal channel group(%d)\n", chl_grp));
				break;
		}
	} else {
		agc_vga0 += cap->rf0_2g_rx_high_gain;
		agc_vga1 += cap->rf1_2g_rx_high_gain;
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s:updated agc_vga0 = 0x%x, agc_vga1 = 0x%x\n", __FUNCTION__, agc_vga0, agc_vga1));

	value0 &= 0xffff80ff;
	value0 |= ((0x7f & agc_vga0) << 8);

	value1 &= 0xffff80ff;
	value1 |= ((0x7f & agc_vga1) << 8);

	RTMP_BBP_IO_WRITE32(ad, AGC1_R8, value0);
	RTMP_BBP_IO_WRITE32(ad, AGC1_R9, value1);

	return 0;
}

int mt76x2_reinit_hi_lna_gain(struct rtmp_adapter *ad, u8 channel)
{
	uint32_t value0, value1;
	CHAR hi_lna0, hi_lna1;
	UINT8 chl_grp;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	value0 = RTMP_BBP_IO_READ32(ad, AGC1_R4);
	hi_lna0 = ((value0 & (0x003f0000)) >> 16) & 0x3f;

	value1 = RTMP_BBP_IO_READ32(ad, AGC1_R5);
	hi_lna1 = ((value1 & (0x003f0000)) >> 16) & 0x3f;

	DBGPRINT(RT_DEBUG_OFF, ("%s:original hi_lna0 = 0x%x, hi_lna1 = 0x%x\n", __FUNCTION__, hi_lna0, hi_lna1));

	if (channel > 14) {
		chl_grp = get_chl_grp(channel);
		switch (chl_grp) {
			case A_BAND_GRP0_CHL:
				hi_lna0 -= (cap->rf0_5g_grp0_rx_high_gain / 2);
				hi_lna1 -= (cap->rf1_5g_grp0_rx_high_gain / 2);
				break;
			case A_BAND_GRP1_CHL:
				hi_lna0 -= (cap->rf0_5g_grp1_rx_high_gain / 2);
				hi_lna1 -= (cap->rf1_5g_grp1_rx_high_gain / 2);
				break;
			case A_BAND_GRP2_CHL:
				hi_lna0 -= (cap->rf0_5g_grp2_rx_high_gain / 2);
				hi_lna1 -= (cap->rf1_5g_grp2_rx_high_gain / 2);
				break;
			case A_BAND_GRP3_CHL:
				hi_lna0 -= (cap->rf0_5g_grp3_rx_high_gain / 2);
				hi_lna1 -= (cap->rf1_5g_grp3_rx_high_gain / 2);
				break;
			case A_BAND_GRP4_CHL:
				hi_lna0 -= (cap->rf0_5g_grp4_rx_high_gain / 2);
				hi_lna1 -= (cap->rf1_5g_grp4_rx_high_gain / 2);
				break;
			case A_BAND_GRP5_CHL:
				hi_lna0 -= (cap->rf0_5g_grp5_rx_high_gain / 2);
				hi_lna1 -= (cap->rf1_5g_grp5_rx_high_gain / 2);
				break;
			default:
				DBGPRINT(RT_DEBUG_OFF, ("illegal channel group(%d)\n", chl_grp));
				break;
		}
	} else {
		hi_lna0 -= (cap->rf0_2g_rx_high_gain / 2);
		hi_lna1 -= (cap->rf1_2g_rx_high_gain / 2);
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s:updated hi_lna0 = 0x%x, hi_lna1 = 0x%x\n", __FUNCTION__, hi_lna0, hi_lna1));

	value0 &= 0xffc0ffff;
	value0 |= ((0x3f & hi_lna0) << 16);

	value1 &= 0xffc0ffff;
	value1 |= ((0x3f & hi_lna1) << 16);

	RTMP_BBP_IO_WRITE32(ad, AGC1_R4, value0);
	RTMP_BBP_IO_WRITE32(ad, AGC1_R5, value1);

	return 0;
}

void mt76x2_get_rx_high_gain(struct rtmp_adapter *ad)
{
	uint16_t value;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	value = mt7612u_read_eeprom16(ad, RF_2G_RX_HIGH_GAIN);
	if ((value & 0xff00) == 0x0000 || ((value & 0xff00) == 0xff00)) {
		cap->rf0_2g_rx_high_gain = 0;
		cap->rf1_2g_rx_high_gain = 0;
	} else {
		if (value & RF0_2G_RX_HIGH_GAIN_SIGN)
			cap->rf0_2g_rx_high_gain =
				((value & RF0_2G_RX_HIGH_GAIN_MASK) >> 8);
		else
			cap->rf0_2g_rx_high_gain =
				-((value & RF0_2G_RX_HIGH_GAIN_MASK) >> 8);

		if (value & RF1_2G_RX_HIGH_GAIN_SIGN)
			cap->rf1_2g_rx_high_gain =
				((value & RF1_2G_RX_HIGH_GAIN_MASK) >> 12);
		else
			cap->rf1_2g_rx_high_gain =
				-((value & RF1_2G_RX_HIGH_GAIN_MASK) >> 12);
	}

	value = mt7612u_read_eeprom16(ad, RF_5G_GRP0_1_RX_HIGH_GAIN);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->rf0_5g_grp0_rx_high_gain = 0;
		cap->rf1_5g_grp0_rx_high_gain = 0;
	} else {
		if (value & RF0_5G_GRP0_RX_HIGH_GAIN_SIGN)
			cap->rf0_5g_grp0_rx_high_gain =
				(value & RF0_5G_GRP0_RX_HIGH_GAIN_MASK);
		else
			cap->rf0_5g_grp0_rx_high_gain =
				-(value & RF0_5G_GRP0_RX_HIGH_GAIN_MASK);

		if (value & RF1_5G_GRP0_RX_HIGH_GAIN_SIGN)
			cap->rf1_5g_grp0_rx_high_gain =
				((value & RF1_5G_GRP0_RX_HIGH_GAIN_MASK) >> 4);
		else
			cap->rf1_5g_grp0_rx_high_gain =
				-((value & RF1_5G_GRP0_RX_HIGH_GAIN_MASK) >> 4);
	}

	if ((value & 0xff00) == 0x0000 || ((value & 0xff00) == 0xff00)) {
		cap->rf0_5g_grp1_rx_high_gain = 0;
		cap->rf1_5g_grp1_rx_high_gain = 0;
	} else {
		if (value & RF0_5G_GRP1_RX_HIGH_GAIN_SIGN)
			cap->rf0_5g_grp1_rx_high_gain =
				((value & RF0_5G_GRP1_RX_HIGH_GAIN_MASK) >> 8);
		else
			cap->rf0_5g_grp1_rx_high_gain =
				-((value & RF0_5G_GRP1_RX_HIGH_GAIN_MASK) >> 8);

		if (value & RF1_5G_GRP1_RX_HIGH_GAIN_SIGN)
			cap->rf1_5g_grp1_rx_high_gain =
				((value & RF1_5G_GRP1_RX_HIGH_GAIN_MASK) >> 12);
		else
			cap->rf1_5g_grp1_rx_high_gain =
				-((value & RF1_5G_GRP1_RX_HIGH_GAIN_MASK) >> 12);

	}

	value = mt7612u_read_eeprom16(ad, RF_5G_GRP2_3_RX_HIGH_GAIN);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->rf0_5g_grp2_rx_high_gain = 0;
		cap->rf1_5g_grp2_rx_high_gain = 0;
	} else {
		if (value & RF0_5G_GRP2_RX_HIGH_GAIN_SIGN)
			cap->rf0_5g_grp2_rx_high_gain =
				(value & RF0_5G_GRP2_RX_HIGH_GAIN_MASK);
		else
			cap->rf0_5g_grp2_rx_high_gain =
				-(value & RF0_5G_GRP2_RX_HIGH_GAIN_MASK);

		if (value & RF1_5G_GRP2_RX_HIGH_GAIN_SIGN)
			cap->rf1_5g_grp2_rx_high_gain =
				((value & RF1_5G_GRP2_RX_HIGH_GAIN_MASK) >> 4);
		else
			cap->rf1_5g_grp2_rx_high_gain =
				-((value & RF1_5G_GRP2_RX_HIGH_GAIN_MASK) >> 4);
	}

	if ((value & 0xff00) == 0x0000 || ((value & 0xff00) == 0xff00)) {
		cap->rf0_5g_grp3_rx_high_gain = 0;
		cap->rf1_5g_grp3_rx_high_gain = 0;
	} else {
		if (value & RF0_5G_GRP3_RX_HIGH_GAIN_SIGN)
			cap->rf0_5g_grp3_rx_high_gain =
				((value & RF0_5G_GRP3_RX_HIGH_GAIN_MASK) >> 8);
		else
			cap->rf0_5g_grp3_rx_high_gain =
				-((value & RF0_5G_GRP3_RX_HIGH_GAIN_MASK) >> 8);

		if (value & RF1_5G_GRP3_RX_HIGH_GAIN_SIGN)
			cap->rf1_5g_grp3_rx_high_gain =
				((value & RF1_5G_GRP3_RX_HIGH_GAIN_MASK) >> 12);
		else
			cap->rf1_5g_grp3_rx_high_gain =
				-((value & RF1_5G_GRP3_RX_HIGH_GAIN_MASK) >> 12);
	}

	value = mt7612u_read_eeprom16(ad, RF_5G_GRP4_5_RX_HIGH_GAIN);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->rf0_5g_grp4_rx_high_gain = 0;
		cap->rf1_5g_grp4_rx_high_gain = 0;
	} else {
		if (value & RF0_5G_GRP4_RX_HIGH_GAIN_SIGN)
			cap->rf0_5g_grp4_rx_high_gain =
				value & RF0_5G_GRP4_RX_HIGH_GAIN_MASK;
		else
			cap->rf0_5g_grp4_rx_high_gain =
				-(value & RF0_5G_GRP4_RX_HIGH_GAIN_MASK);

		if (value & RF1_5G_GRP4_RX_HIGH_GAIN_SIGN)
			cap->rf1_5g_grp4_rx_high_gain =
				((value & RF1_5G_GRP4_RX_HIGH_GAIN_MASK) >> 4);
		else
			cap->rf1_5g_grp4_rx_high_gain =
				-((value & RF1_5G_GRP4_RX_HIGH_GAIN_MASK) >> 4);
	}

	if ((value & 0xff00) == 0x0000 || ((value & 0xff00) == 0xff00)) {
		cap->rf0_5g_grp5_rx_high_gain = 0;
		cap->rf1_5g_grp5_rx_high_gain = 0;
	} else {
		if (value & RF0_5G_GRP5_RX_HIGH_GAIN_SIGN)
			cap->rf0_5g_grp5_rx_high_gain =
				((value & RF0_5G_GRP5_RX_HIGH_GAIN_MASK) >> 8);
		else
			cap->rf0_5g_grp5_rx_high_gain =
				-((value & RF0_5G_GRP5_RX_HIGH_GAIN_MASK) >> 8);

		if (value & RF1_5G_GRP5_RX_HIGH_GAIN_SIGN)
			cap->rf1_5g_grp5_rx_high_gain =
				((value & RF1_5G_GRP5_RX_HIGH_GAIN_MASK) >> 12);
		else
			cap->rf1_5g_grp5_rx_high_gain =
				-((value & RF1_5G_GRP5_RX_HIGH_GAIN_MASK) >> 12);
	}
}

static void mt76x2_get_tx_pwr_info(struct rtmp_adapter *ad)
{
	u16 value;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	value = mt7612u_read_eeprom16(ad, G_BAND_20_40_BW_PWR_DELTA);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->delta_tx_pwr_bw40_g_band = 0;
	} else {
		if (value & G_BAND_20_40_BW_PWR_DELTA_EN) {
			if (value & G_BAND_20_40_BW_PWR_DELTA_SIGN)
				cap->delta_tx_pwr_bw40_g_band =
					value & G_BAND_20_40_BW_PWR_DELTA_MASK;
			else
				cap->delta_tx_pwr_bw40_g_band =
					-(value & G_BAND_20_40_BW_PWR_DELTA_MASK);
		} else {
			cap->delta_tx_pwr_bw40_g_band = 0;
		}
	}

	if (((value & 0xff00) == 0x0000) || ((value & 0xff00) == 0xff00)) {
		cap->delta_tx_pwr_bw40_a_band = 0;
	} else {
		if (value & A_BAND_20_40_BW_PWR_DELTA_EN) {
			if (value & A_BAND_20_40_BW_PWR_DELTA_SIGN)
				cap->delta_tx_pwr_bw40_a_band =
					(value & A_BAND_20_40_BW_PWR_DELTA_MASK) >> 8;
			else
				cap->delta_tx_pwr_bw40_a_band =
					-((value & A_BAND_20_40_BW_PWR_DELTA_MASK) >> 8);
		} else {
			cap->delta_tx_pwr_bw40_a_band = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, A_BAND_20_80_BW_PWR_DELTA);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->delta_tx_pwr_bw80 = 0;
	} else {
		if (value & A_BAND_20_80_BW_PWR_DELTA_EN) {
			if (value & A_BAND_20_80_BW_PWR_DELTA_SIGN)
				cap->delta_tx_pwr_bw80 =
					value & A_BAND_20_80_BW_PWR_DELTA_MASK;
			else
				cap->delta_tx_pwr_bw80 =
					-(value & A_BAND_20_80_BW_PWR_DELTA_MASK);
		} else {
			cap->delta_tx_pwr_bw80 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX0_G_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tssi_0_slope_g_band =
			TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE;
	} else {
		cap->tssi_0_slope_g_band =
			value & TX0_G_BAND_TSSI_SLOPE_MASK;
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tssi_0_offset_g_band =
			TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE;
	} else {
		cap->tssi_0_offset_g_band =
			(value & TX0_G_BAND_TSSI_OFFSET_MASK) >> 8;
	}

	value = mt7612u_read_eeprom16(ad, TX0_G_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_target_pwr_g_band =
			TX_TARGET_PWR_DEFAULT_VALUE;
	} else {
		cap->tx_0_target_pwr_g_band =
			value & TX0_G_BAND_TARGET_PWR_MASK;
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0;
	} else {
		if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] =
					(value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] =
					-((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX0_G_BAND_CHL_PWR_DELTA_MID);

	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0;
	} else {
		if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_EN) {
			if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN)
				cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] =
					value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK;
			else
				cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] =
					-(value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0;
	} else {
		if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] =
					(value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] =
					-((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX1_G_BAND_TSSI_SLOPE);

	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_slope_g_band =
			TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_g_band =
			(value & TX1_G_BAND_TSSI_SLOPE_MASK);

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_offset_g_band =
			TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_g_band =
			(value & TX1_G_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, TX1_G_BAND_TARGET_PWR);

	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_1_target_pwr_g_band =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_g_band =
			(value & TX1_G_BAND_TARGET_PWR_MASK);

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] =  0;
	} else {
		if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] =
					(value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] =
					-((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX1_G_BAND_CHL_PWR_DELTA_MID);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
	} else {
		if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_EN) {
			if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN)
				cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] =
					value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK;
			else
				cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] =
					-(value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
	} else {
		if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] =
					(value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] =
					-((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP0_TX0_A_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_0_slope_a_band[A_BAND_GRP0_CHL] =
			TSSI_0_SLOPE_A_BAND_GRP0_DEFAULT_VALUE;
	else
		cap->tssi_0_slope_a_band[A_BAND_GRP0_CHL] =
			value & GRP0_TX0_A_BAND_TSSI_SLOPE_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_0_offset_a_band[A_BAND_GRP0_CHL] =
			TSSI_0_OFFSET_A_BAND_GRP0_DEFAULT_VALUE;
	else
		cap->tssi_0_offset_a_band[A_BAND_GRP0_CHL] =
			(value & GRP0_TX0_A_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP0_TX0_A_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_0_target_pwr_a_band[A_BAND_GRP0_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_0_target_pwr_a_band[A_BAND_GRP0_CHL] =
			value & GRP0_TX0_A_BAND_TARGET_PWR_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] =
					(value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] =
					-((value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] =
					value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] =
					-(value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_0_slope_a_band[A_BAND_GRP1_CHL] =
			TSSI_0_SLOPE_A_BAND_GRP1_DEFAULT_VALUE;
	else
		cap->tssi_0_slope_a_band[A_BAND_GRP1_CHL] =
			(value & GRP1_TX0_A_BAND_TSSI_SLOPE_MASK) >> 8;


	value = mt7612u_read_eeprom16(ad, GRP1_TX0_A_BAND_TSSI_OFFSET);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_0_offset_a_band[A_BAND_GRP1_CHL] =
			TSSI_0_OFFSET_A_BAND_GRP1_DEFAULT_VALUE;
	else
		cap->tssi_0_offset_a_band[A_BAND_GRP1_CHL] =
			value & GRP1_TX0_A_BAND_TSSI_OFFSET_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tx_0_target_pwr_a_band[A_BAND_GRP1_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_0_target_pwr_a_band[A_BAND_GRP1_CHL] =
			(value & GRP1_TX0_A_BAND_TARGET_PWR_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] =
					value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] =
					-(value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] =
					(value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] =
					-((value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP0_TX0_A_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_0_slope_a_band[A_BAND_GRP2_CHL] =
			TSSI_0_SLOPE_A_BAND_GRP2_DEFAULT_VALUE;
	else
		cap->tssi_0_slope_a_band[A_BAND_GRP2_CHL] =
			value & GRP2_TX0_A_BAND_TSSI_SLOPE_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_0_offset_a_band[A_BAND_GRP2_CHL] =
			TSSI_0_OFFSET_A_BAND_GRP2_DEFAULT_VALUE;
	else
		cap->tssi_0_offset_a_band[A_BAND_GRP2_CHL] =
			(value & GRP2_TX0_A_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP2_TX0_A_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_0_target_pwr_a_band[A_BAND_GRP2_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_0_target_pwr_a_band[A_BAND_GRP2_CHL] =
			value & GRP2_TX0_A_BAND_TARGET_PWR_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] =
					(value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] =
					-((value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] =
					value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] =
					-(value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_0_slope_a_band[A_BAND_GRP3_CHL] =
			TSSI_0_SLOPE_A_BAND_GRP3_DEFAULT_VALUE;
	else
		cap->tssi_0_slope_a_band[A_BAND_GRP3_CHL] =
			(value & GRP3_TX0_A_BAND_TSSI_SLOPE_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP3_TX0_A_BAND_TSSI_OFFSET);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_0_offset_a_band[A_BAND_GRP3_CHL] = TSSI_0_OFFSET_A_BAND_GRP3_DEFAULT_VALUE;
	else
		cap->tssi_0_offset_a_band[A_BAND_GRP3_CHL] = (value & GRP3_TX0_A_BAND_TSSI_OFFSET_MASK);

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tx_0_target_pwr_a_band[A_BAND_GRP3_CHL] = TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_0_target_pwr_a_band[A_BAND_GRP3_CHL] = ((value & GRP3_TX0_A_BAND_TARGET_PWR_MASK) >> 8);

	value = mt7612u_read_eeprom16(ad, GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] =
					value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] =
					-(value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] =
					(value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] =
					-((value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP4_TX0_A_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_0_slope_a_band[A_BAND_GRP4_CHL] =
			TSSI_0_SLOPE_A_BAND_GRP4_DEFAULT_VALUE;
	else
		cap->tssi_0_slope_a_band[A_BAND_GRP4_CHL] =
			value & GRP4_TX0_A_BAND_TSSI_SLOPE_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_0_offset_a_band[A_BAND_GRP4_CHL] =
			TSSI_0_OFFSET_A_BAND_GRP4_DEFAULT_VALUE;
	else
		cap->tssi_0_offset_a_band[A_BAND_GRP4_CHL] =
			(value & GRP4_TX0_A_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP4_TX0_A_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_0_target_pwr_a_band[A_BAND_GRP4_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_0_target_pwr_a_band[A_BAND_GRP4_CHL] =
			value & GRP4_TX0_A_BAND_TARGET_PWR_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] =
					(value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] =
					-((value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] =
					value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] =
					-(value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_0_slope_a_band[A_BAND_GRP5_CHL] =
			TSSI_0_SLOPE_A_BAND_GRP5_DEFAULT_VALUE;
	else
		cap->tssi_0_slope_a_band[A_BAND_GRP5_CHL] =
			(value & GRP5_TX0_A_BAND_TSSI_SLOPE_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP5_TX0_A_BAND_TSSI_OFFSET);

	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_0_offset_a_band[A_BAND_GRP5_CHL] =
			TSSI_0_OFFSET_A_BAND_GRP5_DEFAULT_VALUE;
	else
		cap->tssi_0_offset_a_band[A_BAND_GRP5_CHL] =
			value & GRP5_TX0_A_BAND_TSSI_OFFSET_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tx_0_target_pwr_a_band[A_BAND_GRP5_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_0_target_pwr_a_band[A_BAND_GRP5_CHL] =
			(value & GRP5_TX0_A_BAND_TARGET_PWR_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] =
					value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] =
					-(value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] =
					(value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] =
					-((value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_0_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] = 0;
		}
	}

	/* 5G TX1 chl pwr */
	value = mt7612u_read_eeprom16(ad, GRP0_TX1_A_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_slope_a_band[A_BAND_GRP0_CHL] =
			TSSI_1_SLOPE_A_BAND_GRP0_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_a_band[A_BAND_GRP0_CHL] =
			value & GRP0_TX1_A_BAND_TSSI_SLOPE_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_offset_a_band[A_BAND_GRP0_CHL] =
			TSSI_1_OFFSET_A_BAND_GRP0_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_a_band[A_BAND_GRP0_CHL] =
			(value & GRP0_TX1_A_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP0_TX1_A_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_1_target_pwr_a_band[A_BAND_GRP0_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_a_band[A_BAND_GRP0_CHL] =
			value & GRP0_TX1_A_BAND_TARGET_PWR_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] =
					(value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] =
					-((value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] =
					value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] =
					-(value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP0_CHL][A_BAND_HI] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_slope_a_band[A_BAND_GRP1_CHL] =
			TSSI_1_SLOPE_A_BAND_GRP1_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_a_band[A_BAND_GRP1_CHL] =
			(value & GRP1_TX1_A_BAND_TSSI_SLOPE_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP1_TX1_A_BAND_TSSI_OFFSET);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_offset_a_band[A_BAND_GRP1_CHL] =
			TSSI_1_OFFSET_A_BAND_GRP1_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_a_band[A_BAND_GRP1_CHL] =
			value & GRP1_TX1_A_BAND_TSSI_OFFSET_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tx_1_target_pwr_a_band[A_BAND_GRP1_CHL] = TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_a_band[A_BAND_GRP1_CHL] =
			(value & GRP1_TX1_A_BAND_TARGET_PWR_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] =
					value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] =
					-(value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_LOW] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] =
					(value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] =
					-((value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP1_CHL][A_BAND_HI] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP0_TX1_A_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_slope_a_band[A_BAND_GRP2_CHL] =
			TSSI_1_SLOPE_A_BAND_GRP2_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_a_band[A_BAND_GRP2_CHL] =
			value & GRP2_TX1_A_BAND_TSSI_SLOPE_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_offset_a_band[A_BAND_GRP2_CHL] =
			TSSI_1_OFFSET_A_BAND_GRP2_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_a_band[A_BAND_GRP2_CHL] =
			(value & GRP2_TX1_A_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP2_TX1_A_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_1_target_pwr_a_band[A_BAND_GRP2_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_a_band[A_BAND_GRP2_CHL] =
			value & GRP2_TX1_A_BAND_TARGET_PWR_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] =
					(value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] =
					-((value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] =
					value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] =
					-(value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP2_CHL][A_BAND_HI] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_slope_a_band[A_BAND_GRP3_CHL] =
			TSSI_1_SLOPE_A_BAND_GRP3_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_a_band[A_BAND_GRP3_CHL] =
			(value & GRP3_TX1_A_BAND_TSSI_SLOPE_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP3_TX1_A_BAND_TSSI_OFFSET);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_offset_a_band[A_BAND_GRP3_CHL] =
			TSSI_1_OFFSET_A_BAND_GRP3_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_a_band[A_BAND_GRP3_CHL] =
			value & GRP3_TX1_A_BAND_TSSI_OFFSET_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tx_1_target_pwr_a_band[A_BAND_GRP3_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_a_band[A_BAND_GRP3_CHL] =
			(value & GRP3_TX1_A_BAND_TARGET_PWR_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] =
					value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] =
					-(value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_LOW] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] =
					(value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] =
					-((value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP3_CHL][A_BAND_HI] = 0;
		}
	}


	value = mt7612u_read_eeprom16(ad, GRP4_TX1_A_BAND_TSSI_SLOPE);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_slope_a_band[A_BAND_GRP4_CHL] =
			TSSI_1_SLOPE_A_BAND_GRP4_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_a_band[A_BAND_GRP4_CHL] =
			value & GRP4_TX1_A_BAND_TSSI_SLOPE_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_offset_a_band[A_BAND_GRP4_CHL] =
			TSSI_1_OFFSET_A_BAND_GRP4_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_a_band[A_BAND_GRP4_CHL] =
			(value & GRP4_TX1_A_BAND_TSSI_OFFSET_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP4_TX1_A_BAND_TARGET_PWR);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tx_1_target_pwr_a_band[A_BAND_GRP4_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_a_band[A_BAND_GRP4_CHL] =
			value & GRP4_TX1_A_BAND_TARGET_PWR_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] =
					(value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] =
					-((value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_LOW] = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] =
					value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] =
					-(value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP4_CHL][A_BAND_HI] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tssi_1_slope_a_band[A_BAND_GRP5_CHL] =
			TSSI_1_SLOPE_A_BAND_GRP5_DEFAULT_VALUE;
	else
		cap->tssi_1_slope_a_band[A_BAND_GRP5_CHL] =
			(value & GRP5_TX0_A_BAND_TSSI_SLOPE_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP5_TX1_A_BAND_TSSI_OFFSET);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
		cap->tssi_1_offset_a_band[A_BAND_GRP5_CHL] =
			TSSI_1_OFFSET_A_BAND_GRP5_DEFAULT_VALUE;
	else
		cap->tssi_1_offset_a_band[A_BAND_GRP5_CHL] =
			value & GRP5_TX1_A_BAND_TSSI_OFFSET_MASK;

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
		cap->tx_1_target_pwr_a_band[A_BAND_GRP5_CHL] =
			TX_TARGET_PWR_DEFAULT_VALUE;
	else
		cap->tx_1_target_pwr_a_band[A_BAND_GRP5_CHL] =
			(value & GRP5_TX1_A_BAND_TARGET_PWR_MASK) >> 8;

	value = mt7612u_read_eeprom16(ad, GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] = 0;
	} else {
		if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
			if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] =
					value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] =
					-(value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_LOW] = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] = 0;
	} else {
		if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
			if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] =
					(value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8;
			else
				cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] =
					-((value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
		} else {
			cap->tx_1_chl_pwr_delta_a_band[A_BAND_GRP5_CHL][A_BAND_HI] = 0;
		}
	}

	/* check tssi if enable */
	value = mt7612u_read_eeprom16(ad, NIC_CONFIGURE_1);
	if (value & INTERNAL_TX_ALC_EN)
		cap->tssi_enable = TRUE;
	else
		cap->tssi_enable = FALSE;

	/* check PA type combination */
	value = mt7612u_read_eeprom16(ad, EEPROM_NIC1_OFFSET);
	cap->PAType= GET_PA_TYPE(value);
	DBGPRINT(RT_DEBUG_OFF, ("PA Type %x\n", cap->PAType));
}

static u8 mt76x2_txpwr_chlist[] = {
	1, 2,3,4,5,6,7,8,9,10,11,12,13,14,
	36, 38,40,44,46,48,52,54,56,60,62,64,
	100,102,104,108,110,112,116,118,120,124,126,128,132,134,136,140,
	149,151,153,157,159,161,165,167,169,171,173,
	42, 58, 106, 122, 155,
};

int mt76x2_read_chl_pwr(struct rtmp_adapter *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	u32 i, choffset;

	mt76x2_get_tx_pwr_info(ad);

	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

	DBGPRINT(RT_DEBUG_TRACE, ("%s()--->\n", __FUNCTION__));

	for (i = 0; i < sizeof(mt76x2_txpwr_chlist); i++) {
		ad->TxPower[i].Channel = mt76x2_txpwr_chlist[i];
	}

	/* 0. 11b/g, ch1 - ch 14, 2SS */

	/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
	choffset = 14;
	ASSERT((ad->TxPower[choffset].Channel == 36));

	choffset = 14 + 12 + 16 + 11;

	ASSERT((ad->TxPower[choffset].Channel == 42));

	// TODO: shiang-6590, fix me for the TxPower setting code here!
	/* For VHT80MHz, we need assign tx power for central channel 42, 58, 106, 122, and 155 */
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Update Tx power control of the central channel (42, 58, 106, 122 and 155) for VHT BW80\n", __FUNCTION__));

	memmove(&ad->TxPower[53], &ad->TxPower[16], sizeof(struct CHANNEL_TX_POWER)); // channel 42 = channel 40
	memmove(&ad->TxPower[54], &ad->TxPower[22], sizeof(struct CHANNEL_TX_POWER)); // channel 58 = channel 56
	memmove(&ad->TxPower[55], &ad->TxPower[28], sizeof(struct CHANNEL_TX_POWER)); // channel 106 = channel 104
	memmove(&ad->TxPower[56], &ad->TxPower[34], sizeof(struct CHANNEL_TX_POWER)); // channel 122 = channel 120
	memmove(&ad->TxPower[57], &ad->TxPower[44], sizeof(struct CHANNEL_TX_POWER)); // channel 155 = channel 153

	ad->TxPower[choffset].Channel = 42;
	ad->TxPower[choffset+1].Channel = 58;
	ad->TxPower[choffset+2].Channel = 106;
	ad->TxPower[choffset+3].Channel = 122;
	ad->TxPower[choffset+4].Channel = 155;

	choffset += 5;		/* the central channel of VHT80 */
	choffset = (MAX_NUM_OF_CHANNELS - 1);

	return TRUE;
}

void mt76x2_get_tx_pwr_per_rate(struct rtmp_adapter *ad)
{
	u16 value;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	value = mt7612u_read_eeprom16(ad, TX_PWR_CCK_1_2M);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_cck_1_2 = 0;
	} else {
		if (value & TX_PWR_CCK_1_2M_EN) {
			if (value & TX_PWR_CCK_1_2M_SIGN)
				cap->tx_pwr_cck_1_2 =
					value & TX_PWR_CCK_1_2M_MASK;
			else
				cap->tx_pwr_cck_1_2 =
					-(value & TX_PWR_CCK_1_2M_MASK);
		} else {
			cap->tx_pwr_cck_1_2 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_cck_5_11 = 0;
	} else {
		if (value & TX_PWR_CCK_5_11M_EN) {
			if (value & TX_PWR_CCK_5_11M_SIGN)
				cap->tx_pwr_cck_5_11 =
					(value & TX_PWR_CCK_5_11M_MASK) >> 8;
			else
				cap->tx_pwr_cck_5_11 =
					-((value & TX_PWR_CCK_5_11M_MASK) >> 8);
		} else {
			cap->tx_pwr_cck_5_11 = 0;
		}


	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_G_BAND_OFDM_6_9M);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_g_band_ofdm_6_9 = 0;
	} else {
		if (value & TX_PWR_G_BAND_OFDM_6_9M_EN) {
			if (value & TX_PWR_G_BAND_OFDM_6_9M_SIGN)
				cap->tx_pwr_g_band_ofdm_6_9 =
					value & TX_PWR_G_BAND_OFDM_6_9M_MASK;
			else
				cap->tx_pwr_g_band_ofdm_6_9 =
					-(value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
		} else {
			cap->tx_pwr_g_band_ofdm_6_9 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_g_band_ofdm_12_18 = 0;
	} else {
		if (value & TX_PWR_G_BAND_OFDM_12_18M_EN) {
			if (value & TX_PWR_G_BAND_OFDM_12_18M_SIGN)
				cap->tx_pwr_g_band_ofdm_12_18 =
					(value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8;
			else
				cap->tx_pwr_g_band_ofdm_12_18 =
					-((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
		} else {
			cap->tx_pwr_g_band_ofdm_12_18 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_G_BAND_OFDM_24_36M);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_g_band_ofdm_24_36 = 0;
	} else {
		if (value & TX_PWR_G_BAND_OFDM_24_36M_EN) {
			if (value & TX_PWR_G_BAND_OFDM_24_36M_SIGN)
				cap->tx_pwr_g_band_ofdm_24_36 =
					value & TX_PWR_G_BAND_OFDM_24_36M_MASK;
			else
				cap->tx_pwr_g_band_ofdm_24_36 =
					-(value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
		} else {
			cap->tx_pwr_g_band_ofdm_24_36 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_g_band_ofdm_48_54 = 0;
	} else {
		if (value & TX_PWR_G_BAND_OFDM_48_54M_EN) {
			if (value & TX_PWR_G_BAND_OFDM_48_54M_SIGN)
				cap->tx_pwr_g_band_ofdm_48_54 =
					(value & TX_PWR_G_BAND_OFDM_48_54M_MASK) >> 8;
			else
				cap->tx_pwr_g_band_ofdm_48_54 =
					-((value & TX_PWR_G_BAND_OFDM_48_54M_MASK) >> 8);
		} else {
			cap->tx_pwr_g_band_ofdm_48_54 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_HT_MCS_0_1);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_ht_mcs_0_1 = (value & TX_PWR_HT_MCS_0_1_MASK);
	} else {
		if (value & TX_PWR_HT_MCS_0_1_EN) {
			if (value & TX_PWR_HT_MCS_0_1_SIGN)
				cap->tx_pwr_ht_mcs_0_1 =
					value & TX_PWR_HT_MCS_0_1_MASK;
			else
				cap->tx_pwr_ht_mcs_0_1 =
					-(value & TX_PWR_HT_MCS_0_1_MASK);
		} else {
			cap->tx_pwr_ht_mcs_0_1 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_ht_mcs_2_3 = ((value & TX_PWR_HT_MCS_2_3_MASK) >> 8);
	} else {
		if (value & TX_PWR_HT_MCS_2_3_EN) {
			if (value & TX_PWR_HT_MCS_2_3_SIGN)
				cap->tx_pwr_ht_mcs_2_3 =
					(value & TX_PWR_HT_MCS_2_3_MASK) >> 8;
			else
				cap->tx_pwr_ht_mcs_2_3 =
					-((value & TX_PWR_HT_MCS_2_3_MASK) >> 8);
		} else {
			cap->tx_pwr_ht_mcs_2_3 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_HT_MCS_4_5);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_ht_mcs_4_5 = 0;
	} else {
		if (value & TX_PWR_HT_MCS_4_5_EN) {
			if (value & TX_PWR_HT_MCS_4_5_SIGN)
				cap->tx_pwr_ht_mcs_4_5 =
					value & TX_PWR_HT_MCS_4_5_MASK;
			else
				cap->tx_pwr_ht_mcs_4_5 =
					-(value & TX_PWR_HT_MCS_4_5_MASK);
		} else {
			cap->tx_pwr_ht_mcs_4_5 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_ht_mcs_6_7 = 0;
	} else {
		if (value & TX_PWR_HT_MCS_6_7_EN) {
			if (value & TX_PWR_HT_MCS_6_7_SIGN)
				cap->tx_pwr_ht_mcs_6_7 =
					(value & TX_PWR_HT_MCS_6_7_MASK) >> 8;
			else
				cap->tx_pwr_ht_mcs_6_7 =
					-((value & TX_PWR_HT_MCS_6_7_MASK) >> 8);
		} else {
			cap->tx_pwr_ht_mcs_6_7 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_HT_MCS_8_9);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_ht_mcs_8_9 = 0;
	} else {
		if (value & TX_PWR_HT_MCS_8_9_EN) {
			if (value & TX_PWR_HT_MCS_8_9_SIGN)
				cap->tx_pwr_ht_mcs_8_9 =
					value & TX_PWR_HT_MCS_8_9_MASK;
			else
				cap->tx_pwr_ht_mcs_8_9 =
					-(value & TX_PWR_HT_MCS_8_9_MASK);
		} else {
			cap->tx_pwr_ht_mcs_8_9 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_ht_mcs_10_11 = 0;
	} else {
		if (value & TX_PWR_HT_MCS_10_11_EN) {
			if (value & TX_PWR_HT_MCS_10_11_SIGN)
				cap->tx_pwr_ht_mcs_10_11 =
					(value & TX_PWR_HT_MCS_10_11_MASK) >> 8;
			else
				cap->tx_pwr_ht_mcs_10_11 =
					-((value & TX_PWR_HT_MCS_10_11_MASK) >> 8);
		} else {
			cap->tx_pwr_ht_mcs_10_11 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_HT_MCS_12_13);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_ht_mcs_12_13 = 0;
	} else {
		if (value & TX_PWR_HT_MCS_12_13_EN) {
			if (value & TX_PWR_HT_MCS_12_13_SIGN)
				cap->tx_pwr_ht_mcs_12_13 =
					value & TX_PWR_HT_MCS_12_13_MASK;
			else
				cap->tx_pwr_ht_mcs_12_13 =
					-(value & TX_PWR_HT_MCS_12_13_MASK);
		} else {
			cap->tx_pwr_ht_mcs_12_13 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_ht_mcs_14_15 = 0;
	} else {
		if (value & TX_PWR_HT_MCS_14_15_EN) {
			if (value & TX_PWR_HT_MCS_14_15_SIGN)
				cap->tx_pwr_ht_mcs_14_15 =
					(value & TX_PWR_HT_MCS_14_15_MASK) >> 8;
			else
				cap->tx_pwr_ht_mcs_14_15 =
					-((value & TX_PWR_HT_MCS_14_15_MASK) >> 8);
		} else {
			cap->tx_pwr_ht_mcs_14_15 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_A_BAND_OFDM_6_9M);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_a_band_ofdm_6_9 = 0;
	} else {
		if (value & TX_PWR_A_BAND_OFDM_6_9M_EN) {
			if (value & TX_PWR_A_BAND_OFDM_6_9M_SIGN)
				cap->tx_pwr_a_band_ofdm_6_9 =
					value & TX_PWR_A_BAND_OFDM_6_9M_MASK;
			else
				cap->tx_pwr_a_band_ofdm_6_9 =
					-(value & TX_PWR_A_BAND_OFDM_6_9M_MASK);
		} else {
			cap->tx_pwr_a_band_ofdm_6_9 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_a_band_ofdm_12_18 = 0;
	} else {
		if (value & TX_PWR_A_BAND_OFDM_12_18M_EN) {
			if (value & TX_PWR_A_BAND_OFDM_12_18M_SIGN)
				cap->tx_pwr_a_band_ofdm_12_18 =
					(value & TX_PWR_A_BAND_OFDM_12_18M_MASK) >> 8;
			else
				cap->tx_pwr_a_band_ofdm_12_18 =
					-((value & TX_PWR_A_BAND_OFDM_12_18M_MASK) >> 8);
		} else {
			cap->tx_pwr_a_band_ofdm_12_18 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_A_BAND_OFDM_24_36M);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_a_band_ofdm_24_36 = 0;
	} else {
		if (value & TX_PWR_A_BAND_OFDM_24_36M_EN) {
			if (value & TX_PWR_A_BAND_OFDM_24_36M_SIGN)
				cap->tx_pwr_a_band_ofdm_24_36 =
					value & TX_PWR_A_BAND_OFDM_24_36M_MASK;
			else
				cap->tx_pwr_a_band_ofdm_24_36 =
					-(value & TX_PWR_A_BAND_OFDM_24_36M_MASK);
		} else {
			cap->tx_pwr_a_band_ofdm_24_36 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_a_band_ofdm_48_54 = 0;
	} else {
		if (value & TX_PWR_A_BAND_OFDM_48_54M_EN) {
			if (value & TX_PWR_A_BAND_OFDM_48_54M_SIGN)
				cap->tx_pwr_a_band_ofdm_48_54 =
					(value & TX_PWR_A_BAND_OFDM_48_54M_MASK) >> 8;
			else
				cap->tx_pwr_a_band_ofdm_48_54 =
					-((value & TX_PWR_A_BAND_OFDM_48_54M_MASK) >> 8);
		} else {
			cap->tx_pwr_a_band_ofdm_48_54 = 0;
		}
	}


	value = mt7612u_read_eeprom16(ad, TX_PWR_VHT_MCS_0_1);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_vht_mcs_0_1 = 0;
	} else {
		if (value & TX_PWR_VHT_MCS_0_1_EN) {
			if (value & TX_PWR_VHT_MCS_0_1_SIGN)
				cap->tx_pwr_vht_mcs_0_1 =
					value & TX_PWR_VHT_MCS_0_1_MASK;
			else
				cap->tx_pwr_vht_mcs_0_1 =
					-(value & TX_PWR_VHT_MCS_0_1_MASK);
		} else {
			cap->tx_pwr_vht_mcs_0_1 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_vht_mcs_2_3 = 0;
	} else {
		if (value & TX_PWR_VHT_MCS_2_3_EN) {
			if (value & TX_PWR_VHT_MCS_2_3_SIGN)
				cap->tx_pwr_vht_mcs_2_3 =
					(value & TX_PWR_VHT_MCS_2_3_MASK) >> 8;
			else
				cap->tx_pwr_vht_mcs_2_3 =
					-((value & TX_PWR_VHT_MCS_2_3_MASK) >> 8);
		} else {
			cap->tx_pwr_vht_mcs_2_3 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_VHT_MCS_4_5);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_vht_mcs_4_5 = 0;
	} else {
		if (value & TX_PWR_VHT_MCS_4_5_EN) {
			if (value & TX_PWR_VHT_MCS_4_5_SIGN)
				cap->tx_pwr_vht_mcs_4_5 =
					value & TX_PWR_VHT_MCS_4_5_MASK;
			else
				cap->tx_pwr_vht_mcs_4_5 =
					-(value & TX_PWR_VHT_MCS_4_5_MASK);
		} else {
			cap->tx_pwr_vht_mcs_4_5 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_vht_mcs_6_7 = 0;
	} else {
		if (value & TX_PWR_VHT_MCS_6_7_EN) {
			if (value & TX_PWR_VHT_MCS_6_7_SIGN)
				cap->tx_pwr_vht_mcs_6_7 =
					(value & TX_PWR_VHT_MCS_6_7_MASK) >> 8;
			else
				cap->tx_pwr_vht_mcs_6_7 =
					-((value & TX_PWR_VHT_MCS_6_7_MASK) >> 8);
		} else {
			cap->tx_pwr_vht_mcs_6_7 = 0;
		}
	}

	value = mt7612u_read_eeprom16(ad, TX_PWR_5G_VHT_MCS_8_9);
	if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
		cap->tx_pwr_5g_vht_mcs_8_9 = 0;
	} else {
		if (value & TX_PWR_5G_VHT_MCS_8_9_EN) {
			if (value & TX_PWR_5G_VHT_MCS_8_9_SIGN)
				cap->tx_pwr_5g_vht_mcs_8_9 =
					value & TX_PWR_5G_VHT_MCS_8_9_MASK;
			else
				cap->tx_pwr_5g_vht_mcs_8_9 =
					-(value & TX_PWR_5G_VHT_MCS_8_9_MASK);
		} else {
			cap->tx_pwr_5g_vht_mcs_8_9 = 0;
		}
	}

	if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
		cap->tx_pwr_2g_vht_mcs_8_9 = 0;
	} else {
		if (value & TX_PWR_2G_VHT_MCS_8_9_EN) {
			if (value & TX_PWR_2G_VHT_MCS_8_9_SIGN)
				cap->tx_pwr_2g_vht_mcs_8_9 =
					(value & TX_PWR_2G_VHT_MCS_8_9_MASK) >> 8;
			else
				cap->tx_pwr_2g_vht_mcs_8_9 =
					-((value & TX_PWR_2G_VHT_MCS_8_9_MASK) >> 8);
		} else {
			cap->tx_pwr_2g_vht_mcs_8_9 = 0;
		}
	}
}

void percentage_delta_pwr(struct rtmp_adapter *ad)
{
	CHAR mac_drop_pwr = 0, tx_alc_ch_init_0 = 0, tx_alc_ch_init_1 = 0;
	uint32_t mac_val = 0;

	/*
		Calculate delta power based on the percentage specified from UI.
		EEPROM setting is calibrated for maximum Tx power (i.e. 100%).
		Here lower Tx power according to the percentage specified from UI.
	*/

	if (ad->CommonCfg.TxPowerPercentage > 90) {
		/* 91 ~ 100% & auto, treat as 100% in terms of mW */
		;
	} else if (ad->CommonCfg.TxPowerPercentage > 60) {
		/* 61 ~ 90%, treat as 75% in terms of mW */
		mac_drop_pwr -= 1;
	} else if (ad->CommonCfg.TxPowerPercentage > 30) {
		/* 31 ~ 60%, treat as 50% in terms of mW */
		mac_drop_pwr -= 3;
	} else if (ad->CommonCfg.TxPowerPercentage > 15) {
		/* 16 ~ 30%, treat as 25% in terms of mW */
		mac_drop_pwr -= 6;
	} else if (ad->CommonCfg.TxPowerPercentage > 9) {
		/* 10 ~ 15%, treat as 12.5% in terms of mW */
		mac_drop_pwr -= 9;
	} else {
		/* 0 ~ 9 %, treat as MIN(~3%) in terms of mW */
		mac_drop_pwr -= 12;
	}

	mac_val = mt7612u_read32(ad, TX_ALC_CFG_0);
	tx_alc_ch_init_0 = (mac_val & 0x3F) + mac_drop_pwr*2;
	if (tx_alc_ch_init_0 <= 0)
		tx_alc_ch_init_0 = 0;
	tx_alc_ch_init_1 = ((mac_val & 0x3F00) >> 8) + mac_drop_pwr*2;
	if (tx_alc_ch_init_1<= 0)
		tx_alc_ch_init_1 = 0;
	DBGPRINT(RT_DEBUG_INFO, ("%s::<Before> TX_ALC_CFG_0=0x%0x, tx_alc_ch_init_0=0x%0x, tx_alc_ch_init_1=0x%0x\n",
		__FUNCTION__, mac_val, tx_alc_ch_init_0, tx_alc_ch_init_1));

	mac_val = mac_val & (~TX_ALC_CFG_0_CH_INT_0_MASK);
	mac_val |= TX_ALC_CFG_0_CH_INT_0(tx_alc_ch_init_0);
	mac_val = mac_val & (~TX_ALC_CFG_0_CH_INT_1_MASK);
	mac_val |= TX_ALC_CFG_0_CH_INT_1(tx_alc_ch_init_1);
	mt7612u_write32(ad, TX_ALC_CFG_0, mac_val);

	DBGPRINT(RT_DEBUG_INFO, ("%s::<After> TX_ALC_CFG_0=0x%0x\n",
		__FUNCTION__, mac_val));
}

void mt76x2_get_current_temp(struct rtmp_adapter *ad)
{
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;
	int32_t temp_val = 0;

	mt7612u_read_reg(ad, 0xD000, &temp_val);
	temp_val &= 0x7F;

	if ( pChipCap->temp_25_ref == 0 ) {
		pChipCap->current_temp = 25;
	} else {
		if (temp_val > pChipCap->temp_25_ref)
		        pChipCap->current_temp = ((temp_val -pChipCap->temp_25_ref)*18/10) + 25; /* 1.789 */
		else if (temp_val < pChipCap->temp_25_ref)
			pChipCap->current_temp = 25 - ((pChipCap->temp_25_ref - temp_val)*18/10);
		else
		        pChipCap->current_temp = 25;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s::read_temp=%d (0x%x), current_temp=%d (0x%x)\n",
		__FUNCTION__, temp_val, temp_val, pChipCap->current_temp, pChipCap->current_temp));
}

void mt76x2_read_temp_info_from_eeprom(struct rtmp_adapter *ad)
{
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;
	BOOLEAN is_temp_tx_alc= FALSE;
	USHORT e2p_value = 0;

	e2p_value = mt7612u_read_eeprom16(ad, 0x36);
	if ((e2p_value & 0x2) == 0x2)
		is_temp_tx_alc = TRUE;
	else
		is_temp_tx_alc = FALSE;

	e2p_value = mt7612u_read_eeprom16(ad, 0x54);
	pChipCap->temp_25_ref = (e2p_value & 0x7F00) >> 8;
	if (((e2p_value & 0x8000) == 0x8000) && is_temp_tx_alc)
		pChipCap->temp_tx_alc_enable = TRUE;
	else
		pChipCap->temp_tx_alc_enable = FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: is_temp_tx_alc=%d, temp_tx_alc_enable=%d\n",
		__FUNCTION__, is_temp_tx_alc, pChipCap->temp_tx_alc_enable));
}

static const RTMP_CHIP_CAP MT76x2_ChipCap = {
	.max_nss = 2,
	.max_vht_mcs = VHT_MCS_CAP_9,
	.ac_off_mode = 0,
	.TXWISize = 20,
	.RXWISize = 28,
	.SnrFormula = SNR_FORMULA3,
	.FlgIsHwWapiSup = TRUE,
	.VcoPeriod = 10,
	.FlgIsVcoReCalMode = VCO_CAL_DISABLE,
	.FlgIsHwAntennaDiversitySup = FALSE,
	.Flg7662ChipCap = TRUE,
	.FlgHwTxBfCap = TRUE,
#ifdef FIFO_EXT_SUPPORT
	.FlgHwFifoExtCap = TRUE,
#endif /* FIFO_EXT_SUPPORT */
	.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT),
	.phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | fPHY_CAP_HT | fPHY_CAP_VHT | fPHY_CAP_LDPC),
	.MaxNumOfRfId = MAX_RF_ID,
	.pRFRegTable = NULL,
	.MaxNumOfBbpId = 200,
	.pBBPRegTable = NULL,
	.bbpRegTbSize = 0,
#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	.MBSSIDMode = MBSSID_MODE4,
#else
	.MBSSIDMode = MBSSID_MODE1,
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	.MBSSIDMode = MBSSID_MODE0,
#endif /* NEW_MBSSID_MODE */
#ifdef RTMP_EFUSE_SUPPORT
	.EFUSE_USAGE_MAP_START = 0x1e0,
	.EFUSE_USAGE_MAP_END = 0x1fc,
	.EFUSE_USAGE_MAP_SIZE = 29,
	.EFUSE_RESERVED_SIZE = 21,	// Cal-Free is 22 free block
#endif /* RTMP_EFUSE_SUPPORT */
	.EEPROM_DEFAULT_BIN = MT76x2_E2PImage,
	.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT76x2_E2PImage),
#ifdef CONFIG_ANDES_SUPPORT
	.WlanMemmapOffset = 0x410000,
	.InbandPacketMaxLen = 192,
	.CmdRspRxRing = RX_RING1,
	.IsComboChip = TRUE,
	.need_load_fw = TRUE,
	.need_load_rom_patch = TRUE,
	.ram_code_protect = FALSE,
	.rom_code_protect = TRUE,
	.load_iv = FALSE,
	.ilm_offset = 0x80000,
	.dlm_offset = 0x110000,
	.rom_patch_offset = 0x90000,
#endif /* CONFIG_ANDES_SUPPORT */
	.cmd_header_len = 4,
#ifdef RTMP_USB_SUPPORT
	.cmd_padding_len = 4,
	.CommandBulkOutAddr = 0x8,
	.WMM0ACBulkOutAddr[0] = 0x4,
	.WMM0ACBulkOutAddr[1] = 0x5,
	.WMM0ACBulkOutAddr[2] = 0x6,
	.WMM0ACBulkOutAddr[3] = 0x7,
	.WMM1ACBulkOutAddr	= 0x9,
	.DataBulkInAddr = 0x84,
	.CommandRspBulkInAddr = 0x85,
#endif
	.fw_name = "mt7662u.bin",
	.fw_patch_name = "mt7662u_rom_patch.bin",
	.rf_type = RF_MT,
#ifdef DYNAMIC_VGA_SUPPORT
	.dynamic_vga_support = TRUE,
	.compensate_level = 0,
	.dynamic_chE_mode = 0xFF,
	.dynamic_chE_trigger = FALSE,
	.avg_rssi_all = -90,
	.avg_rssi_0 = -90,
	.avg_rssi_1 = -90,
#ifdef CONFIG_AP_SUPPORT
	.dynamic_lna_trigger_timer = 1,
	.microwave_enable = TRUE,
#endif /* CONFIG_AP_SUPPORT */
#endif /* DYNAMIC_VGA_SUPPORT */
	.chl_smth_enable = TRUE,
	.ed_cca_enable = FALSE,
#ifdef DOT11W_PMF_SUPPORT
	.FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_1,
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
    /* Frequence Calibration */
    .FreqCalibrationSupport = TRUE,
    /* BBP CR for Rx OFDM/CCK frequency offset report is unnecessary */
    .FreqCalMode = FREQ_CAL_MODE2,
    .RxWIFrqOffset = RXWI_FRQ_OFFSET_FIELD0,
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef THERMAL_PROTECT_SUPPORT
	.ThermalProtectSup = FALSE;
#endif /* THERMAL_PROTECT_SUPPORT */
};

static const RTMP_CHIP_OP MT76x2_ChipOp = {
	.ChipBBPAdjust = mt76x2_bbp_adjust,
	.ChipSwitchChannel = mt76x2_switch_channel,
	.AsicMacInit = mt76x2_init_mac_cr,
	.AsicRfInit = mt76x2_init_rf_cr,

	.DisableTxRx = RT65xxDisableTxRx,
#ifdef RTMP_USB_SUPPORT
	.AsicRadioOn = RT65xxUsbAsicRadioOn,
	.AsicRadioOff = RT65xxUsbAsicRadioOff,
	.usb_cfg_read = usb_cfg_read_v3,
	.usb_cfg_write = usb_cfg_write_v3,
#endif
	.cal_test = mt76x2_cal_test,
#ifdef RTMP_TEMPERATURE_TX_ALC
	.TemperCompensation = mt76x2_temp_tx_alc,
#endif /* RTMP_TEMPERATURE_TX_ALC */
};

VOID mt76x2_init(struct rtmp_adapter *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	uint32_t mac_val = 0;

	memcpy(&pAd->chipCap, &MT76x2_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &MT76x2_ChipOp, sizeof(RTMP_CHIP_OP));

	rlt_phy_probe(pAd);

	if (IS_MT7612(pAd)) {
		pChipCap->IsComboChip = FALSE;
		pChipCap->rom_code_protect = FALSE;
	}

	if (IS_MT7632(pAd))
		pChipCap->phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | fPHY_CAP_HT | fPHY_CAP_LDPC);

	mac_val = mt7612u_read32(pAd, 0x38);

	if ((mac_val & 0x80000) == 0x80000)
		pChipCap->ac_off_mode = 1;




	pChipCap->asic_caps |= (fASIC_CAP_MCS_LUT);

	pAd->rateAlg = RATE_ALG_GRP;

	rlt_bcn_buf_init(pAd);


	pChipCap->tssi_stage = TSSI_INIT_STAGE;
}

static void patch_BBPL_on(struct rtmp_adapter *pAd)
{
	uint32_t value = 0;

	mt7612u_read_reg(pAd, 0x130, &value);
	value |= ((1<<16) | (1<<0));
	mt7612u_write_reg(pAd, 0x130, value);

	RtmpusecDelay(1);

	mt7612u_read_reg(pAd, 0x64, &value);
	if ((value >> 29) & 0x1) {
		mt7612u_read_reg(pAd, 0x1c, &value);
		value &= 0xFFFFFF00;
		mt7612u_write_reg(pAd, 0x1c, value);

		mt7612u_read_reg(pAd, 0x1c, &value);
		value |= 0x30;
		mt7612u_write_reg(pAd, 0x1c, value);
	} else {
		mt7612u_read_reg(pAd, 0x1c, &value);
		value &= 0xFFFFFF00;
		mt7612u_write_reg(pAd, 0x1c, value);

		mt7612u_read_reg(pAd, 0x1c, &value);
		value |= 0x30;
		mt7612u_write_reg(pAd, 0x1c, value);
	}

	value = 0x0000484F;
	mt7612u_write_reg(pAd, 0x14, value);

	RtmpusecDelay(1);

	mt7612u_read_reg(pAd, 0x130, &value);
	value |= (1<<17);
	mt7612u_write_reg(pAd, 0x130, value);

	RtmpusecDelay(125);

	mt7612u_read_reg(pAd, 0x130, &value);
	value  &= ~(1<<16);
	mt7612u_write_reg(pAd, 0x130, value);

	RtmpusecDelay(50);

	mt7612u_read_reg(pAd, 0x14C, &value);
	value  |= ((1<<20) | (1<<19));
	mt7612u_write_reg(pAd, 0x14C, value);
}

static VOID WF_CTRL(struct rtmp_adapter *pAd, UINT8 wfID, UINT8 isON)
{
	uint32_t value = 0;
	if(wfID == 0) {
		if(isON == 1) {	/* WIFI ON mode */
		/* Enable WF0 BG */
		mt7612u_read_reg(pAd, 0x130, &value);
		value |= (1<<0);
		mt7612u_write_reg(pAd, 0x130, value);

		RtmpusecDelay(10);

		/* Enable RFDIG LDO/AFE/ABB/ADDA */
		mt7612u_read_reg(pAd, 0x130, &value);
		value |= ((1<<1)|(1<<3)|(1<<4)|(1<<5));
		mt7612u_write_reg(pAd, 0x130, value);

		RtmpusecDelay(10);

		/* Switch WF0 RFDIG power to internal LDO */
		mt7612u_read_reg(pAd, 0x130, &value);
		value &= ~(1<<2);
		mt7612u_write_reg(pAd, 0x130, value);

		patch_BBPL_on(pAd);

		value = mt7612u_read32(pAd, 0x530);
		value  |= 0xF;
		mt7612u_write32(pAd, 0x530, value);
		}
	} else {
		if(isON == 1) {	/* WIFI ON mode */
			/* Enable WF1 BG */
			mt7612u_read_reg(pAd, 0x130, &value);
			value |= (1<<8);
			mt7612u_write_reg(pAd, 0x130, value);

			RtmpusecDelay(10);

			/* Enable RFDIG LDO/AFE/ABB/ADDA */
			mt7612u_read_reg(pAd, 0x130, &value);
			value |= ((1<<9)|(1<<11)|(1<<12)|(1<<13));
			mt7612u_write_reg(pAd, 0x130, value);

			RtmpusecDelay(10);
			/* Switch WF1 RFDIG power to internal LDO */
			mt7612u_read_reg(pAd, 0x130, &value);
			value &= ~(1<<10);
			mt7612u_write_reg(pAd, 0x130, value);

			patch_BBPL_on(pAd);

			value = mt7612u_read32(pAd, 0x530);
			value  |= 0xF;
			mt7612u_write32(pAd, 0x530, value);
		}
	}
}

static void WL_POWER_ON(struct rtmp_adapter *pAd)
{
	uint32_t cnt = 0;
	uint32_t regval = 0;
	uint32_t value = 0;

	mt7612u_read_reg(pAd, 0x148, &value);
	value |= 0x1;
	mt7612u_write_reg(pAd, 0x148, value); // turn on WL MTCMOS
	do {
		mt7612u_read_reg(pAd, 0x148, &regval);

		if((((regval>>28) & 0x1) == 0x1) &&
		   (((regval>>12) & 0x3) == 0x3))
			break;

		RtmpusecDelay(10);
		cnt++;
	} while (cnt < 100);

	mt7612u_read_reg(pAd, 0x148, &value);
	value &= ~(0x7F<<16);
	mt7612u_write_reg(pAd, 0x148, value);

	RtmpusecDelay(10);
	mt7612u_read_reg(pAd, 0x148, &value);
	value &= ~(0xF<<24);
	mt7612u_write_reg(pAd, 0x148, value);
	RtmpusecDelay(10);

	mt7612u_read_reg(pAd, 0x148, &value);
	value |= (0xF<<24);
	mt7612u_write_reg(pAd, 0x148, value);

	mt7612u_read_reg(pAd, 0x148, &value);
	value &= ~(0xFFF);
	mt7612u_write_reg(pAd, 0x148, value);

	/* Set 1'b0 to turn on AD/DA power down */
	mt7612u_read_reg(pAd, 0x1204, &value);
	value &= ~(0x1<<3);
	mt7612u_write_reg(pAd, 0x1204, value);

	/* WLAN function enable */
	mt7612u_read_reg(pAd, 0x80, &value);
	value |= (0x1<<0);
	mt7612u_write_reg(pAd, 0x80, value);

	/* release "BBP software reset */
	mt7612u_read_reg(pAd, 0x64, &value);
	value &= ~(0x1<<18);
	mt7612u_write_reg(pAd, 0x64, value);
}

void mt76x2_pwrOn(struct rtmp_adapter *pAd)
{
	    /* Radio On */
	    DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	    WL_POWER_ON(pAd);
	    WF_CTRL(pAd, 0, 1);
	    WF_CTRL(pAd, 1, 1);
}


int mt76x2_set_ed_cca(struct rtmp_adapter *ad, u8 enable)
{
        uint32_t mac_val;
        uint32_t bbp_val;

        if (enable) {
                mac_val = mt7612u_read32(ad, CH_TIME_CFG);
                mac_val |= 0x05; // enable channel status check
                mt7612u_write32(ad, CH_TIME_CFG, mac_val);

                // BBP: latched ED_CCA and high/low threshold
                mac_val = RTMP_BBP_IO_READ32(ad, AGC1_R2);
                //bbp_val &= 0xFFFF;
                bbp_val = 0x80000808;
                RTMP_BBP_IO_WRITE32(ad, AGC1_R2, bbp_val);

                // MAC: enable ED_CCA/ED_2nd_CCA
                mac_val = mt7612u_read32(ad, TXOP_CTRL_CFG);
                mac_val |= ((1<<20) | (1<<7));
                mt7612u_write32(ad, TXOP_CTRL_CFG, mac_val);

                mac_val = mt7612u_read32(ad, TXOP_HLDR_ET);
                mac_val |= 2;
                mt7612u_write32(ad, TXOP_HLDR_ET, mac_val);
        } else {
                // MAC: disable ED_CCA/ED_2nd_CCA
                mac_val = mt7612u_read32(ad, TXOP_CTRL_CFG);
                mac_val &= (~((1<<20) | (1<<7)));
                mt7612u_write32(ad, TXOP_CTRL_CFG, mac_val);

                mac_val = mt7612u_read32(ad, TXOP_HLDR_ET);
                mac_val &= ~2;
                mt7612u_write32(ad, TXOP_HLDR_ET, mac_val);
        }

        /* Clear previous status */
        mac_val = mt7612u_read32(ad, CH_IDLE_STA);
        mac_val = mt7612u_read32(ad, CH_BUSY_STA);
        mac_val = mt7612u_read32(ad, CH_BUSY_STA_SEC);
        mac_val = mt7612u_read32(ad, 0x1140);

        return TRUE;
}


