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
	ee_efuse.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RTMP_EFUSE_SUPPORT

#include	"rt_config.h"

/* eFuse registers */
#define EFUSE_CTRL				0x0580
#define EFUSE_DATA0				0x0590
#define EFUSE_DATA1				0x0594
#define EFUSE_DATA2				0x0598
#define EFUSE_DATA3				0x059c

#define EFUSE_CTRL_3290			0x24
#define EFUSE_DATA0_3290		0x28
#define EFUSE_DATA1_3290		0x2c
#define EFUSE_DATA2_3290		0x30
#define EFUSE_DATA3_3290		0x34

#define EFUSE_TAG				0x2fe

#ifdef RT_BIG_ENDIAN
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		uint32_t            SEL_EFUSE:1;
		uint32_t            EFSROM_KICK:1;
		uint32_t            RESERVED:4;
		uint32_t            EFSROM_AIN:10;
		uint32_t            EFSROM_LDO_ON_TIME:2;
		uint32_t            EFSROM_LDO_OFF_TIME:6;
		uint32_t            EFSROM_MODE:2;
		uint32_t            EFSROM_AOUT:6;
	}	field;
	uint32_t 		word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#else
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		uint32_t            EFSROM_AOUT:6;
		uint32_t            EFSROM_MODE:2;
		uint32_t            EFSROM_LDO_OFF_TIME:6;
		uint32_t            EFSROM_LDO_ON_TIME:2;
		uint32_t            EFSROM_AIN:10;
		uint32_t            RESERVED:4;
		uint32_t            EFSROM_KICK:1;
		uint32_t            SEL_EFUSE:1;
	}	field;
	uint32_t 		word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

static int eFuseWriteRegistersFromBin(
	IN	struct rtmp_adapter *pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData);


#endif /* RTMP_EFUSE_SUPPORT */

