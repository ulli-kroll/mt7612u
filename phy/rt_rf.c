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
	rt_rf.c

	Abstract:
	Ralink Wireless driver RF related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"


#ifdef RTMP_RF_RW_SUPPORT

/*
	========================================================================

	Routine Description: Write RF register through MAC

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
int RT30xxWriteRFRegister(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i = 0;
	int  ret;


	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		ret = down_interruptible(&pAd->reg_atomic);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return STATUS_UNSUCCESSFUL;
		}
	}
#endif /* RTMP_MAC_USB */

	ret = STATUS_UNSUCCESSFUL;
	do
	{
		rfcsr.word = mt7612u_read32(pAd, RF_CSR_CFG);

		if (!rfcsr.non_bank.RF_CSR_KICK)
			break;
		i++;
	}
	while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s():RF Write failed(RetryCnt=%d, DevNotExistFlag=%d)\n",
						__FUNCTION__, i, RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
		goto done;
	}

	rfcsr.non_bank.RF_CSR_WR = 1;
	rfcsr.non_bank.RF_CSR_KICK = 1;
	rfcsr.non_bank.TESTCSR_RFACC_REGNUM = regID;

	if ((pAd->chipCap.RfReg17WtMethod == RF_REG_WT_METHOD_STEP_ON) && (regID == RF_R17))
	{
		UCHAR IdRf;
		UCHAR RfValue;
		bool beAdd;

		RT30xxReadRFRegister(pAd, RF_R17, &RfValue);
		beAdd =  (RfValue < value) ? TRUE : FALSE;
		IdRf = RfValue;
		while(IdRf != value)
		{
			if (beAdd)
				IdRf++;
			else
				IdRf--;

				rfcsr.non_bank.RF_CSR_DATA = IdRf;
				mt7612u_write32(pAd, RF_CSR_CFG, rfcsr.word);
				RtmpOsMsDelay(1);
		}
	}

	rfcsr.non_bank.RF_CSR_DATA = value;
	mt7612u_write32(pAd, RF_CSR_CFG, rfcsr.word);

	ret = NDIS_STATUS_SUCCESS;

done:
#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		up(&pAd->reg_atomic);
	}
#endif /* RTMP_MAC_USB */

	return ret;
}


/*
	========================================================================

	Routine Description: Read RF register through MAC

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
int RT30xxReadRFRegister(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			regID,
	IN	u8 *		pValue)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i=0, k=0;
	int  ret = STATUS_UNSUCCESSFUL;



	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		i  = down_interruptible(&pAd->reg_atomic);
		if (i != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", i));
			return STATUS_UNSUCCESSFUL;
		}
	}
#endif /* RTMP_MAC_USB */

	for (i=0; i<MAX_BUSY_COUNT; i++)
	{
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			goto done;

		rfcsr.word = mt7612u_read32(pAd, RF_CSR_CFG);

		if (rfcsr.non_bank.RF_CSR_KICK == BUSY)
				continue;

		rfcsr.word = 0;
		rfcsr.non_bank.RF_CSR_WR = 0;
		rfcsr.non_bank.RF_CSR_KICK = 1;
		rfcsr.non_bank.TESTCSR_RFACC_REGNUM = regID;
		mt7612u_write32(pAd, RF_CSR_CFG, rfcsr.word);

		for (k=0; k<MAX_BUSY_COUNT; k++)
		{
			if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
				goto done;

			rfcsr.word = mt7612u_read32(pAd, RF_CSR_CFG);

			if (rfcsr.non_bank.RF_CSR_KICK == IDLE)
				break;
		}

		if ((rfcsr.non_bank.RF_CSR_KICK == IDLE) &&
			(rfcsr.non_bank.TESTCSR_RFACC_REGNUM == regID))
		{
			*pValue = (UCHAR)(rfcsr.non_bank.RF_CSR_DATA);
			break;
		}
	}

	if (rfcsr.non_bank.RF_CSR_KICK == BUSY)
	{
		DBGPRINT_ERR(("%s(): RF read R%d=0x%X fail, i[%d], k[%d]\n",
						__FUNCTION__, regID, rfcsr.word,i,k));
		goto done;
	}
	ret = STATUS_SUCCESS;

done:
#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		up(&pAd->reg_atomic);
	}
#endif /* RTMP_MAC_USB */

	return ret;
}


#endif /* RTMP_RF_RW_SUPPORT */

