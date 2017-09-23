/*
   All functions in this file must be USB-depended, or you should out your function
	in other files.

*/


#include	"rt_config.h"

/* Map USB endpoint number to Q id in the DMA engine */
static enum mt76_qsel ep2dmaq(u8 ep)
{
	if (ep == 5)
		return MT_QSEL_MGMT;
	return MT_QSEL_EDCA;
}

/*
	========================================================================

	Routine	Description:
		This subroutine will scan through releative ring descriptor to find
		out avaliable free ring descriptor and compare with request size.

	Arguments:
		pAd	Pointer	to our adapter
		RingType	Selected Ring

	Return Value:
		NDIS_STATUS_FAILURE		Not enough free descriptor
		NDIS_STATUS_SUCCESS		Enough free descriptor

	Note:

	========================================================================
*/
int RTUSBFreeDescRequest(
	IN struct rtmp_adapter *pAd,
	IN u8 BulkOutPipeId,
	IN uint32_t req_cnt)
{
	int  Status = NDIS_STATUS_FAILURE;
	unsigned long IrqFlags;
	HT_TX_CONTEXT *pHTTXContext;


	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
	spin_lock_bh(&pAd->TxContextQueueLock[BulkOutPipeId]);
	if ((pHTTXContext->CurWritePosition < pHTTXContext->NextBulkOutPosition) && ((pHTTXContext->CurWritePosition + req_cnt + LOCAL_TXBUF_SIZE) > pHTTXContext->NextBulkOutPosition))
	{

		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
	}
	else if ((pHTTXContext->CurWritePosition == 8) && (pHTTXContext->NextBulkOutPosition < (req_cnt + LOCAL_TXBUF_SIZE)))
	{
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
	}
	else if (pHTTXContext->bCurWriting == true)
	{
		DBGPRINT(RT_DEBUG_TRACE,("RTUSBFreeD c3 --> QueIdx=%d, CWPos=%ld, NBOutPos=%ld!\n", BulkOutPipeId, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition));
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
	}
	else
	{
		Status = NDIS_STATUS_SUCCESS;
	}
	spin_unlock_bh(&pAd->TxContextQueueLock[BulkOutPipeId]);


	return Status;
}


bool	RTUSBNeedQueueBackForAgg(struct rtmp_adapter *pAd, u8 BulkOutPipeId)
{
	HT_TX_CONTEXT *pHTTXContext;
	bool needQueBack = false;
	unsigned long   IrqFlags;


	pHTTXContext = &pAd->TxContext[BulkOutPipeId];

	spin_lock_bh(&pAd->TxContextQueueLock[BulkOutPipeId]);
	if ((pHTTXContext->IRPPending == true)  /*&& (pAd->TxSwQueue[BulkOutPipeId].Number == 0) */)
	{
		if ((pHTTXContext->CurWritePosition < pHTTXContext->ENextBulkOutPosition) &&
			(((pHTTXContext->ENextBulkOutPosition+MAX_AGGREGATION_SIZE) < MAX_TXBULK_LIMIT) || (pHTTXContext->CurWritePosition > MAX_AGGREGATION_SIZE)))
		{
			needQueBack = true;
		}
		else if ((pHTTXContext->CurWritePosition > pHTTXContext->ENextBulkOutPosition) &&
				 ((pHTTXContext->ENextBulkOutPosition + MAX_AGGREGATION_SIZE) < pHTTXContext->CurWritePosition))
		{
			needQueBack = true;
		}
	}
	spin_unlock_bh(&pAd->TxContextQueueLock[BulkOutPipeId]);

	return needQueBack;

}


/*
	========================================================================

	Routine	Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pTxD		Pointer to transmit descriptor
		Ack			Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs			Setting for IFS gap
		Rate		Setting for transmit rate
		Service		Setting for service
		Length		Frame length
		TxPreamble  Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	========================================================================
*/
static void rlt_usb_write_txinfo(TXINFO_STRUC *pTxInfo,
	unsigned short USBDMApktLen, bool bWiv, u8 QueueSel)
{
	struct txinfo_nmac_pkt *nmac_info = (struct txinfo_nmac_pkt *)pTxInfo;

	nmac_info->pkt_80211 = 1;
	nmac_info->info_type = 0;
	nmac_info->d_port = 0;
	nmac_info->cso = 0;
	nmac_info->tso = 0;

	nmac_info->pkt_len = USBDMApktLen;
	nmac_info->QSEL = QueueSel;
	if (QueueSel != MT_QSEL_EDCA)
		DBGPRINT(RT_DEBUG_TRACE, ("====> QueueSel != FIFO_EDCA <====\n"));
	nmac_info->next_vld = false;	/*NextValid;   Need to check with Jan about this.*/
	nmac_info->tx_burst = false;
	nmac_info->wiv = bWiv;
	nmac_info->sw_lst_rnd = 0;		/* TxInfoSwLstRnd */
}


static VOID rlt_usb_update_txinfo(
	IN struct rtmp_adapter *pAd,
	IN TXINFO_STRUC *pTxInfo,
	IN TX_BLK *pTxBlk)
{
}


#ifdef CONFIG_STA_SUPPORT
VOID ComposePsPoll(struct rtmp_adapter *pAd)
{
	TXINFO_STRUC *pTxInfo;
	TXWI_STRUC *pTxWI;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	u8 *buf;
	unsigned short data_len;


	DBGPRINT(RT_DEBUG_TRACE, ("ComposePsPoll\n"));
	memset(&pAd->PsPollFrame, 0, sizeof (PSPOLL_FRAME));

	pAd->PsPollFrame.FC.PwrMgmt = 0;
	pAd->PsPollFrame.FC.Type = FC_TYPE_CNTL;
	pAd->PsPollFrame.FC.SubType = SUBTYPE_PS_POLL;
	pAd->PsPollFrame.Aid = pAd->StaActive.Aid | 0xC000;
	COPY_MAC_ADDR(pAd->PsPollFrame.Bssid, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->PsPollFrame.Ta, pAd->CurrentAddress);

	buf = &pAd->PsPollContext.TransferBuffer->field.WirelessPacket[0];
	pTxInfo = (TXINFO_STRUC *)buf;
	pTxWI = (TXWI_STRUC *)&buf[TXINFO_SIZE];
	RTMPZeroMemory(buf, 100);
	data_len = sizeof (PSPOLL_FRAME);
	rlt_usb_write_txinfo(pTxInfo, data_len + TXWISize + TSO_SIZE, true,
						ep2dmaq(MGMTPIPEIDX));
	RTMPWriteTxWI(pAd, pTxWI, false, false, false, false, true, false, 0,
					BSSID_WCID, data_len, 0, 0,
					(u8) pAd->CommonCfg.MlmeTransmit.field.MCS,
					IFS_BACKOFF, &pAd->CommonCfg.MlmeTransmit);
	RTMPMoveMemory((VOID *)&buf[TXWISize + TXINFO_SIZE + TSO_SIZE], (VOID *)&pAd->PsPollFrame, data_len);
	/* Append 4 extra zero bytes. */
	pAd->PsPollContext.BulkOutSize = TXINFO_SIZE + TXWISize + TSO_SIZE + data_len + 4;
}
#endif /* CONFIG_STA_SUPPORT */


/* IRQL = DISPATCH_LEVEL */
VOID ComposeNullFrame(struct rtmp_adapter *pAd)
{
	TXINFO_STRUC *pTxInfo;
	TXWI_STRUC *pTxWI;
	u8 *buf;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	unsigned short data_len = sizeof(pAd->NullFrame);;


	memset(&pAd->NullFrame, 0, data_len);
	pAd->NullFrame.FC.Type = FC_TYPE_DATA;
	pAd->NullFrame.FC.SubType = SUBTYPE_DATA_NULL;
	pAd->NullFrame.FC.ToDs = 1;
	COPY_MAC_ADDR(pAd->NullFrame.Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->NullFrame.Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pAd->NullFrame.Addr3, pAd->CommonCfg.Bssid);
	buf = &pAd->NullContext.TransferBuffer->field.WirelessPacket[0];
	RTMPZeroMemory(buf, 100);
	pTxInfo = (TXINFO_STRUC *)buf;
	pTxWI = (TXWI_STRUC *)&buf[TXINFO_SIZE];
	rlt_usb_write_txinfo(pTxInfo,
			(unsigned short)(data_len + TXWISize + TSO_SIZE), true,
			ep2dmaq(MGMTPIPEIDX));
	RTMPWriteTxWI(pAd, pTxWI, false, false, false, false, true, false, 0,
					BSSID_WCID, data_len, 0, 0,
					(u8)pAd->CommonCfg.MlmeTransmit.field.MCS,
					IFS_BACKOFF, &pAd->CommonCfg.MlmeTransmit);
	RTMPMoveMemory((VOID *)&buf[TXWISize + TXINFO_SIZE], (VOID *)&pAd->NullFrame, data_len);
	pAd->NullContext.BulkOutSize = TXINFO_SIZE + TXWISize + TSO_SIZE + data_len + 4;
}


/*
	We can do copy the frame into pTxContext when match following conditions.
		=>
		=>
		=>
*/
static inline int RtmpUSBCanDoWrite(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN HT_TX_CONTEXT *pHTTXContext)
{
	int canWrite = NDIS_STATUS_RESOURCES;

	if (((pHTTXContext->CurWritePosition) < pHTTXContext->NextBulkOutPosition) && (pHTTXContext->CurWritePosition + LOCAL_TXBUF_SIZE) > pHTTXContext->NextBulkOutPosition)
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c1!\n"));
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	}
	else if ((pHTTXContext->CurWritePosition == 8) && (pHTTXContext->NextBulkOutPosition < LOCAL_TXBUF_SIZE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c2!\n"));
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	}
	else if (pHTTXContext->bCurWriting == true)
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c3!\n"));
	}
	else if ((pHTTXContext->ENextBulkOutPosition == 8)  && ((pHTTXContext->CurWritePosition + 7912 ) > MAX_TXBULK_LIMIT)  )
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c4!\n"));
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	}
	else
	{
		canWrite = NDIS_STATUS_SUCCESS;
	}


	return canWrite;
}


unsigned short RtmpUSB_WriteSubTxResource(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN bool bIsLast,
	OUT	unsigned short *freeCnt)
{

	/* Dummy function. Should be removed in the future.*/
	return 0;

}

unsigned short RtmpUSB_WriteFragTxResource(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN u8 fragNum,
	OUT	unsigned short *freeCnt)
{
	HT_TX_CONTEXT	*pHTTXContext;
	unsigned short 		hwHdrLen;	/* The hwHdrLen consist of 802.11 header length plus the header padding length.*/
	uint32_t 		fillOffset;
	TXINFO_STRUC	*pTxInfo;
	TXWI_STRUC		*pTxWI;
	u8 *		pWirelessPacket = NULL;
	u8 		QueIdx;
	int 	Status;
	unsigned long	IrqFlags;
	uint32_t 		USBDMApktLen = 0, DMAHdrLen, padding;
	bool			TxQLastRound = false;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* get Tx Ring Resource & Dma Buffer address*/

	QueIdx = pTxBlk->QueIdx;
	pHTTXContext  = &pAd->TxContext[QueIdx];

	spin_lock_bh(&pAd->TxContextQueueLock[QueIdx]);

	pHTTXContext  = &pAd->TxContext[QueIdx];
	fillOffset = pHTTXContext->CurWritePosition;

	if(fragNum == 0)
	{
		/* Check if we have enough space for this bulk-out batch.*/
		Status = RtmpUSBCanDoWrite(pAd, QueIdx, pHTTXContext);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			pHTTXContext->bCurWriting = true;

			/* Reserve space for 8 bytes padding.*/
			if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition))
			{
				pHTTXContext->ENextBulkOutPosition += 8;
				pHTTXContext->CurWritePosition += 8;
				fillOffset += 8;
			}
			pTxBlk->Priv = 0;
			pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;
		}
		else
		{
			spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

			dev_kfree_skb_any(pTxBlk->pPacket);
			return(Status);
		}
	}
	else
	{
		/* For sub-sequent frames of this bulk-out batch. Just copy it to our bulk-out buffer.*/
		Status = ((pHTTXContext->bCurWriting == true) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			fillOffset += pTxBlk->Priv;
		}
		else
		{
			spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

			dev_kfree_skb_any(pTxBlk->pPacket);
			return(Status);
		}
	}

	memset((u8 *)(&pTxBlk->HeaderBuf[0]), 0, TXINFO_SIZE);
	pTxInfo = (TXINFO_STRUC *)(&pTxBlk->HeaderBuf[0]);
	pTxWI= (TXWI_STRUC *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);

	pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

	/* copy TXWI + WLAN Header + LLC into DMA Header Buffer*/
	/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
	hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

	/* Build our URB for USBD*/
	DMAHdrLen = TXWISize + hwHdrLen;
	USBDMApktLen = DMAHdrLen + pTxBlk->SrcBufLen;
	padding = (4 - (USBDMApktLen % 4)) & 0x03;	/* round up to 4 byte alignment*/
	USBDMApktLen += padding;

	pTxBlk->Priv += (TXINFO_SIZE + USBDMApktLen);

	/* For TxInfo, the length of USBDMApktLen = TXWI_SIZE + 802.11 header + payload*/
	rlt_usb_write_txinfo(pTxInfo, (unsigned short)(USBDMApktLen), false, MT_QSEL_EDCA);

	if (fragNum == pTxBlk->TotalFragNum)
	{
		pTxInfo->txinfo_nmac_pkt.tx_burst = 0;

		if ((pHTTXContext->CurWritePosition + pTxBlk->Priv + 3906)> MAX_TXBULK_LIMIT)
		{
			pTxInfo->txinfo_nmac_pkt.sw_lst_rnd = 1;
			TxQLastRound = true;
		}
	}
	else
	{
		pTxInfo->txinfo_nmac_pkt.tx_burst = 1;
	}

	memmove(pWirelessPacket, pTxBlk->HeaderBuf, TXINFO_SIZE + TXWISize + hwHdrLen);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (u8 *)(pWirelessPacket + TXINFO_SIZE + TXWISize), DIR_WRITE, false);
#endif /* RT_BIG_ENDIAN */
	pWirelessPacket += (TXINFO_SIZE + TXWISize + hwHdrLen);
	pHTTXContext->CurWriteRealPos += (TXINFO_SIZE + TXWISize + hwHdrLen);

	spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

	memmove(pWirelessPacket, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);

	/*	Zero the last padding.*/
	pWirelessPacket += pTxBlk->SrcBufLen;
	memset(pWirelessPacket, 0, padding + 8);

	if (fragNum == pTxBlk->TotalFragNum)
	{
		spin_lock_bh(&pAd->TxContextQueueLock[QueIdx]);

		/* Update the pHTTXContext->CurWritePosition. 3906 used to prevent the NextBulkOut is a A-RALINK/A-MSDU Frame.*/
		pHTTXContext->CurWritePosition += pTxBlk->Priv;
		if (TxQLastRound == true)
			pHTTXContext->CurWritePosition = 8;

		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;


		/* Finally, set bCurWriting as false*/
	pHTTXContext->bCurWriting = false;

		spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

		/* succeed and release the skb buffer*/
		dev_kfree_skb_any(pTxBlk->pPacket);
	}


	return(Status);

}


unsigned short RtmpUSB_WriteSingleTxResource(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN bool bIsLast,
	OUT	unsigned short *freeCnt)
{
	HT_TX_CONTEXT *pHTTXContext;
	uint32_t fillOffset;
	TXINFO_STRUC *pTxInfo;
	TXWI_STRUC *pTxWI;
	u8 *pWirelessPacket, *buf;
	u8 QueIdx;
	unsigned long	IrqFlags;
	int Status;
	uint32_t hdr_copy_len, hdr_len, dma_len = 0, padding;
	bool bTxQLastRound = false;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* get Tx Ring Resource & Dma Buffer address*/
	QueIdx = pTxBlk->QueIdx;

	spin_lock_bh(&pAd->TxContextQueueLock[QueIdx]);
	pHTTXContext  = &pAd->TxContext[QueIdx];
	fillOffset = pHTTXContext->CurWritePosition;



	/* Check ring full */
	Status = RtmpUSBCanDoWrite(pAd, QueIdx, pHTTXContext);
	if(Status == NDIS_STATUS_SUCCESS)
	{
		pHTTXContext->bCurWriting = true;
		buf = &pTxBlk->HeaderBuf[0];
		pTxInfo = (TXINFO_STRUC *)buf;
		pTxWI= (TXWI_STRUC *)&buf[TXINFO_SIZE];

		/* Reserve space for 8 bytes padding.*/
		if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition))
		{
			pHTTXContext->ENextBulkOutPosition += 8;
			pHTTXContext->CurWritePosition += 8;
			fillOffset += 8;
		}
		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;

		pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

		/* Build our URB for USBD */
		hdr_len = TXWISize + TSO_SIZE + pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;
		hdr_copy_len = TXINFO_SIZE + hdr_len;
		dma_len = hdr_len + pTxBlk->SrcBufLen;
		padding = (4 - (dma_len % 4)) & 0x03;	/* round up to 4 byte alignment*/
		dma_len += padding;

		pTxBlk->Priv = (TXINFO_SIZE + dma_len);

		/* For TxInfo, the length of USBDMApktLen = TXWI_SIZE + TSO_SIZE + 802.11 header + payload */
		rlt_usb_write_txinfo(pTxInfo, (unsigned short)(dma_len), false, MT_QSEL_EDCA);


		if ((pHTTXContext->CurWritePosition + 3906 + pTxBlk->Priv) > MAX_TXBULK_LIMIT)
		{
			pTxInfo->txinfo_nmac_pkt.sw_lst_rnd = 1;
			bTxQLastRound = true;
		}

		memmove(pWirelessPacket, pTxBlk->HeaderBuf, hdr_copy_len);
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (u8 *)(pWirelessPacket + TXINFO_SIZE + TXWISize + TSO_SIZE), DIR_WRITE, false);
#endif /* RT_BIG_ENDIAN */
		pWirelessPacket += (hdr_copy_len);

		/* We unlock it here to prevent the first 8 bytes maybe over-writed issue.*/
		/*	1. First we got CurWritePosition but the first 8 bytes still not write to the pTxcontext.*/
		/*	2. An interrupt break our routine and handle bulk-out complete.*/
		/*	3. In the bulk-out compllete, it need to do another bulk-out, */
		/*			if the ENextBulkOutPosition is just the same as CurWritePosition, it will save the first 8 bytes from CurWritePosition,*/
		/*			but the payload still not copyed. the pTxContext->SavedPad[] will save as allzero. and set the bCopyPad = true.*/
		/*	4. Interrupt complete.*/
		/*  5. Our interrupted routine go back and fill the first 8 bytes to pTxContext.*/
		/*	6. Next time when do bulk-out, it found the bCopyPad==true and will copy the SavedPad[] to pTxContext->NextBulkOutPosition.*/
		/*		and the packet will wrong.*/
		pHTTXContext->CurWriteRealPos += hdr_copy_len;
		spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

#ifdef TX_PKT_SG
		if (pTxBlk->pkt_info.BufferCount > 1) {
			INT i, len;
			void *data;
			PKT_SG_T *sg = &pTxBlk->pkt_info.sg_list[0];

			for (i = 0 ; i < pTxBlk->pkt_info.BufferCount; i++) {
				data = sg[i].data;
				len = sg[i].len;
				if (i == 0) {
					len -= ((ULONG)pTxBlk->pSrcBufData - (ULONG)sg[i].data);
					data = pTxBlk->pSrcBufData;
				}
				//DBGPRINT(RT_DEBUG_TRACE, ("%s:sg[%d]=0x%x, len=%d\n", __FUNCTION__, i, data, len));
				if (len <= 0) {
					DBGPRINT(RT_DEBUG_ERROR, ("%s():sg[%d] info error, sg.data=0x%x, sg.len=%d, pTxBlk->pSrcBufData=0x%x, pTxBlk->SrcBufLen=%d, data=0x%x, len=%d\n",
								__FUNCTION__, i, sg[i].data, sg[i].len, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, data, len));
					break;
				}
				memmove(pWirelessPacket, data, len);
				pWirelessPacket += len;
			}
		}
		else
#endif /* TX_PKT_SG */
		{
			memmove(pWirelessPacket, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
			pWirelessPacket += pTxBlk->SrcBufLen;
		}

		memset(pWirelessPacket, 0, padding + 8);
		spin_lock_bh(&pAd->TxContextQueueLock[QueIdx]);

		pHTTXContext->CurWritePosition += pTxBlk->Priv;
		if (bTxQLastRound)
			pHTTXContext->CurWritePosition = 8;

		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;
		pHTTXContext->bCurWriting = false;
	}


	spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);


	/* succeed and release the skb buffer*/
	dev_kfree_skb_any(pTxBlk->pPacket);

	return(Status);

}


unsigned short RtmpUSB_WriteMultiTxResource(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN u8 frmNum,
	OUT unsigned short *freeCnt)
{
	HT_TX_CONTEXT *pHTTXContext;
	unsigned short hwHdrLen;	/* The hwHdrLen consist of 802.11 header length plus the header padding length.*/
	uint32_t fillOffset;
	TXINFO_STRUC *pTxInfo;
	TXWI_STRUC *pTxWI;
	u8 *pWirelessPacket = NULL;
	u8 QueIdx;
	int Status;
	unsigned long IrqFlags;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* get Tx Ring Resource & Dma Buffer address*/
	QueIdx = pTxBlk->QueIdx;
	pHTTXContext  = &pAd->TxContext[QueIdx];

	spin_lock_bh(&pAd->TxContextQueueLock[QueIdx]);

	if(frmNum == 0)
	{
		/* Check if we have enough space for this bulk-out batch.*/
		Status = RtmpUSBCanDoWrite(pAd, QueIdx, pHTTXContext);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			pHTTXContext->bCurWriting = true;

			pTxInfo = (TXINFO_STRUC *)(&pTxBlk->HeaderBuf[0]);
			pTxWI= (TXWI_STRUC *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);

			/* Reserve space for 8 bytes padding.*/
			if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition))
			{

				pHTTXContext->CurWritePosition += 8;
				pHTTXContext->ENextBulkOutPosition += 8;
			}
			fillOffset = pHTTXContext->CurWritePosition;
			pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;

			pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];


			/* Copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/

			if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
				/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-AMSDU_SUBHEAD_LEN, 4)+AMSDU_SUBHEAD_LEN;*/
				hwHdrLen = pTxBlk->MpduHeaderLen-AMSDU_SUBHEAD_LEN + pTxBlk->HdrPadLen + AMSDU_SUBHEAD_LEN;
			else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
				/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-ARALINK_HEADER_LEN, 4)+ARALINK_HEADER_LEN;*/
				hwHdrLen = pTxBlk->MpduHeaderLen-ARALINK_HEADER_LEN + pTxBlk->HdrPadLen + ARALINK_HEADER_LEN;
			else
				/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
				hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

			/* Update the pTxBlk->Priv.*/
			pTxBlk->Priv = TXINFO_SIZE + TXWISize + hwHdrLen;

			/*	pTxInfo->USBDMApktLen now just a temp value and will to correct latter.*/
			rlt_usb_write_txinfo(pTxInfo, (unsigned short)(pTxBlk->Priv), false, MT_QSEL_EDCA);

			/* Copy it.*/
			memmove(pWirelessPacket, pTxBlk->HeaderBuf, pTxBlk->Priv);
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (u8 *)(pWirelessPacket+ TXINFO_SIZE + TXWISize), DIR_WRITE, false);
#endif /* RT_BIG_ENDIAN */
			pHTTXContext->CurWriteRealPos += pTxBlk->Priv;
			pWirelessPacket += pTxBlk->Priv;
		}
	}
	else
	{	/* For sub-sequent frames of this bulk-out batch. Just copy it to our bulk-out buffer.*/

		Status = ((pHTTXContext->bCurWriting == true) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			fillOffset =  (pHTTXContext->CurWritePosition + pTxBlk->Priv);
			pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

			/*hwHdrLen = pTxBlk->MpduHeaderLen;*/
			memmove(pWirelessPacket, pTxBlk->HeaderBuf, pTxBlk->MpduHeaderLen);
			pWirelessPacket += (pTxBlk->MpduHeaderLen);
			pTxBlk->Priv += pTxBlk->MpduHeaderLen;
		}
		else
		{	/* It should not happened now unless we are going to shutdown.*/
			DBGPRINT(RT_DEBUG_ERROR, ("WriteMultiTxResource():bCurWriting is false when handle sub-sequent frames.\n"));
			Status = NDIS_STATUS_FAILURE;
		}
	}


	/*
		We unlock it here to prevent the first 8 bytes maybe over-write issue.
		1. First we got CurWritePosition but the first 8 bytes still not write to the pTxContext.
		2. An interrupt break our routine and handle bulk-out complete.
		3. In the bulk-out compllete, it need to do another bulk-out,
			if the ENextBulkOutPosition is just the same as CurWritePosition, it will save the first 8 bytes from CurWritePosition,
			but the payload still not copyed. the pTxContext->SavedPad[] will save as allzero. and set the bCopyPad = true.
		4. Interrupt complete.
		5. Our interrupted routine go back and fill the first 8 bytes to pTxContext.
		6. Next time when do bulk-out, it found the bCopyPad==true and will copy the SavedPad[] to pTxContext->NextBulkOutPosition.
			and the packet will wrong.
	*/
	spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("WriteMultiTxResource: CWPos = %ld, NBOutPos = %ld.\n", pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition));
		goto done;
	}

	/* Copy the frame content into DMA buffer and update the pTxBlk->Priv*/
	memmove(pWirelessPacket, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	pWirelessPacket += pTxBlk->SrcBufLen;
	pTxBlk->Priv += pTxBlk->SrcBufLen;

done:
	/* Release the skb buffer here*/
	dev_kfree_skb_any(pTxBlk->pPacket);

	return(Status);

}


VOID RtmpUSB_FinalWriteTxResource(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN unsigned short totalMPDUSize,
	IN unsigned short TxIdx)
{
	u8 		QueIdx;
	HT_TX_CONTEXT	*pHTTXContext;
	uint32_t 		fillOffset;
	TXINFO_STRUC	*pTxInfo;
	TXWI_STRUC		*pTxWI;
	uint32_t 		USBDMApktLen, padding;
	unsigned long	IrqFlags;
	u8 *		pWirelessPacket;

	QueIdx = pTxBlk->QueIdx;
	pHTTXContext  = &pAd->TxContext[QueIdx];

	spin_lock_bh(&pAd->TxContextQueueLock[QueIdx]);

	if (pHTTXContext->bCurWriting == true)
	{
		fillOffset = pHTTXContext->CurWritePosition;
		if (((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition) || ((pHTTXContext->ENextBulkOutPosition-8) == pHTTXContext->CurWritePosition))
			&& (pHTTXContext->bCopySavePad == true))
			pWirelessPacket = (u8 *)(&pHTTXContext->SavedPad[0]);
		else
			pWirelessPacket = (u8 *)(&pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset]);


		/* Update TxInfo->USBDMApktLen , */
		/*		the length = TXWI_SIZE + 802.11_hdr + 802.11_hdr_pad + payload_of_all_batch_frames + Bulk-Out-padding*/

		pTxInfo = (TXINFO_STRUC *)(pWirelessPacket);

		/* Calculate the bulk-out padding*/
		USBDMApktLen = pTxBlk->Priv - TXINFO_SIZE;
		padding = (4 - (USBDMApktLen % 4)) & 0x03;	/* round up to 4 byte alignment*/
		USBDMApktLen += padding;

		pTxInfo->txinfo_nmac_pkt.pkt_len = USBDMApktLen;


		/*
			Update TXWI->TxWIMPDUByteCnt,
				the length = 802.11 header + payload_of_all_batch_frames
		*/
		pTxWI= (TXWI_STRUC *)(pWirelessPacket + TXINFO_SIZE);

		pTxWI->TXWI_N.MPDUtotalByteCnt = totalMPDUSize;

		/* Update the pHTTXContext->CurWritePosition*/

		pHTTXContext->CurWritePosition += (TXINFO_SIZE + USBDMApktLen);
		if ((pHTTXContext->CurWritePosition + 3906)> MAX_TXBULK_LIMIT)
		{	/* Add 3906 for prevent the NextBulkOut packet size is a A-RALINK/A-MSDU Frame.*/
			pHTTXContext->CurWritePosition = 8;
			pTxInfo->txinfo_nmac_pkt.sw_lst_rnd = 1;
		}

		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;



		/*	Zero the last padding.*/
		pWirelessPacket = (&pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset + pTxBlk->Priv]);
		memset(pWirelessPacket, 0, padding + 8);

		/* Finally, set bCurWriting as false*/
		pHTTXContext->bCurWriting = false;

	}
	else
	{	/* It should not happened now unless we are going to shutdown.*/
		DBGPRINT(RT_DEBUG_ERROR, ("FinalWriteTxResource():bCurWriting is false when handle last frames.\n"));
	}

	spin_unlock_bh(&pAd->TxContextQueueLock[QueIdx]);

}


VOID RtmpUSBDataLastTxIdx(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN unsigned short TxIdx)
{
	/* DO nothing for USB.*/
}


/*
	When can do bulk-out:
		1. TxSwFreeIdx < TX_RING_SIZE;
			It means has at least one Ring entity is ready for bulk-out, kick it out.
		2. If TxSwFreeIdx == TX_RING_SIZE
			Check if the CurWriting flag is false, if it's false, we can do kick out.

*/
VOID RtmpUSBDataKickOut(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk,
	IN u8 QueIdx)
{
	RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	RTUSBKickBulkOut(pAd);

}


/*
	Must be run in Interrupt context
	This function handle RT2870 specific TxDesc and cpu index update and kick the packet out.
 */
int RtmpUSBMgmtKickOut(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN struct sk_buff *pPacket,
	IN u8 *pSrcBufVA,
	IN UINT SrcBufLen)
{
	TXINFO_STRUC *pTxInfo;
	ULONG BulkOutSize;
	u8 padLen;
	u8 *pDest;
	ULONG SwIdx = pAd->MgmtRing.TxCpuIdx;
	TX_CONTEXT *pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[SwIdx].AllocVa;
	ULONG IrqFlags;


	pTxInfo = (TXINFO_STRUC *)(pSrcBufVA);

	/* Build our URB for USBD*/
	BulkOutSize = (SrcBufLen + 3) & (~3);
	rlt_usb_write_txinfo(pTxInfo, (unsigned short)(BulkOutSize - TXINFO_SIZE), true, ep2dmaq(MGMTPIPEIDX));

	BulkOutSize += 4; /* Always add 4 extra bytes at every packet.*/

//+++Add by shiang for debug
//---Add by shiang for debug

/* WY , it cause Tx hang on Amazon_SE , Max said the padding is useless*/
	/* If BulkOutSize is multiple of BulkOutMaxPacketSize, add extra 4 bytes again.*/
/*	if ((BulkOutSize % pAd->BulkOutMaxPacketSize) == 0)*/
/*		BulkOutSize += 4;*/

	padLen = BulkOutSize - SrcBufLen;
	ASSERT((padLen <= RTMP_PKT_TAIL_PADDING));

	/* Now memzero all extra padding bytes.*/
	pDest = (u8 *)(pSrcBufVA + SrcBufLen);
	skb_put(pPacket, padLen);
	memset(pDest, 0, padLen);

	spin_lock_bh(&pAd->MLMEBulkOutLock);

	pAd->MgmtRing.Cell[pAd->MgmtRing.TxCpuIdx].pNdisPacket = pPacket;
	pMLMEContext->TransferBuffer = (PTX_BUFFER) pPacket->data;

	/* Length in TxInfo should be 8 less than bulkout size.*/
	pMLMEContext->BulkOutSize = BulkOutSize;
	pMLMEContext->InUse = true;
	pMLMEContext->bWaitingBulkOut = true;


	/*hex_dump("RtmpUSBMgmtKickOut", &pMLMEContext->TransferBuffer->field.WirelessPacket[0], (pMLMEContext->BulkOutSize > 16 ? 16 : pMLMEContext->BulkOutSize));*/

/*
	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	if (pAd->MgmtRing.TxSwFreeIdx == MGMT_RING_SIZE)
		needKickOut = true;
*/

	/* Decrease the TxSwFreeIdx and Increase the TX_CTX_IDX*/
	pAd->MgmtRing.TxSwFreeIdx--;
	INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);

	spin_unlock_bh(&pAd->MLMEBulkOutLock);

	RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);
	/*if (needKickOut)*/
	RTUSBKickBulkOut(pAd);

	return 0;
}


VOID RtmpUSBNullFrameKickOut(
	IN struct rtmp_adapter *pAd,
	IN u8 QueIdx,
	IN u8 *pNullFrame,
	IN uint32_t frameLen)
{
	if (pAd->NullContext.InUse == false)
	{
		PTX_CONTEXT pNullContext;
		TXINFO_STRUC *pTxInfo;
		TXWI_STRUC *pTxWI;
		u8 *pWirelessPkt;
		UINT8 TXWISize = pAd->chipCap.TXWISize;

		pNullContext = &(pAd->NullContext);

		/* Set the in use bit*/
		pNullContext->InUse = true;
		pWirelessPkt = (u8 *)&pNullContext->TransferBuffer->field.WirelessPacket[0];

		RTMPZeroMemory(&pWirelessPkt[0], 100);
		pTxInfo = (TXINFO_STRUC *)&pWirelessPkt[0];
		rlt_usb_write_txinfo(pTxInfo, (unsigned short)(frameLen + TXWISize + TSO_SIZE), true, ep2dmaq(MGMTPIPEIDX));
		pTxWI = (TXWI_STRUC *)&pWirelessPkt[TXINFO_SIZE];
		RTMPWriteTxWI(pAd, pTxWI, false, false, false, false, true, false, 0, BSSID_WCID, frameLen,
						0, 0, (u8)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_HTTXOP, &pAd->CommonCfg.MlmeTransmit);
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, (u8 *)pTxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */
		RTMPMoveMemory(&pWirelessPkt[TXWISize + TXINFO_SIZE + TSO_SIZE], pNullFrame, frameLen);
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (u8 *)&pWirelessPkt[TXINFO_SIZE + TXWISize + TSO_SIZE], DIR_WRITE, false);
#endif /* RT_BIG_ENDIAN */
		pAd->NullContext.BulkOutSize =  TXINFO_SIZE + TXWISize + TSO_SIZE + frameLen + 4;
		pAd->NullContext.BulkOutSize = ( pAd->NullContext.BulkOutSize + 3) & (~3);

		DBGPRINT(RT_DEBUG_TRACE, ("%s - Send NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[pAd->CommonCfg.TxRate]));
		RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ;

		/* Kick bulk out */
		RTUSBKickBulkOut(pAd);
	}

}


/*
========================================================================
Routine Description:
    Get a received packet.

Arguments:
	pAd					device control block
	pSaveRxD			receive descriptor information
	*pbReschedule		need reschedule flag
	*pRxPending			pending received packet flag

Return Value:
    the recieved packet

Note:
========================================================================
*/
struct sk_buff *GetPacketFromRxRing(
	IN struct rtmp_adapter *pAd,
	OUT RX_BLK *pRxBlk,
	OUT bool *pbReschedule,
	INOUT uint32_t *pRxPending,
	u8 RxRingNo)
{
	RX_CONTEXT *pRxContext;
	struct sk_buff *skb;
	u8 *pData;
	ULONG ThisFrameLen, RxBufferLength, valid_len;
	struct mt7612u_rxwi *pRxWI;
	UINT8 RXWISize = pAd->chipCap.RXWISize;
	struct mt7612u_rxinfo *pRxInfo;
	struct mt7612u_rxfce_info_pkt *pRxFceInfo;

	pRxContext = &pAd->RxContext[pAd->NextRxBulkInReadIndex];
	if ((pRxContext->Readable == false) || (pRxContext->InUse == true))
		return NULL;

	RxBufferLength = pRxContext->BulkInOffset - pAd->ReadPosition;
	valid_len = RXDMA_FIELD_SIZE + RXWISize + sizeof(struct mt7612u_rxinfo);

	valid_len += sizeof(struct mt7612u_rxfce_info_pkt);

	if (RxBufferLength < valid_len)
		return NULL;

	pData = &pRxContext->TransferBuffer[pAd->ReadPosition];
//+++Add by shiang for debug
if (0) {
	hex_dump("GetPacketFromRxRing", pData, (RxBufferLength > 7000 ? 7000 : RxBufferLength));
}
//---Add by shiang for debug

	/* The RXDMA field is 4 bytes, now just use the first 2 bytes. The Length including the (RXWI + MSDU + Padding) */
	ThisFrameLen = *pData + (*(pData+1)<<8);
	if (ThisFrameLen == 0) {
		DBGPRINT(RT_DEBUG_TRACE, ("BIRIdx(%d): RXDMALen is zero.[%ld], BulkInBufLen = %ld)\n",
								pAd->NextRxBulkInReadIndex, ThisFrameLen, pRxContext->BulkInOffset));
		return NULL;
	}
	if ((ThisFrameLen & 0x3) != 0) 	{
		DBGPRINT(RT_DEBUG_ERROR, ("BIRIdx(%d): RXDMALen not multiple of 4.[%ld], BulkInBufLen = %ld)\n",
								pAd->NextRxBulkInReadIndex, ThisFrameLen, pRxContext->BulkInOffset));
		return NULL;
	}

	if ((ThisFrameLen + 8) > RxBufferLength) {	/* 8 for (RXDMA_FIELD_SIZE + sizeof(RXINFO_STRUC))*/
		DBGPRINT(RT_DEBUG_TRACE,("BIRIdx(%d):FrameLen(0x%lx) outranges. BulkInLen=0x%lx, remaining RxBufLen=0x%lx, ReadPos=0x%lx\n",
						pAd->NextRxBulkInReadIndex, ThisFrameLen, pRxContext->BulkInOffset, RxBufferLength, pAd->ReadPosition));

		/* error frame. finish this loop*/
		return NULL;
	}

	pData += RXDMA_FIELD_SIZE;

	{
		struct mt7612u_rxwi *rxwi_n;
		pRxInfo = (struct mt7612u_rxinfo *)pData;
		pRxFceInfo = (struct mt7612u_rxfce_info_pkt *)(pData + ThisFrameLen);
		pData += RXINFO_SIZE;
		pRxWI = (struct mt7612u_rxwi *)pData;
		rxwi_n = (struct mt7612u_rxwi *)pData;;
		pRxBlk->pRxWI = (struct mt7612u_rxwi *)pData;
		pRxBlk->MPDUtotalByteCnt = rxwi_n->MPDUtotalByteCnt;
		pRxBlk->wcid = rxwi_n->wcid;
		pRxBlk->key_idx = rxwi_n->key_idx;
		pRxBlk->bss_idx = rxwi_n->bss_idx;
		pRxBlk->TID = rxwi_n->tid;
		pRxBlk->DataSize = rxwi_n->MPDUtotalByteCnt;
		pRxBlk->rx_rate.field.MODE = rxwi_n->phy_mode;
		pRxBlk->rx_rate.field.MCS = rxwi_n->mcs;
		pRxBlk->rx_rate.field.ldpc = rxwi_n->ldpc;
		pRxBlk->rx_rate.field.BW = rxwi_n->bw;
		pRxBlk->rx_rate.field.STBC = rxwi_n->stbc;
		pRxBlk->rx_rate.field.ShortGI = rxwi_n->sgi;
		pRxBlk->rssi[0] = rxwi_n->rssi[0];
		pRxBlk->rssi[1] = rxwi_n->rssi[1];
		pRxBlk->rssi[2] = rxwi_n->rssi[2];
		pRxBlk->snr[0] = rxwi_n->bbp_rxinfo[0];
		pRxBlk->snr[1] = rxwi_n->bbp_rxinfo[1];
		pRxBlk->snr[2] = rxwi_n->bbp_rxinfo[2];
		pRxBlk->freq_offset = rxwi_n->bbp_rxinfo[4];
		pRxBlk->ldpc_ex_sym = rxwi_n->ldpc_ex_sym;
	}

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, pData, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */
	if (pRxBlk->MPDUtotalByteCnt > ThisFrameLen) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():pRxWIMPDUtotalByteCount(%d) large than RxDMALen(%ld)\n",
									__FUNCTION__, pRxBlk->MPDUtotalByteCnt, ThisFrameLen));
		return NULL;

	}
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, pData, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */

	/* allocate a rx packet*/
	skb = dev_alloc_skb(ThisFrameLen);
	if (skb == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,("%s():Cannot Allocate sk buffer for this Bulk-In buffer!\n", __FUNCTION__));
		return NULL;
	}

	/* copy the rx packet*/
	/* ULLI memcpy a whole packet very is fast or better slow  ... */
	memcpy(skb_put(skb, ThisFrameLen), pData, ThisFrameLen);
	skb->dev = get_netdev_from_bssid(pAd, BSS0);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((u8 *)pRxInfo, TYPE_RXINFO);
#endif /* RT_BIG_ENDIAN */

	memmove((VOID *)&pRxBlk->hw_rx_info[0], (VOID *)pRxFceInfo, sizeof(struct mt7612u_rxfce_info_pkt));
	pRxBlk->pRxFceInfo = (struct mt7612u_rxfce_info_pkt *)&pRxBlk->hw_rx_info[0];

	memmove(&pRxBlk->hw_rx_info[RXINFO_OFFSET], pRxInfo, RXINFO_SIZE);
	pRxBlk->pRxInfo = (struct mt7612u_rxinfo *)&pRxBlk->hw_rx_info[RXINFO_OFFSET];

	/* update next packet read position.*/
	pAd->ReadPosition += (ThisFrameLen + RXDMA_FIELD_SIZE + RXINFO_SIZE);	/* 8 for (RXDMA_FIELD_SIZE + sizeof(RXINFO_STRUC))*/

	pRxBlk->pRxPacket = skb;
	pRxBlk->pData = skb->data;
	pRxBlk->pHeader = (HEADER_802_11 *)(pRxBlk->pData + RXWISize);
	pRxBlk->Flags = 0;

	return skb;
}


#ifdef CONFIG_STA_SUPPORT
VOID RtmpUsbStaAsicForceWakeupTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct rtmp_adapter *pAd = (struct rtmp_adapter *)FunctionContext;



	if (pAd && pAd->Mlme.AutoWakeupTimerRunning)
	{
		{
			RTUSBBulkReceive(pAd);

		}
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		pAd->Mlme.AutoWakeupTimerRunning = false;
	}
}


VOID RT28xxUsbStaAsicForceWakeup(struct rtmp_adapter *pAd, bool bFromTx)
{
	bool	Canceled;

	if (pAd->Mlme.AutoWakeupTimerRunning)
	{
		RTMPCancelTimer(&pAd->Mlme.AutoWakeupTimer, &Canceled);
		pAd->Mlme.AutoWakeupTimerRunning = false;
	}

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
}


VOID RT28xxUsbStaAsicSleepThenAutoWakeup(
	IN struct rtmp_adapter *pAd,
	IN unsigned short TbttNumToNextWakeUp)
{


	/* Not going to sleep if in the Count Down Time*/
	if (pAd->CountDowntoPsm > 0)
		return;

	/* we have decided to SLEEP, so at least do it for a BEACON period.*/
	if (TbttNumToNextWakeUp == 0)
		TbttNumToNextWakeUp = 1;

	RTMPSetTimer(&pAd->Mlme.AutoWakeupTimer, AUTO_WAKEUP_TIMEOUT);
	pAd->Mlme.AutoWakeupTimerRunning = true;

	{
		/* cancel bulk-in IRPs prevent blocking CPU enter C3.*/
		if((pAd->PendingRx > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			RTUSBCancelPendingBulkInIRP(pAd);
			/* resend bulk-in IRPs to receive beacons after a period of (pAd->CommonCfg.BeaconPeriod - 40) ms*/
			pAd->PendingRx = 0;
		}
	}

	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);

}
#endif /* CONFIG_STA_SUPPORT */


