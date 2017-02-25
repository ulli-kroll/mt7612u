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
	mt7612u_write32(pAd, E2PROM_CSR, *x);
	RtmpusecDelay(1);				/* Max frequency = 1MHz in Spec. definition */
}

/* IRQL = PASSIVE_LEVEL*/
static inline VOID LowerClock(
    IN	struct rtmp_adapter *pAd,
    IN  uint32_t *x)
{
	*x = *x & ~EESK;
	mt7612u_write32(pAd, E2PROM_CSR, *x);
	RtmpusecDelay(1);
}

/* IRQL = PASSIVE_LEVEL*/
static inline USHORT ShiftInBits(
	IN struct rtmp_adapter *pAd)
{
	uint32_t 	x,i;
	USHORT      data=0;

	x = mt7612u_read32(pAd, E2PROM_CSR);

	x &= ~( EEDO | EEDI);

	for(i=0; i<16; i++)
	{
		data = data << 1;
		RaiseClock(pAd, &x);

		x = mt7612u_read32(pAd, E2PROM_CSR);
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
	x = mt7612u_read32(pAd, E2PROM_CSR);

	x &= ~(EEDO | EEDI);

	do
	{
	    x &= ~EEDI;
	    if(data & mask)		x |= EEDI;

	    mt7612u_write32(pAd, E2PROM_CSR, x);

	    RaiseClock(pAd, &x);
	    LowerClock(pAd, &x);

	    mask = mask >> 1;
	} while(mask);

	x &= ~EEDI;
	mt7612u_write32(pAd, E2PROM_CSR, x);
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID EEpromCleanup(
	IN struct rtmp_adapter *pAd)
{
	uint32_t x;

	x = mt7612u_read32(pAd, E2PROM_CSR);

	x &= ~(EECS | EEDI);
	mt7612u_write32(pAd, E2PROM_CSR, x);

	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
}


static inline VOID EWEN(
	IN struct rtmp_adapter *pAd)
{
	uint32_t x;

	/* reset bits and set EECS*/
	x = mt7612u_read32(pAd, E2PROM_CSR);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	mt7612u_write32(pAd, E2PROM_CSR, x);

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
	x = mt7612u_read32(pAd, E2PROM_CSR);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	mt7612u_write32(pAd, E2PROM_CSR, x);

	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWDS_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);
}
