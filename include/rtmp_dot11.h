/*

*/

#ifndef __DOT11_BASE_H__
#define __DOT11_BASE_H__

#include "rtmp_type.h"
#include "dot11_base.h"
#include "dot11i_wpa.h"

#include "dot11n_ht.h"

#ifdef DOT11_VHT_AC
#include "dot11ac_vht.h"
#endif /* DOT11_VHT_AC */

#ifdef TXBF_SUPPORT
/* CSI/Steering values */
#define DOT11N_BF_FB_NONE		0
#define DOT11N_BF_FB_CSI		1
#define DOT11N_BF_FB_NOCOMP	2
#define DOT11N_BF_FB_COMP		3
#endif /* TXBF_SUPPORT */

/* 4-byte HTC field.  maybe included in any frame except non-QOS data frame.  The Order bit must set 1. */
typedef struct GNU_PACKED _HT_CONTROL{
#ifdef RT_BIG_ENDIAN
	uint32_t RDG:1;
	uint32_t ACConstraint:1;
	uint32_t rsv2:5;
	uint32_t NDPAnnounce:1;
	uint32_t CSISTEERING:2;
	uint32_t rsv1:2;
	uint32_t CalSeq:2;
	uint32_t CalPos:2;
	uint32_t MFBorASC:7;
	uint32_t MFSI:3;
	uint32_t MSI:3;
	uint32_t MRQ:1;
	uint32_t TRQ:1;
	uint32_t vht:1;
#else
	uint32_t vht:1;		/* indicate for VHT variant or HT variant */
	uint32_t TRQ:1;		/*sounding request */
	uint32_t MRQ:1;		/*MCS feedback. Request for a MCS feedback */
	uint32_t MSI:3;		/*MCS Request, MRQ Sequence identifier */
	uint32_t MFSI:3;		/*SET to the received value of MRS. 0x111 for unsolicited MFB. */
	uint32_t MFBorASC:7;	/*Link adaptation feedback containing recommended MCS. 0x7f for no feedback or not available */
	uint32_t CalPos:2;	/* calibration position */
	uint32_t CalSeq:2;	/*calibration sequence */
	uint32_t rsv1:2;		/* Reserved */
	uint32_t CSISTEERING:2;	/*CSI/ STEERING */
	uint32_t NDPAnnounce:1;	/* ZLF announcement */
	uint32_t rsv2:5;		/*calibration sequence */
	uint32_t ACConstraint:1;	/*feedback request */
	uint32_t RDG:1;		/*RDG / More PPDU */
#endif				/* !RT_BIG_ENDIAN */
} HT_CONTROL, *PHT_CONTROL;

/* 2-byte QOS CONTROL field */
typedef struct GNU_PACKED _QOS_CONTROL{
#ifdef RT_BIG_ENDIAN
	USHORT Txop_QueueSize:8;
	USHORT AMsduPresent:1;
	USHORT AckPolicy:2;	/*0: normal ACK 1:No ACK 2:scheduled under MTBA/PSMP  3: BA */
	USHORT EOSP:1;
	USHORT TID:4;
#else
	USHORT TID:4;
	USHORT EOSP:1;
	USHORT AckPolicy:2;	/*0: normal ACK 1:No ACK 2:scheduled under MTBA/PSMP  3: BA */
	USHORT AMsduPresent:1;
	USHORT Txop_QueueSize:8;
#endif				/* !RT_BIG_ENDIAN */
} QOS_CONTROL, *PQOS_CONTROL;


typedef struct GNU_PACKED _AC_PARAM_RECORD{
	UINT8 aci_aifsn;
	UINT8 ecw_max:4;
	UINT8 ecw_min: 4;
	uint16_t txop_limit;
}AC_PARAM_RECORD;


typedef struct GNU_PACKED _PSPOLL_FRAME {
	FRAME_CONTROL FC;
	USHORT Aid;
	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR Ta[MAC_ADDR_LEN];
} PSPOLL_FRAME, *PPSPOLL_FRAME;


typedef struct GNU_PACKED _RTS_FRAME {
	FRAME_CONTROL FC;
	USHORT Duration;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
} RTS_FRAME, *PRTS_FRAME;

#endif /* __DOT11_BASE_H__ */

