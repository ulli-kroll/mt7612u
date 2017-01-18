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


INT RtmpChipOpsEepromHook(
	IN struct rtmp_adapter 	*pAd,
	IN INT				infType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	uint32_t val;


	/* Hook functions based on interface types for EEPROM */
	switch (infType)
	{

#ifdef RTMP_USB_SUPPORT
		case RTMP_DEV_INF_USB:
			pChipOps->eeinit = NULL;
			pChipOps->eeread = RTUSBReadEEPROM16;
			pChipOps->eewrite = RTUSBWriteEEPROM16;
			break;
#endif /* RTMP_USB_SUPPORT */

		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::hook failed\n", __FUNCTION__));
			break;
	}

	DBGPRINT(RT_DEBUG_OFF, ("NVM is EEPROM mode\n"));
	return 0;
}
INT rtmp_ee_bin_read16(
	IN struct rtmp_adapter 	*pAd,
	IN USHORT 			Offset,
	OUT USHORT 			*pValue)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s::Read from EEPROM buffer\n", __FUNCTION__));
	memmove(pValue, &(pAd->EEPROMImage[Offset]), 2);
	*pValue = le2cpu16(*pValue);

	return (*pValue);
}


INT rtmp_ee_bin_write16(
	IN struct rtmp_adapter 	*pAd,
	IN USHORT 			Offset,
	IN USHORT 			data)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s::Write to EEPROM buffer\n", __FUNCTION__));
	data = le2cpu16(data);
	memmove(&(pAd->EEPROMImage[Offset]), &data, 2);

	return 0;
}


INT rtmp_ee_load_from_bin(
	IN struct rtmp_adapter *	pAd)
{
	char *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;

	src = BIN_FILE_PATH;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else
		{
			memset(pAd->EEPROMImage, 0, MAX_EEPROM_BIN_FILE_SIZE);
			ret_val = RtmpOSFileRead(srcf, (char *)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);

			if (ret_val > 0)
				ret_val = NDIS_STATUS_SUCCESS;
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s::Read file \"%s\" failed(errCode=%d)!\n", __FUNCTION__, src, ret_val));
      		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;
}


INT rtmp_ee_write_to_bin(
	IN struct rtmp_adapter *	pAd)
{
	char *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;

	src = BIN_FILE_PATH;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_WRONLY|O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else
			RtmpOSFileWrite(srcf, (char *)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;
}

INT Set_EepromBufferWriteBack_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg)
{
	UINT e2p_mode = simple_strtol(arg, 0, 10);

	if (e2p_mode >= NUM_OF_E2P_MODE)
		return FALSE;

	switch (e2p_mode)
	{
#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to eFuse\n"));
			rtmp_ee_write_to_efuse(pAd);
			break;
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RT65xx
		case E2P_EEPROM_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to EEPROM\n"));
			rtmp_ee_write_to_prom(pAd);
			break;
#endif /* RT65xx */

		case E2P_BIN_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to BIN\n"));
			rtmp_ee_write_to_bin(pAd);
			break;

		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::do not support this EEPROM access mode\n", __FUNCTION__));
			return FALSE;
	}

	return TRUE;
}

