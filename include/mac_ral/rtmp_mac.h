/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_mac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RTMP_MAC_H__
#define __RTMP_MAC_H__


#include "mac_ral/nmac/ral_nmac.h"

/*
	TX / RX ring descriptor format

	TX:
		PCI/RBUS_Descriptor + TXINFO + TXWI + 802.11

	Rx:
		PCI/RBUS/USB_Descripotr + (PCI/RBUS RXFCE_INFO) + (PCI/RBUS  RXINFO) + RXWI + 802.11 + (USB RXINFO)

*/

/* the first 24-byte in TXD is called TXINFO and will be DMAed to MAC block through TXFIFO. */
/* MAC block use this TXINFO to control the transmission behavior of this frame. */
#define FIFO_MGMT	0
#define FIFO_HCCA	1
#define FIFO_EDCA	2

typedef	union GNU_PACKED _TXWI_STRUC {
	struct _TXWI_NMAC TXWI_N;
	uint32_t word;
}TXWI_STRUC;


#define TXINFO_SIZE			4
typedef union GNU_PACKED _TXINFO_STRUC{
	struct _TXINFO_NMAC_PKT txinfo_nmac_pkt;
	uint32_t word;
}TXINFO_STRUC;


/*
	RXWI wireless information format, in PBF. invisible in driver.
*/
typedef union GNU_PACKED _RXWI_STRUC {
	struct _RXWI_NMAC RXWI_N;
}RXWI_STRUC;


typedef	union GNU_PACKED _HW_RATE_CTRL_STRUCT_ {
		struct _NMAC_HW_RATE_CTRL_STRUCT RATE_CTRL_N;
		uint16_t word;
}HW_RATE_CTRL_STRUCT;


/*
	bit31 =>802.3 if set 1, implay you hav did header translation
	bit30 => put VLAN field

*/
#define RXINFO_SIZE			4
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXINFO_STRUC {
	uint32_t 	hdr_trans_ip_sum_err:1;		/* IP checksum error */
	uint32_t 	vlan_taged_tcp_sum_err:1;	/* TCP checksum error */
	uint32_t 	rsv:1;
	uint32_t 	action_wanted:1;
	uint32_t 	deauth:1;
	uint32_t 	disasso:1;
	uint32_t 	beacon:1;
	uint32_t 	probe_rsp:1;
	uint32_t 	sw_fc_type1:1;
	uint32_t 	sw_fc_type0:1;
	uint32_t 	pn_len:3;
	uint32_t 	wapi_kidx:1;
	uint32_t 	BssIdx3:1;
	uint32_t 	Decrypted:1;
	uint32_t 	AMPDU:1;
	uint32_t 	L2PAD:1;
	uint32_t 	RSSI:1;
	uint32_t 	HTC:1;
	uint32_t 	AMSDU:1;		/* rx with 802.3 header, not 802.11 header. obsolete. */
	uint32_t 	CipherErr:2;        /* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	uint32_t 	Crc:1;			/* 1: CRC error */
	uint32_t 	MyBss:1;		/* 1: this frame belongs to the same BSSID */
	uint32_t 	Bcast:1;			/* 1: this is a broadcast frame */
	uint32_t 	Mcast:1;			/* 1: this is a multicast frame */
	uint32_t 	U2M:1;			/* 1: this RX frame is unicast to me */
	uint32_t 	FRAG:1;
	uint32_t 	NULLDATA:1;
	uint32_t 	DATA:1;
	uint32_t 	BA:1;
}	RXINFO_STRUC, *PRXINFO_STRUC;
#else
typedef	struct GNU_PACKED _RXINFO_STRUC {
	uint32_t 	BA:1;
	uint32_t 	DATA:1;
	uint32_t 	NULLDATA:1;
	uint32_t 	FRAG:1;
	uint32_t 	U2M:1;
	uint32_t 	Mcast:1;
	uint32_t 	Bcast:1;
	uint32_t 	MyBss:1;
	uint32_t 	Crc:1;
	uint32_t 	CipherErr:2;
	uint32_t 	AMSDU:1;
	uint32_t 	HTC:1;
	uint32_t 	RSSI:1;
	uint32_t 	L2PAD:1;
	uint32_t 	AMPDU:1;
	uint32_t 	Decrypted:1;
	uint32_t 	BssIdx3:1;
	uint32_t 	wapi_kidx:1;
	uint32_t 	pn_len:3;
	uint32_t 	sw_fc_type0:1;
	uint32_t      sw_fc_type1:1;
	uint32_t      probe_rsp:1;
	uint32_t 	beacon:1;
	uint32_t 	disasso:1;
	uint32_t      deauth:1;
	uint32_t      action_wanted:1;
	uint32_t      rsv:1;
	uint32_t 	vlan_taged_tcp_sum_err:1;
	uint32_t 	hdr_trans_ip_sum_err:1;
}RXINFO_STRUC, *PRXINFO_STRUC;
#endif


#define TSO_SIZE		0


/* ================================================================================= */
/* Register format */
/* ================================================================================= */

#define ASIC_VERSION		0x0000


/*
	SCH/DMA registers - base address 0x0200
*/
#define WMM_AIFSN_CFG   0x0214
#ifdef RT_BIG_ENDIAN
typedef	union _AIFSN_CSR_STRUC{
	struct {
	    uint32_t   Aifsn7:4;       /* for AC_VO */
	    uint32_t   Aifsn6:4;       /* for AC_VI */
	    uint32_t   Aifsn5:4;       /* for AC_BK */
	    uint32_t   Aifsn4:4;       /* for AC_BE */
	    uint32_t   Aifsn3:4;       /* for AC_VO */
	    uint32_t   Aifsn2:4;       /* for AC_VI */
	    uint32_t   Aifsn1:4;       /* for AC_BK */
	    uint32_t   Aifsn0:4;       /* for AC_BE */
	}field;
	uint32_t word;
} AIFSN_CSR_STRUC;
#else
typedef union _AIFSN_CSR_STRUC {
	struct {
	    uint32_t   Aifsn0:4;
	    uint32_t   Aifsn1:4;
	    uint32_t   Aifsn2:4;
	    uint32_t   Aifsn3:4;
	    uint32_t   Aifsn4:4;
	    uint32_t   Aifsn5:4;
	    uint32_t   Aifsn6:4;
	    uint32_t   Aifsn7:4;
	} field;
	uint32_t word;
} AIFSN_CSR_STRUC;
#endif

/* CWMIN_CSR: CWmin for each EDCA AC */
#define WMM_CWMIN_CFG   0x0218
#ifdef RT_BIG_ENDIAN
typedef	union _CWMIN_CSR_STRUC	{
	struct {
		uint32_t   Cwmin7:4;       /* for AC_VO */
		uint32_t   Cwmin6:4;       /* for AC_VI */
		uint32_t   Cwmin5:4;       /* for AC_BK */
		uint32_t   Cwmin4:4;       /* for AC_BE */
		uint32_t   Cwmin3:4;       /* for AC_VO */
		uint32_t   Cwmin2:4;       /* for AC_VI */
		uint32_t   Cwmin1:4;       /* for AC_BK */
		uint32_t   Cwmin0:4;       /* for AC_BE */
	} field;
	uint32_t word;
} CWMIN_CSR_STRUC;
#else
typedef	union _CWMIN_CSR_STRUC	{
	struct {
	    uint32_t   Cwmin0:4;
	    uint32_t   Cwmin1:4;
	    uint32_t   Cwmin2:4;
	    uint32_t   Cwmin3:4;
	    uint32_t   Cwmin4:4;
	    uint32_t   Cwmin5:4;
	    uint32_t   Cwmin6:4;
	    uint32_t   Cwmin7:4;
	} field;
	uint32_t word;
} CWMIN_CSR_STRUC;
#endif


/* CWMAX_CSR: CWmin for each EDCA AC */
#define WMM_CWMAX_CFG   0x021c
#ifdef RT_BIG_ENDIAN
typedef	union _CWMAX_CSR_STRUC {
	struct {
		uint32_t   Cwmax7:4;       /* for AC_VO */
		uint32_t   Cwmax6:4;       /* for AC_VI */
		uint32_t   Cwmax5:4;       /* for AC_BK */
		uint32_t   Cwmax4:4;       /* for AC_BE */
		uint32_t   Cwmax3:4;       /* for AC_VO */
		uint32_t   Cwmax2:4;       /* for AC_VI */
		uint32_t   Cwmax1:4;       /* for AC_BK */
		uint32_t   Cwmax0:4;       /* for AC_BE */
	} field;
	uint32_t word;
} CWMAX_CSR_STRUC;
#else
typedef	union _CWMAX_CSR_STRUC {
	struct {
	    uint32_t   Cwmax0:4;
	    uint32_t   Cwmax1:4;
	    uint32_t   Cwmax2:4;
	    uint32_t   Cwmax3:4;
	    uint32_t   Cwmax4:4;
	    uint32_t   Cwmax5:4;
	    uint32_t   Cwmax6:4;
	    uint32_t   Cwmax7:4;
	} field;
	uint32_t word;
}	CWMAX_CSR_STRUC;
#endif


/* AC_TXOP_CSR0: AC_BK/AC_BE TXOP register */
#define WMM_TXOP0_CFG    0x0220
#ifdef RT_BIG_ENDIAN
typedef	union _AC_TXOP_CSR0_STRUC {
	struct {
	    uint16_t  Ac1Txop; /* for AC_BE, in unit of 32us */
	    uint16_t  Ac0Txop; /* for AC_BK, in unit of 32us */
	} field;
	uint32_t word;
} AC_TXOP_CSR0_STRUC;
#else
typedef	union _AC_TXOP_CSR0_STRUC {
	struct {
	    uint16_t  Ac0Txop;
	    uint16_t  Ac1Txop;
	} field;
	uint32_t word;
} AC_TXOP_CSR0_STRUC;
#endif


/* AC_TXOP_CSR1: AC_VO/AC_VI TXOP register */
#define WMM_TXOP1_CFG    0x0224
#ifdef RT_BIG_ENDIAN
typedef	union _AC_TXOP_CSR1_STRUC {
	struct {
	    uint16_t  Ac3Txop; /* for AC_VO, in unit of 32us */
	    uint16_t  Ac2Txop; /* for AC_VI, in unit of 32us */
	} field;
	uint32_t word;
} AC_TXOP_CSR1_STRUC;
#else
typedef	union _AC_TXOP_CSR1_STRUC {
	struct {
	    uint16_t Ac2Txop;
	    uint16_t Ac3Txop;
	} field;
	uint32_t word;
} AC_TXOP_CSR1_STRUC;
#endif


#define WMM_TXOP2_CFG 0x0228
#define WMM_TXOP3_CFG 0x022c

#define WMM_CTRL	0x0230


/*================================================================================= */
/* MAC  registers                                                                                                                                                                 */
/*================================================================================= */
/*  4.1 MAC SYSTEM  configuration registers (offset:0x1000) */
#define MAC_CSR0            0x1000
#ifdef RT_BIG_ENDIAN
typedef	union _ASIC_VER_ID_STRUC {
	struct {
	    uint16_t  ASICVer;        /* version */
	    uint16_t  ASICRev;        /* reversion */
	} field;
	uint32_t word;
} ASIC_VER_ID_STRUC;
#else
typedef	union _ASIC_VER_ID_STRUC {
	struct {
	    uint16_t  ASICRev;
	    uint16_t  ASICVer;
	} field;
	uint32_t word;
} ASIC_VER_ID_STRUC;
#endif

#define MAC_SYS_CTRL		0x1004
#define PBF_LOOP_EN			(1 << 5)
#define MAC_ADDR_DW0		0x1008
#define MAC_ADDR_DW1		0x100c

/* MAC_CSR2: STA MAC register 0 */
#ifdef RT_BIG_ENDIAN
typedef	union _MAC_DW0_STRUC {
	struct {
		UINT8 Byte3;		/* MAC address byte 3 */
		UINT8 Byte2;		/* MAC address byte 2 */
		UINT8 Byte1;		/* MAC address byte 1 */
		UINT8 Byte0;		/* MAC address byte 0 */
	} field;
	uint32_t word;
} MAC_DW0_STRUC;
#else
typedef	union _MAC_DW0_STRUC {
	struct {
		UINT8 Byte0;
		UINT8 Byte1;
		UINT8 Byte2;
		UINT8 Byte3;
	} field;
	uint32_t word;
} MAC_DW0_STRUC;
#endif


/* MAC_CSR3: STA MAC register 1 */
#ifdef RT_BIG_ENDIAN
typedef	union _MAC_DW1_STRUC {
	struct {
		UINT8		Rsvd1;
		UINT8		U2MeMask;
		UINT8		Byte5;		/* MAC address byte 5 */
		UINT8		Byte4;		/* MAC address byte 4 */
	} field;
	uint32_t word;
} MAC_DW1_STRUC;
#else
typedef	union _MAC_DW1_STRUC {
	struct {
		UINT8 Byte4;
		UINT8 Byte5;
		UINT8 U2MeMask;
		UINT8 Rsvd1;
	} field;
	uint32_t word;
}	MAC_DW1_STRUC;
#endif

#define MAC_BSSID_DW0		0x1010
#define MAC_BSSID_DW1		0x1014
/* MAC_CSR5: BSSID register 1 */
#ifdef RT_BIG_ENDIAN
typedef	union	_MAC_BSSID_DW1_STRUC {
	struct {
		uint32_t NMBssMode3:1;
		uint32_t NMBssMode2:1;
		uint32_t NMBssMode:1;
		uint32_t MBssBcnNum:3;
		uint32_t MBssMode:2; /* 0: one BSSID, 10: 4 BSSID,  01: 2 BSSID , 11: 8BSSID */
		uint32_t Byte5:8;		 /* BSSID byte 5 */
		uint32_t Byte4:8;		 /* BSSID byte 4 */
	} field;
	uint32_t word;
} MAC_BSSID_DW1_STRUC;
#else
typedef	union	_MAC_BSSID_DW1_STRUC {
	struct {
		uint32_t Byte4:8;
		uint32_t Byte5:8;
		uint32_t MBssMode:2;
		uint32_t MBssBcnNum:3;
		uint32_t NMBssMode:1;
		uint32_t NMBssMode2:1;
		uint32_t NMBssMode3:1;
	} field;
	uint32_t word;
} MAC_BSSID_DW1_STRUC;
#endif

/* rt2860b max 16k bytes. bit12:13 Maximum PSDU length (power factor) 0:2^13, 1:2^14, 2:2^15, 3:2^16 */
#define MAX_LEN_CFG              0x1018


/* BBP_CSR_CFG: BBP serial control register */
#define BBP_CSR_CFG            	0x101c
#ifdef RT_BIG_ENDIAN
typedef	union _BBP_CSR_CFG_STRUC {
	struct {
		uint32_t 	:12;
		uint32_t 	BBP_RW_MODE:1;	/* 0: use serial mode  1:parallel */
		uint32_t 	BBP_PAR_DUR:1;		/* 0: 4 MAC clock cycles  1: 8 MAC clock cycles */
		uint32_t 	Busy:1;				/* 1: ASIC is busy execute BBP programming. */
		uint32_t 	fRead:1;				/* 0: Write BBP, 1:	Read BBP */
		uint32_t 	RegNum:8;			/* Selected BBP register */
		uint32_t 	Value:8;				/* Register value to program into BBP */
	} field;
	uint32_t word;
} BBP_CSR_CFG_STRUC;
#else
typedef	union _BBP_CSR_CFG_STRUC {
	struct {
		uint32_t 	Value:8;
		uint32_t 	RegNum:8;
		uint32_t 	fRead:1;
		uint32_t 	Busy:1;
		uint32_t 	BBP_PAR_DUR:1;
		uint32_t 	BBP_RW_MODE:1;
		uint32_t 	:12;
	} field;
	uint32_t word;
} BBP_CSR_CFG_STRUC;
#endif


/* RF_CSR_CFG: RF control register */
#define RF_CSR_CFG0            		0x1020
#ifdef RT_BIG_ENDIAN
typedef	union _RF_CSR_CFG0_STRUC {
	struct {
		uint32_t Busy:1;		    /* 0: idle 1: 8busy */
		uint32_t Sel:1;			/* 0:RF_LE0 activate  1:RF_LE1 activate */
		uint32_t StandbyMode:1;	/* 0: high when stand by 1:	low when standby */
		uint32_t bitwidth:5;		/* Selected BBP register */
		uint32_t RegIdAndContent:24;	/* Register value to program into BBP */
	} field;
	uint32_t word;
} RF_CSR_CFG0_STRUC;
#else
typedef	union _RF_CSR_CFG0_STRUC {
	struct {
		uint32_t RegIdAndContent:24;
		uint32_t bitwidth:5;
		uint32_t StandbyMode:1;
		uint32_t Sel:1;
		uint32_t Busy:1;
	} field;
	uint32_t word;
} RF_CSR_CFG0_STRUC;
#endif


#define RF_CSR_CFG1           		0x1024
#ifdef RT_BIG_ENDIAN
typedef	union _RF_CSR_CFG1_STRUC {
	struct {
		uint32_t rsv:7;	/* 0: idle 1: 8busy */
		uint32_t RFGap:5;	/* Gap between BB_CONTROL_RF and RF_LE. 0: 3 system clock cycle (37.5usec) 1: 5 system clock cycle (62.5usec) */
		uint32_t RegIdAndContent:24;	/* Register value to program into BBP */
	} field;
	uint32_t word;
} RF_CSR_CFG1_STRUC;
#else
typedef	union _RF_CSR_CFG1_STRUC {
	struct {
		uint32_t RegIdAndContent:24;
		uint32_t RFGap:5;
		uint32_t rsv:7;
	} field;
	uint32_t word;
} RF_CSR_CFG1_STRUC;
#endif


#define RF_CSR_CFG2           		0x1028
#ifdef RT_BIG_ENDIAN
typedef	union _RF_CSR_CFG2_STRUC {
	struct {
		uint32_t rsv:8;		    /* 0: idle 1: 8busy */
		uint32_t RegIdAndContent:24; /* Register value to program into BBP */
	} field;
	uint32_t word;
}	RF_CSR_CFG2_STRUC;
#else
typedef	union _RF_CSR_CFG2_STRUC {
	struct {
		uint32_t RegIdAndContent:24;
		uint32_t rsv:8;
	} field;
	uint32_t word;
} RF_CSR_CFG2_STRUC;
#endif


#define LED_CFG           		0x102c
#ifdef RT_BIG_ENDIAN
typedef	union _LED_CFG_STRUC {
	struct {
		uint32_t 	:1;
		uint32_t 	LedPolar:1;			/* Led Polarity.  0: active low1: active high */
		uint32_t 	YLedMode:2;			/* yellow Led Mode */
		uint32_t 	GLedMode:2;			/* green Led Mode */
		uint32_t 	RLedMode:2;			/* red Led Mode    0: off1: blinking upon TX2: periodic slow blinking3: always on */
		uint32_t 	rsv:2;
		uint32_t 	SlowBlinkPeriod:6;			/* slow blinking period. unit:1ms */
		uint32_t 	OffPeriod:8;			/* blinking off period unit 1ms */
		uint32_t 	OnPeriod:8;			/* blinking on period unit 1ms */
	} field;
	uint32_t word;
} LED_CFG_STRUC;
#else
typedef	union _LED_CFG_STRUC {
	struct {
		uint32_t 	OnPeriod:8;
		uint32_t 	OffPeriod:8;
		uint32_t 	SlowBlinkPeriod:6;
		uint32_t 	rsv:2;
		uint32_t 	RLedMode:2;
		uint32_t 	GLedMode:2;
		uint32_t 	YLedMode:2;
		uint32_t 	LedPolar:1;
		uint32_t 	:1;
	} field;
	uint32_t word;
} LED_CFG_STRUC;
#endif


#define AMPDU_MAX_LEN_20M1S	0x1030
#define AMPDU_MAX_LEN_20M2S	0x1034
#define AMPDU_MAX_LEN_20M1S_MCS0_7	0x1030
#define AMPDU_MAX_LEN_20M1S_MCS8_9	0x1034
#define AMPDU_MAX_LEN_40M1S	0x1038
#define AMPDU_MAX_LEN_40M2S	0x103c
#define AMPDU_MAX_LEN			0x1040


/* The number of the Tx chains */
#define NUM_OF_TX_CHAIN		4

#define TX_CHAIN_ADDR0_L	0x1044		/* Stream mode MAC address registers */
#define TX_CHAIN_ADDR0_H	0x1048
#define TX_CHAIN_ADDR1_L	0x104C
#define TX_CHAIN_ADDR1_H	0x1050
#define TX_CHAIN_ADDR2_L	0x1054
#define TX_CHAIN_ADDR2_H	0x1058
#define TX_CHAIN_ADDR3_L	0x105C
#define TX_CHAIN_ADDR3_H	0x1060

#define TX_WCID_DROP_MASK0	0x106C

#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR0_L_STRUC {
	struct {
		UINT8 TxChainAddr0L_Byte3; /* Destination MAC address of Tx chain0 (byte 3) */
		UINT8 TxChainAddr0L_Byte2; /* Destination MAC address of Tx chain0 (byte 2) */
		UINT8 TxChainAddr0L_Byte1; /* Destination MAC address of Tx chain0 (byte 1) */
		UINT8 TxChainAddr0L_Byte0; /* Destination MAC address of Tx chain0 (byte 0) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR0_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR0_L_STRUC {
	struct {
		UINT8 TxChainAddr0L_Byte0;
		UINT8 TxChainAddr0L_Byte1;
		UINT8 TxChainAddr0L_Byte2;
		UINT8 TxChainAddr0L_Byte3;
	} field;
	uint32_t word;
} TX_CHAIN_ADDR0_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR0_H_STRUC {
	struct {
		uint16_t Reserved:12; /* Reserved */
		uint16_t TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr0H_Byte5; /* Destination MAC address of Tx chain0 (byte 5) */
		UINT8	TxChainAddr0H_Byte4; /* Destination MAC address of Tx chain0 (byte 4) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR0_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR0_H_STRUC {
	struct {
		UINT8	TxChainAddr0H_Byte4; /* Destination MAC address of Tx chain0 (byte 4) */
		UINT8	TxChainAddr0H_Byte5; /* Destination MAC address of Tx chain0 (byte 5) */
		uint16_t TxChainSel0:4; /* Selection value of Tx chain0 */
		uint16_t Reserved:12; /* Reserved */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR0_H_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR1_L_STRUC {
	struct {
		UINT8	TxChainAddr1L_Byte3; /* Destination MAC address of Tx chain1 (byte 3) */
		UINT8	TxChainAddr1L_Byte2; /* Destination MAC address of Tx chain1 (byte 2) */
		UINT8	TxChainAddr1L_Byte1; /* Destination MAC address of Tx chain1 (byte 1) */
		UINT8	TxChainAddr1L_Byte0; /* Destination MAC address of Tx chain1 (byte 0) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR1_L_STRUC, *PTX_CHAIN_ADDR1_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR1_L_STRUC {
	struct {
		UINT8	TxChainAddr1L_Byte0;
		UINT8	TxChainAddr1L_Byte1;
		UINT8	TxChainAddr1L_Byte2;
		UINT8	TxChainAddr1L_Byte3;
	} field;
	uint32_t word;
} TX_CHAIN_ADDR1_L_STRUC, *PTX_CHAIN_ADDR1_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR1_H_STRUC {
	struct {
		uint16_t Reserved:12; /* Reserved */
		uint16_t TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr1H_Byte5; /* Destination MAC address of Tx chain1 (byte 5) */
		UINT8	TxChainAddr1H_Byte4; /* Destination MAC address of Tx chain1 (byte 4) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR1_H_STRUC ;
#else
typedef union _TX_CHAIN_ADDR1_H_STRUC {
	struct {
		UINT8	TxChainAddr1H_Byte4;
		UINT8	TxChainAddr1H_Byte5;
		uint16_t TxChainSel0:4;
		uint16_t Reserved:12;
	} field;
	uint32_t word;
} TX_CHAIN_ADDR1_H_STRUC ;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR2_L_STRUC {
	struct {
		UINT8	TxChainAddr2L_Byte3; /* Destination MAC address of Tx chain2 (byte 3) */
		UINT8	TxChainAddr2L_Byte2; /* Destination MAC address of Tx chain2 (byte 2) */
		UINT8	TxChainAddr2L_Byte1; /* Destination MAC address of Tx chain2 (byte 1) */
		UINT8	TxChainAddr2L_Byte0; /* Destination MAC address of Tx chain2 (byte 0) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR2_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR2_L_STRUC {
	struct {
		UINT8	TxChainAddr2L_Byte0;
		UINT8	TxChainAddr2L_Byte1;
		UINT8	TxChainAddr2L_Byte2;
		UINT8	TxChainAddr2L_Byte3;
	} field;
	uint32_t word;
} TX_CHAIN_ADDR2_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR2_H_STRUC {
	struct {
		uint16_t Reserved:12; /* Reserved */
		uint16_t TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr2H_Byte5; /* Destination MAC address of Tx chain2 (byte 5) */
		UINT8	TxChainAddr2H_Byte4; /* Destination MAC address of Tx chain2 (byte 4) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR2_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR2_H_STRUC {
	struct {
		UINT8	TxChainAddr2H_Byte4;
		UINT8	TxChainAddr2H_Byte5;
		uint16_t TxChainSel0:4;
		uint16_t Reserved:12;
	} field;
	uint32_t word;
} TX_CHAIN_ADDR2_H_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR3_L_STRUC {
	struct {
		UINT8	TxChainAddr3L_Byte3; /* Destination MAC address of Tx chain3 (byte 3) */
		UINT8	TxChainAddr3L_Byte2; /* Destination MAC address of Tx chain3 (byte 2) */
		UINT8	TxChainAddr3L_Byte1; /* Destination MAC address of Tx chain3 (byte 1) */
		UINT8	TxChainAddr3L_Byte0; /* Destination MAC address of Tx chain3 (byte 0) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR3_L_STRUC, *PTX_CHAIN_ADDR3_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR3_L_STRUC {
	struct {
		UINT8	TxChainAddr3L_Byte0;
		UINT8	TxChainAddr3L_Byte1;
		UINT8	TxChainAddr3L_Byte2;
		UINT8	TxChainAddr3L_Byte3;
	} field;
	uint32_t word;
}	TX_CHAIN_ADDR3_L_STRUC, *PTX_CHAIN_ADDR3_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR3_H_STRUC {
	struct {
		uint16_t Reserved:12; /* Reserved */
		uint16_t TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr3H_Byte5; /* Destination MAC address of Tx chain3 (byte 5) */
		UINT8	TxChainAddr3H_Byte4; /* Destination MAC address of Tx chain3 (byte 4) */
	} field;
	uint32_t word;
} TX_CHAIN_ADDR3_H_STRUC, *PTX_CHAIN_ADDR3_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR3_H_STRUC {
	struct {
		UINT8	TxChainAddr3H_Byte4;
		UINT8	TxChainAddr3H_Byte5;
		uint16_t TxChainSel0:4;
		uint16_t Reserved:12;
	} field;
	uint32_t word;
} TX_CHAIN_ADDR3_H_STRUC;
#endif

#define TX_BCN_BYPASS_MASK          0x108C
typedef union _BCN_BYPASS_MASK_STRUC {
        struct {
                uint32_t BeaconDropMask:16;
        } field;
        uint32_t word;
} BCN_BYPASS_MASK_STRUC;

/*  4.2 MAC TIMING  configuration registers (offset:0x1100) */
#define XIFS_TIME_CFG             0x1100
#define XIFS_TIME_OFDM_SIFS_MASK (0xff << 8)
#define XIFS_TIME_OFDM_SIFS(p) (((p) & 0xff) << 8)
#ifdef RT_BIG_ENDIAN
typedef	union _IFS_SLOT_CFG_STRUC {
	struct {
	    uint32_t  rsv:2;
	    uint32_t  BBRxendEnable:1;        /*  reference RXEND signal to begin XIFS defer */
	    uint32_t  EIFS:9;        /*  unit 1us */
	    uint32_t  OfdmXifsTime:4;        /*OFDM SIFS. unit 1us. Applied after OFDM RX when MAC doesn't reference BBP signal BBRXEND */
	    uint32_t  OfdmSifsTime:8;        /*  unit 1us. Applied after OFDM RX/TX */
	    uint32_t  CckmSifsTime:8;        /*  unit 1us. Applied after CCK RX/TX */
	} field;
	uint32_t word;
} IFS_SLOT_CFG_STRUC;
#else
typedef	union _IFS_SLOT_CFG_STRUC {
	struct {
	    uint32_t  CckmSifsTime:8;
	    uint32_t  OfdmSifsTime:8;
	    uint32_t  OfdmXifsTime:4;
	    uint32_t  EIFS:9;
	    uint32_t  BBRxendEnable:1;
	    uint32_t  rsv:2;
	} field;
	uint32_t word;
} IFS_SLOT_CFG_STRUC;
#endif

#define BKOFF_SLOT_CFG		0x1104
#define BKOFF_SLOT_CFG_CC_DELAY_TIME_MASK (0x0f << 8)
#define BKOFF_SLOT_CFG_CC_DELAY_TIME(p) (((p) & 0x0f) << 8)

#define NAV_TIME_CFG		0x1108
#define CH_TIME_CFG			0x110C
#define PBF_LIFE_TIMER		0x1110	/*TX/RX MPDU timestamp timer (free run)Unit: 1us */


/* BCN_TIME_CFG : Synchronization control register */
#define BCN_TIME_CFG             0x1114
#ifdef RT_BIG_ENDIAN
typedef	union _BCN_TIME_CFG_STRUC {
	struct {
		uint32_t TxTimestampCompensate:8;
		uint32_t :3;
		uint32_t bBeaconGen:1;		/* Enable beacon generator */
		uint32_t bTBTTEnable:1;
		uint32_t TsfSyncMode:2;		/* Enable TSF sync, 00: disable, 01: infra mode, 10: ad-hoc mode */
		uint32_t bTsfTicking:1;		/* Enable TSF auto counting */
		uint32_t BeaconInterval:16;  /* in unit of 1/16 TU */
	} field;
	uint32_t word;
} BCN_TIME_CFG_STRUC;
#else
typedef union _BCN_TIME_CFG_STRUC {
	struct {
		uint32_t BeaconInterval:16;
		uint32_t bTsfTicking:1;
		uint32_t TsfSyncMode:2;
		uint32_t bTBTTEnable:1;
		uint32_t bBeaconGen:1;
		uint32_t :3;
		uint32_t TxTimestampCompensate:8;
	} field;
	uint32_t word;
} BCN_TIME_CFG_STRUC;
#endif


#define TBTT_SYNC_CFG		0x1118
#define TSF_TIMER_DW0		0x111c	/* Local TSF timer lsb 32 bits. Read-only */
#define TSF_TIMER_DW1		0x1120	/* msb 32 bits. Read-only. */
#define TBTT_TIMER			0x1124	/* TImer remains till next TBTT. Read-only */
#define INT_TIMER_CFG		0x1128
#define INT_TIMER_EN		0x112c	/* GP-timer and pre-tbtt Int enable */
#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2

#define CH_IDLE_STA			0x1130	/* channel idle time */
#define CH_BUSY_STA			0x1134	/* channle busy time */
#define CH_BUSY_STA_SEC	0x1138	/* channel busy time for secondary channel */


/*  4.2 MAC POWER  configuration registers (offset:0x1200) */
#define MAC_STATUS_CFG		0x1200
#define PWR_PIN_CFG		0x1204


/* AUTO_WAKEUP_CFG: Manual power control / status register */
#define AUTO_WAKEUP_CFG	0x1208
#ifdef RT_BIG_ENDIAN
typedef	union _AUTO_WAKEUP_STRUC {
	struct {
		uint32_t :16;
		uint32_t EnableAutoWakeup:1;	/* 0:sleep, 1:awake */
		uint32_t NumofSleepingTbtt:7;          /* ForceWake has high privilege than PutToSleep when both set */
		uint32_t AutoLeadTime:8;
	} field;
	uint32_t word;
} AUTO_WAKEUP_STRUC;
#else
typedef	union _AUTO_WAKEUP_STRUC {
	struct {
		uint32_t AutoLeadTime:8;
		uint32_t NumofSleepingTbtt:7;
		uint32_t EnableAutoWakeup:1;
		uint32_t :16;
	} field;
	uint32_t word;
} AUTO_WAKEUP_STRUC;
#endif

#define AUX_CLK_CFG			0x120C
#define MIMO_PS_CFG		0x1210

#define BB_PA_MODE_CFG0			0x1214
#define BB_PA_MODE_CFG1			0x1218
#define RF_PA_MODE_CFG0			0x121C
#define RF_PA_MODE_CFG1			0x1220

#define DACCLK_EN_DLY_CFG       0x1264

#define TX_ALC_CFG_0				0x13B0
#define TX_ALC_CFG_0_CH_INT_0_MASK (0x3f)
#define TX_ALC_CFG_0_CH_INT_0(p) ((p) & 0x3f)
#define TX_ALC_CFG_0_CH_INT_1_MASK ((0x3f) << 8)
#define TX_ALC_CFG_0_CH_INT_1(p) (((p) & 0x3f) << 8)

#define TX_ALC_CFG_1					0x13B4
#define TX_ALC_CFG_1_TX0_TEMP_COMP_MASK (0x3f)
#define TX_ALC_CFG_1_TX0_TEMP_COMP(p) ((p) & 0x3f)
#define TX_ALC_CFG_2					0x13A8
#define TX_ALC_CFG_2_TX1_TEMP_COMP_MASK (0x3f)
#define TX_ALC_CFG_2_TX1_TEMP_COMP(p) ((p) & 0x3f)
#define TX_ALC_CFG_3					0x13AC
#define TX_ALC_CFG_4 					0x13C0

#define TX0_RF_GAIN_CORR			0x13A0
#define TX0_RF_GAIN_ATTEN			0x13A8
#define TX0_BB_GAIN_ATTEN			0x13C0
#define WL_LOWGAIN_CH_EN (1 << 31)

#define TX_ALC_VGA3					0x13C8

#define TX1_RF_GAIN_CORR			0x13A4
#define TX1_RF_GAIN_ATTEN			0x13AC
#define WL_LOWGAIN_CH_EN 			(1 << 31)


/*  4.3 MAC TX  configuration registers (offset:0x1300) */
#define EDCA_AC0_CFG	0x1300
#define EDCA_AC1_CFG	0x1304
#define EDCA_AC2_CFG	0x1308
#define EDCA_AC3_CFG	0x130c
#ifdef RT_BIG_ENDIAN
typedef	union _EDCA_AC_CFG_STRUC {
	struct {
	    uint32_t :12;
	    uint32_t Cwmax:4;	/* unit power of 2 */
	    uint32_t Cwmin:4;
	    uint32_t Aifsn:4;	/* # of slot time */
	    uint32_t AcTxop:8;	/*  in unit of 32us */
	} field;
	uint32_t word;
} EDCA_AC_CFG_STRUC;
#else
typedef	union _EDCA_AC_CFG_STRUC {
	struct {
	    uint32_t AcTxop:8;
	    uint32_t Aifsn:4;
	    uint32_t Cwmin:4;
	    uint32_t Cwmax:4;
	    uint32_t :12;
	} field;
	uint32_t word;
} EDCA_AC_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */

#define EDCA_TID_AC_MAP	0x1310


/* Default Tx power */
#define DEFAULT_TX_POWER	0x6

#define TX_PWR_CFG_0		0x1314
#define TX_PWR_CCK_1_2_MASK (0x3f)
#define TX_PWR_CCK_1_2(p) (((p) & 0x3f))
#define TX_PWR_CCK_5_11_MASK (0x3f << 8)
#define TX_PWR_CCK_5_11(p) (((p) & 0x3f) << 8)
#define TX_PWR_OFDM_6_9_MASK (0x3f << 16)
#define TX_PWR_OFDM_6_9(p) (((p) & 0x3f) << 16)
#define TX_PWR_OFDM_12_18_MASK (0x3f << 24)
#define TX_PWR_OFDM_12_18(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_0_EXT	0x1390

#define TX_PWR_CFG_1		0x1318
#define TX_PWR_OFDM_24_36_MASK (0x3f)
#define TX_PWR_OFDM_24_36(p) (((p) & 0x3f))
#define TX_PWR_OFDM_48_MASK (0x3f << 8)
#define TX_PWR_OFDM_48(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_VHT_1SS_MCS_0_1_MASK (0x3f << 16)
#define TX_PWR_HT_VHT_1SS_MCS_0_1(p) (((p) & 0x3f) << 16)
#define TX_PWR_HT_VHT_1SS_MCS_2_3_MASK (0x3f << 24)
#define TX_PWR_HT_VHT_1SS_MCS_2_3(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_1_EXT	0x1394

#define TX_PWR_CFG_2		0x131C
#define TX_PWR_HT_VHT_1SS_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_VHT_1SS_MCS_4_5(p) (((p) & 0x3f))
#define TX_PWR_HT_VHT_1SS_MCS_6_MASK (0x3f << 8)
#define TX_PWR_HT_VHT_1SS_MCS_6(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_MCS_8_9_VHT_2SS_0_1_MASK (0x3f << 16)
#define TX_PWR_HT_MCS_8_9_VHT_2SS_0_1(p) (((p) & 0x3f) << 16)
#define TX_PWR_HT_MCS_10_11_VHT_2SS_MCS_2_3_MASK (0x3f << 24)
#define TX_PWR_HT_MCS_10_11_VHT_2SS_MCS_2_3(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_2_EXT	0x1398

#define TX_PWR_CFG_3		0x1320
#define TX_PWR_HT_MCS_12_13_VHT_2SS_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_MCS_12_13_VHT_2SS_MCS_4_5(p) (((p) & 0x3f))
#define TX_PWR_HT_MCS_14_VHT_2SS_MCS_6_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_14_VHT_2SS_MCS_6(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_VHT_STBC_MCS_0_1_MASK (0x3f << 16)
#define TX_PWR_HT_VHT_STBC_MCS_0_1(p) (((p) & 0x3f) << 16)
#define TX_PWR_HT_VHT_STBC_MCS_2_3_MASK (0x3f << 24)
#define TX_PWR_HT_VHT_STBC_MCS_2_3(p) (((p) & 0x3f) << 24)

#define GF20_PORT_CFG       0x1374
#define TX_PWR_CFG_3_EXT	0x139C

#define TX_PWR_CFG_4		0x1324
#define TX_PWR_HT_VHT_STBC_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_VHT_STBC_MCS_4_5(p) (((p) & 0x3f))
#define TX_PWR_HT_VHT_STBC_MCS_6_MASK (0x3f << 8)
#define TX_PWR_HT_VHT_STBC_MCS_6(p) (((p) & 0x3f) << 8)

#define TX_PWR_CFG_0_EXT	0x1390
#define TX_PWR_CFG_1_EXT	0x1394
#define TX_PWR_CFG_2_EXT	0x1398
#define TX_PWR_CFG_3_EXT	0x139C
#define TX_PWR_CFG_4_EXT	0x13A0

#define TX_PWR_CFG_5		0x1384
#define TX_PWR_CFG_6		0x1388

#define TX_PWR_CFG_7		0x13D4
#define TX_PWR_OFDM_54_MASK (0x3f)
#define TX_PWR_OFDM_54(p) (((p) & 0x3f))
#define TX_PWR_VHT_2SS_MCS_8_MASK (0x3f << 8)
#define TX_PWR_VHT_2SS_MCS_8(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_MCS_7_VHT_1SS_MCS_7_MASK (0x3f << 16)
#define TX_PWR_HT_MCS_7_VHT_1SS_MCS_7(p) (((p) & 0x3f) << 16)
#define TX_PWR_VHT_2SS_MCS_9_MASK (0X3f << 24)
#define TX_PWR_VHT_2SS_MCS_9(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_8		0x13D8
#define TX_PWR_HT_MCS_15_VHT_2SS_MCS7_MASK (0x3f)
#define TX_PWR_HT_MCS_15_VHT_2SS_MCS7(p) (((p) & 0x3f))
#define TX_PWR_VHT_1SS_MCS_8_MASK (0x3f << 16)
#define TX_PWR_VHT_1SS_MCS_8(p) (((p) & 0x3f) << 16)
#define TX_PWR_VHT_1SS_MCS_9_MASK (0X3f << 24)
#define TX_PWR_VHT_1SS_MCS_9(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_9		0x13DC
#define TX_PWR_HT_VHT_STBC_MCS_7_MASK (0x3f)
#define TX_PWR_HT_VHT_STBC_MCS_7(p) (((p) & 0x3f))
#define TX_PWR_VHT_STBC_MCS_8_MASK (0x3f << 16)
#define TX_PWR_VHT_STBC_MCS_8(p) (((p) & 0x3f) << 16)
#define TX_PWR_VHT_STBC_MCS_9_MASK (0x3f << 24)
#define TX_PWR_VHT_STBC_MCS_9(p) (((p) & 0x3f) << 24)

#ifdef RT_BIG_ENDIAN
typedef	union _TX_PWR_CFG_STRUC {
	struct {
	    uint32_t Byte3:8;
	    uint32_t Byte2:8;
	    uint32_t Byte1:8;
	    uint32_t Byte0:8;
	} field;
	uint32_t word;
} TX_PWR_CFG_STRUC;
#else
typedef	union _TX_PWR_CFG_STRUC {
	struct {
	    uint32_t Byte0:8;
	    uint32_t Byte1:8;
	    uint32_t Byte2:8;
	    uint32_t Byte3:8;
	} field;
	uint32_t word;
} TX_PWR_CFG_STRUC;
#endif


#define TX_PIN_CFG		0x1328
#define TX_BAND_CFG	0x132c	/* 0x1 use upper 20MHz. 0 juse lower 20MHz */
#define TX_SW_CFG0		0x1330
#define TX_SW_CFG1		0x1334
#define TX_SW_CFG2		0x1338


#define TXOP_THRES_CFG	0x133c
#ifdef RT_BIG_ENDIAN
typedef union _TXOP_THRESHOLD_CFG_STRUC {
	struct {
		uint32_t TXOP_REM_THRES:8; /* Remaining TXOP threshold (unit: 32us) */
		uint32_t CF_END_THRES:8; /* CF-END threshold (unit: 32us) */
		uint32_t RDG_IN_THRES:8; /* Rx RDG threshold (unit: 32us) */
		uint32_t RDG_OUT_THRES:8; /* Tx RDG threshold (unit: 32us) */
	} field;
	uint32_t word;
} TXOP_THRESHOLD_CFG_STRUC;
#else
typedef union _TXOP_THRESHOLD_CFG_STRUC {
	struct {
		uint32_t RDG_OUT_THRES:8;
		uint32_t RDG_IN_THRES:8;
		uint32_t CF_END_THRES:8;
		uint32_t TXOP_REM_THRES:8;
	} field;
	uint32_t word;
} TXOP_THRESHOLD_CFG_STRUC;
#endif

#define TXOP_CTRL_CFG 0x1340


#define TX_RTS_CFG 0x1344
#ifdef RT_BIG_ENDIAN
typedef	union _TX_RTS_CFG_STRUC {
	struct {
	    uint32_t rsv:7;
	    uint32_t RtsFbkEn:1;    /* enable rts rate fallback */
	    uint32_t RtsThres:16;    /* unit:byte */
	    uint32_t AutoRtsRetryLimit:8;
	} field;
	uint32_t word;
} TX_RTS_CFG_STRUC;
#else
typedef	union _TX_RTS_CFG_STRUC	 {
	struct {
	    uint32_t AutoRtsRetryLimit:8;
	    uint32_t RtsThres:16;
	    uint32_t RtsFbkEn:1;
	    uint32_t rsv:7;
	} field;
	uint32_t word;
} TX_RTS_CFG_STRUC;
#endif



#define TX_TXBF_CFG_0 			0x1624
#define TX_TXBF_CFG_1 			0x1628
#define TX_TXBF_CFG_2 			0x162C
#define TX_TXBF_CFG_3 			0x1630
typedef	union _TX_TXBF_CFG_0_STRUC {
	struct {
#ifdef RT_BIG_ENDIAN
	    uint32_t       EtxbfFbkRate:16;
	    uint32_t       EtxbfFbkEn:1;
	    uint32_t       EtxbfFbkSeqEn:1;
	    uint32_t       EtxbfFbkCoef:2;
	    uint32_t       EtxbfFbkCode:2;
	    uint32_t       EtxbfFbkNg:2;
	    uint32_t       CsdBypass:1;
	    uint32_t       EtxbfForce:1;
	    uint32_t       EtxbfEnable:1;
	    uint32_t       AutoTxbfEn:3;
	    uint32_t       ItxbfForce:1;
	    uint32_t       ItxbfEn:1;
#else
	    uint32_t       ItxbfEn:1;
	    uint32_t       ItxbfForce:1;
	    uint32_t       AutoTxbfEn:3;
	    uint32_t       EtxbfEnable:1;
	    uint32_t       EtxbfForce:1;
	    uint32_t       CsdBypass:1;
	    uint32_t       EtxbfFbkNg:2;
	    uint32_t       EtxbfFbkCode:2;
	    uint32_t       EtxbfFbkCoef:2;
	    uint32_t       EtxbfFbkSeqEn:1;
	    uint32_t       EtxbfFbkEn:1;
	    uint32_t       EtxbfFbkRate:16;
#endif
	} field;
	uint32_t word;
} TX_TXBF_CFG_0_STRUC;


#define TX_TIMEOUT_CFG	0x1348
#ifdef RT_BIG_ENDIAN
typedef	union _TX_TIMEOUT_CFG_STRUC {
	struct {
	    uint32_t rsv2:8;
	    uint32_t TxopTimeout:8;	/*TXOP timeout value for TXOP truncation.  It is recommended that (SLOT_TIME) > (TX_OP_TIMEOUT) > (RX_ACK_TIMEOUT) */
	    uint32_t RxAckTimeout:8;	/* unit:slot. Used for TX precedure */
	    uint32_t MpduLifeTime:4;    /*  expiration time = 2^(9+MPDU LIFE TIME)  us */
	    uint32_t rsv:4;
	} field;
	uint32_t word;
} TX_TIMEOUT_CFG_STRUC;
#else
typedef	union _TX_TIMEOUT_CFG_STRUC {
	struct {
	    uint32_t rsv:4;
	    uint32_t MpduLifeTime:4;
	    uint32_t RxAckTimeout:8;
	    uint32_t TxopTimeout:8;
	    uint32_t rsv2:8;
	} field;
	uint32_t word;
} TX_TIMEOUT_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */


#define TX_RTY_CFG	0x134c
#define TX_RTY_CFG_RTY_LIMIT_SHORT	0x1
#define TX_RTY_CFG_RTY_LIMIT_LONG	0x2

#ifdef RT_BIG_ENDIAN
typedef	union _TX_RTY_CFG_STRUC {
	struct {
	    uint32_t rsv:1;
	    uint32_t TxautoFBEnable:1;    /* Tx retry PHY rate auto fallback enable */
	    uint32_t AggRtyMode:1;	/* Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer */
	    uint32_t NonAggRtyMode:1;	/* Non-Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer */
	    uint32_t LongRtyThre:12;	/* Long retry threshoold */
	    uint32_t LongRtyLimit:8;	/*long retry limit */
	    uint32_t ShortRtyLimit:8;	/*  short retry limit */
	} field;
	uint32_t word;
} TX_RTY_CFG_STRUC;
#else
typedef	union _TX_RTY_CFG_STRUC {
	struct {
	    uint32_t ShortRtyLimit:8;
	    uint32_t LongRtyLimit:8;
	    uint32_t LongRtyThre:12;
	    uint32_t NonAggRtyMode:1;
	    uint32_t AggRtyMode:1;
	    uint32_t TxautoFBEnable:1;
	    uint32_t rsv:1;
	} field;
	uint32_t word;
} TX_RTY_CFG_STRUC;
#endif


#define TX_LINK_CFG	0x1350
#ifdef RT_BIG_ENDIAN
typedef	union _TX_LINK_CFG_STRUC {
	struct {
	    uint32_t       RemotMFS:8;	/*remote MCS feedback sequence number */
	    uint32_t       RemotMFB:8;    /*  remote MCS feedback */
	    uint32_t       rsv:3;	/* */
	    uint32_t       TxCFAckEn:1;	/*   Piggyback CF-ACK enable */
	    uint32_t       TxRDGEn:1;	/* RDG TX enable */
	    uint32_t       TxMRQEn:1;	/*  MCS request TX enable */
	    uint32_t       RemoteUMFSEnable:1;	/*  remote unsolicit  MFB enable.  0: not apply remote remote unsolicit (MFS=7) */
	    uint32_t       MFBEnable:1;	/*  TX apply remote MFB 1:enable */
	    uint32_t       RemoteMFBLifeTime:8;	/*remote MFB life time. unit : 32us */
	} field;
	uint32_t word;
} TX_LINK_CFG_STRUC;
#else
typedef	union _TX_LINK_CFG_STRUC {
	struct {
	    uint32_t       RemoteMFBLifeTime:8;
	    uint32_t       MFBEnable:1;
	    uint32_t       RemoteUMFSEnable:1;
	    uint32_t       TxMRQEn:1;
	    uint32_t       TxRDGEn:1;
	    uint32_t       TxCFAckEn:1;
	    uint32_t       rsv:3;
	    uint32_t       RemotMFB:8;
	    uint32_t       RemotMFS:8;
	} field;
	uint32_t word;
} TX_LINK_CFG_STRUC;
#endif


#define HT_FBK_CFG0	0x1354
#ifdef RT_BIG_ENDIAN
typedef	union _HT_FBK_CFG0_STRUC {
	struct {
	    uint32_t HTMCS7FBK:4;
	    uint32_t HTMCS6FBK:4;
	    uint32_t HTMCS5FBK:4;
	    uint32_t HTMCS4FBK:4;
	    uint32_t HTMCS3FBK:4;
	    uint32_t HTMCS2FBK:4;
	    uint32_t HTMCS1FBK:4;
	    uint32_t HTMCS0FBK:4;
	} field;
	uint32_t word;
} HT_FBK_CFG0_STRUC;
#else
typedef	union _HT_FBK_CFG0_STRUC {
	struct {
	    uint32_t HTMCS0FBK:4;
	    uint32_t HTMCS1FBK:4;
	    uint32_t HTMCS2FBK:4;
	    uint32_t HTMCS3FBK:4;
	    uint32_t HTMCS4FBK:4;
	    uint32_t HTMCS5FBK:4;
	    uint32_t HTMCS6FBK:4;
	    uint32_t HTMCS7FBK:4;
	} field;
	uint32_t word;
} HT_FBK_CFG0_STRUC;
#endif


#define HT_FBK_CFG1	0x1358
#ifdef RT_BIG_ENDIAN
typedef	union _HT_FBK_CFG1_STRUC {
	struct {
	    uint32_t       HTMCS15FBK:4;
	    uint32_t       HTMCS14FBK:4;
	    uint32_t       HTMCS13FBK:4;
	    uint32_t       HTMCS12FBK:4;
	    uint32_t       HTMCS11FBK:4;
	    uint32_t       HTMCS10FBK:4;
	    uint32_t       HTMCS9FBK:4;
	    uint32_t       HTMCS8FBK:4;
	} field;
	uint32_t word;
} HT_FBK_CFG1_STRUC;
#else
typedef	union _HT_FBK_CFG1_STRUC {
	struct {
	    uint32_t       HTMCS8FBK:4;
	    uint32_t       HTMCS9FBK:4;
	    uint32_t       HTMCS10FBK:4;
	    uint32_t       HTMCS11FBK:4;
	    uint32_t       HTMCS12FBK:4;
	    uint32_t       HTMCS13FBK:4;
	    uint32_t       HTMCS14FBK:4;
	    uint32_t       HTMCS15FBK:4;
	} field;
	uint32_t word;
} HT_FBK_CFG1_STRUC;
#endif


#define LG_FBK_CFG0	0x135c
#ifdef RT_BIG_ENDIAN
typedef	union _LG_FBK_CFG0_STRUC {
	struct {
	    uint32_t       OFDMMCS7FBK:4;
	    uint32_t       OFDMMCS6FBK:4;
	    uint32_t       OFDMMCS5FBK:4;
	    uint32_t       OFDMMCS4FBK:4;
	    uint32_t       OFDMMCS3FBK:4;
	    uint32_t       OFDMMCS2FBK:4;
	    uint32_t       OFDMMCS1FBK:4;
	    uint32_t       OFDMMCS0FBK:4;
	} field;
	uint32_t word;
} LG_FBK_CFG0_STRUC;
#else
typedef	union _LG_FBK_CFG0_STRUC {
	struct {
	    uint32_t       OFDMMCS0FBK:4;
	    uint32_t       OFDMMCS1FBK:4;
	    uint32_t       OFDMMCS2FBK:4;
	    uint32_t       OFDMMCS3FBK:4;
	    uint32_t       OFDMMCS4FBK:4;
	    uint32_t       OFDMMCS5FBK:4;
	    uint32_t       OFDMMCS6FBK:4;
	    uint32_t       OFDMMCS7FBK:4;
	} field;
	uint32_t word;
} LG_FBK_CFG0_STRUC;
#endif


#define LG_FBK_CFG1		0x1360
#ifdef RT_BIG_ENDIAN
typedef	union _LG_FBK_CFG1_STRUC {
	struct {
	    uint32_t       rsv:16;
	    uint32_t       CCKMCS3FBK:4;
	    uint32_t       CCKMCS2FBK:4;
	    uint32_t       CCKMCS1FBK:4;
	    uint32_t       CCKMCS0FBK:4;
	} field;
	uint32_t word;
}	LG_FBK_CFG1_STRUC;
#else
typedef	union _LG_FBK_CFG1_STRUC {
	struct {
	    uint32_t       CCKMCS0FBK:4;
	    uint32_t       CCKMCS1FBK:4;
	    uint32_t       CCKMCS2FBK:4;
	    uint32_t       CCKMCS3FBK:4;
	    uint32_t       rsv:16;
	}	field;
	uint32_t word;
}	LG_FBK_CFG1_STRUC;
#endif


/*======================================================= */
/*                                     Protection Paramater                                                         */
/*======================================================= */
#define ASIC_SHORTNAV	1
#define ASIC_LONGNAV	2
#define ASIC_RTS		1
#define ASIC_CTS		2

#define CCK_PROT_CFG	0x1364	/* CCK Protection */
#define OFDM_PROT_CFG	0x1368	/* OFDM Protection */
#define MM20_PROT_CFG	0x136C	/* MM20 Protection */
#define MM40_PROT_CFG	0x1370	/* MM40 Protection */
#define GF20_PROT_CFG	0x1374	/* GF20 Protection */
#define GF40_PROT_CFG	0x1378	/* GR40 Protection */
#ifdef RT_BIG_ENDIAN
typedef	union _PROT_CFG_STRUC {
	struct {
		uint32_t 	 ProtectTxop:3; /* TXOP allowance */
		uint32_t       DynCbw:1;  /* RTS use dynamic channel bandwidth when TX signaling mode is turned on */
		uint32_t       RtsTaSignal:1; /* RTS TA signaling mode */
	    uint32_t       RTSThEn:1;	/*RTS threshold enable on CCK TX */
	    uint32_t       TxopAllowGF40:1;	/*CCK TXOP allowance.0:disallow. */
	    uint32_t       TxopAllowGF20:1;	/*CCK TXOP allowance.0:disallow. */
	    uint32_t       TxopAllowMM40:1;	/*CCK TXOP allowance.0:disallow. */
	    uint32_t       TxopAllowMM20:1;	/*CCK TXOP allowance. 0:disallow. */
	    uint32_t       TxopAllowOfdm:1;	/*CCK TXOP allowance.0:disallow. */
	    uint32_t       TxopAllowCck:1;	/*CCK TXOP allowance.0:disallow. */
	    uint32_t       ProtectNav:2;	/*TXOP protection type for CCK TX. 0:None, 1:ShortNAVprotect,  2:LongNAVProtect, 3:rsv */
	    uint32_t       ProtectCtrl:2;	/*Protection control frame type for CCK TX. 1:RTS/CTS, 2:CTS-to-self, 0:None, 3:rsv */
	    uint32_t       ProtectRate:16;	/*Protection control frame rate for CCK TX(RTS/CTS/CFEnd). */
	} field;
	uint32_t word;
} PROT_CFG_STRUC;
#else
typedef	union _PROT_CFG_STRUC {
	struct {
	    uint32_t       ProtectRate:16;
	    uint32_t       ProtectCtrl:2;
	    uint32_t       ProtectNav:2;
	    uint32_t       TxopAllowCck:1;
	    uint32_t       TxopAllowOfdm:1;
	    uint32_t       TxopAllowMM20:1;
	    uint32_t       TxopAllowMM40:1;
	    uint32_t       TxopAllowGF20:1;
	    uint32_t       TxopAllowGF40:1;
	    uint32_t       RTSThEn:1;
		uint32_t       RtsTaSignal:1; /* RTS TA signaling mode */
		uint32_t       DynCbw:1;  /* RTS use dynamic channel bandwidth when TX signaling mode is turned on */
		uint32_t 	 ProtectTxop:3; /* TXOP allowance */
	} field;
	uint32_t word;
} PROT_CFG_STRUC;
#endif

#define EXP_CTS_TIME	0x137C
#define EXP_ACK_TIME	0x1380


#define HT_FBK_TO_LEGACY	0x1384

#define TX_FBK_LIMIT		0x1398


#define TX_AC_RTY_LIMIT		0x13cc
#define TX_AC_FBK_SPEED	0x13d0



/*  4.4 MAC RX configuration registers (offset:0x1400) */

/* RX_FILTR_CFG:  /RX configuration register */
#define RX_FILTR_CFG	0x1400
#ifdef RT_BIG_ENDIAN
typedef	union _RX_FILTR_CFG_STRUC {
	struct {
		uint32_t rsv:15;
		uint32_t DropRsvCntlType:1;
        	uint32_t DropBAR:1;
		uint32_t DropBA:1;
		uint32_t DropPsPoll:1;		/* Drop Ps-Poll */
		uint32_t DropRts:1;		/* Drop Ps-Poll */
		uint32_t DropCts:1;		/* Drop Ps-Poll */
		uint32_t DropAck:1;		/* Drop Ps-Poll */
		uint32_t DropCFEnd:1;		/* Drop Ps-Poll */
		uint32_t DropCFEndAck:1;		/* Drop Ps-Poll */
		uint32_t DropDuplicate:1;		/* Drop duplicate frame */
		uint32_t DropBcast:1;		/* Drop broadcast frames */
		uint32_t DropMcast:1;		/* Drop multicast frames */
		uint32_t DropVerErr:1;	    /* Drop version error frame */
		uint32_t DropNotMyBSSID:1;			/* Drop fram ToDs bit is true */
		uint32_t DropNotToMe:1;		/* Drop not to me unicast frame */
		uint32_t DropPhyErr:1;		/* Drop physical error */
		uint32_t DropCRCErr:1;		/* Drop CRC error */
	} field;
	uint32_t word;
} RX_FILTR_CFG_STRUC;
#else
typedef	union _RX_FILTR_CFG_STRUC {
	struct {
		uint32_t DropCRCErr:1;
		uint32_t DropPhyErr:1;
		uint32_t DropNotToMe:1;
		uint32_t DropNotMyBSSID:1;
		uint32_t DropVerErr:1;
		uint32_t DropMcast:1;
		uint32_t DropBcast:1;
		uint32_t DropDuplicate:1;
		uint32_t DropCFEndAck:1;
		uint32_t DropCFEnd:1;
		uint32_t DropAck:1;
		uint32_t DropCts:1;
		uint32_t DropRts:1;
		uint32_t DropPsPoll:1;
		uint32_t DropBA:1;
        	uint32_t  DropBAR:1;
		uint32_t DropRsvCntlType:1;
		uint32_t rsv:15;
	} field;
	uint32_t word;
}	RX_FILTR_CFG_STRUC;
#endif


/* AUTO_RSP_CFG: Auto-Responder */
#define AUTO_RSP_CFG	0x1404
#ifdef RT_BIG_ENDIAN
typedef union _AUTO_RSP_CFG_STRUC {
	struct {
		uint32_t        :24;
		uint32_t       AckCtsPsmBit:1;   /* Power bit value in conrtrol frame */
		uint32_t       DualCTSEn:1;   /* Power bit value in conrtrol frame */
		uint32_t       rsv:1;   /* Power bit value in conrtrol frame */
		uint32_t       AutoResponderPreamble:1;    /* 0:long, 1:short preamble */
		uint32_t       CTS40MRef:1;  /* Response CTS 40MHz duplicate mode */
		uint32_t       CTS40MMode:1;  /* Response CTS 40MHz duplicate mode */
		uint32_t       BACAckPolicyEnable:1;    /* 0:long, 1:short preamble */
		uint32_t       AutoResponderEnable:1;
	} field;
	uint32_t word;
} AUTO_RSP_CFG_STRUC;
#else
typedef union _AUTO_RSP_CFG_STRUC {
	struct {
		uint32_t       AutoResponderEnable:1;
		uint32_t       BACAckPolicyEnable:1;
		uint32_t       CTS40MMode:1;
		uint32_t       CTS40MRef:1;
		uint32_t       AutoResponderPreamble:1;
		uint32_t       rsv:1;
		uint32_t       DualCTSEn:1;
		uint32_t       AckCtsPsmBit:1;
		uint32_t        :24;
	} field;
	uint32_t   word;
} AUTO_RSP_CFG_STRUC;
#endif


#define LEGACY_BASIC_RATE	0x1408
#define HT_BASIC_RATE		0x140c
#define HT_CTRL_CFG			0x1410
#define SIFS_COST_CFG		0x1414
#define RX_PARSER_CFG		0x1418	/*Set NAV for all received frames */

#define EXT_CCA_CFG			0x141c

#ifdef MAC_APCLI_SUPPORT
#define APCLI_BSSID_IDX			8
#define MAC_APCLI_BSSID_DW0		0x1090
#define MAC_APCLI_BSSID_DW1		0x1094
#endif /* MAC_APCLI_SUPPORT */



/*  4.5 MAC Security configuration (offset:0x1500) */
#define TX_SEC_CNT0		0x1500
#define RX_SEC_CNT0		0x1504
#define CCMP_FC_MUTE	0x1508


/*  4.6 HCCA/PSMP (offset:0x1600) */
#define TXOP_HLDR_ADDR0		0x1600
#define TXOP_HLDR_ADDR1		0x1604
#define TXOP_HLDR_ET			0x1608
#define QOS_CFPOLL_RA_DW0		0x160c
#define QOS_CFPOLL_A1_DW1		0x1610
#define QOS_CFPOLL_QC			0x1614


/*  4.7 MAC Statistis registers (offset:0x1700) */
/* RX_STA_CNT0_STRUC: RX PLCP error count & RX CRC error count */
#define RX_STA_CNT0		0x1700
#ifdef RT_BIG_ENDIAN
typedef	union _RX_STA_CNT0_STRUC {
	struct {
	    uint16_t  PhyErr;
	    uint16_t  CrcErr;
	} field;
	uint32_t word;
} RX_STA_CNT0_STRUC;
#else
typedef	union _RX_STA_CNT0_STRUC {
	struct {
	    uint16_t  CrcErr;
	    uint16_t  PhyErr;
	} field;
	uint32_t word;
} RX_STA_CNT0_STRUC;
#endif


/* RX_STA_CNT1_STRUC: RX False CCA count & RX LONG frame count */
#define RX_STA_CNT1		0x1704
#ifdef RT_BIG_ENDIAN
typedef	union _RX_STA_CNT1_STRUC {
	struct {
	    uint16_t  PlcpErr;
	    uint16_t  FalseCca;
	} field;
	uint32_t word;
} RX_STA_CNT1_STRUC;
#else
typedef	union _RX_STA_CNT1_STRUC {
	struct {
	    uint16_t  FalseCca;
	    uint16_t  PlcpErr;
	} field;
	uint32_t word;
} RX_STA_CNT1_STRUC;
#endif


/* RX_STA_CNT2_STRUC: */
#define RX_STA_CNT2		0x1708
#ifdef RT_BIG_ENDIAN
typedef	union _RX_STA_CNT2_STRUC {
	struct {
	    uint16_t  RxFifoOverflowCount;
	    uint16_t  RxDupliCount;
	} field;
	uint32_t word;
} RX_STA_CNT2_STRUC;
#else
typedef	union _RX_STA_CNT2_STRUC {
	struct {
	    uint16_t  RxDupliCount;
	    uint16_t  RxFifoOverflowCount;
	} field;
	uint32_t word;
} RX_STA_CNT2_STRUC;
#endif


/* STA_CSR3: TX Beacon count */
#define TX_STA_CNT0		0x170C
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_CNT0_STRUC {
	struct {
	    uint16_t  TxBeaconCount;
	    uint16_t  TxFailCount;
	} field;
	uint32_t word;
} TX_STA_CNT0_STRUC;
#else
typedef	union _TX_STA_CNT0_STRUC {
	struct {
	    uint16_t  TxFailCount;
	    uint16_t  TxBeaconCount;
	} field;
	uint32_t word;
} TX_STA_CNT0_STRUC;
#endif



/* TX_STA_CNT1: TX tx count */
#define TX_STA_CNT1		0x1710
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_CNT1_STRUC {
	struct {
	    uint16_t  TxRetransmit;
	    uint16_t  TxSuccess;
	} field;
	uint32_t word;
} TX_STA_CNT1_STRUC;
#else
typedef	union _TX_STA_CNT1_STRUC {
	struct {
	    uint16_t  TxSuccess;
	    uint16_t  TxRetransmit;
	} field;
	uint32_t word;
} TX_STA_CNT1_STRUC;
#endif


/* TX_STA_CNT2: TX tx count */
#define TX_STA_CNT2		0x1714
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_CNT2_STRUC {
	struct {
	    uint16_t  TxUnderFlowCount;
	    uint16_t  TxZeroLenCount;
	} field;
	uint32_t word;
} TX_STA_CNT2_STRUC;
#else
typedef	union _TX_STA_CNT2_STRUC {
	struct {
	    uint16_t  TxZeroLenCount;
	    uint16_t  TxUnderFlowCount;
	} field;
	uint32_t word;
} TX_STA_CNT2_STRUC;
#endif


/* TX_STA_FIFO_STRUC: TX Result for specific PID status fifo register */
#define TX_STA_FIFO		0x1718


#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_FIFO_STRUC {
	struct {
		uint32_t 	PhyMode:3;
		uint32_t 	iTxBF:1; /* iTxBF enable */
		uint32_t 	eTxBF:1; /* eTxBF enable */
		uint32_t 	SuccessRate:11;	/*include MCS, mode ,shortGI, BW settingSame format as TXWI Word 0 Bit 31-16. */
		uint32_t 	wcid:8;		/*wireless client index */
		uint32_t      TxAckRequired:1;    /* ack required */
		uint32_t      TxAggre:1;    /* Tx is aggregated */
		uint32_t      TxSuccess:1;   /* Tx success. whether success or not */
		uint32_t      PidType:4;
		uint32_t      bValid:1;   /* 1:This register contains a valid TX result */
	} field;
	uint32_t word;
} TX_STA_FIFO_STRUC;
#else
typedef	union _TX_STA_FIFO_STRUC {
	struct {
		uint32_t      bValid:1;
		uint32_t      PidType:4;
		uint32_t      TxSuccess:1;
		uint32_t      TxAggre:1;
		uint32_t      TxAckRequired:1;
		uint32_t 	wcid:8;
		uint32_t 	SuccessRate:11;
		uint32_t 	eTxBF:1;
		uint32_t 	iTxBF:1;
		uint32_t 	PhyMode:3;
	} field;
	uint32_t word;
} TX_STA_FIFO_STRUC;
#endif /* RT_BIG_ENDIAN */


/*
	Debug counters
*/
#define TX_AGG_CNT		0x171c
#ifdef RT_BIG_ENDIAN
typedef	union _TX_NAG_AGG_CNT_STRUC {
	struct {
	    uint16_t  AggTxCount;
	    uint16_t  NonAggTxCount;
	} field;
	uint32_t word;
} TX_NAG_AGG_CNT_STRUC;
#else
typedef	union _TX_NAG_AGG_CNT_STRUC {
	struct {
	    uint16_t  NonAggTxCount;
	    uint16_t  AggTxCount;
	} field;
	uint32_t word;
} TX_NAG_AGG_CNT_STRUC;
#endif


#define TX_AGG_CNT0	0x1720
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT0_STRUC {
	struct {
	    uint16_t  AggSize2Count;
	    uint16_t  AggSize1Count;
	} field;
	uint32_t word;
} TX_AGG_CNT0_STRUC;
#else
typedef	union _TX_AGG_CNT0_STRUC {
	struct {
	    uint16_t  AggSize1Count;
	    uint16_t  AggSize2Count;
	} field;
	uint32_t word;
} TX_AGG_CNT0_STRUC;
#endif


#define TX_AGG_CNT1	0x1724
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT1_STRUC {
	struct {
	    uint16_t  AggSize4Count;
	    uint16_t  AggSize3Count;
	} field;
	uint32_t word;
} TX_AGG_CNT1_STRUC;
#else
typedef	union _TX_AGG_CNT1_STRUC {
	struct {
	    uint16_t  AggSize3Count;
	    uint16_t  AggSize4Count;
	} field;
	uint32_t word;
} TX_AGG_CNT1_STRUC;
#endif


#define TX_AGG_CNT2	0x1728
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT2_STRUC {
	struct {
	    uint16_t  AggSize6Count;
	    uint16_t  AggSize5Count;
	} field;
	uint32_t word;
} TX_AGG_CNT2_STRUC;
#else
typedef	union _TX_AGG_CNT2_STRUC {
	struct {
	    uint16_t  AggSize5Count;
	    uint16_t  AggSize6Count;
	} field;
	uint32_t word;
} TX_AGG_CNT2_STRUC;
#endif


#define TX_AGG_CNT3	0x172c
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT3_STRUC {
	struct {
	    uint16_t  AggSize8Count;
	    uint16_t  AggSize7Count;
	} field;
	uint32_t word;
} TX_AGG_CNT3_STRUC;
#else
typedef	union _TX_AGG_CNT3_STRUC {
	struct {
	    uint16_t  AggSize7Count;
	    uint16_t  AggSize8Count;
	} field;
	uint32_t word;
} TX_AGG_CNT3_STRUC;
#endif


#define TX_AGG_CNT4	0x1730
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT4_STRUC {
	struct {
	    uint16_t  AggSize10Count;
	    uint16_t  AggSize9Count;
	} field;
	uint32_t word;
} TX_AGG_CNT4_STRUC;
#else
typedef	union _TX_AGG_CNT4_STRUC {
	struct {
	    uint16_t  AggSize9Count;
	    uint16_t  AggSize10Count;
	} field;
	uint32_t word;
} TX_AGG_CNT4_STRUC;
#endif


#define TX_AGG_CNT5	0x1734
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT5_STRUC {
	struct {
	    uint16_t  AggSize12Count;
	    uint16_t  AggSize11Count;
	} field;
	uint32_t word;
} TX_AGG_CNT5_STRUC;
#else
typedef	union _TX_AGG_CNT5_STRUC {
	struct {
	    uint16_t  AggSize11Count;
	    uint16_t  AggSize12Count;
	} field;
	uint32_t word;
} TX_AGG_CNT5_STRUC;
#endif


#define TX_AGG_CNT6		0x1738
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT6_STRUC {
	struct {
	    uint16_t  AggSize14Count;
	    uint16_t  AggSize13Count;
	} field;
	uint32_t word;
} TX_AGG_CNT6_STRUC;
#else
typedef	union _TX_AGG_CNT6_STRUC {
	struct {
	    uint16_t  AggSize13Count;
	    uint16_t  AggSize14Count;
	} field;
	uint32_t word;
} TX_AGG_CNT6_STRUC;
#endif


#define TX_AGG_CNT7		0x173c
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT7_STRUC {
	struct {
	    uint16_t  AggSize16Count;
	    uint16_t  AggSize15Count;
	} field;
	uint32_t word;
} TX_AGG_CNT7_STRUC;
#else
typedef	union _TX_AGG_CNT7_STRUC {
	struct {
	    uint16_t  AggSize15Count;
	    uint16_t  AggSize16Count;
	} field;
	uint32_t word;
} TX_AGG_CNT7_STRUC;
#endif


#define TX_AGG_CNT8	0x174c	/* AGG_SIZE  = 17,18 */
#define TX_AGG_CNT9	0x1750	/* AGG_SIZE  = 19,20 */
#define TX_AGG_CNT10	0x1754	/* AGG_SIZE  = 21,22 */
#define TX_AGG_CNT11	0x1758	/* AGG_SIZE  = 23,24 */
#define TX_AGG_CNT12	0x175c	/* AGG_SIZE  = 25,26 */
#define TX_AGG_CNT13	0x1760	/* AGG_SIZE  = 27,28 */
#define TX_AGG_CNT14	0x1764	/* AGG_SIZE  = 29,30 */
#define TX_AGG_CNT15	0x1768	/* AGG_SIZE  = 31,32 */
#define TX_AGG_CNT16	0x179C  /* AGG_SIZE = 33, 34 */
#define TX_AGG_CNT17	0x17a0	/* AGG_SIZE = 35, 36 */
#define TX_AGG_CNT18	0x17a4  /* AGG_SIZE = 37, 38 */
#define TX_AGG_CNT19	0x17a8  /* AGG_SIZE = 39, 40 */
#define TX_AGG_CNT20	0x17ac	/* AGG_SIZE = 41, 42 */
#define TX_AGG_CNT21	0x17b0	/* AGG_SIZE = 43, 44 */
#define TX_AGG_CNT22	0x17b4  /* AGG_SIZE = 45, 46 */
#define TX_AGG_CNT23	0x17b8  /* AGG_SIZE = 47, 48 */
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT_STRUC {
	struct {
	    uint16_t  AggCnt_y;	/* the count of aggregation size = x + 1 */
	    uint16_t  AggCnt_x;	/* the count of aggregation size = x */
	} field;
	uint32_t word;
} TX_AGG_CNT_STRUC;
#else
typedef	union _TX_AGG_CNT_STRUC {
	struct {
	    uint16_t  AggCnt_x;
	    uint16_t  AggCnt_y;
	} field;
	uint32_t word;
} TX_AGG_CNT_STRUC;
#endif

typedef	union _TX_AGG_CNTN_STRUC {
	struct {
#ifdef RT_BIG_ENDIAN
	    uint16_t  AggSizeHighCount;
	    uint16_t  AggSizeLowCount;
#else
	    uint16_t  AggSizeLowCount;
	    uint16_t  AggSizeHighCount;
#endif
	} field;
	uint32_t word;
} TX_AGG_CNTN_STRUC;


#define MPDU_DENSITY_CNT		0x1740
#ifdef RT_BIG_ENDIAN
typedef	union _MPDU_DEN_CNT_STRUC {
	struct {
	    uint16_t  RXZeroDelCount;	/*RX zero length delimiter count */
	    uint16_t  TXZeroDelCount;	/*TX zero length delimiter count */
	} field;
	uint32_t word;
} MPDU_DEN_CNT_STRUC;
#else
typedef	union _MPDU_DEN_CNT_STRUC {
	struct {
	    uint16_t  TXZeroDelCount;
	    uint16_t  RXZeroDelCount;
	} field;
	uint32_t word;
} MPDU_DEN_CNT_STRUC;
#endif


#ifdef FIFO_EXT_SUPPORT
/* TX_STA_FIFO_EXT_STRUC: TX retry cnt for specific frame */
#define TX_STA_FIFO_EXT		0x1798		/* Only work after RT53xx */
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_FIFO_EXT_STRUC {
	struct {
		uint32_t 	Reserve:16;
		uint32_t 	PidType:8;
		uint32_t 	txRtyCnt:8;   /* frame Tx retry cnt */
	} field;
	uint32_t word;
} TX_STA_FIFO_EXT_STRUC;
#else
typedef	union _TX_STA_FIFO_EXT_STRUC {
	struct {
		uint32_t 	txRtyCnt:8;
		uint32_t 	PidType:8;
		uint32_t 	Reserve:16;
	} field;
	uint32_t word;
} TX_STA_FIFO_EXT_STRUC;
#endif



#define WCID_TX_CNT_0	0x176c
#define WCID_TX_CNT_1	0x1770
#define WCID_TX_CNT_2	0x1774
#define WCID_TX_CNT_3	0x1778
#define WCID_TX_CNT_4	0x177c
#define WCID_TX_CNT_5	0x1780
#define WCID_TX_CNT_6	0x1784
#define WCID_TX_CNT_7	0x1788
#ifdef RT_BIG_ENDIAN
typedef	union _WCID_TX_CNT_STRUC {
	struct {
		uint32_t 	reTryCnt:16;
		uint32_t 	succCnt:16;
	} field;
	uint32_t word;
} WCID_TX_CNT_STRUC;
#else
typedef	union _WCID_TX_CNT_STRUC {
	struct {
		uint32_t 	succCnt:16;
		uint32_t 	reTryCnt:16;
	} field;
	uint32_t word;
} WCID_TX_CNT_STRUC;
#endif


#define WCID_MAPPING_0	0x178c
#define WCID_MAPPING_1	0x1790
#ifdef RT_BIG_ENDIAN
typedef	union _WCID_MAPPING_STRUC {
	struct {
		uint32_t 	wcid3:8;
		uint32_t 	wcid2:8;
		uint32_t 	wcid1:8;
		uint32_t 	wcid0:8;
	} field;
	uint32_t word;
} WCID_MAPPING_STRUC;
#else
typedef	union _WCID_MAPPING_STRUC {
	struct {
		uint32_t 	wcid0:8;
		uint32_t 	wcid1:8;
		uint32_t 	wcid2:8;
		uint32_t 	wcid3:8;
	} field;
	uint32_t word;
} WCID_MAPPINGT_STRUC;
#endif
#endif /* FIFO_EXT_SUPPORT */

#define TX_REPORT_CNT	0x1794


/* Security key table memory, base address = 0x1000 */
#define MAC_WCID_BASE		0x1800 /*8-bytes(use only 6-bytes) * 256 entry = */
#define HW_WCID_ENTRY_SIZE   8


#ifdef RT_BIG_ENDIAN
typedef	union _SHAREDKEY_MODE_STRUC {
	struct {
		uint32_t       Bss1Key3CipherAlg:4;
		uint32_t       Bss1Key2CipherAlg:4;
		uint32_t       Bss1Key1CipherAlg:4;
		uint32_t       Bss1Key0CipherAlg:4;
		uint32_t       Bss0Key3CipherAlg:4;
		uint32_t       Bss0Key2CipherAlg:4;
		uint32_t       Bss0Key1CipherAlg:4;
		uint32_t       Bss0Key0CipherAlg:4;
	} field;
	uint32_t word;
} SHAREDKEY_MODE_STRUC;
#else
typedef	union _SHAREDKEY_MODE_STRUC {
	struct {
		uint32_t       Bss0Key0CipherAlg:4;
		uint32_t       Bss0Key1CipherAlg:4;
		uint32_t       Bss0Key2CipherAlg:4;
		uint32_t       Bss0Key3CipherAlg:4;
		uint32_t       Bss1Key0CipherAlg:4;
		uint32_t       Bss1Key1CipherAlg:4;
		uint32_t       Bss1Key2CipherAlg:4;
		uint32_t       Bss1Key3CipherAlg:4;
	} field;
	uint32_t word;
} SHAREDKEY_MODE_STRUC;
#endif


/* 64-entry for pairwise key table, 8-byte per entry  */
typedef struct _HW_WCID_ENTRY {
    UINT8   Address[6];
    UINT8   Rsv[2];
} HW_WCID_ENTRY;


/* ================================================================================= */
/* WCID  format */
/* ================================================================================= */
/*7.1	WCID  ENTRY  format  : 8bytes */
typedef	struct _WCID_ENTRY_STRUC {
	UINT8		RXBABitmap7;    /* bit0 for TID8, bit7 for TID 15 */
	UINT8		RXBABitmap0;    /* bit0 for TID0, bit7 for TID 7 */
	UINT8		MAC[6];	/* 0 for shared key table.  1 for pairwise key table */
} WCID_ENTRY_STRUC;


/*8.1.1	SECURITY  KEY  format  : 8DW */
/* 32-byte per entry, total 16-entry for shared key table, 64-entry for pairwise key table */
typedef struct _HW_KEY_ENTRY {
    UINT8   Key[16];
    UINT8   TxMic[8];
    UINT8   RxMic[8];
} HW_KEY_ENTRY;


/*8.1.2	IV/EIV  format  : 2DW */

/* RX attribute entry format  : 1DW */
#ifdef RT_BIG_ENDIAN
typedef	union _WCID_ATTRIBUTE_STRUC {
	struct {
		uint32_t 	WAPIKeyIdx:8;
		uint32_t 	WAPI_rsv:8;
		uint32_t 	WAPI_MCBC:1;
		uint32_t 	rsv:3;
		uint32_t 	BSSIdxExt:1;
		uint32_t 	PairKeyModeExt:1;
		uint32_t 	RXWIUDF:3;
		uint32_t 	BSSIdx:3; /*multipleBSS index for the WCID */
		uint32_t 	PairKeyMode:3;
		uint32_t 	KeyTab:1;	/* 0 for shared key table.  1 for pairwise key table */
	} field;
	uint32_t word;
} WCID_ATTRIBUTE_STRUC;
#else
typedef	union _WCID_ATTRIBUTE_STRUC {
	struct {
		uint32_t 	KeyTab:1;	/* 0 for shared key table.  1 for pairwise key table */
		uint32_t 	PairKeyMode:3;
		uint32_t 	BSSIdx:3; 		/*multipleBSS index for the WCID */
		uint32_t 	RXWIUDF:3;
		uint32_t 	PairKeyModeExt:1;
		uint32_t 	BSSIdxExt:1;
		uint32_t 	rsv:3;
		uint32_t 	WAPI_MCBC:1;
		uint32_t 	WAPI_rsv:8;
		uint32_t 	WAPIKeyIdx:8;
	} field;
	uint32_t word;
} WCID_ATTRIBUTE_STRUC;
#endif


/* ================================================================================= */
/* HOST-MCU communication data structure */
/* ================================================================================= */

/* H2M_MAILBOX_CSR: Host-to-MCU Mailbox */
#ifdef RT_BIG_ENDIAN
typedef union _H2M_MAILBOX_STRUC {
    struct {
        uint32_t       Owner:8;
        uint32_t       CmdToken:8;    /* 0xff tells MCU not to report CmdDoneInt after excuting the command */
        uint32_t       HighByte:8;
        uint32_t       LowByte:8;
    } field;
    uint32_t word;
} H2M_MAILBOX_STRUC;
#else
typedef union _H2M_MAILBOX_STRUC {
    struct {
        uint32_t       LowByte:8;
        uint32_t       HighByte:8;
        uint32_t       CmdToken:8;
        uint32_t       Owner:8;
    } field;
    uint32_t word;
} H2M_MAILBOX_STRUC;
#endif


/* M2H_CMD_DONE_CSR: MCU-to-Host command complete indication */
#ifdef RT_BIG_ENDIAN
typedef union _M2H_CMD_DONE_STRUC {
    struct {
        uint32_t       CmdToken3;
        uint32_t       CmdToken2;
        uint32_t       CmdToken1;
        uint32_t       CmdToken0;
    } field;
    uint32_t word;
} M2H_CMD_DONE_STRUC;
#else
typedef union _M2H_CMD_DONE_STRUC {
    struct {
        uint32_t       CmdToken0;
        uint32_t       CmdToken1;
        uint32_t       CmdToken2;
        uint32_t       CmdToken3;
    } field;
    uint32_t word;
} M2H_CMD_DONE_STRUC;
#endif


/* HOST_CMD_CSR: For HOST to interrupt embedded processor */
#ifdef RT_BIG_ENDIAN
typedef	union _HOST_CMD_CSR_STRUC {
	struct {
	    uint32_t   Rsv:24;
	    uint32_t   HostCommand:8;
	} field;
	uint32_t word;
} HOST_CMD_CSR_STRUC;
#else
typedef	union _HOST_CMD_CSR_STRUC {
	struct {
	    uint32_t   HostCommand:8;
	    uint32_t   Rsv:24;
	} field;
	uint32_t word;
} HOST_CMD_CSR_STRUC;
#endif


// TODO: shiang-6590, Need to check following definitions are useful or not!!!
/* AIFSN_CSR: AIFSN for each EDCA AC */


/* E2PROM_CSR: EEPROM control register */
#ifdef RT_BIG_ENDIAN
typedef	union _E2PROM_CSR_STRUC {
	struct {
		uint32_t 	Rsvd:25;
		uint32_t       LoadStatus:1;   /* 1:loading, 0:done */
		uint32_t 	Type:1;			/* 1: 93C46, 0:93C66 */
		uint32_t 	EepromDO:1;
		uint32_t 	EepromDI:1;
		uint32_t 	EepromCS:1;
		uint32_t 	EepromSK:1;
		uint32_t 	Reload:1;		/* Reload EEPROM content, write one to reload, self-cleared. */
	} field;
	uint32_t word;
} E2PROM_CSR_STRUC;
#else
typedef	union _E2PROM_CSR_STRUC {
	struct {
		uint32_t 	Reload:1;
		uint32_t 	EepromSK:1;
		uint32_t 	EepromCS:1;
		uint32_t 	EepromDI:1;
		uint32_t 	EepromDO:1;
		uint32_t 	Type:1;
		uint32_t       LoadStatus:1;
		uint32_t 	Rsvd:25;
	} field;
	uint32_t word;
} E2PROM_CSR_STRUC;
#endif


/* QOS_CSR0: TXOP holder address0 register */
#ifdef RT_BIG_ENDIAN
typedef	union _QOS_CSR0_STRUC {
	struct {
		UINT8		Byte3;		/* MAC address byte 3 */
		UINT8		Byte2;		/* MAC address byte 2 */
		UINT8		Byte1;		/* MAC address byte 1 */
		UINT8		Byte0;		/* MAC address byte 0 */
	} field;
	uint32_t word;
} QOS_CSR0_STRUC;
#else
typedef	union _QOS_CSR0_STRUC {
	struct {
		UINT8		Byte0;
		UINT8		Byte1;
		UINT8		Byte2;
		UINT8		Byte3;
	} field;
	uint32_t word;
} QOS_CSR0_STRUC;
#endif


/* QOS_CSR1: TXOP holder address1 register */
#ifdef RT_BIG_ENDIAN
typedef	union _QOS_CSR1_STRUC {
	struct {
		UINT8		Rsvd1;
		UINT8		Rsvd0;
		UINT8		Byte5;		/* MAC address byte 5 */
		UINT8		Byte4;		/* MAC address byte 4 */
	} field;
	uint32_t word;
} QOS_CSR1_STRUC;
#else
typedef	union _QOS_CSR1_STRUC {
	struct {
		UINT8		Byte4;		/* MAC address byte 4 */
		UINT8		Byte5;		/* MAC address byte 5 */
		UINT8		Rsvd0;
		UINT8		Rsvd1;
	} field;
	uint32_t word;
} QOS_CSR1_STRUC;
#endif

// TODO: shiang-6590, check upper definitions are useful or not!



/* Other on-chip shared memory space, base = 0x2000 */

/* CIS space - base address = 0x2000 */
#define HW_CIS_BASE             0x2000

/* Carrier-sense CTS frame base address. It's where mac stores carrier-sense frame for carrier-sense function. */
#define HW_CS_CTS_BASE			0x7700
/* DFS CTS frame base address. It's where mac stores CTS frame for DFS. */
#define HW_DFS_CTS_BASE			0x7780
#define HW_CTS_FRAME_SIZE		0x80

/* 2004-11-08 john - since NULL frame won't be that long (256 byte). We steal 16 tail bytes */
/* to save debugging settings */
#define HW_DEBUG_SETTING_BASE   0x77f0  /* 0x77f0~0x77ff total 16 bytes */
#define HW_DEBUG_SETTING_BASE2  0x7770  /* 0x77f0~0x77ff total 16 bytes */


/*
	On-chip BEACON frame space -
   	1. HW_BEACON_OFFSET/64B must be 0;
   	2. BCN_OFFSETx(0~) must also be changed in MACRegTable(common/rtmp_init.c)
 */
#define HW_BEACON_OFFSET		0x0200

/* NullFrame buffer */
#define HW_NULL_BASE            0x7700
#define HW_NULL2_BASE			0x7780



/* 	In order to support maximum 8 MBSS and its maximum length is 512 for each beacon
	Three section discontinue memory segments will be used.
	1. The original region for BCN 0~3
	2. Extract memory from FCE table for BCN 4~5
	3. Extract memory from Pair-wise key table for BCN 6~7
  		It occupied those memory of wcid 238~253 for BCN 6
		and wcid 222~237 for BCN 7  	*/
/*#define HW_BEACON_MAX_COUNT     8 */
#define HW_BEACON_MAX_SIZE(__pAd)      ((__pAd)->chipCap.BcnMaxHwSize)
#define HW_BEACON_BASE0(__pAd)         ((__pAd)->chipCap.BcnBase[0])
/*#define HW_BEACON_BASE1         0x7A00 */
/*#define HW_BEACON_BASE2         0x7C00 */
/*#define HW_BEACON_BASE3         0x7E00 */
/*#define HW_BEACON_BASE4         0x7200 */
/*#define HW_BEACON_BASE5         0x7400 */
/*#define HW_BEACON_BASE6         0x5DC0 */
/*#define HW_BEACON_BASE7         0x5BC0 */


/* Higher 8KB shared memory */
#define HW_BEACON_BASE0_REDIRECTION	0x4000
#define HW_BEACON_BASE1_REDIRECTION	0x4200
#define HW_BEACON_BASE2_REDIRECTION	0x4400
#define HW_BEACON_BASE3_REDIRECTION	0x4600
#define HW_BEACON_BASE4_REDIRECTION	0x4800
#define HW_BEACON_BASE5_REDIRECTION	0x4A00
#define HW_BEACON_BASE6_REDIRECTION	0x4C00
#define HW_BEACON_BASE7_REDIRECTION	0x4E00


/* HOST-MCU shared memory - base address = 0x2100 */
#define HOST_CMD_CSR		0x404
#define H2M_MAILBOX_CSR         0x7010
#define H2M_MAILBOX_CID         0x7014
#define H2M_MAILBOX_STATUS      0x701c
#define H2M_INT_SRC             0x7024
#define H2M_BBP_AGENT           0x7028
#define M2H_CMD_DONE_CSR        0x000c
#define MCU_TXOP_ARRAY_BASE     0x000c   /* TODO: to be provided by Albert */
#define MCU_TXOP_ENTRY_SIZE     32       /* TODO: to be provided by Albert */
#define MAX_NUM_OF_TXOP_ENTRY   16       /* TODO: must be same with 8051 firmware */
#define MCU_MBOX_VERSION        0x01     /* TODO: to be confirmed by Albert */
#define MCU_MBOX_VERSION_OFFSET 5        /* TODO: to be provided by Albert */


/* Host DMA registers - base address 0x200 .  TX0-3=EDCAQid0-3, TX4=HCCA, TX5=MGMT, */
/*  DMA RING DESCRIPTOR */
#define E2PROM_CSR          0x0004
#define IO_CNTL_CSR         0x77d0



/* ================================================================ */
/* Tx /	Rx / Mgmt ring descriptor definition */
/* ================================================================ */

/* the following PID values are used to mark outgoing frame type in TXD->PID so that */
/* proper TX statistics can be collected based on these categories */
/* b3-2 of PID field - */
#define PID_MGMT			0x05
#define PID_BEACON			0x0c
#define PID_DATA_NORMALUCAST	 	0x02
#define PID_DATA_AMPDU	 	0x04
#define PID_DATA_NO_ACK    	0x08
#define PID_DATA_NOT_NORM_ACK	 	0x03
/* value domain of pTxD->HostQId (4-bit: 0~15) */
#define QID_AC_BK               1   /* meet ACI definition in 802.11e */
#define QID_AC_BE               0   /* meet ACI definition in 802.11e */
#define QID_AC_VI               2
#define QID_AC_VO               3
#define QID_HCCA                4
#define NUM_OF_TX_RING		5

#define NUM_OF_RX_RING		1
#undef NUM_OF_RX_RING
#define NUM_OF_RX_RING		2

#define QID_MGMT                13
#define QID_RX                  14
#define QID_OTHER               15
#define QID_CTRL				16




#define RTMP_MAC_SHR_MSEL_PROTECT_LOCK(__pAd, __IrqFlags)	__IrqFlags = __IrqFlags;
#define RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(__pAd, __IrqFlags) __IrqFlags = __IrqFlags;


#define SHAREDKEYTABLE			0
#define PAIRWISEKEYTABLE		1

struct rtmp_adapter;

INT get_pkt_phymode_by_rxwi(struct rtmp_adapter *pAd, RXWI_STRUC *rxwi);
INT get_pkt_rssi_by_rxwi(struct rtmp_adapter *pAd, RXWI_STRUC *rxwi, INT size, CHAR *rssi);
INT get_pkt_snr_by_rxwi(struct rtmp_adapter *pAd, RXWI_STRUC *rxwi, INT size, u8 *snr);

INT rtmp_mac_set_band(struct rtmp_adapter *pAd, int  band);
void mt7612u_mac_set_ctrlch(struct rtmp_adapter *pAd, u8 extch);
INT rtmp_mac_set_mmps(struct  rtmp_adapter *pAd, INT ReduceCorePower);

VOID rtmp_mac_bcn_buf_init(struct rtmp_adapter *pAd);

INT rtmp_mac_init(struct rtmp_adapter *pAd);

#endif /* __RTMP_MAC_H__ */

