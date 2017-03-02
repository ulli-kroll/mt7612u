/****************************************************************************

    Module Name:
	rt_os_util.h

	Abstract:
	All function prototypes are provided from UTIL modules.

	Note:
	But can not use any OS key word and compile option here.
	All functions are provided from UTIL modules.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RT_OS_UTIL_H__
#define __RT_OS_UTIL_H__

/* ============================ rt_linux.c ================================== */
/* General */
VOID RtmpUtilInit(VOID);

/* OS Time */
VOID RtmpusecDelay(ULONG usec);
VOID RtmpOsMsDelay(ULONG msec);

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);

ULONG RTMPMsecsToJiffies(UINT msec);
void RTMP_GetCurrentSystemTick(ULONG *pNow);

VOID RtmpOsWait(uint32_t Time);
uint32_t RtmpOsTimerAfter(ULONG a, ULONG b);
uint32_t RtmpOsTimerBefore(ULONG a, ULONG b);
VOID RtmpOsGetSystemUpTime(ULONG *pTime);
uint32_t RtmpOsTickUnitGet(VOID);



int AdapterBlockAllocateMemory(struct rtmp_adapter **ppAd, uint32_t SizeOfpAd);

VOID *RtmpOsVmalloc(ULONG Size);
VOID RtmpOsVfree(VOID *pMem);

ULONG RtmpOsCopyFromUser(VOID *to, const void *from, ULONG n);
ULONG RtmpOsCopyToUser(VOID *to, const void *from, ULONG n);

BOOLEAN RtmpOsStatsAlloc(VOID **ppStats, VOID **ppIwStats);

/* OS Packet */
struct sk_buff *RtmpOSNetPktAlloc(VOID *pReserved, int size);

struct sk_buff *RTMP_AllocateFragPacketBuffer(VOID *pReserved, ULONG Length);

int RTMPAllocateNdisPacket(
	IN	VOID					*pReserved,
	OUT struct sk_buff *		*ppPacket,
	IN	u8 *				pHeader,
	IN	UINT					HeaderLen,
	IN	u8 *				pData,
	IN	UINT					DataLen);

VOID RTMPFreeNdisPacket(VOID *pReserved, struct sk_buff *pPacket);

void RTMP_QueryPacketInfo(
	IN  struct sk_buff *pPacket,
	OUT PACKET_INFO *pPacketInfo,
	OUT u8 **pSrcBufVA,
	OUT	UINT *pSrcBufLen);

struct sk_buff *DuplicatePacket(
	IN	struct net_device *pNetDev,
	IN	struct sk_buff *pPacket,
	IN	UCHAR FromWhichBSSID);

struct sk_buff *duplicate_pkt(
	IN	struct net_device *pNetDev,
	IN	u8 *pHeader802_3,
    IN  UINT HdrLen,
	IN	u8 *pData,
	IN	ULONG DataSize,
	IN	UCHAR FromWhichBSSID);

struct sk_buff *duplicate_pkt_with_TKIP_MIC(
	IN	VOID					*pReserved,
	IN	struct sk_buff *		pOldPkt);

struct sk_buff *duplicate_pkt_with_VLAN(
	IN	struct net_device *			pNetDev,
	IN	USHORT					VLAN_VID,
	IN	USHORT					VLAN_Priority,
	IN	u8 *				pHeader802_3,
    IN  UINT            		HdrLen,
	IN	u8 *				pData,
	IN	ULONG					DataSize,
	IN	UCHAR					FromWhichBSSID,
	IN	UCHAR					*TPID);

typedef void (*RTMP_CB_8023_PACKET_ANNOUNCE)(
			IN	struct rtmp_adapter *pAdr,
			IN	struct sk_buff *pPacket,
			IN	UCHAR			OpMode);

BOOLEAN RTMPL2FrameTxAction(
	IN  struct rtmp_adapter *pAd,
	IN	struct net_device *			pNetDev,
	IN	RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet,
	IN	UCHAR					apidx,
	IN	u8 *				pData,
	IN	uint32_t 				data_len,
	IN	UCHAR			OpMode);

struct sk_buff *ExpandPacket(
	IN	VOID					*pReserved,
	IN	struct sk_buff *		pPacket,
	IN	uint32_t 				ext_head_len,
	IN	uint32_t 				ext_tail_len);

struct sk_buff *ClonePacket(struct sk_buff *pPacket, u8 *pData,ULONG DataSize);

void wlan_802_11_to_802_3_packet(
	IN	struct net_device *			pNetDev,
	IN	UCHAR					OpMode,
	IN	USHORT					VLAN_VID,
	IN	USHORT					VLAN_Priority,
	IN	struct sk_buff *		pRxPacket,
	IN	UCHAR					*pData,
	IN	ULONG					DataSize,
	IN	u8 *				pHeader802_3,
	IN  UCHAR					FromWhichBSSID,
	IN	UCHAR					*TPID);


UCHAR VLAN_8023_Header_Copy(
	IN	USHORT					VLAN_VID,
	IN	USHORT					VLAN_Priority,
	IN	u8 *				pHeader802_3,
	IN	UINT            		HdrLen,
	OUT u8 *				pData,
	IN	UCHAR					FromWhichBSSID,
	IN	UCHAR					*TPID);

VOID RtmpOsPktBodyCopy(
	IN	struct net_device *			pNetDev,
	IN	struct sk_buff *		pNetPkt,
	IN	ULONG					ThisFrameLen,
	IN	u8 *				pData);

INT RtmpOsIsPktCloned(struct sk_buff *pNetPkt);
struct sk_buff *RtmpOsPktCopy(struct sk_buff *pNetPkt);
struct sk_buff *RtmpOsPktClone(struct sk_buff *pNetPkt);

VOID RtmpOsPktDataPtrAssign(struct sk_buff *pNetPkt, UCHAR *pData);

VOID RtmpOsPktLenAssign(struct sk_buff *pNetPkt, LONG Len);
VOID RtmpOsPktTailAdjust(struct sk_buff *pNetPkt, UINT removedTagLen);

u8 *RtmpOsPktTailBufExtend(struct sk_buff *pNetPkt, UINT len);
u8 *RtmpOsPktHeadBufExtend(struct sk_buff *pNetPkt, UINT len);
VOID RtmpOsPktReserve(struct sk_buff *pNetPkt, UINT len);

VOID RtmpOsPktProtocolAssign(struct sk_buff *pNetPkt);
VOID RtmpOsPktInfPpaSend(struct sk_buff *pNetPkt);
VOID RtmpOsPktRcvHandle(struct sk_buff *pNetPkt);
VOID RtmpOsPktNatMagicTag(struct sk_buff *pNetPkt);
VOID RtmpOsPktNatNone(struct sk_buff *pNetPkt);
VOID RtmpOsPktInit(struct sk_buff *pNetPkt, struct net_device *pNetDev, UCHAR *buf, USHORT len);

struct sk_buff *RtmpOsPktIappMakeUp(struct net_device *pNetDev, UINT8 *pMac);

BOOLEAN RtmpOsPktOffsetInit(VOID);

uint16_t RtmpOsNtohs(uint16_t Value);
uint16_t RtmpOsHtons(uint16_t Value);
uint32_t RtmpOsNtohl(uint32_t Value);
uint32_t RtmpOsHtonl(uint32_t Value);

/* OS File */
RTMP_OS_FD RtmpOSFileOpen(char *pPath,  int flag, int mode);
int RtmpOSFileClose(RTMP_OS_FD osfd);
void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset);
int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen);
int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen);

int32_t RtmpOsFileIsErr(VOID *pFile);

void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet);

/* OS Network Interface */
int RtmpOSNetDevAddrSet(
	IN UCHAR OpMode,
	IN struct net_device *pNetDev,
	IN u8 *pMacAddr,
	IN u8 *dev_name);

void RtmpOSNetDevClose(struct net_device *pNetDev);
void RtmpOSNetDevFree(struct net_device *pNetDev);
INT RtmpOSNetDevAlloc(struct net_device **new_dev_p, uint32_t privDataSize);
INT RtmpOSNetDevOpsAlloc(PVOID *pNetDevOps);


#ifdef CONFIG_STA_SUPPORT
INT RtmpOSNotifyRawData(struct net_device *pNetDev, UCHAR *buf, INT len, ULONG type, USHORT proto);

#endif /* CONFIG_STA_SUPPORT */

struct net_device *RtmpOSNetDevGetByName(struct net_device *pNetDev, char *pDevName);

void RtmpOSNetDeviceRefPut(struct net_device *pNetDev);

INT RtmpOSNetDevDestory(VOID *pReserved, struct net_device *pNetDev);
void RtmpOSNetDevDetach(struct net_device *pNetDev);
int RtmpOSNetDevAttach(
	IN	UCHAR					OpMode,
	IN	struct net_device *			pNetDev,
	IN	RTMP_OS_NETDEV_OP_HOOK	*pDevOpHook);

void RtmpOSNetDevProtect(
	IN BOOLEAN lock_it);

struct net_device *RtmpOSNetDevCreate(
	IN	int32_t 				MC_RowID,
	IN	uint32_t 				*pIoctlIF,
	IN	INT 					devType,
	IN	INT						devNum,
	IN	INT						privMemSize,
	IN	char *				pNamePrefix);

BOOLEAN RtmpOSNetDevIsUp(struct net_device *pDev);

unsigned char *RtmpOsNetDevGetPhyAddr(struct net_device *pDev);

VOID RtmpOsNetQueueStart(struct net_device *pDev);
VOID RtmpOsNetQueueStop(struct net_device *pDev);
VOID RtmpOsNetQueueWake(struct net_device *pDev);

VOID RtmpOsSetPktNetDev(VOID *pPkt, struct net_device *pDev);

char *RtmpOsGetNetDevName(struct net_device *pDev);

uint32_t RtmpOsGetNetIfIndex(struct net_device *pDev);

VOID RtmpOsSetNetDevPriv(struct net_device *pDev, struct rtmp_adapter *pPriv);
struct rtmp_adapter *RtmpOsGetNetDevPriv(struct net_device *pDev);

VOID RtmpOsSetNetDevWdev(struct net_device *net_dev, struct rtmp_wifi_dev *wdev);
struct rtmp_wifi_dev *RtmpOsGetNetDevWdev(struct net_device  *pDev);

USHORT RtmpDevPrivFlagsGet(struct net_device *pDev);
VOID RtmpDevPrivFlagsSet(struct net_device *pDev, USHORT PrivFlags);

VOID RtmpOsSetNetDevType(struct net_device *pDev, USHORT Type);
VOID RtmpOsSetNetDevTypeMonitor(struct net_device *pDev);
UCHAR get_sniffer_mode(struct net_device *pDev);
VOID set_sniffer_mode(struct net_device *pDev, UCHAR mode);

/* OS Semaphore */
VOID RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask);
BOOLEAN RtmpOsSemaInitLocked(RTMP_OS_SEM *pSemOrg, LIST_HEADER *pSemList);
BOOLEAN RtmpOsSemaInit(RTMP_OS_SEM *pSemOrg, LIST_HEADER *pSemList);
BOOLEAN RtmpOsSemaDestory(RTMP_OS_SEM *pSemOrg);
INT RtmpOsSemaWaitInterruptible(RTMP_OS_SEM *pSemOrg);
VOID RtmpOsSemaWakeUp(RTMP_OS_SEM *pSemOrg);
VOID RtmpOsMlmeUp(RTMP_OS_TASK *pMlmeQTask);

VOID RtmpOsInitCompletion(RTMP_OS_COMPLETION *pCompletion);
VOID RtmpOsExitCompletion(RTMP_OS_COMPLETION *pCompletion);
VOID RtmpOsComplete(RTMP_OS_COMPLETION *pCompletion);
ULONG RtmpOsWaitForCompletionTimeout(RTMP_OS_COMPLETION *pCompletion, ULONG Timeout);

/* OS Task */
BOOLEAN RtmpOsTaskletSche(RTMP_NET_TASK_STRUCT *pTasklet);

BOOLEAN RtmpOsTaskletInit(
	RTMP_NET_TASK_STRUCT *pTasklet,
	VOID (*pFunc)(unsigned long data),
	ULONG Data,
	LIST_HEADER *pTaskletList);

BOOLEAN RtmpOsTaskletKill(RTMP_NET_TASK_STRUCT *pTasklet);
VOID RtmpOsTaskletDataAssign(RTMP_NET_TASK_STRUCT *pTasklet, ULONG Data);

VOID RtmpOsTaskWakeUp(RTMP_OS_TASK *pTaskOrg);
int32_t RtmpOsTaskIsKilled(RTMP_OS_TASK *pTaskOrg);

BOOLEAN RtmpOsCheckTaskLegality(RTMP_OS_TASK *pTaskOrg);

BOOLEAN RtmpOSTaskAlloc(RTMP_OS_TASK *pTask, LIST_HEADER *pTaskList);

VOID RtmpOSTaskFree(RTMP_OS_TASK *pTask);

int RtmpOSTaskKill(RTMP_OS_TASK *pTaskOrg);

INT RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTaskOrg);

VOID RtmpOSTaskCustomize(RTMP_OS_TASK *pTaskOrg);

int RtmpOSTaskAttach(
	IN	RTMP_OS_TASK *pTaskOrg,
	IN	RTMP_OS_TASK_CALLBACK	fn,
	IN	ULONG arg);

int RtmpOSTaskInit(
	IN	RTMP_OS_TASK *pTaskOrg,
	IN	char *pTaskName,
	IN	VOID *pPriv,
	IN	LIST_HEADER *pTaskList,
	IN	LIST_HEADER *pSemList);

BOOLEAN RtmpOSTaskWait(
	IN	VOID *pReserved,
	IN	RTMP_OS_TASK *pTaskOrg,
	IN	int32_t *pStatus);

VOID *RtmpOsTaskDataGet(RTMP_OS_TASK *pTaskOrg);

int32_t RtmpThreadPidKill(RTMP_OS_PID	 PID);

/* OS Cache */
VOID RtmpOsDCacheFlush(ULONG AddrStart, ULONG Size);

/* OS Timer */
VOID RTMP_SetPeriodicTimer(
	IN	NDIS_MINIPORT_TIMER *pTimerOrg,
	IN	unsigned long timeout);

VOID RTMP_OS_Init_Timer(
	IN	VOID *pReserved,
	IN	NDIS_MINIPORT_TIMER *pTimerOrg,
	IN	TIMER_FUNCTION function,
	IN	PVOID data,
	IN	LIST_HEADER *pTimerList);

VOID RTMP_OS_Add_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout);
VOID RTMP_OS_Mod_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout);
VOID RTMP_OS_Del_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, BOOLEAN *pCancelled);
VOID RTMP_OS_Release_Timer(NDIS_MINIPORT_TIMER *pTimerOrg);

BOOLEAN RTMP_OS_Alloc_Rsc(LIST_HEADER *pRscList, VOID *pRsc, uint32_t RscLen);
VOID RTMP_OS_Free_Rscs(LIST_HEADER *pRscList);

/* OS Lock */
BOOLEAN RtmpOsAllocateLock(NDIS_SPIN_LOCK *pLock, LIST_HEADER *pLockList);
VOID RtmpOsFreeSpinLock(NDIS_SPIN_LOCK *pLockOrg);
VOID RtmpOsSpinLockBh(NDIS_SPIN_LOCK *pLockOrg);
VOID RtmpOsSpinUnLockBh(NDIS_SPIN_LOCK *pLockOrg);
VOID RtmpOsIntLock(NDIS_SPIN_LOCK *pLockOrg, ULONG *pIrqFlags);
VOID RtmpOsIntUnLock(NDIS_SPIN_LOCK *pLockOrg, ULONG IrqFlags);

/* OS PID */
VOID RtmpOsGetPid(ULONG *pDst, ULONG PID);
VOID RtmpOsTaskPidInit(RTMP_OS_PID *pPid);

/* OS I/O */
VOID RTMP_PCI_Writel(ULONG Value, VOID *pAddr);
VOID RTMP_PCI_Writew(ULONG Value, VOID *pAddr);
VOID RTMP_PCI_Writeb(ULONG Value, VOID *pAddr);
ULONG RTMP_PCI_Readl(VOID *pAddr);
ULONG RTMP_PCI_Readw(VOID *pAddr);
ULONG RTMP_PCI_Readb(VOID *pAddr);

int RtmpOsPciConfigReadWord(
	IN	VOID					*pDev,
	IN	uint32_t 				Offset,
	OUT uint16_t 				*pValue);

int RtmpOsPciConfigWriteWord(VOID *pDev, uint32_t Offset, uint16_t Value);
int RtmpOsPciConfigReadDWord(VOID *pDev, uint32_t Offset, uint32_t *pValue);
int RtmpOsPciConfigWriteDWord(VOID *pDev, uint32_t Offset, uint32_t Value);

int RtmpOsPciFindCapability(VOID *pDev, INT Cap);

VOID *RTMPFindHostPCIDev(VOID *pPciDevSrc);

int RtmpOsPciMsiEnable(VOID *pDev);
VOID RtmpOsPciMsiDisable(VOID *pDev);

/* OS Wireless */
ULONG RtmpOsMaxScanDataGet(VOID);

/* OS Interrutp */
int32_t RtmpOsIsInInterrupt(VOID);

/* OS USB */
VOID *RtmpOsUsbUrbDataGet(VOID *pUrb);
int RtmpOsUsbUrbStatusGet(VOID *pUrb);
ULONG RtmpOsUsbUrbLenGet(VOID *pUrb);

/* OS Atomic */
BOOLEAN RtmpOsAtomicInit(RTMP_OS_ATOMIC *pAtomic, LIST_HEADER *pAtomicList);
VOID RtmpOsAtomicDestroy(RTMP_OS_ATOMIC *pAtomic);
LONG RtmpOsAtomicRead(RTMP_OS_ATOMIC *pAtomic);
VOID RtmpOsAtomicDec(RTMP_OS_ATOMIC *pAtomic);
VOID RtmpOsAtomicInterlockedExchange(RTMP_OS_ATOMIC *pAtomicSrc, LONG Value);

/* OS Utility */
void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

typedef VOID (*RTMP_OS_SEND_WLAN_EVENT)(
	IN	struct rtmp_adapter			*pAdSrc,
	IN	USHORT					Event_flag,
	IN	u8 *					pAddr,
	IN  UCHAR					BssIdx,
	IN	CHAR					Rssi);

VOID RtmpOsSendWirelessEvent(
	IN	struct rtmp_adapter	*pAd,
	IN	USHORT			Event_flag,
	IN	u8 *			pAddr,
	IN	UCHAR			BssIdx,
	IN	CHAR			Rssi,
	IN	RTMP_OS_SEND_WLAN_EVENT pFunc);

#ifdef CONFIG_AP_SUPPORT
void SendSignalToDaemon(
	IN	INT sig,
	IN	RTMP_OS_PID	 pid,
	IN	unsigned long pid_no);
#endif /* CONFIG_AP_SUPPORT */

int RtmpOSWrielessEventSend(
	IN	struct net_device *			pNetDev,
	IN	uint32_t 				eventType,
	IN	INT						flags,
	IN	u8 *				pSrcMac,
	IN	u8 *				pData,
	IN	uint32_t 				dataLen);

int RtmpOSWrielessEventSendExt(
	IN	struct net_device *			pNetDev,
	IN	uint32_t 				eventType,
	IN	INT						flags,
	IN	u8 *				pSrcMac,
	IN	u8 *				pData,
	IN	uint32_t 				dataLen,
	IN	uint32_t 				family);

UINT RtmpOsWirelessExtVerGet(VOID);

VOID RtmpDrvAllMacPrint(
	IN VOID						*pReserved,
	IN uint32_t 				*pBufMac,
	IN uint32_t 				AddrStart,
	IN uint32_t 				AddrEnd,
	IN uint32_t 				AddrStep);

VOID RtmpDrvAllE2PPrint(
	IN	VOID					*pReserved,
	IN	USHORT					*pMacContent,
	IN	uint32_t 				AddrEnd,
	IN	uint32_t 				AddrStep);

VOID RtmpDrvAllRFPrint(
	IN VOID *pReserved,
	IN UCHAR *pBuf,
	IN uint32_t BufLen);

int RtmpOSIRQRelease(
	IN	struct net_device *			pNetDev,
	IN	uint32_t 				infType,
	IN	PPCI_DEV				pci_dev,
	IN	BOOLEAN					*pHaveMsi);

VOID RtmpOsWlanEventSet(
	IN	VOID					*pReserved,
	IN	BOOLEAN					*pCfgWEnt,
	IN	BOOLEAN					FlgIsWEntSup);

uint16_t RtmpOsGetUnaligned(uint16_t *pWord);

uint32_t RtmpOsGetUnaligned32(uint32_t *pWord);

ULONG RtmpOsGetUnalignedlong(ULONG *pWord);

long RtmpOsSimpleStrtol(
	IN	const char				*cp,
	IN	char 					**endp,
	IN	unsigned int			base);


/* ============================ rt_os_util.c ================================ */
VOID RtmpDrvRateGet(
	IN VOID *pReserved,
	IN UINT8 MODE,
	IN UINT8 ShortGI,
	IN UINT8 BW,
	IN UINT8 MCS,
	IN UINT8 Antenna,
	OUT uint32_t *pRate);

char * rtstrchr(const char * s, int c);

char *  WscGetAuthTypeStr(USHORT authFlag);

char *  WscGetEncryTypeStr(USHORT encryFlag);

USHORT WscGetAuthTypeFromStr(char *arg);

USHORT WscGetEncrypTypeFromStr(char *arg);

VOID RtmpMeshDown(
	IN VOID *pDrvCtrlBK,
	IN BOOLEAN WaitFlag,
	IN BOOLEAN (*RtmpMeshLinkCheck)(IN VOID *pAd));

USHORT RtmpOsNetPrivGet(struct net_device *pDev);

BOOLEAN RtmpOsCmdDisplayLenCheck(
	IN	uint32_t 				LenSrc,
	IN	uint32_t 				Offset);

VOID    WpaSendMicFailureToWpaSupplicant(
	IN	struct net_device *			pNetDev,
	IN const u8 *src_addr,
	IN BOOLEAN bUnicast,
	IN INT key_id,
	IN const u8 *tsc);

int wext_notify_event_assoc(
	IN	struct net_device *			pNetDev,
	IN	UCHAR					*ReqVarIEs,
	IN	uint32_t 				ReqVarIELen);

VOID    SendAssocIEsToWpaSupplicant(
	IN	struct net_device *			pNetDev,
	IN	UCHAR					*ReqVarIEs,
	IN	uint32_t 				ReqVarIELen);

/* ============================ rt_rbus_pci_util.c ========================== */
void RtmpAllocDescBuf(
	IN PPCI_DEV pPciDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	PhysicalAddress);

void RtmpFreeDescBuf(
	IN PPCI_DEV pPciDev,
	IN ULONG Length,
	IN VOID *VirtualAddress,
	IN NDIS_PHYSICAL_ADDRESS	PhysicalAddress);

void RTMP_AllocateFirstTxBuffer(
	IN PPCI_DEV pPciDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	PhysicalAddress);

void RTMP_FreeFirstTxBuffer(
	IN	PPCI_DEV				pPciDev,
	IN	ULONG					Length,
	IN	BOOLEAN					Cached,
	IN	PVOID					VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS	PhysicalAddress);

struct sk_buff *RTMP_AllocateRxPacketBuffer(
	IN	VOID					*pReserved,
	IN	VOID					*pPciDev,
	IN	ULONG					Length,
	IN	BOOLEAN					Cached,
	OUT	PVOID					*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS	PhysicalAddress);

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

int RTMP_Usb_AutoPM_Put_Interface(
	IN	VOID			*pUsb_Dev,
	IN	VOID			*intf);

int  RTMP_Usb_AutoPM_Get_Interface(
	IN	VOID			*pUsb_Dev,
	IN	VOID			*intf);

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */



ra_dma_addr_t linux_pci_map_single(void *pPciDev, void *ptr, size_t size, int sd_idx, int direction);

void linux_pci_unmap_single(void *pPciDev, ra_dma_addr_t dma_addr, size_t size, int direction);

/* ============================ rt_usb_util.c =============================== */
#ifdef RTMP_MAC_USB
typedef VOID (*USB_COMPLETE_HANDLER)(VOID *);

void dump_urb(VOID *purb);

int rausb_register(VOID * new_driver);

void rausb_deregister(VOID * driver);

/*struct urb *rausb_alloc_urb(int iso_packets); */

void rausb_free_urb(VOID *urb);

void rausb_put_dev(VOID *dev);

struct usb_device *rausb_get_dev(VOID *dev);

int rausb_submit_urb(VOID *urb);

void *rausb_buffer_alloc(VOID *dev,
							size_t size,
							ra_dma_addr_t *dma);

void rausb_buffer_free(VOID *dev,
							size_t size,
							void *addr,
							ra_dma_addr_t dma);

int rausb_control_msg(VOID *dev,
						unsigned int pipe,
						__u8 request,
						__u8 requesttype,
						__u16 value,
						__u16 index,
						void *data,
						__u16 size,
						int timeout);

void rausb_fill_bulk_urb(void *urb,
						 void *dev,
						 unsigned int pipe,
						 void *transfer_buffer,
						 int buffer_length,
						 USB_COMPLETE_HANDLER complete_fn,
						 void *context);

unsigned int rausb_sndctrlpipe(VOID *dev, ULONG address);

unsigned int rausb_rcvctrlpipe(VOID *dev, ULONG address);


unsigned int rausb_sndbulkpipe(void *dev, ULONG address);
unsigned int rausb_rcvbulkpipe(void *dev, ULONG address);

void rausb_kill_urb(VOID *urb);

VOID	RtmpOsUsbInitHTTxDesc(
	IN	VOID			*pUrbSrc,
	IN	VOID			*pUsb_Dev,
	IN	UINT			BulkOutEpAddr,
	IN	u8 *		pSrc,
	IN	ULONG			BulkOutSize,
	IN	USB_COMPLETE_HANDLER	Func,
	IN	VOID			*pTxContext,
	IN	ra_dma_addr_t		TransferDma);

VOID	RtmpOsUsbInitRxDesc(
	IN	VOID			*pUrbSrc,
	IN	VOID			*pUsb_Dev,
	IN	UINT			BulkInEpAddr,
	IN	UCHAR			*pTransferBuffer,
	IN	uint32_t 		BufSize,
	IN	USB_COMPLETE_HANDLER	Func,
	IN	VOID			*pRxContext,
	IN	ra_dma_addr_t		TransferDma);

VOID *RtmpOsUsbContextGet(
	IN	VOID			*pUrb);

int RtmpOsUsbStatusGet(
	IN	VOID			*pUrb);

VOID RtmpOsUsbDmaMapping(
	IN	VOID			*pUrb);
#endif /* RTMP_MAC_USB */

uint32_t RtmpOsGetUsbDevVendorID(
	IN VOID *pUsbDev);

uint32_t RtmpOsGetUsbDevProductID(
	IN VOID *pUsbDev);

/* CFG80211 */
#ifdef RT_CFG80211_SUPPORT
typedef struct __CFG80211_BAND {

	UINT8 RFICType;
	UINT8 MpduDensity;
	UINT8 TxStream;
	UINT8 RxStream;
	uint32_t MaxTxPwr;
	uint32_t MaxBssTable;

	uint16_t RtsThreshold;
	uint16_t FragmentThreshold;
	uint32_t RetryMaxCnt; /* bit0~7: short; bit8 ~ 15: long */
	BOOLEAN FlgIsBMode;
} CFG80211_BAND;

VOID CFG80211OS_UnRegister(
	IN VOID						*pCB,
	IN VOID						*pNetDev);

BOOLEAN CFG80211_SupBandInit(
	IN VOID						*pCB,
	IN CFG80211_BAND 			*pBandInfo,
	IN VOID						*pWiphyOrg,
	IN VOID						*pChannelsOrg,
	IN VOID						*pRatesOrg);

BOOLEAN CFG80211OS_SupBandReInit(
	IN VOID						*pCB,
	IN CFG80211_BAND 			*pBandInfo);

VOID CFG80211OS_RegHint(
	IN VOID						*pCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen);

VOID CFG80211OS_RegHint11D(
	IN VOID						*pCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen);

BOOLEAN CFG80211OS_BandInfoGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	OUT VOID					**ppBand24,
	OUT VOID					**ppBand5);

uint32_t CFG80211OS_ChanNumGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	IN uint32_t 				IdBand);

BOOLEAN CFG80211OS_ChanInfoGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	IN uint32_t 				IdBand,
	IN uint32_t 				IdChan,
	OUT uint32_t 				*pChanId,
	OUT uint32_t 				*pPower,
	OUT BOOLEAN					*pFlgIsRadar);

BOOLEAN CFG80211OS_ChanInfoInit(
	IN VOID						*pCB,
	IN uint32_t 				InfoIndex,
	IN UCHAR					ChanId,
	IN UCHAR					MaxTxPwr,
	IN BOOLEAN					FlgIsNMode,
	IN BOOLEAN					FlgIsBW20M);

VOID CFG80211OS_Scaning(
	IN VOID						*pCB,
	IN uint32_t 				ChanId,
	IN UCHAR					*pFrame,
	IN uint32_t 				FrameLen,
	IN int32_t 				RSSI,
	IN BOOLEAN					FlgIsNMode,
	IN UINT8					BW);

VOID CFG80211OS_ScanEnd(
	IN VOID						*pCB,
	IN BOOLEAN					FlgIsAborted);

void CFG80211OS_ConnectResultInform(
	IN VOID						*pCB,
	IN UCHAR					*pBSSID,
	IN UCHAR					*pReqIe,
	IN uint32_t 				ReqIeLen,
	IN UCHAR					*pRspIe,
	IN uint32_t 				RspIeLen,
	IN UCHAR					FlgIsSuccess);

void CFG80211OS_P2pClientConnectResultInform(
	IN struct net_device *				pNetDev,
	IN UCHAR					*pBSSID,
	IN UCHAR					*pReqIe,
	IN uint32_t 				ReqIeLen,
	IN UCHAR					*pRspIe,
	IN uint32_t 				RspIeLen,
	IN UCHAR					FlgIsSuccess);

BOOLEAN CFG80211OS_RxMgmt(IN struct net_device *pNetDev, IN int32_t freq, IN u8 *frame, IN uint32_t len);
VOID CFG80211OS_TxStatus(IN struct net_device *pNetDev, IN int32_t cookie, 	IN u8 *frame, IN uint32_t len, IN BOOLEAN ack);
VOID CFG80211OS_NewSta(IN struct net_device *pNetDev, IN const u8 *mac_addr, IN const u8 *assoc_frame, IN uint32_t assoc_len);
VOID CFG80211OS_DelSta(IN struct net_device *pNetDev, IN const u8 *mac_addr);
VOID CFG80211OS_MICFailReport(IN struct net_device *pNetDev, IN const u8 *src_addr, IN BOOLEAN unicast, IN INT key_id, IN const u8 *tsc );
VOID CFG80211OS_Roamed(struct net_device *pNetDev, IN UCHAR *pBSSID,
					   UCHAR *pReqIe, uint32_t ReqIeLen, UCHAR *pRspIe, uint32_t RspIeLen);
VOID CFG80211OS_RecvObssBeacon(VOID *pCB, const u8 *pFrame, INT frameLen, INT freq);
#endif /* RT_CFG80211_SUPPORT */




/* ================================ MACRO =================================== */
#define RTMP_UTIL_DCACHE_FLUSH(__AddrStart, __Size)

/* ================================ EXTERN ================================== */
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR TPID[];
extern UCHAR IPX[2];
extern UCHAR APPLE_TALK[2];
extern UCHAR NUM_BIT8[8];
extern ULONG RTPktOffsetData, RTPktOffsetLen, RTPktOffsetCB;

extern ULONG OS_NumOfMemAlloc, OS_NumOfMemFree;

extern uint32_t RalinkRate_Legacy[];
extern uint32_t RalinkRate_HT_1NSS[Rate_BW_MAX][Rate_GI_MAX][Rate_MCS];
extern uint32_t RalinkRate_VHT_1NSS[Rate_BW_MAX][Rate_GI_MAX][Rate_MCS];
extern UINT8 newRateGetAntenna(UINT8 MCS);


#ifdef PLATFORM_UBM_IPX8
#include "vrut_ubm.h"
#endif /* PLATFORM_UBM_IPX8 */

int32_t  RtPrivIoctlSetVal(VOID);

void OS_SPIN_LOCK(NDIS_SPIN_LOCK *lock);
void OS_SPIN_UNLOCK(NDIS_SPIN_LOCK *lock);
void OS_SPIN_LOCK_IRQSAVE(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void OS_SPIN_UNLOCK_IRQRESTORE(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void OS_SPIN_LOCK_IRQ(NDIS_SPIN_LOCK *lock);
void OS_SPIN_UNLOCK_IRQ(NDIS_SPIN_LOCK *lock);
void RtmpOsSpinLockIrqSave(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void RtmpOsSpinUnlockIrqRestore(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void RtmpOsSpinLockIrq(NDIS_SPIN_LOCK *lock);
void RtmpOsSpinUnlockIrq(NDIS_SPIN_LOCK *lock);
int OS_TEST_BIT(int bit, unsigned long *flags);
void OS_SET_BIT(int bit, unsigned long *flags);
void OS_CLEAR_BIT(int bit, unsigned long *flags);
void OS_LOAD_CODE_FROM_BIN(unsigned char **image, char *bin_name, void *inf_dev, uint32_t *code_len);

#endif /* __RT_OS_UTIL_H__ */
