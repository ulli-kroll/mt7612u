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
    bg_ftph.c

    Abstract:
    Provide fast path between LAN and WLAN.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Sample Lin	01-22-2008    Created

 */

#include "rt_config.h"

#ifdef BG_FT_SUPPORT
#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
#include <linux/netfilter_bridge.h>
#include "../net/bridge/br_private.h"

/* extern export symbol in other drivers */
/*
	Example in other drivers:
		uint32_t (*RALINK_FP_Handle)(struct sk_buff *pPacket);
		EXPORT_SYMBOL(RALINK_FP_Handle);

	packet_forward()
	{
		uint32_t HandRst = 1;

		......

		if (RALINK_FP_Handle != NULL)
			HandRst = RALINK_FP_Handle(skb);

		if (HandRst != 0)
		{
			// pass the packet to upper layer
			skb->protocol = eth_type_trans(skb, skb->dev);
			netif_rx(skb);
		}
	}
*/
uint32_t BG_FTPH_PacketFromApHandle(struct sk_buff *pPacket);

#ifdef BG_FT_OPEN_SUPPORT
extern uint32_t (*RALINK_FP_Handle)(struct sk_buff *pPacket);
#else
uint32_t (*RALINK_FP_Handle)(struct sk_buff *pPacket);
#endif /* BG_FT_OPEN_SUPPORT */




/* --------------------------------- Public -------------------------------- */

/*
========================================================================
Routine Description:
	Init bridge fast path module.

Arguments:
	None

Return Value:
	None

Note:
	Used in module init.
========================================================================
*/
VOID BG_FTPH_Init(VOID)
{
	RALINK_FP_Handle = BG_FTPH_PacketFromApHandle;
}


/*
========================================================================
Routine Description:
	Remove bridge fast path module.

Arguments:
	None

Return Value:
	None

Note:
	Used in module remove.
========================================================================
*/
VOID BG_FTPH_Remove(VOID)
{
	RALINK_FP_Handle = NULL;
} /* End of BG_FTPH_Init */




/*
========================================================================
Routine Description:
	Forward the received packet.

Arguments:
	pPacket			- the received packet

Return Value:
	None

Note:
========================================================================
*/
uint32_t BG_FTPH_PacketFromApHandle(
	IN		struct sk_buff *pPacket)
{
	struct net_device	*pNetDev;
	struct sk_buff		*pRxPkt;
	struct net_bridge_fdb_entry *pSrcFdbEntry, *pDstFdbEntry;


	/* init */
	pRxPkt = RTPKT_TO_OSPKT(pPacket);
	pNetDev = pRxPkt->dev;

	/* if pNetDev is promisc mode ??? */
	DBGPRINT(RT_DEBUG_INFO, ("ft bg> BG_FTPH_PacketFromApHandle\n"));

	if (pNetDev != NULL)
	{
		if (pNetDev->br_port != NULL)
		{
			pDstFdbEntry = br_fdb_get_hook(pNetDev->br_port->br, pRxPkt->data);
			pSrcFdbEntry = br_fdb_get_hook(pNetDev->br_port->br, pRxPkt->data + 6);

			/* check destination address in bridge forwarding table */
			if ((pSrcFdbEntry == NULL) ||
				(pDstFdbEntry == NULL) ||
				(pDstFdbEntry->is_local) ||
				(pDstFdbEntry->dst == NULL) ||
				(pDstFdbEntry->dst->dev == NULL) ||
				(pDstFdbEntry->dst->dev == pNetDev) ||
				(pNetDev->br_port->state != BR_STATE_FORWARDING) ||
				((pSrcFdbEntry->dst != NULL) &&
					(pSrcFdbEntry->dst->dev != NULL) &&
					(pSrcFdbEntry->dst->dev != pNetDev)))
			{

				goto LabelPassToUpperLayer;
			} /* End of if */

			if ((!pDstFdbEntry->is_local) &&
				(pDstFdbEntry->dst != NULL) &&
				(pDstFdbEntry->dst->dev != NULL))
			{
				pRxPkt->dev = pDstFdbEntry->dst->dev;
				pDstFdbEntry->dst->dev->hard_start_xmit(pRxPkt, pDstFdbEntry->dst->dev);
				return 0;
			} /* End of if */
		} /* End of if */
	} /* End of if */

LabelPassToUpperLayer:
	DBGPRINT(RT_DEBUG_TRACE, ("ft bg> Pass packet to bridge module.\n"));
	return 1;
} /* End of BG_FTPH_PacketFromApHandle */


#endif /* CONFIG_BRIDGE || CONFIG_BRIDGE_MODULE */
#endif /* BG_FT_SUPPORT */

/* End of bg_ftph.c */
