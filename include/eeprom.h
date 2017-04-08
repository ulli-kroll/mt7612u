/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	eeprom.h

	Abstract:
	Miniport header file for eeprom related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

#if defined (CONFIG_RT2880_FLASH_32M)
#define DEFAULT_RF_OFFSET                                       0x1FE0000
#else
#define DEFAULT_RF_OFFSET                                       0x40000
#endif

/*************************************************************************
  *
  *     EEPROM Related definitions
  *
  ************************************************************************/
#if defined(CONFIG_RALINK_RT3050_1T1R)
#if defined(CONFIG_RALINK_RT3350)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3350_AP_1T1R_V1_0.bin"
#else
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3050_AP_1T1R_V1_0.bin"
#endif /* CONFIG_RALINK_RT3350 */
#endif /* CONFIG_RALINK_RT3050_1T1R */

#if defined(CONFIG_RALINK_RT3051_1T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3051_AP_1T2R_V1_0.bin"
#endif /* CONFIG_RALINK_RT3051_1T2R */

#if defined(CONFIG_RALINK_RT3052_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3052_AP_2T2R_V1_1.bin"
#endif /* CONFIG_RALINK_RT3052_2T2R */

#if defined(CONFIG_RALINK_RT3883_3T3R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3883_AP_3T3R_V0_1.bin"
#endif /* CONFIG_RALINK_RT3883_3T3R */

#if defined(CONFIG_RALINK_RT3662_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3662_AP_2T2R_V0_0.bin"
#endif /* CONFIG_RALINK_RT3662_2T2R */

#if defined(CONFIG_RALINK_RT3352_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3352_AP_2T2R-4L_V12.BIN"
#endif /* CONFIG_RALINK_RT3352_2T2R */

#if defined(CONFIG_RALINK_RT5350_1T1R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT5350_AP_1T1R_V1_0.bin"
#endif // CONFIG_RALINK_RT5350_1T1R //

#if defined(CONFIG_RT2860V2_2850)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT2880_RT2850_AP_2T3R_V1_6.bin"
#endif /* CONFIG_RT2860V2_2850 */

#if defined (CONFIG_RALINK_RT6352)  || defined (CONFIG_RALINK_MT7620)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/MT7620_AP_2T2R-4L_V15.BIN"
#endif /* defined (CONFIG_RALINK_RT6352)  || defined (CONFIG_RALINK_MT7620) */


#ifndef EEPROM_DEFAULT_FILE_PATH
/* RFIC 2820 */
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT2880_RT2820_AP_2T3R_V1_6.bin"
#endif /* EEPROM_DEFAULT_FILE_PATH */

/* For ioctl check usage */
#define EEPROM_IS_PROGRAMMED		0x80

#define E2P_NONE					0x00
#define E2P_EFUSE_MODE				0x01
#define E2P_FLASH_MODE				0x02
#define E2P_EEPROM_MODE			0x03
#define E2P_BIN_MODE				0x04
#define NUM_OF_E2P_MODE			0x05

#define MAX_EEPROM_BIN_FILE_SIZE	1024

#define EEPROM_SIZE					0x400

#define EEPROM_DEFULT_BIN_FILE	"RT30xxEEPROM.bin"
#ifdef BB_SOC
#define BIN_FILE_PATH				"/etc/RT30xxEEPROM.bin"
#else
#define BIN_FILE_PATH				"/tmp/RT30xxEEPROM.bin"
#endif /* BB_SOC */

#define EEPROM_FILE_DIR			"/etc_ro/wlan/"
#define EEPROM_1ST_FILE_DIR		"/etc_ro/Wireless/iNIC/"
#define EEPROM_2ND_FILE_DIR		"/etc_ro/Wireless/iNIC/"

#ifdef RT_BIG_ENDIAN
typedef	union _EEPROM_WORD_STRUC {
	struct {
		u8 Byte1;				// High Byte
		u8 Byte0;				// Low Byte
	} field;
	unsigned short word;
} EEPROM_WORD_STRUC;
#else
typedef	union _EEPROM_WORD_STRUC {
	struct {
		u8 Byte0;
		u8 Byte1;
	} field;
	unsigned short word;
} EEPROM_WORD_STRUC;
#endif


/* ------------------------------------------------------------------- */
/*  E2PROM data layout */
/* ------------------------------------------------------------------- */

/* Board type */

#define BOARD_TYPE_MINI_CARD		0	/* Mini card */
#define BOARD_TYPE_USB_PEN		1	/* USB pen */

/*
	EEPROM antenna select format
*/

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		unsigned short DACTestBit:1;	/* control if driver should patch the DAC issue */
		unsigned short CoexBit:1;
		unsigned short bInternalTxALC:1;	/* Internal Tx ALC */
		unsigned short AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		unsigned short AntDiversity:1;	/* Antenna diversity */
		unsigned short ExternalLNA:1;
		unsigned short BW40MAvailForA:1;	/* 0:enable, 1:disable */
		unsigned short BW40MAvailForG:1;	/* 0:enable, 1:disable */
		unsigned short EnableWPSPBC:1;	/* WPS PBC Control bit */
		unsigned short BW40MSidebandForA:1;
		unsigned short BW40MSidebandForG:1;
		unsigned short CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		unsigned short ExternalLNAForA:1;	/* external LNA enable for 5G */
		unsigned short ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		unsigned short DynamicTxAgcControl:1;	/* */
		unsigned short HardwareRadioControl:1;	/* Whether RF is controlled by driver or HW. 1:enable hw control, 0:disable */
	} field;
	unsigned short word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#else
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		unsigned short HardwareRadioControl:1;	/* 1:enable, 0:disable */
		unsigned short DynamicTxAgcControl:1;	/* */
		unsigned short ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		unsigned short ExternalLNAForA:1;	/* external LNA enable for 5G */
		unsigned short CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		unsigned short BW40MSidebandForG:1;
		unsigned short BW40MSidebandForA:1;
		unsigned short EnableWPSPBC:1;	/* WPS PBC Control bit */
		unsigned short BW40MAvailForG:1;	/* 0:enable, 1:disable */
		unsigned short BW40MAvailForA:1;	/* 0:enable, 1:disable */
		unsigned short ExternalLNA:1;
		unsigned short AntDiversity:1;	/* Antenna diversity */
		unsigned short AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		unsigned short bInternalTxALC:1;	/* Internal Tx ALC */
		unsigned short CoexBit:1;
		unsigned short DACTestBit:1;	/* control if driver should patch the DAC issue */
	} field;
	unsigned short word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#endif /* RT_BIG_ENDIAN */


#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_NIC_CINFIG3_STRUC {
	struct {
		unsigned short CoexMethod:3;
		unsigned short Rsv1:1;
		unsigned short TemperatureDisable:1;	/* 0:enable, 1:disable */
		unsigned short XtalOption:2;
		unsigned short HWAntDiv:1;
		unsigned short TxStream:4;	/* Number of Tx stream */
		unsigned short RxStream:4;	/* Number of rx stream */
	} field;
	unsigned short word;
} EEPROM_NIC_CONFIG3_STRUC, *PEEPROM_NIC_CONFIG3_STRUC;
#else
typedef union _EEPROM_NIC_CINFIG3_STRUC {
	struct {
		unsigned short RxStream:4;	/* Number of rx stream */
		unsigned short TxStream:4;	/* Number of Tx stream */
		unsigned short HWAntDiv:1;
		unsigned short XtalOption:2;
		unsigned short TemperatureDisable:1;	/* 0:enable, 1:disable */
		unsigned short Rsv1:1;
		unsigned short CoexMethod:3;
	} field;
	unsigned short word;
} EEPROM_NIC_CONFIG3_STRUC, *PEEPROM_NIC_CONFIG3_STRUC;
#endif /* RT_BIG_ENDIAN */



/*
	TX_PWR Value valid range 0xFA(-6) ~ 0x24(36)
*/
#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte1;	/* High Byte */
		signed char Byte0;	/* Low Byte */
	} field;
	unsigned short word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#else
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte0;	/* Low Byte */
		signed char Byte1;	/* High Byte */
	} field;
	unsigned short word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_VERSION_STRUC {
	struct {
		u8 Version;	/* High Byte */
		u8 FaeReleaseNumber;	/* Low Byte */
	} field;
	unsigned short word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#else
typedef union _EEPROM_VERSION_STRUC {
	struct {
		u8 FaeReleaseNumber;	/* Low Byte */
		u8 Version;	/* High Byte */
	} field;
	unsigned short word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_LED_STRUC {
	struct {
		unsigned short Rsvd:3;	/* Reserved */
		unsigned short LedMode:5;	/* Led mode. */
		unsigned short PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		unsigned short PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		unsigned short PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		unsigned short PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		unsigned short PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		unsigned short PolarityACT:1;	/* Polarity ACT setting. */
		unsigned short PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		unsigned short PolarityRDY_G:1;	/* Polarity RDY_G setting. */
	} field;
	unsigned short word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#else
typedef union _EEPROM_LED_STRUC {
	struct {
		unsigned short PolarityRDY_G:1;	/* Polarity RDY_G setting. */
		unsigned short PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		unsigned short PolarityACT:1;	/* Polarity ACT setting. */
		unsigned short PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		unsigned short PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		unsigned short PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		unsigned short PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		unsigned short PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		unsigned short LedMode:5;	/* Led mode. */
		unsigned short Rsvd:3;	/* Reserved */
	} field;
	unsigned short word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		u8 TxPowerEnable:1;	/* Enable */
		u8 Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		u8 DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
	} field;
	u8 value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#else
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		u8 DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
		u8 Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		u8 TxPowerEnable:1;	/* Enable */
	} field;
	u8 value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#endif /* RT_BIG_ENDIAN */


#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		u8 Byte1;	/* High Byte */
		u8 Byte0;	/* Low Byte */
	} field;

	unsigned short 	word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#else
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		u8 Byte0;	/* Low Byte */
		u8 Byte1;	/* High Byte */
	} field;

	unsigned short 	word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#endif /* RT_BIG_ENDIAN */

#define NIC_CONFIGURE_0 0x34
#define EXTERNAL_PA_MASK (0x3 << 8)
#define GET_PA_TYPE(p) (((p) & EXTERNAL_PA_MASK) >> 8)

#define NIC_CONFIGURE_1 0x36
#define INTERNAL_TX_ALC_EN (1 << 13)

#define XTAL_TRIM1 0x3A
#define XTAL_TRIM1_DIP_SELECTION (1 << 7)
#define XTAL_TRIM1_MASK (0x7F)

#define G_BAND_20_40_BW_PWR_DELTA 0x50
#define G_BAND_20_40_BW_PWR_DELTA_MASK (0x3f)
#define G_BAND_20_40_BW_PWR_DELTA_SIGN (1 << 6)
#define G_BAND_20_40_BW_PWR_DELTA_EN (1 << 7)
#define A_BAND_20_40_BW_PWR_DELTA_MASK (0x3f << 8)
#define A_BAND_20_40_BW_PWR_DELTA_SIGN (1 << 14)
#define A_BAND_20_40_BW_PWR_DELTA_EN (1 << 15)

enum EEPROM_STORAGE_TYPE{
	EEPROM_PROM = 0,
};

#define A_BAND_20_80_BW_PWR_DELTA 0x52
#define A_BAND_20_80_BW_PWR_DELTA_MASK (0x3f)
#define A_BAND_20_80_BW_PWR_DELTA_SIGN (1 << 6)
#define A_BAND_20_80_BW_PWR_DELTA_EN (1 << 7)
#define G_BAND_EXT_PA_SETTING_MASK (0x7f << 8)
#define G_BAND_EXT_PA_SETTING_EN (1 << 15)

#define A_BAND_EXT_PA_SETTING 0x54
#define A_BAND_EXT_PA_SETTING_MASK (0x7f)
#define A_BAND_EXT_PA_SETTING_EN (1 << 7)
#define TEMP_SENSOR_CAL_MASK (0x7f << 8)
#define TEMP_SENSOR_CAL_EN (1 << 15)

#define TX0_G_BAND_TSSI_SLOPE 0x56
#define TX0_G_BAND_TSSI_SLOPE_MASK (0xff)
#define TX0_G_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define TX0_G_BAND_TARGET_PWR 0x58
#define TX0_G_BAND_TARGET_PWR_MASK (0xff)
#define TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define TX0_G_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define TX0_G_BAND_CHL_PWR_DELTA_MID 0x5A
#define TX0_G_BAND_CHL_PWR_DELTA_MID_MASK (0x3f)
#define TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN (1 << 6)
#define TX0_G_BAND_CHL_PWR_DELTA_MID_EN (1 << 7)
#define TX0_G_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define TX0_G_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define TX1_G_BAND_TSSI_SLOPE 0x5C
#define TX1_G_BAND_TSSI_SLOPE_MASK (0xff)
#define TX1_G_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define TX1_G_BAND_TARGET_PWR 0x5E
#define TX1_G_BAND_TARGET_PWR_MASK (0xff)
#define TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define TX1_G_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define TX1_G_BAND_CHL_PWR_DELTA_MID 0x60
#define TX1_G_BAND_CHL_PWR_DELTA_MID_MASK (0x3f)
#define TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN (1 << 6)
#define TX1_G_BAND_CHL_PWR_DELTA_MID_EN (1 << 7)
#define TX1_G_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define TX1_G_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP0_TX0_A_BAND_TSSI_SLOPE 0x62
#define GRP0_TX0_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP0_TX0_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP0_TX0_A_BAND_TARGET_PWR 0x64
#define GRP0_TX0_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI 0x66
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP1_TX0_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP1_TX0_A_BAND_TSSI_OFFSET 0x68
#define GRP1_TX0_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP1_TX0_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW 0x6A
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP2_TX0_A_BAND_TSSI_SLOPE 0x6C
#define GRP2_TX0_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP2_TX0_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP2_TX0_A_BAND_TARGET_PWR 0x6E
#define GRP2_TX0_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI 0x70
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP3_TX0_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP3_TX0_A_BAND_TSSI_OFFSET 0x72
#define GRP3_TX0_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP3_TX0_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW 0x74
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP4_TX0_A_BAND_TSSI_SLOPE 0x76
#define GRP4_TX0_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP4_TX0_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP4_TX0_A_BAND_TARGET_PWR 0x78
#define GRP4_TX0_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI 0x7A
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP5_TX0_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP5_TX0_A_BAND_TSSI_OFFSET 0x7C
#define GRP5_TX0_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP5_TX0_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW 0X7E
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP0_TX1_A_BAND_TSSI_SLOPE 0x80
#define GRP0_TX1_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP0_TX1_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP0_TX1_A_BAND_TARGET_PWR 0x82
#define GRP0_TX1_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI 0x84
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP1_TX1_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP1_TX1_A_BAND_TSSI_OFFSET 0x86
#define GRP1_TX1_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP1_TX1_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW 0x88
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP2_TX1_A_BAND_TSSI_SLOPE 0x8A
#define GRP2_TX1_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP2_TX1_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP2_TX1_A_BAND_TARGET_PWR 0x8C
#define GRP2_TX1_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI 0x8E
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP3_TX1_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP3_TX1_A_BAND_TSSI_OFFSET 0x90
#define GRP3_TX1_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP3_TX1_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW 0x92
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP4_TX1_A_BAND_TSSI_SLOPE 0x94
#define GRP4_TX1_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP4_TX1_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP4_TX1_A_BAND_TARGET_PWR 0x96
#define GRP4_TX1_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI 0x98
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP5_TX1_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP5_TX1_A_BAND_TSSI_OFFSET 0x9A
#define GRP5_TX1_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP5_TX1_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW 0x9C
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define G_BAND_BANDEDGE_PWR_BACK_OFF 0x9E
#define G_BAND_BANDEDGE_PWR_BACK_OFF_MASK (0x7f)
#define G_BAND_BANDEDGE_PWR_BACK_OFF_EN (1 << 7)
#define XTAL_TRIM2_MASK (0x7f << 8)
#define XTAL_TRIM2_DIP_SELECTION (1 << 15)

#define TX_PWR_CCK_1_2M 0xA0
#define TX_PWR_CCK_1_2M_MASK (0x3f)
#define TX_PWR_CCK_1_2M_SIGN (1 << 6)
#define TX_PWR_CCK_1_2M_EN (1 << 7)
#define TX_PWR_CCK_5_11M_MASK (0x3f << 8)
#define TX_PWR_CCK_5_11M_SIGN (1 << 14)
#define TX_PWR_CCK_5_11M_EN (1 << 15)

#define TX_PWR_G_BAND_OFDM_6_9M 0xA2
#define TX_PWR_G_BAND_OFDM_6_9M_MASK (0x3f)
#define TX_PWR_G_BAND_OFDM_6_9M_SIGN (1 << 6)
#define TX_PWR_G_BAND_OFDM_6_9M_EN (1 << 7)
#define TX_PWR_G_BAND_OFDM_12_18M_MASK (0x3f << 8)
#define TX_PWR_G_BAND_OFDM_12_18M_SIGN (1 << 14)
#define TX_PWR_G_BAND_OFDM_12_18M_EN (1 << 15)

#define TX_PWR_G_BAND_OFDM_24_36M 0xA4
#define TX_PWR_G_BAND_OFDM_24_36M_MASK (0x3f)
#define TX_PWR_G_BAND_OFDM_24_36M_SIGN (1 << 6)
#define TX_PWR_G_BAND_OFDM_24_36M_EN (1 << 7)
#define TX_PWR_G_BAND_OFDM_48_54M_MASK (0x3f << 8)
#define TX_PWR_G_BAND_OFDM_48_54M_SIGN (1 << 14)
#define TX_PWR_G_BAND_OFDM_48_54M_EN (1 << 15)

#define TX_PWR_HT_MCS_0_1 0xA6
#define TX_PWR_HT_MCS_0_1_MASK (0x3f)
#define TX_PWR_HT_MCS_0_1_SIGN (1 << 6)
#define TX_PWR_HT_MCS_0_1_EN (1 << 7)
#define TX_PWR_HT_MCS_2_3_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_2_3_SIGN (1 << 14)
#define TX_PWR_HT_MCS_2_3_EN (1 << 15)

#define TX_PWR_HT_MCS_4_5 0xA8
#define TX_PWR_HT_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_MCS_4_5_SIGN (1 << 6)
#define TX_PWR_HT_MCS_4_5_EN (1 << 7)
#define TX_PWR_HT_MCS_6_7_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_6_7_SIGN (1 << 14)
#define TX_PWR_HT_MCS_6_7_EN (1 << 15)

#define TX_PWR_HT_MCS_8_9 0xAA
#define TX_PWR_HT_MCS_8_9_MASK (0x3f)
#define TX_PWR_HT_MCS_8_9_SIGN (1 << 6)
#define TX_PWR_HT_MCS_8_9_EN (1 << 7)
#define TX_PWR_HT_MCS_10_11_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_10_11_SIGN (1 << 14)
#define TX_PWR_HT_MCS_10_11_EN (1 << 15)

#define TX_PWR_HT_MCS_12_13 0xAC
#define TX_PWR_HT_MCS_12_13_MASK (0x3f)
#define TX_PWR_HT_MCS_12_13_SIGN (1 << 6)
#define TX_PWR_HT_MCS_12_13_EN (1 << 7)
#define TX_PWR_HT_MCS_14_15_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_14_15_SIGN (1 << 14)
#define TX_PWR_HT_MCS_14_15_EN (1 << 15)

#define CONFIG_G_BAND_CHL 0xB0
#define CONFIG_G_BAND_CHL_GRP1_MASK (0xff)
#define CONFIG_G_BAND_CHL_GRP2_MASK (0xff << 8)

#define TX_PWR_A_BAND_OFDM_6_9M 0xB2
#define TX_PWR_A_BAND_OFDM_6_9M_MASK (0x3f)
#define TX_PWR_A_BAND_OFDM_6_9M_SIGN (1 << 6)
#define TX_PWR_A_BAND_OFDM_6_9M_EN (1 << 7)
#define TX_PWR_A_BAND_OFDM_12_18M_MASK (0x3f << 8)
#define TX_PWR_A_BAND_OFDM_12_18M_SIGN (1 << 14)
#define TX_PWR_A_BAND_OFDM_12_18M_EN (1 << 15)

#define TX_PWR_A_BAND_OFDM_24_36M 0xB4
#define TX_PWR_A_BAND_OFDM_24_36M_MASK (0x3f)
#define TX_PWR_A_BAND_OFDM_24_36M_SIGN (1 << 6)
#define TX_PWR_A_BAND_OFDM_24_36M_EN (1 << 7)
#define TX_PWR_A_BAND_OFDM_48_54M_MASK (0x3f << 8)
#define TX_PWR_A_BAND_OFDM_48_54M_SIGN (1 << 14)
#define TX_PWR_A_BAND_OFDM_48_54M_EN (1 << 15)

#define CONFIG1_A_BAND_CHL 0xB6
#define CONFIG1_A_BAND_CHL_GRP1_MASK 0xff
#define CONFIG1_A_BAND_CHL_GRP2_MASK (0xff << 8)

#define CONFIG2_A_BAND_CHL 0xB8
#define CONFIG2_A_BAND_CHL_GRP1_MASK (0xff)
#define CONFIG2_A_BAND_CHL_GRP2_MASK (0xff << 8)

#define TX_PWR_VHT_MCS_0_1 0xBA
#define TX_PWR_VHT_MCS_0_1_MASK (0x3f)
#define TX_PWR_VHT_MCS_0_1_SIGN (1 << 6)
#define TX_PWR_VHT_MCS_0_1_EN (1 << 7)
#define TX_PWR_VHT_MCS_2_3_MASK (0x3f << 8)
#define TX_PWR_VHT_MCS_2_3_SIGN (1 << 14)
#define TX_PWR_VHT_MCS_2_3_EN (1 << 15)

#define TX_PWR_VHT_MCS_4_5 0xBC
#define TX_PWR_VHT_MCS_4_5_MASK (0x3f)
#define TX_PWR_VHT_MCS_4_5_SIGN (1 << 6)
#define TX_PWR_VHT_MCS_4_5_EN (1 << 7)
#define TX_PWR_VHT_MCS_6_7_MASK (0x3f << 8)
#define TX_PWR_VHT_MCS_6_7_SIGN (1 << 14)
#define TX_PWR_VHT_MCS_6_7_EN (1 << 15)

#define TX_PWR_5G_VHT_MCS_8_9 0xBE
#define TX_PWR_5G_VHT_MCS_8_9_MASK (0x3f)
#define TX_PWR_5G_VHT_MCS_8_9_SIGN (1 << 6)
#define TX_PWR_5G_VHT_MCS_8_9_EN (1 << 7)
#define TX_PWR_2G_VHT_MCS_8_9_MASK (0x3f << 8)
#define TX_PWR_2G_VHT_MCS_8_9_SIGN (1 << 14)
#define TX_PWR_2G_VHT_MCS_8_9_EN (1 << 15)

#define CP_FT_VERSION 0xF6
#define CP_FT_VERSION_MASK 0xff

#define RF_2G_RX_HIGH_GAIN 0xF8
#define RF0_2G_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_2G_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_2G_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_2G_RX_HIGH_GAIN_SIGN (1 << 15)

#define RF_5G_GRP0_1_RX_HIGH_GAIN 0xFA
#define RF0_5G_GRP0_RX_HIGH_GAIN_MASK (0x07)
#define RF0_5G_GRP0_RX_HIGH_GAIN_SIGN (1 << 3)
#define RF1_5G_GRP0_RX_HIGH_GAIN_MASK (0x07 << 4)
#define RF1_5G_GRP0_RX_HIGH_GAIN_SIGN (1 << 7)
#define RF0_5G_GRP1_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_5G_GRP1_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_5G_GRP1_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_5G_GRP1_RX_HIGH_GAIN_SIGN (1 << 15)

#define RF_5G_GRP2_3_RX_HIGH_GAIN 0xFC
#define RF0_5G_GRP2_RX_HIGH_GAIN_MASK (0x07)
#define RF0_5G_GRP2_RX_HIGH_GAIN_SIGN (1 << 3)
#define RF1_5G_GRP2_RX_HIGH_GAIN_MASK (0x07 << 4)
#define RF1_5G_GRP2_RX_HIGH_GAIN_SIGN (1 << 7)
#define RF0_5G_GRP3_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_5G_GRP3_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_5G_GRP3_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_5G_GRP3_RX_HIGH_GAIN_SIGN (1 << 15)

#define RF_5G_GRP4_5_RX_HIGH_GAIN 0xFE
#define RF0_5G_GRP4_RX_HIGH_GAIN_MASK (0x07)
#define RF0_5G_GRP4_RX_HIGH_GAIN_SIGN (1 << 3)
#define RF1_5G_GRP4_RX_HIGH_GAIN_MASK (0x07 << 4)
#define RF1_5G_GRP4_RX_HIGH_GAIN_SIGN (1 << 7)
#define RF0_5G_GRP5_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_5G_GRP5_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_5G_GRP5_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_5G_GRP5_RX_HIGH_GAIN_SIGN (1 << 15)

#define BT_RCAL_RESULT 0x138
#define BT_VCDL_CALIBRATION 0x13C
#define BT_PMUCFG 0x13E

struct rtmp_adapter;



/*************************************************************************
  *	Public function declarations for usb-based prom chipset
  ************************************************************************/
u16 mt7612u_read_eeprom16(struct rtmp_adapter *pAd, unsigned short offset);





/*************************************************************************
  *	Public function declarations for prom operation callback functions setting
  ************************************************************************/
#endif /* __EEPROM_H__ */
