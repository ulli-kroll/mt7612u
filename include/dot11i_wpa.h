/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36 Taiyuan St., Jhubei City,
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2008, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	dot11i_wpa.h

	Revision History:
	Who 			When			What
	--------		----------		----------------------------------------------

*/

#ifndef	__DOT11I_WPA_H__
#define	__DOT11I_WPA_H__

#include "rtmp_type.h"
#include "dot11_base.h"

/* The length is the EAPoL-Key frame except key data field.
   Please refer to 802.11i-2004 ,Figure 43u in p.78 */
#define MIN_LEN_OF_EAPOL_KEY_MSG	95

/* The related length of the EAPOL Key frame */
#define LEN_KEY_DESC_NONCE			32
#define LEN_KEY_DESC_IV				16
#define LEN_KEY_DESC_RSC			8
#define LEN_KEY_DESC_ID				8
#define LEN_KEY_DESC_REPLAY			8
#define LEN_KEY_DESC_MIC			16

/* EAP Code Type */
#define EAP_CODE_REQUEST	1
#define EAP_CODE_RESPONSE	2
#define EAP_CODE_SUCCESS    3
#define EAP_CODE_FAILURE    4

/* EAPOL frame Protocol Version */
#define	EAPOL_VER					1
#define	EAPOL_VER2					2

/* EAPOL-KEY Descriptor Type */
#define	WPA1_KEY_DESC				0xfe
#define WPA2_KEY_DESC               0x02

/* Key Descriptor Version of Key Information */
#define	KEY_DESC_TKIP			1
#define	KEY_DESC_AES			2
#define KEY_DESC_EXT			3

#define IE_WPA					221
#define IE_RSN					48

#define WPA_KDE_TYPE			0xdd

/*EAP Packet Type */
#define	EAPPacket		0
#define	EAPOLStart		1
#define	EAPOLLogoff		2
#define	EAPOLKey		3
#define	EAPOLASFAlert	4
#define	EAPTtypeMax		5

#define PAIRWISEKEY					1
#define GROUPKEY					0

/* RSN IE Length definition */
#define MAX_LEN_OF_RSNIE         	255
#define MIN_LEN_OF_RSNIE         	18
#define MAX_LEN_GTK					32
#define MIN_LEN_GTK					5

#define LEN_PMK						32
#define LEN_PMKID					16
#define LEN_PMK_NAME				16

#define LEN_GMK						32

#define LEN_PTK_KCK					16
#define LEN_PTK_KEK					16
#define LEN_TK						16	/* The length Temporal key. */
#define LEN_TKIP_MIC				8	/* The length of TX/RX Mic of TKIP */
#define LEN_TK2						(2 * LEN_TKIP_MIC)
#define LEN_PTK						(LEN_PTK_KCK + LEN_PTK_KEK + LEN_TK + LEN_TK2)

#define LEN_TKIP_PTK				LEN_PTK
#define LEN_AES_PTK					(LEN_PTK_KCK + LEN_PTK_KEK + LEN_TK)
#define LEN_TKIP_GTK				(LEN_TK + LEN_TK2)
#define LEN_AES_GTK					LEN_TK
#define LEN_TKIP_TK					(LEN_TK + LEN_TK2)
#define LEN_AES_TK					LEN_TK

#define LEN_WEP64					5
#define LEN_WEP128					13

#define OFFSET_OF_PTK_TK			(LEN_PTK_KCK + LEN_PTK_KEK)	/* The offset of the PTK Temporal key in PTK */
#define OFFSET_OF_AP_TKIP_TX_MIC	(OFFSET_OF_PTK_TK + LEN_TK)
#define OFFSET_OF_AP_TKIP_RX_MIC	(OFFSET_OF_AP_TKIP_TX_MIC + LEN_TKIP_MIC)
#define OFFSET_OF_STA_TKIP_RX_MIC	(OFFSET_OF_PTK_TK + LEN_TK)
#define OFFSET_OF_STA_TKIP_TX_MIC	(OFFSET_OF_AP_TKIP_TX_MIC + LEN_TKIP_MIC)

#define LEN_KDE_HDR					6
#define LEN_NONCE					32
#define LEN_PN						6
#define LEN_TKIP_IV_HDR				8
#define LEN_CCMP_HDR				8
#define LEN_CCMP_MIC				8
#define LEN_OUI_SUITE				4
#define LEN_WEP_TSC					3
#define LEN_WPA_TSC					6
#define LEN_WEP_IV_HDR				4
#define LEN_ICV						4

/* It's defined in IEEE Std 802.11-2007 Table 8-4 */
typedef enum _WPA_KDE_ID
{
   	KDE_RESV0,
   	KDE_GTK,
   	KDE_RESV2,
   	KDE_MAC_ADDR,
   	KDE_PMKID,
   	KDE_SMK,
   	KDE_NONCE,
   	KDE_LIFETIME,
   	KDE_ERROR,
   	KDE_RESV_OTHER
} WPA_KDE_ID;

/* EAPOL Key Information definition within Key descriptor format */
typedef	struct GNU_PACKED _KEY_INFO
{
#ifdef RT_BIG_ENDIAN
	u8 KeyAck:1;
    u8 Install:1;
    u8 KeyIndex:2;
    u8 KeyType:1;
    u8 KeyDescVer:3;
    u8 Rsvd:3;
    u8 EKD_DL:1;		/* EKD for AP; DL for STA */
    u8 Request:1;
    u8 Error:1;
    u8 Secure:1;
    u8 KeyMic:1;
#else
	u8 KeyMic:1;
	u8 Secure:1;
	u8 Error:1;
	u8 Request:1;
	u8 EKD_DL:1;       /* EKD for AP; DL for STA */
	u8 Rsvd:3;
	u8 KeyDescVer:3;
	u8 KeyType:1;
	u8 KeyIndex:2;
	u8 Install:1;
	u8 KeyAck:1;
#endif
}	KEY_INFO, *PKEY_INFO;

/* EAPOL Key descriptor format */
typedef	struct GNU_PACKED _KEY_DESCRIPTER
{
	u8 	Type;
	KEY_INFO	KeyInfo;
	u8 	KeyLength[2];
	u8 	ReplayCounter[LEN_KEY_DESC_REPLAY];
	u8 	KeyNonce[LEN_KEY_DESC_NONCE];
	u8 	KeyIv[LEN_KEY_DESC_IV];
	u8 	KeyRsc[LEN_KEY_DESC_RSC];
	u8 	KeyId[LEN_KEY_DESC_ID];
	u8 	KeyMic[LEN_KEY_DESC_MIC];
	u8 	KeyDataLen[2];
	u8 	KeyData[0];
}	KEY_DESCRIPTER, *PKEY_DESCRIPTER;

typedef	struct GNU_PACKED _EAPOL_PACKET
{
	u8  			ProVer;
	u8  			ProType;
	u8  			Body_Len[2];
	KEY_DESCRIPTER		KeyDesc;
}	EAPOL_PACKET, *PEAPOL_PACKET;

typedef struct GNU_PACKED _KDE_HDR
{
    u8               Type;
    u8               Len;
    u8               OUI[3];
    u8               DataType;
	u8 			octet[0];
}   KDE_HDR, *PKDE_HDR;

/*802.11i D10 page 83 */
typedef struct GNU_PACKED _GTK_KDE
{
#ifndef RT_BIG_ENDIAN
    u8               Kid:2;
    u8               tx:1;
    u8               rsv:5;
    u8               rsv1;
#else
    u8               rsv:5;
    u8               tx:1;
    u8               Kid:2;
    u8               rsv1;
#endif
    u8               GTK[0];
}   GTK_KDE, *PGTK_KDE;

/* For WPA1 */
typedef struct GNU_PACKED _RSNIE {
    u8   oui[4];
    unsigned short  version;
    u8   mcast[4];
    unsigned short  ucount;
    struct GNU_PACKED {
        u8 oui[4];
    }ucast[1];
} RSNIE, *PRSNIE;

/* For WPA2 */
typedef struct GNU_PACKED _RSNIE2 {
    unsigned short  version;
    u8   mcast[4];
    unsigned short  ucount;
    struct GNU_PACKED {
        u8 oui[4];
    }ucast[1];
} RSNIE2, *PRSNIE2;

/* AKM Suite */
typedef struct GNU_PACKED _RSNIE_AUTH {
    unsigned short acount;
    struct GNU_PACKED {
        u8 oui[4];
    }auth[1];
} RSNIE_AUTH,*PRSNIE_AUTH;

/* PMKID List */
typedef struct GNU_PACKED _RSNIE_PMKID {
    unsigned short pcount;
    struct GNU_PACKED {
        u8 list[16];
    }pmkid[1];
} RSNIE_PMKID,*PRSNIE_PMKID;

typedef	union GNU_PACKED _RSN_CAPABILITIES	{
	struct	GNU_PACKED {
#ifdef RT_BIG_ENDIAN
        unsigned short 	Rsvd:8;
		unsigned short 	MFPC:1;
		unsigned short 	MFPR:1;
        unsigned short 	GTKSA_R_Counter:2;
        unsigned short 	PTKSA_R_Counter:2;
        unsigned short 	No_Pairwise:1;
		unsigned short 	PreAuth:1;
#else
        unsigned short 	PreAuth:1;
		unsigned short 	No_Pairwise:1;
		unsigned short 	PTKSA_R_Counter:2;
		unsigned short 	GTKSA_R_Counter:2;
		unsigned short 	MFPR:1;
		unsigned short 	MFPC:1;
		unsigned short 	Rsvd:8;
#endif
	}	field;
	unsigned short 		word;
}	RSN_CAPABILITIES, *PRSN_CAPABILITIES;

typedef struct GNU_PACKED _EAP_HDR {
    u8   ProVer;
    u8   ProType;
    u8   Body_Len[2];
    u8   code;
    u8   identifier;
    u8   length[2]; /* including code and identifier, followed by length-2 octets of data */
} EAP_HDR, *PEAP_HDR;


#endif /* __DOT11I_WPA_H__ */

