/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ral_nmac.h

	Abstract:
	Ralink Wireless Chip RAL MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RAL_NMAC_H__
#define __RAL_NMAC_H__

enum INFO_TYPE {
	NORMAL_PACKET,
	CMD_PACKET,
};

enum D_PORT {
	WLAN_PORT,
	CPU_RX_PORT,
	CPU_TX_PORT,
	HOST_PORT,
	VIRTUAL_CPU_RX_PORT,
	VIRTUAL_CPU_TX_PORT,
	DISCARD,
};

#include "rtmp_type.h"

#ifdef RT_BIG_ENDIAN
struct __attribute__ ((packed)) mt7612_txinfo_pkt {
	uint32_t info_type:2;
	uint32_t d_port:3;
	uint32_t QSEL:2;
	uint32_t wiv:1;
	uint32_t rsv1:2;
	uint32_t cso:1;
	uint32_t tso:1;
	uint32_t pkt_80211:1;
	uint32_t sw_lst_rnd:1;
	uint32_t tx_burst:1;
	uint32_t next_vld:1;
	uint32_t pkt_len:16;
};
#else
struct __attribute__ ((packed)) mt7612_txinfo_pkt {
	uint32_t pkt_len:16;
	uint32_t next_vld:1;
	uint32_t tx_burst:1;
	uint32_t sw_lst_rnd:1;
	uint32_t pkt_80211:1;
	uint32_t tso:1;
	uint32_t cso:1;
	uint32_t rsv1:2;
	uint32_t wiv:1;
	uint32_t QSEL:2;
	uint32_t d_port:3;
	uint32_t info_type:2;
};
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _LED_NMAC_CMD{
	uint32_t  rsv:8;
	uint32_t CmdID:8;
	uint32_t Arg0:8;
	uint32_t Arg1:8;
}LED_NMAC_CMD;
#else
typedef struct GNU_PACKED _LED_NMAC_CMD{
	uint32_t Arg1:8;
	uint32_t Arg0:8;
	uint32_t CmdID:8;
	uint32_t rsv:8;
}LED_NMAC_CMD;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
struct  __attribute__ ((packed)) mt7612u_txwi {
	/* Word 0 */
	uint32_t 	PHYMODE:3;
	uint32_t 	iTxBF:1;
	uint32_t 	eTxBF:1;
	uint32_t 	STBC:1;
	uint32_t 	ShortGI:1;
	uint32_t 	BW:2;			/* channel bandwidth 20/40/80 MHz */
	uint32_t 	LDPC:1;
	uint32_t 	MCS:6;

	uint32_t 	lut_en:1;
	uint32_t 	Sounding:1;
	uint32_t 	NDPSndBW:2;	/* NDP sounding BW */
	uint32_t 	RTSBWSIG:1;
	uint32_t 	NDPSndRate:1;	/* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	uint32_t 	txop:2;

	uint32_t 	MpduDensity:3;
	uint32_t 	AMPDU:1;
	uint32_t 	TS:1;
	uint32_t 	CFACK:1;
	uint32_t 	MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	uint32_t 	FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */

	/* Word 1 */
	uint32_t 	TIM:1;
	uint32_t 	TXBF_PT_SCA:1;
	uint32_t 	MPDUtotalByteCnt:14;
	uint32_t 	wcid:8;
	uint32_t 	BAWinSize:6;
	uint32_t 	NSEQ:1;
	uint32_t 	ACK:1;

	/* Word 2 */
	uint32_t 	IV;
	/* Word 3 */
	uint32_t 	EIV;

	/* Word 4 */
	uint32_t 	TxPktId:8;
	uint32_t     GroupID:1;
	uint32_t     Rsv4:3;
	//uint32_t 	Rsv4:4;
	uint32_t 	TxPwrAdj:4;
	uint32_t 	TxStreamMode:8;
	uint32_t 	TxEAPId:8;
};
#else
struct  __attribute__ ((packed)) mt7612u_txwi {
	/* Word	0 */
	/* ex: 00 03 00 40 means txop = 3, PHYMODE = 1 */
	uint32_t 	FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	uint32_t 	MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	uint32_t 	CFACK:1;
	uint32_t 	TS:1;
	uint32_t 	AMPDU:1;
	uint32_t 	MpduDensity:3;

	uint32_t 	txop:2;
	uint32_t 	NDPSndRate:1; /* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	uint32_t 	RTSBWSIG:1;
	uint32_t 	NDPSndBW:2; /* NDP sounding BW */
	uint32_t 	Sounding:1;
	uint32_t 	lut_en:1;

	uint32_t 	MCS:6;
	uint32_t 	LDPC:1;
	uint32_t 	BW:2;		/*channel bandwidth 20/40/80 MHz */
	uint32_t 	ShortGI:1;
	uint32_t 	STBC:1;
	uint32_t 	eTxBF:1;
	uint32_t 	iTxBF:1;
	uint32_t 	PHYMODE:3;

	/* Word1 */
	/* ex:  1c ff 38 00 means ACK=0, BAWinSize=7, MPDUtotalByteCnt = 0x38 */
	uint32_t 	ACK:1;
	uint32_t 	NSEQ:1;
	uint32_t 	BAWinSize:6;
	uint32_t 	wcid:8;
	uint32_t 	MPDUtotalByteCnt:14;
	uint32_t 	TXBF_PT_SCA:1;
	uint32_t      TIM:1;

	/*Word2 */
	uint32_t 	IV;

	/*Word3 */
	uint32_t 	EIV;

	/* Word 4 */
	uint32_t 	TxEAPId:8;
	uint32_t 	TxStreamMode:8;
	uint32_t 	TxPwrAdj:4;
	//uint32_t 	Rsv4:4;
	uint32_t     Rsv4:3;
	uint32_t     GroupID:1;
	uint32_t 	TxPktId:8;
};
#endif

/*
	Rx Memory layout:

	1. Rx Descriptor
		Rx Descriptor(12 Bytes) + RX_FCE_Info(4 Bytes)
	2. Rx Buffer
		RxInfo(4 Bytes) + RXWI() + 802.11 packet
*/


#ifdef RT_BIG_ENDIAN
struct __attribute__ ((packed)) mt7612u_struct mt7612u_rxfce_info_pkt {
	uint32_t info_type:2;
	uint32_t s_port:3;
	uint32_t qsel:2;
	uint32_t pcie_intr:1;

	uint32_t mac_len:3;
	uint32_t l3l4_done:1;
	uint32_t pkt_80211:1;
	uint32_t ip_err:1;
	uint32_t tcp_err:1;
	uint32_t udp_err:1;

	uint32_t rsv:2;
	uint32_t pkt_len:14;
};
#else
struct __attribute__ ((packed)) mt7612u_rxfce_info_pkt{
	uint32_t pkt_len:14;
	uint32_t rsv:2;

	uint32_t udp_err:1;
	uint32_t tcp_err:1;
	uint32_t ip_err:1;
	uint32_t pkt_80211:1;
	uint32_t l3l4_done:1;
	uint32_t mac_len:3;

	uint32_t pcie_intr:1;
	uint32_t qsel:2;
	uint32_t s_port:3;
	uint32_t info_type:2;
};
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
struct __attribute__ ((packed)) mt7612u_rxfce_info_cmd {
	uint32_t info_type:2;
	uint32_t d_port:3;
	uint32_t qsel:2;
	uint32_t pcie_intr:1;
	uint32_t evt_type:4;
	uint32_t cmd_seq:4;
	uint32_t self_gen:1;
	uint32_t rsv:1;
	uint32_t pkt_len:14;
};
#else
struct __attribute__ ((packed)) mt7612u_rxfce_info_cmd {
	uint32_t pkt_len:14;
	uint32_t rsv:1;
	uint32_t self_gen:1;
	uint32_t cmd_seq:4;
	uint32_t evt_type:4;
	uint32_t pcie_intr:1;
	uint32_t qsel:2;
	uint32_t d_port:3;
	uint32_t info_type:2;
};
#endif



/*
	RXWI wireless information format.
*/
#ifdef RT_BIG_ENDIAN
struct __attribute__ ((packed)) mt7612u_rxwi {
	/* Word 0 */
	uint32_t eof:1;
	uint32_t rsv:1;
	uint32_t MPDUtotalByteCnt:14; /* = rxfceinfo_len - rxwi_len- rxinfo_len - l2pad */
	uint32_t udf:3;
	uint32_t bss_idx:3;
	uint32_t key_idx:2;
	uint32_t wcid:8;

	/* Word 1 */
	uint32_t phy_mode:3;
	uint32_t rsv_1:1;
	uint32_t ldpc_ex_sym:1;
	uint32_t stbc:1;
	uint32_t sgi:1;
	uint32_t bw:2;
	uint32_t ldpc:1;
	uint32_t mcs:6;
	uint32_t sn:12;
	uint32_t tid:4;

	/* Word 2 */
	UINT8 rssi[4];

	/* Word 3~6 */
	UINT8 bbp_rxinfo[16];
};
#else
struct __attribute__ ((packed)) mt7612u_rxwi {
	/* Word 0 */
	uint32_t wcid:8;
	uint32_t key_idx:2;
	uint32_t bss_idx:3;
	uint32_t udf:3;
	uint32_t MPDUtotalByteCnt:14;
	uint32_t rsv:1;
	uint32_t eof:1;

	/* Word 1 */
	uint32_t tid:4;
	uint32_t sn:12;
	uint32_t mcs:6;
	uint32_t ldpc:1;
	uint32_t bw:2;
	uint32_t sgi:1;
	uint32_t stbc:1;
	uint32_t ldpc_ex_sym:1;
	uint32_t rsv_1:1;
	uint32_t phy_mode:3;

	/* Word 2 */
	UINT8 rssi[4];

	/* Word 3~6 */
	UINT8 bbp_rxinfo[16];
};
#endif /* RT_BIG_ENDIAN */




typedef struct GNU_PACKED _NMAC_HW_RATE_CTRL_STRUCT{
#ifdef RT_BIG_ENDIAN
	uint16_t PHYMODE:3;
	uint16_t iTxBF:1;
	uint16_t eTxBF:1;
	uint16_t STBC:1;
	uint16_t ShortGI:1;
	uint16_t BW:2;			/* channel bandwidth 20/40/80 MHz */
	uint16_t ldpc:1;
	uint16_t MCS:6;
#else
	uint16_t MCS:6;
	uint16_t ldpc:1;
	uint16_t BW:2;
	uint16_t ShortGI:1;
	uint16_t STBC:1;
	uint16_t eTxBF:1;
	uint16_t iTxBF:1;
	uint16_t PHYMODE:3;
#endif /* RT_BIG_ENDIAN */
}NMAC_HW_RATE_CTRL_STRUCT;




/* ================================================================================= */
/* Register format */
/* ================================================================================= */
#define CMB_CTRL 0x0020
#define CMB_CTRL_AUX_OPT_MASK (0xffff)
#define CMB_CTRL_AUX_OPT_ANTSEL (1 << 12)
#define CMB_CTRL_AUX_OPT_GPIO_SETTING (1 << 14)
#define CMB_CTRL_CSR_UART_MODE (1 << 16)
#define CMB_CTRL_GPIO_MODE_LED1 (1 << 17)
#define CMB_CTRL_GPIO_MODE_LED2 (1 << 18)
#define CMB_CTRL_FOR_CLK_XTAL (1 << 19)
#define CMB_CTRL_CSR_UART_NFC (1 << 20)
#define CMB_CTRL_XTAL_RDY (1 << 22)
#define CMB_CTRL_PLL_LD (1 << 23)

#define COEXCFG0 0x0040
#define COEXCFG0_COEX_EN (1 << 0)
#define COEXCFG0_FIX_WL_DI_ANT (1 << 1)
#define COEXCFG0_FIX_WL_ANT_EN (1 << 2)
#define COEXCFG0_FIX_WL_TX_PWR_MASK (0x3 << 6)
#define COEXCFG0_FIX_WL_GCRF_EN (1 << 8)
#define COEXCFG0_FIX_WL_RF_LNA0_MASK (0x3 << 12)
#define COEXCFG0_FIX_WL_RF_LNA1_MASK (0x3 << 14)
#define COEXCFG0_COEX_CFG0_MASK (0xff << 16)
#define COEXCFG0_COEX_CFG1_MASK (0xff << 24)

#define COEXCFG1 0x0044
#define COEXCFG1_DIS_WL_TR_DELAY_MASK (0xff << 0)
#define COEXCFG1_DIS_WL_PA_DELAY_MASK (0xff << 8)
#define COEXCFG1_DIS_WL_RF_DELAY_MASK (0xff << 16)
#define COEXCFG1_FIX_WL_TX_GCRF0_MASK (0xf << 24)
#define COEXCFG1_FIX_WL_TX_GCRF1_MASK (0xf << 28)

#define COEXCFG2 0x0048
#define COEXCFG2_WL_COEX_CFG0_MASK (0xff << 0)
#define COEXCFG2_WL_COEX_CFG1_MASK (0xff << 8)
#define COEXCFG2_BT_COEX_CFG0_MASK (0xff << 16)
#define COEXCFG2_BT_COEX_CFG1_MASK (0xff << 24)

#define COEXCFG3 0x004c
#define COEXCFG3_COEX_VER (1 << 0)
#define COEXCFG3_TDM_EN (1 << 1)
#define COEXCFG3_IO_TR_SW0_MODE (1 << 2)
#define COEXCFG3_CSR_FRC_TR_SW0 (1 << 3)
#define COEXCFG3_FIX_IO_ANT_SEL_EN (1 << 4)
#define COEXCFG3_REG_IO_ANT_SEL_EN (1 << 5)
#define COEXCFG3_PTA_CNT_CLEAR (1 << 6)
#define COEXCFG3_PTA_CNT_EN (1 << 7)
#define COEXCFG3_BT_TX_STATUS (1 << 16)

#define MISC_CTRL	0x64

#define WLAN_FUN_CTRL		0x80
#define WLAN_FUN_CTRL_WLAN_EN (1 << 0)
#define WLAN_FUN_CTRL_WLAN_CLK_EN (1 << 1)
#define WLAN_FUN_CTRL_WLAN_RESET_RF (1 << 2)

/* MT76x0 definition */
#define WLAN_FUN_CTRL_WLAN_RESET (1 << 3)

/* MT76x2 definition */
#define WLAN_FUN_CTRL_CSR_F20M_CKEN (1 << 3)

#define WLAN_FUN_CTRL_PCIE_APP0_CLK_REQ (1 << 4)
#define WLAN_FUN_CTRL_FRC_WL_ANT_SEL (1 << 5)
#define WLAN_FUN_CTRL_INV_ANT_SEL (1 << 6)
#define WLAN_FUN_CTRL_WAKE_HOST_F0 (1 << 7)

/* MT76x0 definition */
#define WLAN_FUN_CTRL_GPIO0_IN_MASK (0xff << 8)
#define WLAN_FUN_CTRL_GPIO0_OUT_MASK (0xff << 16)
#define WLAN_FUN_CTRL_GPIO0_OUT_OE_N_MASK (0xff << 24)

/* MT76x2 definition */
#define WLAN_FUN_CTRL_THERM_RST (1 << 8)
#define WLAN_FUN_CTRL_THERM_CKEN (1 << 9)

#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		uint32_t GPIO0_OUT_OE_N:8;
		uint32_t GPIO0_OUT:8;
		uint32_t GPIO0_IN:8;
		uint32_t WLAN_ACC_BT:1;
		uint32_t INV_TR_SW0:1;
		uint32_t FRC_WL_ANT_SET:1;
		uint32_t PCIE_APP0_CLK_REQ:1;
		uint32_t WLAN_RESET:1;
		uint32_t WLAN_RESET_RF:1;
		uint32_t WLAN_CLK_EN:1;
		uint32_t WLAN_EN:1;
	}field;
	uint32_t word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#else
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		uint32_t WLAN_EN:1;
		uint32_t WLAN_CLK_EN:1;
		uint32_t WLAN_RESET_RF:1;
		uint32_t WLAN_RESET:1;
		uint32_t PCIE_APP0_CLK_REQ:1;
		uint32_t FRC_WL_ANT_SET:1;
		uint32_t INV_TR_SW0:1;
		uint32_t WLAN_ACC_BT:1;
		uint32_t GPIO0_IN:8;
		uint32_t GPIO0_OUT:8;
		uint32_t GPIO0_OUT_OE_N:8;
	}field;
	uint32_t word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#endif


#define WLAN_FUN_INFO		0x84
#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		uint32_t 	BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
		uint32_t 	Rsv1:26;
		uint32_t 	COEX_MODE:5; /* WLAN function enable */
	}field;
	uint32_t word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#else
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		uint32_t 	COEX_MODE:5; /* WLAN function enable */
		uint32_t 	Rsv1:26;
		uint32_t 	BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
	}field;
	uint32_t word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#endif


#define BT_FUN_CTRL		0xC0
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_CTRL_STRUC{
	struct{
		uint32_t 	GPIO1_OUT_OE_N:8;
		uint32_t 	GPIO1_OUT:8;
		uint32_t 	GPIO1_IN:8;
		uint32_t 	BT_ACC_WLAN:1;
		uint32_t 	INV_TR_SW1:1;
		uint32_t 	URXD_GPIO_MODE:1;
		uint32_t 	PCIE_APP1_CLK_REQ:1;
		uint32_t 	BT_RESET:1;
		uint32_t 	BT_RF_EN:1;
		uint32_t 	BT_CLK_EN:1;
		uint32_t 	BT_EN:1;
	}field;
	uint32_t word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#else
typedef union _BT_FUN_CTRL_STRUC	{
	struct{
		uint32_t 	BT_EN:1;
		uint32_t 	BT_CLK_EN:1;
		uint32_t 	BT_RF_EN:1;
		uint32_t 	BT_RESET:1;
		uint32_t 	PCIE_APP1_CLK_REQ:1;
		uint32_t 	URXD_GPIO_MODE:1;
		uint32_t 	INV_TR_SW1:1;
		uint32_t 	BT_ACC_WLAN:1;
		uint32_t 	GPIO1_IN:8;
		uint32_t 	GPIO1_OUT:8;
		uint32_t 	GPIO1_OUT_OE_N:8;
	}field;
	uint32_t word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#endif


#define BT_FUN_INFO		0xC4
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_INFO_STRUC{
	struct{
		uint32_t 	WLAN_EEP_BUSY:1;
		uint32_t 	BTPower1:7;	/* Peer */
		uint32_t 	BTPower0:8; /* Self */
		uint32_t 	AFH_END_CH:8;
		uint32_t 	AFH_START_CH:8;
	}field;
	uint32_t word;
} BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#else
typedef	union _BT_FUN_INFO_STRUC	{
	struct{
		uint32_t 	AFH_START_CH:8;
		uint32_t 	AFH_END_CH:8;
		uint32_t 	BTPower0:8;	/* Self */
		uint32_t 	BTPower1:7;	/* Peer */
		uint32_t 	WLAN_EEP_BUSY:1;
	}field;
	uint32_t word;
}BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#endif

// TODO: shiang, this data structure is not defined for register. may can move to other place
typedef struct _WLAN_BT_COEX_SETTING
{
	bool					ampduOff;
	bool					coexSettingRunning;
	bool					RateSelectionForceToUseRSSI;
	u8 				TxQualityFlag;
	ULONG					alc;
	ULONG					slna;
}WLAN_BT_COEX_SETTING, *PWLAN_BT_COEX_SETTING;

#define XO_CTRL0 0x0100
#define XO_CTRL1 0x0104
#define XO_CTRL2 0x0108
#define XO_CTRL3 0x010C
#define XO_CTRL4 0x0110
#define XO_CTRL5 0x0114
#define XO_CTRL6 0x0118

#define CLK_ENABLE 0x014C

#define MCU_CMD_CFG 0x0234


#define TSO_CTRL	0x0250
#ifdef RT_BIG_ENDIAN
typedef union _TSO_CTRL_STRUC {
	struct {
		uint32_t rsv:13;
		uint32_t TSO_WR_LEN_EN:1;
		uint32_t TSO_SEG_EN:1;
		uint32_t TSO_EN:1;
		uint32_t RXWI_LEN:4;
		uint32_t RX_L2_FIX_LEN:4;
		uint32_t TXWI_LEN:4;
		uint32_t TX_L2_FIX_LEN:4;
	} field;
	uint32_t word;
} TSO_CTRL_STRUC;
#else
typedef union _TSO_CTRL_STRUC {
	struct {
		uint32_t TX_L2_FIX_LEN:4;
		uint32_t TXWI_LEN:4;
		uint32_t RX_L2_FIX_LEN:4;
		uint32_t RXWI_LEN:4;
		uint32_t TSO_EN:1;
		uint32_t TSO_SEG_EN:1;
		uint32_t TSO_WR_LEN_EN:1;
		uint32_t rsv:13;
	} field;
	uint32_t word;
} TSO_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */


#define TX_PROT_CFG6    0x13E0    // VHT 20 Protection
#define TX_PROT_CFG7    0x13E4    // VHT 40 Protection
#define TX_PROT_CFG8    0x13E8    // VHT 80 Protection
#define PIFS_TX_CFG     0x13EC    // PIFS setting

//----------------------------------------------------------------
// Header Translation
//----------------------------------------------------------------

/*
	header translation control register
	bit0 --> TX translation enable
	bit1 --> RX translation enable
*/
#define HEADER_TRANS_CTRL_REG	0x0260
#define HT_TX_ENABLE			0x1
#define HT_RX_ENABLE			0x2

#define HT_MAC_ADDR_DW0		0x02A4
#define HT_MAC_ADDR_DW1		0x02A8
#define HT_MAC_BSSID_DW0		0x02AC
#define HT_MAC_BSSID_DW1		0x02B0

#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HDR_TRANS_CTRL_STRUC {
	struct {
    	uint32_t Rsv:30;
    	uint32_t Rx_En:1;
    	uint32_t Tx_En:1;
	} field;
	uint32_t word;
} HDR_TRANS_CTRL_STRUC, *PHDR_TRANS_CTRL_STRUC;
#else
typedef union GNU_PACKED _HDR_TRANS_CTRL_STRUC {
	struct {
    	uint32_t Tx_En:1;
    	uint32_t Rx_En:1;
    	uint32_t Rsv:30;
	} field;
	uint32_t word;
} HDR_TRANS_CTRL_STRUC, *PHDR_TRANS_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX header translation enable by WCID */
#define HT_RX_WCID_EN_BASE	0x0264
#define HT_RX_WCID_OFFSET	32
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_RX_WCID_EN_STRUC {
	struct {
    	uint32_t RX_WCID31_TRAN_EN:1;
    	uint32_t RX_WCID30_TRAN_EN:1;
    	uint32_t RX_WCID29_TRAN_EN:1;
    	uint32_t RX_WCID28_TRAN_EN:1;
    	uint32_t RX_WCID27_TRAN_EN:1;
    	uint32_t RX_WCID26_TRAN_EN:1;
    	uint32_t RX_WCID25_TRAN_EN:1;
    	uint32_t RX_WCID24_TRAN_EN:1;
    	uint32_t RX_WCID23_TRAN_EN:1;
    	uint32_t RX_WCID22_TRAN_EN:1;
    	uint32_t RX_WCID21_TRAN_EN:1;
    	uint32_t RX_WCID20_TRAN_EN:1;
    	uint32_t RX_WCID19_TRAN_EN:1;
    	uint32_t RX_WCID18_TRAN_EN:1;
    	uint32_t RX_WCID17_TRAN_EN:1;
    	uint32_t RX_WCID16_TRAN_EN:1;
    	uint32_t RX_WCID15_TRAN_EN:1;
    	uint32_t RX_WCID14_TRAN_EN:1;
    	uint32_t RX_WCID13_TRAN_EN:1;
    	uint32_t RX_WCID12_TRAN_EN:1;
    	uint32_t RX_WCID11_TRAN_EN:1;
    	uint32_t RX_WCID10_TRAN_EN:1;
    	uint32_t RX_WCID9_TRAN_EN:1;
    	uint32_t RX_WCID8_TRAN_EN:1;
    	uint32_t RX_WCID7_TRAN_EN:1;
    	uint32_t RX_WCID6_TRAN_EN:1;
    	uint32_t RX_WCID5_TRAN_EN:1;
    	uint32_t RX_WCID4_TRAN_EN:1;
    	uint32_t RX_WCID3_TRAN_EN:1;
    	uint32_t RX_WCID2_TRAN_EN:1;
    	uint32_t RX_WCID1_TRAN_EN:1;
    	uint32_t RX_WCID0_TRAN_EN:1;
	} field;
	uint32_t word;
} HT_RX_WCID_EN_STRUC, *PHT_RX_WCID_EN_STRUC;
#else
typedef union GNU_PACKED _HT_RX_WCID_EN_STRUC {
	struct {
    	uint32_t RX_WCID0_TRAN_EN:1;
    	uint32_t RX_WCID1_TRAN_EN:1;
    	uint32_t RX_WCID2_TRAN_EN:1;
    	uint32_t RX_WCID3_TRAN_EN:1;
    	uint32_t RX_WCID4_TRAN_EN:1;
    	uint32_t RX_WCID5_TRAN_EN:1;
    	uint32_t RX_WCID6_TRAN_EN:1;
    	uint32_t RX_WCID7_TRAN_EN:1;
    	uint32_t RX_WCID8_TRAN_EN:1;
    	uint32_t RX_WCID9_TRAN_EN:1;
    	uint32_t RX_WCID10_TRAN_EN:1;
    	uint32_t RX_WCID11_TRAN_EN:1;
    	uint32_t RX_WCID12_TRAN_EN:1;
    	uint32_t RX_WCID13_TRAN_EN:1;
    	uint32_t RX_WCID14_TRAN_EN:1;
    	uint32_t RX_WCID15_TRAN_EN:1;
    	uint32_t RX_WCID16_TRAN_EN:1;
    	uint32_t RX_WCID17_TRAN_EN:1;
    	uint32_t RX_WCID18_TRAN_EN:1;
    	uint32_t RX_WCID19_TRAN_EN:1;
    	uint32_t RX_WCID20_TRAN_EN:1;
    	uint32_t RX_WCID21_TRAN_EN:1;
    	uint32_t RX_WCID22_TRAN_EN:1;
    	uint32_t RX_WCID23_TRAN_EN:1;
    	uint32_t RX_WCID24_TRAN_EN:1;
    	uint32_t RX_WCID25_TRAN_EN:1;
    	uint32_t RX_WCID26_TRAN_EN:1;
    	uint32_t RX_WCID27_TRAN_EN:1;
    	uint32_t RX_WCID28_TRAN_EN:1;
    	uint32_t RX_WCID29_TRAN_EN:1;
    	uint32_t RX_WCID30_TRAN_EN:1;
    	uint32_t RX_WCID31_TRAN_EN:1;
	} field;
	uint32_t word;
} HT_RX_WCID_EN_STRUC, *PHT_RX_WCID_EN_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX header translation - black list */
#define HT_RX_BL_BASE		0x0284
#define HT_RX_BL_OFFSET		2
#define HT_RX_BL_SIZE		8
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_RX_BLACK_LIST_STRUC {
	struct {
    	uint32_t BLACK_ETHER_TYPE1:16;
    	uint32_t BLACK_ETHER_TYPE0:16;
	} field;
	uint32_t word;
} HT_RX_BLACK_LIST_STRUC, *PHT_RX_BLACK_LIST_STRUC;
#else
typedef union GNU_PACKED _HT_RX_BLACK_LIST_STRUC {
	struct {
    	uint32_t BLACK_ETHER_TYPE0:16;
    	uint32_t BLACK_ETHER_TYPE1:16;
	} field;
	uint32_t word;
} HT_RX_BLACK_LIST_STRUC, *PHT_RX_BLACK_LIST_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX VLAN Mapping (TCI) */
#define HT_BSS_VLAN_BASE	0x0294
#define HT_BSS_VLAN_OFFSET	2
#define HT_BSS_VLAN_SIZE	8
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_BSS_VLAN_STRUC {
	struct {
    	uint32_t TCI1_VID:12;
    	uint32_t TCI1_CFI:1;
    	uint32_t TCI1_PCP:3;
    	uint32_t TCI0_VID:12;
    	uint32_t TCI0_CFI:1;
    	uint32_t TCI0_PCP:3;
	} field;
	uint32_t word;
} HT_BSS_VLAN_STRUC, *PHT_BSS_VLAN_STRUC;
#else
typedef union GNU_PACKED _HT_BSS_VLAN_STRUC {
	struct {
    	uint32_t TCI0_PCP:3;
    	uint32_t TCI0_CFI:1;
    	uint32_t TCI0_VID:12;
    	uint32_t TCI1_PCP:3;
    	uint32_t TCI1_CFI:1;
    	uint32_t TCI1_VID:12;
	} field;
	uint32_t word;
} HT_BSS_VLAN_STRUC, *PHT_BSS_VLAN_STRUC;
#endif /* RT_BIG_ENDIAN */


// TODO: shiang-6590, following definitions are dummy and not used for RT6590, shall remove/correct these!
#define GPIO_CTRL_CFG	0x0228
#define RLT_PBF_MAX_PCNT	0x0408 //TX_MAX_PCNT
// TODO:shiang-6590 --------------------------


#define RF_MISC	0x0518
#ifdef RT_BIG_ENDIAN
typedef union _RF_MISC_STRUC{
	struct{
		uint32_t Rsv1:29;
		uint32_t EXT_PA_EN:1;
		uint32_t ADDAC_LDO_ADC9_EN:1;
		uint32_t ADDAC_LDO_ADC6_EN:1;
	}field;
	uint32_t word;
}RF_MISC_STRUC, *PRF_MISC_STRUC;
#else
typedef union _RF_MISC_STRUC{
	struct{
		uint32_t ADDAC_LDO_ADC6_EN:1;
		uint32_t ADDAC_LDO_ADC9_EN:1;
		uint32_t EXT_PA_EN:1;
		uint32_t Rsv1:29;
	}field;
	uint32_t word;
}RF_MISC_STRUC, *PRF_MISC_STRUC;
#endif

#define RF_BSI_CKDIV    0x520

#define PAMODE_PWR_ADJ0 0x1228
#define PAMODE_PWR_ADJ1 0x122c
#define DACCLK_EN_DLY_CFG	0x1264

#define RLT_PAIRWISE_KEY_TABLE_BASE     0x8000      /* 32-byte * 256-entry =  -byte */
#define RLT_HW_KEY_ENTRY_SIZE           0x20

#define RLT_PAIRWISE_IVEIV_TABLE_BASE     0xa000      /* 8-byte * 256-entry =  -byte */
#define RLT_MAC_IVEIV_TABLE_BASE     0xa000      /* 8-byte * 256-entry =  -byte */
#define RLT_HW_IVEIV_ENTRY_SIZE   8

#define RLT_MAC_WCID_ATTRIBUTE_BASE     0xa800      /* 4-byte * 256-entry =  -byte */
#define RLT_HW_WCID_ATTRI_SIZE   4

#define RLT_SHARED_KEY_TABLE_BASE       0xac00      /* 32-byte * 16-entry = 512-byte */
#define RLT_SHARED_KEY_MODE_BASE       0xb000      /* 32-byte * 16-entry = 512-byte */

#define RLT_HW_SHARED_KEY_MODE_SIZE   4

#define RLT_SHARED_KEY_TABLE_BASE_EXT      0xb400      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */
#define RLT_SHARED_KEY_MODE_BASE_EXT       0xb3f0      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */

/* This resgiser is ONLY be supported for RT3883 or later.
   It conflicted with BCN#0 offset of previous chipset. */
#define RLT_WAPI_PN_TABLE_BASE		0xb800
#define RLT_WAPI_PN_ENTRY_SIZE   		8

struct rtmp_adapter;
struct mt7612u_rxinfo;
struct mt7612_txinfo_pkt;
struct mt7612u_txwi;

VOID dump_rlt_rxinfo(struct rtmp_adapter *pAd, struct mt7612u_rxinfo *pRxInfo);
VOID dump_rlt_txinfo(struct rtmp_adapter *pAd, struct mt7612_txinfo_pkt *pTxInfo);
VOID dump_rlt_txwi(struct rtmp_adapter *pAd, struct mt7612u_txwi *pTxWI);
VOID dump_rlt_rxwi(struct rtmp_adapter *pAd, struct mt7612u_rxwi *pRxWI);
VOID dumpRxFCEInfo(struct rtmp_adapter *pAd, struct mt7612u_rxfce_info_pkt *pRxFceInfo);

INT rlt_get_rxwi_phymode(struct mt7612u_rxwi *rxwi);
INT rlt_get_rxwi_rssi(struct mt7612u_rxwi *rxwi, INT size, CHAR *rssi);
INT rlt_get_rxwi_snr(struct rtmp_adapter *pAd, struct mt7612u_rxwi *rxwi, INT size, u8 *snr);
VOID rlt_asic_init_txrx_ring(struct rtmp_adapter *pAd);

int mt7612u_chip_onoff(struct rtmp_adapter *pAd, bool enable, bool reset);

#endif /* __RAL_NMAC_H__ */

