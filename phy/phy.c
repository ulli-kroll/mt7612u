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


INT phy_probe(struct rtmp_adapter *pAd)
{

	return true;
}


static int mt7612u_bbp_is_ready(struct rtmp_adapter *pAd)
{
	INT idx = 0;
	uint32_t val;

	do {
		val = RTMP_BBP_IO_READ32(pAd, CORE_R0);
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return false;
	} while ((++idx < 20) && ((val == 0xffffffff) || (val == 0x0)));

	if (!((val == 0xffffffff) || (val == 0x0)))
		DBGPRINT(RT_DEBUG_TRACE, ("BBP version = %x\n", val));

	return (((val == 0xffffffff) || (val == 0x0)) ? false : true);
}

int mt7612u_phy_init_bbp(struct rtmp_adapter *pAd)
{
	uint32_t Index = 0, val;

	/* Before program BBP, we need to wait BBP/RF get wake up.*/
	do {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return NDIS_STATUS_FAILURE;

		val = mt7612u_read32(pAd, MAC_STATUS_CFG);
		if ((val & 0x03) == 0)	/* if BB.RF is stable*/
			break;

		DBGPRINT(RT_DEBUG_TRACE, ("Check if MAC_STATUS_CFG is busy(=%x)\n", val));
		RtmpusecDelay(1000);
	} while (Index++ < 100);

	/* Read BBP register, make sure BBP is up and running before write new data*/
	if (mt7612u_bbp_is_ready(pAd) == false)
		return NDIS_STATUS_FAILURE;

	// TODO: shiang-6590, check these bbp registers if need to remap to new BBP_Registers

	return NDIS_STATUS_SUCCESS;

}

int mt7612u_phy_bbp_get_agc(struct rtmp_adapter *pAd, CHAR *agc, RX_CHAIN_IDX chain)
{
	u8 idx, val;
	uint32_t bbp_val, bbp_reg = AGC1_R8;


	if (((pAd->MACVersion & 0xffff0000) < 0x28830000) ||
	    (pAd->Antenna.field.RxPath == 1)) {
		chain = RX_CHAIN_0;
	}

	idx = val = 0;
	while(chain != 0) {
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (chain & 0x01) {
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


int mt7612u_phy_bbp_set_agc(struct rtmp_adapter *pAd, u8 agc, RX_CHAIN_IDX chain)
{
	u8 idx = 0;
	uint32_t bbp_val, bbp_reg = AGC1_R8;

	if (((pAd->MACVersion & 0xf0000000) < 0x28830000) ||
	     (pAd->Antenna.field.RxPath == 1)) {
		chain = RX_CHAIN_0;
	}

	while (chain != 0) {
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (idx & 0x01) {
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

u8 get_random_seed_by_phy(struct rtmp_adapter *pAd)
{
	if (pAd->phy_op && pAd->phy_op->get_random_seed_by_phy)
		return pAd->phy_op->get_random_seed_by_phy(pAd);
	else
		return 0;
}

