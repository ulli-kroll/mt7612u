/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	rtusb_io.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
	Paul Lin    06-25-2004  created
*/



#include	"rt_config.h"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
#include <linux/bitfield.h>
#else
/* Force a compilation error if a constant expression is not a power of 2 */
#define __BUILD_BUG_ON_NOT_POWER_OF_2(n)	\
	BUILD_BUG_ON(((n) & ((n) - 1)) != 0)
#define BUILD_BUG_ON_NOT_POWER_OF_2(n)			\
	BUILD_BUG_ON((n) == 0 || (((n) & ((n) - 1)) != 0))
#include <bitfield.h>
#endif

#define MAX_VENDOR_REQ_RETRY_COUNT  10

/*
	========================================================================

	Routine Description: NIC initialization complete

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/

int RTUSBMultiWrite(
	IN struct rtmp_adapter *pAd,
	IN unsigned short Offset,
	IN u8 *pData,
	IN unsigned short length)
{
	int Status;
	unsigned short index = 0,Value;
	u8 *pSrc = pData;
	unsigned short resude = 0;

	resude = length % 2;
	length  += resude;
	do {
		Value =(unsigned short)( *pSrc  | (*(pSrc + 1) << 8));
		Status = RTUSBSingleWrite(pAd,Offset + index, Value);
		index +=2;
		length -= 2;
		pSrc = pSrc + 2;
	}while(length > 0);

	return Status;
}


int RTUSBSingleWrite(
	IN 	struct rtmp_adapter *pAd,
	IN	unsigned short Offset,
	IN	unsigned short Value)
{
	bool WriteHigh = false;

	return mt76u_vendor_request(pAd, DEVICE_VENDOR_REQUEST_OUT,
				   MT_VEND_SINGLE_WRITE, Value, Offset, NULL, 0);
}


/*
	========================================================================

	Routine Description: Read 32-bit MAC register

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
u32 mt76u_reg_read(struct rtmp_adapter *pAd, unsigned short Offset)
{
	int Status = 0;
	u32 val;

	Status = mt76u_vendor_request(pAd, DEVICE_VENDOR_REQUEST_IN,
				     MT_VEND_MULTI_READ, 0, Offset,
				     &val, 4);

	if (Status != 0)
		val =0xffffffff;

	return le2cpu32(val);
}


/*
	========================================================================

	Routine Description: Write 32-bit MAC register

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
void mt76u_reg_write(struct rtmp_adapter *pAd, unsigned short Offset,
			       u32 _val)
{
	u32 val = cpu2le32(_val);


	mt76u_vendor_request(pAd, DEVICE_VENDOR_REQUEST_OUT,
			     MT_VEND_MULTI_WRITE, 0, Offset,
			     &val, 4);
}


/*
 * ULLI : only for mt7612u/mt7662 chipsets
 * ULLI : new registers
 */

int mt76u_sys_write(struct rtmp_adapter *ad, uint16_t offset, uint32_t val)
{
	int ret;
#if 0
	UINT8 req;
#endif
	uint32_t io_value;

#if 0	/* ULLI : this remains currently here as remark */
	if (base == 0x40)
		req = 0x46;

	req = 0x46;
#endif

	io_value = cpu2le32(val);

	ret = mt76u_vendor_request(ad, DEVICE_VENDOR_REQUEST_OUT,
				  MT_VEND_SYS_WRITE, 0, offset,
				  &io_value, 4);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("write reg fail\n"));
	}

	return ret;
}

/*
 * ULLI : only for mt7612u/mt7662 chipsets
 * ULLI : new registers
 */

u32 mt76u_sys_read(struct rtmp_adapter *ad, uint16_t offset)
{
	int ret;
#if 0
	UINT8 req;
#endif
	u32 val;

#if 0	/* ULLI : this remains currently here as remark */
	if (base == 0x40)
		req = 0x47;
	else if (base == 0x41)
		req = 0x7;

	req = 0x47;
#endif
	ret = mt76u_vendor_request(ad, DEVICE_VENDOR_REQUEST_IN,
				  MT_VEND_SYS_READ, 0, offset,
				  &val, 4);

	if (ret != 0)
		val =0xffffffff;

	return le2cpu32(val);
}

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/

u16 mt76u_read_eeprom(struct rtmp_adapter *pAd, unsigned short offset)
{
	u16 val = 0;

	mt76u_vendor_request(pAd, DEVICE_VENDOR_REQUEST_IN,
				   MT_VEND_READ_EEPROM, 0, offset, &val, 2);

	return le2cpu16(val);
}

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
int RTUSBWakeUp(struct rtmp_adapter *pAd)
{
	return mt76u_vendor_request(pAd, DEVICE_VENDOR_REQUEST_OUT,
				   MT_VEND_DEVICE_MODE, 0x09, 0, NULL, 0);
}

/*
    ========================================================================
 	Routine Description:
		mt76u_vendor_request - Builds a ralink specific request, sends it off to USB endpoint zero and waits for completion

	Arguments:
		@pAd:
	  	@TransferFlags:
	  	@RequestType: USB message request type value
	  	@Request: USB message request value
	  	@Value: USB message value
	  	@Index: USB message index value
	  	@TransferBuffer: USB data to be sent
	  	@TransferBufferLength: Lengths in bytes of the data to be sent

	Context: ! in atomic context

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	Note:
		This function sends a simple control message to endpoint zero
		and waits for the message to complete, or CONTROL_TIMEOUT_JIFFIES timeout.
		Because it is synchronous transfer, so don't use this function within an atomic context,
		otherwise system will hang, do be careful.

		TransferBuffer may located in stack region which may not in DMA'able region in some embedded platforms,
		so need to copy TransferBuffer to vend_buf allocated by kmalloc to do DMA transfer.
		Use UsbVendorReq_semaphore to protect this region which may be accessed by multi task.
		Normally, coherent issue is resloved by low-level HC driver, so do not flush this zone by mt76u_vendor_request.

	========================================================================
*/

/*
 * ULLI : on mt7612u we have three different request type
 *
 * DEVICE_VENDOR_REQUEST_OUT   	0x40
 * DEVICE_VENDOR_REQUEST_IN    	0xc0
 * DEVICE_CLASS_REQUEST_OUT	0x20
 *
 * the latter one controls the rom patch command and usb reset !!
 * if we remove DEVICE_CLASS_REQUEST_OUT request type the device becomes unusable
 *
 */


int mt76u_vendor_request(struct rtmp_adapter *pAd, u8 requesttype, enum mt_vendor_req request,
			u16 value, u16 index, void *data, u16 size)
{
	int ret = 0;
	struct usb_device *udev = mt7612u_to_usb_dev(pAd);

	if(in_interrupt()) {
		DBGPRINT(RT_DEBUG_ERROR, ("BUG: mt76u_vendor_request is called from invalid context\n"));
		return NDIS_STATUS_FAILURE;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		/*DBGPRINT(RT_DEBUG_ERROR, ("WIFI device has been disconnected\n"));*/
		return NDIS_STATUS_FAILURE;
	} else if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP)) {
		DBGPRINT(RT_DEBUG_ERROR, ("MCU has entered sleep mode\n"));
		return NDIS_STATUS_FAILURE;
	} else {
		int RetryCount = 0; /* RTUSB_CONTROL_MSG retry counts*/
		ASSERT(size <MAX_PARAM_BUFFER_SIZE);

		ret = down_interruptible(&(pAd->UsbVendorReq_semaphore));
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("UsbVendorReq_semaphore get failed\n"));
			return NDIS_STATUS_FAILURE;
		}

		if ((size > 0) && ((requesttype == DEVICE_VENDOR_REQUEST_OUT) || (requesttype == DEVICE_CLASS_REQUEST_OUT)))
			memmove(pAd->vend_buf, data, size);

		do {
			int pipe;

			if (requesttype == DEVICE_VENDOR_REQUEST_OUT ||
			    requesttype == DEVICE_CLASS_REQUEST_OUT)
				pipe = usb_sndctrlpipe(udev, 0);
			else
				pipe = usb_rcvctrlpipe(udev, 0);

			ret = usb_control_msg(udev, pipe, request,
					      requesttype, value, index,
					      pAd->vend_buf,
					      size, CONTROL_TIMEOUT_JIFFIES);

			if (ret < 0) {
				DBGPRINT(RT_DEBUG_OFF, ("#\n"));
				if (ret == -ENODEV) {
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
					break;
				}
				RetryCount++;
				mdelay(5); /* wait for 5ms*/
			}
		} while((ret < 0) && (RetryCount < MAX_VENDOR_REQ_RETRY_COUNT));

		if (ret >= 0 && size > 0 && requesttype == DEVICE_VENDOR_REQUEST_IN)
			memmove(data, pAd->vend_buf, size);

		up(&(pAd->UsbVendorReq_semaphore));

		if (ret < 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("mt76u_vendor_request failed(%d), ReqType=%s, Req=0x%x, Idx=0x%x,pAd->Flags=0x%lx\n",
						ret, (requesttype == DEVICE_VENDOR_REQUEST_OUT ? "OUT" : "IN"), request, index, pAd->Flags));

			if (request == MT_VEND_SINGLE_WRITE)
				DBGPRINT(RT_DEBUG_ERROR, ("\tRequest Value=0x%04x!\n", value));

			if (ret == -ENODEV)
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);

		}

	}

	if (ret < 0)
		return NDIS_STATUS_FAILURE;
	else
		return NDIS_STATUS_SUCCESS;

}

int CheckGPIOHdlr(struct rtmp_adapter *pAd, PCmdQElmt CMDQelmt)
{
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		uint32_t data;

		/* Read GPIO pin2 as Hardware controlled radio state*/
		data = mt76u_reg_read( pAd, GPIO_CTRL_CFG);
		pAd->StaCfg.bHwRadio = (data & 0x04) ? true : false;

		if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio)) {
			pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! Radio %s !!!\n",
							(pAd->StaCfg.bRadio == true ? "On" : "Off")));
			if (pAd->StaCfg.bRadio == true) {
				MlmeRadioOn(pAd);
				pAd->ExtraInfo = EXTRA_INFO_CLEAR;
			} else {
				MlmeRadioOff(pAd);
				pAd->ExtraInfo = HW_RADIO_OFF;
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


static int ResetBulkOutHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	int32_t MACValue = 0;
	u8 Index = 0;
	int ret=0;
	PHT_TX_CONTEXT	pHTTXContext;
	unsigned long IrqFlags;

	DBGPRINT(RT_DEBUG_TRACE, ("CMDTHREAD_RESET_BULK_OUT(ResetPipeid=0x%0x)===>\n", pAd->bulkResetPipeid));

	/* All transfers must be aborted or cancelled before attempting to reset the pipe. */
	/*RTUSBCancelPendingBulkOutIRP(pAd);*/
	/* Wait 10ms to let previous packet that are already in HW FIFO to clear. by MAXLEE 12-25-2007*/
	do {
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			break;

		MACValue = mt76u_reg_read(pAd, TXRXQ_PCNT);
		if ((MACValue & 0xf00000/*0x800000*/) == 0)
			break;

		Index++;
		mdelay(10);
	} while(Index < 100);

	MACValue = mt76u_sys_read(pAd, U3DMA_WLCFG);

	/* 2nd, to prevent Read Register error, we check the validity.*/
	if ((MACValue & 0xc00000) == 0)
		MACValue = mt76u_sys_read(pAd, U3DMA_WLCFG);

	/* 3rd, to prevent Read Register error, we check the validity.*/
	if ((MACValue & 0xc00000) == 0)
		MACValue = mt76u_sys_read(pAd, U3DMA_WLCFG);

	MACValue |= 0x80000;
	mt76u_sys_write(pAd, U3DMA_WLCFG, MACValue);

	/* Wait 1ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007*/
	mdelay(1);

	MACValue &= (~0x80000);
	mt76u_sys_write(pAd, U3DMA_WLCFG, MACValue);
	DBGPRINT(RT_DEBUG_TRACE, ("\tSet 0x2a0 bit19. Clear USB DMA TX path\n"));

	/* Wait 5ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007*/
	/* mdelay(5);*/

	if ((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG) {
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

		if (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE /* pMLMEContext->bWaitingBulkOut == true */)
			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);

		RTUSBKickBulkOut(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("\tTX MGMT RECOVER Done!\n"));
	} else {
		pHTTXContext = &(pAd->TxContext[pAd->bulkResetPipeid]);

		/*spin_lock_bh(&pAd->BulkOutLock[pAd->bulkResetPipeid]);*/
		spin_lock_irqsave(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
		if ( pAd->BulkOutPending[pAd->bulkResetPipeid] == false) {
			pAd->BulkOutPending[pAd->bulkResetPipeid] = true;
			pHTTXContext->IRPPending = true;
			pAd->watchDogTxPendingCnt[pAd->bulkResetPipeid] = 1;

			/* no matter what, clean the flag*/
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			/*spin_unlock_bh(&pAd->BulkOutLock[pAd->bulkResetPipeid]);*/
			spin_unlock_irqrestore(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

			RTUSBInitHTTxDesc(pAd, pHTTXContext,
					  pAd->bulkResetPipeid,
					  pHTTXContext->BulkOutSize,
					  RtmpUsbBulkOutDataPacketComplete);

			if ((ret = usb_submit_urb(pHTTXContext->pUrb, GFP_ATOMIC)) != 0) {
					spin_lock_irqsave(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
					pAd->BulkOutPending[pAd->bulkResetPipeid] = false;
					pHTTXContext->IRPPending = false;
					pAd->watchDogTxPendingCnt[pAd->bulkResetPipeid] = 0;
					spin_unlock_irqrestore(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

					DBGPRINT(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_OUT:Submit Tx URB failed %d\n", ret));
			} else {
					spin_lock_irqsave(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

					DBGPRINT(RT_DEBUG_TRACE,("\tCMDTHREAD_RESET_BULK_OUT: TxContext[%d]:CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d, pending=%d!\n",
										pAd->bulkResetPipeid, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition,
										pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad,
										pAd->BulkOutPending[pAd->bulkResetPipeid]));
					DBGPRINT(RT_DEBUG_TRACE,("\t\tBulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n",
										pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther));

					spin_unlock_irqrestore(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

					DBGPRINT(RT_DEBUG_TRACE, ("\tCMDTHREAD_RESET_BULK_OUT: Submit Tx DATA URB for failed BulkReq(0x%lx) Done, status=%d!\n",
										pAd->bulkResetReq[pAd->bulkResetPipeid],
										pHTTXContext->pUrb->status));
			}
		} else {
			/*spin_unlock_bh(&pAd->BulkOutLock[pAd->bulkResetPipeid]);*/
			/*spin_unlock_irqrestore(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);*/

			DBGPRINT(RT_DEBUG_ERROR, ("CmdThread : TX DATA RECOVER FAIL for BulkReq(0x%lx) because BulkOutPending[%d] is true!\n",
								pAd->bulkResetReq[pAd->bulkResetPipeid], pAd->bulkResetPipeid));

			if (pAd->bulkResetPipeid == 0) 	{
				u8 pendingContext = 0;
				PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)(&pAd->TxContext[pAd->bulkResetPipeid ]);
				PTX_CONTEXT pMLMEContext = (PTX_CONTEXT)(pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa);
				PTX_CONTEXT pNULLContext = (PTX_CONTEXT)(&pAd->PsPollContext);
				PTX_CONTEXT pPsPollContext = (PTX_CONTEXT)(&pAd->NullContext);

				if (pHTTXContext->IRPPending)
					pendingContext |= 1;
				else if (pMLMEContext->IRPPending)
					pendingContext |= 2;
				else if (pNULLContext->IRPPending)
					pendingContext |= 4;
				else if (pPsPollContext->IRPPending)
					pendingContext |= 8;
				else
					pendingContext = 0;

				DBGPRINT(RT_DEBUG_ERROR, ("\tTX Occupied by %d!\n", pendingContext));
			}

			/* no matter what, clean the flag*/
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			spin_unlock_irqrestore(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

			RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << pAd->bulkResetPipeid));
		}

		RTMPDeQueuePacket(pAd, false, NUM_OF_TX_RING, MAX_TX_PROCESS);
		/*RTUSBKickBulkOut(pAd);*/
	}

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_OUT<===\n"));
	return NDIS_STATUS_SUCCESS;


}


/* All transfers must be aborted or cancelled before attempting to reset the pipe.*/
static int ResetBulkInHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN === >\n"));

#ifdef CONFIG_STA_SUPPORT
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)) {
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
		return NDIS_STATUS_SUCCESS;
	}
#endif /* CONFIG_STA_SUPPORT */

	/*while ((atomic_read(&pAd->PendingRx) > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) */
	if ((pAd->PendingRx > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("BulkIn IRP Pending!!!\n"));
		RTUSBCancelPendingBulkInIRP(pAd);
		mdelay(100);
		pAd->PendingRx = 0;
	}

	/* Wait 10ms before reading register.*/
	mdelay(10);

	/* It must be removed. Or ATE will have no RX success. */
	if ((!(RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF |
				    fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))))
	{
		u8 i;

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF |
					 fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))
			return NDIS_STATUS_SUCCESS;

		pAd->NextRxBulkInPosition = pAd->RxContext[pAd->NextRxBulkInIndex].BulkInOffset;

		DBGPRINT(RT_DEBUG_TRACE, ("BULK_IN_RESET: NBIIdx=0x%x,NBIRIdx=0x%x, BIRPos=0x%lx. BIReq=x%lx, BIComplete=0x%lx, BICFail0x%lx\n",
					pAd->NextRxBulkInIndex,  pAd->NextRxBulkInReadIndex, pAd->NextRxBulkInPosition, pAd->BulkInReq, pAd->BulkInComplete, pAd->BulkInCompleteFail));

		for (i = 0; i < RX_RING_SIZE; i++) {
 			DBGPRINT(RT_DEBUG_TRACE, ("\tRxContext[%d]: IRPPending=%d, InUse=%d, Readable=%d!\n"
							, i, pAd->RxContext[i].IRPPending, pAd->RxContext[i].InUse, pAd->RxContext[i].Readable));
		}

		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);

		for (i = 0; i < pAd->CommonCfg.NumOfBulkInIRP; i++) {
			/*RTUSBBulkReceive(pAd);*/
			PRX_CONTEXT		pRxContext;
			struct urb *		pUrb;
			int				ret = 0;
			unsigned long	IrqFlags;

			spin_lock_bh(&pAd->BulkInLock);
			pRxContext = &(pAd->RxContext[pAd->NextRxBulkInIndex]);

			if ((pAd->PendingRx > 0) ||
			    (pRxContext->Readable == true) ||
			    (pRxContext->InUse == true)) {
				spin_unlock_bh(&pAd->BulkInLock);
				return NDIS_STATUS_SUCCESS;
			}

			pRxContext->InUse = true;
			pRxContext->IRPPending = true;
			pAd->PendingRx++;
			pAd->BulkInReq++;
			spin_unlock_bh(&pAd->BulkInLock);

			/* Init Rx context descriptor*/
			RTUSBInitRxDesc(pAd, pRxContext);
			pUrb = pRxContext->pUrb;
			if ((ret = usb_submit_urb(pUrb, GFP_ATOMIC))!=0)
			{	/* fail*/
				spin_lock_bh(&pAd->BulkInLock);
				pRxContext->InUse = false;
				pRxContext->IRPPending = false;
				pAd->PendingRx--;
				pAd->BulkInReq--;
				spin_unlock_bh(&pAd->BulkInLock);
				DBGPRINT(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_IN: Submit Rx URB failed(%d), status=%d\n", ret, pUrb->status));
			}
			else
			{	/* success*/
				/*DBGPRINT(RT_DEBUG_TRACE, ("BIDone, Pend=%d,BIIdx=%d,BIRIdx=%d!\n", */
				/*							pAd->PendingRx, pAd->NextRxBulkInIndex, pAd->NextRxBulkInReadIndex));*/
				DBGPRINT_RAW(RT_DEBUG_TRACE, ("CMDTHREAD_RESET_BULK_IN: Submit Rx URB Done, status=%d!\n", pUrb->status));
				ASSERT((pRxContext->InUse == pRxContext->IRPPending));
			}
		}

	} else {
		/* Card must be removed*/
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_IN: Cannot do bulk in because flags(0x%lx) on !\n", pAd->Flags));
	}

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN <===\n"));
	return NDIS_STATUS_SUCCESS;
}


static int SetAsicWcidHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_SET_ASIC_WCID	SetAsicWcid;
	unsigned short 	offset;
	uint32_t 	MACValue, MACRValue = 0;
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
		return NDIS_STATUS_FAILURE;

	offset = MAC_WCID_BASE + ((u8)SetAsicWcid.WCID)*HW_WCID_ENTRY_SIZE;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_SET_ASIC_WCID : WCID = %ld, SetTid  = %lx, DeleteTid = %lx.\n",
						SetAsicWcid.WCID, SetAsicWcid.SetTid, SetAsicWcid.DeleteTid));

	MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[3]<<24)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[2]<<16)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[1]<<8)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[0]);

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("1-MACValue= %x,\n", MACValue));
	mt76u_reg_write(pAd, offset, MACValue);
	/* Read bitmask*/
	MACRValue = mt76u_reg_read(pAd, offset+4);
	if (SetAsicWcid.DeleteTid != 0xffffffff)
		MACRValue &= (~SetAsicWcid.DeleteTid);
	if (SetAsicWcid.SetTid != 0xffffffff)
		MACRValue |= (SetAsicWcid.SetTid);

	MACRValue &= 0xffff0000;
	MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[5]<<8)+pAd->MacTab.Content[SetAsicWcid.WCID].Addr[4];
	MACValue |= MACRValue;
	mt76u_reg_write(pAd, offset+4, MACValue);

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("2-MACValue= %x,\n", MACValue));

	return NDIS_STATUS_SUCCESS;
}

static int DelAsicWcidHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
		return NDIS_STATUS_FAILURE;

        AsicDelWcidTab(pAd, (u8)SetAsicWcid.WCID);

        return NDIS_STATUS_SUCCESS;
}

static int SetWcidSecInfoHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_SEC_INFO pInfo;

	pInfo = (PRT_ASIC_WCID_SEC_INFO)CMDQelmt->buffer;
	RTMPSetWcidSecurityInfo(pAd, pInfo->BssIdx,
				pInfo->KeyIdx, pInfo->CipherAlg,
				pInfo->Wcid, pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}


static int SetAsicWcidIVEIVHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_IVEIV_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_IVEIV_ENTRY)CMDQelmt->buffer;
	AsicUpdateWCIDIVEIV(pAd, pInfo->Wcid,
			    pInfo->Iv, pInfo->Eiv);

	return NDIS_STATUS_SUCCESS;
}


static int SetAsicWcidAttrHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_ATTR_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_ATTR_ENTRY)CMDQelmt->buffer;
	AsicUpdateWcidAttributeEntry(pAd, pInfo->BssIdx,
				     pInfo->KeyIdx, pInfo->CipherAlg,
				     pInfo->Wcid, pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}

static int SETAsicSharedKeyHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_SHARED_KEY pInfo;

	pInfo = (PRT_ASIC_SHARED_KEY)CMDQelmt->buffer;
	AsicAddSharedKeyEntry(pAd, pInfo->BssIndex,
			      pInfo->KeyIdx, &pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}

static int SetAsicPairwiseKeyHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PAIRWISE_KEY pInfo;

	pInfo = (PRT_ASIC_PAIRWISE_KEY)CMDQelmt->buffer;
	AsicAddPairwiseKeyEntry(pAd, pInfo->WCID,
				&pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_STA_SUPPORT
static int SetPortSecuredHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	STA_PORT_SECURED(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT */


static int RemovePairwiseKeyHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	u8 Wcid = *((u8 *)(CMDQelmt->buffer));

	AsicRemovePairwiseKeyEntry(pAd, Wcid);
	return NDIS_STATUS_SUCCESS;
}


static int SetClientMACEntryHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_SET_ASIC_WCID pInfo;

	pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;
	AsicUpdateRxWCIDTable(pAd, pInfo->WCID, pInfo->Addr);
	return NDIS_STATUS_SUCCESS;
}


static int UpdateProtectHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PROTECT_INFO pAsicProtectInfo;

	pAsicProtectInfo = (PRT_ASIC_PROTECT_INFO)CMDQelmt->buffer;
	AsicUpdateProtect(pAd, pAsicProtectInfo->OperationMode,
			  pAsicProtectInfo->SetMask,
			  pAsicProtectInfo->bDisableBGProtect,
			  pAsicProtectInfo->bNonGFExist);

	return NDIS_STATUS_SUCCESS;
}



#ifdef CONFIG_AP_SUPPORT
static int APUpdateCapabilityAndErpieHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	APUpdateCapabilityAndErpIe(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
static int _802_11_CounterMeasureHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MAC_TABLE_ENTRY *pEntry;

		pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
		HandleCounterMeasure(pAd, pEntry);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static int SetPSMBitHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		unsigned short *pPsm = (unsigned short *)CMDQelmt->buffer;
		MlmeSetPsmBit(pAd, *pPsm);
	}

	return NDIS_STATUS_SUCCESS;
}


static int ForceWakeUpHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		AsicForceWakeup(pAd, true);

	return NDIS_STATUS_SUCCESS;
}


static int ForceSleepAutoWakeupHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	unsigned short  TbttNumToNextWakeUp;
	unsigned short  NextDtim = pAd->StaCfg.DtimPeriod;
	ULONG   Now;

	NdisGetSystemUpTime(&Now);
	NextDtim -= (unsigned short)(Now - pAd->StaCfg.LastBeaconRxTime)/pAd->CommonCfg.BeaconPeriod;

	TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
		TbttNumToNextWakeUp = NextDtim;

	RTMP_SET_PSM_BIT(pAd, PWR_SAVE);

	/* if WMM-APSD is failed, try to disable following line*/
	AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);

	return NDIS_STATUS_SUCCESS;
}


int QkeriodicExecutHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	StaQuickResponeForRateUpExec(NULL, pAd, NULL, NULL);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT*/


#ifdef CONFIG_AP_SUPPORT
static int APEnableTXBurstHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		u32 Ac0Cfg;
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_ENABLE_TX_BURST  \n"));

		Ac0Cfg = mt76u_reg_read(pAd, EDCA_AC0_CFG);
		Ac0Cfg &= ~MT_EDCA_CFG_TXOP;
		Ac0Cfg |= FIELD_PREP(MT_EDCA_CFG_TXOP, 0x20);
		mt76u_reg_write(pAd, EDCA_AC0_CFG, Ac0Cfg);
	}

	return NDIS_STATUS_SUCCESS;
}


static int APDisableTXBurstHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		u32 Ac0Cfg;
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_DISABLE_TX_BURST  \n"));

		Ac0Cfg = mt76u_reg_read(pAd, EDCA_AC0_CFG);
		Ac0Cfg &= ~MT_EDCA_CFG_TXOP;
		Ac0Cfg |= FIELD_PREP(MT_EDCA_CFG_TXOP, 0x0);
		mt76u_reg_write(pAd, EDCA_AC0_CFG, Ac0Cfg);
	}

	return NDIS_STATUS_SUCCESS;
}


static int APAdjustEXPAckTimeHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME  \n"));
		mt76u_reg_write(pAd, EXP_ACK_TIME, 0x005400ca);
	}

	return NDIS_STATUS_SUCCESS;
}


static int APRecoverEXPAckTimeHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME  \n"));
		mt76u_reg_write(pAd, EXP_ACK_TIME, 0x002400ca);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */





#ifdef CONFIG_AP_SUPPORT
static int ChannelRescanHdlr(IN struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	DBGPRINT(RT_DEBUG_TRACE, ("cmd> Re-scan channel! \n"));

	pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, true);
	/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
	N_ChannelCheck(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("cmd> Switch to %d! \n", pAd->CommonCfg.Channel));
	APStop(pAd);
	APStartUp(pAd);

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadAlarmResume(pAd);
#endif /* AP_QLOAD_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT*/


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
static int RegHintHdlr (struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static int RegHint11DHdlr(struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT11D(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static int RT_Mac80211_ScanEnd(struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_SCAN_END(pAd, false);
	return NDIS_STATUS_SUCCESS;
}

static int RT_Mac80211_ConnResultInfom(struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt)
{
#ifdef CONFIG_STA_SUPPORT
	RT_CFG80211_CONN_RESULT_INFORM(pAd, pAd->MlmeAux.Bssid,
				       pAd->StaCfg.ReqVarIEs, pAd->StaCfg.ReqVarIELen,
				       CMDQelmt->buffer, CMDQelmt->bufferlength,
				       true);
#endif /*CONFIG_STA_SUPPORT*/
	return NDIS_STATUS_SUCCESS;
}
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */






typedef int (*CMDHdlr)(struct rtmp_adapter *pAd, IN PCmdQElmt CMDQelmt);

static CMDHdlr CMDHdlrTable[] = {
	ResetBulkOutHdlr,				/* CMDTHREAD_RESET_BULK_OUT*/
	ResetBulkInHdlr,					/* CMDTHREAD_RESET_BULK_IN*/
	CheckGPIOHdlr,					/* CMDTHREAD_CHECK_GPIO	*/
	SetAsicWcidHdlr,					/* CMDTHREAD_SET_ASIC_WCID*/
	DelAsicWcidHdlr,					/* CMDTHREAD_DEL_ASIC_WCID*/
	SetClientMACEntryHdlr,			/* CMDTHREAD_SET_CLIENT_MAC_ENTRY*/

#ifdef CONFIG_STA_SUPPORT
	SetPSMBitHdlr,					/* CMDTHREAD_SET_PSM_BIT*/
	ForceWakeUpHdlr,				/* CMDTHREAD_FORCE_WAKE_UP*/
	ForceSleepAutoWakeupHdlr,		/* CMDTHREAD_FORCE_SLEEP_AUTO_WAKEUP*/
	QkeriodicExecutHdlr,				/* CMDTHREAD_QKERIODIC_EXECUT*/
#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	APUpdateCapabilityAndErpieHdlr,	/* CMDTHREAD_AP_UPDATE_CAPABILITY_AND_ERPIE*/
	APEnableTXBurstHdlr,			/* CMDTHREAD_AP_ENABLE_TX_BURST*/
	APDisableTXBurstHdlr,			/* CMDTHREAD_AP_DISABLE_TX_BURST*/
	APAdjustEXPAckTimeHdlr,		/* CMDTHREAD_AP_ADJUST_EXP_ACK_TIME*/
	APRecoverEXPAckTimeHdlr,		/* CMDTHREAD_AP_RECOVER_EXP_ACK_TIME*/
	ChannelRescanHdlr,				/* CMDTHREAD_CHAN_RESCAN*/
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* CONFIG_AP_SUPPORT */

    NULL,

	NULL,

	/* Security related */
	SetWcidSecInfoHdlr,				/* CMDTHREAD_SET_WCID_SEC_INFO*/
	SetAsicWcidIVEIVHdlr,			/* CMDTHREAD_SET_ASIC_WCID_IVEIV*/
	SetAsicWcidAttrHdlr,				/* CMDTHREAD_SET_ASIC_WCID_ATTR*/
	SETAsicSharedKeyHdlr,			/* CMDTHREAD_SET_ASIC_SHARED_KEY*/
	SetAsicPairwiseKeyHdlr,			/* CMDTHREAD_SET_ASIC_PAIRWISE_KEY*/
	RemovePairwiseKeyHdlr,			/* CMDTHREAD_REMOVE_PAIRWISE_KEY*/

#ifdef CONFIG_STA_SUPPORT
	SetPortSecuredHdlr,				/* CMDTHREAD_SET_PORT_SECURED*/
#else
	NULL,
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	_802_11_CounterMeasureHdlr,	/* CMDTHREAD_802_11_COUNTER_MEASURE*/
#else
	NULL,
#endif /* CONFIG_AP_SUPPORT */

	UpdateProtectHdlr,				/* CMDTHREAD_UPDATE_PROTECT*/


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RegHintHdlr,
	RegHint11DHdlr,
	RT_Mac80211_ScanEnd,
	RT_Mac80211_ConnResultInfom,
#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* RT_CFG80211_SUPPORT */

#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* LINUX */

	NULL,

	NULL,

	NULL,

	NULL,
};


static inline bool ValidCMD(IN PCmdQElmt CMDQelmt)
{
	short CMDIndex = CMDQelmt->command - CMDTHREAD_FIRST_CMD_ID;
	unsigned short CMDHdlrTableLength= sizeof(CMDHdlrTable) / sizeof(CMDHdlr);

	if ( (CMDIndex >= 0) && (CMDIndex < CMDHdlrTableLength)) {
		if (CMDHdlrTable[CMDIndex] > 0) {
			return true;
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("No corresponding CMDHdlr for this CMD(%x)\n",  CMDQelmt->command));
			return false;
		}
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("CMD(%x) is out of boundary\n", CMDQelmt->command));
		return false;
	}
}


VOID CMDHandler(struct rtmp_adapter *pAd)
{
	PCmdQElmt		cmdqelmt;
	int 	NdisStatus = NDIS_STATUS_SUCCESS;
	int 	ntStatus;
/*	unsigned long	IrqFlags;*/

	while (pAd && pAd->CmdQ.size > 0) {
		NdisStatus = NDIS_STATUS_SUCCESS;

		spin_lock_bh(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		spin_unlock_bh(&pAd->CmdQLock);

		if (cmdqelmt == NULL)
			break;


		if(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
		    RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			if(ValidCMD(cmdqelmt))
				ntStatus = (*CMDHdlrTable[cmdqelmt->command - CMDTHREAD_FIRST_CMD_ID])(pAd, cmdqelmt);
		}

		if (cmdqelmt->CmdFromNdis == true) {
			if (cmdqelmt->buffer != NULL)
				kfree(cmdqelmt->buffer);
			kfree(cmdqelmt);
		} else {
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				kfree(cmdqelmt->buffer);
			kfree(cmdqelmt);
		}
	}	/* end of while */
}


VOID RTUSBWatchDog(struct rtmp_adapter *pAd)
{
	PHT_TX_CONTEXT pHTTXContext;
	int idx;
	ULONG irqFlags;
	struct urb *pUrb;
	bool needDumpSeq = false;
	uint32_t MACValue;

	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

#ifdef CONFIG_STA_SUPPORT
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;
#endif /* CONFIG_STA_SUPPORT */

	return;
}


