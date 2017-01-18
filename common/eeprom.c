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
	eeprom.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#include "rt_config.h"

struct chip_map{
	uint32_t ChipVersion;
	char *name;
};

struct chip_map chip_card_id_map[] ={
	{7620, ""},
};


INT RtmpChipOpsEepromHook(struct rtmp_adapter *pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	uint32_t val;


	/* Hook functions based on interface types for EEPROM */
	pChipOps->eeread = RTUSBReadEEPROM16;
	pChipOps->eewrite = RTUSBWriteEEPROM16;

	return 0;
}
