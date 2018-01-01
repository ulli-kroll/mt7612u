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
	rtmp_mcu.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

INT mcu_sys_init(struct rtmp_adapter *pAd)
{
	int Status;

	/* Load MCU firmware*/
	mt7612u_mcu_ctrl_init(pAd);

	Status = mt7612u_mcu_usb_load_rom_patch(pAd);
	if (Status < 0) {
		DBGPRINT_ERR(("load patch failed, Status[=0x%08x]\n", Status));
		return false;
	}

	Status = mt7612u_mcu_usb_loadfw(pAd);
	if (Status < 0) {
		DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		return false;
	}

	return true;
}
