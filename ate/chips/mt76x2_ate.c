/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
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
	mt7662_ate.c

	Abstract:
	Specific ATE funcitons and variables for MT7662

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef MT76x2


#include "rt_config.h"

extern RTMP_REG_PAIR mt76x2_mac_g_band_cr_table[];
extern UCHAR mt76x2_mac_g_band_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_g_band_external_pa_cr_table[];
extern UCHAR mt76x2_mac_g_band_external_pa_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_g_band_internal_pa_cr_table[];
extern UCHAR mt76x2_mac_g_band_internal_pa_cr_nums;

extern RTMP_REG_PAIR mt76x2_mac_a_band_cr_table[];
extern UCHAR mt76x2_mac_a_band_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_a_band_external_pa_cr_table[];
extern UCHAR mt76x2_mac_a_band_external_pa_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_a_band_internal_pa_cr_table[];
extern UCHAR mt76x2_mac_a_band_internal_pa_cr_nums;

#define ATE_TX_TARGET_PWR_DEFAULT_VALUE		5
#define MT76x2_TSSI_STABLE_COUNT		3

static uint32_t mt76x2_ate_calibration_delay;
static uint32_t mt76x2_ate_tssi_stable_count;
static UCHAR mt76x2_2G_tx0_pwr_offset_save = 0;
static UCHAR mt76x2_2G_tx1_pwr_offset_save = 0;
static UCHAR mt76x2_5G_tx0_pwr_offset_save = 0;
static UCHAR mt76x2_5G_tx1_pwr_offset_save = 0;

static BOOLEAN mt76x2_tx0_tssi_small_pwr_adjust = FALSE;
static BOOLEAN mt76x2_tx1_tssi_small_pwr_adjust = FALSE;

#ifdef RTMP_TEMPERATURE_TX_ALC
void mt76x2_ate_temp_tx_alc(struct rtmp_adapter *ad)
{
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;
	int32_t temp_diff = 0, dB_diff = 0, tx0_temp_comp = 0, tx1_temp_comp = 0;

	if (pChipCap->temp_tx_alc_enable) {
		mt76x2_get_current_temp(ad);
		temp_diff = pChipCap->current_temp - 25;

		if (ad->ate.Channel > 14) {
			if (temp_diff > 0)
				dB_diff = (temp_diff / pChipCap->high_temp_slope_a_band);
			else if (temp_diff < 0)
		 		dB_diff = 0 - ((0 - temp_diff) / pChipCap->low_temp_slope_a_band);
			else
				dB_diff = 0;

			/* temperature compensation boundary check and limit */
			dB_diff = (dB_diff > pChipCap->tc_upper_bound_a_band) ? pChipCap->tc_upper_bound_a_band : dB_diff;
			dB_diff = (dB_diff < pChipCap->tc_lower_bound_a_band) ? pChipCap->tc_lower_bound_a_band : dB_diff;
		} else {
			if (temp_diff > 0)
				dB_diff = (temp_diff / pChipCap->high_temp_slope_g_band);
			else if (temp_diff < 0)
		 		dB_diff = 0 - ((0 - temp_diff) / pChipCap->low_temp_slope_g_band);
			else
				dB_diff = 0;

			/* temperature compensation boundary check and limit */
			dB_diff = (dB_diff > pChipCap->tc_upper_bound_g_band) ? pChipCap->tc_upper_bound_g_band : dB_diff;
			dB_diff = (dB_diff < pChipCap->tc_lower_bound_g_band) ? pChipCap->tc_lower_bound_g_band : dB_diff;
		}

		DBGPRINT(RT_DEBUG_INFO, ("%s::temp_diff=%d (0x%x), dB_diff=%d (0x%x)\n",
			__FUNCTION__, temp_diff, temp_diff, dB_diff, dB_diff));

		RTMP_IO_READ32(ad, TX_ALC_CFG_1, &tx0_temp_comp);
		tx0_temp_comp &= ~TX_ALC_CFG_1_TX0_TEMP_COMP_MASK;
		tx0_temp_comp |= (dB_diff*2 & TX_ALC_CFG_1_TX0_TEMP_COMP_MASK);
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_1, tx0_temp_comp);
		DBGPRINT(RT_DEBUG_INFO, ("%s::Tx0 power compensation = 0x%x\n",
			__FUNCTION__, tx0_temp_comp & 0x3f));

		RTMP_IO_READ32(ad, TX_ALC_CFG_2, &tx1_temp_comp);
		tx1_temp_comp &= ~TX_ALC_CFG_2_TX1_TEMP_COMP_MASK;
		tx1_temp_comp |= (dB_diff*2 & TX_ALC_CFG_2_TX1_TEMP_COMP_MASK);
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_2, tx1_temp_comp);
		DBGPRINT(RT_DEBUG_INFO, ("%s::Tx1 power compensation = 0x%x\n",
			__FUNCTION__, tx1_temp_comp & 0x3f));
	}
}
#endif /* RTMP_TEMPERATURE_TX_ALC */

VOID mt76x2_adjust_tssi_offset(
		IN struct rtmp_adapter *pAd,
		IN uint32_t *slope_offset)
{
	CHAR OrgTSSIOffset0, OrgTSSIOffset1;
	CHAR TSSIOffsetDelta0, TSSIOffsetDelta1;
	CHAR NewTSSIOffset0, NewTSSIOffset1;
	int32_t CurrentTemperature;

	OrgTSSIOffset0 = (*slope_offset >> 16) & 0xFF;
	OrgTSSIOffset1 = (*slope_offset >> 24) & 0xFF;

	//read temperature and get TSSI offset delta in correspont temperature
	mt76x2_get_current_temp(pAd);
	CurrentTemperature = pAd->chipCap.current_temp;

	if(CurrentTemperature < -30)
	{
		TSSIOffsetDelta0 = -10;
		TSSIOffsetDelta1 = -12;
	}
	else if(CurrentTemperature <= -21)
	{
		TSSIOffsetDelta0 = -8;
		TSSIOffsetDelta1 = -10;
	}
	else if(CurrentTemperature <= -11)
	{
		TSSIOffsetDelta0 = -6;
		TSSIOffsetDelta1 = -8;
	}
	else if(CurrentTemperature <= -1)
	{
		TSSIOffsetDelta0 = -5;
		TSSIOffsetDelta1 = -6;
	}
	else if(CurrentTemperature <= 9)
	{
		TSSIOffsetDelta0 = -3;
		TSSIOffsetDelta1 = -4;
	}
	else if(CurrentTemperature <= 19)
	{
		TSSIOffsetDelta0 = -2;
		TSSIOffsetDelta1 = -2;
	}
	else if(CurrentTemperature <= 29)
	{
		TSSIOffsetDelta0 = 0;
		TSSIOffsetDelta1 = 0;
		DBGPRINT(RT_DEBUG_INFO, ("TSSIOffsetDelta = 0, not need adjust\n"));
		return;
	}
	else if(CurrentTemperature <= 39)
	{
		TSSIOffsetDelta0 = 2;
		TSSIOffsetDelta1 = 2;
	}
	else if(CurrentTemperature <= 49)
	{
		TSSIOffsetDelta0 = 3;
		TSSIOffsetDelta1 = 4;
	}
	else if(CurrentTemperature <= 59)
	{
		TSSIOffsetDelta0 = 5;
		TSSIOffsetDelta1 = 6;
	}
	else if(CurrentTemperature <= 69)
	{
		TSSIOffsetDelta0 = 6;
		TSSIOffsetDelta1 = 8;
	}
	else if(CurrentTemperature <= 79)
	{
		TSSIOffsetDelta0 = 8;
		TSSIOffsetDelta1 = 10;
	}
	else if(CurrentTemperature <= 89)
	{
		TSSIOffsetDelta0 = 10;
		TSSIOffsetDelta1 = 12;
	}
	else if(CurrentTemperature <= 99)
	{
		TSSIOffsetDelta0 = 11;
		TSSIOffsetDelta1 = 14;
	}
	else
	{
		TSSIOffsetDelta0 = 13;
		TSSIOffsetDelta1 = 16;
	}

	NewTSSIOffset0 = OrgTSSIOffset0 + TSSIOffsetDelta0;
	if( (OrgTSSIOffset0 > 0) && (TSSIOffsetDelta0 >0))
	{
		if(NewTSSIOffset0 < 0)
			NewTSSIOffset0 = 127;
	}
	else if( (OrgTSSIOffset0 < 0) && (TSSIOffsetDelta0 < 0))
	{
		if(NewTSSIOffset0 > 0)
			NewTSSIOffset0 = -128;
	}

	NewTSSIOffset1 = OrgTSSIOffset1 + TSSIOffsetDelta1;
	if( (OrgTSSIOffset1 > 0) && (TSSIOffsetDelta1 >0))
	{
		if(NewTSSIOffset1 < 0)
			NewTSSIOffset1 = 127;
	}
	else if( (OrgTSSIOffset1 < 0) && (TSSIOffsetDelta1 < 0))
	{
		if(NewTSSIOffset1 > 0)
			NewTSSIOffset1 = -128;
	}

	*slope_offset = (*slope_offset & 0x0000FFFF) | (NewTSSIOffset1<< 24) | (NewTSSIOffset0 << 16);
}


VOID mt76x2_ate_asic_adjust_tx_power(
	IN struct rtmp_adapter *pAd)
{
	RTMP_CHIP_CAP *cap = &pAd->chipCap;
	ANDES_CALIBRATION_PARAM param;
	uint32_t pa_mode = 0, tssi_slope_offset = 0;
	uint32_t ret = 0;
	PATE_INFO   pATEInfo = &(pAd->ate);
	uint32_t value;
	char TxPower = 0;


	if ((pAd->chipCap.tssi_enable) &&
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF |
				fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET) == FALSE)) {
		//mt76x2_tssi_compensation(pAd, pAd->ate.Channel);

#ifdef RTMP_MAC_USB
		if (IS_USB_INF(pAd)) {
			RTMP_SEM_EVENT_WAIT(&pAd->tssi_lock, ret);
			if (ret != 0) {
				DBGPRINT(RT_DEBUG_ERROR, ("tssi_lock get failed(ret=%d)\n", ret));
				return;// STATUS_UNSUCCESSFUL;
			}
		}
#endif /* RTMP_MAC_USB */


		if (pAd->ate.Channel > 14) {
			if (pAd->chipCap.PAType == EXT_PA_2G_5G)
				pa_mode = 1;
			else if (pAd->chipCap.PAType == EXT_PA_5G_ONLY)
				pa_mode = 1;
			else
				pa_mode = 0;
		} else {
			if (pAd->chipCap.PAType == EXT_PA_2G_5G)
				pa_mode = 1;
			else if ((pAd->chipCap.PAType == EXT_PA_5G_ONLY) ||
					(pAd->chipCap.PAType == INT_PA_2G_5G))
				pa_mode = 0;
			else if (pAd->chipCap.PAType == EXT_PA_2G_ONLY)
				pa_mode = 1;
		}

		if (pAd->ate.Channel < 14) {
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE0(cap->tssi_0_slope_g_band);
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE1(cap->tssi_1_slope_g_band);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET0(cap->tssi_0_offset_g_band);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET1(cap->tssi_1_offset_g_band);
		} else {
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE0(cap->tssi_0_slope_a_band[get_chl_grp(pAd->ate.Channel )]);
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE1(cap->tssi_1_slope_a_band[get_chl_grp(pAd->ate.Channel )]);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET0(cap->tssi_0_offset_a_band[get_chl_grp(pAd->ate.Channel )]);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET1(cap->tssi_1_offset_a_band[get_chl_grp(pAd->ate.Channel )]);

			//mt76x2_adjust_tssi_offset(pAd, &tssi_slope_offset);
		}

		param.mt76x2_tssi_comp_param.pa_mode = pa_mode;
		param.mt76x2_tssi_comp_param.tssi_slope_offset = tssi_slope_offset;

		/* TSSI Compensation */
		if(pAd->chipOps.Calibration != NULL)
			pAd->chipOps.Calibration(pAd, TSSI_COMPENSATION_7662, &param);


#ifdef RTMP_MAC_USB
		if (IS_USB_INF(pAd)) {
			RTMP_SEM_EVENT_UP(&pAd->tssi_lock);
		}
#endif

		mt76x2_ate_calibration_delay ++;

		if ( (mt76x2_ate_calibration_delay % 10) == 0 ) {
			if ( pATEInfo->TxAntennaSel == 0 ) {
				if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx0_pwr_offset_save = value;
					else
						mt76x2_2G_tx0_pwr_offset_save = value;
				}

				if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx1_pwr_offset_save = value;
					else
						mt76x2_2G_tx1_pwr_offset_save = value;
				}
			} else if ( pATEInfo->TxAntennaSel == 1 ) {
				if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx0_pwr_offset_save = value;
					else
						mt76x2_2G_tx0_pwr_offset_save = value;
				}
			} else if ( pATEInfo->TxAntennaSel == 2 ) {
				if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx1_pwr_offset_save = value;
					else
						mt76x2_2G_tx1_pwr_offset_save = value;
				}
			}
		}

		//DBGPRINT(RT_DEBUG_INFO, ("mt76x2_ate_calibration_delay %d   mt76x2_ate_tssi_stable_count %d\n", mt76x2_ate_calibration_delay,mt76x2_ate_tssi_stable_count));
		//if ( mt76x2_ate_calibration_delay == mt76x2_ate_tssi_stable_count ) {
		if ( mt76x2_ate_calibration_delay % 3 == 0 ) {
			DBGPRINT(RT_DEBUG_INFO,("mt76x2_ate_calibration_delay mod 3 == 0 mt76x2_tx0_tssi_small_pwr_adjust %d mt76x2_tx1_tssi_small_pwr_adjust %d\n",mt76x2_tx0_tssi_small_pwr_adjust,mt76x2_tx1_tssi_small_pwr_adjust));
			if ( pATEInfo->TxAntennaSel == 0 ) {

				if ( mt76x2_tx0_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower0;

					/* TX0 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_0_MASK);
					value |= TX_ALC_CFG_0_CH_INT_0(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);
				}

				if ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower1;

					/* TX1 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_1_MASK);
					value |= TX_ALC_CFG_0_CH_INT_1(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);
				}

				if ( (mt76x2_tx0_tssi_small_pwr_adjust == TRUE) || ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE ))
				{
					mt76x2_ate_tssi_stable_count += MT76x2_TSSI_STABLE_COUNT;
				}

			} else if ( pATEInfo->TxAntennaSel == 1 ) {
				if ( mt76x2_tx0_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower0;

					/* TX0 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_0_MASK);
					value |= TX_ALC_CFG_0_CH_INT_0(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);

					mt76x2_ate_tssi_stable_count += MT76x2_TSSI_STABLE_COUNT;
				}

			} else if ( pATEInfo->TxAntennaSel == 2 ) {
				if ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower1;

					/* TX1 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_1_MASK);
					value |= TX_ALC_CFG_0_CH_INT_1(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);

					mt76x2_ate_tssi_stable_count += MT76x2_TSSI_STABLE_COUNT;
				}
			}
		}

		if ( mt76x2_ate_calibration_delay == mt76x2_ate_tssi_stable_count ) {
			/* DPD Calibration */
			if ( (pAd->chipCap.PAType== INT_PA_2G_5G)
				|| ((pAd->chipCap.PAType == INT_PA_5G) && ( pAd->ate.Channel  > 14 ) )
				|| ((pAd->chipCap.PAType == INT_PA_2G) && ( pAd->ate.Channel  <= 14 ) )
			)
			{

				if ( pATEInfo->TxAntennaSel == 0 ) {
					if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx0_pwr_offset_save = value;
						else
							mt76x2_2G_tx0_pwr_offset_save = value;
					}

					if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx1_pwr_offset_save = value;
						else
							mt76x2_2G_tx1_pwr_offset_save = value;
					}
				} else if ( pATEInfo->TxAntennaSel == 1 ) {
					if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx0_pwr_offset_save = value;
						else
							mt76x2_2G_tx0_pwr_offset_save = value;
					}
				} else if ( pATEInfo->TxAntennaSel == 2 ) {
					if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx1_pwr_offset_save = value;
						else
							mt76x2_2G_tx1_pwr_offset_save = value;
					}
				}

				CHIP_CALIBRATION(pAd, DPD_CALIBRATION_7662, pAd->ate.Channel );
			}
		}
	}
#ifdef RTMP_TEMPERATURE_TX_ALC
	else
	{
		mt76x2_ate_temp_tx_alc(pAd);
	}
#endif /* RTMP_TEMPERATURE_TX_ALC */

}

struct ate_chip_struct mt76x2ate =
{
	/* functions */
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL /* RT5572_ATETssiCalibrationExtend */,
	.AdjustTxPower = mt76x2_ate_asic_adjust_tx_power,
	//.AsicExtraPowerOverMAC = DefaultATEAsicExtraPowerOverMAC,
	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = FALSE,/* ralink debug */
};

#endif /* MT76x2 */





