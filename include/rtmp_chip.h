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
	rtmp_chip.h

	Abstract:
	Ralink Wireless Chip related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef	__RTMP_CHIP_H__
#define	__RTMP_CHIP_H__

#include "rtmp_type.h"

struct rtmp_adapter;
struct _RSSI_SAMPLE;

#include "mac_ral/pbf.h"

#include "eeprom.h"


#ifdef RTMP_MAC_USB
#include "mac_ral/rtmp_mac.h"
#include "mac_ral/mac_usb.h"
#endif /* RTMP_MAC_USB */





















#include "chip/rt65xx.h"


#include "chip/mt76x2.h"


#include "mcu/mcu.h"


#define IS_RT8592(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x85590000)

#define IS_MT7601(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x76010000)
#define IS_MT7601U(_pAd)	(IS_MT7601(_pAd) && (IS_USB_INF(_pAd)))

#define IS_MT7650(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76500000)
#define IS_MT7650U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76500000) && (IS_USB_INF(_pAd)))
#define IS_MT7630(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76300000)
#define IS_MT7630U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76300000) && (IS_USB_INF(_pAd)))
#define IS_MT7610(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76100000)
#define IS_MT7610U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76100000) && (IS_USB_INF(_pAd)))
#define IS_MT76x0(_pAd)		(IS_MT7610(_pAd) || IS_MT7630(_pAd) || IS_MT7650(_pAd))
#define IS_MT76x0U(_pAd)	(IS_MT7650U(_pAd) || IS_MT7630U(_pAd) || IS_MT7610U(_pAd))

#define IS_MT7662(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76620000)
#define IS_MT7662U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76620000) && (IS_USB_INF(_pAd)))
#define IS_MT7632(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76320000)
#define IS_MT7632U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76320000) && (IS_USB_INF(_pAd)))
#define IS_MT7612(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76120000)
#define IS_MT7612U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76120000) && (IS_USB_INF(_pAd)))
#define IS_MT7602(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76020000)
#define IS_MT76x2(_pAd)		(IS_MT7662(_pAd) || IS_MT7632(_pAd) || IS_MT7612(_pAd) || IS_MT7602(_pAd))
#define IS_MT76x2U(_pAd)	(IS_MT7662U(_pAd) || IS_MT7632U(_pAd) || IS_MT7612U(_pAd))
#define REV_MT76x2E3        	0x0022
#define REV_MT76x2E4        	0x0033

#define IS_MT76xx(_pAd)		(IS_MT76x0(_pAd) || IS_MT76x2(_pAd))

#define IS_RT65XX(_pAd)		((((_pAd)->MACVersion & 0xFFFF0000) == 0x65900000) ||\
							 (IS_RT8592(_pAd))||\
							 (IS_MT76x0(_pAd)) ||\
							 (IS_MT76x2(_pAd)))


/* RT3592BC8 (WiFi + BT) */

#define IS_USB_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_USB)
#define IS_USB3_INF(_pAd)		((IS_USB_INF(_pAd)) && ((_pAd)->BulkOutMaxPacketSize == 1024))
#define IS_RBUS_INF(_pAd) ((_pAd)->infType == RTMP_DEV_INF_RBUS)

#define RT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) < (_rev))

#define RT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) >= (_rev))

#define MT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) < (_rev))

#define MT_REV_ET(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) == (_rev))

#define MT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) >= (_rev))

/* Dual-band NIC (RF/BBP/MAC are in the same chip.) */

#define IS_RT_NEW_DUAL_BAND_NIC(_pAd) ((FALSE))


/* Is the NIC dual-band NIC? */

#define IS_DUAL_BAND_NIC(_pAd) (((_pAd->RfIcType == RFIC_2850) || (_pAd->RfIcType == RFIC_2750) || (_pAd->RfIcType == RFIC_3052)		\
								|| (_pAd->RfIcType == RFIC_3053) || (_pAd->RfIcType == RFIC_2853) || (_pAd->RfIcType == RFIC_3853) 	\
								|| IS_RT_NEW_DUAL_BAND_NIC(_pAd)) && !IS_RT5390(_pAd))


/* RT3593 over PCIe bus */
#define RT3593OverPCIe(_pAd) (IS_RT3593(_pAd) && (_pAd->CommonCfg.bPCIeBus == TRUE))

/* RT3593 over PCI bus */
#define RT3593OverPCI(_pAd) (IS_RT3593(_pAd) && (_pAd->CommonCfg.bPCIeBus == FALSE))

/*RT3390,RT3370 */
#define IS_RT3390(_pAd)				(((_pAd)->MACVersion & 0xFFFF0000) == 0x33900000)

#define CCA_AVG_MAX_COUNT	5

/* ------------------------------------------------------ */
/* PCI registers - base address 0x0000 */
/* ------------------------------------------------------ */
#define CHIP_PCI_CFG		0x0000
#define CHIP_PCI_EECTRL		0x0004
#define CHIP_PCI_MCUCTRL	0x0008

#define OPT_14			0x114

#define RETRY_LIMIT		10

/* ------------------------------------------------------ */
/* BBP & RF	definition */
/* ------------------------------------------------------ */
#define	BUSY		                1
#define	IDLE		                0

/*------------------------------------------------------------------------- */
/* EEPROM definition */
/*------------------------------------------------------------------------- */
#define EEDO                        0x08
#define EEDI                        0x04
#define EECS                        0x02
#define EESK                        0x01
#define EERL                        0x80

#define EEPROM_WRITE_OPCODE         0x05
#define EEPROM_READ_OPCODE          0x06
#define EEPROM_EWDS_OPCODE          0x10
#define EEPROM_EWEN_OPCODE          0x13

#define NUM_EEPROM_BBP_PARMS		19	/* Include NIC Config 0, 1, CR, TX ALC step, BBPs */
#define NUM_EEPROM_TX_G_PARMS		7

#define VALID_EEPROM_VERSION        1
#define EEPROM_VERSION_OFFSET       0x02
#define EEPROM_NIC1_OFFSET          0x34	/* The address is from NIC config 0, not BBP register ID */
#define EEPROM_NIC2_OFFSET          0x36	/* The address is from NIC config 1, not BBP register ID */


#define EEPROM_COUNTRY_REGION			0x38
#define COUNTRY_REGION_A_BAND_MASK (0xff)
#define COUNTRY_REGION_G_BAND (0xff << 8)

#define EEPROM_DEFINE_MAX_TXPWR			0x4e
#define MAX_EIRP_TX_PWR_G_BAND_MASK (0xff)
#define MAX_EIRP_TX_PWR_A_BAND_MASK (0xff << 8)

#define EEPROM_FREQ_OFFSET			0x3a
#define FREQ_OFFSET_MASK (0x7f)
#define FREQ_OFFSET_DIP (1 << 7)
#define LED_MODE_MASK (0xff << 8)

#define EEPROM_LEDAG_CONF_OFFSET	0x3c
#define EEPROM_LEDACT_CONF_OFFSET	0x3e
#define EEPROM_LED_POLARITY_OFFSET	0x40
#define	EEPROM_NIC3_OFFSET			0x42

#define EEPROM_LNA_OFFSET			0x44
#define LNA_GAIN_G_BAND_MASK (0x7f)
#define LNA_GAIN_G_BAND_EN (1 << 7)
#define LNA_GAIN_A_BAND_CH36_64_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH36_64_EN (1 << 15)

#define EEPROM_RSSI_BG_OFFSET			0x46
#define RSSI0_OFFSET_G_BAND_MASK (0x3f)
#define RSSI0_OFFSET_G_BAND_SIGN (1 << 6)
#define RSSI0_OFFSET_G_BAND_EN (1 << 7)
#define RSSI1_OFFSET_G_BAND_MASK (0x3f << 8)
#define RSSI1_OFFSET_G_BAND_SIGN (1 << 14)
#define RSSI1_OFFSET_G_BAND_EN (1 << 15)

#define EEPROM_TXMIXER_GAIN_2_4G		0x48
#define LNA_GAIN_A_BAND_CH100_128_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH100_128_EN (1 << 15)

#define EEPROM_RSSI_A_OFFSET			0x4a
#define RSSI0_OFFSET_A_BAND_MASK (0x3f)
#define RSSI0_OFFSET_A_BAND_SIGN (1 << 6)
#define RSSI0_OFFSET_A_BANE_EN (1 << 7)
#define RSSI1_OFFSET_A_BAND_MASK (0x3f << 8)
#define RSSI1_OFFSET_A_BAND_SIGN (1 << 14)
#define RSSI1_OFFSET_A_BAND_EN (1 << 15)

#define EEPROM_TXMIXER_GAIN_5G			0x4c
#define LNA_GAIN_A_BAND_CH132_165_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH132_165_EN (1 << 15)

#define EEPROM_TXPOWER_DELTA			0x50	/* 20MHZ AND 40 MHZ use different power. This is delta in 40MHZ. */

#define EEPROM_G_TX_PWR_OFFSET			0x52
#define EEPROM_G_TX2_PWR_OFFSET			0x60

#define EEPROM_G_TSSI_BOUND1			0x6e
#define EEPROM_G_TSSI_BOUND2			0x70
#define EEPROM_G_TSSI_BOUND3			0x72
#define EEPROM_G_TSSI_BOUND4			0x74
#define EEPROM_G_TSSI_BOUND5			0x76



#define EEPROM_A_TX_PWR_OFFSET      		0x78
#define EEPROM_A_TX2_PWR_OFFSET			0xa6

#define MBSSID_MODE0 	0
#define MBSSID_MODE1 	1	/* Enhance NEW MBSSID MODE mapping to mode 0 */
#ifdef ENHANCE_NEW_MBSSID_MODE
#define MBSSID_MODE2	2	/* Enhance NEW MBSSID MODE mapping to mode 1 */
#define MBSSID_MODE3	3	/* Enhance NEW MBSSID MODE mapping to mode 2 */
#define MBSSID_MODE4	4	/* Enhance NEW MBSSID MODE mapping to mode 3 */
#define MBSSID_MODE5	5	/* Enhance NEW MBSSID MODE mapping to mode 4 */
#define MBSSID_MODE6	6	/* Enhance NEW MBSSID MODE mapping to mode 5 */
#endif /* ENHANCE_NEW_MBSSID_MODE */

enum FREQ_CAL_INIT_MODE {
	FREQ_CAL_INIT_UNKNOW,
};

enum FREQ_CAL_MODE {
	FREQ_CAL_MODE2,
};

enum RXWI_FRQ_OFFSET_FIELD {
	RXWI_FRQ_OFFSET_FIELD0, /* SNR1 */
	RXWI_FRQ_OFFSET_FIELD1, /* Frequency Offset */
};



#define EEPROM_A_TSSI_BOUND1		0xd4
#define EEPROM_A_TSSI_BOUND2		0xd6
#define EEPROM_A_TSSI_BOUND3		0xd8
#define EEPROM_A_TSSI_BOUND4		0xda
#define EEPROM_A_TSSI_BOUND5		0xdc


/* ITxBF calibration values EEPROM locations 0xC0 to 0xF1 */
#define EEPROM1_ITXBF_CAL				0xc0

#define EEPROM_TXPOWER_BYRATE 			0xde	/* 20MHZ power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0xde	/* 20MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0xee	/* 40MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0xfa	/* 20MHZ 5G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x10a	/* 40MHZ 5G tx power. */

#define EEPROM_BBP_BASE_OFFSET			0xf0	/* The address is from NIC config 0, not BBP register ID */

/* */
/* Bit mask for the Tx ALC and the Tx fine power control */
/* */
#define GET_TX_ALC_BIT_MASK					0x1F	/* Valid: 0~31, and in 0.5dB step */
#define GET_TX_FINE_POWER_CTRL_BIT_MASK	0xE0	/* Valid: 0~4, and in 0.1dB step */
#define NUMBER_OF_BITS_FOR_TX_ALC			5	/* The length, in bit, of the Tx ALC field */


/* TSSI gain and TSSI attenuation */

#define EEPROM_TSSI_GAIN_AND_ATTENUATION	0x76

/*#define EEPROM_Japan_TX_PWR_OFFSET      0x90 // 802.11j */
/*#define EEPROM_Japan_TX2_PWR_OFFSET      0xbe */
/*#define EEPROM_TSSI_REF_OFFSET	0x54 */
/*#define EEPROM_TSSI_DELTA_OFFSET	0x24 */
/*#define EEPROM_CCK_TX_PWR_OFFSET  0x62 */
/*#define EEPROM_CALIBRATE_OFFSET	0x7c */

#define EEPROM_NIC_CFG1_OFFSET		0
#define EEPROM_NIC_CFG2_OFFSET		1
#define EEPROM_NIC_CFG3_OFFSET		2
#define EEPROM_COUNTRY_REG_OFFSET	3
#define EEPROM_BBP_ARRAY_OFFSET		4

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_ANTENNA_STRUC {
	struct {
		USHORT RssiIndicationMode:1; 	/* RSSI indication mode */
		USHORT Rsv:1;
		USHORT BoardType:2; 		/* 0: mini card; 1: USB pen */
		USHORT RfIcType:4;			/* see E2PROM document */
		USHORT TxPath:4;			/* 1: 1T, 2: 2T, 3: 3T */
		USHORT RxPath:4;			/* 1: 1R, 2: 2R, 3: 3R */
	} field;
	USHORT word;
} EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#else
typedef union _EEPROM_ANTENNA_STRUC {
	struct {
		USHORT RxPath:4;			/* 1: 1R, 2: 2R, 3: 3R */
		USHORT TxPath:4;			/* 1: 1T, 2: 2T, 3: 3T */
		USHORT RfIcType:4;			/* see E2PROM document */
		USHORT BoardType:2; 		/* 0: mini card; 1: USB pen */
		USHORT Rsv:1;
		USHORT RssiIndicationMode:1; 	/* RSSI indication mode */
	} field;
	USHORT word;
} EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#endif


/*
  *   EEPROM operation related marcos
  */


struct RF_BANK_OFFSET {
	UINT8 RFBankIndex;
	uint16_t RFStart;
	uint16_t RFEnd;
};

struct RF_INDEX_OFFSET {
	UINT8 RFIndex;
	uint16_t RFStart;
	uint16_t RFEnd;
};

/*
	2860: 28xx
	2870: 28xx

	30xx:
		3090
		3070
		2070 3070

	33xx:	30xx
		3390 3090
		3370 3070

	35xx:	30xx
		3572, 2870, 28xx
		3062, 2860, 28xx
		3562, 2860, 28xx

	3593, 28xx, 30xx, 35xx

	< Note: 3050, 3052, 3350 can not be compiled simultaneously. >
	305x:
		3052
		3050
		3350, 3050

	3352: 305x

	2880: 28xx
	2883:
	3883:
*/

struct _RTMP_CHIP_CAP_ {
	/* register */
	REG_PAIR *pRFRegTable;
	REG_PAIR *pBBPRegTable;
	UCHAR bbpRegTbSize;

	uint32_t MaxNumOfRfId;
	uint32_t MaxNumOfBbpId;

#define RF_REG_WT_METHOD_NONE			0
#define RF_REG_WT_METHOD_STEP_ON		1
	UCHAR RfReg17WtMethod;

	/* beacon */
	BOOLEAN FlgIsSupSpecBcnBuf;	/* SPECIFIC_BCN_BUF_SUPPORT */
	UINT8 BcnMaxNum;	/* software use */
	UINT8 BcnMaxHwNum;	/* hardware limitation */
	UINT8 WcidHwRsvNum;	/* hardware available WCID number */
	uint16_t BcnMaxHwSize;	/* hardware maximum beacon size */
	uint16_t BcnBase[HW_BEACON_MAX_NUM];	/* hardware beacon base address */

	/* function */
	/* use UINT8, not bit-or to speed up driver */
	BOOLEAN FlgIsHwWapiSup;

	/* VCO calibration mode */
	UINT8 VcoPeriod; /* default 10s */
#define VCO_CAL_DISABLE		0	/* not support */
#define VCO_CAL_MODE_1		1	/* toggle RF7[0] */
#define VCO_CAL_MODE_2		2	/* toggle RF3[7] */
#define VCO_CAL_MODE_3		3	/* toggle RF4[7] or RF5[7] */
	UINT8	FlgIsVcoReCalMode;

	BOOLEAN FlgIsHwAntennaDiversitySup;
	BOOLEAN Flg7662ChipCap;
	BOOLEAN FlgHwTxBfCap;
#ifdef FIFO_EXT_SUPPORT
	BOOLEAN FlgHwFifoExtCap;
#endif /* FIFO_EXT_SUPPORT */

	UCHAR ba_max_cnt;


	enum ASIC_CAP asic_caps;
	enum PHY_CAP phy_caps;
	enum MAC_TYPE mac_type;
	enum BBP_TYPE bbp_type;
	enum RF_TYPE rf_type;


	BOOLEAN temp_tx_alc_enable;
	int32_t temp_25_ref; /* a quantification value of temperature, but not ¢J */
	int32_t current_temp; /* unit ¢J */

#ifdef DYNAMIC_VGA_SUPPORT
	BOOLEAN dynamic_vga_support;
	int32_t compensate_level;
	int32_t avg_rssi_0;
	int32_t avg_rssi_1;
	int32_t avg_rssi_all;
	UCHAR dynamic_chE_mode;
	BOOLEAN dynamic_chE_trigger;
#ifdef CONFIG_AP_SUPPORT
	int32_t dynamic_lna_trigger_timer;
	BOOLEAN microwave_enable;
	int32_t agc1_r35_backup;
	int32_t agc1_r39_backup;
	int32_t agc1_r41_backup;
#endif /* CONFIG_AP_SUPPORT */
#endif /* DYNAMIC_VGA_SUPPORT */

	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb     - pAd->StaCfg.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg.LastSNR0) * 3) / 16; */
	UINT8 SnrFormula;

	UINT8 max_nss;			/* maximum Nss, 3 for 3883 or 3593 */
	UINT8 max_vht_mcs;		/* Maximum Vht MCS */

	UINT8 ac_off_mode;		/* 11AC off mode */

	BOOLEAN bTempCompTxALC;
	BOOLEAN rx_temp_comp;

	BOOLEAN bLimitPowerRange; /* TSSI compensation range limit */

	/* ---------------------------- packet ---------------------------------- */
	UINT8 TXWISize;
	UINT8 RXWISize;

	/* ---------------------------- others ---------------------------------- */

	UCHAR *EEPROM_DEFAULT_BIN;
	uint16_t EEPROM_DEFAULT_BIN_SIZE;

	/*
		Define the burst size of WPDMA of PCI
		0 : 4 DWORD (16bytes)
		1 : 8 DWORD (32 bytes)
		2 : 16 DWORD (64 bytes)
		3 : 32 DWORD (128 bytes)
	*/
	UINT8 WPDMABurstSIZE;

	/*
 	 * 0: MBSSID_MODE0
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit2:bit0] of byte5)
 	 * 1: MBSSID_MODE1
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit4:bit2] of byte0)
 	 */
	UINT8 MBSSIDMode;

#ifdef DOT11W_PMF_SUPPORT
#define PMF_ENCRYPT_MODE_0      0	/* All packets must software encryption. */
#define PMF_ENCRYPT_MODE_1      1	/* Data packets do hardware encryption, management packet do software encryption. */
	UINT8	FlgPMFEncrtptMode;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
	UINT8 FreqCalibrationSupport;
	UINT8 FreqCalMode;
	UINT8 RxWIFrqOffset;
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef RT5592EP_SUPPORT
	uint32_t Priv; /* Flag for RT5592 EP */
#endif /* RT5592EP_SUPPORT */

	UINT8 PAType; /* b'00: 2.4G+5G external PA, b'01: 5G external PA, b'10: 2.4G external PA, b'11: Internal PA */
	UINT8 LNA_type; /* b'00: 2.4G+5G external LNA, b'01: 5G external LNA, b'10: 2.4G external LNA, b'11: Internal LNA */

#ifdef CONFIG_ANDES_SUPPORT
	uint32_t WlanMemmapOffset;
	uint32_t InbandPacketMaxLen; /* must be 48 multible */
	UINT8 CmdRspRxRing;
	BOOLEAN IsComboChip;
	BOOLEAN need_load_fw;
	BOOLEAN need_load_rom_patch;
	u8 ram_code_protect;
	u8 rom_code_protect;
	u8 load_iv;
	u32 ilm_offset;
	u32 dlm_offset;
	u32 rom_patch_offset;
#endif

	UINT8 cmd_header_len;
	UINT8 cmd_padding_len;

#ifdef RTMP_USB_SUPPORT
	UINT8 DataBulkInAddr;
	UINT8 CommandRspBulkInAddr;
	UINT8 WMM0ACBulkOutAddr[4];
	UINT8 WMM1ACBulkOutAddr;
	UINT8 CommandBulkOutAddr;
#endif

#ifdef CONFIG_SWITCH_CHANNEL_OFFLOAD
	uint16_t ChannelParamsSize;
	UCHAR *ChannelParam;
	INT XtalType;
#endif

	u8 *fw_name;		/* ULLI : rename to catch compiler errors */
	u8 *fw_patch_name;	/* ULLI : rename to catch compiler errors */

	BOOLEAN tssi_enable;
	BOOLEAN ed_cca_enable;

#define TSSI_INIT_STAGE 0
#define TSSI_CAL_STAGE 1
#define TSSI_TRIGGER_STAGE 2
#define TSSI_COMP_STAGE 3
	UINT8 tssi_stage;
#define TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE 0x84
#define TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE 0x83
	UINT8 tssi_0_slope_g_band;
	UINT8 tssi_1_slope_g_band;
#define TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE 0x0A
#define TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE 0x0B
	UINT8 tssi_0_offset_g_band;
	UINT8 tssi_1_offset_g_band;
#define A_BAND_GRP0_CHL 0
#define A_BAND_GRP1_CHL 1
#define A_BAND_GRP2_CHL 2
#define A_BAND_GRP3_CHL 3
#define A_BAND_GRP4_CHL 4
#define A_BAND_GRP5_CHL 5
#define A_BAND_GRP_NUM 6
#define TSSI_0_SLOPE_A_BAND_GRP0_DEFAULT_VALUE 0x7A
#define TSSI_1_SLOPE_A_BAND_GRP0_DEFAULT_VALUE 0x79
#define TSSI_0_SLOPE_A_BAND_GRP1_DEFAULT_VALUE 0x7A
#define TSSI_1_SLOPE_A_BAND_GRP1_DEFAULT_VALUE 0x79
#define TSSI_0_SLOPE_A_BAND_GRP2_DEFAULT_VALUE 0x78
#define TSSI_1_SLOPE_A_BAND_GRP2_DEFAULT_VALUE 0x7B
#define TSSI_0_SLOPE_A_BAND_GRP3_DEFAULT_VALUE 0x7E
#define TSSI_1_SLOPE_A_BAND_GRP3_DEFAULT_VALUE 0x77
#define TSSI_0_SLOPE_A_BAND_GRP4_DEFAULT_VALUE 0x76
#define TSSI_1_SLOPE_A_BAND_GRP4_DEFAULT_VALUE 0x7E
#define TSSI_0_SLOPE_A_BAND_GRP5_DEFAULT_VALUE 0x75
#define TSSI_1_SLOPE_A_BAND_GRP5_DEFAULT_VALUE 0x79
	UINT8 tssi_0_slope_a_band[A_BAND_GRP_NUM];
	UINT8 tssi_1_slope_a_band[A_BAND_GRP_NUM];
#define TSSI_0_OFFSET_A_BAND_GRP0_DEFAULT_VALUE 0x06
#define TSSI_1_OFFSET_A_BAND_GRP0_DEFAULT_VALUE 0x0A
#define TSSI_0_OFFSET_A_BAND_GRP1_DEFAULT_VALUE 0x06
#define TSSI_1_OFFSET_A_BAND_GRP1_DEFAULT_VALUE 0x0A
#define TSSI_0_OFFSET_A_BAND_GRP2_DEFAULT_VALUE 0x08
#define TSSI_1_OFFSET_A_BAND_GRP2_DEFAULT_VALUE 0x06
#define TSSI_0_OFFSET_A_BAND_GRP3_DEFAULT_VALUE 0x02
#define TSSI_1_OFFSET_A_BAND_GRP3_DEFAULT_VALUE 0x14
#define TSSI_0_OFFSET_A_BAND_GRP4_DEFAULT_VALUE 0x16
#define TSSI_1_OFFSET_A_BAND_GRP4_DEFAULT_VALUE 0x05
#define TSSI_0_OFFSET_A_BAND_GRP5_DEFAULT_VALUE 0x1C
#define TSSI_1_OFFSET_A_BAND_GRP5_DEFAULT_VALUE 0x11
	UINT8 tssi_0_offset_a_band[A_BAND_GRP_NUM];
	UINT8 tssi_1_offset_a_band[A_BAND_GRP_NUM];
#define TX_TARGET_PWR_DEFAULT_VALUE 0x14
	CHAR tx_0_target_pwr_g_band;
	CHAR tx_1_target_pwr_g_band;
	CHAR tx_0_target_pwr_a_band[A_BAND_GRP_NUM];
	CHAR tx_1_target_pwr_a_band[A_BAND_GRP_NUM];
#define G_BAND_LOW 0
#define G_BAND_MID 1
#define G_BAND_HI 2
	CHAR tx_0_chl_pwr_delta_g_band[3];
	CHAR tx_1_chl_pwr_delta_g_band[3];
#define A_BAND_LOW 0
#define A_BAND_HI 1
	CHAR tx_0_chl_pwr_delta_a_band[A_BAND_GRP_NUM][2];
	CHAR tx_1_chl_pwr_delta_a_band[A_BAND_GRP_NUM][2];
	CHAR delta_tx_pwr_bw40_g_band;
	CHAR delta_tx_pwr_bw40_a_band;
	CHAR delta_tx_pwr_bw80;

	CHAR tx_pwr_cck_1_2;
	CHAR tx_pwr_cck_5_11;
	CHAR tx_pwr_g_band_ofdm_6_9;
	CHAR tx_pwr_g_band_ofdm_12_18;
	CHAR tx_pwr_g_band_ofdm_24_36;
	CHAR tx_pwr_g_band_ofdm_48_54;
	CHAR tx_pwr_ht_mcs_0_1;
	CHAR tx_pwr_ht_mcs_2_3;
	CHAR tx_pwr_ht_mcs_4_5;
	CHAR tx_pwr_ht_mcs_6_7;
	CHAR tx_pwr_ht_mcs_8_9;
	CHAR tx_pwr_ht_mcs_10_11;
	CHAR tx_pwr_ht_mcs_12_13;
	CHAR tx_pwr_ht_mcs_14_15;
	CHAR tx_pwr_a_band_ofdm_6_9;
	CHAR tx_pwr_a_band_ofdm_12_18;
	CHAR tx_pwr_a_band_ofdm_24_36;
	CHAR tx_pwr_a_band_ofdm_48_54;
	CHAR tx_pwr_vht_mcs_0_1;
	CHAR tx_pwr_vht_mcs_2_3;
	CHAR tx_pwr_vht_mcs_4_5;
	CHAR tx_pwr_vht_mcs_6_7;
	CHAR tx_pwr_2g_vht_mcs_8_9;
	CHAR tx_pwr_5g_vht_mcs_8_9;

	CHAR rf0_2g_rx_high_gain;
	CHAR rf1_2g_rx_high_gain;
	CHAR rf0_5g_grp0_rx_high_gain;
	CHAR rf1_5g_grp0_rx_high_gain;
	CHAR rf0_5g_grp1_rx_high_gain;
	CHAR rf1_5g_grp1_rx_high_gain;
	CHAR rf0_5g_grp2_rx_high_gain;
	CHAR rf1_5g_grp2_rx_high_gain;
	CHAR rf0_5g_grp3_rx_high_gain;
	CHAR rf1_5g_grp3_rx_high_gain;
	CHAR rf0_5g_grp4_rx_high_gain;
	CHAR rf1_5g_grp4_rx_high_gain;
	CHAR rf0_5g_grp5_rx_high_gain;
	CHAR rf1_5g_grp5_rx_high_gain;
	BOOLEAN chl_smth_enable;

	MT76x2_RATE_PWR_Table rate_pwr_table;


};

typedef VOID (*CHIP_SPEC_FUNC)(VOID *pAd, VOID *pData, ULONG Data);

/* The chip specific function ID */
typedef enum _CHIP_SPEC_ID
{
	CHIP_SPEC_RESV_FUNC
} CHIP_SPEC_ID;

#define CHIP_SPEC_ID_NUM 	CHIP_SPEC_RESV_FUNC


struct _RTMP_CHIP_OP_ {
	int (*sys_onoff)(struct rtmp_adapter *pAd, BOOLEAN on, BOOLEAN reser);

	/* MCU related callback functions */
	int (*load_rom_patch)(struct rtmp_adapter *ad);
	int (*loadFirmware)(struct rtmp_adapter *pAd);
	int (*eraseFirmware)(struct rtmp_adapter *pAd);
	int (*sendCommandToMcu)(struct rtmp_adapter *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1, BOOLEAN FlgIsNeedLocked);	/* int (*sendCommandToMcu)(struct rtmp_adapter *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1); */
#ifdef CONFIG_ANDES_SUPPORT
	int (*sendCommandToAndesMcu)(struct rtmp_adapter *pAd, UCHAR QueIdx, UCHAR cmd, UCHAR *pData, USHORT DataLen, BOOLEAN FlgIsNeedLocked);
#endif

	void (*AsicRfInit)(struct rtmp_adapter *pAd);
	void (*AsicBbpInit)(struct rtmp_adapter *pAd);
	void (*AsicMacInit)(struct rtmp_adapter *pAd);

	/* Power save */
	void (*EnableAPMIMOPS)(struct rtmp_adapter *pAd, IN BOOLEAN ReduceCorePower);
	void (*DisableAPMIMOPS)(struct rtmp_adapter *pAd);
	INT (*PwrSavingOP)(struct rtmp_adapter *pAd, uint32_t PwrOP, uint32_t PwrLevel,
							uint32_t ListenInterval, uint32_t PreTBTTLeadTime,
							UINT8 TIMByteOffset, UINT8 TIMBytePattern);

	/* BBP adjust */
	VOID (*ChipBBPAdjust)(IN struct rtmp_adapter *pAd);

	/* Channel */
	VOID (*ChipSwitchChannel)(struct rtmp_adapter *pAd, UCHAR ch, BOOLEAN bScan);


	/* IQ Calibration */
	VOID (*ChipIQCalibration)(struct rtmp_adapter *pAd, UCHAR Channel);

	uint32_t (*ChipGetCurrentTemp)(struct rtmp_adapter *pAd);

	VOID (*AsicGetTxPowerOffset)(struct rtmp_adapter *pAd, ULONG *TxPwr);
	VOID (*AsicExtraPowerOverMAC)(struct rtmp_adapter *pAd);

	VOID (*AsicAdjustTxPower)(struct rtmp_adapter *pAd);

	/* Antenna */
	VOID (*SetRxAnt)(struct rtmp_adapter *pAd, UCHAR Ant);

	/* EEPROM */
	VOID (*NICInitAsicFromEEPROM)(IN struct rtmp_adapter *pAd);

	/* Temperature Compensation */
	VOID (*InitTemperCompensation)(IN struct rtmp_adapter *pAd);
	VOID (*TemperCompensation)(IN struct rtmp_adapter *pAd);

	/* high power tuning */
	VOID (*HighPowerTuning)(struct rtmp_adapter *pAd, struct _RSSI_SAMPLE *pRssi);

	/* The chip specific function list */
	CHIP_SPEC_FUNC ChipSpecFunc[CHIP_SPEC_ID_NUM];

	VOID (*CckMrcStatusCtrl)(struct rtmp_adapter *pAd);
	VOID (*RadarGLRTCompensate)(struct rtmp_adapter *pAd);
	VOID (*SecondCCADetection)(struct rtmp_adapter *pAd);

	/* MCU */
	void (*MCUCtrlInit)(struct rtmp_adapter *ad);
	void (*MCUCtrlExit)(struct rtmp_adapter *ad);
#ifdef CONFIG_ANDES_SUPPORT
	void (*fw_init)(struct rtmp_adapter *ad);
	int (*RandomWrite)(struct rtmp_adapter *ad, RTMP_REG_PAIR *RegPair, uint32_t Num);
	void (*Calibration)(struct rtmp_adapter *pAd, uint32_t CalibrationID, ANDES_CALIBRATION_PARAM *param);
#endif /* CONFIG_ANDES_SUPPORT */
	void (*DisableTxRx)(struct rtmp_adapter *ad, UCHAR Level);
	void (*AsicRadioOn)(struct rtmp_adapter *ad, UCHAR Stage);
	void (*AsicRadioOff)(struct rtmp_adapter *ad, u8 Stage);

#ifdef DYNAMIC_VGA_SUPPORT
	VOID (*AsicDynamicVgaGainControl)(IN struct rtmp_adapter *pAd);
#endif /* DYNAMIC_VGA_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
	VOID (*AsicWOWEnable)(struct rtmp_adapter *ad);
	VOID (*AsicWOWDisable)(struct rtmp_adapter *ad);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	void (*usb_cfg_read)(struct rtmp_adapter *ad, u32 *value);
	void (*usb_cfg_write)(struct rtmp_adapter *ad, u32 value);
	void (*cal_test)(struct rtmp_adapter *ad, uint32_t type);
};

#define RTMP_CHIP_ENABLE_AP_MIMOPS(__pAd, __ReduceCorePower)	\
do {	\
		if (__pAd->chipOps.EnableAPMIMOPS != NULL)	\
			__pAd->chipOps.EnableAPMIMOPS(__pAd, __ReduceCorePower);	\
} while (0)

#define RTMP_CHIP_DISABLE_AP_MIMOPS(__pAd)	\
do {	\
		if (__pAd->chipOps.DisableAPMIMOPS != NULL)	\
			__pAd->chipOps.DisableAPMIMOPS(__pAd);	\
} while (0)

#define PWR_SAVING_OP(__pAd, __PwrOP, __PwrLevel, __ListenInterval, \
						__PreTBTTLeadTime, __TIMByteOffset, __TIMBytePattern)	\
do {	\
		if (__pAd->chipOps.PwrSavingOP != NULL)	\
			__pAd->chipOps.PwrSavingOP(__pAd, __PwrOP, __PwrLevel,	\
										__ListenInterval,__PreTBTTLeadTime, \
										__TIMByteOffset, __TIMBytePattern);	\
} while (0)

#define RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(__pAd, __pCfgOfTxPwrCtrlOverMAC)	\
do {	\
		if (__pAd->chipOps.AsicGetTxPowerOffset != NULL)	\
			__pAd->chipOps.AsicGetTxPowerOffset(__pAd, __pCfgOfTxPwrCtrlOverMAC);	\
} while (0)

#define RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(__pAd)	\
do {	\
		if (__pAd->chipOps.AsicExtraPowerOverMAC != NULL)	\
			__pAd->chipOps.AsicExtraPowerOverMAC(__pAd);	\
} while (0)

#define RTMP_CHIP_GET_CURRENT_TEMP(__pAd, __pCurrentTemp) \
do {	\
		if (__pAd->chipOps.ChipGetCurrentTemp != NULL) \
			__pCurrentTemp = __pAd->chipOps.ChipGetCurrentTemp(__pAd); \
} while (0)

#define RTMP_CHIP_ASIC_FREQ_CAL_STOP(__pAd)	\
do {	\
		if (__pAd->chipOps.AsicFreqCalStop != NULL)	\
			__pAd->chipOps.AsicFreqCalStop(__pAd);	\
} while (0)

#define RTMP_CHIP_IQ_CAL(__pAd, __pChannel)	\
do {	\
		if (__pAd->chipOps.ChipIQCalibration != NULL)	\
			 __pAd->chipOps.ChipIQCalibration(__pAd, __pChannel);	\
} while (0)

#define RTMP_CHIP_HIGH_POWER_TUNING(__pAd, __pRssi)	\
do {	\
		if (__pAd->chipOps.HighPowerTuning != NULL)	\
			__pAd->chipOps.HighPowerTuning(__pAd, __pRssi);	\
} while (0)

#define RTMP_EEPROM_ASIC_INIT(__pAd)	\
do {	\
		if (__pAd->chipOps.NICInitAsicFromEEPROM != NULL)	\
			__pAd->chipOps.NICInitAsicFromEEPROM(__pAd);	\
} while (0)

#define RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(__pAd)						\
do {	\
		if (__pAd->chipOps.TemperCompensation != NULL)					\
			__pAd->chipOps.TemperCompensation(__pAd);	\
} while (0)

#define RTMP_CHIP_SPECIFIC(__pAd, __FuncId, __pData, __Data)	\
do {	\
		if ((__FuncId >= 0) && (__FuncId < CHIP_SPEC_RESV_FUNC))	\
		{	\
			if (__pAd->chipOps.ChipSpecFunc[__FuncId] != NULL)	\
				__pAd->chipOps.ChipSpecFunc[__FuncId](__pAd, __pData, __Data);	\
		}	\
} while (0)

#define RTMP_CHIP_CCK_MRC_STATUS_CTRL(__pAd)	\
do {	\
		if(__pAd->chipOps.CckMrcStatusCtrl != NULL)	\
			__pAd->chipOps.CckMrcStatusCtrl(__pAd);	\
} while (0)

#define RTMP_CHIP_RADAR_GLRT_COMPENSATE(__pAd) \
do {	\
		if(__pAd->chipOps.RadarGLRTCompensate != NULL)	\
			__pAd->chipOps.RadarGLRTCompensate(__pAd);	\
} while (0)


#define CHIP_CALIBRATION(__pAd, __CalibrationID, param) \
do {	\
	ANDES_CALIBRATION_PARAM calibration_param; \
	calibration_param.generic = param; \
	if(__pAd->chipOps.Calibration != NULL) \
		__pAd->chipOps.Calibration(__pAd, __CalibrationID, &calibration_param); \
} while (0)

#define BURST_WRITE(_pAd, _Offset, _pData, _Cnt)	\
do {												\
		if (_pAd->chipOps.BurstWrite != NULL)		\
			_pAd->chipOps.BurstWrite(_pAd, _Offset, _pData, _Cnt);\
} while (0)

#define RANDOM_WRITE(_pAd, _RegPair, _Num)	\
do {	\
		if (_pAd->chipOps.RandomWrite != NULL)	\
			_pAd->chipOps.RandomWrite(_pAd, _RegPair, _Num);	\
} while (0)

#define RTMP_SECOND_CCA_DETECTION(__pAd) \
do {	\
	if (__pAd->chipOps.SecondCCADetection != NULL)	\
	{	\
		__pAd->chipOps.SecondCCADetection(__pAd);	\
	}	\
} while (0)

#define DISABLE_TX_RX(_pAd, _Level)	\
do {	\
	if (_pAd->chipOps.DisableTxRx != NULL)	\
		_pAd->chipOps.DisableTxRx(_pAd, _Level);	\
} while (0)

#define ASIC_RADIO_ON(_pAd, _Stage)	\
do {	\
	if (_pAd->chipOps.AsicRadioOn != NULL)	\
		_pAd->chipOps.AsicRadioOn(_pAd, _Stage);	\
} while (0)

#define ASIC_RADIO_OFF(_pAd, _Stage)	\
do {	\
	if (_pAd->chipOps.AsicRadioOff != NULL)	\
		_pAd->chipOps.AsicRadioOff(_pAd, _Stage);	\
} while (0)

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
#define ASIC_WOW_ENABLE(_pAd)	\
do {	\
	if (_pAd->chipOps.AsicWOWEnable != NULL)	\
		_pAd->chipOps.AsicWOWEnable(_pAd);	\
} while (0)

#define ASIC_WOW_DISABLE(_pAd)	\
do {	\
	if (_pAd->chipOps.AsicWOWDisable != NULL)	\
		_pAd->chipOps.AsicWOWDisable(_pAd);	\
} while(0)
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#define MCU_CTRL_INIT(_pAd)	\
do {	\
	if (_pAd->chipOps.MCUCtrlInit != NULL)	\
		_pAd->chipOps.MCUCtrlInit(_pAd);	\
} while (0)

#define MCU_CTRL_EXIT(_pAd)	\
do {	\
	if (_pAd->chipOps.MCUCtrlExit != NULL)	\
		_pAd->chipOps.MCUCtrlExit(_pAd);	\
} while (0)

#define USB_CFG_READ(_ad, _pvalue)	\
do {	\
	if (_ad->chipOps.usb_cfg_read != NULL)	\
		_ad->chipOps.usb_cfg_read(_ad, _pvalue);	\
	else {\
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): usb_cfg_read not inited!\n", __FUNCTION__));\
	}\
} while (0)

#define USB_CFG_WRITE(_ad, _value)	\
do {	\
	if (_ad->chipOps.usb_cfg_write != NULL)	\
		_ad->chipOps.usb_cfg_write(_ad, _value);	\
	else {\
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): usb_cfg_write not inited!\n", __FUNCTION__));\
	}\
} while (0)

#ifdef DYNAMIC_VGA_SUPPORT
#define RTMP_ASIC_DYNAMIC_VGA_GAIN_CONTROL(_pAd)	\
		do {	\
			if (_pAd->chipOps.AsicDynamicVgaGainControl != NULL)	\
				_pAd->chipOps.AsicDynamicVgaGainControl(_pAd);	\
		} while (0)
#endif /* DYNAMIC_VGA_SUPPORT */

int RtmpChipOpsHook(VOID *pCB);
VOID RtmpChipBcnSpecInit(struct rtmp_adapter *pAd);
#ifdef RLT_MAC
VOID rlt_bcn_buf_init(struct rtmp_adapter *pAd);
#endif /* RLT_MAC */

#ifdef GREENAP_SUPPORT
VOID EnableAPMIMOPSv2(struct rtmp_adapter *pAd, BOOLEAN ReduceCorePower);
VOID DisableAPMIMOPSv2(struct rtmp_adapter *pAd);
VOID EnableAPMIMOPSv1(struct rtmp_adapter *pAd, BOOLEAN ReduceCorePower);
VOID DisableAPMIMOPSv1(struct rtmp_adapter *pAd);
#endif /* GREENAP_SUPPORT */

/* global variable */
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern UCHAR NUM_OF_3020_CHNL;
extern FREQUENCY_ITEM *FreqItems3020;
extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

BOOLEAN AsicWaitPDMAIdle(struct rtmp_adapter *pAd, INT round, INT wait_us);
INT AsicSetPreTbttInt(struct rtmp_adapter *pAd, BOOLEAN enable);
INT AsicReadAggCnt(struct rtmp_adapter *pAd, ULONG *aggCnt, int cnt_len);

INT rtmp_asic_top_init(struct rtmp_adapter *pAd);

INT StopDmaTx(struct rtmp_adapter *pAd, UCHAR Level);
INT StopDmaRx(struct rtmp_adapter *pAd, UCHAR Level);

BOOLEAN isExternalPAMode(struct rtmp_adapter *ad, INT channel);
BOOLEAN is_external_lna_mode(struct rtmp_adapter *ad, INT channel);
#endif /* __RTMP_CHIP_H__ */

