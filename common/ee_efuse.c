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



/*
========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

========================================================================
*/
VOID eFusePhysicalReadRegisters(
	IN	struct rtmp_adapter *pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	uint32_t data;
	uint32_t efuse_ctrl_reg = EFUSE_CTRL;

#if defined(RT3290) || defined(RT65xx) || defined(MT7601)
	if (IS_RT65XX(pAd) || IS_MT7601(pAd))
		efuse_ctrl_reg = EFUSE_CTRL_3290;
#endif /* defined(RT3290) || defined(RT65xx) || defined(MT7601) */

	eFuseCtrlStruc.word = mt7612u_read32(pAd, efuse_ctrl_reg);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.*/
	/*Read in physical view*/
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	memmove(&data, &eFuseCtrlStruc, 4);
	mt7612u_write32(pAd, efuse_ctrl_reg, data);

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while(i < 500)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return;

		eFuseCtrlStruc.word = mt7612u_read32(pAd, EFUSE_CTRL);
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RtmpusecDelay(2);
			break;
		}

		RtmpusecDelay(2);
		i++;
	}

	/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)*/
	/*Because the size of each EFUSE_DATA is 4 Bytes, the size of address of each is 2 bits.*/
	/*The previous 2 bits is the EFUSE_DATA number, the last 2 bits is used to decide which bytes*/
	/*Decide which EFUSE_DATA to read*/
	/*590:F E D C */
	/*594:B A 9 8 */
	/*598:7 6 5 4*/
	/*59C:3 2 1 0*/
#if defined(RT3290) || defined(RT65xx) || defined(MT7601)
	if (IS_RT65XX(pAd) || IS_MT7601(pAd))
		efuseDataOffset =  EFUSE_DATA0_3290 + (Offset & 0xC)  ;
	else
#endif /* defined(RT3290) || defined(RT65xx) || defined(MT7601) */
	efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;

	data = mt7612u_read32(pAd, efuseDataOffset);

#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));
#else
	data = data >> (8*(Offset & 0x3));
#endif /* RT_BIG_ENDIAN */

	memmove(pData, &data, Length);

}

VOID eFuseGetFreeBlockCount(IN struct rtmp_adapter *pAd,
	PUINT EfuseFreeBlock)
{

	USHORT i=0, StartBlock=0, EndBlock=0;
	USHORT	LogicalAddress;
	USHORT	FirstFreeBlock = 0xffff, LastFreeBlock = 0xffff;

	*EfuseFreeBlock = 0;
	/* find first free block*/
	if( (pAd->chipCap.EFUSE_USAGE_MAP_START % 2) != 0)
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_START-1;
	}
	else
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_START;
	}

	if( (pAd->chipCap.EFUSE_USAGE_MAP_END % 2) != 0)
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_END-1;
	}
	else
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_END;
	}

	for (i = StartBlock; i <= EndBlock; i+=2)
	{
		eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);

		if( (LogicalAddress & 0xff) == 0)
		{
			if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
			{
				FirstFreeBlock = i;
				break;
			}
		}

		if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
			{
				FirstFreeBlock = i+1;
				break;
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, FirstFreeBlock= 0x%x\n", FirstFreeBlock));

	/*if not find, return free block number = 0*/
	if(FirstFreeBlock == 0xffff)
	{
		*EfuseFreeBlock = 0;
		return;
	}
	for (i = EndBlock; i >= StartBlock; i-=2)
	{
		eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);

		if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
			{
				LastFreeBlock = i+1;
				break;
			}
		}

		if( (LogicalAddress & 0xff) == 0)
		{
			if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
			{
				LastFreeBlock = i;
				break;
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, LastFreeBlock= 0x%x\n", LastFreeBlock));

	/*if not find last free block, return free block number = 0, this should not happen since we have checked first free block number previously*/
	if(LastFreeBlock == 0xffff)
	{
		*EfuseFreeBlock = 0;
		return;
	}

	/* return total free block number, last free block number must >= first free block number*/
	if(LastFreeBlock < FirstFreeBlock)
	{
		*EfuseFreeBlock = 0;
	}
	else
	{
		*EfuseFreeBlock = LastFreeBlock - FirstFreeBlock + 1;
	}

	DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount is %d\n",*EfuseFreeBlock));
}

#endif /* RTMP_EFUSE_SUPPORT */

