/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
        mac_usb.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __MAC_USB_H__
#define __MAC_USB_H__

#include "rtmp_type.h"
#include "mac_ral/nmac/ral_nmac_usb.h"

#include "mac_ral/rtmp_mac.h"
#include "chip/rtmp_phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"


#define USB_CYC_CFG				0x02a4

#define MT_USB_DMA_CFG_RX_BULK_AGG_TOUT	GENMASK(7, 0)	/* Rx Bulk Aggregation TimeOut  in unit of 33ns */
#define MT_USB_DMA_CFG_RX_BULK_AGG_LMT	GENMASK(15, 8)	/* Rx Bulk Aggregation Limit  in unit of 256 bytes */
#define MT_USB_DMA_CFG_UDMA_TX_WL_DROP	BIT(16)		/* Drop current WL TX packets in UDMA */
#define MT_USB_DMA_CFG_WAKEUP_EN	BIT(17)		/* USB wakeup host enable */
#define MT_USB_DMA_CFG_RX_DROP_OR_PAD	BIT(18)		/* Drop current WL RX packets in UDMA */
#define MT_USB_DMA_CFG_TX_CLR		BIT(19)		/* Clear USB DMA TX path */
#define MT_USB_DMA_CFG_WL_LPK_EN	BIT(20)		/* Halt TXOP count down when TX buffer is full.*/
#define MT_USB_DMA_CFG_RX_BULK_AGG_EN	BIT(21)		/* Enable Rx Bulk Aggregation */
#define MT_USB_DMA_CFG_RX_BULK_EN	BIT(22)		/* Enable USB DMA Rx */
#define MT_USB_DMA_CFG_TX_BULK_EN	BIT(23)		/* Enable USB DMA Tx */
#define MT_USB_DMA_CFG_EP_OUT_VALID	GENMASK(29, 24)	/* OUT endpoint data valid */
#define MT_USB_DMA_CFG_RX_BUSY		BIT(30)		/* USB DMA RX FSM busy */
#define MT_USB_DMA_CFG_TX_BUSY		BIT(31)		/* USB DMA TX FSM busy */


/*#define BEACON_RING_SIZE		2 */
#define MGMTPIPEIDX				0	/* EP6 is highest priority */

/* os abl move */
/*#define RTMP_PKT_TAIL_PADDING 	11 // 3(max 4 byte padding) + 4 (last packet padding) + 4 (MaxBulkOutsize align padding) */

#define fRTMP_ADAPTER_NEED_STOP_TX		\
		(fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS |	\
		 fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_BULKOUT_RESET | \
		 fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_REMOVE_IN_PROGRESS)


/* =================================================================================
	USB TX / RX Frame Descriptors format

	Tx Memory Layout
	1. Packet Buffer
		TxINFO(4 bytes) + TXWI( 16 bytes) + 802.11
         31                                                                                                                             0
	+--------------------------------------------------------------------------+
	|                                   TXINFO[31:0]                                                                            |
	+--------------------------------------------------------------------------+
	|					TxWI                                                                                          |
	+                                                                                                                                  +
	|                                                                                                                                   |
	+                                                                                                                                  +
	|                                                                                                                                   |
	+	                                                                                                                             +
	|	                                                                                                                             |
	+--------------------------------------------------------------------------+
	|                                      802.11                                                                                  |
	|                                      .........                                                                                   |
	+--------------------------------------------------------------------------+


	Rx Memory Layout
	1. Packet Buffer
		RxDMALen(4 bytes) + RXWI(16 bytes) + 802.11 + RXINFO (4 bytes)
         31                                                                                                                             0
	+--------------------------------------------------------------------------+
	|                                  RXDMALen[31:0]                                                                        |
	+--------------------------------------------------------------------------+
	|					 RxWI                                                                                         |
	+                                                                                                                                  +
	|                                                                                                                                   |
	+                                                                                                                                  +
	|                                                                                                                                   |
	+	                                                                                                                             +
	|	                                                                                                                             |
	+--------------------------------------------------------------------------+
	|                                  802.11                                                                                       |
	|                                  .........                                                                                        |
	+--------------------------------------------------------------------------+
	|                                  RXINFO                                                                                     |
	+--------------------------------------------------------------------------+

================================================================================= */


/*
	RXINFO appends at the end of each rx packet
*/
#define RXDMA_FIELD_SIZE	4


/*
	Rx descriptor format, Rx Ring
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXD_STRUC{
	uint32_t dma_len;
}RXD_STRUC, *PRXD_STRUC;
#else
typedef	struct GNU_PACKED _RXD_STRUC{
	uint32_t dma_len;
}RXD_STRUC, *PRXD_STRUC;
#endif


/*
	Management ring buffer format
*/
typedef	struct _MGMT_STRUC {
	bool		Valid;
	u8 *	pBuffer;
	ULONG		Length;
} MGMT_STRUC, *PMGMT_STRUC;


/*////////////////////////////////////////////////////////////////////////*/
/* The TX_BUFFER structure forms the transmitted USB packet to the device */
/*////////////////////////////////////////////////////////////////////////*/
typedef struct __TX_BUFFER{
	union{
		u8 		WirelessPacket[TX_BUFFER_NORMSIZE];
		HEADER_802_11	NullFrame;
		PSPOLL_FRAME	PsPollPacket;
		RTS_FRAME		RTSFrame;
	}field;
	u8 		Aggregation[4];  /*Buffer for save Aggregation size. */
} TX_BUFFER, *PTX_BUFFER;

typedef struct __HTTX_BUFFER{
	union{
		u8 		WirelessPacket[MAX_TXBULK_SIZE];
		HEADER_802_11	NullFrame;
		PSPOLL_FRAME	PsPollPacket;
		RTS_FRAME		RTSFrame;
	}field;
	u8 		Aggregation[4];  /*Buffer for save Aggregation size. */
} HTTX_BUFFER, *PHTTX_BUFFER;


#define EDCA_AC0_PIPE	0	/* Bulk EP1 OUT */
#define EDCA_AC1_PIPE	1	/* Bulk EP2 OUT */
#define EDCA_AC2_PIPE	2	/* Bulk EP3	OUT */
#define	EDCA_AC3_PIPE	3	/* Bulk EP4 OUT */
#define	HCCA_PIPE		4	/* Bulk EP5	OUT */

/* used to track driver-generated write irps */
typedef struct _TX_CONTEXT
{
	PVOID			pAd;		/*Initialized in MiniportInitialize */
	struct urb *		pUrb;			/*Initialized in MiniportInitialize */
	PIRP			pIrp;			/*used to cancel pending bulk out. */
									/*Initialized in MiniportInitialize */
	PTX_BUFFER		TransferBuffer;	/*Initialized in MiniportInitialize */
	ULONG			BulkOutSize;
	u8 		BulkOutPipeId;
	u8 		SelfIdx;
	bool			InUse;
	bool			bWaitingBulkOut; /* at least one packet is in this TxContext, ready for making IRP anytime. */
	bool			bFullForBulkOut; /* all tx buffer are full , so waiting for tx bulkout. */
	bool			IRPPending;
	bool			LastOne;
	u8 		Header_802_3[LENGTH_802_3];
	u8 		Rsv[2];
	ULONG			DataOffset;
	UINT			TxRate;
	ra_dma_addr_t		data_dma;

}	TX_CONTEXT, *PTX_CONTEXT, **PPTX_CONTEXT;


/* used to track driver-generated write irps */
typedef struct _HT_TX_CONTEXT
{
	PVOID			pAd;		/*Initialized in MiniportInitialize */
	struct urb *		pUrb;			/*Initialized in MiniportInitialize */
	PIRP			pIrp;			/*used to cancel pending bulk out. */
									/*Initialized in MiniportInitialize */
	PHTTX_BUFFER	TransferBuffer;	/*Initialized in MiniportInitialize */
	ULONG			BulkOutSize;	/* Indicate the total bulk-out size in bytes in one bulk-transmission */
	u8 		BulkOutPipeId;
	bool			IRPPending;
	bool			LastOne;
	bool			bCurWriting;
	bool			bRingEmpty;
	bool			bCopySavePad;
	u8 		SavedPad[8];
	u8 		Header_802_3[LENGTH_802_3];
	ULONG			CurWritePosition;		/* Indicate the buffer offset which packet will be inserted start from. */
	ULONG			CurWriteRealPos;		/* Indicate the buffer offset which packet now are writing to. */
	ULONG			NextBulkOutPosition;	/* Indicate the buffer start offset of a bulk-transmission */
	ULONG			ENextBulkOutPosition;	/* Indicate the buffer end offset of a bulk-transmission */
	UINT			TxRate;
	ra_dma_addr_t		data_dma;		/* urb dma on linux */

}	HT_TX_CONTEXT, *PHT_TX_CONTEXT, **PPHT_TX_CONTEXT;


typedef struct _CMD_CONTEXT
{
	PVOID pAd;
	struct urb *pUrb;
	ra_dma_addr_t data_dma;
	u8 *TransferBuffer;
	bool IRPPending;
}  CMD_CONTEXT, *PCMD_CONTEXT, **PPCMD_CONTEXT;

/*
	Structure to keep track of receive packets and buffers to indicate
	receive data to the protocol.
*/
typedef struct _RX_CONTEXT
{
	u8 *			TransferBuffer;
	PVOID				pAd;
	PIRP				pIrp;/*used to cancel pending bulk in. */
	struct urb *			pUrb;
	/*These 2 Boolean shouldn't both be 1 at the same time. */
	ULONG				BulkInOffset;	/* number of packets waiting for reordering . */
/*	bool				ReorderInUse;	// At least one packet in this buffer are in reordering buffer and wait for receive indication */
	bool				bRxHandling;	/* Notify this packet is being process now. */
	bool				InUse;			/* USB Hardware Occupied. Wait for USB HW to put packet. */
	bool				Readable;		/* Receive Complete back. OK for driver to indicate receiving packet. */
	bool				IRPPending;		/* TODO: To be removed */
	/*atomic_t				IrpLock; */
	spinlock_t		RxContextLock;
	ra_dma_addr_t			data_dma;		/* urb dma on linux */
}	RX_CONTEXT, *PRX_CONTEXT;


typedef struct _CMD_RSP_CONTEXT
{
	u8 *CmdRspBuffer;
	PVOID pAd;
	struct urb *pUrb;
	bool IRPPending;
	bool InUse;
	bool Readable;
	ra_dma_addr_t data_dma;
} CMD_RSP_CONTEXT, *PCMD_RSP_CONTEXT;

/******************************************************************************

  	USB Frimware Related MACRO

******************************************************************************/
/* 8051 firmware image for usb - use last-half base address = 0x3000 */
#define FIRMWARE_IMAGE_BASE			0x3000
#define MAX_FIRMWARE_IMAGE_SIZE		0x1000    /* 4kbyte */




/******************************************************************************

  	USB TX Related MACRO

******************************************************************************/
#define RTMP_START_DEQUEUE(pAd, QueIdx, irqFlags)				\
			{													\
				spin_lock_bh(&pAd->DeQueueLock[QueIdx]);		\
				if (pAd->DeQueueRunning[QueIdx])						\
				{														\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
					DBGPRINT(RT_DEBUG_INFO, ("DeQueueRunning[%d]= true!\n", QueIdx));		\
					continue;											\
				}														\
				else													\
				{														\
					pAd->DeQueueRunning[QueIdx] = true;					\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
				}														\
			}

#define RTMP_STOP_DEQUEUE(pAd, QueIdx, irqFlags)						\
			do{															\
				spin_lock_bh(&pAd->DeQueueLock[QueIdx]);		\
				pAd->DeQueueRunning[QueIdx] = false;					\
				RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);	\
			}while(0)

#define	RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) \
		(RTUSBFreeDescRequest(pAd, pTxBlk->QueIdx, (pTxBlk->TotalFrameLen + pPacket->len)) == NDIS_STATUS_SUCCESS)

#define RTMP_RELEASE_DESC_RESOURCE(pAd, QueIdx)			\
		do{}while(0)

#define NEED_QUEUE_BACK_FOR_AGG(_pAd, _QueIdx, _freeNum, _TxFrameType) 		\
		((_TxFrameType == TX_RALINK_FRAME) && (RTUSBNeedQueueBackForAgg(_pAd, _QueIdx)))

#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
			RtmpUSB_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)	\
			RtmpUSB_WriteSingleTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber) \
			RtmpUSB_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber)

#define HAL_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)	\
			RtmpUSB_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)

#define HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)	\
			RtmpUSB_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)

#define HAL_LastTxIdx(pAd, QueIdx,TxIdx) \
			/*RtmpUSBDataLastTxIdx(pAd, QueIdx,TxIdx)*/

#define HAL_KickOutTx(pAd, pTxBlk, QueIdx)	\
			RtmpUSBDataKickOut(pAd, pTxBlk, QueIdx)

#define HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen)	\
			RtmpUSBMgmtKickOut(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen)

#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
			RtmpUSBNullFrameKickOut(_pAd, _QueIdx, _pNullFrame, _frameLen)

#define GET_TXRING_FREENO(_pAd, _QueIdx)	(_QueIdx) /*(_pAd->TxRing[_QueIdx].TxSwFreeIdx) */
#define GET_MGMTRING_FREENO(_pAd)			(_pAd->MgmtRing.TxSwFreeIdx)


/* ----------------- RX Related MACRO ----------------- */


/*
  *	MLME Related MACRO
  */
#define RTMP_MLME_HANDLER(pAd)			RtmpMLMEUp(&(pAd->mlmeTask))

#define RTMP_MLME_PRE_SANITY_CHECK(pAd)								\
	{	if ((pAd->StaCfg.bHardwareRadio == true) && 					\
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&		\
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))) {	\
			RTEnqueueInternalCmd(pAd, CMDTHREAD_CHECK_GPIO, NULL, 0); } }

#define RTMP_MLME_RESET_STATE_MACHINE(pAd)	\
		        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_RESET_CONF, 0, NULL, 0);	\
		        RTMP_MLME_HANDLER(pAd);

#define RTMP_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)		\
	{	RTEnqueueInternalCmd(_pAd, CMDTHREAD_802_11_COUNTER_MEASURE, _pEntry, sizeof(MAC_TABLE_ENTRY));	\
		RTMP_MLME_HANDLER(_pAd);									\
	}


/*
  *	Power Save Related MACRO
  */
#ifdef CONFIG_STA_SUPPORT
#define RTMP_PS_POLL_ENQUEUE(pAd)						\
	{	RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL);	\
		RTUSBKickBulkOut(pAd); }

#define RTMP_STA_FORCE_WAKEUP(_pAd, bFromTx) \
	RT28xxUsbStaAsicForceWakeup(_pAd, bFromTx);

#define RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp) \
    RT28xxUsbStaAsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);

#define RTMP_SET_PSM_BIT(_pAd, _val) \
	{\
		if ((_pAd)->StaCfg.WindowsPowerMode == Ndis802_11PowerModeFast_PSP) \
			MlmeSetPsmBit(_pAd, _val);\
		else \
		{ \
			unsigned short _psm_val = _val; \
			RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_PSM_BIT, &(_psm_val), sizeof(unsigned short)); \
		}\
	}
#endif /* CONFIG_STA_SUPPORT */

#define RTMP_MLME_RADIO_ON(pAd) \
    RT28xxUsbMlmeRadioOn(pAd);

#define RTMP_MLME_RADIO_OFF(pAd) \
    RT28xxUsbMlmeRadioOFF(pAd);

/* MAC Search table */
/* add this entry into ASIC RX WCID search table */
#define RTMP_STA_ENTRY_ADD(pAd, pEntry)							\
{																\
	RT_SET_ASIC_WCID Info;									\
																\
	Info.WCID = pEntry->wcid;									\
	memmove(Info.Addr, pEntry->Addr, MAC_ADDR_LEN);		\
																\
	RTEnqueueInternalCmd(pAd, CMDTHREAD_SET_CLIENT_MAC_ENTRY, 	\
							&Info, sizeof(RT_SET_ASIC_WCID));	\
}

/* ----------------- Security Related MACRO ----------------- */

/* Set Asic WCID Attribute table */
#define RTMP_SET_WCID_SEC_INFO(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)	\
{																						\
	RT_ASIC_WCID_SEC_INFO Info;															\
																						\
	Info.BssIdx = _BssIdx;																\
	Info.KeyIdx = _KeyIdx;																\
	Info.CipherAlg = _CipherAlg;														\
	Info.Wcid = _Wcid;																	\
	Info.KeyTabFlag = _KeyTabFlag;														\
																						\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_WCID_SEC_INFO, 							\
							&Info, sizeof(RT_ASIC_WCID_SEC_INFO));						\
}

/* Set Asic WCID IV/EIV table */
#define RTMP_ASIC_WCID_IVEIV_TABLE(_pAd, _Wcid, _uIV, _uEIV)	\
{																\
	RT_ASIC_WCID_IVEIV_ENTRY Info;							\
																\
	Info.Wcid = _Wcid;											\
	Info.Iv = _uIV;												\
	Info.Eiv = _uEIV;											\
																\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_ASIC_WCID_IVEIV, 	\
							&Info, 								\
							sizeof(RT_ASIC_WCID_IVEIV_ENTRY));	\
}

/* Set Asic WCID Attribute table */
#define RTMP_ASIC_WCID_ATTR_TABLE(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)	\
{																							\
	RT_ASIC_WCID_ATTR_ENTRY Info;															\
																							\
	Info.BssIdx = _BssIdx;																	\
	Info.KeyIdx = _KeyIdx;																	\
	Info.CipherAlg = _CipherAlg;															\
	Info.Wcid = _Wcid;																		\
	Info.KeyTabFlag = _KeyTabFlag;															\
																							\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_ASIC_WCID_ATTR, 								\
							&Info, sizeof(RT_ASIC_WCID_ATTR_ENTRY));						\
}

/* Set Asic Pairwise key table */
#define RTMP_ASIC_PAIRWISE_KEY_TABLE(_pAd, _WCID, _pCipherKey)			\
{																		\
	RT_ASIC_PAIRWISE_KEY Info;										\
																		\
	Info.WCID = _WCID;													\
	memmove(&Info.CipherKey, _pCipherKey, sizeof(CIPHER_KEY));	\
																		\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_ASIC_PAIRWISE_KEY,			\
							&Info, sizeof(RT_ASIC_PAIRWISE_KEY));		\
}

/* Set Asic Shared key table */
#define RTMP_ASIC_SHARED_KEY_TABLE(_pAd, _BssIndex, _KeyIdx, _pCipherKey) 	\
{																			\
	RT_ASIC_SHARED_KEY Info;											\
																			\
	Info.BssIndex = _BssIndex;												\
	Info.KeyIdx = _KeyIdx;													\
	memmove(&Info.CipherKey, _pCipherKey, sizeof(CIPHER_KEY));		\
																			\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_ASIC_SHARED_KEY,				\
							&Info, sizeof(RT_ASIC_SHARED_KEY));				\
}

#ifdef CONFIG_STA_SUPPORT
/* Set Port Secured */
#define RTMP_SET_PORT_SECURED(_pAd) 										\
{																			\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_PORT_SECURED, NULL, 0);		\
}
#endif /* CONFIG_STA_SUPPORT */

/* Remove Pairwise Key table */
#define RTMP_REMOVE_PAIRWISE_KEY_ENTRY(_pAd, _Wcid)										\
{																						\
	u8 _tWcid =_Wcid;																\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_REMOVE_PAIRWISE_KEY, &(_tWcid), sizeof(u8));\
}

#define RTMP_OS_IRQ_RELEASE(_pAd, _NetDev)

VOID BeaconUpdateExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

#endif /*__MAC_USB_H__ */

