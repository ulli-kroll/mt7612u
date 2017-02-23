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
	ee_prom.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"



/* IRQL = PASSIVE_LEVEL*/
static inline VOID RaiseClock(
    IN	struct rtmp_adapter *pAd,
    IN  uint32_t *x)
{
	*x = *x | EESK;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
	RtmpusecDelay(1);				/* Max frequency = 1MHz in Spec. definition */
}

/* IRQL = PASSIVE_LEVEL*/
static inline VOID LowerClock(
    IN	struct rtmp_adapter *pAd,
    IN  uint32_t *x)
{
	*x = *x & ~EESK;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
	RtmpusecDelay(1);
}

/* IRQL = PASSIVE_LEVEL*/
static inline USHORT ShiftInBits(
	IN struct rtmp_adapter *pAd)
{
	uint32_t 	x,i;
	USHORT      data=0;

	mt7612u_read32(pAd, E2PROM_CSR, &x);

	x &= ~( EEDO | EEDI);

	for(i=0; i<16; i++)
	{
		data = data << 1;
		RaiseClock(pAd, &x);

		mt7612u_read32(pAd, E2PROM_CSR, &x);
		LowerClock(pAd, &x); /*prevent read failed*/

		x &= ~(EEDI);
		if(x & EEDO)
		    data |= 1;
	}

	return data;
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID ShiftOutBits(
	IN struct rtmp_adapter *pAd,
	IN USHORT			data,
	IN USHORT			count)
{
	uint32_t       x,mask;

	mask = 0x01 << (count - 1);
	mt7612u_read32(pAd, E2PROM_CSR, &x);

	x &= ~(EEDO | EEDI);

	do
	{
	    x &= ~EEDI;
	    if(data & mask)		x |= EEDI;

	    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	    RaiseClock(pAd, &x);
	    LowerClock(pAd, &x);

	    mask = mask >> 1;
	} while(mask);

	x &= ~EEDI;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID EEpromCleanup(
	IN struct rtmp_adapter *pAd)
{
	uint32_t x;

	mt7612u_read32(pAd, E2PROM_CSR, &x);

	x &= ~(EECS | EEDI);
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
}


static inline VOID EWEN(
	IN struct rtmp_adapter *pAd)
{
	uint32_t x;

	/* reset bits and set EECS*/
	mt7612u_read32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWEN_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);
}


static inline VOID EWDS(
	IN struct rtmp_adapter *pAd)
{
	uint32_t x;

	/* reset bits and set EECS*/
	mt7612u_read32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWDS_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);
}


/* IRQL = PASSIVE_LEVEL*/
int rtmp_ee_prom_read16(
	IN struct rtmp_adapter *pAd,
	IN USHORT Offset,
	OUT USHORT *pValue)
{
	uint32_t x;
	USHORT data;



	Offset /= 2;
	/* reset bits and set EECS*/
	mt7612u_read32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* patch can not access e-Fuse issue*/

	/* output the read_opcode and register number in that order    */
	ShiftOutBits(pAd, EEPROM_READ_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);

	/* Now read the data (16 bits) in from the selected EEPROM word*/
	data = ShiftInBits(pAd);

	EEpromCleanup(pAd);


	*pValue = data;

	return NDIS_STATUS_SUCCESS;
}


int rtmp_ee_prom_write16(
    IN struct rtmp_adapter *pAd,
    IN USHORT Offset,
    IN USHORT Data)
{
	uint32_t x;



	Offset /= 2;

	EWEN(pAd);

	/* reset bits and set EECS*/
	mt7612u_read32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* patch can not access e-Fuse issue*/

	/* output the read_opcode ,register number and data in that order */
	ShiftOutBits(pAd, EEPROM_WRITE_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);
	ShiftOutBits(pAd, Data, 16);		/* 16-bit access*/

	/* read DO status*/
	mt7612u_read32(pAd, E2PROM_CSR, &x);

	EEpromCleanup(pAd);

	RtmpusecDelay(10000);	/*delay for twp(MAX)=10ms*/

	EWDS(pAd);

	EEpromCleanup(pAd);


	return NDIS_STATUS_SUCCESS;

}


INT rtmp_ee_write_to_prom(
	IN struct rtmp_adapter *	pAd)
{
	USHORT value, offset = 0;

#ifdef RTMP_USB_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_USB)
		RTUSBWriteEEPROM(pAd, offset, (u8 *)pAd->EEPROMImage, EEPROM_SIZE);
	else
#endif /* RTMP_USB_SUPPORT */
	{
		for (offset = 0; offset < (EEPROM_SIZE >> 1);)
		{
			value = ((PUSHORT)pAd->EEPROMImage)[offset];
			RT28xx_EEPROM_WRITE16(pAd, (offset << 1), value);
			offset++;
		}
	}

	return TRUE;
}
