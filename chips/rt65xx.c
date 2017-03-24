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
	rt65xx.c

	Abstract:
	Specific funcitons and configurations for RT65xx

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"


#ifdef RTMP_USB_SUPPORT
VOID RT65xxUsbAsicRadioOff(struct rtmp_adapter *pAd, UCHAR Stage)
{
	uint32_t Value, ret;

	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	DISABLE_TX_RX(pAd, RTMP_HALT);

	if (IS_USB_INF(pAd)) {
		ret = down_interruptible(&pAd->hw_atomic);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return;
		}
	}

	RTMP_SET_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);

	if (Stage == MLME_RADIO_OFF)
		PWR_SAVING_OP(pAd, RADIO_OFF, 1, 0, 0, 0, 0);

	MCU_CTRL_EXIT(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

	/* Stop bulkin pipe*/
	//if((pAd->PendingRx > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	if((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		RTUSBCancelPendingBulkInIRP(pAd);
		//pAd->PendingRx = 0;
	}

	if (IS_USB_INF(pAd)) {
		up(&pAd->hw_atomic);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));
}


VOID RT65xxUsbAsicRadioOn(struct rtmp_adapter *pAd, UCHAR Stage)
{
	uint32_t MACValue = 0;
	uint32_t rx_filter_flag;
	WPDMA_GLO_CFG_STRUC GloCfg;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	uint32_t ret;

	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	struct os_cookie * pObj = pAd->OS_Cookie;

	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	if( (RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == 1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: autopm_resume success\n", __FUNCTION__));
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
	}
	else if ((RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == (-1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: autopm_resume fail ------\n", __FUNCTION__));
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		return;
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("%s: autopm_resume do nothing\n", __FUNCTION__));

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	if (pAd->WlanFunCtrl.field.WLAN_EN == 0)
		rlt_wlan_chip_onoff(pAd, TRUE, FALSE);

	/* make some traffic to invoke EvtDeviceD0Entry callback function*/
	MACValue = mt7612u_read32(pAd,0x1000);
	DBGPRINT(RT_DEBUG_TRACE,("A MAC query to invoke EvtDeviceD0Entry, MACValue = 0x%x\n",MACValue));

	/* enable RX of MAC block*/
		rx_filter_flag = STANORMAL;     /* Staion not drop control frame will fail WiFi Certification.*/


	mt7612u_write32(pAd, RX_FILTR_CFG, rx_filter_flag);
	mt7612u_write32(pAd, MAC_SYS_CTRL, 0x0c);

	/* 4. Clear idle flag*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);

	/* Send Bulkin IRPs after flag fRTMP_ADAPTER_IDLE_RADIO_OFF is cleared.*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		RTUSBBulkReceive(pAd);
	}
#endif /* CONFIG_STA_SUPPORT */

	MCU_CTRL_INIT(pAd);

	if (IS_USB_INF(pAd)) {
		ret = down_interruptible(&pAd->hw_atomic);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return;
		}
	}

	mt7612u_write32(pAd, MAC_SYS_CTRL, 0x0c);

	if (IS_USB_INF(pAd)) {
		up(&pAd->hw_atomic);
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);

	if (Stage == MLME_RADIO_ON)
		PWR_SAVING_OP(pAd, RADIO_ON, 1, 0, 0, 0, 0);

	DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));
}
#endif


VOID RT65xxDisableTxRx(struct rtmp_adapter *pAd, UCHAR Level)
{
	uint32_t MacReg = 0;
	uint32_t MTxCycle;
	bool bResetWLAN = FALSE;
	bool bFree = TRUE;
	UINT8 CheckFreeTimes = 0;

	if (!IS_RT65XX(pAd))
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("----> %s\n", __FUNCTION__));

	if (Level == RTMP_HALT)
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);

	DBGPRINT(RT_DEBUG_TRACE, ("%s Tx success = %ld\n",
		__FUNCTION__, (ULONG)pAd->WlanCounters.TransmittedFragmentCount.u.LowPart));
	DBGPRINT(RT_DEBUG_TRACE, ("%s Tx success = %ld\n",
		__FUNCTION__, (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart));

	StopDmaTx(pAd, Level);

	/*
		Check page count in TxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++) {
		bool bFree = TRUE;
		MacReg = mt7612u_read32(pAd, 0x438);
		if (MacReg != 0)
			bFree = FALSE;
		MacReg = mt7612u_read32(pAd, 0xa30);
		if (MacReg & 0x000000FF)
			bFree = FALSE;
		MacReg = mt7612u_read32(pAd, 0xa34);
		if (MacReg & 0xFF00FF00)
			bFree = FALSE;
		if (bFree)
			break;
		if (MacReg == 0xFFFFFFFF) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000) {
		DBGPRINT(RT_DEBUG_ERROR, ("Check TxQ page count max\n"));
		MacReg = mt7612u_read32(pAd, 0x0a30);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a30 = 0x%08x\n", MacReg));

		MacReg = mt7612u_read32(pAd, 0x0a34);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a34 = 0x%08x\n", MacReg));

		MacReg = mt7612u_read32(pAd, 0x438);
		DBGPRINT(RT_DEBUG_TRACE, ("0x438 = 0x%08x\n", MacReg));
		bResetWLAN = TRUE;
	}

	/*
		Check MAC Tx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++) {
		MacReg = mt7612u_read32(pAd, MAC_STATUS_CFG);
		if (MacReg & 0x1)
			RtmpusecDelay(50);
		else
			break;

		if (MacReg == 0xFFFFFFFF) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000) {
		DBGPRINT(RT_DEBUG_ERROR, ("Check MAC Tx idle max(0x%08x)\n", MacReg));
		bResetWLAN = TRUE;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) == FALSE) {
		if (Level == RTMP_HALT) {
			/*
				Disable MAC TX/RX
			*/
			MacReg = mt7612u_read32(pAd, MAC_SYS_CTRL);
			MacReg &= ~(0x0000000c);
			mt7612u_write32(pAd, MAC_SYS_CTRL, MacReg);
		} else {
			/*
				Disable MAC RX
			*/
			MacReg = mt7612u_read32(pAd, MAC_SYS_CTRL);
			MacReg &= ~(0x00000008);
			mt7612u_write32(pAd, MAC_SYS_CTRL, MacReg);
		}
	}

	/*
		Check page count in RxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++) {
		bFree = TRUE;
		MacReg = mt7612u_read32(pAd, 0x430);

		if (MacReg & (0x00FF0000))
			bFree = FALSE;

		MacReg = mt7612u_read32(pAd, 0xa30);

		if (MacReg != 0)
			bFree = FALSE;

		MacReg = mt7612u_read32(pAd, 0xa34);

		if (MacReg != 0)
			bFree = FALSE;

		if (bFree && (CheckFreeTimes > 20))
			break;

		if (bFree)
			CheckFreeTimes++;

		if (MacReg == 0xFFFFFFFF) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
#ifdef RTMP_MAC_USB
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
		usb_rx_cmd_msgs_receive(pAd);
		RTUSBBulkReceive(pAd);
#endif
	}

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);

	if (MTxCycle >= 2000) {
		DBGPRINT(RT_DEBUG_ERROR, ("Check RxQ page count max\n"));

		MacReg = mt7612u_read32(pAd, 0x0a30);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a30 = 0x%08x\n", MacReg));

		MacReg = mt7612u_read32(pAd, 0x0a34);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a34 = 0x%08x\n", MacReg));

		MacReg = mt7612u_read32(pAd, 0x0430);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0430 = 0x%08x\n", MacReg));
		bResetWLAN = TRUE;
	}

	/*
		Check MAC Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++) {
		MacReg = mt7612u_read32(pAd, MAC_STATUS_CFG);
		if (MacReg & 0x2)
			RtmpusecDelay(50);
		else
			break;
		if (MacReg == 0xFFFFFFFF) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000) {
		DBGPRINT(RT_DEBUG_ERROR, ("Check MAC Rx idle max(0x%08x)\n", MacReg));
		bResetWLAN = TRUE;
	}

	StopDmaRx(pAd, Level);

	if ((Level == RTMP_HALT) &&
	    (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) == FALSE)) {

		/*
 		 * Disable RF/MAC and do not do reset WLAN under below cases
 		 * 1. Combo card
 		 * 2. suspend including wow application
 		 * 3. radion off command
 		 */
		if ((pAd->chipCap.IsComboChip) ||
		     RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND)	||
		     RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_CMD_RADIO_OFF))
			bResetWLAN = 0;

		rlt_wlan_chip_onoff(pAd, FALSE, bResetWLAN);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<---- %s\n", __FUNCTION__));
}




VOID dump_bw_info(struct rtmp_adapter *pAd)
{
		uint32_t core_r1, agc_r0, be_r0, band_cfg;
		static UCHAR *bw_str[]={"20", "10", "40", "80"};
		UCHAR bw, prim_ch_idx, decode_cap;
		static UCHAR *decode_str[] = {"0", "20", "40", "20/40",
									"80", "20/80", "40/80", "20/40/80"};
		UCHAR tx_prim;


		core_r1 = RTMP_BBP_IO_READ32(pAd, CORE_R1);
		agc_r0 = RTMP_BBP_IO_READ32(pAd, AGC1_R0);
		be_r0 = RTMP_BBP_IO_READ32(pAd, TXBE_R0);
		band_cfg = mt7612u_read32(pAd, TX_BAND_CFG);

		/*  Tx/RX : control channel setting */
		DBGPRINT(RT_DEBUG_OFF, ("\n%s():RegisterSetting: TX_BAND_CFG=0x%x, CORE_R1=0x%x, AGC1_R0=0x%x, TXBE_R0=0x%x\n",
				__FUNCTION__, band_cfg, core_r1, agc_r0, be_r0));
		bw = ((core_r1 & 0x18) >> 3) & 0xff;
		DBGPRINT(RT_DEBUG_OFF, ("[CORE_R1]\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\tTx/Rx BandwidthCtrl(CORE_R1[4:3])=%d(%s MHz)\n",
					bw, bw_str[bw]));

		DBGPRINT(RT_DEBUG_OFF, ("[AGC_R0]\n"));
		prim_ch_idx = ((agc_r0 & 0x300) >> 8) & 0xff;
		DBGPRINT(RT_DEBUG_OFF, ("\tPrimary Channel Idx(AGC_R0[9:8])=%d\n", prim_ch_idx));
		decode_cap = ((agc_r0 & 0x7000) >> 12);
		DBGPRINT(RT_DEBUG_OFF, ("\tDecodeBWCap(AGC_R0[14:12])=%d(%s MHz Data)\n",
					decode_cap, decode_str[decode_cap]));

		DBGPRINT(RT_DEBUG_OFF, ("[TXBE_R0 - PPM]\n"));
		tx_prim = (be_r0 & 0x3);
		DBGPRINT(RT_DEBUG_OFF, ("\tTxPrimary(TXBE_R0[1:0])=%d\n", tx_prim));
}

