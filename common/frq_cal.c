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
	frq_cal.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef CONFIG_STA_SUPPORT

#include	"rt_config.h"

/*
	Sometimes frequency will be shift we need to adjust it when
	the frequencey shift.
*/

/* Initialize the frequency calibration*/
VOID InitFrequencyCalibration(
	IN struct rtmp_adapter *pAd)
{
	if (pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == true)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("---> %s\n", __FUNCTION__));

		StopFrequencyCalibration(pAd);

		DBGPRINT(RT_DEBUG_ERROR, ("%s: frequency offset in the EEPROM = %d\n",
					__FUNCTION__, pAd->RfFreqOffset));

		DBGPRINT(RT_DEBUG_ERROR, ("<--- %s\n", __FUNCTION__));
	}
}


/* To stop the frequency calibration algorithm*/
VOID StopFrequencyCalibration(
	IN struct rtmp_adapter *pAd)
{
	if (pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == true)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

		/* Base on the frequency offset of the EEPROM*/
		pAd->FreqCalibrationCtrl.AdaptiveFreqOffset = (0x7F & ((CHAR)(pAd->RfFreqOffset))); /* C1 value control - Crystal calibration*/

		pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon = INVALID_FREQUENCY_OFFSET;
		pAd->FreqCalibrationCtrl.bSkipFirstFrequencyCalibration = true;

		DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->FreqCalibrationCtrl.AdaptiveFreqOffset = 0x%X\n",
			__FUNCTION__,
			pAd->FreqCalibrationCtrl.AdaptiveFreqOffset));

		DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
	}
}


/* The frequency calibration algorithm*/
VOID FrequencyCalibration(
	IN struct rtmp_adapter *pAd)
{
	/*bool bUpdateRFR = false;*/
	CHAR HighFreqTriggerPoint = 0, LowFreqTriggerPoint = 0;
	CHAR DecreaseFreqOffset = 0, IncreaseFreqOffset = 0;

	if (IS_MT76x2(pAd))
	{
		PFREQUENCY_CALIBRATION_CONTROL pFrqCal = &pAd->FreqCalibrationCtrl;
		CHAR upBound = 0, lowBound =0;

		if (pFrqCal->bEnableFrequencyCalibration &&
	            pFrqCal->LatestFreqOffsetOverBeacon != INVALID_FREQUENCY_OFFSET)
		{
			if (pFrqCal->BeaconPhyMode == MODE_CCK)
			{
				upBound = 4;
				lowBound = -4;
			}
			else
			{
				/* Beacon on OFDM Mode*/
				upBound = 32;
				lowBound = -32;
			}
		}

		if ((pFrqCal->LatestFreqOffsetOverBeacon >= upBound) ||
		   (pFrqCal->LatestFreqOffsetOverBeacon <= lowBound))
		{
			 pFrqCal->bApproachFrequency = true;
		}
		else
		{
			pFrqCal->bApproachFrequency = false;
		}

		if (pFrqCal->bApproachFrequency == true)
		{
			u32 value = 0;
			value = mt7612u_cfg3_read(pAd, XO_CTRL5);
			DBGPRINT(RT_DEBUG_TRACE, ("FRQ:  Read Value => %08x\n", value));
			pFrqCal->AdaptiveFreqOffset = (value & ~0xffff80ff) >> 8;

			if (pFrqCal->LatestFreqOffsetOverBeacon > 0)
			{
				if (pFrqCal->AdaptiveFreqOffset > 0)
					pFrqCal->AdaptiveFreqOffset--;
			}
			else
			{
				if (pFrqCal->AdaptiveFreqOffset < 0x7F)
					pFrqCal->AdaptiveFreqOffset++;
			}

			value = (value & 0xffff80ff) | (pFrqCal->AdaptiveFreqOffset << 8);
			DBGPRINT(RT_DEBUG_TRACE, ("FRQ:  After just Value => %08x\n", value ));
			mt7612u_cfg3_write(pAd, XO_CTRL5, value);
		}

		return;
	}

	/* Frequency calibration period: */
	/* a) 10 seconds: Check the reported frequency offset*/
	/* b) 500 ms: Update the RF frequency if possible*/
	if ((pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == true) &&
	     (((pAd->FreqCalibrationCtrl.bApproachFrequency == false) && ((pAd->Mlme.PeriodicRound % FREQUENCY_CALIBRATION_PERIOD) == 0)) ||
	       ((pAd->FreqCalibrationCtrl.bApproachFrequency == true) && ((pAd->Mlme.PeriodicRound % (FREQUENCY_CALIBRATION_PERIOD / 20)) == 0))))
	{
		DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

		if (pAd->FreqCalibrationCtrl.bSkipFirstFrequencyCalibration == true)
		{
			pAd->FreqCalibrationCtrl.bSkipFirstFrequencyCalibration = false;

			DBGPRINT(RT_DEBUG_INFO, ("%s: Skip cuurent frequency calibration (avoid calibrating frequency at the time the STA is just link-up)\n", __FUNCTION__));
		}
		else
		{
			if (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon != INVALID_FREQUENCY_OFFSET)
			{
				if (pAd->FreqCalibrationCtrl.BeaconPhyMode == MODE_CCK) /* CCK*/
				{
					{
						HighFreqTriggerPoint = HIGH_FREQUENCY_TRIGGER_POINT_CCK;
						LowFreqTriggerPoint = LOW_FREQUENCY_TRIGGER_POINT_CCK;

						DecreaseFreqOffset = DECREASE_FREQUENCY_OFFSET_CCK;
						IncreaseFreqOffset = INCREASE_FREQUENCY_OFFSET_CCK;
					}
				}
				else /* OFDM*/
				{
					{
						HighFreqTriggerPoint = HIGH_FREQUENCY_TRIGGER_POINT_OFDM;
						LowFreqTriggerPoint = LOW_FREQUENCY_TRIGGER_POINT_OFDM;

						DecreaseFreqOffset = DECREASE_FREQUENCY_OFFSET_OFDM;
						IncreaseFreqOffset = INCREASE_FREQUENCY_OFFSET_OFDM;
					}
				}

				if ((pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon >= HighFreqTriggerPoint) ||
				     (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon <= LowFreqTriggerPoint))
				{
					pAd->FreqCalibrationCtrl.bApproachFrequency = true;
				}

				if (pAd->FreqCalibrationCtrl.bApproachFrequency == true)
				{
					if ((pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon <= DecreaseFreqOffset) &&
					      (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon >= IncreaseFreqOffset))
					{
						pAd->FreqCalibrationCtrl.bApproachFrequency = false; /* Stop approaching frquency if -10 < reported frequency offset < 10*/
					}
					else if (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon > DecreaseFreqOffset)
					{
						pAd->FreqCalibrationCtrl.AdaptiveFreqOffset--;
						DBGPRINT(RT_DEBUG_TRACE, ("%s: -- frequency offset = 0x%X\n", __FUNCTION__, pAd->FreqCalibrationCtrl.AdaptiveFreqOffset));
					}
					else if (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon < IncreaseFreqOffset)
					{
						pAd->FreqCalibrationCtrl.AdaptiveFreqOffset++;
						DBGPRINT(RT_DEBUG_TRACE, ("%s: ++ frequency offset = 0x%X\n", __FUNCTION__, pAd->FreqCalibrationCtrl.AdaptiveFreqOffset));
					}
				}

				DBGPRINT(RT_DEBUG_INFO, ("%s: AdaptiveFreqOffset = %d, LatestFreqOffsetOverBeacon = %d, bApproachFrequency = %d\n",
					__FUNCTION__,
					pAd->FreqCalibrationCtrl.AdaptiveFreqOffset,
					pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon,
					pAd->FreqCalibrationCtrl.bApproachFrequency));

			}
		}

		DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
	}
}


inline CHAR GetFrequencyOffsetField(
	struct rtmp_adapter *pAd,
	RXWI_STRUC *pRxWI,
	UINT8 RxWIFrqOffsetField)
{
	CHAR FreqOffset = 0;

	if (RxWIFrqOffsetField == RXWI_FRQ_OFFSET_FIELD0) {
		FreqOffset = (CHAR)(pRxWI->RXWI_N.bbp_rxinfo[1]);
	} else if (RxWIFrqOffsetField == RXWI_FRQ_OFFSET_FIELD1) {
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:Unknow Frequency Offset location(%d)\n", __FUNCTION__, RxWIFrqOffsetField));

	return FreqOffset;
}


/* Get the frequency offset*/
CHAR GetFrequencyOffset(
	IN struct rtmp_adapter *pAd,
	IN RXWI_STRUC *pRxWI)
{
	CHAR FreqOffset = 0;

	if (pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration)
	{
		DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

		FreqOffset = GetFrequencyOffsetField(pAd, pRxWI, pAd->chipCap.RxWIFrqOffset);

		if (IS_MT76x2(pAd))
			goto ret;

		if ((FreqOffset < LOWERBOUND_OF_FREQUENCY_OFFSET) ||
		     (FreqOffset > UPPERBOUND_OF_FREQUENCY_OFFSET))
		{
			FreqOffset = INVALID_FREQUENCY_OFFSET;

			DBGPRINT(RT_DEBUG_ERROR, ("%s: (out-of-range) FreqOffset = %d\n",
				__FUNCTION__,
				FreqOffset));
		}

		DBGPRINT(RT_DEBUG_INFO, ("%s: FreqOffset = %d\n",
					 __FUNCTION__, FreqOffset));

		DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
	}

ret:
	return FreqOffset;
}
#endif /* CONFIG_STA_SUPPORT */


