
#ifndef __NET_IF_BLOCK_H__
#define __NET_IF_BLOCK_H__

#include "link_list.h"
#include "rtmp.h"

#define FREE_NETIF_POOL_SIZE 32

typedef struct _NETIF_ENTRY
{
	struct _NETIF_ENTRY *pNext;
	struct net_device *pNetDev;
} NETIF_ENTRY, *PNETIF_ENTRY;

void initblockQueueTab(
	IN Pstruct rtmp_adapter pAd);

BOOLEAN blockNetIf(
	IN PBLOCK_QUEUE_ENTRY pBlockQueueEntry,
	IN struct net_device *pNetDev);

VOID releaseNetIf(
	IN PBLOCK_QUEUE_ENTRY pBlockQueueEntry);

VOID StopNetIfQueue(
	IN Pstruct rtmp_adapter pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket);
#endif /* __NET_IF_BLOCK_H__ */

