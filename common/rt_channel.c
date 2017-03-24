/*

*/
#include "rt_config.h"


CH_FREQ_MAP CH_HZ_ID_MAP[]=
		{
			{1, 2412},
			{2, 2417},
			{3, 2422},
			{4, 2427},
			{5, 2432},
			{6, 2437},
			{7, 2442},
			{8, 2447},
			{9, 2452},
			{10, 2457},
			{11, 2462},
			{12, 2467},
			{13, 2472},
			{14, 2484},

			/*  UNII */
			{36, 5180},
			{40, 5200},
			{44, 5220},
			{48, 5240},
			{52, 5260},
			{56, 5280},
			{60, 5300},
			{64, 5320},
			{149, 5745},
			{153, 5765},
			{157, 5785},
			{161, 5805},
			{165, 5825},
			{167, 5835},
			{169, 5845},
			{171, 5855},
			{173, 5865},

			/* HiperLAN2 */
			{100, 5500},
			{104, 5520},
			{108, 5540},
			{112, 5560},
			{116, 5580},
			{120, 5600},
			{124, 5620},
			{128, 5640},
			{132, 5660},
			{136, 5680},
			{140, 5700},

			/* Japan MMAC */
			{34, 5170},
			{38, 5190},
			{42, 5210},
			{46, 5230},

			/*  Japan */
			{184, 4920},
			{188, 4940},
			{192, 4960},
			{196, 4980},

			{208, 5040},	/* Japan, means J08 */
			{212, 5060},	/* Japan, means J12 */
			{216, 5080},	/* Japan, means J16 */
};

INT	CH_HZ_ID_MAP_NUM = (sizeof(CH_HZ_ID_MAP)/sizeof(CH_FREQ_MAP));

CH_DESC Country_Region0_ChDesc_2GHZ[] =
{
	{1, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_2GHZ[] =
{
	{1, 13, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_2GHZ[] =
{
	{10, 2, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_2GHZ[] =
{
	{10, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_2GHZ[] =
{
	{14, 1, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region5_ChDesc_2GHZ[] =
{
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_2GHZ[] =
{
	{3, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_2GHZ[] =
{
	{5, 9, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region31_ChDesc_2GHZ[] =
{
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 3, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region32_ChDesc_2GHZ[] =
{
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 2, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region33_ChDesc_2GHZ[] =
{
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[] =
{
	{REGION_0_BG_BAND, Country_Region0_ChDesc_2GHZ},
	{REGION_1_BG_BAND, Country_Region1_ChDesc_2GHZ},
	{REGION_2_BG_BAND, Country_Region2_ChDesc_2GHZ},
	{REGION_3_BG_BAND, Country_Region3_ChDesc_2GHZ},
	{REGION_4_BG_BAND, Country_Region4_ChDesc_2GHZ},
	{REGION_5_BG_BAND, Country_Region5_ChDesc_2GHZ},
	{REGION_6_BG_BAND, Country_Region6_ChDesc_2GHZ},
	{REGION_7_BG_BAND, Country_Region7_ChDesc_2GHZ},
	{REGION_31_BG_BAND, Country_Region31_ChDesc_2GHZ},
	{REGION_32_BG_BAND, Country_Region32_ChDesc_2GHZ},
	{REGION_33_BG_BAND, Country_Region33_ChDesc_2GHZ},
	{}
};

uint16_t const Country_Region_GroupNum_2GHZ = sizeof(Country_Region_ChDesc_2GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_DESC Country_Region0_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_5GHZ[] =
{
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};
CH_DESC Country_Region5_ChDesc_5GHZ[] =
{
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_5GHZ[] =
{
	{36, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region8_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region9_ChDesc_5GHZ[] =
{
	{36, 8 , CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region10_ChDesc_5GHZ[] =
{
	{36,4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region11_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 6, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region12_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region13_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region14_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{136, 2, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region15_ChDesc_5GHZ[] =
{
	{149, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region16_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region17_ChDesc_5GHZ[] =
{
	{36, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region18_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region19_ChDesc_5GHZ[] =
{
	{56, 3, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region20_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 7, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region21_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region22_ChDesc_5GHZ[] =
{
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[] =
{
	{REGION_0_A_BAND, Country_Region0_ChDesc_5GHZ},
	{REGION_1_A_BAND, Country_Region1_ChDesc_5GHZ},
	{REGION_2_A_BAND, Country_Region2_ChDesc_5GHZ},
	{REGION_3_A_BAND, Country_Region3_ChDesc_5GHZ},
	{REGION_4_A_BAND, Country_Region4_ChDesc_5GHZ},
	{REGION_5_A_BAND, Country_Region5_ChDesc_5GHZ},
	{REGION_6_A_BAND, Country_Region6_ChDesc_5GHZ},
	{REGION_7_A_BAND, Country_Region7_ChDesc_5GHZ},
	{REGION_8_A_BAND, Country_Region8_ChDesc_5GHZ},
	{REGION_9_A_BAND, Country_Region9_ChDesc_5GHZ},
	{REGION_10_A_BAND, Country_Region10_ChDesc_5GHZ},
	{REGION_11_A_BAND, Country_Region11_ChDesc_5GHZ},
	{REGION_12_A_BAND, Country_Region12_ChDesc_5GHZ},
	{REGION_13_A_BAND, Country_Region13_ChDesc_5GHZ},
	{REGION_14_A_BAND, Country_Region14_ChDesc_5GHZ},
	{REGION_15_A_BAND, Country_Region15_ChDesc_5GHZ},
	{REGION_16_A_BAND, Country_Region16_ChDesc_5GHZ},
	{REGION_17_A_BAND, Country_Region17_ChDesc_5GHZ},
	{REGION_18_A_BAND, Country_Region18_ChDesc_5GHZ},
	{REGION_19_A_BAND, Country_Region19_ChDesc_5GHZ},
	{REGION_20_A_BAND, Country_Region20_ChDesc_5GHZ},
	{REGION_21_A_BAND, Country_Region21_ChDesc_5GHZ},
	{REGION_22_A_BAND, Country_Region22_ChDesc_5GHZ},
	{}
};

uint16_t const Country_Region_GroupNum_5GHZ = sizeof(Country_Region_ChDesc_5GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

uint16_t TotalChNum(PCH_DESC pChDesc)
{
	uint16_t TotalChNum = 0;

	while(pChDesc->FirstChannel)
	{
		TotalChNum += pChDesc->NumOfCh;
		pChDesc++;
	}

	return TotalChNum;
}

UCHAR GetChannel_5GHZ(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel)
	{
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index * 4;
		else
		{
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannel_2GHZ(PCH_DESC pChDesc, UCHAR index)
{

	while (pChDesc->FirstChannel)
	{
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index;
		else
		{
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannelFlag(PCH_DESC pChDesc, UCHAR index)
{

	while (pChDesc->FirstChannel)
	{
		if (index < pChDesc->NumOfCh)
			return pChDesc->ChannelProp;
		else
		{
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

static bool IsValidChannel(
	IN struct rtmp_adapter *pAd,
	IN UCHAR channel)

{
	INT i;

	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return FALSE;
	else
		return TRUE;
}

static UCHAR GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction)
{
	CHAR ExtCh;

	if (Direction == EXTCHA_ABOVE)
		ExtCh = Channel + 4;
	else
		ExtCh = (Channel - 4) > 0 ? (Channel - 4) : 0;

	return ExtCh;
}

INT get_vht_neighbor_index(IN UCHAR channel)
{
	/*
	 * ULLI : stupid  thing here, we should never 4 * 6 channles used
	 * ULLI : and only 24 channels available, this make if () else wrong
	 * ULLI : so we do simple fix return value to -3 if we fall through
	 */

	if ((channel == 36) || (channel == 52)
		|| (channel == 100) || (channel == 116)
		|| (channel == 132) || (channel == 149)) {
		return 0;
	}
	else if ((channel == 40) || (channel == 56)
		|| (channel == 104) || (channel == 120)
		|| (channel == 136) || (channel == 153)) {
		return -1;
	}
	else if ((channel == 44) || (channel == 60)
		|| (channel == 108) || (channel == 124)
		|| (channel == 140) || (channel == 157)) {
		return -2;
	}
	else if ((channel == 48) || (channel == 64)
		|| (channel == 112) || (channel == 128)
		|| (channel == 144) || (channel == 161)) {
		return -3;
	}

	return -3;
}

bool AC_ChannelGroupCheck(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Channel)
{
	bool	RetVal = FALSE;
	UCHAR	vht_ch_group[] = {
		36, 40, 44, 48,
		52, 56, 60, 64,
		100, 104, 108, 112,
		116, 120, 124, 128,
		132, 136, 140, 144,
		149, 153, 157, 161
	};
	UINT8	num_ch = sizeof(vht_ch_group)/sizeof(UCHAR);
	UINT8	idx;

	if (Channel > 14)
	{ /* 5G Band */
		for (idx=0; idx<num_ch; idx++) {
			if (Channel == vht_ch_group[idx]) {
				/* in BW_80 channel group */
				RetVal = TRUE;
				break;
			}
		}
	}

	return RetVal;
}

bool N_ChannelGroupCheck(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Channel)
{
	bool	RetVal = FALSE;

	if (Channel > 14)
	{
		if ((Channel == 36) || (Channel == 44) || (Channel == 52)
				|| (Channel == 60) || (Channel == 100) || (Channel == 108)
				|| (Channel == 116) || (Channel == 124) || (Channel == 132)
				|| (Channel == 149) || (Channel == 157))
		{
			RetVal = TRUE;
		}
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56)
				|| (Channel == 64) || (Channel == 104) || (Channel == 112)
				|| (Channel == 120) || (Channel == 128) || (Channel == 136)
				|| (Channel == 153) || (Channel == 161))
		{
			RetVal = TRUE;
		}
	}
	else
	{
		do
		{
			UCHAR ExtCh;

			if (Channel == 14)
			{
				RetVal = FALSE;
				break;
			}

			ExtCh = GetExtCh(Channel, EXTCHA_ABOVE);
			if (IsValidChannel(pAd, ExtCh))
				RetVal = TRUE;
			else
			{
				ExtCh = GetExtCh(Channel, EXTCHA_BELOW);
				if (IsValidChannel(pAd, ExtCh))
				RetVal = TRUE;
			}
		} while(FALSE);
	}

	return RetVal;
}


VOID N_ChannelCheck(struct rtmp_adapter *pAd)
{
	INT idx;
	UCHAR Channel = pAd->CommonCfg.Channel;
	static const UCHAR wfa_ht_ch_ext[] = {
			36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
			44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
			52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
			60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
			100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
			108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
			116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
			124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
			132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
			149, EXTCHA_ABOVE, 153, EXTCHA_BELOW,
			157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
			0, 0};

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
		(pAd->CommonCfg.RegTransmitSetting.field.BW >= BW_40))
	{
		if (Channel > 14)
		{
			idx = 0;
			while(wfa_ht_ch_ext[idx] != 0) {
				if (wfa_ht_ch_ext[idx] == Channel) {
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = wfa_ht_ch_ext[idx + 1];
					break;
				}
				idx += 2;
			};

			if (wfa_ht_ch_ext[idx] == 0) {
				pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;
				if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && (pAd->CommonCfg.vht_bw > VHT_BW_2040))
					pAd->CommonCfg.vht_bw = VHT_BW_2040;
			}
		}
		else
		{
			do
			{
				UCHAR ExtCh;
				UCHAR Dir = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
				ExtCh = GetExtCh(Channel, Dir);
				if (IsValidChannel(pAd, ExtCh))
					break;

				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = GetExtCh(Channel, Dir);
				if (IsValidChannel(pAd, ExtCh))
				{
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = Dir;
					break;
				}
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
			} while(FALSE);

			if (Channel == 14)
			{
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
				/*pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_NONE; We didn't set the ExtCh as NONE due to it'll set in RTMPSetHT()*/
			}
		}
	}
}


UCHAR N_SetCenCh(struct rtmp_adapter *pAd, UCHAR prim_ch)
{
	if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
	{
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			pAd->CommonCfg.CentralChannel = prim_ch + 2;
		else
		{
			if (prim_ch == 14)
				pAd->CommonCfg.CentralChannel = prim_ch - 1;
			else
				pAd->CommonCfg.CentralChannel = prim_ch - 2;
		}
	}
	else
		pAd->CommonCfg.CentralChannel = prim_ch;

	return pAd->CommonCfg.CentralChannel;
}


UINT8 GetCuntryMaxTxPwr(
	IN struct rtmp_adapter *pAd,
	IN UINT8 channel)
{
	int i;
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return 0xff;

	return pAd->ChannelList[i].MaxTxPwr;
}


/* for OS_ABL */
VOID RTMP_MapChannelID2KHZ(
	IN UCHAR Ch,
	OUT uint32_t *pFreq)
{
	int chIdx;
	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++)
	{
		if ((Ch) == CH_HZ_ID_MAP[chIdx].channel)
		{
			(*pFreq) = CH_HZ_ID_MAP[chIdx].freqKHz * 1000;
			break;
		}
	}
	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pFreq) = 2412000;
}

/* for OS_ABL */
VOID RTMP_MapKHZ2ChannelID(
	IN ULONG Freq,
	OUT INT *pCh)
{
	int chIdx;
	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++)
	{
		if ((Freq) == CH_HZ_ID_MAP[chIdx].freqKHz)
		{
			(*pCh) = CH_HZ_ID_MAP[chIdx].channel;
			break;
		}
	}
	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pCh) = 1;
}

