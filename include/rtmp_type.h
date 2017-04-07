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
    rtmp_type.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Paul Lin    1-2-2004
*/

#ifndef __RTMP_TYPE_H__
#define __RTMP_TYPE_H__

#include <linux/types.h>

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */


#ifdef LINUX
/* Put platform dependent declaration here */
/* For example, linux type definition */
typedef unsigned char UINT8;
typedef short INT16;

typedef unsigned int UINT;
typedef unsigned long ULONG;
#endif /* LINUX */

/* modified for fixing compile warning on Sigma 8634 platform */
typedef char STRING;

typedef signed char CHAR;

typedef signed int INT;
typedef signed long LONG;
typedef signed long long LONGLONG;

typedef unsigned long long ULONGLONG;

#ifdef LINUX
typedef void VOID;
#endif /* LINUX */

typedef VOID *PVOID;
typedef CHAR *PCHAR;
typedef LONG *PLONG;

typedef unsigned int NDIS_MEDIA_STATE;

typedef union _LARGE_INTEGER {
	struct {
#ifdef RT_BIG_ENDIAN
		int32_t HighPart;
		UINT LowPart;
#else
		UINT LowPart;
		int32_t HighPart;
#endif
	} u;
	int64_t QuadPart;
} LARGE_INTEGER;


/* Register set pair for initialzation register set definition */
typedef struct _RTMP_REG_PAIR {
	uint32_t Register;
	uint32_t Value;
} RTMP_REG_PAIR, *PRTMP_REG_PAIR;

typedef struct _REG_PAIR {
	u8 Register;
	u8 Value;
} REG_PAIR, *PREG_PAIR;

typedef struct _REG_PAIR_CHANNEL {
	u8 Register;
	u8 FirstChannel;
	u8 LastChannel;
	u8 Value;
} REG_PAIR_CHANNEL, *PREG_PAIR_CHANNEL;

typedef struct _REG_PAIR_BW {
	u8 Register;
	u8 BW;
	u8 Value;
} REG_PAIR_BW, *PREG_PAIR_BW;


typedef struct _REG_PAIR_PHY{
	u8 reg;
	u8 s_ch;
	u8 e_ch;
	u8 phy;	/* RF_MODE_XXX */
	u8 bw;	/* RF_BW_XX */
	u8 val;
}REG_PAIR_PHY;


/* Register set pair for initialzation register set definition */
typedef struct _RTMP_RF_REGS {
	u8 Channel;
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R4;
} RTMP_RF_REGS, *PRTMP_RF_REGS;

typedef struct _FREQUENCY_ITEM {
	u8 Channel;
	u8 N;
	u8 R;
	u8 K;
} FREQUENCY_ITEM, *PFREQUENCY_ITEM;

#define STATUS_SUCCESS			0x00
#define STATUS_UNSUCCESSFUL 		0x01

typedef struct _QUEUE_ENTRY {
	struct _QUEUE_ENTRY *Next;
} QUEUE_ENTRY, *PQUEUE_ENTRY;

/* Queue structure */
typedef struct _QUEUE_HEADER {
	PQUEUE_ENTRY Head;
	PQUEUE_ENTRY Tail;
	UINT Number;
} QUEUE_HEADER, *PQUEUE_HEADER;

typedef struct _CR_REG {
	uint32_t flags;
	uint32_t offset;
	uint32_t value;
} CR_REG, *PCR_REG;

typedef struct _BANK_RF_CR_REG {
	uint32_t flags;
	u8 bank;
	u8 offset;
	u8 value;
} BANK_RF_CR_REG, *PBANK_RF_CR_REG;

struct mt_dev_priv{
	struct rtmp_adapter *sys_handle;
	struct rtmp_wifi_dev *rtmp_wifi_dev;
	unsigned long priv_flags;
	u8 sniffer_mode;
};

#endif /* __RTMP_TYPE_H__ */

