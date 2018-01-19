#ifndef __RTMP_USB_H__
#define __RTMP_USB_H__

#include "rtusb_io.h"

#define RXBULKAGGRE_SIZE			12
#define MAX_TXBULK_LIMIT			(LOCAL_TXBUF_SIZE*(BULKAGGRE_SIZE-1))
#define MAX_TXBULK_SIZE			(LOCAL_TXBUF_SIZE*BULKAGGRE_SIZE)
#define MAX_RXBULK_SIZE			(LOCAL_TXBUF_SIZE*RXBULKAGGRE_SIZE)
#define MAX_MLME_HANDLER_MEMORY 20
#define CMD_RSP_BULK_SIZE	1024

/*Power saving */
#define PowerWakeCID		3
#define CID0MASK		0x000000ff
#define CID1MASK		0x0000ff00
#define CID2MASK		0x00ff0000
#define CID3MASK		0xff000000


/* Flags for Bulkflags control for bulk out data */
/* */
#define	fRTUSB_BULK_OUT_DATA_NULL			0x00000001
#define	fRTUSB_BULK_OUT_RTS					0x00000002
#define	fRTUSB_BULK_OUT_MLME				0x00000004

#define	fRTUSB_BULK_OUT_PSPOLL				0x00000010
#define	fRTUSB_BULK_OUT_DATA_FRAG			0x00000020
#define	fRTUSB_BULK_OUT_DATA_FRAG_2			0x00000040
#define	fRTUSB_BULK_OUT_DATA_FRAG_3			0x00000080
#define	fRTUSB_BULK_OUT_DATA_FRAG_4			0x00000100

#define	fRTUSB_BULK_OUT_DATA_NORMAL			0x00010000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_2		0x00020000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_3		0x00040000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_4		0x00080000

/* TODO:move to ./ate/include/iface/ate_usb.h */

#define FREE_HTTX_RING(_pCookie, _pipeId, _txContext)			\
{										\
	if ((_txContext)->ENextBulkOutPosition == (_txContext)->CurWritePosition)	\
	{																	\
		(_txContext)->bRingEmpty = true;			\
	}																	\
	/*NdisInterlockedDecrement(&(_p)->TxCount); */\
}

#define NT_SUCCESS(status)			(((status) >=0) ? (true):(false))




#define PIRP		PVOID
/*#define NDIS_OID	UINT */
#ifndef USB_ST_NOERROR
#define USB_ST_NOERROR     0
#endif


/* vendor-specific control operations */
#define CONTROL_TIMEOUT_JIFFIES ( (300 * OS_HZ) / 1000)
/*#define UNLINK_TIMEOUT_MS		3 // os abl move */


#define DEVICE_CLASS_REQUEST_OUT		0x20
#define DEVICE_VENDOR_REQUEST_OUT       0x40
#define DEVICE_VENDOR_REQUEST_IN        0xc0
/*#define INTERFACE_VENDOR_REQUEST_OUT    0x41 */
/*#define INTERFACE_VENDOR_REQUEST_IN     0xc1 */
#define BULKOUT_MGMT_RESET_FLAG		0x80

#define RTUSB_SET_BULK_FLAG(_M, _F)	((_M)->BulkFlags |= (_F))
#define RTUSB_CLEAR_BULK_FLAG(_M, _F)	((_M)->BulkFlags &= ~(_F))
#define RTUSB_TEST_BULK_FLAG(_M, _F)	(((_M)->BulkFlags & (_F)) != 0)

struct _MGMT_STRUC;
struct _TX_BLK;

int mt7612u_vendor_request(struct rtmp_adapter *pAd, u8 requesttype, enum mt_vendor_req request,
			u16 value, u16 index, void *data, u16 size);

int RTUSBMultiWrite(struct rtmp_adapter *pAd, unsigned short Offset, u8 *buf, unsigned short len);
int RTUSBSingleWrite(struct rtmp_adapter *pAd, unsigned short Offset, unsigned short val);

int RTUSBReadBBPRegister(struct rtmp_adapter *pAd, u8 Id, u8 *pValue);
int RTUSBWriteBBPRegister(struct rtmp_adapter *pAd, u8 Id, u8 Value);
int RTUSBWriteRFRegister(struct rtmp_adapter *pAd, uint32_t Value);
void mt7612u_write32(struct rtmp_adapter *pAd, unsigned short Offset, uint32_t val);
u32 mt76u_reg_read(struct rtmp_adapter *pAd, unsigned short Offset);

int RTUSBReadEEPROM(struct rtmp_adapter *pAd, unsigned short Offset, u8 *buf, unsigned short len);

bool AsicCheckCommandOk(struct rtmp_adapter *pAd, u8 cmd);


VOID RTUSBDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt);
INT RTUSBCmdThread(ULONG Context);


VOID RTUSBBssBeaconExit(struct rtmp_adapter *pAd);
VOID RTUSBBssBeaconStop(struct rtmp_adapter *pAd);
VOID RTUSBBssBeaconStart(struct rtmp_adapter * pAd);
VOID RTUSBBssBeaconInit(struct rtmp_adapter *pAd);


int RTUSBSetHardWareRegister(struct rtmp_adapter *pAd, PVOID pBuf);
int RTUSBQueryHardWareRegister(struct rtmp_adapter *pAd, PVOID pBuf);

VOID RTUSBMlmeHardTransmit(struct rtmp_adapter *pAd, struct _MGMT_STRUC *pMgmt);

int RTUSBFreeDescRequest(struct rtmp_adapter *pAd, u8 BulkOutPipeId, uint32_t req_cnt);
bool	RTUSBNeedQueueBackForAgg(struct rtmp_adapter *pAd, u8 BulkOutPipeId);

unsigned short RtmpUSB_WriteSubTxResource(struct rtmp_adapter *pAd, struct _TX_BLK *pTxBlk, bool bIsLast, unsigned short *freeCnt);
unsigned short RtmpUSB_WriteSingleTxResource(struct rtmp_adapter *pAd, struct _TX_BLK *pTxBlk, bool bIsLast, unsigned short *freeCnt);
unsigned short RtmpUSB_WriteFragTxResource(struct rtmp_adapter *pAd, struct _TX_BLK *pTxBlk, u8 fragNum, unsigned short *freeCnt);
unsigned short RtmpUSB_WriteMultiTxResource(struct rtmp_adapter *pAd, struct _TX_BLK *pTxBlk, u8 frmNum, unsigned short *freeCnt);
VOID RtmpUSB_FinalWriteTxResource(struct rtmp_adapter *pAd, struct _TX_BLK *pTxBlk, unsigned short mpdu_len, unsigned short TxIdx);

VOID RtmpUSBDataLastTxIdx(struct rtmp_adapter *pAd, u8 QueIdx, unsigned short TxIdx);
VOID RtmpUSBDataKickOut(struct rtmp_adapter *pAd, struct _TX_BLK *pTxBlk, u8 QueIdx);
int RtmpUSBMgmtKickOut(struct rtmp_adapter *pAd, u8 QIdx, struct sk_buff *pkt, u8 *pSrcBufVA, UINT SrcBufLen);
VOID RtmpUSBNullFrameKickOut(struct rtmp_adapter *pAd, u8 QIdx, u8 *pNullFrm, uint32_t frmLen);

VOID RTUSBWatchDog(struct rtmp_adapter *pAd);

int RTUSBWakeUp(struct rtmp_adapter *pAd);

VOID RtmpUsbStaAsicForceWakeupTimeout(PVOID arg1, PVOID FuncContext, PVOID arg2, PVOID arg3);

VOID RT28xxUsbStaAsicForceWakeup(struct rtmp_adapter *pAd, bool bFromTx);

VOID RT28xxUsbStaAsicSleepThenAutoWakeup(struct rtmp_adapter *pAd, unsigned short TbttNumToNextWakeUp);

VOID RT28xxUsbMlmeRadioOn(struct rtmp_adapter *pAd);
VOID RT28xxUsbMlmeRadioOFF(struct rtmp_adapter *pAd);

VOID RT28xxUsbAsicWOWEnable(struct rtmp_adapter *pAd);
VOID RT28xxUsbAsicWOWDisable(struct rtmp_adapter *pAd);

struct usb_control {
	bool usb_aggregation;
};

#endif /* __RTMP_USB_H__ */

