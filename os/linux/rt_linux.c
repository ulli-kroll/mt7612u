/****************************************************************************

    Module Name:
    UTIL/rt_linux.c

    Abstract:
	All functions provided from OS module are put here.

	Note:
	1. Can not use sizeof() for a structure with any parameter included
	by any compile option, such as RTMP_ADAPTER.

	Because the struct rtmp_adapter size in the UTIL module is different with
	DRIVER/NETIF.

	2. Do not use any structure with any parameter included by PCI/USB/RBUS/
	AP/STA.

	Because the structure size in the UTIL module is different with
	DRIVER/NETIF.

	3. Do not use any structure defined in DRIVER module, EX: pAd.
	So we can do module partition.

	Revision History:
	Who        When          What
	---------  ----------    -------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "dot11i_wpa.h"
#include <linux/rtnetlink.h>

#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../net/nat/hw_nat/frame_engine.h"
#endif

/* TODO */
#undef RT_CONFIG_IF_OPMODE_ON_AP
#undef RT_CONFIG_IF_OPMODE_ON_STA

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)	if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif

ULONG RTDebugLevel = RT_DEBUG_ERROR;
ULONG RTDebugFunc = 0;

#ifdef VENDOR_FEATURE4_SUPPORT
ULONG OS_NumOfMemAlloc = 0, OS_NumOfMemFree = 0;
#endif /* VENDOR_FEATURE4_SUPPORT */

/*
 * the lock will not be used in TX/RX
 * path so throughput should not be impacted
 */
BOOLEAN FlgIsUtilInit = FALSE;
OS_NDIS_SPIN_LOCK UtilSemLock;

BOOLEAN RTMP_OS_Alloc_RscOnly(VOID *pRscSrc, uint32_t RscLen);
BOOLEAN RTMP_OS_Remove_Rsc(LIST_HEADER *pRscList, VOID *pRscSrc);

/*
========================================================================
Routine Description:
	Initialize something in UTIL module.

Arguments:
	None

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpUtilInit(VOID)
{
	if (FlgIsUtilInit == FALSE) {
		OS_NdisAllocateSpinLock(&UtilSemLock);
		FlgIsUtilInit = TRUE;
	}
}

/* timeout -- ms */
static inline VOID __RTMP_SetPeriodicTimer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
static inline VOID __RTMP_OS_Init_Timer(
	IN VOID *pReserved,
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN TIMER_FUNCTION function,
	IN PVOID data)
{
	if (!timer_pending(pTimer)) {
		init_timer(pTimer);
		pTimer->data = (unsigned long)data;
		pTimer->function = function;
	}
}

static inline VOID __RTMP_OS_Add_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

static inline VOID __RTMP_OS_Mod_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	timeout = ((timeout * OS_HZ) / 1000);
	mod_timer(pTimer, jiffies + timeout);
}

static inline VOID __RTMP_OS_Del_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	OUT BOOLEAN *pCancelled)
{
	if (timer_pending(pTimer))
		*pCancelled = del_timer_sync(pTimer);
	else
		*pCancelled = TRUE;
}

static inline VOID __RTMP_OS_Release_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer)
{
	/* nothing to do */
}


/* Unify all delay routine by using udelay */
VOID RtmpusecDelay(ULONG usec)
{
	ULONG i;

	for (i = 0; i < (usec / 50); i++)
		udelay(50);

	if (usec % 50)
		udelay(usec % 50);
}


VOID RtmpOsMsDelay(ULONG msec)
{
	mdelay(msec);
}

void RTMP_GetCurrentSystemTime(LARGE_INTEGER * time)
{
	time->u.LowPart = jiffies;
}

void RTMP_GetCurrentSystemTick(ULONG *pNow)
{
	*pNow = jiffies;
}

ULONG RTMPMsecsToJiffies(uint32_t m)
{

	return msecs_to_jiffies(m);
}

/* pAd MUST allow to be NULL */

struct sk_buff *RtmpOSNetPktAlloc(VOID *dummy, int size)
{
	struct sk_buff *skb;
	/* Add 2 more bytes for ip header alignment */
	skb = dev_alloc_skb(size + 2);

	return skb;
}

struct sk_buff *RTMP_AllocateFragPacketBuffer(VOID *dummy, ULONG len)
{
	struct sk_buff *pkt;

	pkt = dev_alloc_skb(len);

	if (pkt == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("can't allocate frag rx %ld size packet\n", len));
	}

	return pkt;
}




/*
	The allocated NDIS PACKET must be freed via RTMPFreeNdisPacket()
*/
int RTMPAllocateNdisPacket(
	IN VOID *pReserved,
	OUT struct sk_buff **ppPacket,
	IN UCHAR *pHeader,
	IN UINT HeaderLen,
	IN UCHAR *pData,
	IN UINT DataLen)
{
	struct sk_buff *pPacket;


	ASSERT(pData);
	ASSERT(DataLen);

	/* Add LEN_CCMP_HDR + LEN_CCMP_MIC for PMF */
	pPacket = dev_alloc_skb(HeaderLen + DataLen + RTMP_PKT_TAIL_PADDING + LEN_CCMP_HDR + LEN_CCMP_MIC);
	if (pPacket == NULL) {
		*ppPacket = NULL;
#ifdef DEBUG
		printk(KERN_ERR "RTMPAllocateNdisPacket Fail\n\n");
#endif
		return NDIS_STATUS_FAILURE;
	}

	/* Clone the frame content and update the length of packet */
	if (HeaderLen > 0)
		memmove(pPacket->data, pHeader, HeaderLen);
	if (DataLen > 0)
		memmove(pPacket->data + HeaderLen, pData, DataLen);
	skb_put(pPacket, HeaderLen + DataLen);
/* printk(KERN_ERR "%s : pPacket = %p, len = %d\n", __FUNCTION__, pPacket, GET_OS_PKT_LEN(pPacket));*/

	*ppPacket = pPacket;

	return NDIS_STATUS_SUCCESS;
}


/*
  ========================================================================
  Description:
	This routine frees a miniport internally allocated NDIS_PACKET and its
	corresponding NDIS_BUFFER and allocated memory.
  ========================================================================
*/
VOID RTMPFreeNdisPacket(VOID *pReserved, struct sk_buff *pPacket)
{
	dev_kfree_skb_any(RTPKT_TO_OSPKT(pPacket));
}


void RTMP_QueryPacketInfo(
	IN struct sk_buff *pPacket,
	OUT PACKET_INFO *info,
	OUT UCHAR **pSrcBufVA,
	OUT UINT *pSrcBufLen)
{
	info->BufferCount = 1;
	info->pFirstBuffer = (PNDIS_BUFFER) GET_OS_PKT_DATAPTR(pPacket);
	info->PhysicalBufferCount = 1;
	info->TotalPacketLength = GET_OS_PKT_LEN(pPacket);

	*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
	*pSrcBufLen = GET_OS_PKT_LEN(pPacket);

#ifdef TX_PKT_SG
	if (RTMP_GET_PKT_SG(pPacket)) {
		struct sk_buff *skb = pPacket;
		int i, nr_frags = skb_shinfo(skb)->nr_frags;

		info->BufferCount =  nr_frags + 1;
		info->PhysicalBufferCount = info->BufferCount;
		info->sg_list[0].data = (VOID *)GET_OS_PKT_DATAPTR(pPacket);
		info->sg_list[0].len = skb_headlen(skb);
		for (i = 0; i < nr_frags; i++) {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

			info->sg_list[i+1].data = ((void *) page_address(frag->page) +
									frag->page_offset);
			info->sg_list[i+1].len = frag->size;
		}
	}
#endif /* TX_PKT_SG */
}




struct sk_buff *DuplicatePacket(
	IN struct net_device *pNetDev,
	IN struct sk_buff *pPacket,
	IN UCHAR FromWhichBSSID)
{
	struct sk_buff *skb;
	struct sk_buff *pRetPacket = NULL;
	USHORT DataSize;
	UCHAR *pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (u8 *) GET_OS_PKT_DATAPTR(pPacket);

	skb = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);
	if (skb) {
		skb->dev = pNetDev;
		pRetPacket = skb;
	}

	return pRetPacket;

}


struct sk_buff *duplicate_pkt(
	IN struct net_device *pNetDev,
	IN u8 *pHeader802_3,
	IN UINT HdrLen,
	IN u8 *pData,
	IN ULONG DataSize,
	IN UCHAR FromWhichBSSID)
{
	struct sk_buff *skb;
	struct sk_buff *pPacket = NULL;

	if ((skb =
	     __dev_alloc_skb(HdrLen + DataSize + 2, MEM_ALLOC_FLAG)) != NULL) {

		skb_reserve(skb, 2);
		memmove(skb_tail_pointer(skb), pHeader802_3, HdrLen);
		skb_put(skb, HdrLen);
		memmove(skb_tail_pointer(skb), pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = pNetDev;
		pPacket = skb;
	}

	return pPacket;
}


#define TKIP_TX_MIC_SIZE		8
struct sk_buff *duplicate_pkt_with_TKIP_MIC(VOID *pReserved, struct sk_buff *pPacket)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);
	if (skb_tailroom(skb) < TKIP_TX_MIC_SIZE) {
		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, skb_headroom(skb), TKIP_TX_MIC_SIZE, GFP_ATOMIC);

		dev_kfree_skb_any(skb);

		if (newskb == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, ("Extend Tx.MIC for packet failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
	}

	return skb;


}

#ifdef CONFIG_AP_SUPPORT
struct sk_buff *duplicate_pkt_with_VLAN(
	IN struct net_device *pNetDev,
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN u8 *pHeader802_3,
	IN UINT HdrLen,
	IN u8 *pData,
	IN ULONG DataSize,
	IN UCHAR FromWhichBSSID,
	IN UCHAR *TPID)
{
	struct sk_buff *skb;
	struct sk_buff *pPacket = NULL;
	uint16_t VLAN_Size;

	if ((skb = __dev_alloc_skb(HdrLen + DataSize + LENGTH_802_1Q + 2,
				   MEM_ALLOC_FLAG)) != NULL) {

		skb_reserve(skb, 2);

		/* copy header (maybe +VLAN tag) */
		VLAN_Size = VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
						  pHeader802_3, HdrLen,
						  skb_tail_pointer(skb), FromWhichBSSID,
						  TPID);
		skb_put(skb, HdrLen + VLAN_Size);

		/* copy data body */
		memmove(skb_tail_pointer(skb), pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = pNetDev;	/*get_netdev_from_bssid(pAd, FromWhichBSSID); */
		pPacket = skb;
	}

	return pPacket;
}
#endif /* CONFIG_AP_SUPPORT */

/*
	========================================================================

	Routine Description:
		Send a L2 frame to upper daemon to trigger state machine

	Arguments:
		pAd - pointer to our pAdapter context

	Return Value:

	Note:

	========================================================================
*/
BOOLEAN RTMPL2FrameTxAction(
	IN struct rtmp_adapter *pAd,
	IN struct net_device *pNetDev,
	IN RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet,
	IN UCHAR apidx,
	IN UCHAR *pData,
	IN uint32_t data_len,
	IN UCHAR OpMode)
{
	struct sk_buff *skb = dev_alloc_skb(data_len + 2);

	if (!skb) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s : Error! Can't allocate a skb.\n", __FUNCTION__));
		return FALSE;
	}

	skb->dev = pNetDev;

	/* 16 byte align the IP header */
	skb_reserve(skb, 2);

	/* Insert the frame content */
	memmove(GET_OS_PKT_DATAPTR(skb), pData, data_len);

	/* End this frame */
	skb_put(GET_OS_PKT_TYPE(skb), data_len);

	DBGPRINT(RT_DEBUG_TRACE, ("%s doen\n", __FUNCTION__));

	_announce_802_3_packet(pAd, skb, OpMode);

	return TRUE;

}


struct sk_buff *ExpandPacket(
	IN VOID *pReserved,
	IN struct sk_buff *pPacket,
	IN uint32_t ext_head_len,
	IN uint32_t ext_tail_len)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);
	if (skb_cloned(skb) || (skb_headroom(skb) < ext_head_len)
	    || (skb_tailroom(skb) < ext_tail_len)) {
		uint32_t head_len =
		    (skb_headroom(skb) <
		     ext_head_len) ? ext_head_len : skb_headroom(skb);
		uint32_t tail_len =
		    (skb_tailroom(skb) <
		     ext_tail_len) ? ext_tail_len : skb_tailroom(skb);

		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, head_len, tail_len, GFP_ATOMIC);

		dev_kfree_skb_any(skb);

		if (newskb == NULL) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("Extend Tx buffer for WPI failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
	}

	return skb;

}

struct sk_buff *ClonePacket(struct sk_buff *skb, u8 *pData,
			    ULONG DataSize)
{
	struct sk_buff *clone;


	/* clone the packet */
	clone = skb_clone(skb, MEM_ALLOC_FLAG);

	if (clone) {
		/* set the correct dataptr and data len */
		clone->dev = skb->dev;
		clone->data = pData;
		clone->len = DataSize;
		skb_set_tail_pointer(clone, DataSize);
		ASSERT(DataSize < 1530);
	}
	return clone;
}

VOID RtmpOsPktInit(struct sk_buff *skb, struct net_device *pNetDev,
		   UCHAR *pData, USHORT DataSize)
{
	skb->dev =  pNetDev;
	skb->data = pData;
	skb->len = DataSize;
	skb_set_tail_pointer(skb, DataSize);
}


void wlan_802_11_to_802_3_packet(
	IN struct net_device *pNetDev,
	IN UCHAR OpMode,
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN struct sk_buff *pRxPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN u8 *pHeader802_3,
	IN UCHAR FromWhichBSSID,
	IN UCHAR *TPID)
{
	struct sk_buff *pOSPkt;

	ASSERT(pHeader802_3);

	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

	pOSPkt->dev = pNetDev;
	pOSPkt->data = pData;
	pOSPkt->len = DataSize;
	skb_set_tail_pointer(pOSPkt, DataSize);

	/* copy 802.3 header */
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(OpMode)
	{
		/* maybe insert VLAN tag to the received packet */
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;

		if (VLAN_VID != 0)
			VLAN_Size = LENGTH_802_1Q;

		data_p = skb_push(pOSPkt, LENGTH_802_3 + VLAN_Size);

		VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
				      pHeader802_3, LENGTH_802_3,
				      data_p, FromWhichBSSID, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode)
	{
	    memmove(skb_push(pOSPkt, LENGTH_802_3), pHeader802_3, LENGTH_802_3);
	}
#endif /* CONFIG_STA_SUPPORT */

}


void hex_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
#ifdef DBG
	unsigned char *pt;
	int x;

	if (RTDebugLevel < RT_DEBUG_TRACE)
		return;

	pt = pSrcBufVA;
	printk("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			printk("0x%04x : ", x);
		printk("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			printk("\n");
	}
	printk("\n");
#endif /* DBG */
}

#ifdef SYSTEM_LOG_SUPPORT
/*
	========================================================================

	Routine Description:
		Send log message through wireless event

		Support standard iw_event with IWEVCUSTOM. It is used below.

		iwreq_data.data.flags is used to store event_flag that is
		defined by user. iwreq_data.data.length is the length of the
		event log.

		The format of the event log is composed of the entry's MAC
		address and the desired log message (refer to
		pWirelessEventText).

			ex: 11:22:33:44:55:66 has associated successfully

		p.s. The requirement of Wireless Extension is v15 or newer.

	========================================================================
*/
VOID RtmpOsSendWirelessEvent(
	IN struct rtmp_adapter *pAd,
	IN USHORT Event_flag,
	IN u8 *pAddr,
	IN UCHAR BssIdx,
	IN CHAR Rssi,
	IN RTMP_OS_SEND_WLAN_EVENT pFunc)
{
#if WIRELESS_EXT >= 15
	pFunc(pAd, Event_flag, pAddr, BssIdx, Rssi);
#else
	DBGPRINT(RT_DEBUG_ERROR,
		 ("%s : The Wireless Extension MUST be v15 or newer.\n",
		  __FUNCTION__));
#endif /* WIRELESS_EXT >= 15 */
}
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID SendSignalToDaemon(
	IN INT sig,
	RTMP_OS_PID pid,
	unsigned long pid_no)
{
}
#endif /* CONFIG_AP_SUPPORT */

/*******************************************************************************

	Task create/management/kill related functions.

 *******************************************************************************/
static inline int __RtmpOSTaskKill(OS_TASK *pTask)
{
	int ret = NDIS_STATUS_FAILURE;

#ifdef KTHREAD_SUPPORT
	if (pTask->kthread_task) {
		kthread_stop(pTask->kthread_task);
		ret = NDIS_STATUS_SUCCESS;
	}
#else
	CHECK_PID_LEGALITY(pTask->taskPID) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("Terminate the task(%s) with pid(%d)!\n",
			  pTask->taskName, GET_PID_NUMBER(pTask->taskPID)));
		mb();
		pTask->task_killed = 1;
		mb();
		ret = KILL_THREAD_PID(pTask->taskPID, SIGTERM, 1);
		if (ret) {
			printk(KERN_WARNING
			       "kill task(%s) with pid(%d) failed(retVal=%d)!\n",
			       pTask->taskName, GET_PID_NUMBER(pTask->taskPID),
			       ret);
		} else {
			wait_for_completion(&pTask->taskComplete);
			pTask->taskPID = THREAD_PID_INIT_VALUE;
			pTask->task_killed = 0;
			RTMP_SEM_EVENT_DESTORY(&pTask->taskSema);
			ret = NDIS_STATUS_SUCCESS;
		}
	}
#endif

	return ret;

}


static inline INT __RtmpOSTaskNotifyToExit(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	complete_and_exit(&pTask->taskComplete, 0);
#endif

	return 0;
}


static inline void __RtmpOSTaskCustomize(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT

	daemonize((char *) & pTask->taskName[0] /*"%s",pAd->net_dev->name */ );

	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	current->flags |= PF_NOFREEZE;

	RTMP_GET_OS_PID(pTask->taskPID, current->pid);

	/* signal that we've started the thread */
	complete(&pTask->taskComplete);

#endif
}


static inline int __RtmpOSTaskAttach(
	IN OS_TASK *pTask,
	IN RTMP_OS_TASK_CALLBACK fn,
	IN ULONG arg)
{
	int status = NDIS_STATUS_SUCCESS;
#ifndef KTHREAD_SUPPORT
	pid_t pid_number = -1;
#endif /* KTHREAD_SUPPORT */

#ifdef KTHREAD_SUPPORT
	pTask->task_killed = 0;
	pTask->kthread_task = NULL;
	pTask->kthread_task =
	    kthread_run((cast_fn) fn, (void *)arg, pTask->taskName);
	if (IS_ERR(pTask->kthread_task))
		status = NDIS_STATUS_FAILURE;
#else
	pid_number =
	    kernel_thread((cast_fn) fn, (void *)arg, RTMP_OS_MGMT_TASK_FLAGS);
	if (pid_number < 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("Attach task(%s) failed!\n", pTask->taskName));
		status = NDIS_STATUS_FAILURE;
	} else {
		/* Wait for the thread to start */
		wait_for_completion(&pTask->taskComplete);
		status = NDIS_STATUS_SUCCESS;
	}
#endif
	return status;
}


static inline int __RtmpOSTaskInit(
	IN OS_TASK *pTask,
	IN char *pTaskName,
	IN VOID *pPriv,
	IN LIST_HEADER *pSemList)
{
	int len;

	ASSERT(pTask);

#ifndef KTHREAD_SUPPORT
	memset((u8 *) (pTask), 0, sizeof (OS_TASK));
#endif

	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN - 1) ? (RTMP_OS_TASK_NAME_LEN - 1) : len;
	memmove(&pTask->taskName[0], pTaskName, len);
	pTask->priv = pPriv;

#ifndef KTHREAD_SUPPORT
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema), pSemList);
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	init_completion(&pTask->taskComplete);
#endif

#ifdef KTHREAD_SUPPORT
	init_waitqueue_head(&(pTask->kthread_q));
#endif /* KTHREAD_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


BOOLEAN __RtmpOSTaskWait(
	IN VOID *pReserved,
	IN OS_TASK *pTask,
	IN int32_t *pStatus)
{
#ifdef KTHREAD_SUPPORT
	RTMP_WAIT_EVENT_INTERRUPTIBLE((*pStatus), pTask);

	if ((pTask->task_killed == 1) || ((*pStatus) != 0))
		return FALSE;
#else

	RTMP_SEM_EVENT_WAIT(&(pTask->taskSema), (*pStatus));

	/* unlock the device pointers */
	if ((*pStatus) != 0) {
/*		RTMP_SET_FLAG_(*pFlags, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
		return FALSE;
	}
#endif /* KTHREAD_SUPPORT */

	return TRUE;
}

static uint32_t RtmpOSWirelessEventTranslate(IN uint32_t eventType)
{
	switch (eventType) {
	case RT_WLAN_EVENT_CUSTOM:
		eventType = IWEVCUSTOM;
		break;

	case RT_WLAN_EVENT_CGIWAP:
		eventType = SIOCGIWAP;
		break;

#if WIRELESS_EXT > 17
	case RT_WLAN_EVENT_ASSOC_REQ_IE:
		eventType = IWEVASSOCREQIE;
		break;
#endif /* WIRELESS_EXT */

#if WIRELESS_EXT >= 14
	case RT_WLAN_EVENT_SCAN:
		eventType = SIOCGIWSCAN;
		break;
#endif /* WIRELESS_EXT */

	case RT_WLAN_EVENT_EXPIRED:
		eventType = IWEVEXPIRED;
		break;

	default:
		printk("Unknown event: 0x%x\n", eventType);
		break;
	}

	return eventType;
}


int RtmpOSWrielessEventSend(
	IN struct net_device *pNetDev,
	IN uint32_t eventType,
	IN INT flags,
	IN u8 *pSrcMac,
	IN u8 *pData,
	IN uint32_t dataLen)
{
	union iwreq_data wrqu;

	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);

	memset(&wrqu, 0, sizeof (wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;
	else
		wrqu.data.length = 0;

	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
	return 0;
}


int RtmpOSWrielessEventSendExt(
	IN struct net_device *pNetDev,
	IN uint32_t eventType,
	IN INT flags,
	IN u8 *pSrcMac,
	IN u8 *pData,
	IN uint32_t dataLen,
	IN uint32_t family)
{
	union iwreq_data wrqu;

	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);

	/* translate event type */
	memset(&wrqu, 0, sizeof (wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;

	wrqu.addr.sa_family = family;

	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
	return 0;
}


/*
========================================================================
Routine Description:
	Check if the network interface is up.

Arguments:
	*pDev			- Network Interface

Return Value:
	None

Note:
========================================================================
*/
BOOLEAN RtmpOSNetDevIsUp(struct net_device *pNetDev)
{
	if ((pNetDev == NULL) || !(pNetDev->flags & IFF_UP))
		return FALSE;

	return TRUE;
}


/*
========================================================================
Routine Description:
	Assign sys_handle data pointer (pAd) to the priv info structured linked to
	the OS network interface.

Arguments:
	pDev			- the os net device data structure
	pPriv			- the sys_handle want to assigned

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsSetNetDevPriv(struct net_device *pDev, struct rtmp_adapter *pPriv)
{
	struct mt_dev_priv *priv_info = NULL;

	priv_info = (struct mt_dev_priv *)netdev_priv(pDev);

	priv_info->sys_handle = pPriv;
	priv_info->priv_flags = 0;
}


/*
========================================================================
Routine Description:
	Get rtmp_wifi_dev from the priv info linked to OS network interface data structure.

Arguments:
	pDev			- the device

Return Value:
	sys_handle

Note:
========================================================================
*/
struct rtmp_adapter *RtmpOsGetNetDevPriv(struct net_device *pDev)
{
	struct mt_dev_priv *priv_info= netdev_priv(pDev);

	return priv_info->sys_handle;
}


VOID RtmpOsSetNetDevWdev(struct net_device *pDev, struct rtmp_wifi_dev *wdev)
{
	struct mt_dev_priv *priv_info= netdev_priv(pDev);

	priv_info->rtmp_wifi_dev = wdev;
}


struct rtmp_wifi_dev *RtmpOsGetNetDevWdev(struct net_device *pDev)
{
	struct mt_dev_priv *priv_info = netdev_priv(pDev);

	return priv_info->rtmp_wifi_dev;
}


/*
========================================================================
Routine Description:
	Get private flags from the network interface.

Arguments:
	pDev			- the device

Return Value:
	pPriv			- the pointer

Note:
========================================================================
*/
USHORT RtmpDevPrivFlagsGet(struct net_device *pDev)
{
	struct mt_dev_priv *priv_info = netdev_priv(pDev);

	return priv_info->priv_flags;
}


/*
========================================================================
Routine Description:
	Get private flags from the network interface.

Arguments:
	pDev			- the device

Return Value:
	pPriv			- the pointer

Note:
========================================================================
*/
VOID RtmpDevPrivFlagsSet(struct net_device *pDev, USHORT PrivFlags)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv(pDev);
	priv_info->priv_flags = PrivFlags;
}

UCHAR get_sniffer_mode(struct net_device *pDev)
{
	struct mt_dev_priv *priv_info = netdev_priv(pDev);

	return priv_info->sniffer_mode;
}

VOID set_sniffer_mode(struct net_device *pDev, UCHAR sniffer_mode)
{
	struct mt_dev_priv *priv_info = netdev_priv(pDev);

	priv_info->sniffer_mode = sniffer_mode;
}

/*
========================================================================
Routine Description:
	Get network interface name.

Arguments:
	pDev			- the device

Return Value:
	the name

Note:
========================================================================
*/
char *RtmpOsGetNetDevName(struct net_device *pDev)
{
	return pDev->name;
}


uint32_t RtmpOsGetNetIfIndex(struct net_device*pDev)
{
	return pDev->ifindex;
}


int RtmpOSNetDevAddrSet(
	IN UCHAR OpMode,
	IN struct net_device *net_dev,
	IN u8 *pMacAddr,
	IN u8 *dev_name)
{
#ifdef CONFIG_STA_SUPPORT
	/* work-around for the SuSE due to it has it's own interface name management system. */
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode) {
		if (dev_name != NULL) {
			memset(dev_name, 0, 16);
			memmove(dev_name, net_dev->name, strlen(net_dev->name));
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	memmove(net_dev->dev_addr, pMacAddr, 6);

	return 0;
}


/*
  *	Assign the network dev name for created Ralink WiFi interface.
  */
static int RtmpOSNetDevRequestName(
	IN int32_t MC_RowID,
	IN uint32_t *pIoctlIF,
	IN struct net_device *dev,
	IN char *pPrefixStr,
	IN INT devIdx)
{
	struct net_device *existNetDev;
	STRING suffixName[IFNAMSIZ];
	STRING desiredName[IFNAMSIZ];
	int ifNameIdx,
	 prefixLen,
	 slotNameLen;
	int Status;

	prefixLen = strlen(pPrefixStr);
	ASSERT((prefixLen < IFNAMSIZ));

	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++) {
		memset(suffixName, 0, IFNAMSIZ);
		memset(desiredName, 0, IFNAMSIZ);
		strncpy(&desiredName[0], pPrefixStr, prefixLen);

		sprintf(suffixName, "%d", ifNameIdx);

		slotNameLen = strlen(suffixName);
		ASSERT(((slotNameLen + prefixLen) < IFNAMSIZ));
		strcat(desiredName, suffixName);

		existNetDev = RtmpOSNetDevGetByName(dev, &desiredName[0]);
		if (existNetDev == NULL)
			break;
		else
			RtmpOSNetDeviceRefPut(existNetDev);
	}

	if (ifNameIdx < 32) {
#ifdef HOSTAPD_SUPPORT
		*pIoctlIF = ifNameIdx;
#endif /*HOSTAPD_SUPPORT */
		strcpy(&dev->name[0], &desiredName[0]);
		Status = NDIS_STATUS_SUCCESS;
	} else {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n",
			  pPrefixStr));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}

void RtmpOSNetDevClose(struct net_device *pNetDev)
{
	dev_close(pNetDev);
}

void RtmpOSNetDevFree(struct net_device *pNetDev)
{
	ASSERT(pNetDev);

	free_netdev(pNetDev);

#ifdef VENDOR_FEATURE4_SUPPORT
	printk("OS_NumOfMemAlloc = %ld, OS_NumOfMemFree = %ld\n",
			OS_NumOfMemAlloc, OS_NumOfMemFree);
#endif /* VENDOR_FEATURE4_SUPPORT */
}

INT RtmpOSNetDevAlloc(
	IN struct net_device **new_dev_p,
	IN uint32_t privDataSize)
{
	*new_dev_p = NULL;

	DBGPRINT(RT_DEBUG_TRACE,
		 ("Allocate a net device with private data size=%d!\n",
		  privDataSize));
	*new_dev_p = alloc_etherdev(privDataSize);

	if (*new_dev_p)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


INT RtmpOSNetDevOpsAlloc(PVOID *pNetDevOps)
{
	*pNetDevOps = (PVOID) vmalloc(sizeof (struct net_device_ops));
	if (*pNetDevOps) {
		memset(*pNetDevOps, 0, sizeof (struct net_device_ops));
		return NDIS_STATUS_SUCCESS;
	} else {
		return NDIS_STATUS_FAILURE;
	}
}

struct net_device *RtmpOSNetDevGetByName(struct net_device *pNetDev, char *pDevName)
{
	struct net_device *pTargetNetDev = NULL;

	pTargetNetDev = dev_get_by_name(dev_net(pNetDev), pDevName);

	return pTargetNetDev;
}


void RtmpOSNetDeviceRefPut(struct net_device *pNetDev)
{
	/*
	   every time dev_get_by_name is called, and it has returned a valid struct
	   net_device*, dev_put should be called afterwards, because otherwise the
	   machine hangs when the device is unregistered (since dev->refcnt > 1).
	 */
	if (pNetDev)
		dev_put(pNetDev);
}


INT RtmpOSNetDevDestory(VOID *pReserved, struct net_device *pNetDev)
{

	/* TODO: Need to fix this */
	printk("WARNING: This function(%s) not implement yet!!!\n",
	       __FUNCTION__);
	return 0;
}


void RtmpOSNetDevDetach(struct net_device *pNetDev)
{
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;

	unregister_netdevice(pNetDev);

	vfree(pNetDevOps);
}


void RtmpOSNetDevProtect(BOOLEAN lock_it)
{
	if (lock_it)
		rtnl_lock();
	else
		rtnl_unlock();

}

static void RALINK_ET_DrvInfoGet(
	struct net_device *pDev,
	struct ethtool_drvinfo *pInfo)
{
	strcpy(pInfo->driver, "RALINK WLAN");


	sprintf(pInfo->bus_info, "CSR 0x%lx", pDev->base_addr);
}

static struct ethtool_ops RALINK_Ethtool_Ops = {
	.get_drvinfo = RALINK_ET_DrvInfoGet,
};

int RtmpOSNetDevAttach(UCHAR OpMode, struct net_device *pNetDev,
		       struct RTMP_OS_NETDEV_OP_HOOK *pDevOpHook)
{
	int ret,
	 rtnl_locked = FALSE;

	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;

	DBGPRINT(RT_DEBUG_TRACE, ("RtmpOSNetDevAttach()--->\n"));

	/* If we need hook some callback function to the net device structrue, now do it. */
	if (pDevOpHook) {
		pNetDevOps->ndo_open = pDevOpHook->open;
		pNetDevOps->ndo_stop = pDevOpHook->stop;
		pNetDevOps->ndo_start_xmit =
		    (HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDevOps->ndo_do_ioctl = pDevOpHook->ioctl;
		pNetDev->ethtool_ops = &RALINK_Ethtool_Ops;

		/* if you don't implement get_stats, just leave the callback function as NULL, a dummy
		   function will make kernel panic.
		 */
		if (pDevOpHook->get_stats)
			pNetDevOps->ndo_get_stats = pDevOpHook->get_stats;

		/* OS specific flags, here we used to indicate if we are virtual interface */
/*		pNetDev->priv_flags = pDevOpHook->priv_flags; */
		RT_DEV_PRIV_FLAGS_SET(pNetDev, pDevOpHook->priv_flags);

#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
/*		pNetDev->get_wireless_stats = rt28xx_get_wireless_stats; */
		pNetDev->get_wireless_stats = pDevOpHook->get_wstats;
#endif

#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
		if (OpMode == OPMODE_STA) {
/*			pNetDev->wireless_handlers = &rt28xx_iw_handler_def; */
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;
		}
#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
		if (OpMode == OPMODE_AP) {
/*			pNetDev->wireless_handlers = &rt28xx_ap_iw_handler_def; */
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;
		}
#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

		/* copy the net device mac address to the net_device structure. */
		memmove(pNetDev->dev_addr, &pDevOpHook->devAddr[0],
			       MAC_ADDR_LEN);

		rtnl_locked = pDevOpHook->needProtcted;
	}

	pNetDevOps->ndo_validate_addr = NULL;
	/*pNetDev->netdev_ops = ops; */

	if (rtnl_locked)
		ret = register_netdevice(pNetDev);
	else
		ret = register_netdev(pNetDev);

	netif_stop_queue(pNetDev);

	DBGPRINT(RT_DEBUG_TRACE, ("<---RtmpOSNetDevAttach(), ret=%d\n", ret));
	if (ret == 0)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


struct net_device *RtmpOSNetDevCreate(
	IN int32_t MC_RowID,
	IN uint32_t *pIoctlIF,
	IN INT devType,
	IN INT devNum,
	IN INT privMemSize,
	IN char *pNamePrefix)
{
	struct net_device *pNetDev = NULL;
	struct net_device_ops *pNetDevOps = NULL;
	int status;

	/* allocate a new network device */
	status = RtmpOSNetDevAlloc(&pNetDev, privMemSize);
	if (status != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (%s)...\n", pNamePrefix));
		return NULL;
	}
	status = RtmpOSNetDevOpsAlloc((PVOID) & pNetDevOps);

	if (status != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate net device ops fail!\n"));
		RtmpOSNetDevFree(pNetDev);

		return NULL;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate net device ops success!\n"));
		pNetDev->netdev_ops = pNetDevOps;
	}

	/* find a available interface name, max 32 interfaces */
	status = RtmpOSNetDevRequestName(MC_RowID, pIoctlIF, pNetDev, pNamePrefix, devNum);
	if (status != NDIS_STATUS_SUCCESS) {
		/* error! no any available ra name can be used! */
		DBGPRINT(RT_DEBUG_ERROR,
					("Assign inf name (%s with suffix 0~32) failed\n", pNamePrefix));
		RtmpOSNetDevFree(pNetDev);

		return NULL;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("The name of the new %s interface is %s\n",
			  pNamePrefix, pNetDev->name));
	}

	return pNetDev;
}


#ifdef CONFIG_AP_SUPPORT
UCHAR VLAN_8023_Header_Copy(
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN u8 *pHeader802_3,
	IN UINT HdrLen,
	OUT u8 *pData,
	IN UCHAR FromWhichBSSID,
	IN UCHAR *TPID)
{
	uint16_t TCI;
	UCHAR VLAN_Size = 0;

	if (VLAN_VID != 0) {
		/* need to insert VLAN tag */
		VLAN_Size = LENGTH_802_1Q;

		/* make up TCI field */
		TCI = (VLAN_VID & 0x0fff) | ((VLAN_Priority & 0x7) << 13);

#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif /* RT_BIG_ENDIAN */

		/* copy dst + src MAC (12B) */
		memcpy(pData, pHeader802_3, LENGTH_802_3_NO_TYPE);

		/* copy VLAN tag (4B) */
		/* do NOT use memcpy to speed up */
		*(uint16_t *) (pData + LENGTH_802_3_NO_TYPE) = *(uint16_t *) TPID;
		*(uint16_t *) (pData + LENGTH_802_3_NO_TYPE + 2) = TCI;

		/* copy type/len (2B) */
		*(uint16_t *) (pData + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q) =
		    *(uint16_t *) & pHeader802_3[LENGTH_802_3 -
					       LENGTH_802_3_TYPE];

		/* copy tail if exist */
		if (HdrLen > LENGTH_802_3)
			memcpy(pData + LENGTH_802_3 + LENGTH_802_1Q, pHeader802_3 + LENGTH_802_3, HdrLen - LENGTH_802_3);
	}
	else
	{
		/* no VLAN tag is needed to insert */
		memcpy(pData, pHeader802_3, HdrLen);
	}

	return VLAN_Size;
}
#endif /* CONFIG_AP_SUPPORT */


/*
========================================================================
Routine Description:
    Allocate memory for adapter control block.

Arguments:
    pAd					Pointer to our adapter

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE
	NDIS_STATUS_RESOURCES

Note:
========================================================================
*/
int AdapterBlockAllocateMemory(struct rtmp_adapter **ppAd, uint32_t SizeOfpAd)
{
	*ppAd = vmalloc(SizeOfpAd);
	if (*ppAd) {
		memset(*ppAd, 0, SizeOfpAd);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}


/* ========================================================================== */

UINT RtmpOsWirelessExtVerGet(VOID)
{
	return WIRELESS_EXT;
}


VOID RtmpDrvAllMacPrint(
	IN VOID *pReserved,
	IN uint32_t *pBufMac,
	IN uint32_t AddrStart,
	IN uint32_t AddrEnd,
	IN uint32_t AddrStep)
{
	struct file *file_w;
	char *fileName = "MacDump.txt";
	mm_segment_t orig_fs;
	STRING *msg;
	uint32_t macAddr = 0, macValue = 0;

	msg = kmalloc(1024, GFP_ATOMIC);
	if (!msg)
		return;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(file_w)) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("-->2) %s: Error %ld opening %s\n", __FUNCTION__,
			  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op && file_w->f_op->write) {
			file_w->f_pos = 0;
			macAddr = AddrStart;

			while (macAddr <= AddrEnd) {
/*				mt7612u_read32(pAd, macAddr, &macValue); // sample */
				macValue = *pBufMac++;
				sprintf(msg, "%04x = %08x\n", macAddr, macValue);

				/* write data to file */
				file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);

				printk("%s", msg);
				macAddr += AddrStep;
			}
			sprintf(msg, "\nDump all MAC values to %s\n", fileName);
		}
		filp_close(file_w, NULL);
	}
	set_fs(orig_fs);
	kfree(msg);
}


VOID RtmpDrvAllE2PPrint(
	IN VOID *pReserved,
	IN USHORT *pMacContent,
	IN uint32_t AddrEnd,
	IN uint32_t AddrStep)
{
	struct file *file_w;
	char *fileName = "EEPROMDump.txt";
	mm_segment_t orig_fs;
	STRING *msg;
	USHORT eepAddr = 0;
	USHORT eepValue;

	msg = kmalloc(1024, GFP_ATOMIC);
	if (!msg)
		return;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(file_w)) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("-->2) %s: Error %ld opening %s\n", __FUNCTION__,
			  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op && file_w->f_op->write) {
			file_w->f_pos = 0;
			eepAddr = 0x00;

			while (eepAddr <= AddrEnd) {
				eepValue = *pMacContent;
				sprintf(msg, "%08x = %04x\n", eepAddr, eepValue);

				/* write data to file */
				file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);

				printk("%s", msg);
				eepAddr += AddrStep;
				pMacContent += (AddrStep >> 1);
			}
			sprintf(msg, "\nDump all EEPROM values to %s\n",
				fileName);
		}
		filp_close(file_w, NULL);
	}
	set_fs(orig_fs);
	kfree(msg);
}


VOID RtmpDrvAllRFPrint(
	IN VOID *pReserved,
	IN UCHAR *pBuf,
	IN uint32_t BufLen)
{
	struct file *file_w;
	char *fileName = "RFDump.txt";
	mm_segment_t orig_fs;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(file_w)) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("-->2) %s: Error %ld opening %s\n", __FUNCTION__,
			  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op && file_w->f_op->write) {
			file_w->f_pos = 0;
			/* write data to file */
			file_w->f_op->write(file_w, pBuf, BufLen, &file_w->f_pos);
		}
		filp_close(file_w, NULL);
	}
	set_fs(orig_fs);
}


/*
========================================================================
Routine Description:
	Wake up the command thread.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pCmdQTask);
#ifdef KTHREAD_SUPPORT
	pTask->kthread_running = TRUE;
	wake_up(&pTask->kthread_q);
#else
	CHECK_PID_LEGALITY(pTask->taskPID) {
		RTMP_SEM_EVENT_UP(&(pTask->taskSema));
	}
#endif /* KTHREAD_SUPPORT */
}


/*
========================================================================
Routine Description:
	Wake up USB Mlme thread.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsMlmeUp(IN RTMP_OS_TASK *pMlmeQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pMlmeQTask);

#ifdef KTHREAD_SUPPORT
	if ((pTask != NULL) && (pTask->kthread_task)) {
		pTask->kthread_running = TRUE;
		wake_up(&pTask->kthread_q);
	}
#else
	if (pTask != NULL) {
		CHECK_PID_LEGALITY(pTask->taskPID) {
			RTMP_SEM_EVENT_UP(&(pTask->taskSema));
		}
	}
#endif /* KTHREAD_SUPPORT */
}


/*
========================================================================
Routine Description:
	Check if the file is error.

Arguments:
	pFile			- the file

Return Value:
	OK or any error

Note:
	rt_linux.h, not rt_drv.h
========================================================================
*/
int32_t RtmpOsFileIsErr(IN VOID *pFile)
{
	return IS_FILE_OPEN_ERR(pFile);
}

int RtmpOSIRQRelease(
	IN struct net_device *pNetDev,
	IN uint32_t infType,
	IN PPCI_DEV pci_dev,
	IN BOOLEAN *pHaveMsi)
{
	struct net_device *net_dev = (struct net_device *)pNetDev;



	return 0;
}


/*
========================================================================
Routine Description:
	Enable or disable wireless event sent.

Arguments:
	pReserved		- Reserved
	FlgIsWEntSup	- TRUE or FALSE

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsWlanEventSet(
	IN VOID *pReserved,
	IN BOOLEAN *pCfgWEnt,
	IN BOOLEAN FlgIsWEntSup)
{
#if WIRELESS_EXT >= 15
/*	pAd->CommonCfg.bWirelessEvent = FlgIsWEntSup; */
	*pCfgWEnt = FlgIsWEntSup;
#else
	*pCfgWEnt = 0;		/* disable */
#endif
}

/*
========================================================================
Routine Description:
	vmalloc

Arguments:
	Size			- memory size

Return Value:
	the memory

Note:
========================================================================
*/
VOID *RtmpOsVmalloc(ULONG Size)
{
	return vmalloc(Size);
}

/*
========================================================================
Routine Description:
	vfree

Arguments:
	pMem			- the memory

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsVfree(VOID *pMem)
{
	if (pMem != NULL)
		vfree(pMem);
}


/*
========================================================================
Routine Description:
	Assign protocol to the packet.

Arguments:
	pPkt			- the packet

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsPktProtocolAssign(struct sk_buff *pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
}


BOOLEAN RtmpOsStatsAlloc(
	IN VOID **ppStats,
	IN VOID **ppIwStats)
{
	*ppStats = kmalloc(sizeof (struct net_device_stats), GFP_ATOMIC);
	if ((*ppStats) == NULL)
		return FALSE;
	memset((UCHAR *) *ppStats, 0, sizeof (struct net_device_stats));

#if WIRELESS_EXT >= 12
	*ppIwStats = kmalloc(sizeof (struct iw_statistics), GFP_ATOMIC);
	if ((*ppIwStats) == NULL) {
		kfree(*ppStats);
		return FALSE;
	}
	memset((UCHAR *)* ppIwStats, 0, sizeof (struct iw_statistics));
#endif

	return TRUE;
}


/*
========================================================================
Routine Description:
	Pass the received packet to OS.

Arguments:
	pPkt			- the packet

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsPktRcvHandle(struct sk_buff *pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);


	netif_rx(pRxPkt);
}


VOID RtmpOsTaskPidInit(RTMP_OS_PID *pPid)
{
	*pPid = THREAD_PID_INIT_VALUE;
}

/*
========================================================================
Routine Description:
	Get the network interface for the packet.

Arguments:
	pPkt			- the packet

Return Value:
	None

Note:
========================================================================
*/

VOID RtmpOsPktNatMagicTag(IN struct sk_buff *pNetPkt)
{
#if !defined(CONFIG_RA_NAT_NONE)
#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
	FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_WLAN;
#endif /* CONFIG_RA_HW_NAT || CONFIG_RA_HW_NAT_MODULE */
#endif /* CONFIG_RA_NAT_NONE */
}


VOID RtmpOsPktNatNone(IN struct sk_buff *pNetPkt)
{
#if defined(CONFIG_RA_NAT_NONE)
#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
	FOE_AI(((struct sk_buff *)pNetPkt)) = UN_HIT;
#endif /* CONFIG_RA_HW_NAT || CONFIG_RA_HW_NAT_MODULE */
#endif /* CONFIG_RA_NAT_NONE */
}


/*
========================================================================
Routine Description:
	Flush a data cache line.

Arguments:
	AddrStart		- the start address
	Size			- memory size

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsDCacheFlush(
	IN ULONG AddrStart,
	IN ULONG Size)
{
	RTMP_UTIL_DCACHE_FLUSH(AddrStart, Size);
}


#ifdef CONFIG_STA_SUPPORT
INT RtmpOSNotifyRawData(
	IN struct net_device *pNetDev,
	IN u8 *buff,
	IN INT len,
	IN ULONG type,
	IN USHORT protocol)
{
	struct sk_buff *skb = NULL;

	skb = dev_alloc_skb(len + 2);

	if (!skb)
	{
		DBGPRINT(RT_DEBUG_ERROR,( "%s: failed to allocate sk_buff for notification\n", pNetDev->name));
		return -ENOMEM;
	}
	else
	{
		skb_reserve(skb, 2);
		memcpy(skb_put(skb, len), buff, len);
		skb->len = len;
		skb->dev = pNetDev;
		skb_set_mac_header(skb, 0);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->pkt_type = PACKET_OTHERHOST;
		skb->protocol = htons(protocol);
		memset(skb->cb, 0, sizeof(skb->cb));

		netif_rx(skb);
	}
	return 0;
}

#endif /* CONFIG_STA_SUPPORT */


void OS_SPIN_LOCK_IRQSAVE(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	spin_lock_irqsave((spinlock_t *)(lock), *flags);
}

void OS_SPIN_UNLOCK_IRQRESTORE(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	spin_unlock_irqrestore((spinlock_t *)(lock), *flags);
}

void OS_SPIN_LOCK(NDIS_SPIN_LOCK *lock)
{
	spin_lock((spinlock_t *)(lock));
}

void OS_SPIN_UNLOCK(NDIS_SPIN_LOCK *lock)
{
	spin_unlock((spinlock_t *)(lock));
}

void OS_SPIN_LOCK_IRQ(NDIS_SPIN_LOCK *lock)
{
	spin_lock_irq((spinlock_t *)(lock));
}

void OS_SPIN_UNLOCK_IRQ(NDIS_SPIN_LOCK *lock)
{
	spin_unlock_irq((spinlock_t *)(lock));
}

int OS_TEST_BIT(int bit, unsigned long *flags)
{
	return test_bit(bit, flags);
}

void OS_SET_BIT(int bit, unsigned long *flags)
{
	set_bit(bit, flags);
}

void OS_CLEAR_BIT(int bit, unsigned long *flags)
{
	clear_bit(bit, flags);
}



/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_SetPeriodicTimer(pTimerOrg, timeout);
}


/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
VOID RTMP_OS_Init_Timer(
	VOID *pReserved,
	NDIS_MINIPORT_TIMER *pTimerOrg,
	TIMER_FUNCTION function,
	PVOID data,
	LIST_HEADER *pTimerList)
{
	__RTMP_OS_Init_Timer(pReserved, pTimerOrg, function, data);
}


VOID RTMP_OS_Add_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Add_Timer(pTimerOrg, timeout);
}


VOID RTMP_OS_Mod_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Mod_Timer(pTimerOrg, timeout);
}


VOID RTMP_OS_Del_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, BOOLEAN *pCancelled)
{
	__RTMP_OS_Del_Timer(pTimerOrg, pCancelled);
}


VOID RTMP_OS_Release_Timer(NDIS_MINIPORT_TIMER *pTimerOrg)
{
	__RTMP_OS_Release_Timer(pTimerOrg);
}


int RtmpOSTaskKill(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskKill(pTask);
}


INT RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskNotifyToExit(pTask);
}


void RtmpOSTaskCustomize(RTMP_OS_TASK *pTask)
{
	__RtmpOSTaskCustomize(pTask);
}


int RtmpOSTaskAttach(
	RTMP_OS_TASK *pTask,
	RTMP_OS_TASK_CALLBACK fn,
	ULONG arg)
{
	return __RtmpOSTaskAttach(pTask, fn, arg);
}


int RtmpOSTaskInit(
	RTMP_OS_TASK *pTask,
	char *pTaskName,
	VOID *pPriv,
	LIST_HEADER *pTaskList,
	LIST_HEADER *pSemList)
{
	return __RtmpOSTaskInit(pTask, pTaskName, pPriv, pSemList);
}


BOOLEAN RtmpOSTaskWait(VOID *pReserved, RTMP_OS_TASK * pTask, int32_t *pStatus)
{
	return __RtmpOSTaskWait(pReserved, pTask, pStatus);
}


VOID RtmpOsTaskWakeUp(RTMP_OS_TASK *pTask)
{
#ifdef KTHREAD_SUPPORT
	WAKE_UP(pTask);
#else
	RTMP_SEM_EVENT_UP(&pTask->taskSema);
#endif
}

