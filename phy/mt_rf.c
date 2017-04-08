/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_rf.c
*/

#include "rt_config.h"

int mt_rf_write(
	struct rtmp_adapter *ad,
	u8 rf_idx,
	u16 offset,
	u32 data)
{
	u32 i = 0;
	u32 value;
	int ret = 0;

	if (IS_USB_INF(ad)) {
		ret = down_interruptible(&ad->reg_atomic);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return STATUS_UNSUCCESSFUL;
		}
	}

	/* rf data */
	mt7612u_write32(ad, W_RFDATA, data);

	/* rf control */
	value = mt7612u_read32(ad, RF_CTRL);

	/* rf address */
	value &= ~RF_ADDR_MASK;
	value |= RF_ADDR(offset);

	/* write control */
	value |= RF_R_W_CTRL;

	/* rf index */
	value &= ~RF_IDX_MASK;
	value |= RF_IDX(rf_idx);

	mt7612u_write32(ad, RF_CTRL, value);

	do {
		value = mt7612u_read32(ad, RF_CTRL);

		if (RF_READY(value))
			break;
		i++;
		//RtmpOsMsDelay(1);
		RtmpusecDelay(50);
	//} while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)));
	} while ((i < MAX_BUSY_COUNT_US) && (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	//if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
	if ((i == MAX_BUSY_COUNT_US) || (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		ret = STATUS_UNSUCCESSFUL;
	}

	if (IS_USB_INF(ad))
		up(&ad->reg_atomic);

	return ret;
}

