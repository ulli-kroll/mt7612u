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
        Who             When                    What
        --------        ----------              ----------------------------------------------
*/


#include "rt_config.h"


#ifdef RLT_BBP

static INT rlt_bbp_is_ready(struct rtmp_adapter *pAd)
{
	INT idx = 0;
	uint32_t val;

	do
	{
		val = RTMP_BBP_IO_READ32(pAd, CORE_R0);
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return false;
	} while ((++idx < 20) && ((val == 0xffffffff) || (val == 0x0)));

	if (!((val == 0xffffffff) || (val == 0x0)))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BBP version = %x\n", val));
	}

	return (((val == 0xffffffff) || (val == 0x0)) ? false : true);
}


static INT rlt_bbp_init(struct rtmp_adapter *pAd)
{
	INT idx;

	/* Read BBP register, make sure BBP is up and running before write new data*/
	if (rlt_bbp_is_ready(pAd) == false)
		return NDIS_STATUS_FAILURE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Init BBP Registers\n", __FUNCTION__));

	// TODO: shiang-6590, check these bbp registers if need to remap to new BBP_Registers

	return NDIS_STATUS_SUCCESS;

}


static INT rlt_bbp_get_temp(struct rtmp_adapter *pAd, CHAR *temp_val)
{
	return true;
}


static INT rlt_bbp_tx_comp_init(struct rtmp_adapter *pAd, INT adc_insel, INT tssi_mode)
{
	uint32_t bbp_val;
	UCHAR rf_val;

#if defined(RTMP_INTERNAL_TX_ALC)
	bbp_val = RTMP_BBP_IO_READ32(pAd, CORE_R34);
	bbp_val = (bbp_val & 0xe7);
	bbp_val = (bbp_val | 0x80);
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);

	RT30xxReadRFRegister(pAd, RF_R27, &rf_val);
	rf_val = ((rf_val & 0x3f) | 0x40);
	RT30xxWriteRFRegister(pAd, RF_R27, rf_val);

	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Set RF_R27 to 0x%x\n", rf_val));
#endif
	return 0;
}


static INT rlt_bbp_set_txdac(struct rtmp_adapter *pAd, INT tx_dac)
{
	uint32_t txbe, txbe_r5 = 0;

	txbe_r5 = RTMP_BBP_IO_READ32(pAd, TXBE_R5);
	txbe = txbe_r5 & (~0x3);
	switch (tx_dac)
	{
		case 2:
			txbe |= 0x3;
			break;
		case 1:
		case 0:
		default:
			txbe &= (~0x3);
			break;
	}

	if (txbe != txbe_r5)
		RTMP_BBP_IO_WRITE32(pAd, TXBE_R5, txbe);

	return true;
}


static INT rlt_bbp_set_rxpath(struct rtmp_adapter *pAd, INT rxpath)
{
	uint32_t agc, agc_r0 = 0;

	agc_r0 = RTMP_BBP_IO_READ32(pAd, AGC1_R0);
	agc = agc_r0 & (~0x18);
	if(rxpath == 2)
		agc |= (0x8);
	else if(rxpath == 1)
		agc |= (0x0);

	if (agc != agc_r0)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);

//DBGPRINT(RT_DEBUG_OFF, ("%s(): rxpath=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, rxpath, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): rxpath=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, rxpath, agc));

	return true;
}


static UCHAR vht_prim_ch_val[] = {
	42, 36, 0,
	42, 40, 1,
	42, 44, 2,
	42, 48, 3,
	58, 52, 0,
	58, 56, 1,
	58, 60, 2,
	58, 64, 3,
	106, 100, 0,
	106, 104, 1,
	106, 108, 2,
	106, 112, 3,
	122, 116, 0,
	122, 120, 1,
	122, 124, 2,
	122, 128, 3,
	138, 132, 0,
	138, 136, 1,
	138, 140, 2,
	138, 144, 3,
	155, 149, 0,
	155, 153, 1,
	155, 157, 2,
	155, 161, 3
};


void mt7612u_bbp_set_ctrlch(struct rtmp_adapter *pAd, u8 ext_ch)
{
	uint32_t agc, agc_r0 = 0;
	uint32_t be, be_r0 = 0;

	agc_r0 = RTMP_BBP_IO_READ32(pAd, AGC1_R0);
	agc = agc_r0 & (~0x300);
	be_r0 = RTMP_BBP_IO_READ32(pAd, TXBE_R0);
	be = (be_r0 & (~0x03));
	if (pAd->CommonCfg.BBPCurrentBW == BW_80 &&
		pAd->CommonCfg.Channel >= 36 &&
		pAd->CommonCfg.vht_cent_ch)
	{
		if (pAd->CommonCfg.Channel < pAd->CommonCfg.vht_cent_ch)
		{
			switch (pAd->CommonCfg.vht_cent_ch - pAd->CommonCfg.Channel)
			{
				case 6:
					be |= 0;
					agc |=0x000;
					break;
				case 2:
					be |= 1;
					agc |=0x100;
					break;

			}
		}
		else if (pAd->CommonCfg.Channel > pAd->CommonCfg.vht_cent_ch)
		{
			switch (pAd->CommonCfg.Channel - pAd->CommonCfg.vht_cent_ch)
			{
				case 6:
					be |= 0x3;
					agc |=0x300;
					break;
				case 2:
					be |= 0x2;
					agc |=0x200;
					break;
			}
		}
	}
	else
	{
		switch (ext_ch)
		{
			case EXTCHA_BELOW:
				agc |= 0x100;
				be |= 0x01;
				break;
			case EXTCHA_ABOVE:
				agc &= (~0x300);
				be &= (~0x03);
				break;
			case EXTCHA_NONE:
			default:
				agc &= (~0x300);
				be &= (~0x03);
				break;
		}
	}
	if (agc != agc_r0)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);

	if (be != be_r0)
		RTMP_BBP_IO_WRITE32(pAd, TXBE_R0, be);

//DBGPRINT(RT_DEBUG_OFF, ("%s(): ext_ch=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, ext_ch, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): ext_ch=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, ext_ch, agc));
}


/*
	<<Gamma2.1 Control Registers Rev1.3.pdf>>
	BBP bandwidth (CORE_R1[4:3]) change procedure:
	1. Hold BBP in reset by setting CORE_R4[0] to '1'
	2. Wait 0.5 us to ensure BBP is in the idle State
	3. Change BBP bandwidth with CORE_R1[4:3]
		CORE_R1 (Bit4:3)
		0: 20MHz
		1: 10MHz (11J)
		2: 40MHz
		3: 80MHz
	4. Wait 0.5 us for BBP clocks to settle
	5. Release BBP from reset by clearing CORE_R4[0]
*/
void mt7612u_bbp_set_bw(struct rtmp_adapter *pAd, u8 bw)
{
	uint32_t core, core_r1 = 0;
	uint32_t agc, agc_r0 = 0;

#if defined(MT76x0) || defined(MT76x2)
	uint32_t core_r4;
#endif /* defined(MT76x0) || defined(MT76x2) */

#ifdef RTMP_USB_SUPPORT

	if (IS_USB_INF(pAd)) {
		uint32_t ret;

		ret = down_interruptible(&pAd->hw_atomic);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return;
		}
	}
#endif /* RTMP_USB_SUPPORT */

	core_r1 = RTMP_BBP_IO_READ32(pAd, CORE_R1);
		core = (core_r1 & (~0x18));
	agc_r0 = RTMP_BBP_IO_READ32(pAd, AGC1_R0);
	agc = agc_r0 & (~0x7000);
	switch (bw)
	{
		case BW_80:
			core |= 0x18;
			agc |= 0x7000;
			break;
		case BW_40:
			core |= 0x10;
			agc |= 0x3000;
			break;
		case BW_20:
			core &= (~0x18);
			agc |= 0x1000;
			break;
		case BW_10:
			core |= 0x08;
			agc |= 0x1000;
			break;
	}

	if (core != core_r1) {
		RTMP_BBP_IO_WRITE32(pAd, CORE_R1, core);
	}

	if (agc != agc_r0) {
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): bw=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, bw, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): bw=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, bw, agc));
	}

	pAd->CommonCfg.BBPCurrentBW = bw;

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		up(&pAd->hw_atomic);
	}
#endif /* RTMP_MAC_USB */

	return;
}


static INT rlt_bbp_set_mmps(struct rtmp_adapter *pAd, bool ReduceCorePower)
{
	uint32_t bbp_val, org_val;

	org_val = RTMP_BBP_IO_READ32(pAd, AGC1_R0);
	bbp_val = org_val;
	if (ReduceCorePower)
		bbp_val |= 0x04;
	else
		bbp_val &= ~0x04;

	if (bbp_val != org_val)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, bbp_val);

	return true;
}


static INT rlt_bbp_get_agc(struct rtmp_adapter *pAd, CHAR *agc, RX_CHAIN_IDX chain)
{
	UCHAR idx, val;
	uint32_t bbp_val, bbp_reg = AGC1_R8;


	if (((pAd->MACVersion & 0xffff0000) < 0x28830000) ||
		(pAd->Antenna.field.RxPath == 1))
	{
		chain = RX_CHAIN_0;
	}

	idx = val = 0;
	while(chain != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (chain & 0x01)
		{
			bbp_val = RTMP_BBP_IO_READ32(pAd, bbp_reg);
			val = ((bbp_val & (0x0000ff00)) >> 8) & 0xff;
			break;
		}
		chain >>= 1;
		bbp_reg += 4;
		idx++;
	}

	*agc = val;

	return NDIS_STATUS_SUCCESS;
}


static INT rlt_bbp_set_agc(struct rtmp_adapter *pAd, UCHAR agc, RX_CHAIN_IDX chain)
{
	UCHAR idx = 0;
	uint32_t bbp_val, bbp_reg = AGC1_R8;

	if (((pAd->MACVersion & 0xf0000000) < 0x28830000) ||
		(pAd->Antenna.field.RxPath == 1))
	{
		chain = RX_CHAIN_0;
	}

	while (chain != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (idx & 0x01)
		{
			bbp_val = RTMP_BBP_IO_READ32(pAd, bbp_reg);
			bbp_val = (bbp_val & 0xffff00ff) | (agc << 8);
			RTMP_BBP_IO_WRITE32(pAd, bbp_reg, bbp_val);

			DBGPRINT(RT_DEBUG_INFO,
					("%s(Idx):Write(R%d,val:0x%x) to Chain(0x%x, idx:%d)\n",
						__FUNCTION__, bbp_reg, bbp_val, chain, idx));
		}
		chain >>= 1;
		bbp_reg += 4;
		idx++;
	}

	return true;
}


static INT rlt_bbp_set_filter_coefficient_ctrl(struct rtmp_adapter *pAd, UCHAR Channel)
{
	uint32_t bbp_val = 0, org_val = 0;

	if (Channel == 14)
	{
		/* when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
		bbp_val = RTMP_BBP_IO_READ32(pAd, CORE_R1);
		bbp_val = org_val;
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
			bbp_val |= 0x20;
		else
			bbp_val &= (~0x20);

		if (bbp_val != org_val)
			RTMP_BBP_IO_WRITE32(pAd, CORE_R1, bbp_val);
	}

	return true;
}


static UCHAR rlt_bbp_get_random_seed(struct rtmp_adapter *pAd)
{
	uint32_t value, value2;
	UCHAR seed;

	value = RTMP_BBP_IO_READ32(pAd, AGC1_R16);
	seed = (UCHAR)((value & 0xff) ^ ((value & 0xff00) >> 8)^
					((value & 0xff0000) >> 16));
	value2 = RTMP_BBP_IO_READ32(pAd, RXO_R9);

	return (UCHAR)(seed ^ (value2 & 0xff)^ ((value2 & 0xff00) >> 8));
}


static struct phy_ops rlt_phy_ops = {
	.bbp_init = rlt_bbp_init,
	.bbp_is_ready = rlt_bbp_is_ready,
	.get_random_seed_by_phy = rlt_bbp_get_random_seed,
	.filter_coefficient_ctrl = rlt_bbp_set_filter_coefficient_ctrl,
	.bbp_set_agc = rlt_bbp_set_agc,
	.bbp_get_agc = rlt_bbp_get_agc,
	.bbp_set_rxpath = rlt_bbp_set_rxpath,
	.bbp_set_txdac = rlt_bbp_set_txdac,
	.bbp_set_mmps = rlt_bbp_set_mmps,
	.bbp_tx_comp_init = rlt_bbp_tx_comp_init,
	.bbp_get_temp = rlt_bbp_get_temp,
};


INT rlt_phy_probe(struct rtmp_adapter *pAd)
{
	pAd->phy_op = &rlt_phy_ops;

	return true;
}

#endif /* RLT_BBP */

