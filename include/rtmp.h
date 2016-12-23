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
    rtmp.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    2002-08-01    created
    James Tan   2002-09-06    modified (Revise NTCRegTable)
    John Chang  2004-09-06    modified for RT2600
*/
#ifndef __RTMP_H__
#define __RTMP_H__

#include "link_list.h"
#include "spectrum_def.h"

#include "rtmp_dot11.h"
#include "wpa_cmm.h"

#ifdef CONFIG_AP_SUPPORT
#include "ap_autoChSel_cmm.h"
#endif /* CONFIG_AP_SUPPORT */

#include "wsc.h"



#include "rtmp_chip.h"



#ifdef DOT11W_PMF_SUPPORT
#include "pmf_cmm.h"
#endif /* DOT11W_PMF_SUPPORT */






#ifdef RT_CFG80211_SUPPORT
#include "cfg80211_cmm.h"
#endif /* RT_CFG80211_SUPPORT */


#include "drs_extr.h"

struct _RTMP_RA_LEGACY_TB;

typedef struct _RTMP_CHIP_OP_ RTMP_CHIP_OP;
typedef struct _RTMP_CHIP_CAP_ RTMP_CHIP_CAP;

#ifdef BB_SOC
#include "os/bb_soc.h"
#endif

typedef struct _UAPSD_INFO {
	BOOLEAN bAPSDCapable;
} UAPSD_INFO;

#include "mcu/mcu.h"


#ifdef CONFIG_ANDES_SUPPORT
#include "mcu/mcu_and.h"
#endif

#include "radar.h"


#ifdef DFS_SUPPORT
#include "dfs.h"
#endif /* DFS_SUPPORT */


#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#include "frq_cal.h"
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

#ifdef RALINK_ATE
#include "rt_ate.h"
#endif /* RALINK_ATE */







// TODO: shiang-6590, remove it after ATE fully re-organized! copy from rtmp_bbp.h
#ifndef MAX_BBP_ID
	#define MAX_BBP_ID	136



#endif
// TODO: ---End


/*#define DBG		1 */

/*#define DBG_DIAGNOSE		1 */


/*+++Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
#define MAX_DATAMM_RETRY	3
#define MGMT_USE_QUEUE_FLAG	0x80
/*---Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
/* The number of channels for per-channel Tx power offset */


#define	MAXSEQ		(0xFFF)

#ifdef DOT11N_SS3_SUPPORT
#define MAX_MCS_SET 24		/* From MCS 0 ~ MCS 23 */
#else
#define MAX_MCS_SET 16		/* From MCS 0 ~ MCS 15 */
#endif /* DOT11N_SS3_SUPPORT */
#define MAX_VHT_MCS_SET 	20 /* for 1ss~ 2ss with MCS0~9 */


#define MAX_TXPOWER_ARRAY_SIZE	5

#define MAX_EEPROM_BUFFER_SIZE	1024

extern unsigned char CISCO_OUI[];
extern UCHAR BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern char *CipherName[];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR TPID[];
extern UCHAR APPLE_TALK[2];
extern UCHAR OfdmRateToRxwiMCS[];
extern UCHAR WMM_UP2AC_MAP[8];

extern unsigned char RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR CipherSuiteWpaNoneTkip[];
extern UCHAR CipherSuiteWpaNoneTkipLen;

extern UCHAR CipherSuiteWpaNoneAes[];
extern UCHAR CipherSuiteWpaNoneAesLen;

extern UCHAR SsidIe;
extern UCHAR SupRateIe;
extern UCHAR ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR HtCapIe;
extern UCHAR AddHtInfoIe;
extern UCHAR NewExtChanIe;
extern UCHAR BssCoexistIe;
extern UCHAR ExtHtCapIe;
#endif /* DOT11_N_SUPPORT */
extern UCHAR ExtCapIe;

extern UCHAR ErpIe;
extern UCHAR DsIe;
extern UCHAR TimIe;
extern UCHAR WpaIe;
extern UCHAR Wpa2Ie;
extern UCHAR IbssIe;
extern UCHAR WapiIe;

extern UCHAR WPA_OUI[];
extern UCHAR RSN_OUI[];
extern UCHAR WAPI_OUI[];
extern UCHAR WME_INFO_ELEM[];
extern UCHAR WME_PARM_ELEM[];
extern UCHAR RALINK_OUI[];
extern UCHAR PowerConstraintIE[];

struct _RX_BLK;

typedef union _CAPTURE_MODE_PACKET_BUFFER {
	struct
	{
		uint32_t       BYTE0:8;
		uint32_t       BYTE1:8;
		uint32_t       BYTE2:8;
		uint32_t       BYTE3:8;
	} field;
	uint32_t                   Value;
}CAPTURE_MODE_PACKET_BUFFER, *PCAPTURE_MODE_PACKET_BUFFER;

typedef struct _RSSI_SAMPLE {
	CHAR LastRssi0;		/* last received RSSI */
	CHAR LastRssi1;		/* last received RSSI */
	CHAR LastRssi2;		/* last received RSSI */
	CHAR AvgRssi0;
	CHAR AvgRssi1;
	CHAR AvgRssi2;
	SHORT AvgRssi0X8;
	SHORT AvgRssi1X8;
	SHORT AvgRssi2X8;
	CHAR LastSnr0;
	CHAR LastSnr1;
	CHAR LastSnr2;
	CHAR AvgSnr0;
	CHAR AvgSnr1;
	CHAR AvgSnr2;
	SHORT AvgSnr0X8;
	SHORT AvgSnr1X8;
	SHORT AvgSnr2X8;
	CHAR LastNoiseLevel0;
	CHAR LastNoiseLevel1;
	CHAR LastNoiseLevel2;
} RSSI_SAMPLE;


struct raw_rssi_info;

struct rx_signal_info{
	CHAR raw_rssi[3];
	UCHAR raw_snr[3];
	CHAR freq_offset;
};


/*
	Queue structure and macros
*/
#define InitializeQueueHeader(QueueHeader)              \
{                                                       \
	(QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
	(QueueHeader)->Number = 0;                          \
}

#define RemoveHeadQueue(QueueHeader)                \
(QueueHeader)->Head;                                \
{                                                   \
	PQUEUE_ENTRY pNext;                             \
	if ((QueueHeader)->Head != NULL)				\
	{												\
		pNext = (QueueHeader)->Head->Next;          \
		(QueueHeader)->Head->Next = NULL;		\
		(QueueHeader)->Head = pNext;                \
		if (pNext == NULL)                          \
			(QueueHeader)->Tail = NULL;             \
		(QueueHeader)->Number--;                    \
	}												\
}

#define InsertHeadQueue(QueueHeader, QueueEntry)            \
{                                                           \
		((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
		if ((QueueHeader)->Tail == NULL)                        \
			(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
		(QueueHeader)->Number++;                                \
}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
{                                                               \
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
	if ((QueueHeader)->Tail)                                    \
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
	else                                                        \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
	(QueueHeader)->Number++;                                    \
}

#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)			\
{																		\
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;							\
	if ((QueueHeader)->Tail)											\
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);			\
	else																\
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);				\
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);					\
	(QueueHeader)->Number++;											\
}


/* */
/*  Macros for flag and ref count operations */
/* */
#define RTMP_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))
/* Macro for power save flag. */
#define RTMP_SET_PSFLAG(_M, _F)       ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)     ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)        ((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)      (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)     (((_M)->PSFlags & (_F)) == (_F))

#define OPSTATUS_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define WIFI_TEST_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.WiFiTestFlags |= (_F))
#define WIFI_TEST_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.WiFiTestFlags &= ~(_F))
#define WIFI_TEST_CHECK_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.WiFiTestFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)    ((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#define RTMP_SET_MORE_FLAG(_M, _F)       ((_M)->MoreFlags |= (_F))
#define RTMP_TEST_MORE_FLAG(_M, _F)      (((_M)->MoreFlags & (_F)) != 0)
#define RTMP_CLEAR_MORE_FLAG(_M, _F)     ((_M)->MoreFlags &= ~(_F))

#define SET_ASIC_CAP(_pAd, _caps)		((_pAd)->chipCap.asic_caps |= (_caps))
#define IS_ASIC_CAP(_pAd, _caps)			(((_pAd)->chipCap.asic_caps & (_caps)) != 0)
#define CLR_ASIC_CAP(_pAd, _caps)		((_pAd)->chipCap.asic_caps &= ~(_caps))


#ifdef CONFIG_STA_SUPPORT
#define STA_NO_SECURITY_ON(_p)          (_p->StaCfg.wdev.WepStatus == Ndis802_11EncryptionDisabled)
#define STA_WEP_ON(_p)                  (_p->StaCfg.wdev.WepStatus == Ndis802_11WEPEnabled)
#define STA_TKIP_ON(_p)                 (_p->StaCfg.wdev.WepStatus == Ndis802_11TKIPEnable)
#define STA_AES_ON(_p)                  (_p->StaCfg.wdev.WepStatus == Ndis802_11AESEnable)

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg.bTGnWifiTest == TRUE)
#endif /* CONFIG_STA_SUPPORT */

#define CKIP_KP_ON(_p)				((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))

#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}

#ifdef USB_BULK_BUF_ALIGMENT
#define CUR_WRITE_IDX_INC(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}
#endif /* USB_BULK_BUF_ALIGMENT */

#ifdef DOT11_N_SUPPORT
/* StaActive.SupportedHtPhy.MCSSet is copied from AP beacon.  Don't need to update here. */
#define COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
	_pAd->StaActive.SupportedHtPhy.ChannelWidth = _pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;      \
	_pAd->StaActive.SupportedHtPhy.MimoPs = _pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs;      \
	_pAd->StaActive.SupportedHtPhy.GF = _pAd->MlmeAux.HtCapability.HtCapInfo.GF;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor20 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor40 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40;      \
	_pAd->StaActive.SupportedHtPhy.TxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.RxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.ExtChanOffset = _pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;      \
	_pAd->StaActive.SupportedHtPhy.RecomWidth = _pAd->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth;      \
	_pAd->StaActive.SupportedHtPhy.OperaionMode = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;      \
	_pAd->StaActive.SupportedHtPhy.NonGfPresent = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent;      \
	memmove((_pAd)->MacTab.Content[BSSID_WCID].HTCapability.MCSSet, (_pAd)->StaActive.SupportedPhyInfo.MCSSet, sizeof(UCHAR) * 16);\
}

#define COPY_AP_HTSETTINGS_FROM_BEACON(_pAd, _pHtCapability)                                 \
{                                                                                       \
	_pAd->MacTab.Content[BSSID_WCID].AMsduSize = (UCHAR)(_pHtCapability->HtCapInfo.AMsduSize);	\
	_pAd->MacTab.Content[BSSID_WCID].MmpsMode= (UCHAR)(_pHtCapability->HtCapInfo.MimoPs);	\
	_pAd->MacTab.Content[BSSID_WCID].MaxRAmpduFactor = (UCHAR)(_pHtCapability->HtCapParm.MaxRAmpduFactor);	\
}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
#define COPY_VHT_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
}
#endif /* DOT11_VHT_AC */


/*
	Common fragment list structure -  Identical to the scatter gather frag list structure
*/
#define NIC_MAX_PHYS_BUF_COUNT              8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
	PVOID Address;
	ULONG Length;
	PULONG Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;

typedef struct _RTMP_SCATTER_GATHER_LIST {
	ULONG NumberOfElements;
	PULONG Reserved;
	RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;


/*
	Some utility macros
*/
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define INC_COUNTER64(Val)          (Val.QuadPart++)

#define INFRA_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON))
#define ADHOC_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#ifdef CONFIG_STA_SUPPORT
#define MONITOR_ON(_p)              (((_p)->StaCfg.BssType) == BSS_MONITOR)
#else
#define MONITOR_ON(_p)              (((_p)->ApCfg.BssType) == BSS_MONITOR)
#endif

#define IDLE_ON(_p)                 (!INFRA_ON(_p) && !ADHOC_ON(_p))

/* Check LEAP & CCKM flags */
#define LEAP_ON(_p)                 (((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)            ((((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg.LeapAuthInfo.CCKM == TRUE))

/* if orginal Ethernet frame contains no LLC/SNAP, then an extra LLC/SNAP encap is required */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(_pBufVA, _pExtraLlcSnapEncap)		\
{																\
	if (((*(_pBufVA + 12) << 8) + *(_pBufVA + 13)) > 1500)		\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA + 12, 2) || 			\
			NdisEqualMemory(APPLE_TALK, _pBufVA + 12, 2))		\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}

/* New Define for new Tx Path. */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(_pBufVA, _pExtraLlcSnapEncap)	\
{																\
	if (((*(_pBufVA) << 8) + *(_pBufVA + 1)) > 1500)			\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA, 2) || 				\
			NdisEqualMemory(APPLE_TALK, _pBufVA, 2))			\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}

#define MAKE_802_3_HEADER(_buf, _pMac1, _pMac2, _pType)                   \
{                                                                       \
    memmove(_buf, _pMac1, MAC_ADDR_LEN);                           \
    memmove((_buf + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);          \
    memmove((_buf + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
}

/*
	if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel),
		keep it that way.
	else if the received frame is LLC/SNAP-encaped IPX or APPLETALK,
		preserve the LLC/SNAP field
	else remove the LLC/SNAP field from the result Ethernet frame

	Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
	Note:
		_pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
		_pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
*/
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)      \
{                                                                       \
    char LLC_Len[2];                                                    \
                                                                        \
    _pRemovedLLCSNAP = NULL;                                            \
    if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  ||                     \
        NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6))                 \
    {                                                                   \
        PUCHAR pProto = _pData + 6;                                     \
                                                                        \
        if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&  \
            NdisEqualMemory(SNAP_802_1H, _pData, 6))                    \
        {                                                               \
            LLC_Len[0] = (UCHAR)(_DataSize >> 8);                       \
            LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));                \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);          \
        }                                                               \
        else                                                            \
        {                                                               \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);           \
            _pRemovedLLCSNAP = _pData;                                  \
            _DataSize -= LENGTH_802_1_H;                                \
            _pData += LENGTH_802_1_H;                                   \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        LLC_Len[0] = (UCHAR)(_DataSize >> 8);                           \
        LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));                    \
        MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);              \
    }                                                                   \
}

/*
	Enqueue this frame to MLME engine
	We need to enqueue the whole frame because MLME need to pass data type
	information from 802.11 header
*/
#ifdef RTMP_MAC_USB
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _MinSNR, _OpMode)        \
{                                                                                       \
    uint32_t High32TSF=0, Low32TSF=0;                                                          \
    MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_MinSNR, _OpMode);   \
}
#endif /* RTMP_MAC_USB */

#define IPV4_ADDR_EQUAL(pAddr1, pAddr2)         RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 4)
#define IPV6_ADDR_EQUAL(pAddr1, pAddr2)         RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 16)
#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))


#ifdef CONFIG_STA_SUPPORT
#define STA_EXTRA_SETTING(_pAd)

#define STA_PORT_SECURED(_pAd) \
{ \
	BOOLEAN	Cancelled; \
	(_pAd)->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_SECURED; \
	RTMP_IndicateMediaState(_pAd, NdisMediaStateConnected); \
	NdisAcquireSpinLock(&((_pAd)->MacTabLock)); \
	(_pAd)->MacTab.Content[BSSID_WCID].PortSecured = (_pAd)->StaCfg.wdev.PortSecured; \
	(_pAd)->MacTab.Content[BSSID_WCID].PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
	NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
	RTMPCancelTimer(&((_pAd)->Mlme.LinkDownTimer), &Cancelled);\
	STA_EXTRA_SETTING(_pAd); \
}
#endif /* CONFIG_STA_SUPPORT */

/*
	Data buffer for DMA operation, the buffer must be contiguous physical memory
	Both DMA to / from CPU use the same structure.
*/
typedef struct _RTMP_DMABUF {
	ULONG AllocSize;
	PVOID AllocVa;		/* TxBuf virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* TxBuf physical address */
} RTMP_DMABUF, *PRTMP_DMABUF;


/*
	Control block (Descriptor) for all ring descriptor DMA operation, buffer must be
	contiguous physical memory. NDIS_PACKET stored the binding Rx packet descriptor
	which won't be released, driver has to wait until upper layer return the packet
	before giveing up this rx ring descriptor to ASIC. NDIS_BUFFER is assocaited pair
	to describe the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
	which driver should ACK upper layer when the tx is physically done or failed.
*/
typedef struct _RTMP_DMACB {
	ULONG AllocSize;	/* Control block size */
	PVOID AllocVa;		/* Control block virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* Control block physical address */
	struct sk_buff *pNdisPacket;
	struct sk_buff *pNextNdisPacket;

	RTMP_DMABUF DmaBuf;	/* Associated DMA buffer structure */
#ifdef CACHE_LINE_32B
	RXD_STRUC LastBDInfo;
#endif /* CACHE_LINE_32B */
} RTMP_DMACB, *PRTMP_DMACB;

typedef struct _RTMP_TX_RING {
	RTMP_DMACB Cell[TX_RING_SIZE];
	uint32_t TxCpuIdx;
	uint32_t TxDmaIdx;
	uint32_t TxSwFreeIdx;	/* software next free tx index */
	uint32_t hw_desc_base;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
} RTMP_TX_RING;

typedef struct _RTMP_RX_RING {
	RTMP_DMACB Cell[RX_RING_SIZE];
	uint32_t RxCpuIdx;
	uint32_t RxDmaIdx;
	int32_t RxSwReadIdx;	/* software next read index */
	uint32_t hw_desc_base;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
} RTMP_RX_RING;

typedef struct _RTMP_MGMT_RING {
	RTMP_DMACB Cell[MGMT_RING_SIZE];
	uint32_t TxCpuIdx;
	uint32_t TxDmaIdx;
	uint32_t TxSwFreeIdx;	/* software next free tx index */
	uint32_t hw_desc_base;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
} RTMP_MGMT_RING, *PRTMP_MGMT_RING;

typedef struct _RTMP_CTRL_RING {
	RTMP_DMACB Cell[MGMT_RING_SIZE];
	uint32_t TxCpuIdx;
	uint32_t TxDmaIdx;
	uint32_t TxSwFreeIdx;	/* software next free tx index */
	uint32_t hw_desc_base;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
} RTMP_CTRL_RING, *PRTMP_CTRL_RING;

/*
	Statistic counter structure
*/
typedef struct _COUNTER_802_3 {
	/* General Stats */
	ULONG GoodTransmits;
	ULONG GoodReceives;
	ULONG TxErrors;
	ULONG RxErrors;
	ULONG RxNoBuffer;
} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG Length;
/*	LARGE_INTEGER   LastTransmittedFragmentCount; */
	LARGE_INTEGER TransmittedFragmentCount;
	LARGE_INTEGER MulticastTransmittedFrameCount;
	LARGE_INTEGER FailedCount;
	LARGE_INTEGER RetryCount;
	LARGE_INTEGER MultipleRetryCount;
	LARGE_INTEGER RTSSuccessCount;
	LARGE_INTEGER RTSFailureCount;
	LARGE_INTEGER ACKFailureCount;
	LARGE_INTEGER FrameDuplicateCount;
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER MulticastReceivedFrameCount;
	LARGE_INTEGER FCSErrorCount;
	LARGE_INTEGER TransmittedFrameCount;
	LARGE_INTEGER WEPUndecryptableCount;
	LARGE_INTEGER TransmitCountFrmOs;
} COUNTER_802_11, *PCOUNTER_802_11;



typedef struct _COUNTER_RALINK {
	uint32_t OneSecStart;	/* for one sec count clear use */
	uint32_t OneSecBeaconSentCnt;
	uint32_t OneSecFalseCCACnt;	/* CCA error count, for debug purpose, might move to global counter */
	uint32_t OneSecRxFcsErrCnt;	/* CRC error */
	uint32_t OneSecRxOkCnt;	/* RX without error */
	uint32_t OneSecTxFailCount;
	uint32_t OneSecTxNoRetryOkCount;
	uint32_t OneSecTxRetryOkCount;
	uint32_t OneSecRxOkDataCnt;	/* unicast-to-me DATA frame count */
	uint32_t OneSecTransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */

	ULONG OneSecOsTxCount[NUM_OF_TX_RING];
	ULONG OneSecDmaDoneCount[NUM_OF_TX_RING];
	uint32_t OneSecTxDoneCount;
	ULONG OneSecRxCount;
	uint32_t OneSecReceivedByteCount;
	uint32_t OneSecTxAggregationCount;
	uint32_t OneSecRxAggregationCount;
	uint32_t OneSecEnd;	/* for one sec count clear use */

	ULONG TransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */
	ULONG ReceivedByteCount;	/* both CRC okay and CRC error, used to calculate RX throughput */
	ULONG BadCQIAutoRecoveryCount;
	ULONG PoorCQIRoamingCount;
	ULONG MgmtRingFullCount;
	ULONG RxCountSinceLastNULL;
	ULONG RxCount;
	ULONG KickTxCount;
	LARGE_INTEGER RealFcsErrCount;
	ULONG PendingNdisPacketCount;
	ULONG FalseCCACnt;                    /* CCA error count */

	uint32_t LastOneSecTotalTxCount;	/* OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount */
	uint32_t LastOneSecRxOkDataCnt;	/* OneSecRxOkDataCnt */
	ULONG DuplicateRcv;
	ULONG TxAggCount;
	ULONG TxNonAggCount;
	ULONG TxAgg1MPDUCount;
	ULONG TxAgg2MPDUCount;
	ULONG TxAgg3MPDUCount;
	ULONG TxAgg4MPDUCount;
	ULONG TxAgg5MPDUCount;
	ULONG TxAgg6MPDUCount;
	ULONG TxAgg7MPDUCount;
	ULONG TxAgg8MPDUCount;
	ULONG TxAgg9MPDUCount;
	ULONG TxAgg10MPDUCount;
	ULONG TxAgg11MPDUCount;
	ULONG TxAgg12MPDUCount;
	ULONG TxAgg13MPDUCount;
	ULONG TxAgg14MPDUCount;
	ULONG TxAgg15MPDUCount;
	ULONG TxAgg16MPDUCount;

	LARGE_INTEGER TransmittedOctetsInAMSDU;
	LARGE_INTEGER TransmittedAMSDUCount;
	LARGE_INTEGER ReceivedOctesInAMSDUCount;
	LARGE_INTEGER ReceivedAMSDUCount;
	LARGE_INTEGER TransmittedAMPDUCount;
	LARGE_INTEGER TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER TransmittedOctetsInAMPDUCount;
	LARGE_INTEGER MPDUInReceivedAMPDUCount;

	ULONG PhyErrCnt;
	ULONG PlcpErrCnt;
} COUNTER_RALINK, *PCOUNTER_RALINK;

typedef struct _COUNTER_DRS {
	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX+1];
	UCHAR PER[MAX_TX_RATE_INDEX+1];
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
	ULONG CurrTxRateStableTime;	/* # of second in current TX rate */
	/*BOOLEAN         fNoisyEnvironment; */
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	UCHAR LastTimeTxRateChangeAction;	/*Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;


#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
typedef struct _COUNTER_TXBF{
	ULONG TxSuccessCount;
	ULONG TxRetryCount;
	ULONG TxFailCount;
	ULONG ETxSuccessCount;
	ULONG ETxRetryCount;
	ULONG ETxFailCount;
	ULONG ITxSuccessCount;
	ULONG ITxRetryCount;
	ULONG ITxFailCount;
} COUNTER_TXBF;
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */


#ifdef STREAM_MODE_SUPPORT
typedef struct _STREAM_MODE_ENTRY_{
#define STREAM_MODE_STATIC		1
	USHORT flag;
	UCHAR macAddr[MAC_ADDR_LEN];
}STREAM_MODE_ENTRY;
#endif /* STREAM_MODE_SUPPORT */

/* for Microwave oven */
#ifdef MICROWAVE_OVEN_SUPPORT
typedef struct _MO_CFG_STRUCT {
	BOOLEAN		bEnable;
	UINT8  		nPeriod_Cnt; 	/* measurement period 100ms, mitigate the interference period 900 ms */
	uint16_t 		nFalseCCACnt;
	uint16_t 	nFalseCCATh;	/* default is 100 */
} MO_CFG_STRUCT, *PMO_CFG_STRUCT;
#endif /* MICROWAVE_OVEN_SUPPORT */

/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
/* for dynamic vga */
typedef struct _LNA_VGA_CTL_STRUCT {
	BOOLEAN		bEnable;
	BOOLEAN		bDyncVgaEnable;
	UINT8  		nPeriod_Cnt; 	/* measurement period 100ms, mitigate the interference period 900 ms */
	uint16_t 		nFalseCCACnt;
	uint16_t 	nFalseCCATh;	/* default is 100 */
	uint16_t 	nLowFalseCCATh;
		uint32_t 	agc1_r8_backup;
	UCHAR		agc_vga_init_0;
	UCHAR 		agc_vga_ori_0; /* the original vga gain initialized by firmware at start up */
	uint16_t 	agc_0_vga_set1_2;
		uint32_t 	agc1_r9_backup;
	UCHAR 		agc_vga_init_1;
	UCHAR 		agc_vga_ori_1; /* the original vga gain initialized by firmware at start up */
	uint16_t 	agc_1_vga_set1_2;
} LNA_VGA_CTL_STRUCT, *PLNA_VGA_CTL_STRUCT;
#endif /* DYNAMIC_VGA_SUPPORT */

/***************************************************************************
  *	security key related data structure
  **************************************************************************/

/* structure to define WPA Group Key Rekey Interval */
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;	/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG ReKeyInterval;	/* time-based: seconds, packet-based: kilo-packets */
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;


#ifdef RTMP_MAC_USB
/***************************************************************************
  *	RTUSB I/O related data structure
  **************************************************************************/

/* for USB interface, avoid in interrupt when write key */
typedef struct RT_ADD_PAIRWISE_KEY_ENTRY {
	UCHAR MacAddr[6];
	USHORT MacTabMatchWCID;	/* ASIC */
	CIPHER_KEY CipherKey;
} RT_ADD_PAIRWISE_KEY_ENTRY,*PRT_ADD_PAIRWISE_KEY_ENTRY;


/* Cipher suite type for mixed mode group cipher, P802.11i-2004 */
typedef enum _RT_802_11_CIPHER_SUITE_TYPE {
	Cipher_Type_NONE,
	Cipher_Type_WEP40,
	Cipher_Type_TKIP,
	Cipher_Type_RSVD,
	Cipher_Type_CCMP,
	Cipher_Type_WEP104
} RT_802_11_CIPHER_SUITE_TYPE, *PRT_802_11_CIPHER_SUITE_TYPE;
#endif /* RTMP_MAC_USB */

typedef struct {
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR ErrorCode[2];	/*00 01-Invalid authentication type */
	/*00 02-Authentication timeout */
	/*00 03-Challenge from AP failed */
	/*00 04-Challenge to AP failed */
	BOOLEAN Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR RogueApNr;
	ROGUEAP_ENTRY RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

/*
  *	Fragment Frame structure
  */
typedef struct _FRAGMENT_FRAME {
	struct sk_buff *pFragPacket;
	ULONG RxSize;
	USHORT Sequence;
	USHORT LastFrag;
	ULONG Flags;		/* Some extra frame information. bit 0: LLC presented */
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;


/*
	Tkip Key structure which RC4 key & MIC calculation
*/
typedef struct _TKIP_KEY_INFO {
	UINT nBytesInM;		/* # bytes in M for MICKEY */
	ULONG IV16;
	ULONG IV32;
	ULONG K0;		/* for MICKEY Low */
	ULONG K1;		/* for MICKEY Hig */
	ULONG L;		/* Current state for MICKEY */
	ULONG R;		/* Current state for MICKEY */
	ULONG M;		/* Message accumulator for MICKEY */
	UCHAR RC4KEY[16];
	UCHAR MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;


/*
	Private / Misc data, counters for driver internal use
*/
typedef struct __PRIVATE_STRUC {
	UINT SystemResetCnt;	/* System reset counter */
	/* Tx ring full occurrance number */
	UINT TxRingFullCnt;
	UINT PhyRxErrCnt;	/* PHY Rx error count, for debug purpose, might move to global counter */
	/* Variables for WEP encryption / decryption in rtmp_wep.c */
	/* Tkip stuff */
	TKIP_KEY_INFO Tx;
	TKIP_KEY_INFO Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;


/***************************************************************************
  *	Channel and BBP related data structures
  **************************************************************************/
/* structure to tune BBP R66 (BBP TUNING) */
typedef struct _BBP_R66_TUNING {
	BOOLEAN bEnable;
	USHORT FalseCcaLowerThreshold;	/* default 100 */
	USHORT FalseCcaUpperThreshold;	/* default 512 */
	UCHAR R66Delta;
	UCHAR R66CurrentValue;
	BOOLEAN R66LowerUpperSelect;	/*Before LinkUp, Used LowerBound or UpperBound as R66 value. */
} BBP_R66_TUNING, *PBBP_R66_TUNING;


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#define EFFECTED_CH_SECONDARY 0x1
#define EFFECTED_CH_PRIMARY	0x2
#define EFFECTED_CH_LEGACY		0x4
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/* structure to store channel TX power */
typedef struct _CHANNEL_TX_POWER {
	USHORT RemainingTimeForUse;	/*unit: sec */
	UCHAR Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN bEffectedChannel;	/* For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz. */
#endif /* DOT11N_DRAFT3 */
	CHAR Power;
	CHAR Power2;
#ifdef DOT11N_SS3_SUPPORT
	CHAR Power3;
#endif /* DOT11N_SS3_SUPPORT */
	UCHAR MaxTxPwr;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;

/*
	Channel property:

	CHANNEL_DISABLED: The channel is disabled.
	CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
	CHANNEL_NO_IBSS: IBSS is not allowed.
	CHANNEL_RADAR: Radar detection is required.
	CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
	CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
	CHANNEL_40M_CAP: 40 BW channel group
	CHANNEL_80M_CAP: 800 BW channel group
 */
#define CHANNEL_DEFAULT_PROP	0x00
#define CHANNEL_DISABLED		0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS			0x04
#define CHANNEL_RADAR			0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20
#define CHANNEL_40M_CAP			0x40
#define CHANNEL_80M_CAP			0x80

	UCHAR Flags;

} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

/* Channel list subset */
typedef struct _CHANNEL_LIST_SUB {
	UCHAR	Channel;
	UCHAR	IdxMap; /* Index mapping to original channel list */
} CHANNEL_LIST_SUB, *PCHANNEL_LIST_SUB;


typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR EvaluatePeriod;	/* 0:not evalute status, 1: evaluate status, 2: switching status */
	UCHAR EvaluateStableCnt;
	UCHAR Pair1PrimaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
	UCHAR Pair1SecondaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
#ifdef CONFIG_STA_SUPPORT
	SHORT Pair1AvgRssi[2];	/* AvgRssi[0]:E1, AvgRssi[1]:E2 */
	SHORT Pair2AvgRssi[2];	/* AvgRssi[0]:E3, AvgRssi[1]:E4 */
#endif /* CONFIG_STA_SUPPORT */
	SHORT Pair1LastAvgRssi;	/* */
	SHORT Pair2LastAvgRssi;	/* */
	ULONG RcvPktNumWhenEvaluate;
	BOOLEAN FirstPktArrivedWhenEvaluate;
#ifdef CONFIG_AP_SUPPORT
	LONG Pair1AvgRssiGroup1[2];
	LONG Pair1AvgRssiGroup2[2];
	ULONG RcvPktNum[2];
#endif /* CONFIG_AP_SUPPORT */
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
/***************************************************************************
  *	structure for MLME state machine
  **************************************************************************/
typedef struct _MLME_STRUCT {
#ifdef CONFIG_STA_SUPPORT
	/* STA state machines */
	STATE_MACHINE CntlMachine;
	STATE_MACHINE AssocMachine;
	STATE_MACHINE AuthMachine;
	STATE_MACHINE AuthRspMachine;
	STATE_MACHINE SyncMachine;
	STATE_MACHINE WpaPskMachine;
	STATE_MACHINE LeapMachine;
	STATE_MACHINE_FUNC AssocFunc[ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC AuthFunc[AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC AuthRspFunc[AUTH_RSP_FUNC_SIZE];
	STATE_MACHINE_FUNC SyncFunc[SYNC_FUNC_SIZE];

#endif /* CONFIG_STA_SUPPORT */
	STATE_MACHINE_FUNC ActFunc[ACT_FUNC_SIZE];
	/* Action */
	STATE_MACHINE ActMachine;







#ifdef CONFIG_AP_SUPPORT
	/* AP state machines */
	STATE_MACHINE ApAssocMachine;
	STATE_MACHINE ApAuthMachine;
	STATE_MACHINE ApSyncMachine;
	STATE_MACHINE_FUNC ApAssocFunc[AP_ASSOC_FUNC_SIZE];
/*	STATE_MACHINE_FUNC		ApDlsFunc[DLS_FUNC_SIZE]; */
	STATE_MACHINE_FUNC ApAuthFunc[AP_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC ApSyncFunc[AP_SYNC_FUNC_SIZE];
#ifdef APCLI_SUPPORT
	STATE_MACHINE ApCliAuthMachine;
	STATE_MACHINE ApCliAssocMachine;
	STATE_MACHINE ApCliCtrlMachine;
	STATE_MACHINE ApCliSyncMachine;
	STATE_MACHINE ApCliWpaPskMachine;

	STATE_MACHINE_FUNC ApCliAuthFunc[APCLI_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliAssocFunc[APCLI_ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliCtrlFunc[APCLI_CTRL_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliSyncFunc[APCLI_SYNC_FUNC_SIZE];
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* common WPA state machine */
	STATE_MACHINE WpaMachine;
	STATE_MACHINE_FUNC WpaFunc[WPA_FUNC_SIZE];


	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
	ULONG Now32;		/* latch the value of NdisGetSystemUpTime() */
	ULONG LastSendNULLpsmTime;

	BOOLEAN bRunning;
	NDIS_SPIN_LOCK TaskLock;
	MLME_QUEUE Queue;

	UINT ShiftReg;

	RALINK_TIMER_STRUCT PeriodicTimer;
	RALINK_TIMER_STRUCT APSDPeriodicTimer;
	RALINK_TIMER_STRUCT LinkDownTimer;
	RALINK_TIMER_STRUCT LinkUpTimer;
	ULONG PeriodicRound;
	ULONG GPIORound;
	ULONG OneSecPeriodicRound;

	UCHAR RealRxPath;
	BOOLEAN bLowThroughput;
	BOOLEAN bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT RxAntEvalTimer;


#ifdef RTMP_MAC_USB
	RALINK_TIMER_STRUCT AutoWakeupTimer;
	BOOLEAN AutoWakeupTimerRunning;
#endif /* RTMP_MAC_USB */

} MLME_STRUCT, *PMLME_STRUCT;

#ifdef DOT11_N_SUPPORT
/***************************************************************************
  *	802.11 N related data structures
  **************************************************************************/
struct reordering_mpdu {
	struct reordering_mpdu *next;
	struct sk_buff *pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR					OpMode;
};

struct reordering_list {
	struct reordering_mpdu *next;
	int qlen;
};

struct reordering_mpdu_pool {
	PVOID mem;
	NDIS_SPIN_LOCK lock;
	struct reordering_list freelist;
};

typedef enum _REC_BLOCKACK_STATUS {
	Recipient_NONE = 0,
	Recipient_USED,
	Recipient_HandleRes,
	Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS {
	Originator_NONE = 0,
	Originator_USED,
	Originator_WaitRes,
	Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;
	UCHAR Token;
	UCHAR amsdu_cap;
/* Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header. */
	USHORT Sequence;
	USHORT TimeOutValue;
	ORI_BLOCKACK_STATUS ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct _BA_REC_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;	/* 7.3.1.14. each buffer is capable of holding a max AMSDU or MSDU. */
	/*UCHAR NumOfRxPkt; */
	/*UCHAR    Curindidx; // the head in the RX reordering buffer */
	USHORT LastIndSeq;
/*	USHORT		LastIndSeqAtTimer; */
	USHORT TimeOutValue;
	RALINK_TIMER_STRUCT RECBATimer;
	ULONG LastIndSeqAtTimer;
	ULONG nDropPacket;
	ULONG rcvSeq;
	REC_BLOCKACK_STATUS REC_BA_Status;
/*	UCHAR	RxBufIdxUsed; */
	/* corresponding virtual address for RX reordering packet storage. */
	/*RTMP_REORDERDMABUF MAP_RXBuf[MAX_RX_REORDERBUF]; */
	NDIS_SPIN_LOCK RxReRingLock;	/* Rx Ring spinlock */
/*	struct _BA_REC_ENTRY *pNext; */
	PVOID pAdapter;
	struct reordering_list list;
} BA_REC_ENTRY, *PBA_REC_ENTRY;


typedef struct {
	ULONG numAsRecipient;	/* I am recipient of numAsRecipient clients. These client are in the BARecEntry[] */
	ULONG numAsOriginator;	/* I am originator of   numAsOriginator clients. These clients are in the BAOriEntry[] */
	ULONG numDoneOriginator;	/* count Done Originator sessions */
	BA_ORI_ENTRY BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	BA_REC_ENTRY BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
} BA_TABLE, *PBA_TABLE;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_REC_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize */
	UCHAR rsv;
	UCHAR BufSize[8];
	REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_ORI_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status */
	UCHAR rsv;
	UCHAR BufSize[8];
	ORI_BLOCKACK_STATUS ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE {
	OID_BA_ORI_ENTRY BAOriEntry[32];
	OID_BA_REC_ENTRY BARecEntry[32];
	UCHAR OriNum;		/* Number of below BAOriEntry */
	UCHAR RecNum;		/* Number of below BARecEntry */
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef union _BACAP_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		uint32_t:4;
		uint32_t b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		uint32_t bHtAdhoc:1;	/* adhoc can use ht rate. */
		uint32_t MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		uint32_t AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		uint32_t AmsduEnable:1;	/*Enable AMSDU transmisstion */
		uint32_t MpduDensity:3;
		uint32_t Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		uint32_t AutoBA:1;	/* automatically BA */
		uint32_t TxBAWinLimit:8;
		uint32_t RxBAWinLimit:8;
	} field;
#else
	struct {
		uint32_t RxBAWinLimit:8;
		uint32_t TxBAWinLimit:8;
		uint32_t AutoBA:1;	/* automatically BA */
		uint32_t Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		uint32_t MpduDensity:3;
		uint32_t AmsduEnable:1;	/*Enable AMSDU transmisstion */
		uint32_t AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		uint32_t MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		uint32_t bHtAdhoc:1;	/* adhoc can use ht rate. */
		uint32_t b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		uint32_t:4;
	} field;
#endif
	uint32_t word;
} BACAP_STRUC, *PBACAP_STRUC;

typedef struct {
	BOOLEAN IsRecipient;
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR TID;
	UCHAR nMSDU;
	USHORT TimeOut;
	BOOLEAN bAllTid;	/* If True, delete all TID for BA sessions with this MACaddr. */
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG {
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
} BSS2040COEXIST_FLAG;

typedef struct _BssCoexChRange_ {
	UCHAR primaryCh;
	UCHAR secondaryCh;
	UCHAR effectChStart;
	UCHAR effectChEnd;
} BSS_COEX_CH_RANGE;
#endif /* DOT11N_DRAFT3 */

#define IS_VHT_STA(_pMacEntry)	(_pMacEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#ifdef DOT11_VHT_AC
#define IS_VHT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE == MODE_VHT)
#endif
#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#endif /* DOT11_N_SUPPORT */

/*This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic) */
typedef struct _IOT_STRUC {
	BOOLEAN bRTSLongProtOn;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bLastAtheros;
	//BOOLEAN bCurrentAtheros;
	//BOOLEAN bNowAtherosBurstOn;
	//BOOLEAN bNextDisableRxBA;
	BOOLEAN bToggle;
#endif /* CONFIG_STA_SUPPORT */
} IOT_STRUC;

/* This is the registry setting for 802.11n transmit setting.  Used in advanced page. */
typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		uint32_t rsv:13;
		uint32_t EXTCHA:2;
		uint32_t HTMODE:1;
		uint32_t TRANSNO:2;
		uint32_t STBC:1;	/*SPACE */
		uint32_t ShortGI:1;
		uint32_t BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		uint32_t TxBF:1;	/* 3*3 */
		uint32_t ITxBfEn:1;
		uint32_t rsv0:9;
		/*uint32_t  MCS:7;                 // MCS */
		/*uint32_t  PhyMode:4; */
	} field;
#else
	struct {
		/*uint32_t  PhyMode:4; */
		/*uint32_t  MCS:7;                 // MCS */
		uint32_t rsv0:9;
		uint32_t ITxBfEn:1;
		uint32_t TxBF:1;
		uint32_t BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		uint32_t ShortGI:1;
		uint32_t STBC:1;	/*SPACE */
		uint32_t TRANSNO:2;
		uint32_t HTMODE:1;
		uint32_t EXTCHA:2;
		uint32_t rsv:13;
	} field;
#endif
	uint32_t word;
} REG_TRANSMIT_SETTING;


typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT rsv:2;
		USHORT FixedTxMode:3;	/* If MCS isn't AUTO, fix rate in CCK, OFDM, HT or VHT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;
		USHORT PhyMode:4;
		USHORT FixedTxMode:3;
		USHORT rsv:2;
	} field;
#endif
	USHORT word;
 } DESIRED_TRANSMIT_SETTING;


struct hw_setting{
	UCHAR prim_ch;
	UCHAR cent_ch;
	UINT8 bbp_bw;
	UCHAR rf_band;
	UCHAR cur_ch_pwr[3];
	CHAR lan_gain;
};


#define WDEV_TYPE_AP		0x01
#define WDEV_TYPE_STA		0x02
#define WDEV_TYPE_ADHOC	0x04
#define WDEV_TYPE_WDS		0x08
#define WDEV_TYPE_MESH	0x10

#define WDEV_NUM_MAX		16
struct rtmp_wifi_dev{
	struct net_device *if_dev;
	VOID *func_dev;
	struct rtmp_adapter *sys_handle;

	CHAR wdev_idx;	/* index refer from pAd->wdev_list[] */
	CHAR func_idx; /* index refer to func_dev which pointer to */
	UCHAR tr_tb_idx; /* index refer to BSS which this device belong */
	UCHAR wdev_type;
	UCHAR PhyMode;
	UCHAR channel;
	UCHAR if_addr[MAC_ADDR_LEN];
	UCHAR bssid[MAC_ADDR_LEN];

	/* security segment */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER WpaMixPairCipher;
	UCHAR DefaultKeyId;
	UCHAR PortSecured;
#if defined(DOT1X_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
	BOOLEAN IEEE8021X; /* Only indicate if we are running in dynamic WEP mode (WEP+802.1x) */
#endif /* DOT1X_SUPPORT */
	BOOLEAN bWpaAutoMode;

	/* transmit segment */
	BOOLEAN allow_data_tx;
	BOOLEAN IgmpSnoopEnable; /* Only enabled for AP/WDS mode */
	RT_PHY_INFO DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. this is for reading registry setting only. not useful. */
	BOOLEAN bAutoTxRateSwitch;
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */

	/* 802.11 protocol related characters */
	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;


	/* VLAN related */
	BOOLEAN bVLAN_Tag;
	USHORT VLAN_VID;
	USHORT VLAN_Priority;

	/* operations */
	INT (*tx_pkt_allowed)(struct rtmp_adapter *pAd, struct rtmp_wifi_dev *wdev, struct sk_buff *pPacket, UCHAR *pWcid);
	INT (*tx_pkt_handle)(struct rtmp_adapter *pAd, struct sk_buff *pPacket);
	INT (*rx_pkt_allowed)(struct rtmp_adapter *pAd, struct _RX_BLK *pRxBlk);
	INT (*rx_pkt_hdr_chk)(struct rtmp_adapter *pAd, struct _RX_BLK *pRxBlk);
	INT (*rx_ps_handle)(struct rtmp_adapter *pAd, struct _RX_BLK *pRxBlk);
	INT (*rx_pkt_foward)(struct rtmp_adapter *pAd, struct rtmp_wifi_dev *wdev, struct sk_buff *pPacket);

	/* last received packet's SNR for each antenna */
	UCHAR LastSNR0;
	UCHAR LastSNR1;
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;
	int32_t BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
#endif /* DOT11N_SS3_SUPPORT */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;
#if defined(RT_CFG80211_SUPPORT) || defined(HOSTAPD_SUPPORT)
	NDIS_HOSTAPD_STATUS Hostapd;
#endif
};


#ifdef RTMP_MAC_USB
/***************************************************************************
  *	USB-based chip Beacon related data structures
  **************************************************************************/
#define BEACON_BITMAP_MASK		0xff
typedef struct _BEACON_SYNC_STRUCT_ {
	UCHAR BeaconBuf[HW_BEACON_MAX_NUM][HW_BEACON_OFFSET];
	UCHAR *BeaconTxWI[HW_BEACON_MAX_NUM];
	ULONG TimIELocationInBeacon[HW_BEACON_MAX_NUM];
	ULONG CapabilityInfoLocationInBeacon[HW_BEACON_MAX_NUM];
	BOOLEAN EnableBeacon;	/* trigger to enable beacon transmission. */
	UCHAR BeaconBitMap;	/* NOTE: If the MAX_MBSSID_NUM is larger than 8, this parameter need to change. */
	UCHAR DtimBitOn;	/* NOTE: If the MAX_MBSSID_NUM is larger than 8, this parameter need to change. */
} BEACON_SYNC_STRUCT;
#endif /* RTMP_MAC_USB */

/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_MAX_NUM_OF_TIM			((MAX_LEN_OF_MAC_TABLE >> 3) + 1)	/* /8 + 1 */
#define WLAN_CT_TIM_BCMC_OFFSET		0	/* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~NUM_BIT8[0];

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= NUM_BIT8[0];

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] &= (~NUM_BIT8[bit_offset]); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] |= NUM_BIT8[bit_offset]; }


#ifdef CONFIG_AP_SUPPORT
typedef struct _MULTISSID_STRUCT {
	struct rtmp_wifi_dev wdev;

	INT mbss_idx;

#ifdef HOSTAPD_SUPPORT
	NDIS_HOSTAPD_STATUS Hostapd;
	BOOLEAN HostapdWPS;
#endif

	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	BOOLEAN bHideSsid;

	USHORT CapabilityInfo;

	UCHAR MaxStaNum;	/* Limit the STA connection number per BSS */
	UCHAR StaCount;
	uint16_t StationKeepAliveTime;	/* unit: second */

	struct net_device *MSSIDDev;
	/*
		Security segment
	*/
	UCHAR RSNIE_Len[2];
	UCHAR RSN_IE[2][MAX_LEN_OF_RSNIE];

	/* WPA */
	UCHAR WPAKeyString[65];
	UCHAR GMK[32];
	UCHAR PMK[32];
	UCHAR GTK[32];
	UCHAR GNonce[32];
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;

	/* for Group Rekey, AP ONLY */
	RT_WPA_REKEY WPAREKEY;
	ULONG REKEYCOUNTER;
	RALINK_TIMER_STRUCT REKEYTimer;
	UCHAR REKEYTimerRunning;
	UINT8 RekeyCountDown;

	/* For PMK Cache using, AP ONLY */
	ULONG PMKCachePeriod;	/* unit : jiffies */
	NDIS_AP_802_11_PMKID PMKIDCache;

#ifdef DOT1X_SUPPORT
	BOOLEAN PreAuth;

	/* For 802.1x daemon setting per BSS */
	UINT8 radius_srv_num;
	RADIUS_SRV_INFO radius_srv_info[MAX_RADIUS_SRV_NUM];
	UINT8 NasId[IFNAMSIZ];
	UINT8 NasIdLen;
#endif /* DOT1X_SUPPORT */



	/*
		Transmitting segment
	*/
	UCHAR TxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11, ... */
	UCHAR DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR DesiredRatesIndex;
	UCHAR MaxTxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11 */


	/*
		Statistics segment
	*/
	/*MBSS_STATISTICS MbssStat;*/
	ULONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;

	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;

	ULONG StatTxRetryOkCount;
	ULONG StatTxFailCount;

	UCHAR BANClass3Data;
	ULONG IsolateInterStaTraffic;
	UCHAR IsolateInterStaMBCast;

	/* outgoing BEACON frame buffer and corresponding TXWI */
	BOOLEAN bBcnSntReq;	/* used in if beacon send or stop */
	UCHAR BcnBufIdx;
	CHAR BeaconBuf[MAX_BEACON_SIZE];	/* NOTE: BeaconBuf should be 4-byte aligned */
	UCHAR TimBitmaps[WLAN_MAX_NUM_OF_TIM];
	UCHAR TimIELocationInBeacon;
	UCHAR CapabilityInfoLocationInBeacon;


	RT_802_11_ACL AccessControlList;

	/* EDCA Qos */
	/*BOOLEAN bWmmCapable;*/	/* 0:disable WMM, 1:enable WMM */
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */

	/*
	   Why need the parameter: 2009/09/22

	   1. iwpriv ra0 set WmmCapable=0
	   2. iwpriv ra0 set WirelessMode=9
	   3. iwpriv ra0 set WirelessMode=0
	   4. iwpriv ra0 set SSID=SampleAP

	   After the 4 commands, WMM is still enabled.
	   So we need the parameter to recover WMM Capable flag.

	   No the problem in station mode.
	 */
	BOOLEAN bWmmCapableOrg;	/* origin Wmm Capable in non-11n mode */

	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;


	/*
`		WPS segment
	*/
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;





#ifdef DOT11W_PMF_SUPPORT
	PMF_CFG PmfCfg;
#endif /* DOT11W_PMF_SUPPORT */


	/* YF@20120417: Avoid connecting to AP in Poor Env, value 0 fOr disable. */
	CHAR AssocReqFailRssiThreshold;
	CHAR AssocReqNoRspRssiThreshold;
	CHAR AuthFailRssiThreshold;
	CHAR AuthNoRspRssiThreshold;
	CHAR RssiLowForStaKickOut;
	CHAR ProbeRspRssiThreshold;

	CHAR FilterUnusedPacket;



#ifdef SPECIFIC_TX_POWER_SUPPORT
	CHAR TxPwrAdj;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

	CHAR ProbeRspTimes;
} MULTISSID_STRUCT, *PMULTISSID_STRUCT;

#define FILTER_NONE 	0x00
#define FILTER_IPV6_MC	0x01
#define FILTER_IPV4_MC	0x02
#define FILTER_IPV6_ALL 0x04
#define FILTER_TOTAL	0x08
#endif /* CONFIG_AP_SUPPORT */
/* configuration common to OPMODE_AP as well as OPMODE_STA */
typedef struct _COMMON_CONFIG {
	BOOLEAN bCountryFlag;
	UCHAR CountryCode[3];
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR Geography;
	UCHAR DfsType;
	PUCHAR pChDesp;
#endif /* EXT_BUILD_CHANNEL_LIST */
	UCHAR CountryRegion;	/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
	UCHAR CountryRegionForABand;	/* Enum of country region for A band */
	UCHAR PhyMode;
	UCHAR cfg_wmode;
	UCHAR SavedPhyMode;
	USHORT Dsifs;		/* in units of usec */
	ULONG PacketFilter;	/* Packet filter for receiving */
	UINT8 RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];

	CHAR Ssid[MAX_LEN_OF_SSID];	/* NOT NULL-terminated */
	UCHAR SsidLen;		/* the actual ssid length in used */
	UCHAR LastSsidLen;	/* the actual ssid length in used */
	CHAR LastSsid[MAX_LEN_OF_SSID];	/* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];

	UCHAR Bssid[MAC_ADDR_LEN];
	USHORT BeaconPeriod;
	UCHAR Channel;
	UCHAR CentralChannel;	/* Central Channel when using 40MHz is indicating. not real channel. */

	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR MaxDesiredRate;
	UCHAR ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	ULONG BasicRateBitmap;	/* backup basic ratebitmap */
	ULONG BasicRateBitmapOld;	/* backup basic ratebitmap */

	BOOLEAN bInServicePeriod;


	BOOLEAN bAPSDAC_BE;
	BOOLEAN bAPSDAC_BK;
	BOOLEAN bAPSDAC_VI;
	BOOLEAN bAPSDAC_VO;


	/* because TSPEC can modify the APSD flag, we need to keep the APSD flag
	   requested in association stage from the station;
	   we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN bACMAPSDBackup[4];	/* for delivery-enabled & trigger-enabled both */
	BOOLEAN bACMAPSDTr[4];	/* no use */
	UCHAR MaxSPLength;

	BOOLEAN bNeedSendTriggerFrame;
	BOOLEAN bAPSDForcePowerSave;	/* Force power save mode, should only use in APSD-STAUT */
	ULONG TriggerTimerCount;
	UCHAR BBPCurrentBW;	/* BW_10, BW_20, BW_40, BW_80 */
	REG_TRANSMIT_SETTING RegTransmitSetting;	/*registry transmit setting. this is for reading registry setting only. not useful. */
	UCHAR TxRate;		/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR MaxTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR TxRateIndex;	/* Tx rate index in Rate Switch Table */
	UCHAR MinTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR RtsRate;		/* RATE_xxx */
	HTTRANSMIT_SETTING MlmeTransmit;	/* MGMT frame PHY rate setting when operatin at Ht rate. */
	UCHAR MlmeRate;		/* RATE_xxx, used to send MLME frames */
	UCHAR BasicMlmeRate;	/* Default Rate for sending MLME frames */

	USHORT RtsThreshold;	/* in unit of BYTE */
	USHORT FragmentThreshold;	/* in unit of BYTE */

	UCHAR TxPower;		/* in unit of mW */
	ULONG TxPowerPercentage;	/* 0~100 % */
	ULONG TxPowerDefault;	/* keep for TxPowerPercentage */
	UINT8 PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC BACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	BACAP_STRUC REGBACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	BOOLEAN force_vht;
	UCHAR vht_bw;
	UCHAR vht_sgi_80;
	UCHAR vht_stbc;
	UCHAR vht_bw_signal;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN vht_ldpc;
#endif /* DOT11_VHT_AC */

	IOT_STRUC IOTestParm;	/* 802.11n InterOpbility Test Parameter; */
	ULONG TxPreamble;	/* Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto */
	BOOLEAN bUseZeroToDisableFragment;	/* Microsoft use 0 as disable */
	ULONG UseBGProtection;	/* 0: auto, 1: always use, 2: always not use */
	BOOLEAN bUseShortSlotTime;	/* 0: disable, 1 - use short slot (9us) */
	BOOLEAN bEnableTxBurst;	/* 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST */
	BOOLEAN bAggregationCapable;	/* 1: enable TX aggregation when the peer supports it */
	BOOLEAN bPiggyBackCapable;	/* 1: enable TX piggy-back according MAC's version */
	BOOLEAN bIEEE80211H;	/* 1: enable IEEE802.11h spec. */
	UCHAR RDDurRegion; /* Region of radar detection */
	ULONG DisableOLBCDetect;	/* 0: enable OLBC detect; 1 disable OLBC detect */

#ifdef DOT11_N_SUPPORT
	BOOLEAN bRdg;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	BOOLEAN b256QAM_2G;
#endif /* DOT11_VHT_AC */

	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	QOS_CAPABILITY_PARM APQosCapability;	/* QOS capability of the current associated AP */
	EDCA_PARM APEdcaParm;	/* EDCA parameters of the current associated AP */
#ifdef MULTI_CLIENT_SUPPORT
	BOOLEAN bWmm;		/* have BSS enable/disable WMM */
	UCHAR APCwmin;		/* record ap cwmin */
	UCHAR APCwmax;	/* record ap cwmax */
	UCHAR BSSCwmin;	/* record BSS cwmin */
	uint32_t txRetryCfg;
#endif /* MULTI_CLIENT_SUPPORT */
	QBSS_LOAD_PARM APQbssLoad;	/* QBSS load of the current associated AP */
	UCHAR AckPolicy[4];	/* ACK policy of the specified AC. see ACK_xxx */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */
#endif /* CONFIG_STA_SUPPORT */
	/* a bitmap of BOOLEAN flags. each bit represent an operation status of a particular */
	/* BOOLEAN control, either ON or OFF. These flags should always be accessed via */
	/* OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros. */
	/* see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition */
	ULONG OpStatusFlags;

	BOOLEAN NdisRadioStateOff;	/*For HCT 12.0, set this flag to TRUE instead of called MlmeRadioOff. */
	ABGBAND_STATE BandState;        /* For setting BBP used on B/G or A mode. */


#ifdef DFS_SUPPORT
	/* IEEE802.11H--DFS. */
	RADAR_DETECT_STRUCT RadarDetect;
#endif /* DFS_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* HT */
	RT_HT_CAPABILITY DesiredHtPhy;
	HT_CAPABILITY_IE HtCapability;
	ADD_HT_INFO_IE AddHTInfo;	/* Useful as AP. */
	/*This IE is used with channel switch announcement element when changing to a new 40MHz. */
	/*This IE is included in channel switch ammouncement frames 7.4.1.5, beacons, probe Rsp. */
	NEW_EXT_CHAN_IE NewExtChanOffset;	/*7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present */

	EXT_CAP_INFO_ELEMENT ExtCapIE;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */

#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable;
	/*
	   Following two paramters now only used for the initial scan operation. the AP only do
	   bandwidth fallback when BssCoexApCnt > BssCoexApCntThr
	   By default, the "BssCoexApCntThr" is set as 0 in "UserCfgInit()".
	 */
	UCHAR BssCoexApCntThr;
	UCHAR BssCoexApCnt;

	UCHAR Bss2040CoexistFlag;	/* bit 0: bBssCoexistTimerRunning, bit 1: NeedSyncAddHtInfo. */
	RALINK_TIMER_STRUCT Bss2040CoexistTimer;
	UCHAR Bss2040NeedFallBack; 	/* 1: Need Fall back to 20MHz */

	/*This IE is used for 20/40 BSS Coexistence. */
	BSS_2040_COEXIST_IE BSS2040CoexistInfo;

	USHORT Dot11OBssScanPassiveDwell;	/* Unit : TU. 5~1000 */
	USHORT Dot11OBssScanActiveDwell;	/* Unit : TU. 10~1000 */
	USHORT Dot11BssWidthTriggerScanInt;	/* Unit : Second */
	USHORT Dot11OBssScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
	USHORT Dot11OBssScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
	USHORT Dot11BssWidthChanTranDelayFactor;
	USHORT Dot11OBssScanActivityThre;	/* Unit : percentage */

	ULONG Dot11BssWidthChanTranDelay;	/* multiple of (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */
	ULONG CountDownCtr;	/* CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */

	BSS_2040_COEXIST_IE LastBSSCoexist2040;
	BSS_2040_COEXIST_IE BSSCoexist2040;
	TRIGGER_EVENT_TAB TriggerEventTab;
	UCHAR ChannelListIdx;

	BOOLEAN bOverlapScanning;
	BOOLEAN bBssCoexNotify;
#endif /* DOT11N_DRAFT3 */

	BOOLEAN bHTProtect;
	BOOLEAN bMIMOPSEnable;
	BOOLEAN bBADecline;
	BOOLEAN bDisableReordering;
	BOOLEAN bForty_Mhz_Intolerant;
	BOOLEAN bExtChannelSwitchAnnouncement;
	BOOLEAN bRcvBSSWidthTriggerEvents;
	ULONG LastRcvBSSWidthTriggerEventsTime;

	UCHAR TxBASize;

	BOOLEAN bRalinkBurstMode;
	uint32_t RestoreBurstMode;
	BOOLEAN ht_ldpc;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	uint32_t cfg_vht;
	VHT_CAP_INFO vht_info;
	VHT_CAP_IE vht_cap_ie;
	BOOLEAN bNonVhtDisallow; /* Disallow non-VHT connection */
#endif /* DOT11_VHT_AC */

#ifdef SYSTEM_LOG_SUPPORT
	/* Enable wireless event */
	BOOLEAN bWirelessEvent;
#endif /* SYSTEM_LOG_SUPPORT */

	BOOLEAN bWiFiTest;	/* Enable this parameter for WiFi test */

	/* Tx & Rx Stream number selection */
	UCHAR TxStream;
	UCHAR RxStream;

	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
	UCHAR McastTransmitMcs;
	UCHAR McastTransmitPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */

#ifdef RTMP_MAC_USB
	BOOLEAN bMultipleIRP;	/* Multiple Bulk IN flag */
	UCHAR NumOfBulkInIRP;	/* if bMultipleIRP == TRUE, NumOfBulkInIRP will be 4 otherwise be 1 */
	RT_HT_CAPABILITY SupportedHtPhy;
	ULONG MaxPktOneTxBulk;
	UCHAR TxBulkFactor;
	UCHAR RxBulkFactor;

	BOOLEAN IsUpdateBeacon;
	BEACON_SYNC_STRUCT *pBeaconSync;
	RALINK_TIMER_STRUCT BeaconUpdateTimer;
	uint32_t BeaconAdjust;
	uint32_t BeaconFactor;
	uint32_t BeaconRemain;
#endif /* RTMP_MAC_USB */



	NDIS_SPIN_LOCK MeasureReqTabLock;
	PMEASURE_REQ_TAB pMeasureReqTab;

	NDIS_SPIN_LOCK TpcReqTabLock;
	PTPC_REQ_TAB pTpcReqTab;

	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
	HTTRANSMIT_SETTING MCastPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

#ifdef SINGLE_SKU
	uint16_t DefineMaxTxPwr;
	BOOLEAN bSKUMode;
	uint16_t AntGain;
	uint16_t BandedgeDelta;
	uint16_t ModuleTxpower;
#endif /* SINGLE_SKU */

#ifdef SINGLE_SKU_V2
#endif /* SINGLE_SKU_V2 */



	BOOLEAN HT_DisallowTKIP;	/* Restrict the encryption type in 11n HT mode */

	BOOLEAN HT_Disable;	/* 1: disable HT function; 0: enable HT function */


#ifdef PRE_ANT_SWITCH
	BOOLEAN PreAntSwitch;	/* Preamble Antenna Switch */
	SHORT PreAntSwitchRSSI;	/* Preamble Antenna Switch RSSI threshold */
	SHORT PreAntSwitchTimeout; /* Preamble Antenna Switch timeout in seconds */
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
	SHORT	CFOTrack;	/* CFO Tracking. 0=>use default, 1=>track, 2-7=> track 8-n times, 8=>done tracking */
#endif /* CFO_TRACK */

#ifdef NEW_RATE_ADAPT_SUPPORT
	USHORT	lowTrafficThrd;		/* Threshold for reverting to default MCS when traffic is low */
	SHORT	TrainUpRuleRSSI;	/* If TrainUpRule=2 then use Hybrid rule when RSSI < TrainUpRuleRSSI */
	USHORT	TrainUpLowThrd;		/* QuickDRS Hybrid train up low threshold */
	USHORT	TrainUpHighThrd;	/* QuickDRS Hybrid train up high threshold */
	BOOLEAN	TrainUpRule;		/* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
#define		STREAM_MODE_STA_NUM		4

	UCHAR	StreamMode; /* 0=disabled, 1=enable for 1SS, 2=enable for 2SS, 3=enable for 1,2SS */
	UCHAR	StreamModeMac[STREAM_MODE_STA_NUM][MAC_ADDR_LEN];
	uint16_t StreamModeMCS;	/* Bit map for enabling Stream Mode based on MCS */
#endif /* STREAM_MODE_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ITxBfTimeout;
	ULONG ETxBfTimeout;
	ULONG	ETxBfEnCond;		/* Enable sending of sounding and beamforming */
	BOOLEAN	ETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN	ETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
	ULONG DebugFlags;	/* Temporary debug flags */
#endif /* DBG_CTRL_SUPPORT */




#ifdef MICROWAVE_OVEN_SUPPORT
	MO_CFG_STRUCT MO_Cfg;	/* data structure for mitigating microwave interference */
#endif /* MICROWAVE_OVEN_SUPPORT */


/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	LNA_VGA_CTL_STRUCT lna_vga_ctl;
#endif /* DYNAMIC_VGA_SUPPORT */


} COMMON_CONFIG, *PCOMMON_CONFIG;

#ifdef DBG_CTRL_SUPPORT
/* DebugFlag definitions */
#define DBF_NO_BF_AWARE_RA		0x0001	/* Revert to older Rate Adaptation that is not BF aware */
#define DBF_SHOW_BF_STATS		0x0002	/* Display BF statistics in AP "iwpriv stat" display */
#define DBF_NO_TXBF_3SS			0x0004	/* Disable TXBF for MCS > 20 */
#define DBF_UNUSED0008			0x0008	/* Unused */
#define DBF_DBQ_RA_LOG			0x0010	/* Log RA information in DBQ */
#define DBF_INIT_MCS_MARGIN		0x0020	/* Use 6 dB margin when selecting initial MCS */
#define DBF_INIT_MCS_DIS1		0x0040	/* Disable highest MCSs when selecting initial MCS */
#define DBF_FORCE_QUICK_DRS		0x0080	/* Force Quick DRS even if rate didn't change */
#define DBF_FORCE_SGI			0x0100	/* Force Short GI */
#define DBF_DBQ_NO_BCN			0x0200	/* Disable logging of RX Beacon frames */
#define DBF_LOG_VCO_CAL			0x0400	/* Log VCO cal */
#define DBF_DISABLE_CAL			0x0800	/* Disable Divider Calibration at channel change */
#ifdef INCLUDE_DEBUG_QUEUE
#define DBF_DBQ_TXFIFO			0x1000	/* Enable logging of TX information from FIFO */
#define DBF_DBQ_TXFRAME			0x2000	/* Enable logging of Frames queued for TX */
#define DBF_DBQ_RXWI_FULL		0x4000	/* Enable logging of full RXWI */
#define DBF_DBQ_RXWI			0x8000	/* Enable logging of partial RXWI */
#endif /* INCLUDE_DEBUG_QUEUE */

#define DBF_SHOW_RA_LOG			0x010000	/* Display concise Rate Adaptation information */
#define DBF_SHOW_ZERO_RA_LOG	0x020000	/* Include RA Log entries when TxCount is 0 */
#define DBF_FORCE_20MHZ			0x040000	/* Force 20 MHz TX */
#define DBF_FORCE_40MHZ 		0x080000	/* Force 40 MHz Tx */
#define DBF_DISABLE_CCK			0x100000	/* Disable CCK */
#define DBF_UNUSED200000		0x200000	/* Unused */
#define DBF_ENABLE_HT_DUP		0x400000	/* Allow HT Duplicate mode in TX rate table */
#define DBF_ENABLE_CCK_5G		0x800000	/* Enable CCK rates in 5G band */
#define DBF_UNUSED0100000		0x0100000	/* Unused */
#define DBF_ENABLE_20MHZ_MCS8	0x02000000	/* Substitute 20MHz MCS8 for 40MHz MCS8 */
#define DBF_DISABLE_20MHZ_MCS0	0x04000000	/* Disable substitution of 20MHz MCS0 for 40MHz MCS32 */
#define DBF_DISABLE_20MHZ_MCS1	0x08000000	/* Disable substitution of 20MHz MCS1 for 40MHz MCS0 */
#endif /* DBG_CTRL_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT

typedef struct _WPA_SUPPLICANT_INFO{
        /*
                802.1x WEP + MD5 will set key to driver before assoc, but we need to apply the key to
                ASIC after get EAPOL-Success frame, so we use this flag to indicate that
        */
        BOOLEAN IEEE8021x_required_keys;
        CIPHER_KEY DesireSharedKey[4];  /* Record user desired WEP keys */
        UCHAR DesireSharedKeyId;

        /* 0x00: driver ignores wpa_supplicant */
        /* 0x01: wpa_supplicant initiates scanning and AP selection */
        /* 0x02: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters */
        /* 0x80: wpa_supplicant trigger driver to do WPS */
        UCHAR WpaSupplicantUP;
        UCHAR WpaSupplicantScanCount;
        BOOLEAN bRSN_IE_FromWpaSupplicant;
        BOOLEAN bLostAp;
        UCHAR *pWpsProbeReqIe;
        UINT WpsProbeReqIeLen;
        UCHAR *pWpaAssocIe;
        UINT WpaAssocIeLen;
}WPA_SUPPLICANT_INFO;
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef CONFIG_STA_SUPPORT

#ifdef CREDENTIAL_STORE
typedef struct _STA_CONNECT_INFO {
	BOOLEAN Changeable;
	BOOLEAN IEEE8021X;
	CHAR Ssid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
	UCHAR SsidLen; // the actual ssid length in used
	NDIS_802_11_AUTHENTICATION_MODE AuthMode; // This should match to whatever microsoft defined
	NDIS_802_11_WEP_STATUS WepStatus;
	UCHAR DefaultKeyId;
	UCHAR PMK[LEN_PMK]; // WPA PSK mode PMK
	UCHAR WpaPassPhrase[64]; // WPA PSK pass phrase
	UINT WpaPassPhraseLen; // the length of WPA PSK pass phrase
	UINT8 WpaState;
	CIPHER_KEY SharedKey[1][4]; // STA always use SharedKey[BSS0][0..3]
	NDIS_SPIN_LOCK Lock;
} STA_CONNECT_INFO, *P_STA_CONNECT_INFO;
#endif /* CREDENTIAL_STORE */






/* Modified by Wu Xi-Kun 4/21/2006 */
/* STA configuration and status */
typedef struct _STA_ADMIN_CONFIG {
	struct rtmp_wifi_dev wdev;

	/*
		GROUP 1 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, but not necessary fully equal to the final
		settings in ACTIVE BSS after negotiation/compromize with the BSS holder (either
		AP or IBSS holder).
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	UCHAR BssType;		/* BSS_INFRA or BSS_ADHOC */

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
#define MONITOR_FLAG_11N_SNIFFER		0x01
	UCHAR BssMonitorFlag;	/* Specific flag for monitor */
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

	USHORT AtimWin;		/* used when starting a new IBSS */

	/*
		GROUP 2 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, and should be always applied to the final
		settings in ACTIVE BSS without compromising with the BSS holder.
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	USHORT DefaultListenCount;	/* default listen count; */
	USHORT ThisTbttNumToNextWakeUp;
	ULONG WindowsPowerMode;	/* Power mode for AC power */
	ULONG WindowsBatteryPowerMode;	/* Power mode for battery if exists */
	BOOLEAN bWindowsACCAMEnable;	/* Enable CAM power mode when AC on */
	BOOLEAN bAutoReconnect;	/* Set to TRUE when setting OID_802_11_SSID with no matching BSSID */
	UCHAR RssiTrigger;
	UCHAR RssiTriggerMode;	/* RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD */

	ULONG WindowsPowerProfile;	/* Windows power profile, for NDIS5.1 PnP */

	BOOLEAN	 FlgPsmCanNotSleep; /* TRUE: can not switch ASIC to sleep */
	/* MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1) */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE) */
	USHORT DisassocReason;
	UCHAR DisassocSta[MAC_ADDR_LEN];
	USHORT DeauthReason;
	UCHAR DeauthSta[MAC_ADDR_LEN];
	USHORT AuthFailReason;
	UCHAR AuthFailSta[MAC_ADDR_LEN];

	/*
		Security segment
	*/
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */

	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;	/* Indicate current Pair & Group use different cipher suites */
	USHORT RsnCapability;

	UCHAR WpaPassPhrase[64];	/* WPA PSK pass phrase */
	UINT WpaPassPhraseLen;	/* the length of WPA PSK pass phrase */
	UCHAR PMK[LEN_PMK];	/* WPA PSK mode PMK */
	UCHAR PTK[LEN_PTK];	/* WPA PSK mode PTK */
	UCHAR GMK[LEN_GMK];	/* WPA PSK mode GMK */
	UCHAR GTK[MAX_LEN_GTK];	/* GTK from authenticator */
	UCHAR GNonce[32];	/* GNonce for WPA2PSK from authenticator */
	CIPHER_KEY TxGTK;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum;	/* Saved PMKID number */


	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG MicErrCnt;	/* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */
	/* For WPA-PSK supplicant state */
	UINT8 WpaState;		/* Default is SS_NOTUSE and handled by microsoft 802.1x */
	UCHAR ReplayCounter[8];
	UCHAR ANonce[32];	/* ANonce for WPA-PSK from aurhenticator */
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */

	UCHAR LastSNR0;		/* last received BEACON's SNR */
	UCHAR LastSNR1;		/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;		/* last received BEACON's SNR for 3nd  antenna */
	int32_t BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
#endif /* DOT11N_SS3_SUPPORT */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	ULONG LastBeaconRxTime;	/* OS's timestamp of the last BEACON RX time */

	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */
	BOOLEAN bNotFirstScan;	/* Sam add for ADHOC flag to do first scan when do initialization */
	BOOLEAN bSwRadio;	/* Software controlled Radio On/Off, TRUE: On */
	BOOLEAN bHwRadio;	/* Hardware controlled Radio On/Off, TRUE: On */
	BOOLEAN bRadio;		/* Radio state, And of Sw & Hw radio state */
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */
	BOOLEAN bShowHiddenSSID;	/* Show all known SSID in SSID list get operation */

	/* New for WPA, windows want us to to keep association information and */
	/* Fixed IEs from last association response */
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN];	/* The content saved here should be little-endian format. */
	USHORT ResVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];

	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be little-endian format. */

	//ULONG CLBusyBytes;	/* Save the total bytes received durning channel load scan time */
	USHORT RPIDensity[8];	/* Array for RPI density collection */

	UCHAR RMReqCnt;		/* Number of measurement request saved. */
	UCHAR CurrentRMReqIdx;	/* Number of measurement request saved. */
	BOOLEAN ParallelReq;	/* Parallel measurement, only one request performed, */
	/* It must be the same channel with maximum duration */
	USHORT ParallelDuration;	/* Maximum duration for parallel measurement */
	UCHAR ParallelChannel;	/* Only one channel with parallel measurement */
	USHORT IAPPToken;	/* IAPP dialog token */
	/* Hack for channel load and noise histogram parameters */
	UCHAR NHFactor;		/* Parameter for Noise histogram */
	UCHAR CLFactor;		/* Parameter for channel load */

	RALINK_TIMER_STRUCT StaQuickResponeForRateUpTimer;
	BOOLEAN StaQuickResponeForRateUpTimerRunning;

	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */



	RALINK_TIMER_STRUCT WpaDisassocAndBlockAssocTimer;
	/* Fast Roaming */
	BOOLEAN bAutoRoaming;	/* 0:disable auto roaming by RSSI, 1:enable auto roaming by RSSI */
	CHAR dBmToRoam;		/* the condition to roam when receiving Rssi less than this value. It's negative value. */

#ifdef WPA_SUPPLICANT_SUPPORT
	WPA_SUPPLICANT_INFO wpa_supplicant_info;
#endif /* WPA_SUPPLICANT_SUPPORT */


	CHAR dev_name[16];
	USHORT OriDevType;

	BOOLEAN bTGnWifiTest;
	BOOLEAN bSkipAutoScanConn;


#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR IEEE80211dClientMode;
	UCHAR StaOriCountryCode[3];
	UCHAR StaOriGeography;
#endif /* EXT_BUILD_CHANNEL_LIST */




#ifdef DOT11W_PMF_SUPPORT
	PMF_CFG PmfCfg;
#endif /* DOT11W_PMF_SUPPORT */

	BOOLEAN bAutoConnectByBssid;
	ULONG BeaconLostTime;	/* seconds */
	BOOLEAN bForceTxBurst;	/* 1: force enble TX PACKET BURST, 0: disable */
	BOOLEAN bAutoConnectIfNoSSID;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	UCHAR RegClass;		/*IE_SUPP_REG_CLASS: 2009 PF#3: For 20/40 Intolerant Channel Report */
#endif /* DOT11N_DRAFT3 */
	BOOLEAN bAdhocN;
#endif /* DOT11_N_SUPPORT */
	BOOLEAN bAdhocCreator;	/*TRUE indicates divice is Creator. */


	/*
	   Enhancement Scanning Mechanism
	   To decrease the possibility of ping loss
	 */
	BOOLEAN bImprovedScan;
	BOOLEAN BssNr;
	UCHAR ScanChannelCnt;	/* 0 at the beginning of scan, stop at 7 */
	UCHAR LastScanChannel;
	/************************************/

	BOOLEAN bFastConnect;

/*connectinfo  for tmp store connect info from UI*/
	BOOLEAN Connectinfoflag;
	UCHAR   ConnectinfoBssid[MAC_ADDR_LEN];
	UCHAR   ConnectinfoChannel;
	UCHAR   ConnectinfoSsidLen;
	CHAR    ConnectinfoSsid[MAX_LEN_OF_SSID];
	UCHAR ConnectinfoBssType;



#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
	BOOLEAN				AdaptiveFreq;  /* Todo: iwpriv and profile support. */
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;





} STA_ADMIN_CONFIG, *PSTA_ADMIN_CONFIG;


/*
	This data structure keep the current active BSS/IBSS's configuration that
	this STA had agreed upon joining the network. Which means these parameters
	are usually decided by the BSS/IBSS creator instead of user configuration.
	Data in this data structurre is valid only when either ADHOC_ON()/INFRA_ON()
	is TRUE. Normally, after SCAN or failed roaming attempts, we need to
	recover back to the current active settings
*/
typedef struct _STA_ACTIVE_CONFIG {
	USHORT Aid;
	USHORT AtimWin;		/* in kusec; IBSS parameter set element */
	USHORT CapabilityInfo;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	USHORT CfpMaxDuration;
	USHORT CfpPeriod;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRateLen;
	/* Copy supported ht from desired AP's beacon. We are trying to match */
	RT_PHY_INFO SupportedPhyInfo;
	RT_HT_CAPABILITY SupportedHtPhy;
#ifdef DOT11_VHT_AC
	RT_VHT_CAP	SupVhtCap;
#endif /* DOT11_VHT_AC */
} STA_ACTIVE_CONFIG, *PSTA_ACTIVE_CONFIG;



#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP related data structures
  **************************************************************************/
/* AUTH-RSP State Machine Aux data structure */
typedef struct _AP_MLME_AUX {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Alg;
	CHAR Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;
#endif /* CONFIG_AP_SUPPORT */



typedef struct _MAC_TABLE_ENTRY {
	/*
	   0:Invalid,
	   Bit 0: AsCli, Bit 1: AsWds, Bit 2: AsAPCLI,
	   Bit 3: AsMesh, Bit 4: AsDls, Bit 5: AsTDls
	 */
	uint32_t EntryType;
	uint32_t ent_status;
	struct rtmp_wifi_dev *wdev;
	PVOID pAd;
	struct _MAC_TABLE_ENTRY *pNext;

	/*
		A bitmap of BOOLEAN flags. each bit represent an operation status of a particular
		BOOLEAN control, either ON or OFF. These flags should always be accessed via
		CLIENT_STATUS_TEST_FLAG(), CLIENT_STATUS_SET_FLAG(), CLIENT_STATUS_CLEAR_FLAG() macros.
		see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition. fCLIENT_STATUS_AMSDU_INUSED
	*/
	ULONG ClientStatusFlags;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode;	/* For transmit phy setting in TXWI. */
	HTTRANSMIT_SETTING MinHTPhyMode;

/*
	wcid:

	tr_tb_idx:

	func_tb_idx used to indicate following index:
		in ApCfg.ApCliTab
		in pAd->MeshTab
		in WdsTab.MacTab

	apidx: should remove this
*/
	UCHAR wcid;
	UCHAR tr_tb_idx;
	UCHAR func_tb_idx;
	UCHAR apidx;		/* MBSS number */

	BOOLEAN isCached;

	BOOLEAN isRalink;
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */

	UCHAR Addr[MAC_ADDR_LEN];
#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif /* CONFIG_AP_SUPPORT */

	/*
		STATE MACHINE Status
	*/
	USHORT Aid;	/* in range 1~2007, with bit 14~15 = b'11, e.g., 0xc001~0xc7d7 */
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */

#ifdef RT_CFG80211_P2P_SUPPORT
	CFG_P2P_ENTRY_PARM CFGP2pInfo;
#endif /* RT_CFG80211_SUPPORT */

#ifdef VENDOR_FEATURE1_SUPPORT
	/* total 128B, use uint32_t to avoid alignment problem */
	uint32_t HeaderBuf[32];	/* (total 128B) TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */

	UCHAR HdrPadLen;	/* recording Header Padding Length; */
	UCHAR MpduHeaderLen;
	uint16_t Protocol;
#endif /* VENDOR_FEATURE1_SUPPORT */

	USHORT TxSeq[NUM_OF_TID];
	USHORT NonQosDataSeq;

	/* Rx status related parameters */
	UCHAR LastRssi;
	RSSI_SAMPLE RssiSample;
	uint32_t LastTxRate;
	uint32_t LastRxRate;
	SHORT freqOffset;		/* Last RXWI FOFFSET */
	SHORT freqOffsetValid;	/* Set when freqOffset field has been updated */

#ifdef CONFIG_AP_SUPPORT
#define MAX_LAST_DATA_RSSI_LEN 5
	CHAR LastDataRssi[MAX_LAST_DATA_RSSI_LEN];
	CHAR curLastDataRssiIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
	USHORT NoBADataCountDown;

	uint32_t CachedBuf[16];	/* UINT (4 bytes) for alignment */
#endif /* DOT11_N_SUPPORT */


	/* WPA/WPA2 4-way database */
	UCHAR EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */
	RALINK_TIMER_STRUCT EnqueueStartForPSKTimer;	/* A timer which enqueue EAPoL-Start for triggering PSK SM */


#ifdef DROP_MASK_SUPPORT
	BOOLEAN	tx_fail_drop_mask_enabled;
	NDIS_SPIN_LOCK	drop_mask_lock;
	BOOLEAN	ps_drop_mask_enabled;
	RALINK_TIMER_STRUCT	dropmask_timer;
#endif /* DROP_MASK_SUPPORT */

	/* record which entry revoke MIC Failure , if it leaves the BSS itself, AP won't update aMICFailTime MIB */
	UCHAR CMTimerRunning;
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR PTK[64];
	UCHAR ReTryCounter;
	RALINK_TIMER_STRUCT RetryTimer;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	UINT8 WpaState;
	UINT8 GTKState;
	USHORT PortSecured;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	CIPHER_KEY PairwiseKey;
	INT PMKID_CacheIdx;
	UCHAR PMKID[LEN_PMKID];
	UCHAR NegotiatedAKM[LEN_OUI_SUITE];	/* It indicate the negotiated AKM suite */


	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	ULONG NoDataIdleCount;
	ULONG AssocDeadLine;
	uint16_t StationKeepAliveCount;	/* unit: second */
	USHORT CapabilityInfo;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	UCHAR VirtualTimeout; /* peer power save virtual timeout */
	ULONG PsQIdleCount;
	QUEUE_HEADER PsQueue;


/*
	wdev_idx used to indicate following index:
		in ApCfg.ApCliTab
		in pAd->MeshTab
		in WdsTab.MacTab
*/
	UCHAR wdev_idx;

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	uint32_t StaConnectTime;	/* the live time of this station since associated with AP */
	uint32_t StaIdleTimeout;	/* idle timeout per entry */

#ifdef UAPSD_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN bAPSDCapablePerAC[4];	/* for trigger-enabled */
	BOOLEAN bAPSDDeliverEnabledPerAC[4];	/* for delivery-enabled */


	UCHAR MaxSPLength;

	BOOLEAN bAPSDAllAC;	/* 1: all AC are delivery-enabled U-APSD */

	QUEUE_HEADER UAPSDQueue[WMM_NUM_OF_AC];	/* queue for each U-APSD */
	USHORT UAPSDQIdleCount;	/* U-APSD queue timeout */

	PQUEUE_ENTRY pUAPSDEOSPFrame;	/* the last U-APSD frame */
	USHORT UAPSDTxNum;	/* total U-APSD frame number */
	BOOLEAN bAPSDFlagEOSPOK;	/* 1: EOSP frame is tx by ASIC */
	BOOLEAN bAPSDFlagSPStart;	/* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
	   unsigned long */
	unsigned long UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN bAPSDFlagSpRoughUse;	/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
	   clear it when statistics handle.
	   if the flag is set when PS-poll frame is received then calling
	   statistics handler to clear it. */
	BOOLEAN bAPSDFlagLegacySent;	/* 1: Legacy PS sent but yet statistics handle */

#ifdef RTMP_MAC_USB
	uint32_t UAPSDTagOffset[WMM_NUM_OF_AC];
#endif /* RTMP_MAC_USB */
#endif /* UAPSD_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	uint32_t StreamModeMACReg;	/* MAC reg used to control stream mode for this client. 0=>No stream mode */
#endif /* STREAM_MODE_SUPPORT */

	UINT FIFOCount;
	UINT DebugFIFOCount;
	UINT DebugTxCount;

/* ==================================================== */
	enum RATE_ADAPT_ALG rateAlg;
	UCHAR RateLen;
	UCHAR MaxSupportedRate;
	BOOLEAN bAutoTxRateSwitch;
	UCHAR CurrTxRate;
	UCHAR CurrTxRateIndex;
	UCHAR lastRateIdx;
	UCHAR *pTable;	/* Pointer to this entry's Tx Rate Table */

	UCHAR lowTrafficCount;
#ifdef NEW_RATE_ADAPT_SUPPORT
	UCHAR fewPktsCnt;
	BOOLEAN perThrdAdj;
	UCHAR mcsGroup;/* the mcs group to be tried */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	AGS_CONTROL AGSCtrl;	/* AGS control */
#endif /* AGS_SUPPORT */

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	CHAR LastTimeTxRateChangeAction;	/* Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount; /* TxSuccess count in last Rate Adaptation interval */
	UCHAR LastTxPER;	/* Tx PER in last Rate Adaptation interval */
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	uint32_t CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */

	UCHAR SupportRateMode; /* 1: CCK 2:OFDM 4: HT, 8:VHT */
	BOOLEAN SupportCCKMCS[MAX_LEN_OF_CCK_RATES];
	BOOLEAN SupportOFDMMCS[MAX_LEN_OF_OFDM_RATES];
#ifdef DOT11_N_SUPPORT
	BOOLEAN SupportHTMCS[MAX_LEN_OF_HT_RATES];
#ifdef DOT11_VHT_AC
	BOOLEAN SupportVHTMCS[MAX_LEN_OF_VHT_RATES];
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
	uint32_t bPeerDelBaTxAdaptEn;
	RALINK_TIMER_STRUCT DelBA_tx_AdaptTimer;
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef MFB_SUPPORT
	UCHAR lastLegalMfb;	/* last legal mfb which is used to set rate */
	BOOLEAN isMfbChanged;	/* purpose: true when mfb has changed but the new mfb is not adopted for Tx */
	struct _RTMP_RA_LEGACY_TB *LegalMfbRS;
	BOOLEAN fLastChangeAccordingMfb;
	NDIS_SPIN_LOCK fLastChangeAccordingMfbLock;
/* Tx MRQ */
	BOOLEAN toTxMrq;
	UCHAR msiToTx, mrqCnt;	/*mrqCnt is used to count down the inverted-BF mrq to be sent */
/* Rx mfb */
	UCHAR pendingMfsi;
/* Tx MFB */
	BOOLEAN toTxMfb;
	UCHAR	mfbToTx;
	UCHAR	mfb0, mfb1;
#endif	/* MFB_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR			TxSndgType;
	NDIS_SPIN_LOCK	TxSndgLock;

/* ETxBF */
	UCHAR		bfState;
	UCHAR		sndgMcs;
	UCHAR		sndgBW;
	UCHAR		sndg0Mcs;
	INT			sndg0Snr0, sndg0Snr1, sndg0Snr2;

#ifdef ETXBF_EN_COND3_SUPPORT
	UCHAR		bestMethod;
	UCHAR		sndgRateIdx;
	UCHAR		bf0Mcs, sndg0RateIdx, bf0RateIdx;
	UCHAR		sndg1Mcs, bf1Mcs, sndg1RateIdx, bf1RateIdx;
	INT			sndg1Snr0, sndg1Snr1, sndg1Snr2;
#endif /* ETXBF_EN_COND3_SUPPORT */
	UCHAR		noSndgCnt;
	UCHAR		eTxBfEnCond;
	UCHAR		noSndgCntThrd, ndpSndgStreams;
	UCHAR		iTxBfEn;
	RALINK_TIMER_STRUCT eTxBfProbeTimer;

	BOOLEAN		phyETxBf;			/* True=>Set ETxBF bit in PHY rate */
	BOOLEAN		phyITxBf;			/* True=>Set ITxBF bit in PHY rate */
	UCHAR		lastNonBfRate;		/* Last good non-BF rate */
	BOOLEAN		lastRatePhyTxBf;	/* For Quick Check. True if last rate was BF */
	USHORT      BfTxQuality[MAX_TX_RATE_INDEX + 1];	/* Beamformed TX Quality */

	COUNTER_TXBF TxBFCounters;		/* TxBF Statistics */
	UINT LastETxCount;		/* Used to compute %BF statistics */
	UINT LastITxCount;
	UINT LastTxCount;
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */

	uint32_t OneSecTxNoRetryOkCount;
	uint32_t OneSecTxRetryOkCount;
	uint32_t OneSecTxFailCount;
	uint32_t OneSecRxLGICount;		/* unicast-to-me Long GI count */
	uint32_t OneSecRxSGICount;      	/* unicast-to-me Short GI count */

	uint32_t StatTxRetryOkCount;
	uint32_t StatTxFailCount;

	int32_t  DownTxMCSRate[NUM_OF_SWFB];
	uint32_t LowPacket;
	UCHAR  LastSaveRateIdx;

#ifdef FIFO_EXT_SUPPORT
	uint32_t fifoTxSucCnt;
	uint32_t fifoTxRtyCnt;
#endif /* FIFO_EXT_SUPPORT */

	uint32_t ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;

/*==================================================== */
	EXT_CAP_INFO_ELEMENT ext_cap;

#ifdef DOT11_N_SUPPORT
	HT_CAPABILITY_IE HTCapability;

	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */

	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UCHAR MmpsMode;		/* MIMO power save mode. */

#ifdef DOT11N_DRAFT3
	UCHAR BSS2040CoexistenceMgmtSupport;
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap_ie;

	/* only take effect if ext_cap.operating_mode_notification = 1 */
	BOOLEAN force_op_mode;
	OPERATING_MODE operating_mode;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */


	BOOLEAN bWscCapable;
	UCHAR Receive_EapolStart_EapRspId;

	uint32_t TXMCSExpected[MAX_MCS_SET];
	uint32_t TXMCSSuccessful[MAX_MCS_SET];
	uint32_t TXMCSFailed[MAX_MCS_SET];
	uint32_t TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];

#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */






#ifdef DOT11W_PMF_SUPPORT
	UCHAR PmfTxTsc[LEN_WPA_TSC];
	UCHAR PmfRxTsc[LEN_WPA_TSC];
        RALINK_TIMER_STRUCT SAQueryTimer;
        RALINK_TIMER_STRUCT SAQueryConfirmTimer;
	UCHAR SAQueryStatus;
        USHORT TransactionID;
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG TxBytes;
	ULONG RxBytes;
#endif /* CONFIG_AP_SUPPORT */



	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;


typedef enum _MAC_ENT_STATUS_{
	/* fAnyStationInPsm */
	MAC_TB_ANY_PSM = 0x1,
	/*
		fAnyStationBadAtheros
		Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip.
	*/
	MAC_TB_ANY_ATH = 0x2,
	/*
		fAnyTxOPForceDisable
		Check if it is necessary to disable BE TxOP
	*/
	MAC_TB_FORCE_TxOP = 0x4,
	/*
		fAllStationAsRalink
		Check if all stations are ralink-chipset
	*/
	MAC_TB_ALL_RALINK = 0x8,
	/*
		fAnyStationIsLegacy
		Check if I use legacy rate to transmit to my BSS Station
	*/
	MAC_TB_ANY_LEGACY = 0x10,
	/*
		fAnyStationNonGF
		Check if any Station can't support GF
	*/
	MAC_TB_ANY_NON_GF = 0x20,
	/* fAnyStation20Only */
	MAC_TB_ANY_HT20 = 0x40,
	/*
		fAnyStationMIMOPSDynamic
		Check if any Station is MIMO Dynamic
	*/
	MAC_TB_ANY_MIMO_DYNAMIC = 0x80,
	/*
		fAnyBASession
		Check if there is BA session.  Force turn on RTS/CTS
	*/
	MAC_TB_ANY_BA = 0x100,
	/* fAnyStaFortyIntolerant */
	MAC_TB_ANY_40_INTOlERANT = 0x200,
	/*
		fAllStationGainGoodMCS
		Check if all stations more than MCS threshold
	*/
	MAC_TB_ALL_GOOD_MCS = 0x400,
	/*
		fAnyStationIsHT
		Check if still has any station set the Intolerant bit on!
	*/
	MAC_TB_ANY_HT = 0x800,
	/* fAnyWapiStation */
	MAC_TB_ANY_WAPI = 0x1000,
}MAC_ENT_STATUS;

typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];

	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	USHORT Size;
	QUEUE_HEADER McastPsQueue;
	ULONG PsQIdleCount;
	BOOLEAN fAnyStationInPsm;
	BOOLEAN fAnyStationBadAtheros;	/* Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip. */
	BOOLEAN fAnyTxOPForceDisable;	/* Check if it is necessary to disable BE TxOP */
	BOOLEAN fAllStationAsRalink;	/* Check if all stations are ralink-chipset */
#ifdef DOT11_N_SUPPORT
	BOOLEAN fAnyStationIsLegacy;	/* Check if I use legacy rate to transmit to my BSS Station/ */
	BOOLEAN fAnyStationNonGF;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStation20Only;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStationMIMOPSDynamic;	/* Check if any Station is MIMO Dynamic */
	BOOLEAN fAnyBASession;	/* Check if there is BA session.  Force turn on RTS/CTS */
	BOOLEAN fAnyStaFortyIntolerant;	/* Check if still has any station set the Intolerant bit on! */
	BOOLEAN fAllStationGainGoodMCS; /* Check if all stations more than MCS threshold */

#ifdef CONFIG_AP_SUPPORT
	BOOLEAN fAnyStationIsHT;	/* Check if there is 11n STA.  Force turn off AP MIMO PS */
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */


	USHORT MsduLifeTime; /* life time for PS packet */
} MAC_TABLE, *PMAC_TABLE;



#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP WDS related data structures
  **************************************************************************/


/***************************************************************************
  *	AP APCLI related data structures
  **************************************************************************/
typedef struct _APCLI_STRUCT {
	struct rtmp_wifi_dev wdev;

	BOOLEAN Enable;		/* Set it as 1 if the apcli interface was configured to "1"  or by iwpriv cmd "ApCliEnable" */
	BOOLEAN Valid;		/* Set it as 1 if the apcli interface associated success to remote AP. */

	MLME_AUX MlmeAux;			/* temporary settings used during MLME state machine */

	UCHAR MacTabWCID;	/*WCID value, which point to the entry of ASIC Mac table. */
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];

#ifdef APCLI_CONNECTION_TRIAL
	UCHAR	TrialCh; /* the channel that Apcli interface will try to connect the rootap locates */
	RALINK_TIMER_STRUCT TrialConnectTimer;
	RALINK_TIMER_STRUCT TrialConnectPhase2Timer;
	RALINK_TIMER_STRUCT TrialConnectRetryTimer;
	MAC_TABLE_ENTRY	oldRootAP;
	USHORT NewRootApRetryCnt;
	UCHAR	ifIndex;
	PVOID pAd;
#endif /* APCLI_CONNECTION_TRIAL */

	UCHAR CfgSsidLen;
	CHAR CfgSsid[MAX_LEN_OF_SSID];
	UCHAR CfgApCliBssid[MAC_ADDR_LEN];

	ULONG ApCliRcvBeaconTime;
	ULONG ApCliLinkUpTime;
	USHORT ApCliBeaconPeriod;

	ULONG CtrlCurrState;
	ULONG SyncCurrState;
	ULONG AuthCurrState;
	ULONG AssocCurrState;
	ULONG WpaPskCurrState;

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT	ProbeReqCnt;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	USHORT AuthReqCnt;
	USHORT AssocReqCnt;

	UCHAR MpduDensity;


	/*
		Security segment
	*/
	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;	/* Indicate current Pair & Group use different cipher suites */
	USHORT RsnCapability;

	UCHAR PSK[100];		/* reserve PSK key material */
	UCHAR PSKLen;
	UCHAR PMK[32];		/* WPA PSK mode PMK */
	UCHAR PTK[64];                /* WPA PSK mode PTK */
	UCHAR GTK[32];		/* GTK from authenticator */

	/*CIPHER_KEY            PairwiseKey; */
	CIPHER_KEY SharedKey[SHARE_KEY_NUM];

	/* store RSN_IE built by driver */
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be convert to little-endian format. */
	UCHAR RSNIE_Len;

	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG       MicErrCnt;          /* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */

	/* For WPA-PSK supplicant state */
	UCHAR         ReplayCounter[8];
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */
	UCHAR GNonce[32];	/* GNonce for WPA-PSK from authenticator */

#ifdef WPA_SUPPLICANT_SUPPORT
	WPA_SUPPLICANT_INFO wpa_supplicant_info;

	BOOLEAN	 bScanReqIsFromWebUI;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum; /* Saved PMKID number */
	BOOLEAN bConfigChanged;
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen; /* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN]; /* The content saved here should be little-endian format. */
	USHORT ResVarIELen; /* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];
	UCHAR LastSsidLen;               /* the actual ssid length in used */
	CHAR LastSsid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];
#endif /* WPA_SUPPLICANT_SUPPORT */


	/*
		WPS segment
	*/

	/*
		Transmitting segment
	*/
	UCHAR RxMcsSet[16];


	PSPOLL_FRAME PsPollFrame;
	HEADER_802_11 NullFrame;

	UAPSD_INFO	UapsdInfo;

} APCLI_STRUCT, *PAPCLI_STRUCT;


typedef struct _AP_ADMIN_CONFIG {
	USHORT CapabilityInfo;
	/* Multiple SSID */
	UCHAR BssidNum;
	UCHAR MacMask;
	MULTISSID_STRUCT MBSSID[HW_BEACON_MAX_NUM];
	ULONG IsolateInterStaTrafficBTNBSSID;
#ifdef APCLI_SUPPORT
	UCHAR ApCliInfRunned;	/* Number of  ApClient interface which was running. value from 0 to MAX_APCLI_INTERFACE */
	UINT8 ApCliNum;
	BOOLEAN FlgApCliIsUapsdInfoUpdated;
	APCLI_STRUCT ApCliTab[MAX_APCLI_NUM];	/*AP-client */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	BOOLEAN		ApCliAutoConnectRunning;
	BOOLEAN		ApCliAutoConnectChannelSwitching;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef AP_PARTIAL_SCAN_SUPPORT
	BOOLEAN bPartialScanning;
#define DEFLAUT_PARTIAL_SCAN_CH_NUM		1   /* Must be move to other place */
	UINT8	 PartialScanChannelNum; /* How many channels to scan each time */
	UINT8	 LastPartialScanChannel;
#define DEFLAUT_PARTIAL_SCAN_BREAK_TIME	4  /* Period of partial scaning: unit: 100ms *//* Must be move to other place */
	UINT8	 PartialScanBreakTime;	/* Period of partial scaning: unit: 100ms */
#endif /* AP_PARTIAL_SCAN_SUPPORT */

	/* for wpa */
	RALINK_TIMER_STRUCT CounterMeasureTimer;

	UCHAR CMTimerRunning;
	UCHAR BANClass3Data;
	LARGE_INTEGER aMICFailTime;
	LARGE_INTEGER PrevaMICFailTime;
	ULONG MICFailureCounter;

	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	BOOLEAN bAutoChannelAtBootup;	/* 0: disable, 1: enable */
	ChannelSel_Alg AutoChannelAlg;	/* Alg for selecting Channel */
#ifdef AP_SCAN_SUPPORT
	uint32_t  ACSCheckTime;           /* Periodic timer to trigger Auto Channel Selection (unit: second) */
	uint32_t  ACSCheckCount;          /* if  ACSCheckCount > ACSCheckTime, then do ACS check */
#endif /* AP_SCAN_SUPPORT */
	BOOLEAN bAvoidDfsChannel;	/* 0: disable, 1: enable */
	BOOLEAN bIsolateInterStaTraffic;
	BOOLEAN bHideSsid;

	/* temporary latch for Auto channel selection */
	ULONG ApCnt;		/* max RSSI during Auto Channel Selection period */
	UCHAR AutoChannel_Channel;	/* channel number during Auto Channel Selection */
	UCHAR current_channel_index;	/* current index of channel list */
	UCHAR AutoChannelSkipListNum;	/* number of rejected channel list */
	UCHAR AutoChannelSkipList[10];
	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	UCHAR ErpIeContent;
	ULONG LastOLBCDetectTime;
	ULONG LastNoneHTOLBCDetectTime;
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */

	UCHAR LastSNR0;		/* last received BEACON's SNR */
	UCHAR LastSNR1;		/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;		/* last received BEACON's SNR for 2nd  antenna */
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT1X_SUPPORT
	/* dot1x related parameter */
	uint32_t own_ip_addr;
	uint32_t retry_interval;
	uint32_t session_timeout_interval;
	uint32_t quiet_interval;
	UCHAR EAPifname[HW_BEACON_MAX_NUM][IFNAMSIZ];	/* indicate as the binding interface for EAP negotiation. */
	UCHAR EAPifname_len[HW_BEACON_MAX_NUM];
	UCHAR PreAuthifname[HW_BEACON_MAX_NUM][IFNAMSIZ];	/* indicate as the binding interface for WPA2 Pre-authentication. */
	UCHAR PreAuthifname_len[HW_BEACON_MAX_NUM];
#endif /* DOT1X_SUPPORT */

	/* EDCA parameters to be announced to its local BSS */
	EDCA_PARM BssEdcaParm;

	RALINK_TIMER_STRUCT ApQuickResponeForRateUpTimer;
	BOOLEAN ApQuickResponeForRateUpTimerRunning;


	/* Indicate the maximum idle timeout */
	uint32_t StaIdleTimeout;

	ULONG EntryLifeCheck;




#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	UCHAR GreenAPLevel;
	BOOLEAN bGreenAPEnable;
	BOOLEAN bGreenAPActive;
#endif /* GREENAP_SUPPORT */

#endif /* DOT11_N_SUPPORT */

	UCHAR EntryClientCount;

	UCHAR ChangeTxOpClient;
} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;



#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
typedef enum _RT_GREEN_AP_LEVEL {
	GREENAP_11BGN_STAS = 0,
	GREENAP_ONLY_11BG_STAS,
	GREENAP_WITHOUT_ANY_STAS_CONNECT
} RT_GREEN_AP_LEVEL;
#endif /* DOT11_N_SUPPORT */
#endif /* GREENAP_SUPPORT */

/* ----------- end of AP ---------------------------- */
#endif /* CONFIG_AP_SUPPORT */

#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY {
	BOOLEAN SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif /* BLOCK_NET_IF */


struct wificonf {
	BOOLEAN bShortGI;
	BOOLEAN bGreenField;
};

typedef struct _RTMP_DEV_INFO_ {
	UCHAR chipName[16];
	RTMP_INF_TYPE infType;
} RTMP_DEV_INFO;

#ifdef DBG_DIAGNOSE
#define DIAGNOSE_TIME	10	/* 10 sec */


struct dbg_diag_info{
	USHORT TxDataCnt;	/* Tx total data count */
	USHORT TxFailCnt;
	USHORT RxDataCnt;	/* Rx Total Data count. */
	USHORT RxCrcErrCnt;

#ifdef DBG_TXQ_DEPTH
	/* TxSwQueue length in scale of 0, 1, 2, 3, 4, 5, 6, 7, >=8 */
	USHORT TxSWQueCnt[4][9];
#endif /* DBG_TXQ_DEPTH */

#ifdef DBG_TX_RING_DEPTH
	/* TxDesc queue length in scale of 0~14, >=15 */
	USHORT TxDescCnt[24];
#endif /* DBG_TX_RING_DEPTH */

#ifdef DBG_TX_AGG_CNT
	USHORT TxAggCnt;
	USHORT TxNonAggCnt;
	/* TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1. */
	USHORT TxAMPDUCnt[MAX_MCS_SET];
#endif /* DBG_TX_AGG_CNT */

#ifdef DBG_TX_MCS
	/* TxDate MCS Count in range from 0 to 15, step in 1 */
	USHORT TxMcsCnt_HT[MAX_MCS_SET];
#ifdef DOT11_VHT_AC
	UINT TxMcsCnt_VHT[MAX_VHT_MCS_SET];
#endif /* DOT11_VHT_AC */
#endif /* DBG_TX_MCS */

#ifdef DBG_RX_MCS
	USHORT RxCrcErrCnt_HT[MAX_MCS_SET];
	/* Rx HT MCS Count in range from 0 to 15, step in 1 */
	USHORT RxMcsCnt_HT[MAX_MCS_SET];
#ifdef DOT11_VHT_AC
	USHORT RxCrcErrCnt_VHT[MAX_VHT_MCS_SET];
	/* for VHT80MHz only, not calcuate 20/40 MHz packets */
	UINT RxMcsCnt_VHT[MAX_VHT_MCS_SET];
#endif /* DOT11_VHT_AC */
#endif /* DBG_RX_MCS */
};

typedef struct _RtmpDiagStrcut_ {	/* Diagnosis Related element */
	unsigned char inited;
	unsigned char qIdx;
	unsigned char ArrayStartIdx;
	unsigned char ArrayCurIdx;

	struct dbg_diag_info diag_info[DIAGNOSE_TIME];
} RtmpDiagStruct;
#endif /* DBG_DIAGNOSE */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
/*
	The number of channels for per-channel Tx power offset
*/
#define NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET	14

/* The Tx power control using the internal ALC */
#ifdef RT8592
// TODO: shiang-6590, fix me for this ugly definition!
#define LOOKUP_TB_SIZE		45
#else
#define LOOKUP_TB_SIZE		33
#endif /* RT8592 */

typedef struct _TX_POWER_CONTROL {
	BOOLEAN bInternalTxALC; /* Internal Tx ALC */
	BOOLEAN bExtendedTssiMode; /* The extended TSSI mode (each channel has different Tx power if needed) */
	CHAR PerChTxPwrOffset[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1]; /* Per-channel Tx power offset */
	CHAR idxTxPowerTable; /* The index of the Tx power table for ant0 */
	CHAR idxTxPowerTable2; /* The index of the Tx power table for ant1 */
	CHAR RF_TX_ALC; /* 3390: RF R12[4:0]: Tx0 ALC, 3352: RF R47[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC */
	CHAR MAC_PowerDelta; /* Tx power control over MAC 0x1314~0x1324 */
	CHAR MAC_PowerDelta2; /* Tx power control for Tx1 */
	CHAR TotalDeltaPower2; /* Tx power control for Tx1 */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	INT LookupTable[IEEE80211_BAND_NUMS][LOOKUP_TB_SIZE];
	INT RefTemp[IEEE80211_BAND_NUMS];
	UCHAR TssiGain[IEEE80211_BAND_NUMS];
	/* Index offset, -7....25. */
	INT LookupTableIndex;
#endif /* RTMP_TEMPERATURE_COMPENSATION */

} TX_POWER_CONTROL, *PTX_POWER_CONTROL;
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

/* */
/* The entry of transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY {
	USHORT MACRegisterOffset;	/* MAC register offset */
	ULONG RegisterValue;	/* Register value */
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

/* */
/* The maximum registers of transmit power control */
/* */
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS 5




/* */
/* The configuration of the transmit power control over MAC */
/* */
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC {
	UCHAR NumOfEntries;	/* Number of entries */
	TX_POWER_CONTROL_OVER_MAC_ENTRY TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

/* */
/* The extension of the transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC {
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over5G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;

/* For Wake on Wireless LAN */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
typedef struct _WOW_CFG_STRUCT {
	BOOLEAN			bEnable;		/* Enable WOW function*/
	BOOLEAN			bWOWFirmware;	/* Enable WOW function, trigger to reload WOW-support firmware */
	BOOLEAN			bInBand;		/* use in-band signal to wakeup system */
	UINT8			nSelectedGPIO;	/* Side band signal to wake up system */
	UINT8			nDelay;			/* Delay number is multiple of 3 secs, and it used to postpone the WOW function */
	UINT8           nHoldTime;      /* GPIO puls hold time, unit: 10ms */
} WOW_CFG_STRUCT, *PWOW_CFG_STRUCT;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#ifdef NEW_WOW_SUPPORT
typedef enum {
	WOW_PKT_TO_HOST,
	WOW_PKT_TO_ANDES
} WOW_PKT_FLOW_T;

typedef enum {
	WOW_WAKEUP_BY_PCIE,
	WOW_WAKEUP_BY_USB,
	WOW_WAKEUP_BY_GPIO
} WOW_WAKEUP_METHOD_T;

typedef enum {
	WOW_ENABLE = 1,
	WOW_TRAFFIC = 3,
	WOW_WAKEUP = 4
} WOW_FEATURE_T;

typedef enum {
	WOW_MASK_CFG = 1,
	WOW_SEC_CFG,
	WOW_INFRA_CFG,
	WOW_P2P_CFG,
} WOW_CONFIG_T;

enum {
	WOW_MAGIC_PKT,
	WOW_BITMAP,
	WOW_IPV4_TCP_SYNC,
	WOW_IPV6_TCP_SYNC
};

typedef struct NEW_WOW_MASK_CFG_STRUCT {
	uint32_t 	Config_Type;
	uint32_t 	Function_Enable;
	uint32_t 	Detect_Mask;
	uint32_t 	Event_Mask;
} NEW_WOW_MASK_CFG_STRUCT, PNEW_WOW_MASK_CFG_STRUCT;

typedef struct NEW_WOW_SEC_CFG_STRUCT {
	uint32_t 	Config_Type;
	uint32_t 	WPA_Ver;
	UCHAR 	PTK[64];
	UCHAR 	R_COUNTER[8];
	UCHAR 	Key_Id;
	UCHAR 	Cipher_Alg;
	UCHAR 	WCID;
	UCHAR 	Group_Cipher;
} NEW_WOW_SEC_CFG_STRUCT, PNEW_WOW_SEC_CFG_STRUCT;

typedef struct NEW_WOW_INFRA_CFG_STRUCT {
	uint32_t 	Config_Type;
	UCHAR 	STA_MAC[6];
	UCHAR 	AP_MAC[6];
	uint32_t 	AP_Status;
} NEW_WOW_INFRA_CFG_STRUCT, PNEW_WOW_INFRA_CFG_STRUCT;

typedef struct _NEW_WOW_P2P_CFG_STRUCT {
	uint32_t 	Config_Type;
	UCHAR 	GO_MAC[6];
	UCHAR 	CLI_MAC[6];
	uint32_t 	P2P_Status;
} NEW_WOW_P2P_CFG_STRUCT, *PNEW_WOW_P2P_CFG_STRUCT;

typedef struct _NEW_WOW_PARAM_STRUCT {
	uint32_t 	Parameter;
	uint32_t 	Value;
} NEW_WOW_PARAM_STRUCT, *PNEW_WOW_PARAM_STRUCT;
#endif /* NEW_WOW_SUPPORT */

/*
	Packet drop reason code
*/
typedef enum{
	PKT_ATE_ON = 1 << 8,
	PKT_RADAR_ON = 2 << 8,
	PKT_RRM_QUIET = 3 << 8,
	PKT_TX_STOP = 4 <<8,
	PKT_TX_JAM = 5 << 8,

	PKT_NETDEV_DOWN = 6 < 8,
	PKT_NETDEV_NO_MATCH = 7 << 8,
	PKT_NOT_ALLOW_SEND = 8 << 8,

	PKT_INVALID_DST = 9<< 8,
	PKT_INVALID_SRC = 10 << 8,
	PKT_INVALID_PKT_DATA = 11 << 8,
	PKT_INVALID_PKT_LEN = 12 << 8,
	PKT_INVALID_ETH_TYPE = 13 << 8,
	PKT_INVALID_TXBLK_INFO = 14 << 8,
	PKT_INVALID_SW_ENCRYPT = 15 << 8,
	PKT_INVALID_PKT_TYPE = 16 << 8,
	PKT_INVALID_PKT_MIC = 17 << 8,

	PKT_PORT_NOT_SECURE = 18 << 8,
	PKT_TSPEC_NO_MATCH  = 19 << 8,
	PKT_NO_ASSOCED_STA = 20 << 8,
	PKT_INVALID_MAC_ENTRY = 21 << 8,

	PKT_TX_QUE_FULL = 22 << 8,
	PKT_TX_QUE_ADJUST = 23<<8,

	PKT_PS_QUE_TIMEOUT = 24 <<8,
	PKT_PS_QUE_CLEAN = 25 << 8,
	PKT_MCAST_PS_QUE_FULL = 26 << 8,
	PKT_UCAST_PS_QUE_FULL = 27 << 8,

	PKT_RX_EAPOL_SANITY_FAIL = 28 <<8,
	PKT_RX_NOT_TO_KERNEL = 29 << 8,
	PKT_RX_MESH_SIG_FAIL = 30 << 8,
	PKT_APCLI_FAIL = 31 << 8,
	PKT_ZERO_DATA = 32 <<8,
	PKT_SW_DECRYPT_FAIL = 33 << 8,
	PKT_TX_SW_ENC_FAIL = 34 << 8,

	PKT_ACM_FAIL = 35 << 8,
	PKT_IGMP_GRP_FAIL = 36 << 8,
	PKT_MGMT_FAIL = 37 << 8,
	PKT_AMPDU_OUT_ORDER = 38 << 8,
	PKT_UAPSD_EOSP = 39 << 8,
	PKT_UAPSD_Q_FULL = 40 << 8,

	PKT_DRO_REASON_MAX = 41,
}PKT_DROP_REASON;

/* Packet drop Direction code */
typedef enum{
	PKT_TX = 0,
	PKT_RX = 1 << 31,
}PKT_DROP_DIECTION;




typedef struct _BBP_RESET_CTL
{
#define BBP_RECORD_NUM	48

	REG_PAIR BBPRegDB[BBP_RECORD_NUM];
	BOOLEAN	AsicCheckEn;
} BBP_RESET_CTL, *PBBP_RESET_CTL;


typedef struct _SCAN_CTRL_{
	UCHAR ScanType;
	UCHAR BssType;
	UCHAR Channel;
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR Bssid[MAC_ADDR_LEN];

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT APScanTimer;
#endif /* CONFIG_AP_SUPPORT */
}SCAN_CTRL;


#ifdef RT_CFG80211_SUPPORT
typedef struct _CFG80211_VIF_DEV
{
	struct _CFG80211_VIF_DEV *pNext;
	BOOLEAN isMainDev;
	uint32_t devType;
	struct net_device *net_dev;
	UCHAR CUR_MAC[MAC_ADDR_LEN];

	/* ProbeReq Frame */
	BOOLEAN Cfg80211RegisterProbeReqFrame;
	CHAR Cfg80211ProbeReqCount;

	/* Action Frame */
	BOOLEAN Cfg80211RegisterActionFrame;
	CHAR Cfg80211ActionCount;
} CFG80211_VIF_DEV, *PCFG80211_VIF_DEV;

typedef struct _CFG80211_VIF_DEV_SET
{
#define MAX_CFG80211_VIF_DEV_NUM  2

	BOOLEAN inUsed;
	uint32_t vifDevNum;
	LIST_HEADER vifDevList;
	BOOLEAN isGoingOn; /* To check any vif in list */
} CFG80211_VIF_DEV_SET;

/* TxMmgt Related */
typedef struct _CFG80211_TX_PACKET
{
	struct _CFG80211_TX_PACKET *pNext;
	uint32_t TxStatusSeq;			  /* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		  /* TxMgmt Packet buffer content */
	uint32_t TxStatusBufLen;		  /* TxMgmt Packet buffer Length */

} CFG80211_TX_PACKET, *PCFG80211_TX_PACKET;

/* CFG80211 Total CTRL Point */
typedef struct _CFG80211_CONTROL
{
	BOOLEAN FlgCfg8021Disable2040Scan;
	BOOLEAN FlgCfg80211Scanning;   /* Record it When scanReq from wpa_supplicant */
	BOOLEAN FlgCfg80211Connecting; /* Record it When ConnectReq from wpa_supplicant*/
	BOOLEAN FlgCfg80211ApBeaconUpdate;
	/* Scan Related */
    uint32_t *pCfg80211ChanList;    /* the channel list from from wpa_supplicant */
    UCHAR Cfg80211ChanListLen;    /* channel list length */
	UCHAR Cfg80211CurChanIndex;   /* current index in channel list when driver in scanning */

	UCHAR *pExtraIe;  /* Carry on Scan action from supplicant */
	UINT   ExtraIeLen;

	UCHAR Cfg_pending_Ssid[MAX_LEN_OF_SSID+1]; /* Record the ssid, When ScanTable Full */
   	UCHAR Cfg_pending_SsidLen;

	/* ROC Related */
	RALINK_TIMER_STRUCT Cfg80211RocTimer;
	CMD_RTPRIV_IOCTL_80211_CHAN Cfg80211ChanInfo;
	BOOLEAN Cfg80211RocTimerInit;
	BOOLEAN Cfg80211RocTimerRunning;

	/* Tx_Mmgt Related */
	uint32_t TxStatusSeq;			  /* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		  /* TxMgmt Packet buffer content */
	uint32_t TxStatusBufLen;		  /* TxMgmt Packet buffer Length */
	BOOLEAN TxStatusInUsed;
	LIST_HEADER cfg80211TxPacketList;

	/* P2P Releated*/
	UCHAR P2PCurrentAddress[MAC_ADDR_LEN];	  /* User changed MAC address */
	BOOLEAN isCfgDeviceInP2p; 				  /* For BaseRate 6 */

	/* MainDevice Info. */
	CFG80211_VIF_DEV cfg80211MainDev;

	/* For add_virtual_intf */
	CFG80211_VIF_DEV_SET Cfg80211VifDevSet;

#ifdef RT_CFG80211_P2P_SUPPORT
	BOOLEAN bP2pCliPmEnable;

	BOOLEAN bPreKeepSlient;
	BOOLEAN	bKeepSlient;

	UCHAR MyGOwcid;
	UCHAR NoAIndex;
	UCHAR CTWindows; /* CTWindows and OppPS parameter field */

	P2PCLIENT_NOA_SCHEDULE GONoASchedule;
	RALINK_TIMER_STRUCT P2pCTWindowTimer;
	RALINK_TIMER_STRUCT P2pSwNoATimer;
	RALINK_TIMER_STRUCT P2pPreAbsenTimer;

	UCHAR P2pSupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR P2pSupRateLen;
    UCHAR P2pExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR P2pExtRateLen;



#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Dummy P2P Device for ANDROID JB */
	struct net_device *dummy_p2p_net_dev;
	BOOLEAN flg_cfg_dummy_p2p_init;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	ULONG P2POpStatusFlags; /* P2P_CLI_UP / P2P_GO_UP*/
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

	/* In AP Mode */
	UINT8 isCfgInApMode;    /* Is any one Device in AP Mode */
	UCHAR *beacon_tail_buf; /* Beacon buf from upper layer */
	uint32_t beacon_tail_len;

	UCHAR *pCfg80211ExtraIeAssocRsp;
	uint32_t Cfg80211ExtraIeAssocRspLen;

	/* TODO: need fix it */
	UCHAR Cfg80211_Alpha2[2];
} CFG80211_CTRL, *PCFG80211_CTRL;
#endif /* RT_CFG80211_SUPPORT */

typedef struct rtmp_phy_ctrl{
	UINT8 rf_band_cap;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	UINT8 FlgQloadEnable;	/* 1: any BSS WMM is enabled */
	ULONG QloadUpTimeLast;	/* last up time */
	UINT8 QloadChanUtil;	/* last QBSS Load, unit: us */
	uint32_t QloadChanUtilTotal;	/* current QBSS Load Total */
	UINT8 QloadChanUtilBeaconCnt;	/* 1~100, default: 50 */
	UINT8 QloadChanUtilBeaconInt;	/* 1~100, default: 50 */
	uint32_t QloadLatestChannelBusyTimePri;
	uint32_t QloadLatestChannelBusyTimeSec;

	/*
	   ex: For 100ms beacon interval,
	   if the busy time in last TBTT is smaller than 5ms, QloadBusyCount[0] ++;
	   if the busy time in last TBTT is between 5 and 10ms, QloadBusyCount[1] ++;
	   ......
	   if the busy time in last TBTT is larger than 95ms, QloadBusyCount[19] ++;

	   Command: "iwpriv ra0 qload show".
	 */

/* provide busy time statistics for every TBTT */
#define QLOAD_FUNC_BUSY_TIME_STATS

/* provide busy time alarm mechanism */
/* use the function to avoid to locate in some noise environments */
#define QLOAD_FUNC_BUSY_TIME_ALARM

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
#define QLOAD_BUSY_INTERVALS	20	/* partition TBTT to QLOAD_BUSY_INTERVALS */
	/* for primary channel & secondary channel */
	uint32_t QloadBusyCountPri[QLOAD_BUSY_INTERVALS];
	uint32_t QloadBusyCountSec[QLOAD_BUSY_INTERVALS];
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
#define QLOAD_DOES_ALARM_OCCUR(pAd)	(pAd->phy_ctrl.FlgQloadAlarmIsSuspended == TRUE)
#define QLOAD_ALARM_EVER_OCCUR(pAd) (pAd->phy_ctrl.QloadAlarmNumber > 0)
	BOOLEAN FlgQloadAlarmIsSuspended;	/* 1: suspend */

	UINT8 QloadAlarmBusyTimeThreshold;	/* unit: 1/100 */
	UINT8 QloadAlarmBusyNumThreshold;	/* unit: 1 */
	UINT8 QloadAlarmBusyNum;
	UINT8 QloadAlarmDuration;	/* unit: TBTT */

	uint32_t QloadAlarmNumber;	/* total alarm times */
	BOOLEAN FlgQloadAlarm;	/* 1: alarm occurs */

	/* speed up use */
	uint32_t QloadTimePeriodLast;
	uint32_t QloadBusyTimeThreshold;
#else

#define QLOAD_DOES_ALARM_OCCUR(pAd)	0
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */

#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
}RTMP_PHY_CTRL;


/*
	The miniport adapter structure
*/
struct rtmp_adapter {
	struct os_cookie *OS_Cookie;	/* save specific structure relative to OS */
	struct net_device *net_dev;


	NDIS_SPIN_LOCK irq_lock;

	/*======Cmd Thread in PCI/RBUS/USB */
	CmdQ CmdQ;
	NDIS_SPIN_LOCK CmdQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK cmdQTask;

#ifdef RTMP_MAC_USB
/**********************************************************/
/*      USB related parameters                                                         */
/**********************************************************/
/*	struct usb_config_descriptor *config; */
	VOID *config;

	UINT NumberOfPipes;
	USHORT BulkOutMaxPacketSize;
	USHORT BulkInMaxPacketSize;
	UINT8 BulkOutEpAddr[6];
	UINT8 BulkInEpAddr[2];

	/*======Control Flags */
	ULONG BulkFlags;
	BOOLEAN bUsbTxBulkAggre;	/* Flags for bulk out data priority */

	/*======Semaphores (event) */
	RTMP_OS_SEM UsbVendorReq_semaphore;
	RTMP_OS_SEM reg_atomic;
	RTMP_OS_SEM hw_atomic;
	RTMP_OS_SEM mcu_atomic;
	RTMP_OS_SEM tssi_lock;
	PVOID UsbVendorReqBuf;
/*	wait_queue_head_t	 *wait; */
	VOID *wait;

#ifdef RALINK_ATE
	/* lock for ATE */
	NDIS_SPIN_LOCK GenericLock;	/* ATE Tx/Rx generic spinlock */
#endif /* RALINK_ATE */

#endif /* RTMP_MAC_USB */

	/* resource for software backlog queues */
	NDIS_SPIN_LOCK page_lock;	/* for nat speedup by bruce */

/*********************************************************/
/*      Both PCI/USB related parameters                                         */
/*********************************************************/
	/*RTMP_DEV_INFO                 chipInfo; */
	RTMP_INF_TYPE infType;

/*********************************************************/
/*      Driver Mgmt related parameters                                            */
/*********************************************************/
	/* OP mode: either AP or STA */
	UCHAR OpMode;		/* OPMODE_STA, OPMODE_AP */

	struct rtmp_wifi_dev *wdev_list[WDEV_NUM_MAX];

	RTMP_OS_TASK mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	/* If you want use timer task to handle the timer related jobs, enable this. */
	RTMP_TIMER_TASK_QUEUE TimerQ;
	NDIS_SPIN_LOCK TimerQLock;
	RTMP_OS_TASK timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */

/*********************************************************/
/*      Tx related parameters                                                           */
/*********************************************************/
	BOOLEAN DeQueueRunning[NUM_OF_TX_RING];	/* for ensuring RTUSBDeQueuePacket get call once */
	NDIS_SPIN_LOCK DeQueueLock[NUM_OF_TX_RING];

#ifdef RTMP_MAC_USB
	/* Data related context and AC specified, 4 AC supported */
	NDIS_SPIN_LOCK BulkOutLock[6];	/* BulkOut spinlock for 4 ACs */
	NDIS_SPIN_LOCK MLMEBulkOutLock;	/* MLME BulkOut lock */

	HT_TX_CONTEXT TxContext[NUM_OF_TX_RING];
	NDIS_SPIN_LOCK TxContextQueueLock[NUM_OF_TX_RING];	/* TxContextQueue spinlock */

	/* 4 sets of Bulk Out index and pending flag */
	/*
	   array size of NextBulkOutIndex must be larger than or equal to 5;
	   Or BulkOutPending[0] will be overwrited in NICInitTransmit().
	 */
	UCHAR NextBulkOutIndex[NUM_OF_TX_RING];	/* only used for 4 EDCA bulkout pipe */

	BOOLEAN BulkOutPending[6];	/* used for total 6 bulkout pipe */
	UCHAR bulkResetPipeid;
	BOOLEAN MgmtBulkPending;
	ULONG bulkResetReq[6];

#ifdef CONFIG_STA_SUPPORT
	USHORT CountDowntoPsm;
#endif /* CONFIG_STA_SUPPORT */

#endif /* RTMP_MAC_USB */

	/* resource for software backlog queues */
	QUEUE_HEADER TxSwQueue[NUM_OF_TX_RING];	/* 4 AC + 1 HCCA */
	NDIS_SPIN_LOCK TxSwQueueLock[NUM_OF_TX_RING];	/* TxSwQueue spinlock */

	/* Maximum allowed tx software Queue length */
	uint32_t TxSwQMaxLen;

	RTMP_DMABUF MgmtDescRing;	/* Shared memory for MGMT descriptors */
	RTMP_MGMT_RING MgmtRing;
	NDIS_SPIN_LOCK MgmtRingLock;	/* Prio Ring spinlock */


	UCHAR LastMCUCmd;

/*********************************************************/
/*      Rx related parameters                                                          */
/*********************************************************/


#ifdef RTMP_MAC_USB
	RX_CONTEXT RxContext[RX_RING_SIZE];	/* 1 for redundant multiple IRP bulk in. */
	NDIS_SPIN_LOCK BulkInLock;	/* BulkIn spinlock for 4 ACs */
	NDIS_SPIN_LOCK CmdRspLock;
	UCHAR PendingRx;	/* The Maximum pending Rx value should be       RX_RING_SIZE. */
	UCHAR NextRxBulkInIndex;	/* Indicate the current RxContext Index which hold by Host controller. */
	UCHAR NextRxBulkInReadIndex;	/* Indicate the current RxContext Index which driver can read & process it. */
	ULONG NextRxBulkInPosition;	/* Want to contatenate 2 URB buffer while 1st is bulkin failed URB. This Position is 1st URB TransferLength. */
	ULONG TransferBufferLength;	/* current length of the packet buffer */
	ULONG ReadPosition;	/* current read position in a packet buffer */

	CMD_RSP_CONTEXT CmdRspEventContext;
#endif /* RTMP_MAC_USB */

	/* RX re-assembly buffer for fragmentation */
	FRAGMENT_FRAME FragFrame;	/* Frame storage for fragment frame */

/***********************************************************/
/*      ASIC related parameters                                                          */
/***********************************************************/
	RTMP_CHIP_OP chipOps;
	RTMP_CHIP_CAP chipCap;
	struct phy_ops *phy_op;
	struct hw_setting hw_cfg;

	uint32_t MACVersion;	/* MAC version. Record rt2860C(0x28600100) or rt2860D (0x28600101).. */
	uint32_t ChipID;
	uint16_t ChipId; 		/* Chip version. Read from EEPROM 0x00 to identify RT5390H */
	INT dev_idx;

	/* --------------------------- */
	/* E2PROM                                     */
	/* --------------------------- */
	enum EEPROM_STORAGE_TYPE eeprom_type;

	ULONG EepromVersion;	/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG FirmwareVersion;	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	USHORT EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR EEPROMAddressNum;	/* 93c46=6  93c66=8 */
	BOOLEAN EepromAccess;
	UCHAR EFuseTag;

#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN bUseEfuse;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR EEPROMImage[MAX_EEPROM_BUFFER_SIZE];
	UCHAR E2pAccessMode; /* Used to identify flash, efuse, eeprom or bin from start-up */

#ifdef RTMP_FLASH_SUPPORT
	UCHAR *eebuf;
	uint32_t flash_offset;
#endif /* RTMP_FLASH_SUPPORT */

	EEPROM_ANTENNA_STRUC Antenna;	/* Since ANtenna definition is different for a & g. We need to save it for future reference. */
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
#if defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) || defined(MT76x2)
	EEPROM_NIC_CONFIG3_STRUC NicConfig3;
#endif /* defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) */

	/* --------------------------- */
	/* BBP Control                               */
	/* --------------------------- */
#if defined(RALINK_ATE)
	// TODO: shiang-6590, remove "defined(RALINK_ATE)" after ATE has fully re-organized!
	UCHAR BbpWriteLatch[MAX_BBP_ID + 1];	/* record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID */
	UCHAR Bbp94;
#endif /* defined(RALINK_ATE) */

	CHAR BbpRssiToDbmDelta;	/* change from UCHAR to CHAR for high power */
	BBP_R66_TUNING BbpTuning;

	/* ---------------------------- */
	/* RFIC control                                 */
	/* ---------------------------- */
	struct rtmp_phy_ctrl phy_ctrl;

	UCHAR RfIcType;		/* RFIC_xxx */
	UCHAR RfFreqOffset;	/* Frequency offset for channel switching */

	RTMP_RF_REGS LatchRfRegs;	/* latch th latest RF programming value since RF IC doesn't support READ */


	/* This soft Rx Antenna Diversity mechanism is used only when user set */
	/* RX Antenna = DIVERSITY ON */
	SOFT_RX_ANT_DIVERSITY RxAnt;

	/* ---------------------------- */
	/* TxPower control                           */
	/* ---------------------------- */
	CHANNEL_TX_POWER TxPower[MAX_NUM_OF_CHANNELS];	/* Store Tx power value for all channels. */
	CHANNEL_TX_POWER ChannelList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */



	UCHAR ChannelListNum;	/* number of channel in ChannelList[] */
	BOOLEAN BbpForCCK;
	ULONG Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
#ifdef DOT11_VHT_AC
	ULONG Tx80MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE]; // Per-rate Tx power control for VHT BW80 (5GHz only)
	BOOLEAN force_vht_op_mode;
#endif /* DOT11_VHT_AC */



	BOOLEAN bAutoTxAgcA;	/* Enable driver auto Tx Agc control */
	UCHAR TssiRefA;		/* Store Tssi reference value as 25 temperature. */
	UCHAR TssiPlusBoundaryA[5];	/* Tssi boundary for increase Tx power to compensate. */
	UCHAR TssiMinusBoundaryA[5];	/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR TxAgcStepA;	/* Store Tx TSSI delta increment / decrement value */
	CHAR TxAgcCompensateA;	/* Store the compensation (TxAgcStep * (idx-1)) */

	BOOLEAN bAutoTxAgcG;	/* Enable driver auto Tx Agc control */
	UCHAR TssiRefG;		/* Store Tssi reference value as 25 temperature. */
	UCHAR TssiPlusBoundaryG[5];	/* Tssi boundary for increase Tx power to compensate. */
	UCHAR TssiMinusBoundaryG[5];	/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR TxAgcStepG;	/* Store Tx TSSI delta increment / decrement value */
	CHAR TxAgcCompensateG;	/* Store the compensation (TxAgcStep * (idx-1)) */
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	TX_POWER_CONTROL TxPowerCtrl;	/* The Tx power control using the internal ALC */
	CHAR curr_temp;
	CHAR rx_temp_base[2];	/* initial VGA value for chain 0/1,  used for base of rx temp compensation power base */
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */


#ifdef THERMAL_PROTECT_SUPPORT
	BOOLEAN force_one_tx_stream;
	int32_t last_thermal_pro_temp;
	int32_t thermal_pro_criteria;
#endif /* THERMAL_PROTECT_SUPPORT */


#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
	FREQUENCY_CALIBRATION_CONTROL FreqCalibrationCtrl;	/* The frequency calibration control */
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

	signed char BGRssiOffset[3]; /* Store B/G RSSI #0/1/2 Offset value on EEPROM 0x46h */
	signed char ARssiOffset[3]; /* Store A RSSI 0/1/2 Offset value on EEPROM 0x4Ah */

	CHAR BLNAGain;		/* Store B/G external LNA#0 value on EEPROM 0x44h */
	CHAR ALNAGain0;		/* Store A external LNA#0 value for ch36~64 */
	CHAR ALNAGain1;		/* Store A external LNA#1 value for ch100~128 */
	CHAR ALNAGain2;		/* Store A external LNA#2 value for ch132~165 */
	UCHAR TxMixerGain24G;	/* Tx mixer gain value from EEPROM to improve Tx EVM / Tx DAC, 2.4G */
	UCHAR TxMixerGain5G;



	/* ---------------------------- */
	/* MAC control                                 */
	/* ---------------------------- */

	UCHAR wmm_cw_min; /* CW_MIN_IN_BITS, actual CwMin = 2^CW_MIN_IN_BITS - 1 */
	UCHAR wmm_cw_max; /* CW_MAX_IN_BITS, actual CwMax = 2^CW_MAX_IN_BITS - 1 */

#ifdef RT8592
	CHAR bw_cal[3];	// bw cal value for RF_R32, 0:20M, 1:40M, 2:80M
// TODO: shiang-6590, temporary get from windows and need to revise it!!
	/* IQ Calibration */
	UCHAR IQGainTx[3][4];
	UCHAR IQPhaseTx[3][4];
	USHORT IQControl;
#endif /* RT8592 */

#if defined(RT3290) || defined(RLT_MAC)

#ifdef RTMP_MAC_USB
	RTMP_OS_SEM WlanEnLock;
#endif

	WLAN_FUN_CTRL_STRUC WlanFunCtrl;
#endif /* defined(RT3290) || defined(RLT_MAC) */

/*****************************************************************************************/
/*      802.11 related parameters                                                        */
/*****************************************************************************************/
	/* outgoing BEACON frame buffer and corresponding TXD */
	TXWI_STRUC BeaconTxWI;
	PUCHAR BeaconBuf;
	USHORT BeaconOffset[HW_BEACON_MAX_NUM];

	/* pre-build PS-POLL and NULL frame upon link up. for efficiency purpose. */
#ifdef CONFIG_STA_SUPPORT
	PSPOLL_FRAME PsPollFrame;
#endif /* CONFIG_STA_SUPPORT */
	HEADER_802_11 NullFrame;

#ifdef RTMP_MAC_USB
	TX_CONTEXT NullContext;
	TX_CONTEXT PsPollContext;
#endif /* RTMP_MAC_USB */


#ifdef UAPSD_SUPPORT
	NDIS_SPIN_LOCK UAPSDEOSPLock;	/* EOSP frame access lock use */
	BOOLEAN bAPSDFlagSPSuspend;	/* 1: SP is suspended; 0: SP is not */
#endif /* UAPSD_SUPPORT */


/*=========AP=========== */
#ifdef CONFIG_AP_SUPPORT
	/* ----------------------------------------------- */
	/* AP specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_AP */
	/* ----------------------------------------------- */
	AP_ADMIN_CONFIG ApCfg;	/* user configuration when in AP mode */
	AP_MLME_AUX ApMlmeAux;



#ifdef MBSS_SUPPORT
	BOOLEAN FlgMbssInit;
#endif /* MBSS_SUPPORT */


#ifdef APCLI_SUPPORT
	BOOLEAN flg_apcli_init;
#endif /* APCLI_SUPPORT */

/*#ifdef AUTO_CH_SELECT_ENHANCE */
	PBSSINFO pBssInfoTab;
	PCHANNELINFO pChannelInfo;
/*#endif // AUTO_CH_SELECT_ENHANCE */


#endif /* CONFIG_AP_SUPPORT */

/*=======STA=========== */
#ifdef CONFIG_STA_SUPPORT
	/* ----------------------------------------------- */
	/* STA specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_STA */
	/* ----------------------------------------------- */
	STA_ADMIN_CONFIG StaCfg;	/* user desired settings */
	STA_ACTIVE_CONFIG StaActive;	/* valid only when ADHOC_ON(pAd) || INFRA_ON(pAd) */
	char nickname[IW_ESSID_MAX_SIZE + 1];	/* nickname, only used in the iwconfig i/f */
	NDIS_MEDIA_STATE PreMediaState;
#endif /* CONFIG_STA_SUPPORT */

/*=======Common=========== */
	enum RATE_ADAPT_ALG rateAlg;		/* Rate adaptation algorithm */

	NDIS_MEDIA_STATE IndicateMediaState;	/* Base on Indication state, default is NdisMediaStateDisConnected */

#ifdef PROFILE_STORE
	RTMP_OS_TASK 	WriteDatTask;
	BOOLEAN			bWriteDat;
#endif /* PROFILE_STORE */

#ifdef CREDENTIAL_STORE
	STA_CONNECT_INFO StaCtIf;
#endif /* CREDENTIAL_STORE */



	/* MAT related parameters */


	/*
		Frequency setting for rate adaptation
			@ra_interval: 		for baseline time interval
			@ra_fast_interval:	for quick response time interval
	*/
	uint32_t ra_interval;
	uint32_t ra_fast_interval;

	/* configuration: read from Registry & E2PROM */
	BOOLEAN bLocalAdminMAC;	/* Use user changed MAC */
	UCHAR PermanentAddress[MAC_ADDR_LEN];	/* Factory default MAC address */
	UCHAR CurrentAddress[MAC_ADDR_LEN];	/* User changed MAC address */

	/* ------------------------------------------------------ */
	/* common configuration to both OPMODE_STA and OPMODE_AP */
	/* ------------------------------------------------------ */
	UINT VirtualIfCnt;

	COMMON_CONFIG CommonCfg;
	MLME_STRUCT Mlme;

	/* AP needs those vaiables for site survey feature. */
	MLME_AUX MlmeAux;	/* temporary settings used during MLME state machine */
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	SCAN_CTRL ScanCtrl;
	BSS_TABLE ScanTab;	/* store the latest SCAN result */
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

	/*About MacTab, the sta driver will use #0 and #1 for multicast and AP. */
	MAC_TABLE MacTab;	/* ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table. */
	NDIS_SPIN_LOCK MacTabLock;

#ifdef DOT11_N_SUPPORT
	BA_TABLE BATable;
	NDIS_SPIN_LOCK BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;

	BOOLEAN HTCEnable;
#endif /* DOT11_N_SUPPORT */

	EXT_CAP_INFO_ELEMENT ExtCapInfo;

	/* DOT11_H */
	DOT11_H Dot11_H;

	/* encryption/decryption KEY tables */
	CIPHER_KEY SharedKey[HW_BEACON_MAX_NUM + MAX_P2P_NUM][4];	/* STA always use SharedKey[BSS0][0..3] */

	/* various Counters */
	COUNTER_802_3 Counters8023;	/* 802.3 counters */
	COUNTER_802_11 WlanCounters;	/* 802.11 MIB counters */
	COUNTER_RALINK RalinkCounters;	/* Ralink propriety counters */
	/* COUNTER_DRS DrsCounters;	*/ /* counters for Dynamic TX Rate Switching */
	PRIVATE_STRUC PrivateInfo;	/* Private information & counters */

	/* flags, see fRTMP_ADAPTER_xxx flags */
	ULONG Flags;		/* Represent current device status */
	ULONG PSFlags;		/* Power Save operation flag. */
	ULONG MoreFlags;	/* Represent specific requirement */

	/* current TX sequence # */
	USHORT Sequence;

	/* Control disconnect / connect event generation */
	/*+++Didn't used anymore */
	ULONG LinkDownTime;
	/*--- */
	ULONG LastRxRate;
	ULONG LastTxRate;
	/*+++Used only for Station */
	BOOLEAN bConfigChanged;	/* Config Change flag for the same SSID setting */
	/*--- */

	ULONG ExtraInfo;	/* Extra information for displaying status of UI */
	ULONG SystemErrorBitmap;	/* b0: E2PROM version error */

#ifdef SYSTEM_LOG_SUPPORT
	/* --------------------------- */
	/* System event log                       */
	/* --------------------------- */
	RT_802_11_EVENT_TABLE EventTab;
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef HOSTAPD_SUPPORT
	uint32_t IoctlIF;
#endif /* HOSTAPD_SUPPORT */
#ifdef INF_PPA_SUPPORT
	uint32_t g_if_id;
	BOOLEAN PPAEnable;
	PPA_DIRECTPATH_CB *pDirectpathCb;
#endif /* INF_PPA_SUPPORT */

	/**********************************************************/
	/*      Statistic related parameters                                                    */
	/**********************************************************/
#ifdef RTMP_MAC_USB
	ULONG BulkOutDataOneSecCount;
	ULONG BulkInDataOneSecCount;
	ULONG BulkLastOneSecCount;	/* BulkOutDataOneSecCount + BulkInDataOneSecCount */
	ULONG watchDogRxCnt;
	ULONG watchDogRxOverFlowCnt;
	ULONG watchDogTxPendingCnt[NUM_OF_TX_RING];
#endif /* RTMP_MAC_USB */

	BOOLEAN bUpdateBcnCntDone;

	ULONG macwd;
	/* ---------------------------- */
	/* DEBUG paramerts */
	/* ---------------------------- */

	/* ---------------------------- */
	/* rt2860c emulation-use Parameters */
	/* ---------------------------- */
	BOOLEAN bLinkAdapt;
	BOOLEAN bForcePrintTX;
	BOOLEAN bForcePrintRX;
	BOOLEAN bStaFifoTest;
	BOOLEAN bProtectionTest;
	BOOLEAN bHCCATest;
	BOOLEAN bGenOneHCCA;
	BOOLEAN bBroadComHT;
	/*+++Following add from RT2870 USB. */
	ULONG BulkOutReq;
	ULONG BulkOutComplete;
	ULONG BulkOutCompleteOther;
	ULONG BulkOutCompleteCancel;	/* seems not use now? */
	ULONG BulkInReq;
	ULONG BulkInComplete;
	ULONG BulkInCompleteFail;
	/*--- */

	int32_t rts_tx_retry_num;
	struct wificonf WIFItestbed;

	UCHAR TssiGain;
#ifdef RALINK_ATE
	ATE_INFO ate;
#ifdef RTMP_MAC_USB
	BOOLEAN ContinBulkOut;	/*ATE bulk out control */
	BOOLEAN ContinBulkIn;	/*ATE bulk in control */
	RTMP_OS_ATOMIC BulkOutRemained;
	RTMP_OS_ATOMIC BulkInRemained;
#endif /* RTMP_MAC_USB */
#endif /* RALINK_ATE */

#ifdef DOT11_N_SUPPORT
	struct reordering_mpdu_pool mpdu_blk_pool;
#endif /* DOT11_N_SUPPORT */

	/* statistics count */

	VOID *iw_stats;
	VOID *stats;

#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY blockQueueTab[NUM_OF_TX_RING];
#endif /* BLOCK_NET_IF */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	ULONG TbttTickCount;	/* beacon timestamp work-around */

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PeriodicTimer;
#endif /* CONFIG_AP_SUPPORT */
	/* for detect_wmm_traffic() BE TXOP use */
	ULONG OneSecondnonBEpackets;	/* record non BE packets per second */
	UCHAR is_on;

	/* for detect_wmm_traffic() BE/BK TXOP use */
#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR flg_be_adjust;
	ULONG be_adjust_last_time;

	UINT8 FlgCtsEnabled;
	UINT8 PM_FlgSuspend;

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */


	UCHAR FifoUpdateDone, FifoUpdateRx;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	CFG80211_CTRL cfg80211_ctrl;
	VOID *pCfg80211_CB;
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

	uint32_t ContinueMemAllocFailCount;

	struct {
		INT IeLen;
		UCHAR *pIe;
	} ProbeRespIE[MAX_LEN_OF_BSS_TABLE];

	/* purpose: We free all kernel resources when module is removed */
	LIST_HEADER RscTimerMemList;	/* resource timers memory */
	LIST_HEADER RscTaskMemList;	/* resource tasks memory */
	LIST_HEADER RscLockMemList;	/* resource locks memory */
	LIST_HEADER RscTaskletMemList;	/* resource tasklets memory */
	LIST_HEADER RscSemMemList;	/* resource semaphore memory */
	LIST_HEADER RscAtomicMemList;	/* resource atomic memory */

	/* purpose: Cancel all timers when module is removed */
	LIST_HEADER RscTimerCreateList;	/* timers list */


#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
	WOW_CFG_STRUCT WOW_Cfg; /* data structure for wake on wireless */
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	TXWI_STRUC NullTxWI;
	USHORT NullBufOffset[2];

#ifdef APCLI_CERT_SUPPORT
	BOOLEAN bApCliCertTest;
#endif /* APCLI_CERT_SUPPORT */
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
	BOOLEAN bUseMultiMacAddrExt;
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

#ifdef MCS_LUT_SUPPORT
	BOOLEAN bUseHwTxLURate;
#endif /* MCS_LUT_SUPPORT */


#ifdef CONFIG_ANDES_SUPPORT
	struct MCU_CTRL MCUCtrl;
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef WLAN_SKB_RECYCLE
    struct sk_buff_head rx0_recycle;
#endif /* WLAN_SKB_RECYCLE */


#ifdef SINGLE_SKU_V2
	DL_LIST SingleSkuPwrList;
	UCHAR DefaultTargetPwr;
	CHAR SingleSkuRatePwrDiff[18];
	BOOLEAN bOpenFileSuccess;
	BOOLEAN sku_init_done;
	UCHAR tc_init_val;
#endif /* SINGLE_SKU_V2 */

#ifdef ED_MONITOR
	BOOLEAN ed_chk;

//for AP Mode's threshold
#ifdef CONFIG_AP_SUPPORT
	UCHAR ed_sta_threshold;
	UCHAR ed_ap_threshold;
#endif /* CONFIG_AP_SUPPORT */

//for STA Mode's threshold
#ifdef CONFIG_STA_SUPPORT
	UCHAR ed_ap_scaned;
	UCHAR ed_current_ch_aps;
	CHAR ed_rssi_threshold;
#endif /* CONFIG_STA_SUPPORT */

	UCHAR ed_threshold;
	UINT false_cca_threshold;
	UINT ed_block_tx_threshold;
	INT ed_chk_period;  // in unit of ms

	UCHAR ed_stat_sidx;
	UCHAR ed_stat_lidx;
	BOOLEAN ed_tx_stoped;
	UINT ed_trigger_cnt;
	UINT ed_silent_cnt;
	UINT ed_false_cca_cnt;

#define ED_STAT_CNT 20
	uint32_t ed_stat[ED_STAT_CNT];
	uint32_t ed_trigger_stat[ED_STAT_CNT];
	uint32_t ed_silent_stat[ED_STAT_CNT];
	uint32_t ed_2nd_stat[ED_STAT_CNT];
	uint32_t ch_idle_stat[ED_STAT_CNT];
	uint32_t ch_busy_stat[ED_STAT_CNT];
	uint32_t false_cca_stat[ED_STAT_CNT];
	ULONG chk_time[ED_STAT_CNT];
	RALINK_TIMER_STRUCT ed_timer;
	BOOLEAN ed_timer_inited;
#endif /* ED_MONITOR */

#ifdef WFA_VHT_PF
	BOOLEAN force_amsdu;
	BOOLEAN force_noack;
	BOOLEAN vht_force_sgi;
	BOOLEAN vht_force_tx_stbc;

	BOOLEAN force_vht_op_mode;
	UCHAR vht_pf_op_ss;
	UCHAR vht_pf_op_bw;
#endif /* WFA_VHT_PF */


#ifdef DBG_DIAGNOSE
	RtmpDiagStruct DiagStruct;
#endif /* DBG_DIAGNOSE */


#ifdef RTMP_USB_SUPPORT
	struct usb_control usb_ctl;
#endif


#ifdef VHT_TXBF_SUPPORT
	BOOLEAN NDPA_Request;
    BOOLEAN BeaconSndDimensionFlag;    // Peer sounding dimension flag
#endif
};

#ifdef ED_MONITOR
INT ed_status_read(struct rtmp_adapter *pAd);
INT ed_monitor_init(struct rtmp_adapter *pAd);
INT ed_monitor_exit(struct rtmp_adapter *pAd);
#endif /* ED_MONITOR */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
/* The offset of the Tx power tuning entry (zero-based array) */
#define TX_POWER_TUNING_ENTRY_OFFSET			30

/* The lower-bound of the Tx power tuning entry */
#define LOWERBOUND_TX_POWER_TUNING_ENTRY		-30

/* The upper-bound of the Tx power tuning entry in G band */
#define UPPERBOUND_TX_POWER_TUNING_ENTRY(__pAd)		((__pAd)->chipCap.TxAlcTxPowerUpperBound_2G)

#ifdef A_BAND_SUPPORT
/* The upper-bound of the Tx power tuning entry in A band */
#define UPPERBOUND_TX_POWER_TUNING_ENTRY_5G(__pAd)		((__pAd)->chipCap.TxAlcTxPowerUpperBound_5G)
#endif /* A_BAND_SUPPORT */

/* Temperature compensation lookup table */

#define TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET	7

/* The lower/upper power delta index for the TSSI rate table */

#define LOWER_POWER_DELTA_INDEX		0
#define UPPER_POWER_DELTA_INDEX		24

/* The offset of the TSSI rate table */

#define TSSI_RATIO_TABLE_OFFSET	12


/* Get the power delta bound */

#define GET_TSSI_RATE_TABLE_INDEX(x) (((x) > UPPER_POWER_DELTA_INDEX) ? (UPPER_POWER_DELTA_INDEX) : (((x) < LOWER_POWER_DELTA_INDEX) ? (LOWER_POWER_DELTA_INDEX) : ((x))))

/* 802.11b CCK TSSI information */

typedef union _CCK_TSSI_INFO
{
#ifdef RT_BIG_ENDIAN
	struct
	{
		UCHAR	Reserved:1;
		UCHAR	ShortPreamble:1;
		UCHAR	Rate:2;
		UCHAR	Tx40MSel:2;
		UCHAR	TxType:2;
	} field;
#else
	struct
	{
		UCHAR	TxType:2;
		UCHAR	Tx40MSel:2;
		UCHAR	Rate:2;
		UCHAR	ShortPreamble:1;
		UCHAR	Reserved:1;
	} field;
#endif /* RT_BIG_ENDIAN */

	UCHAR	value;
} CCK_TSSI_INFO, *PCCK_TSSI_INFO;


/* 802.11a/g OFDM TSSI information */

typedef union _OFDM_TSSI_INFO
{
#ifdef RT_BIG_ENDIAN
	struct
	{
		UCHAR	Rate:4;
		UCHAR	Tx40MSel:2;
		UCHAR	TxType:2;
	} field;
#else
	struct
	{
		UCHAR	TxType:2;
		UCHAR	Tx40MSel:2;
		UCHAR	Rate:4;
	} field;
#endif /* RT_BIG_ENDIAN */

	UCHAR	value;
} OFDM_TSSI_INFO, *POFDM_TSSI_INFO;


/* 802.11n HT TSSI information */

typedef struct _HT_TSSI_INFO {
	union {
#ifdef RT_BIG_ENDIAN
		struct {
			UCHAR SGI:1;
			UCHAR STBC:2;
			UCHAR Aggregation:1;
			UCHAR Tx40MSel:2;
			UCHAR TxType:2;
		} field;
#else
		struct {
			UCHAR TxType:2;
			UCHAR Tx40MSel:2;
			UCHAR Aggregation:1;
			UCHAR STBC:2;
			UCHAR SGI:1;
		} field;
#endif /* RT_BIG_ENDIAN */

		UCHAR value;
	} PartA;

	union {
#ifdef RT_BIG_ENDIAN
		struct {
			UCHAR BW:1;
			UCHAR MCS:7;
		} field;
#else
		struct {
			UCHAR MCS:7;
			UCHAR BW:1;
		} field;
#endif /* RT_BIG_ENDIAN */
		UCHAR	value;
	} PartB;
} HT_TSSI_INFO, *PHT_TSSI_INFO;

typedef struct _TSSI_INFO_{
	UCHAR tssi_info_0;
	union {
		CCK_TSSI_INFO cck_tssi_info;
		OFDM_TSSI_INFO ofdm_tssi_info;
		HT_TSSI_INFO ht_tssi_info_1;
		UCHAR byte;
	}tssi_info_1;
	HT_TSSI_INFO ht_tssi_info_2;
}TSSI_INFO;

#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */


typedef struct _PEER_PROBE_REQ_PARAM {
	UCHAR Addr2[MAC_ADDR_LEN];
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	BOOLEAN bRequestRssi;
} PEER_PROBE_REQ_PARAM, *PPEER_PROBE_REQ_PARAM;


/***************************************************************************
  *	Rx Path software control block related data structures
  **************************************************************************/
typedef enum RX_BLK_FLAGS{
	fRX_AMPDU = 0x0001,
	fRX_AMSDU = 0x0002,
	fRX_ARALINK = 0x0004,
	fRX_HTC = 0x0008,
	fRX_PAD = 0x0010,
	fRX_QOS = 0x0020,
	fRX_EAP = 0x0040,
	fRX_WPI = 0x0080,
	fRX_AP = 0x0100,		// Packet received from AP
	fRX_STA = 0x0200,	// Packet received from Client(Infra moed)
	fRX_ADHOC = 0x400,	// packet received from AdHoc peer
	fRX_WDS = 0x0800,	// Packet received from WDS
	fRX_MESH = 0x1000,	// Packet received from MESH
	fRX_DLS = 0x2000,	// Packet received from DLS peer
	fRX_TDLS = 0x4000,	// Packet received from TDLS peer
}RX_BLK_FLAGS;

typedef struct _RX_BLK
{
	UCHAR hw_rx_info[RXD_SIZE]; /* include "RXD_STRUC RxD" and "RXINFO_STRUC rx_info " */
	RXINFO_STRUC *pRxInfo; /* for RLT, in head of frame buffer, for RTMP, in hw_rx_info[RXINFO_OFFSET] */
#ifdef RLT_MAC
	RXFCE_INFO *pRxFceInfo; /* for RLT, in in hw_rx_info[RXINFO_OFFSET], for RTMP, no such field */
#endif /* RLT_MAC */
	RXWI_STRUC *pRxWI; /*in frame buffer and after "rx_info" fields */
	HEADER_802_11 *pHeader; /* poiter of 802.11 header, pointer to frame buffer and shall not shift this pointer */
	struct sk_buff *pRxPacket; /* os_packet pointer, shall not change */
	UCHAR *pData; /* init to pRxPacket->data, refer to frame buffer, may changed depends on processing */
	USHORT DataSize; /* init to  RXWI->MPDUtotalByteCnt, and may changes depends on processing */
	USHORT Flags;

	/* Mirror info of partial fields of RxWI and RxInfo */
	USHORT MPDUtotalByteCnt; /* Refer to RXWI->MPDUtotalByteCnt */
	UCHAR UserPriority;	/* for calculate TKIP MIC using */
	UCHAR OpMode;	/* 0:OPMODE_STA 1:OPMODE_AP */
	UCHAR wcid;		/* copy of pRxWI->wcid */
	UCHAR U2M;
	UCHAR key_idx;
	UCHAR bss_idx;
	UCHAR TID;
	CHAR rssi[3];
	CHAR snr[3];
	CHAR freq_offset;
	CHAR ldpc_ex_sym;
	HTTRANSMIT_SETTING rx_rate;
#ifdef HDR_TRANS_SUPPORT
	BOOLEAN	bHdrRxTrans;	/* this packet's header is translated to 802.3 by HW  */
	BOOLEAN bHdrVlanTaged;	/* VLAN tag is added to this header */
	UCHAR *pTransData;
	USHORT TransDataSize;
#endif /* HDR_TRANS_SUPPORT */
} RX_BLK;


#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))


#define fRX_WDS			0x0001
#define fRX_AMSDU		0x0002
#define fRX_ARALINK		0x0004
#define fRX_HTC			0x0008
#define fRX_PAD			0x0010
#define fRX_AMPDU		0x0020
#define fRX_QOS			0x0040
#define fRX_INFRA		0x0080
#define fRX_EAP			0x0100
#define fRX_MESH		0x0200
#define fRX_APCLI		0x0400
#define fRX_DLS			0x0800
#define fRX_WPI			0x1000
#define fRX_P2PGO		0x2000
#define fRX_P2PCLI		0x4000

#define AMSDU_SUBHEAD_LEN	14
#define ARALINK_SUBHEAD_LEN	14
#define ARALINK_HEADER_LEN	 2


/***************************************************************************
  *	Tx Path software control block related data structures
  **************************************************************************/
#define TX_UNKOWN_FRAME		0x00
#define TX_MCAST_FRAME		0x01
#define TX_LEGACY_FRAME		0x02
#define TX_AMPDU_FRAME		0x04
#define TX_AMSDU_FRAME		0x08
#define TX_RALINK_FRAME		0x10
#define TX_FRAG_FRAME		0x20
#define TX_NDPA_FRAME		0x40


/* Currently the sizeof(TX_BLK) is 148 bytes. */
typedef struct _TX_BLK {
	UCHAR				QueIdx;
	UCHAR				TxFrameType;				/* Indicate the Transmission type of the all frames in one batch */
	UCHAR				TotalFrameNum;				/* Total frame number want to send-out in one batch */
	USHORT				TotalFragNum;				/* Total frame fragments required in one batch */
	USHORT				TotalFrameLen;				/* Total length of all frames want to send-out in one batch */

	QUEUE_HEADER		TxPacketList;
	MAC_TABLE_ENTRY	*pMacEntry;					/* NULL: packet with 802.11 RA field is multicast/broadcast address */
	HTTRANSMIT_SETTING	*pTransmit;

	/* Following structure used for the characteristics of a specific packet. */
	struct sk_buff *	pPacket;
	UCHAR				*pSrcBufHeader;				/* Reference to the head of sk_buff->data */
	UCHAR				*pSrcBufData;				/* Reference to the sk_buff->data, will changed depends on hanlding progresss */
	UINT				SrcBufLen;					/* Length of packet payload which not including Layer 2 header */

	PUCHAR				pExtraLlcSnapEncap;			/* NULL means no extra LLC/SNAP is required */
#ifndef VENDOR_FEATURE1_SUPPORT
	/*
		Note: Can not insert any other new parameters
		between pExtraLlcSnapEncap & HeaderBuf; Or
		the start address of HeaderBuf will not be aligned by 4.

		But we can not change HeaderBuf[128] to HeaderBuf[32] because
		many codes use HeaderBuf[index].
	*/
	UCHAR				HeaderBuf[128];				/* TempBuffer for TX_INFO + TX_WI + TSO_INFO + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */
#else
	uint32_t 			HeaderBuffer[32];			/* total 128B, use uint32_t to avoid alignment problem */
	UCHAR				*HeaderBuf;
#endif /* VENDOR_FEATURE1_SUPPORT */
	UCHAR				MpduHeaderLen;				/* 802.11 header length NOT including the padding */
	UCHAR				HdrPadLen;					/* recording Header Padding Length; */
	UCHAR				UserPriority;				/* priority class of packet */
	UCHAR				FrameGap;					/* what kind of IFS this packet use */
	UCHAR				MpduReqNum;					/* number of fragments of this frame */
// TODO: shiang-6590, potential to remove
	UCHAR				TxRate;						/* TODO: Obsoleted? Should change to MCS? */
	UCHAR				CipherAlg;					/* cipher alogrithm */
	PCIPHER_KEY			pKey;
	UCHAR				KeyIdx;						/* Indicate the transmit key index */
	UCHAR				OpMode;
	UCHAR				Wcid;						/* The MAC entry associated to this packet */
	UCHAR				apidx;						/* The interface associated to this packet */
	UCHAR				wdev_idx;				// Used to replace apidx

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT				ApCliIfidx;
	PAPCLI_STRUCT		pApCliEntry;
#endif /* APCLI_SUPPORT */

	MULTISSID_STRUCT *pMbss;

#endif /* CONFIG_AP_SUPPORT */
// TODO: ---End

	uint32_t 			Flags;						/*See following definitions for detail. */

	/*YOU SHOULD NOT TOUCH IT! Following parameters are used for hardware-depended layer. */
	ULONG				Priv;						/* Hardware specific value saved in here. */

#ifdef TXBF_SUPPORT
	UCHAR				TxSndgPkt; /* 1: sounding 2: NDP sounding */
	UCHAR				TxNDPSndgBW;
	UCHAR				TxNDPSndgMcs;
#endif /* TXBF_SUPPORT */

#ifdef TX_PKT_SG
	PACKET_INFO pkt_info;
#endif /* TX_PKT_SG */

#ifdef HDR_TRANS_SUPPORT
	BOOLEAN				NeedTrans;	/* indicate the packet needs to do hw header translate */
#endif /* HDR_TRANS_SUPPORT */

	struct rtmp_wifi_dev *wdev;
} TX_BLK;


#define fTX_bRtsRequired			0x0001	/* Indicate if need send RTS frame for protection. Not used in RT2860/RT2870. */
#define fTX_bAckRequired			0x0002	/* the packet need ack response */
#define fTX_bPiggyBack			0x0004	/* Legacy device use Piggback or not */
#define fTX_bHTRate				0x0008	/* allow to use HT rate */
#define fTX_bForceNonQoS		0x0010	/* force to transmit frame without WMM-QoS in HT mode */
#define fTX_bAllowFrag			0x0020	/* allow to fragment the packet, A-MPDU, A-MSDU, A-Ralink is not allowed to fragment */
#define fTX_bMoreData			0x0040	/* there are more data packets in PowerSave Queue */
#define fTX_bWMM				0x0080	/* QOS Data */
#define fTX_bClearEAPFrame		0x0100

#define	fTX_bSwEncrypt			0x0400	/* this packet need to be encrypted by software before TX */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define fTX_bApCliPacket			0x0200
#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */

#ifdef UAPSD_SUPPORT
#define	fTX_bWMM_UAPSD_EOSP	0x0800	/* Used when UAPSD_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */





#ifdef WFA_VHT_PF
#define fTX_AmsduInAmpdu			0x40000
#endif /* WFA_VHT_PF */

#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))



typedef struct dequeue_info{
	UCHAR qidx;
	UCHAR wcid;
	INT pkt_bytes;
	INT pkt_cnt;
}DEQUE_INFO;


#ifdef RT_BIG_ENDIAN
/***************************************************************************
  *	Endian conversion related functions
  **************************************************************************/
/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd 	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline VOID	RTMPWIEndianChange(
	IN	struct rtmp_adapter *pAd,
	IN	PUCHAR			pData,
	IN	ULONG			DescriptorType)
{
	int size;
	int i;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 RXWISize = pAd->chipCap.RXWISize;

	size = ((DescriptorType == TYPE_TXWI) ? TXWISize : RXWISize);

	if(DescriptorType == TYPE_TXWI)
	{
		*((uint32_t *)(pData)) = SWAP32(*((uint32_t *)(pData)));		/* Byte 0~3 */
		*((uint32_t *)(pData + 4)) = SWAP32(*((uint32_t *)(pData+4)));	/* Byte 4~7 */
		if (size > 16)
			*((uint32_t *)(pData + 16)) = SWAP32(*((uint32_t *)(pData + 16)));	/* Byte 16~19 */
	}
	else
	{
		for(i=0; i < size/4 ; i++)
			*(((uint32_t *)pData) +i) = SWAP32(*(((uint32_t *)pData)+i));
	}
}




/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd 	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/

#ifdef RTMP_MAC_USB
static inline VOID RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescType)
{
	*((uint32_t *)(pData)) = SWAP32(*((uint32_t *)(pData)));
}
#endif /* RTMP_MAC_USB */
/*
	========================================================================

	Routine Description:
		Endian conversion of all kinds of 802.11 frames .

	Arguments:
		pAd 	Pointer to our adapter
		pData			Pointer to the 802.11 frame structure
		Dir 			Direction of the frame
		FromRxDoneInt	Caller is from RxDone interrupt

	Return Value:
		None

	Note:
		Call this function when read or update buffer data
	========================================================================
*/
static inline VOID	RTMPFrameEndianChange(
	IN	struct rtmp_adapter *pAd,
	IN	PUCHAR			pData,
	IN	ULONG			Dir,
	IN	BOOLEAN 		FromRxDoneInt)
{
	PHEADER_802_11 pFrame;
	PUCHAR	pMacHdr;

	/* swab 16 bit fields - Frame Control field */
	if(Dir == DIR_READ)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;

	/* swab 16 bit fields - Duration/ID field */
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	if (pFrame->FC.Type != FC_TYPE_CNTL)
	{
		/* swab 16 bit fields - Sequence Control field */
		*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));
	}

	if(pFrame->FC.Type == FC_TYPE_MGMT)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_REASSOC_REQ:
				/* swab 16 bit fields - CapabilityInfo field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - Listen Interval field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_RSP:
				/* swab 16 bit fields - CapabilityInfo field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - Status Code field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - AID field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_AUTH:
				 /* When the WEP bit is on, don't do the conversion here.
					This is only shared WEP can hit this condition.
					For AP, it shall do conversion after decryption.
					For STA, it shall do conversion before encryption. */
				if (pFrame->FC.Wep == 1)
					break;
				else
				{
					/* swab 16 bit fields - Auth Alg No. field */
					pMacHdr += sizeof(HEADER_802_11);
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					/* swab 16 bit fields - Auth Seq No. field */
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					/* swab 16 bit fields - Status Code field */
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				}
				break;

			case SUBTYPE_BEACON:
			case SUBTYPE_PROBE_RSP:
				/* swab 16 bit fields - BeaconInterval field */
				pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - CapabilityInfo field */
				pMacHdr += sizeof(USHORT);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_DEAUTH:
			case SUBTYPE_DISASSOC:
				/* If the PMF is negotiated, those frames shall be encrypted */
				if(!FromRxDoneInt && pFrame->FC.Wep == 1)
					break;
				else
				{
					/* swab 16 bit fields - Reason code field */
					pMacHdr += sizeof(HEADER_802_11);
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				}
				break;
		}
	}
	else if( pFrame->FC.Type == FC_TYPE_DATA )
	{
	}
	else if(pFrame->FC.Type == FC_TYPE_CNTL)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_BLOCK_ACK_REQ:
				{
					PFRAME_BA_REQ pBAReq = (PFRAME_BA_REQ)pFrame;
					*(USHORT *)(&pBAReq->BARControl) = SWAP16(*(USHORT *)(&pBAReq->BARControl));
					pBAReq->BAStartingSeq.word = SWAP16(pBAReq->BAStartingSeq.word);
				}
				break;
			case SUBTYPE_BLOCK_ACK:
				/* For Block Ack packet, the HT_CONTROL field is in the same offset with Addr3 */
				*(uint32_t *)(&pFrame->Addr3[0]) = SWAP32(*(uint32_t *)(&pFrame->Addr3[0]));
				break;

			case SUBTYPE_ACK:
				/*For ACK packet, the HT_CONTROL field is in the same offset with Addr2 */
				*(uint32_t *)(&pFrame->Addr2[0])=	SWAP32(*(uint32_t *)(&pFrame->Addr2[0]));
				break;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("Invalid Frame Type!!!\n"));
	}

	/* swab 16 bit fields - Frame Control */
	if(Dir == DIR_WRITE)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}
}
#endif /* RT_BIG_ENDIAN */


/***************************************************************************
  *	Other static inline function definitions
  **************************************************************************/
static inline VOID ConvertMulticastIP2MAC(
	IN PUCHAR pIpAddr,
	IN PUCHAR *ppMacAddr,
	IN uint16_t ProtoType)
{
	if (pIpAddr == NULL)
		return;

	if (ppMacAddr == NULL || *ppMacAddr == NULL)
		return;

	switch (ProtoType)
	{
		case ETH_P_IPV6:
/*			memset(*ppMacAddr, 0, MAC_ADDR_LEN); */
			*(*ppMacAddr) = 0x33;
			*(*ppMacAddr + 1) = 0x33;
			*(*ppMacAddr + 2) = pIpAddr[12];
			*(*ppMacAddr + 3) = pIpAddr[13];
			*(*ppMacAddr + 4) = pIpAddr[14];
			*(*ppMacAddr + 5) = pIpAddr[15];
			break;

		case ETH_P_IP:
		default:
/*			memset(*ppMacAddr, 0, MAC_ADDR_LEN); */
			*(*ppMacAddr) = 0x01;
			*(*ppMacAddr + 1) = 0x00;
			*(*ppMacAddr + 2) = 0x5e;
			*(*ppMacAddr + 3) = pIpAddr[1] & 0x7f;
			*(*ppMacAddr + 4) = pIpAddr[2];
			*(*ppMacAddr + 5) = pIpAddr[3];
			break;
	}

	return;
}


char *get_phymode_str(int phy_mode);
char *get_bw_str(int bandwidth);


BOOLEAN RTMPCheckForHang(
	IN  NDIS_HANDLE MiniportAdapterContext);

int RTMPAllocTxRxRingMemory(struct rtmp_adapter *pAd);

#ifdef RESOURCE_PRE_ALLOC
int RTMPInitTxRxRingMemory(struct rtmp_adapter *pAd);
#endif /* RESOURCE_PRE_ALLOC */

VOID get_dev_config_idx(struct rtmp_adapter *pAd);
UCHAR *get_dev_name_prefix(struct rtmp_adapter *pAd, INT dev_type);

int RTMPReadParametersHook(struct rtmp_adapter *pAd);
int RTMPSetProfileParameters(struct rtmp_adapter *pAd, char *pBuffer);

INT RTMPGetKeyParameter(
    IN char *key,
    OUT char *dest,
    IN INT destsize,
    IN char *buffer,
    IN BOOLEAN bTrimSpace);


#ifdef SINGLE_SKU_V2
int RTMPSetSingleSKUParameters(
	IN struct rtmp_adapter *pAd);

VOID UpdateSkuRatePwr(struct rtmp_adapter *pAd, UCHAR ch, UCHAR bw, CHAR base_pwr);
#endif /* SINGLE_SKU_V2 */


#ifdef RTMP_RF_RW_SUPPORT
VOID NICInitRFRegisters(
	IN struct rtmp_adapter *pAd);

VOID RTMP_ReadRF(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			RegID,
	OUT	PUCHAR			pValue1,
	OUT PUCHAR			pValue2,
	IN	UCHAR			BitMask);

VOID RTMP_WriteRF(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			RegID,
	IN	UCHAR			Value,
	IN	UCHAR			BitMask);

int RT30xxWriteRFRegister(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value);

int RT30xxReadRFRegister(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

int RT635xWriteRFRegister(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	UCHAR			value);

int RT635xReadRFRegister(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

BOOLEAN RTMPAdjustFrequencyOffset(
	IN struct rtmp_adapter *pAd,
	INOUT PUCHAR pRefFreqOffset);
#endif /* RTMP_RF_RW_SUPPORT */

BOOLEAN RTMPCheckPhyMode(
	IN struct rtmp_adapter *pAd,
	IN UINT8 BandSupported,
	INOUT UCHAR *pPhyMode);

#ifdef RLT_RF
int rlt_rf_write(
	IN struct rtmp_adapter *pAd,
	IN UCHAR bank,
	IN UCHAR regID,
	IN UCHAR value);

int rlt_rf_read(
	IN struct rtmp_adapter *pAd,
	IN UCHAR bank,
	IN UCHAR regID,
	IN UCHAR *pValue);
#endif /* RLT_RF */

int rtmp_rf_write(
	IN struct rtmp_adapter *pAd,
	IN UCHAR bank,
	IN UCHAR regID,
	IN UCHAR value);

int rtmp_rf_read(
	IN struct rtmp_adapter *pAd,
	IN UCHAR bank,
	IN UCHAR regID,
	IN UCHAR *pValue);

VOID NICReadEEPROMParameters(struct rtmp_adapter *pAd);
VOID NICInitAsicFromEEPROM(struct rtmp_adapter *pAd);

int NICInitializeAdapter(struct rtmp_adapter *pAd, BOOLEAN bHardReset);
int NICInitializeAsic(struct rtmp_adapter *pAd, BOOLEAN bHardReset);


VOID RTMPRingCleanUp(
	IN  struct rtmp_adapter *pAd,
	IN  UCHAR           RingType);

VOID UserCfgExit(struct rtmp_adapter *pAd);
VOID UserCfgInit(struct rtmp_adapter *pAd);

int load_patch(struct rtmp_adapter *ad);
int NICLoadFirmware(struct rtmp_adapter *pAd);

VOID NICUpdateFifoStaCounters(struct rtmp_adapter *pAd);
VOID NICUpdateRawCounters(struct rtmp_adapter *pAd);

#ifdef FIFO_EXT_SUPPORT
BOOLEAN NicGetMacFifoTxCnt(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY *pEntry);

VOID AsicFifoExtSet(struct rtmp_adapter *pAd);
VOID AsicFifoExtEntryClean(struct rtmp_adapter * pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* FIFO_EXT_SUPPORT */

VOID NicResetRawCounters(struct rtmp_adapter *pAd);

VOID NicGetTxRawCounters(
	IN struct rtmp_adapter *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1);

VOID RTMPZeroMemory(VOID *pSrc, ULONG Length);
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length);
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length);

VOID AtoH(char *src, UCHAR *dest, int destlen);
UCHAR BtoH(char ch);

VOID RTMP_TimerListAdd(struct rtmp_adapter *pAd, VOID *pRsc);
VOID RTMP_TimerListRelease(struct rtmp_adapter *pAd, VOID *pRsc);
VOID RTMP_AllTimerListRelease(struct rtmp_adapter *pAd);

VOID RTMPInitTimer(
	IN struct rtmp_adapter *pAd,
	IN RALINK_TIMER_STRUCT *pTimer,
	IN VOID *pTimerFunc,
	IN VOID *pData,
	IN BOOLEAN Repeat);

VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);

VOID RTMPEnableRxTx(struct rtmp_adapter *pAd);

VOID AntCfgInit(struct rtmp_adapter *pAd);

/* */
/* prototype in action.c */
/* */
VOID ActHeaderInit(
    IN struct rtmp_adapter *pAd,
    IN OUT HEADER_802_11 *pHdr80211,
    IN UCHAR *da,
    IN UCHAR *sa,
    IN UCHAR *bssid);

VOID ActionStateMachineInit(
    IN	struct rtmp_adapter *pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeADDBAAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDELBAAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID SendSMPSAction(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Wcid,
	IN UCHAR smps);

#ifdef CONFIG_AP_SUPPORT
VOID SendBeaconRequest(
	IN struct rtmp_adapter *pAd,
	IN UCHAR				Wcid);
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID RECBATimerTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID ORIBATimerTimeout(
	IN	struct rtmp_adapter *pAd);

VOID SendRefreshBAR(
	IN	struct rtmp_adapter *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(
	IN struct rtmp_adapter *pAd);

VOID SendBSS2040CoexistMgmtAction(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR	Wcid,
	IN	UCHAR	apidx,
	IN	UCHAR	InfoReq);

VOID SendNotifyBWActionFrame(
	IN struct rtmp_adapter *pAd,
	IN UCHAR  Wcid,
	IN UCHAR apidx);

BOOLEAN ChannelSwitchSanityCheck(
	IN	struct rtmp_adapter *pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  NewChannel,
	IN    UCHAR  Secondary);

VOID ChannelSwitchAction(
	IN	struct rtmp_adapter *pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  Channel,
	IN    UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
	IN	struct rtmp_adapter *pAd,
	IN    PUCHAR  pDest);

VOID Update2040CoexistFrameAndNotify(
	IN	struct rtmp_adapter *pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID Send2040CoexistAction(
	IN	struct rtmp_adapter *pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID UpdateBssScanParm(
	IN struct rtmp_adapter *pAd,
	IN OVERLAP_BSS_SCAN_IE APBssScan);
#endif /* DOT11N_DRAFT3 */

INT AsicSetRalinkBurstMode(struct rtmp_adapter *pAd, BOOLEAN enable);
#endif /* DOT11_N_SUPPORT */

VOID AsicSetTxPreamble(struct rtmp_adapter *pAd, USHORT TxPreamble);

VOID BarHeaderInit(
	IN	struct rtmp_adapter *pAd,
	IN OUT PFRAME_BAR pCntlBar,
	IN PUCHAR pDA,
	IN PUCHAR pSA);

VOID InsertActField(
	IN struct rtmp_adapter *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode);

BOOLEAN QosBADataParse(
	IN struct rtmp_adapter *pAd,
	IN BOOLEAN bAMSDU,
	IN PUCHAR p8023Header,
	IN UCHAR	WCID,
	IN UCHAR	TID,
	IN USHORT Sequence,
	IN UCHAR DataOffset,
	IN USHORT Datasize,
	IN UINT   CurRxIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN CntlEnqueueForRecv(
    IN	struct rtmp_adapter *pAd,
	IN ULONG Wcid,
    IN ULONG MsgLen,
	IN PFRAME_BA_REQ pMsg);

VOID BaAutoManSwitch(
	IN	struct rtmp_adapter *pAd);
#endif /* DOT11_N_SUPPORT */

VOID HTIOTCheck(
	IN	struct rtmp_adapter *pAd,
	IN    UCHAR     BatRecIdx);


INT rtmp_wdev_idx_reg(struct rtmp_adapter *pAd, struct rtmp_wifi_dev *wdev);
INT rtmp_wdev_idx_unreg(struct rtmp_adapter *pAd, struct rtmp_wifi_dev *wdev);
VOID wdev_tx_pkts(NDIS_HANDLE dev_hnd, struct sk_buff **pkt_list, UINT pkt_cnt, struct rtmp_wifi_dev *wdev);

#ifdef DOT11_N_SUPPORT
VOID RTMP_BASetup(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN UINT8 UserPriority);
#endif /* DOT11_N_SUPPORT */

VOID RTMPDeQueuePacket(
	IN struct rtmp_adapter *pAd,
   	IN BOOLEAN bIntContext,
	IN UCHAR QueIdx,
	IN INT Max_Tx_Packets);

int RTMPFreeTXDRequest(
	IN  struct rtmp_adapter *pAd,
	IN  UCHAR           RingType,
	IN  UCHAR           NumberRequired,
	IN 	PUCHAR          FreeNumberIs);

int MlmeHardTransmit(
	IN  struct rtmp_adapter *pAd,
	IN  UCHAR	QueIdx,
	IN  struct sk_buff *   pPacket,
	IN	BOOLEAN			FlgDataQForce,
	IN	BOOLEAN			FlgIsLocked);

int MlmeHardTransmitMgmtRing(
	IN  struct rtmp_adapter *pAd,
	IN  UCHAR	QueIdx,
	IN  struct sk_buff *   pPacket);


USHORT RTMPCalcDuration(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Rate,
	IN ULONG Size);

VOID RTMPWriteTxWI(
	IN struct rtmp_adapter *pAd,
	IN TXWI_STRUC *pTxWI,
	IN BOOLEAN FRAG,
	IN BOOLEAN CFACK,
	IN BOOLEAN InsTimestamp,
	IN BOOLEAN AMPDU,
	IN BOOLEAN Ack,
	IN BOOLEAN NSeq, /* HW new a sequence. */
	IN UCHAR BASize,
	IN UCHAR WCID,
	IN ULONG Length,
	IN UCHAR PID,
	IN UCHAR TID,
	IN UCHAR TxRate,
	IN UCHAR Txopmode,
	IN HTTRANSMIT_SETTING *pTransmit);


VOID RTMPWriteTxWI_Data(
	IN struct rtmp_adapter *pAd,
	INOUT TXWI_STRUC *pTxWI,
	IN TX_BLK *pTxBlk);


VOID RTMPWriteTxWI_Cache(
	IN struct rtmp_adapter *pAd,
	INOUT TXWI_STRUC *pTxWI,
	IN TX_BLK *pTxBlk);

VOID RTMPSuspendMsduTransmission(
	IN struct rtmp_adapter *pAd);

VOID RTMPResumeMsduTransmission(
	IN struct rtmp_adapter *pAd);

int MiniportMMRequest(
	IN struct rtmp_adapter *pAd,
	IN UCHAR QueIdx,
	IN UCHAR *pData,
	IN UINT Length);

VOID RTMPSendNullFrame(
	IN struct rtmp_adapter *pAd,
	IN UCHAR TxRate,
	IN BOOLEAN bQosNull,
	IN USHORT PwrMgmt);

#ifdef APCLI_SUPPORT
VOID	ApCliRTMPReportMicError(
	IN	struct rtmp_adapter *pAd,
	IN	PCIPHER_KEY 	pWpaKey,
	IN	INT			ifIndex);

VOID   ApCliWpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);
#endif/*APCLI_SUPPORT*/



BOOLEAN RTMPFreeTXDUponTxDmaDone(
	IN struct rtmp_adapter *pAd,
	IN UCHAR            QueIdx);

BOOLEAN RTMPCheckEtherType(
	IN struct rtmp_adapter *pAd,
	IN struct sk_buff *pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN struct rtmp_wifi_dev *wdev,
	OUT PUCHAR pUserPriority,
	OUT PUCHAR pQueIdx);


VOID RTMPCckBbpTuning(
	IN	struct rtmp_adapter *pAd,
	IN	UINT			TxRate);


/*
	MLME routines
*/

/* Asic/RF/BBP related functions */
VOID AsicGetTxPowerOffset(
	IN struct rtmp_adapter *			pAd,
	IN PULONG					TxPwr);

VOID AsicGetAutoAgcOffsetForExternalTxAlc(
	IN struct rtmp_adapter *		pAd,
	IN PCHAR 				pDeltaPwr,
	IN PCHAR 				pTotalDeltaPwr,
	IN PCHAR 				pAgcCompensate,
	IN PCHAR 				pDeltaPowerByBbpR1);

VOID AsicExtraPowerOverMAC(struct rtmp_adapter *pAd);

#ifdef RTMP_TEMPERATURE_COMPENSATION
VOID AsicGetAutoAgcOffsetForTemperatureSensor(
	IN struct rtmp_adapter *		pAd,
	IN PCHAR				pDeltaPwr,
	IN PCHAR				pTotalDeltaPwr,
	IN PCHAR				pAgcCompensate,
	IN PCHAR 				pDeltaPowerByBbpR1);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN 		struct rtmp_adapter *pAd,
	IN 		PCHAR 			pTotalDeltaPower,
	INOUT 	PULONG			pSingleSKUTotalDeltaPwr,
	INOUT  	PUCHAR              	pSingleSKUBbpR1Offset);
#endif /* SINGLE_SKU*/

VOID AsicPercentageDeltaPower(
	IN 		struct rtmp_adapter *		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1);

VOID AsicCompensatePowerViaBBP(
	IN struct rtmp_adapter *pAd,
	INOUT CHAR *pTotalDeltaPower);

VOID AsicAdjustTxPower(struct rtmp_adapter *pAd);

VOID 	AsicUpdateProtect(
	IN		struct rtmp_adapter *pAd,
	IN 		USHORT			OperaionMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist);

VOID AsicBBPAdjust(
	IN struct rtmp_adapter *pAd);

VOID AsicSwitchChannel(
	IN  struct rtmp_adapter *pAd,
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan);

INT AsicSetChannel(
	IN struct rtmp_adapter *pAd,
	IN UCHAR ch,
	IN UINT8 bw,
	IN UINT8 ext_ch,
	IN BOOLEAN bScan);

#ifdef THERMAL_PROTECT_SUPPORT
VOID thermal_protection(
	IN  struct rtmp_adapter *  pAd);
#endif /* THERMAL_PROTECT_SUPPORT */

VOID AsicLockChannel(struct rtmp_adapter *pAd, UCHAR Channel);
VOID AsicAntennaSelect(struct rtmp_adapter *pAd, UCHAR Channel);

VOID AsicResetBBPAgent(struct rtmp_adapter *pAd);

#ifdef CONFIG_STA_SUPPORT
VOID AsicSleepThenAutoWakeup(struct rtmp_adapter *pAd, USHORT TbttNumToNextWakeUp);
VOID AsicForceSleep(struct rtmp_adapter *pAd);
VOID AsicForceWakeup(struct rtmp_adapter *pAd, BOOLEAN bFromTx);
#endif /* CONFIG_STA_SUPPORT */

INT AsicSetDevMac(struct rtmp_adapter *pAd, UCHAR *addr);
VOID AsicSetBssid(struct rtmp_adapter *pAd, UCHAR *pBssid);

VOID AsicDelWcidTab(struct rtmp_adapter *pAd, UCHAR Wcid);

#ifdef MAC_APCLI_SUPPORT
VOID AsicSetApCliBssid(struct rtmp_adapter *pAd, UCHAR *pBssid, UCHAR index);
#endif /* MAC_APCLI_SUPPORT */

INT AsicSetRxFilter(struct rtmp_adapter *pAd);

#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(struct rtmp_adapter *pAd, BOOLEAN bEnable);
#endif /* DOT11_N_SUPPORT */

VOID AsicCtrlBcnMask(struct rtmp_adapter *pAd, INT mask);
INT AsicSetPreTbtt(struct rtmp_adapter *pAd, BOOLEAN enable);
INT AsicSetGPTimer(struct rtmp_adapter *pAd, BOOLEAN enable, uint32_t timeout);

VOID AsicDisableSync(struct rtmp_adapter *pAd);
VOID AsicEnableBssSync(struct rtmp_adapter *pAd);
VOID AsicEnableApBssSync(struct rtmp_adapter *pAd);
VOID AsicEnableIbssSync(struct rtmp_adapter *pAd);

VOID AsicSetEdcaParm(struct rtmp_adapter *pAd, PEDCA_PARM pEdcaParm);

INT AsicSetRetryLimit(struct rtmp_adapter *pAd, uint32_t type, uint32_t limit);
uint32_t AsicGetRetryLimit(struct rtmp_adapter *pAd, uint32_t type);

VOID AsicSetSlotTime(struct rtmp_adapter *pAd, BOOLEAN bUseShortSlotTime);

VOID AsicAddSharedKeyEntry(
	IN struct rtmp_adapter *pAd,
	IN UCHAR         BssIndex,
	IN UCHAR         KeyIdx,
	IN PCIPHER_KEY	 pCipherKey);

VOID AsicRemoveSharedKeyEntry(
	IN struct rtmp_adapter *pAd,
	IN UCHAR         BssIndex,
	IN UCHAR         KeyIdx);

VOID AsicUpdateWCIDIVEIV(
	IN struct rtmp_adapter *pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV);

VOID AsicUpdateRxWCIDTable(
	IN struct rtmp_adapter *pAd,
	IN USHORT		WCID,
	IN PUCHAR        pAddr);

VOID	AsicUpdateWcidAttributeEntry(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8				Wcid,
	IN	UINT8				KeyTabFlag);

VOID AsicAddPairwiseKeyEntry(
	IN struct rtmp_adapter *pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey);

VOID AsicRemovePairwiseKeyEntry(struct rtmp_adapter *pAd, UCHAR Wcid);

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssMode(struct rtmp_adapter *pAd, UCHAR NumOfBcns);
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN AsicSendCommandToMcu(
	IN struct rtmp_adapter *pAd,
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCmdToMcuAndWait(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCommandToMcuBBP(
	IN struct rtmp_adapter *pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked);





#ifdef MCS_LUT_SUPPORT
VOID asic_mcs_lut_update(
	IN struct rtmp_adapter *pAd,
        IN MAC_TABLE_ENTRY *pEntry);
#endif /* MCS_LUT_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
uint32_t StreamModeRegVal(
	IN struct rtmp_adapter *pAd);

VOID AsicSetStreamMode(
	IN struct rtmp_adapter *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled);

VOID RtmpStreamModeInit(
	IN struct rtmp_adapter *pAd);

/*
	Update the Tx chain address
	Parameters
		pAd: The adapter data structure
		pMacAddress: The MAC address of the peer STA

	Return Value:
		None
*/
VOID AsicUpdateTxChainAddress(
	IN struct rtmp_adapter *pAd,
	IN PUCHAR pMacAddress);

INT Set_StreamMode_Proc(
    IN  struct rtmp_adapter *pAd,
    IN  char *        arg);

INT Set_StreamModeMac_Proc(
    IN  struct rtmp_adapter *pAd,
    IN  char *         arg);

INT Set_StreamModeMCS_Proc(
    IN  struct rtmp_adapter *pAd,
    IN  char *        arg);
#endif /* STREAM_MODE_SUPPORT */

#ifdef WOW_SUPPORT
#ifdef RTMP_MAC_USB
/* For WOW, 8051 MUC send full frame */
VOID AsicWOWSendNullFrame(
    IN struct rtmp_adapter *pAd,
    IN UCHAR TxRate,
    IN BOOLEAN bQosNull);

VOID AsicLoadWOWFirmware(
    IN struct rtmp_adapter *pAd,
    IN BOOLEAN WOW);
#endif /* RTMP_MAC_USB */
#endif /* WOW_SUPPORT */

VOID MacAddrRandomBssid(
	IN  struct rtmp_adapter *pAd,
	OUT PUCHAR pAddr);

VOID MgtMacHeaderInit(
	IN  struct rtmp_adapter *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pSA,
	IN UCHAR *pBssid);

VOID MgtMacHeaderInitExt(
    IN  struct rtmp_adapter *pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN PUCHAR pDA,
    IN PUCHAR pSA,
    IN PUCHAR pBssid);

VOID MlmeRadioOff(
	IN struct rtmp_adapter *pAd);

VOID MlmeRadioOn(
	IN struct rtmp_adapter *pAd);


VOID BssTableInit(
	IN BSS_TABLE *Tab);

#ifdef DOT11_N_SUPPORT
VOID BATableInit(
	IN struct rtmp_adapter *pAd,
    IN BA_TABLE *Tab);

VOID BATableExit(
	IN struct rtmp_adapter *pAd);
#endif /* DOT11_N_SUPPORT */

#ifdef ED_MONITOR
ULONG BssChannelAPCount(
	IN BSS_TABLE *Tab,
	IN UCHAR Channel);
#endif /* ED_MONITOR */

ULONG BssTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR pBssid,
	IN UCHAR Channel);

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR    pBssid,
	IN PUCHAR    pSsid,
	IN UCHAR     SsidLen,
	IN UCHAR     Channel);

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR    Bssid,
	IN PUCHAR    pSsid,
	IN UCHAR     SsidLen,
	IN UCHAR     Channel);

ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen);

VOID BssTableDeleteEntry(
	IN OUT  PBSS_TABLE pTab,
	IN      PUCHAR pBssid,
	IN      UCHAR Channel);

ULONG BssTableSetEntry(
	IN struct rtmp_adapter *pAd,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST *ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);


#ifdef DOT11_N_SUPPORT
VOID BATableInsertEntry(
    IN	struct rtmp_adapter *pAd,
	IN USHORT Aid,
    IN USHORT		TimeOutValue,
	IN USHORT		StartingSeq,
    IN UCHAR TID,
	IN UCHAR BAWinSize,
	IN UCHAR OriginatorStatus,
    IN BOOLEAN IsRecipient);

#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


VOID  TriEventInit(
	IN	struct rtmp_adapter *pAd);

INT TriEventTableSetEntry(
	IN	struct rtmp_adapter *pAd,
	OUT TRIGGER_EVENT_TAB *Tab,
	IN PUCHAR pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

VOID BssTableSsidSort(
	IN  struct rtmp_adapter *pAd,
	OUT BSS_TABLE *OutTab,
	IN  CHAR Ssid[],
	IN  UCHAR SsidLen);

VOID  BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder);

VOID BssCipherParse(BSS_ENTRY *pBss);

int  MlmeQueueInit(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE *Queue);

VOID  MlmeQueueDestroy(
	IN MLME_QUEUE *Queue);

BOOLEAN MlmeEnqueue(
	IN struct rtmp_adapter *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv);

BOOLEAN MlmeEnqueueForRecv(
	IN  struct rtmp_adapter *  pAd,
	IN ULONG Wcid,
	IN ULONG TimeStampHigh,
	IN ULONG TimeStampLow,
	IN UCHAR Rssi0,
	IN UCHAR Rssi1,
	IN UCHAR Rssi2,
	IN ULONG MsgLen,
	IN PVOID Msg,
	IN UCHAR Signal,
	IN UCHAR OpMode);


BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue,
	OUT MLME_QUEUE_ELEM **Elem);

VOID    MlmeRestartStateMachine(
	IN  struct rtmp_adapter *pAd);

BOOLEAN  MlmeQueueEmpty(
	IN MLME_QUEUE *Queue);

BOOLEAN  MlmeQueueFull(
	IN MLME_QUEUE *Queue,
	IN UCHAR SendId);

BOOLEAN  MsgTypeSubst(
	IN struct rtmp_adapter *pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

VOID StateMachineInit(
	IN STATE_MACHINE *Sm,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base);

VOID StateMachineSetAction(
	IN STATE_MACHINE *S,
	IN ULONG St,
	ULONG Msg,
	IN STATE_MACHINE_FUNC F);

VOID StateMachinePerformAction(
	IN  struct rtmp_adapter *pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState);

VOID Drop(
	IN  struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID AssocStateMachineInit(
	IN  struct rtmp_adapter *pAd,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ReassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID DisassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

/*---------------------------------------------- */
VOID MlmeDisassocReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeAssocReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeReassocReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeDisassocReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAssocRspAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerReassocRspAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerDisassocAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID DisassocTimeoutAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AssocTimeoutAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  ReassocTimeoutAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  Cls3errAction(
	IN  struct rtmp_adapter *pAd,
	IN  PUCHAR pAddr);

VOID  InvalidStateWhenAssoc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  InvalidStateWhenReassoc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenDisassociate(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

#ifdef RTMP_MAC_USB
VOID MlmeCntlConfirm(
	IN struct rtmp_adapter *pAd,
	IN ULONG MsgType,
	IN USHORT Msg);
#endif /* RTMP_MAC_USB */

VOID  ComposePsPoll(
	IN  struct rtmp_adapter *pAd);

VOID  ComposeNullFrame(
	IN  struct rtmp_adapter *pAd);

VOID  AssocPostProc(
	IN  struct rtmp_adapter *pAd,
	IN  PUCHAR pAddr2,
	IN  USHORT CapabilityInfo,
	IN  USHORT Aid,
	IN  UCHAR SupRate[],
	IN  UCHAR SupRateLen,
	IN  UCHAR ExtRate[],
	IN  UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN IE_LISTS *ie_list,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN  UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo);

VOID AuthStateMachineInit(
	IN  struct rtmp_adapter *pAd,
	IN PSTATE_MACHINE sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeAuthReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq2Action(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq4Action(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AuthTimeoutAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID Cls2errAction(
	IN  struct rtmp_adapter *pAd,
	IN  PUCHAR pAddr);

VOID MlmeDeauthReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAuth(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

/*============================================= */

VOID AuthRspStateMachineInit(
	IN  struct rtmp_adapter *pAd,
	IN  PSTATE_MACHINE Sm,
	IN  STATE_MACHINE_FUNC Trans[]);

VOID PeerDeauthAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAuthSimpleRspGenAndSend(
	IN  struct rtmp_adapter *pAd,
	IN  PHEADER_802_11  pHdr80211,
	IN  USHORT Alg,
	IN  USHORT Seq,
	IN  USHORT Reason,
	IN  USHORT Status);

/* */
/* Private routines in dls.c */
/* */
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */


BOOLEAN PeerProbeReqSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PEER_PROBE_REQ_PARAM *Param);

/*======================================== */

VOID SyncStateMachineInit(
	IN  struct rtmp_adapter *pAd,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID BeaconTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeScanReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenScan(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenJoin(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenStart(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID EnqueueProbeRequest(
	IN struct rtmp_adapter *pAd);

BOOLEAN ScanRunning(
		IN struct rtmp_adapter *pAd);
/*========================================= */

VOID MlmeCntlInit(
	IN  struct rtmp_adapter *pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeCntlMachinePerformAction(
	IN  struct rtmp_adapter *pAd,
	IN  STATE_MACHINE *S,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlIdleProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidScanProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidSsidProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlOidRTBssidProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlMlmeRoamingProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlWaitDisassocProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitJoinProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitReassocProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitStartProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc2(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAssocProc(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);



VOID LinkUp(
	IN  struct rtmp_adapter *pAd,
	IN  UCHAR BssType);

VOID LinkDown(
	IN  struct rtmp_adapter *pAd,
	IN  BOOLEAN         IsReqFromAP);

VOID IterateOnBssTab(
	IN  struct rtmp_adapter *pAd);

VOID IterateOnBssTab2(
	IN  struct rtmp_adapter *pAd);;

VOID JoinParmFill(
	IN  struct rtmp_adapter *pAd,
	IN  OUT MLME_JOIN_REQ_STRUCT *JoinReq,
	IN  ULONG BssIdx);

VOID AssocParmFill(
	IN  struct rtmp_adapter *pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN  PUCHAR pAddr,
	IN  USHORT CapabilityInfo,
	IN  ULONG Timeout,
	IN  USHORT ListenIntv);

VOID ScanParmFill(
	IN  struct rtmp_adapter *pAd,
	IN  OUT MLME_SCAN_REQ_STRUCT *ScanReq,
	IN  STRING Ssid[],
	IN  UCHAR SsidLen,
	IN  UCHAR BssType,
	IN  UCHAR ScanType);

VOID DisassocParmFill(
	IN  struct rtmp_adapter *pAd,
	IN  OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN  PUCHAR pAddr,
	IN  USHORT Reason);

VOID StartParmFill(
	IN  struct rtmp_adapter *pAd,
	IN  OUT MLME_START_REQ_STRUCT *StartReq,
	IN  CHAR Ssid[],
	IN  UCHAR SsidLen);

VOID AuthParmFill(
	IN  struct rtmp_adapter *pAd,
	IN  OUT MLME_AUTH_REQ_STRUCT *AuthReq,
	IN  PUCHAR pAddr,
	IN  USHORT Alg);

VOID EnqueuePsPoll(
	IN  struct rtmp_adapter *pAd);

VOID EnqueueBeaconFrame(
	IN  struct rtmp_adapter *pAd);

VOID MlmeJoinReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeScanReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeStartReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeForceJoinReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeForceScanReqAction(
	IN struct rtmp_adapter *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ScanTimeoutAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID BeaconTimeoutAtJoinAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtScanAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtJoinAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerProbeReqAction(
	IN  struct rtmp_adapter *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID ScanNextChannel(
	IN  struct rtmp_adapter *pAd,
	IN	UCHAR	OpMode);


ULONG MakeIbssBeacon(
	IN  struct rtmp_adapter *pAd);

BOOLEAN MlmeScanReqSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT UCHAR *BssType,
	OUT CHAR ssid[],
	OUT UCHAR *SsidLen,
	OUT UCHAR *ScanType);


BOOLEAN PeerBeaconAndProbeRspSanity_Old(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	IN  UCHAR MsgChannel,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pBssid,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pBssType,
	OUT USHORT *pBeaconPeriod,
	OUT UCHAR *pChannel,
	OUT UCHAR *pNewChannel,
	OUT LARGE_INTEGER *pTimestamp,
	OUT CF_PARM *pCfParm,
	OUT USHORT *pAtimWin,
	OUT USHORT *pCapabilityInfo,
	OUT UCHAR *pErp,
	OUT UCHAR *pDtimCount,
	OUT UCHAR *pDtimPeriod,
	OUT UCHAR *pBcastFlag,
	OUT UCHAR *pMessageToMe,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT	UCHAR *pCkipFlag,
	OUT	UCHAR *pAironetCellPowerLimit,
	OUT PEDCA_PARM       pEdcaParm,
	OUT PQBSS_LOAD_PARM  pQbssLoad,
	OUT PQOS_CAPABILITY_PARM pQosCapability,
	OUT ULONG *pRalinkIe,
	OUT UCHAR		 *pHtCapabilityLen,
#ifdef CONFIG_STA_SUPPORT
	OUT UCHAR		 *pPreNHtCapabilityLen,
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
    OUT UCHAR 	*pSelReg,
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR		 *AddHtInfoLen,
	OUT ADD_HT_INFO_IE *AddHtInfo,
	OUT UCHAR *NewExtChannel,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE);


BOOLEAN PeerBeaconAndProbeRspSanity(
	IN struct rtmp_adapter *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN UCHAR  MsgChannel,
	OUT BCN_IE_LIST *ie_list,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE);


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN struct rtmp_adapter *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN OVERLAP_BSS_SCAN_IE *BssScan,
	OUT UCHAR 	*RegClass);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

BOOLEAN PeerAddBAReqActionSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
	OUT PUCHAR pAddr2);

BOOLEAN PeerAddBARspActionSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(
    IN struct rtmp_adapter *pAd,
    IN UCHAR Wcid,
    IN VOID *pMsg,
    IN ULONG MsgLen);

BOOLEAN MlmeAssocReqSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pApAddr,
	OUT USHORT *CapabilityInfo,
	OUT ULONG *Timeout,
	OUT USHORT *ListenIntv);

BOOLEAN MlmeAuthReqSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT ULONG *Timeout,
	OUT USHORT *Alg);

BOOLEAN MlmeStartReqSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT CHAR Ssid[],
	OUT UCHAR *Ssidlen);

BOOLEAN PeerAuthSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT USHORT *Alg,
	OUT USHORT *Seq,
	OUT USHORT *Status,
	OUT CHAR ChlgText[]);

BOOLEAN PeerAssocRspSanity(
	IN  struct rtmp_adapter *pAd,
    IN VOID *pMsg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pCapabilityInfo,
	OUT USHORT *pStatus,
	OUT USHORT *pAid,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE		*pHtCapability,
    OUT ADD_HT_INFO_IE		*pAddHtInfo,	/* AP might use this additional ht info IE */
    OUT UCHAR			*pHtCapabilityLen,
    OUT UCHAR			*pAddHtInfoLen,
    OUT UCHAR			*pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR *pCkipFlag,
	OUT IE_LISTS *ie_list);

BOOLEAN PeerDisassocSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *Reason);

BOOLEAN PeerDeauthSanity(
	IN  struct rtmp_adapter *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr1,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pAddr3,
	OUT USHORT *Reason);

BOOLEAN GetTimBit(
	IN  CHAR *Ptr,
	IN  USHORT Aid,
	OUT UCHAR *TimLen,
	OUT UCHAR *BcastFlag,
	OUT UCHAR *DtimCount,
	OUT UCHAR *DtimPeriod,
	OUT UCHAR *MessageToMe);

UCHAR ChannelSanity(
	IN struct rtmp_adapter *pAd,
	IN UCHAR channel);

NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
	IN BSS_ENTRY *pBss);

BOOLEAN MlmeDelBAReqSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen);

BOOLEAN MlmeAddBAReqSanity(
    IN struct rtmp_adapter *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2);

ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer,
	OUT ULONG *Length, ...);

UCHAR RandomByte(
	IN  struct rtmp_adapter *pAd);

UCHAR RandomByte2(
	IN  struct rtmp_adapter *pAd);

VOID AsicUpdateAutoFallBackTable(struct rtmp_adapter *pAd, UCHAR *pTxRate);
INT AsicSetAutoFallBack(struct rtmp_adapter *pAd, BOOLEAN enable);
INT AsicAutoFallbackInit(struct rtmp_adapter *pAd);



VOID  MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkDownExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID STAMlmePeriodicExec(
	struct rtmp_adapter *pAd);

VOID MlmeAutoScan(
	IN struct rtmp_adapter *pAd);

VOID MlmeAutoReconnectLastSSID(
	IN struct rtmp_adapter *pAd);

BOOLEAN MlmeValidateSSID(
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen);

VOID MlmeCheckForRoaming(
	IN struct rtmp_adapter *pAd,
	IN ULONG    Now32);

BOOLEAN MlmeCheckForFastRoaming(
	IN  struct rtmp_adapter *pAd);

#ifdef TXBF_SUPPORT
BOOLEAN MlmeTxBfAllowed(
	IN struct rtmp_adapter *		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN struct _RTMP_RA_LEGACY_TB *pTxRate);
#endif /* TXBF_SUPPORT */

VOID MlmeCalculateChannelQuality(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now);

VOID MlmeCheckPsmChange(
	IN struct rtmp_adapter *pAd,
	IN ULONG    Now32);

VOID MlmeSetPsmBit(
	IN struct rtmp_adapter *pAd,
	IN USHORT psm);

VOID MlmeSetTxPreamble(
	IN struct rtmp_adapter *pAd,
	IN USHORT TxPreamble);

VOID UpdateBasicRateBitmap(
	IN	struct rtmp_adapter *pAd);

VOID MlmeUpdateTxRates(
	IN struct rtmp_adapter *pAd,
	IN 	BOOLEAN		 	bLinkUp,
	IN	UCHAR			apidx);

#ifdef DOT11_N_SUPPORT
VOID MlmeUpdateHtTxRates(
	IN struct rtmp_adapter *		pAd,
	IN	UCHAR				apidx);
#endif /* DOT11_N_SUPPORT */

VOID    RTMPCheckRates(
	IN      struct rtmp_adapter *pAd,
	IN OUT  UCHAR           SupRate[],
	IN OUT  UCHAR           *SupRateLen);

BOOLEAN RTMPCheckHt(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Wcid,
	INOUT HT_CAPABILITY_IE *pHtCapability,
	INOUT ADD_HT_INFO_IE *pAddHtInfo);

#ifdef DOT11_VHT_AC
BOOLEAN RTMPCheckVht(
	IN struct rtmp_adapter *pAd,
	IN UCHAR Wcid,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE *vht_op);
#endif /* DOT11_VHT_AC */

VOID RTMPUpdateMlmeRate(
	IN struct rtmp_adapter *pAd);

CHAR RTMPMaxRssi(
	IN struct rtmp_adapter *pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

CHAR RTMPAvgRssi(
        IN struct rtmp_adapter *pAd,
        IN RSSI_SAMPLE		*pRssi);


CHAR RTMPMinSnr(
	IN struct rtmp_adapter *pAd,
	IN CHAR				Snr0,
	IN CHAR				Snr1);

VOID AsicSetRxAnt(
	IN struct rtmp_adapter *pAd,
	IN UCHAR			Ant);

#ifdef MICROWAVE_OVEN_SUPPORT
INT Set_MO_FalseCCATh_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

VOID AsicMeasureFalseCCA(
	IN struct rtmp_adapter *pAd
);

VOID AsicMitigateMicrowave(
	IN struct rtmp_adapter *pAd
);
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
INT set_eFuseGetFreeBlockCount_Proc(
   	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT set_eFusedump_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT set_eFuseLoadFromBin_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

UCHAR eFuseReadRegisters(
	IN	struct rtmp_adapter *pAd,
	IN	USHORT			Offset,
	IN	USHORT			Length,
	OUT	USHORT*			pData);

VOID eFusePhysicalReadRegisters(
	IN struct rtmp_adapter *pAd,
	IN USHORT 			Offset,
	IN USHORT 			Length,
	OUT	USHORT			*pData);

int RtmpEfuseSupportCheck(
	IN struct rtmp_adapter *pAd);

VOID eFuseGetFreeBlockCount(
	IN struct rtmp_adapter *	pAd,
	INOUT PUINT 			EfuseFreeBlock);

INT eFuse_init(
	IN struct rtmp_adapter *	pAd);

INT efuse_probe(
	IN struct rtmp_adapter 	*pAd);

NTSTATUS eFuseRead(
	IN struct rtmp_adapter *pAd,
	IN USHORT			Offset,
	OUT	PUSHORT			pData,
	IN USHORT			Length);

NTSTATUS eFuseWrite(
   	IN struct rtmp_adapter *pAd,
	IN USHORT			Offset,
	IN PUSHORT			pData,
	IN USHORT			length);

#ifdef RALINK_ATE
INT Set_LoadEepromBufferFromEfuse_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT set_eFuseBufferModeWriteBack_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

INT set_BinModeWriteBack_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *		arg);

#endif /* RALINK_ATE */
#endif /* RTMP_EFUSE_SUPPORT */




VOID AsicEvaluateRxAnt(
	IN struct rtmp_adapter *pAd);

VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID RTMPSetPiggyBack(
	IN struct rtmp_adapter *pAd,
	IN BOOLEAN			bPiggyBack);

BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

UCHAR RTMPStaFixedTxMode(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

VOID RTMPUpdateLegacyTxSetting(
		UCHAR				fixed_tx_mode,
		PMAC_TABLE_ENTRY	pEntry);

BOOLEAN RTMPAutoRateSwitchCheck(
	IN struct rtmp_adapter *pAd);


#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
VOID InitFrequencyCalibration(struct rtmp_adapter *pAd);
VOID StopFrequencyCalibration(struct rtmp_adapter *pAd);
VOID FrequencyCalibration(struct rtmp_adapter *pAd);
CHAR GetFrequencyOffset(struct rtmp_adapter *pAd, RXWI_STRUC *pRxWI);
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef RTMP_TEMPERATURE_COMPENSATION
VOID InitLookupTable(struct rtmp_adapter *pAd);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

VOID MlmeHalt(struct rtmp_adapter *pAd);
int MlmeInit(struct rtmp_adapter *pAd);

VOID MlmeResetRalinkCounters(struct rtmp_adapter *pAd);

VOID BuildChannelList(struct rtmp_adapter *pAd);
UCHAR FirstChannel(struct rtmp_adapter *pAd);
UCHAR NextChannel(struct rtmp_adapter *pAd, UCHAR channel);

UCHAR RTMPFindScanChannel(
	IN struct rtmp_adapter *pAd,
	IN UINT8 LastScanChannel);

VOID ChangeToCellPowerLimit(struct rtmp_adapter *pAd, UCHAR AironetCellPowerLimit);


VOID    RTMPInitMICEngine(
	IN  struct rtmp_adapter *pAd,
	IN  PUCHAR          pKey,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  UCHAR           UserPriority,
	IN  PUCHAR          pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
	IN  struct rtmp_adapter *pAd,
	IN  PUCHAR          pSrc,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  PUCHAR          pMICKey,
	IN	UCHAR			UserPriority,
	IN  UINT            Len);

VOID    RTMPCalculateMICValue(
	IN  struct rtmp_adapter *pAd,
	IN  struct sk_buff *   pPacket,
	IN  PUCHAR          pEncap,
	IN  PCIPHER_KEY     pKey,
	IN	UCHAR			apidx);

VOID    RTMPTkipAppendByte(
	IN  PTKIP_KEY_INFO  pTkip,
	IN  UCHAR           uChar);

VOID    RTMPTkipAppend(
	IN  PTKIP_KEY_INFO  pTkip,
	IN  PUCHAR          pSrc,
	IN  UINT            nBytes);

VOID RTMPTkipGetMIC(TKIP_KEY_INFO *pTkip);


INT RT_CfgSetCountryRegion(
	IN struct rtmp_adapter *pAd,
	IN char *arg,
	IN INT band);

INT RT_CfgSetWirelessMode(struct rtmp_adapter *pAd, char *arg);

RT_802_11_PHY_MODE wmode_2_cfgmode(UCHAR wmode);
UCHAR cfgmode_2_wmode(UCHAR cfg_mode);
UCHAR *wmode_2_str(UCHAR wmode);

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
INT RT_CfgSetMbssWirelessMode(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT RT_CfgSetShortSlot(
	IN	struct rtmp_adapter *pAd,
	IN	char *		arg);

INT	RT_CfgSetWepKey(
	IN	struct rtmp_adapter *pAd,
	IN	char *		keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx);

INT RT_CfgSetWPAPSKKey(
	IN struct rtmp_adapter *pAd,
	IN char *	keyString,
	IN INT			keyStringLen,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf);

INT	RT_CfgSetFixedTxPhyMode(char *arg);
INT	RT_CfgSetMacAddress(struct rtmp_adapter *pAd, char *arg);
INT	RT_CfgSetTxMCSProc(char *arg, BOOLEAN *pAutoRate);
INT	RT_CfgSetAutoFallBack(struct rtmp_adapter *pAd, char *arg);


INT	Set_Antenna_Proc(struct rtmp_adapter *pAd, char *arg);



#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);

INT	Set_MultiMacAddrExt_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	char *arg);
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

INT set_tssi_enable(struct rtmp_adapter *pAd, char *arg);


#ifdef CONFIG_ANDES_SUPPORT
INT set_fw_debug(struct rtmp_adapter *pAd, char *arg);
#endif /* CONFIG_ANDES_SUPPORT */

INT set_cal_test(struct rtmp_adapter *pAd, char *arg);

int RTMPWPARemoveKeyProc(
	IN  struct rtmp_adapter *pAd,
	IN  PVOID           pBuf);

VOID RTMPWPARemoveAllKeys(
	IN  struct rtmp_adapter *pAd);

BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr,
    IN  UCHAR strLen);

VOID RTMPSetPhyMode(
	IN  struct rtmp_adapter *pAd,
	IN  ULONG phymode);

VOID RTMPUpdateHTIE(
	IN	RT_HT_CAPABILITY	*pRtHt,
	IN		UCHAR				*pMcsSet,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE		*pAddHtInfo);

VOID RTMPAddWcidAttributeEntry(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN 	MAC_TABLE_ENTRY *pEntry);

char *GetEncryptType(CHAR enc);
char *GetAuthMode(CHAR auth);

#ifdef DOT11_N_SUPPORT
VOID RTMPSetHT(
	IN	struct rtmp_adapter *pAd,
	IN	OID_SET_HT_PHYMODE *pHTPhyMode);

VOID RTMPSetIndividualHT(struct rtmp_adapter *pAd, UCHAR apidx);

UCHAR get_cent_ch_by_htinfo(
	struct rtmp_adapter *pAd,
	ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap);

INT get_ht_cent_ch(struct rtmp_adapter *pAd, UINT8 *rf_bw, UINT8 *ext_ch);
INT ht_mode_adjust(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer, RT_HT_CAPABILITY *my);
INT set_ht_fixed_mcs(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound);
INT get_ht_max_mcs(struct rtmp_adapter *pAd, UCHAR *desire_mcs, UCHAR *cap_mcs);
#endif /* DOT11_N_SUPPORT */

VOID RTMPDisableDesiredHtInfo(
	IN	struct rtmp_adapter *pAd);

#ifdef SYSTEM_LOG_SUPPORT
VOID RtmpDrvSendWirelessEvent(
	IN	struct rtmp_adapter	*pAdSrc,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN  UCHAR			BssIdx,
	IN	CHAR			Rssi);
#else
#define RtmpDrvSendWirelessEvent(_pAd, _Event_flag, _pAddr, _BssIdx, _Rssi)
#endif /* SYSTEM_LOG_SUPPORT */

CHAR    ConvertToRssi(
	IN struct rtmp_adapter * pAd,
	IN CHAR				Rssi,
	IN UCHAR    RssiNumber);

CHAR ConvertToSnr(struct rtmp_adapter *pAd, UCHAR Snr);

#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN struct rtmp_adapter *pAd);


VOID DeleteEffectedChannelList(struct rtmp_adapter *pAd);

VOID CntlChannelWidth(
	IN struct rtmp_adapter *pAd,
	IN UCHAR			PrimaryChannel,
	IN UCHAR			CentralChannel,
	IN UCHAR			ChannelWidth,
	IN UCHAR			SecondaryChannelOffset);

#endif /* DOT11N_DRAFT3 */


VOID APAsicEvaluateRxAnt(
	IN struct rtmp_adapter *pAd);


VOID APAsicRxAntEvalTimeout(
	IN struct rtmp_adapter *pAd);


VOID RTMPGetTxTscFromAsic(struct rtmp_adapter *pAd, UCHAR apidx, UCHAR *pTxTsc);

MAC_TABLE_ENTRY *PACInquiry(struct rtmp_adapter *pAd, UCHAR Wcid);

UINT APValidateRSNIE(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR			pRsnIe,
	IN UCHAR			rsnie_len);

VOID HandleCounterMeasure(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY  *pEntry);

VOID WPAStart4WayHS(
	IN  struct rtmp_adapter *pAd,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	ULONG			TimeInterval);

VOID WPAStart2WayGroupHS(
	IN  struct rtmp_adapter *pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID PeerPairMsg1Action(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg2Action(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg3Action(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg4Action(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerGroupMsg1Action(
	IN  struct rtmp_adapter *pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
    IN  MLME_QUEUE_ELEM  *Elem);

VOID PeerGroupMsg2Action(
	IN  struct rtmp_adapter *pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  VOID             *Msg,
	IN  UINT             MsgLen);

VOID CMTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WPARetryExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#ifdef TXBF_SUPPORT
VOID eTxBfProbeTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* TXBF_SUPPORT */

VOID EnqueueStartForPSKExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID RTMPHandleSTAKey(
    IN struct rtmp_adapter *pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID MlmeDeAuthAction(
	IN  struct rtmp_adapter *pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  USHORT           Reason,
	IN  BOOLEAN          bDataFrameFirst);

#ifdef DOT11W_PMF_SUPPORT
VOID PMF_SAQueryTimeOut(
        IN PVOID SystemSpecific1,
        IN PVOID FunctionContext,
        IN PVOID SystemSpecific2,
        IN PVOID SystemSpecific3);

VOID PMF_SAQueryConfirmTimeOut(
        IN PVOID SystemSpecific1,
        IN PVOID FunctionContext,
        IN PVOID SystemSpecific2,
        IN PVOID SystemSpecific3);
#endif /* DOT11W_PMF_SUPPORT */

VOID GREKEYPeriodicExec(
	IN  PVOID   SystemSpecific1,
	IN  PVOID   FunctionContext,
	IN  PVOID   SystemSpecific2,
	IN  PVOID   SystemSpecific3);

VOID AES_128_CMAC(
	IN	PUCHAR	key,
	IN	PUCHAR	input,
	IN	INT		len,
	OUT	PUCHAR	mac);

#ifdef DOT1X_SUPPORT
VOID    WpaSend(
    IN  struct rtmp_adapter *pAd,
    IN  PUCHAR          pPacket,
    IN  ULONG           Len);

VOID RTMPAddPMKIDCache(
	IN  struct rtmp_adapter *pAd,
	IN	INT						apidx,
	IN	PUCHAR				pAddr,
	IN	UCHAR					*PMKID,
	IN	UCHAR					*PMK);

INT RTMPSearchPMKIDCache(
	IN  struct rtmp_adapter *pAd,
	IN	INT				apidx,
	IN	PUCHAR		pAddr);

VOID RTMPDeletePMKIDCache(
	IN  struct rtmp_adapter *pAd,
	IN	INT				apidx,
	IN  INT				idx);

VOID RTMPMaintainPMKIDCache(
	IN  struct rtmp_adapter *pAd);
#else
#define RTMPMaintainPMKIDCache(_pAd)
#endif /* DOT1X_SUPPORT */

#ifdef RESOURCE_PRE_ALLOC
VOID RTMPResetTxRxRingMemory(
	IN  struct rtmp_adapter   *pAd);
#endif /* RESOURCE_PRE_ALLOC */

VOID RTMPFreeTxRxRingMemory(
    IN  struct rtmp_adapter *pAd);

BOOLEAN RTMP_FillTxBlkInfo(
	IN struct rtmp_adapter *pAd,
	IN TX_BLK *pTxBlk);

 void announce_802_3_packet(
	IN	struct rtmp_adapter	*pAdSrc,
	IN	struct sk_buff *pPacket,
	IN	UCHAR			OpMode);

#ifdef DOT11_N_SUPPORT
UINT BA_Reorder_AMSDU_Annnounce(
	IN	struct rtmp_adapter *pAd,
	IN	struct sk_buff *pPacket,
	IN	UCHAR			OpMode);
#endif /* DOT11_N_SUPPORT */

struct net_device *get_netdev_from_bssid(struct rtmp_adapter *pAd, UCHAR FromWhichBSSID);

#ifdef DOT11_N_SUPPORT
void ba_flush_reordering_timeout_mpdus(
	IN struct rtmp_adapter *pAd,
	IN PBA_REC_ENTRY	pBAEntry,
	IN ULONG			Now32);


VOID BAOriSessionSetUp(
			IN struct rtmp_adapter *pAd,
			IN MAC_TABLE_ENTRY	*pEntry,
			IN UCHAR			TID,
			IN USHORT			TimeOut,
			IN ULONG			DelayTime,
			IN BOOLEAN		isForced);

VOID BASessionTearDownALL(
	IN OUT	struct rtmp_adapter *pAd,
	IN		UCHAR Wcid);

VOID BAOriSessionTearDown(
	IN OUT	struct rtmp_adapter *pAd,
	IN		UCHAR			Wcid,
	IN		UCHAR			TID,
	IN		BOOLEAN			bPassive,
	IN		BOOLEAN			bForceSend);

VOID BARecSessionTearDown(
	IN OUT	struct rtmp_adapter *pAd,
	IN		UCHAR			Wcid,
	IN		UCHAR			TID,
	IN		BOOLEAN			bPassive);
#endif /* DOT11_N_SUPPORT */

BOOLEAN ba_reordering_resource_init(struct rtmp_adapter *pAd, int num);
void ba_reordering_resource_release(struct rtmp_adapter *pAd);

INT ComputeChecksum(
	IN UINT PIN);

UINT GenerateWpsPinCode(
	IN	struct rtmp_adapter *pAd,
    IN  BOOLEAN         bFromApcli,
	IN	UCHAR	apidx);





BOOLEAN rtstrmactohex(char *s1, char *s2);
BOOLEAN rtstrcasecmp(char *s1, char *s2);
char *rtstrstruncasecmp(char *s1, char *s2);

char *rtstrstr( const char *s1, const char *s2);
char *rstrtok( char *s, const char *ct);
int rtinet_aton(const char *cp, unsigned int *addr);

/*//////// common ioctl functions ////////*/
INT Set_DriverVersion_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_CountryRegion_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_CountryRegionABand_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_WirelessMode_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_MBSS_WirelessMode_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_Channel_Proc(struct rtmp_adapter *pAd, char *arg);
#ifdef RT_CFG80211_SUPPORT
INT Set_DisableCfg2040Scan_Proc(struct rtmp_adapter *pAd, char *arg);
#endif
INT	Set_ShortSlot_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_TxPower_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_BGProtection_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_TxPreamble_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_RTSThreshold_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_FragThreshold_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_TxBurst_Proc(struct rtmp_adapter *pAd, char *arg);


#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* AGGREGATION_SUPPORT */

#ifdef INF_PPA_SUPPORT
INT	Set_INF_AMAZON_SE_PPA_Proc(struct rtmp_adapter *pAd, char *arg);

INT ifx_ra_start_xmit (
	IN	struct net_device *rx_dev,
	IN	struct net_device *tx_dev,
	IN	struct sk_buff *skb,
	IN	int len);
#endif /* INF_PPA_SUPPORT */

INT	Set_IEEE80211H_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ExtCountryCode_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ExtDfsType_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ChannelListAdd_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ChannelListShow_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ChannelListDel_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DBG
INT	Set_Debug_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_DebugFunc_Proc(struct rtmp_adapter *pAd, char *arg);
#endif

#ifdef TXBF_SUPPORT
INT	Set_ReadITxBf_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_ReadETxBf_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_WriteITxBf_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_WriteETxBf_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_StatITxBf_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_StatETxBf_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_TxBfTag_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ITxBfTimeout_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ETxBfTimeout_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_InvTxBfTag_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_ITxBfCal_Proc(struct rtmp_adapter *pAd, char *arg);
#ifdef MT76x2
INT	mt76x2_Set_ITxBfCal_Proc(struct rtmp_adapter *pAd, char *arg);
#endif
INT	Set_ITxBfDivCal_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_ITxBfLnaCal_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_ETxBfEnCond_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ETxBfCodebook_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ETxBfCoefficient_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ETxBfGrouping_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ETxBfNoncompress_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ETxBfIncapable_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_StaETxBfEnCond_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_NoSndgCntThrd_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_NdpSndgStreams_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_Trigger_Sounding_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_ITxBfEn_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef MT76x2
INT Set_TxBfProfileTag_Help(
    IN  struct rtmp_adapter *  pAd,
	IN  char *        arg);

INT Set_TxBfProfileTagValid(
	IN  struct rtmp_adapter *  pAd,
	IN  char *        arg);

INT Set_TxBfProfileTagRead(
    IN  struct rtmp_adapter *  pAd,
	IN  char *        arg);

INT Set_TxBfProfileTagWrite(
    IN  struct rtmp_adapter *  pAd,
	IN  char *        arg);

INT Set_TxBfProfileDataRead(
    IN struct rtmp_adapter *   pAd,
	IN char *         arg);

INT Set_TxBfProfileDataWrite(
    IN struct rtmp_adapter *   pAd,
	IN char *         arg);

INT Set_TxBfProfileDataWriteAll(
    IN struct rtmp_adapter *   pAd,
	IN char *         arg);

INT Set_TxBfProfileDataReadAll(
    IN struct rtmp_adapter *   pAd,
	IN char *         arg);

INT Set_TxBfProfileTag_TimeOut(
    IN struct rtmp_adapter *   pAd,
	IN char *         arg);


INT Set_TxBfProfileTag_Matrix(
    IN struct rtmp_adapter *   pAd,
	IN char *         arg);


INT Set_TxBfProfileTag_SNR(
    IN struct rtmp_adapter *  pAd,
	IN char *        arg);


INT Set_TxBfProfileTag_TxScale(
    IN struct rtmp_adapter *  pAd,
	IN char *        arg);


INT Set_TxBfProfileTag_MAC(
    IN struct rtmp_adapter *  pAd,
	IN char *        arg);

INT Set_TxBfProfileTag_Flg(
    IN struct rtmp_adapter *  pAd,
	IN char *        arg);
#endif

#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
INT Set_VhtNDPA_Sounding_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* VHT_TXBF_SUPPORT */


INT Set_RateAdaptInterval(struct rtmp_adapter *pAd, char *arg);


#ifdef PRE_ANT_SWITCH
INT Set_PreAntSwitch_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_PreAntSwitchRSSI_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_PreAntSwitchTimeout_Proc(struct rtmp_adapter *pAd, char *arg);

#endif /* PRE_ANT_SWITCH */



#ifdef CFO_TRACK
INT Set_CFOTrack_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef CFO_TRACK
#ifdef CONFIG_AP_SUPPORT
INT rtmp_cfo_track(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, INT lastClient);
#endif /* CONFIG_AP_SUPPORT */
#endif /* CFO_TRACK */

#endif // CFO_TRACK //

#ifdef DBG_CTRL_SUPPORT
INT Set_DebugFlags_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef INCLUDE_DEBUG_QUEUE
INT Set_DebugQueue_Proc(struct rtmp_adapter *pAd, char *arg);
void dbQueueEnqueue(UCHAR type, UCHAR *data);
void dbQueueEnqueueTxFrame(UCHAR *pTxWI, UCHAR *pDot11Hdr);
void dbQueueEnqueueRxFrame(UCHAR *pRxWI, UCHAR *pDot11Hdr ULONG flags);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

INT Show_DescInfo_Proc(struct rtmp_adapter *pAd, char *arg);
INT Show_MacTable_Proc(struct rtmp_adapter *pAd, char *arg);
INT Show_sta_tr_proc(struct rtmp_adapter *pAd, char *arg);
INT show_stainfo_proc(struct rtmp_adapter *pAd, char *arg);
INT show_devinfo_proc(struct rtmp_adapter *pAd, char *arg);
INT show_sysinfo_proc(struct rtmp_adapter *pAd, char *arg);
INT show_trinfo_proc(struct rtmp_adapter *pAd, char *arg);
INT Show_TxInfo_Proc(struct rtmp_adapter *pAd, char *arg);

INT	Set_ResetStatCounter_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_BADecline_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_BAOriTearDown_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_BARecTearDown_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtBw_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtMcs_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtGi_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtOpMode_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtStbc_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtHtc_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtExtcha_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtMpduDensity_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtBaWinSize_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtRdg_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtLinkAdapt_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtAmsdu_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtAutoBa_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtProtect_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtMimoPs_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_HT_BssCoexApCntThr_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* DOT11N_DRAFT3 */


#ifdef CONFIG_AP_SUPPORT
INT	Set_HtTxStream_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtRxStream_Proc(struct rtmp_adapter *pAd, char *arg);
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT	SetCommonHT(struct rtmp_adapter *pAd);

INT	Set_ForceShortGI_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_ForceGF_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_SendSMPSAction_Proc(struct rtmp_adapter *pAd, char *arg);

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	IN struct rtmp_adapter *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID);

INT	Set_HtMIMOPSmode_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtTxBASize_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_HtDisallowTKIP_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_BurstMode_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
INT Set_VhtBw_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_VhtStbc_Proc(struct rtmp_adapter *pAd, char *arg);
INT set_VhtBwSignal_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_VhtDisallowNonVHT_Proc(
	IN struct rtmp_adapter *pAd,
	IN char *arg);
#endif /* DOT11_VHT_AC */


#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(struct rtmp_adapter *pAd, char *arg);
#endif /*APCLI_SUPPORT*/




#ifdef CONFIG_STA_SUPPORT
VOID RTMPSendDLSTearDownFrame(struct rtmp_adapter *pAd, UCHAR *pDA);

#ifdef DOT11_N_SUPPORT
VOID QueryBATABLE(
	IN  struct rtmp_adapter *pAd,
	OUT PQUERYBA_TABLE pBAT);
#endif /* DOT11_N_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
INT	    WpaCheckEapCode(
	IN  struct rtmp_adapter *  	pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet);
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID detect_wmm_traffic(struct rtmp_adapter *pAd, UCHAR up, UCHAR bOutput);
VOID dynamic_tune_be_tx_op(struct rtmp_adapter *pAd, ULONG nonBEpackets);
#endif /* CONFIG_AP_SUPPORT */


#ifdef DOT11_N_SUPPORT
VOID Handle_BSS_Width_Trigger_Events(struct rtmp_adapter *pAd);

void build_ext_channel_switch_ie(
	IN struct rtmp_adapter *pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE);

void assoc_ht_info_debugshow(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR ht_cap_len,
	IN HT_CAPABILITY_IE *pHTCapability);
#endif /* DOT11_N_SUPPORT */

BOOLEAN rtmp_rx_done_handle(struct rtmp_adapter *pAd);


#ifdef DOT11_N_SUPPORT
VOID Indicate_AMPDU_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
VOID Indicate_AMPDU_Packet_Hdr_Trns(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);
#endif /* HDR_TRANS_SUPPORT */

/* AMSDU packet indication */
VOID Indicate_AMSDU_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID BaReOrderingBufferMaintain(
    IN struct rtmp_adapter *pAd);
#endif /* DOT11_N_SUPPORT */

/* Normal legacy Rx packet indication */
VOID Indicate_Legacy_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
VOID Indicate_Legacy_Packet_Hdr_Trns(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);
#endif /* HDR_TRANS_SUPPORT */

VOID Indicate_EAPOL_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

UINT deaggregate_AMSDU_announce(
	IN	struct rtmp_adapter *pAd,
	IN	RX_BLK			*pRxBlk,
	struct sk_buff *	pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			OpMode);

#ifdef TXBF_SUPPORT
BOOLEAN clientSupportsETxBF(struct rtmp_adapter *pAd, HT_BF_CAP *pTxBFCap);
void setETxBFCap(struct rtmp_adapter *pAd, HT_BF_CAP *pTxBFCap);
#ifdef VHT_TXBF_SUPPORT
BOOLEAN clientSupportsVHTETxBF(struct rtmp_adapter *pAd, VHT_CAP_INFO *pTxBFCapInfo);
void setVHTETxBFCap(struct rtmp_adapter *pAd, VHT_CAP_INFO *pTxBFCap);
#endif /* VHT_TXBF_SUPPORT */

#ifdef ETXBF_EN_COND3_SUPPORT
VOID txSndgSameMcs(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY * pEnt, UCHAR smoothMfb);
VOID txSndgOtherGroup(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry);
VOID txMrqInvTxBF(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry);
VOID chooseBestMethod(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR mfb);
VOID rxBestSndg(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* ETXBF_EN_COND3_SUPPORT */

VOID handleBfFb(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);

VOID TxBFInit(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN supETxBF);

VOID eTxBFProbing(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry);

VOID Trigger_Sounding_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	UCHAR			SndgType,
	IN	UCHAR			SndgBW,
	IN	UCHAR			SndgMcs,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID rtmp_asic_set_bf(struct rtmp_adapter *pAd);
BOOLEAN rtmp_chk_itxbf_calibration(struct rtmp_adapter *pAd);

#endif /* TXBF_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* remove LLC and get 802_3 Header */
#define  RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
{																						\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;                                 						\
																				\
																				\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH)) 		\
	{                                                                           											\
		_pDA = _pRxBlk->pHeader->Addr3;                                         							\
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);                					\
	}                                                                           											\
	else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_APCLI))										\
	{																					\
		_pDA = _pRxBlk->pHeader->Addr1; 													\
		_pSA = _pRxBlk->pHeader->Addr3;													\
	}																					\
	else                                                                        										\
	{                                                                           											\
		_pDA = _pRxBlk->pHeader->Addr3;                                         							\
		_pSA = _pRxBlk->pHeader->Addr2;                                         							\
	}                                                                           											\
																				\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData, 						\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);                                   						\
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
/* remove LLC and get 802_3 Header */
#define  RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
{																				\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;                                 \
																				\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH)) \
	{                                                                           \
		_pDA = _pRxBlk->pHeader->Addr3;                                         \
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);                \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_INFRA))                              	\
		{                                                                       \
			_pDA = _pRxBlk->pHeader->Addr1;                                     \
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_DLS))									\
			_pSA = _pRxBlk->pHeader->Addr2;										\
		else																	\
			_pSA = _pRxBlk->pHeader->Addr3;                                     \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			_pDA = _pRxBlk->pHeader->Addr1;                                     \
			_pSA = _pRxBlk->pHeader->Addr2;                                     \
		}                                                                       \
	}                                                                           \
																				\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData, 				\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);                                   \
}
#endif /* CONFIG_STA_SUPPORT */


BOOLEAN APFowardWirelessStaToWirelessSta(
	IN	struct rtmp_adapter *pAd,
	IN	struct sk_buff *pPacket,
	IN	ULONG			FromWhichBSSID);

VOID Announce_or_Forward_802_3_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	struct sk_buff *pPacket,
	IN	UCHAR			FromWhichBSSID);

VOID Sta_Announce_or_Forward_802_3_Packet(
	IN	struct rtmp_adapter *pAd,
	IN	struct sk_buff *pPacket,
	IN	UCHAR			FromWhichBSSID);

#ifdef CONFIG_AP_SUPPORT
#define AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
			Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#define ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
			Sta_Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
			/*announce_802_3_packet(_pAd, _pPacket); */
#endif /* CONFIG_STA_SUPPORT */


/* Normal, AMPDU or AMSDU */
VOID CmmRxnonRalinkFrameIndicate(
	IN struct rtmp_adapter *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
VOID CmmRxnonRalinkFrameIndicate_Hdr_Trns(
	IN struct rtmp_adapter *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID);
#endif /* HDR_TRANS_SUPPORT */

VOID CmmRxRalinkFrameIndicate(
	IN struct rtmp_adapter *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID);

VOID Update_Rssi_Sample(
	IN struct rtmp_adapter *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN RXWI_STRUC *pRxWI);

struct sk_buff *GetPacketFromRxRing(
	IN struct rtmp_adapter *pAd,
	OUT RX_BLK *pRxBlk,
	OUT BOOLEAN	 *pbReschedule,
	INOUT uint32_t *pRxPending,
	BOOLEAN *bCmdRspPacket,
	UCHAR RxRingNo);

struct sk_buff *RTMPDeFragmentDataFrame(
	IN struct rtmp_adapter *pAd,
	IN RX_BLK *pRxBlk);

/*////////////////////////////////////*/

#if defined (AP_SCAN_SUPPORT) || defined (CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
INT Set_ApCli_AuthMode_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ApCli_EncrypType_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ApCli_Enable_Proc(IN struct rtmp_adapter *pAd, IN char *arg);
INT Set_ApCli_Ssid_Proc(IN struct rtmp_adapter *pAd, IN char *arg);


#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
typedef struct CountryCodeToCountryRegion {
	USHORT		CountryNum;
	UCHAR		IsoName[3];
	/*UCHAR		CountryName[40]; */
	char *	pCountryName;
	BOOLEAN		SupportABand;
	/*ULONG		RegDomainNum11A; */
	UCHAR		RegDomainNum11A;
	BOOLEAN  	SupportGBand;
	/*ULONG		RegDomainNum11G; */
	UCHAR		RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;
#endif /* CONFIG_AP_SUPPORT */

#ifdef SNMP_SUPPORT
/*for snmp */
typedef struct _DefaultKeyIdxValue
{
	UCHAR	KeyIdx;
	UCHAR	Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif



void STA_MonPktSend(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);
#ifdef CONFIG_STA_SUPPORT
VOID RTMPSetDesiredRates(struct rtmp_adapter *pAd, LONG Rates);

#endif /* CONFIG_STA_SUPPORT */

INT	Set_FixedTxMode_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

INT Set_LongRetryLimit_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ShortRetryLimit_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_AutoFallBack_Proc(struct rtmp_adapter *pAd, char *arg);
VOID RT28XXDMADisable(struct rtmp_adapter *pAd);
VOID RT28XXDMAEnable(struct rtmp_adapter *pAd);

VOID RT28xx_UpdateBeaconToAsic(
	IN struct rtmp_adapter * pAd,
	IN INT apidx,
	IN ULONG BeaconLen,
	IN ULONG UpdatePos);

void CfgInitHook(struct rtmp_adapter *pAd);


int RtmpNetTaskInit(struct rtmp_adapter *pAd);
int RtmpMgmtTaskInit(struct rtmp_adapter *pAd);
VOID RtmpNetTaskExit(struct rtmp_adapter *pAd);
VOID RtmpMgmtTaskExit(struct rtmp_adapter *pAd);

void tbtt_tasklet(unsigned long data);



#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
int RecoverConnectInfo(struct rtmp_adapter *pAd);
int StoreConnectInfo(struct rtmp_adapter *pAd);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

VOID AsicTurnOffRFClk(struct rtmp_adapter *pAd, UCHAR Channel);



#ifdef RTMP_TIMER_TASK_SUPPORT
INT RtmpTimerQThread(ULONG Context);

RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(
	IN struct rtmp_adapter *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RtmpTimerQRemove(
	IN struct rtmp_adapter *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

void RtmpTimerQExit(struct rtmp_adapter *pAd);
void RtmpTimerQInit(struct rtmp_adapter *pAd);
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef NEW_WOW_SUPPORT
VOID RT28xxAndesWOWEnable(struct rtmp_adapter *pAd);
VOID RT28xxAndesWOWDisable(struct rtmp_adapter *pAd);
#endif /* NEW_WOW_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
VOID RT28xxAsicWOWEnable(struct rtmp_adapter *pAd);
VOID RT28xxAsicWOWDisable(struct rtmp_adapter *pAd);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

/*////////////////////////////////////*/

#ifdef AP_QLOAD_SUPPORT
VOID QBSS_LoadInit(struct rtmp_adapter *pAd);
VOID QBSS_LoadAlarmReset(struct rtmp_adapter *pAd);
VOID QBSS_LoadAlarmResume(struct rtmp_adapter *pAd);
uint32_t QBSS_LoadBusyTimeGet(struct rtmp_adapter *pAd);
BOOLEAN QBSS_LoadIsAlarmIssued(struct rtmp_adapter *pAd);
BOOLEAN QBSS_LoadIsBusyTimeAccepted(struct rtmp_adapter *pAd, uint32_t BusyTime);
uint32_t QBSS_LoadElementAppend(struct rtmp_adapter *pAd, UINT8 *buf_p);
uint32_t QBSS_LoadElementParse(
 	IN		struct rtmp_adapter *pAd,
	IN		UINT8			*pElement,
	OUT		uint16_t 		*pStationCount,
	OUT		UINT8			*pChanUtil,
	OUT		uint16_t 		*pAvalAdmCap);

VOID QBSS_LoadUpdate(struct rtmp_adapter *pAd, ULONG UpTime);
VOID QBSS_LoadStatusClear(struct rtmp_adapter *pAd);

INT	Show_QoSLoad_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* AP_QLOAD_SUPPORT */

/*///////////////////////////////////*/
INT RTMPShowCfgValue(struct rtmp_adapter *pAd, char *name, char *buf, uint32_t MaxLen);
char *RTMPGetRalinkAuthModeStr(NDIS_802_11_AUTHENTICATION_MODE authMode);
char *RTMPGetRalinkEncryModeStr(USHORT encryMode);
/*//////////////////////////////////*/

#ifdef CONFIG_STA_SUPPORT
VOID AsicStaBbpTuning(struct rtmp_adapter *pAd);

BOOLEAN StaUpdateMacTableEntry(
	IN  struct rtmp_adapter *pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN  HT_CAPABILITY_IE	*pHtCapability,
	IN  UCHAR				HtCapabilityLen,
	IN  ADD_HT_INFO_IE		*pAddHtInfo,
	IN  UCHAR				AddHtInfoLen,
	IN IE_LISTS *ie_list,
	IN  USHORT        		CapabilityInfo);


BOOLEAN	AUTH_ReqSend(
	IN  struct rtmp_adapter *		pAd,
	IN  PMLME_QUEUE_ELEM	pElem,
	IN  PRALINK_TIMER_STRUCT pAuthTimer,
	IN  char *			pSMName,
	IN  USHORT				SeqNo,
	IN  PUCHAR				pNewElement,
	IN  ULONG				ElementLen);
#endif /* CONFIG_STA_SUPPORT */


VOID ReSyncBeaconTime(struct rtmp_adapter *pAd);
VOID RTMPSetAGCInitValue(struct rtmp_adapter *pAd, UCHAR BandWidth);

#ifdef TXBF_SUPPORT
VOID handleHtcField(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);
#endif /* TXBF_SUPPORT */

#ifdef MFB_SUPPORT
VOID MFB_PerPareMRQ(struct rtmp_adapter *pAd, VOID* pBuf, MAC_TABLE_ENTRY *pEntry);
VOID MFB_PerPareMFB(struct rtmp_adapter *pAd, VOID *pBuf, MAC_TABLE_ENTRY *pEntry);
#endif /* MFB_SUPPORT */

#define VIRTUAL_IF_INC(__pAd) ((__pAd)->VirtualIfCnt++)
#define VIRTUAL_IF_DEC(__pAd) ((__pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(__pAd) ((__pAd)->VirtualIfCnt)


#ifdef RTMP_USB_SUPPORT
/*
 * Function Prototype in rtusb_bulk.c
 */

VOID RTUSBInitTxDesc(
	IN	struct rtmp_adapter *pAd,
	IN	PTX_CONTEXT		pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	usb_complete_t	Func);

VOID RTUSBInitHTTxDesc(
	IN	struct rtmp_adapter *pAd,
	IN	PHT_TX_CONTEXT	pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	ULONG			BulkOutSize,
	IN	usb_complete_t	Func);

VOID RTUSBInitRxDesc(struct rtmp_adapter *pAd, RX_CONTEXT *pRxContext);
VOID RTUSBCleanUpDataBulkOutQueue(struct rtmp_adapter *pAd);
VOID RTUSBCancelPendingBulkOutIRP(struct rtmp_adapter *pAd);

VOID RTUSBBulkOutDataPacket(struct rtmp_adapter *pAd, UCHAR BulkOutPipeId, UCHAR Index);
VOID RTUSBBulkOutNullFrame(struct rtmp_adapter *pAd);
VOID RTUSBBulkOutRTSFrame(struct rtmp_adapter *pAd);

VOID RTUSBCancelPendingBulkInIRP(struct rtmp_adapter *pAd);
VOID RTUSBCancelPendingIRPs(struct rtmp_adapter *pAd);

VOID RTUSBBulkOutMLMEPacket(struct rtmp_adapter *pAd, UCHAR Index);
VOID RTUSBBulkOutPsPoll(struct rtmp_adapter *pAd);
VOID RTUSBCleanUpMLMEBulkOutQueue(struct rtmp_adapter *pAd);

VOID RTUSBKickBulkOut(struct rtmp_adapter *pAd);
VOID RTUSBBulkReceive(struct rtmp_adapter *pAd);

VOID RTUSBBulkCmdRspEventReceive(struct rtmp_adapter *pAd);

VOID DoBulkIn(struct rtmp_adapter *pAd);

VOID RTUSBInitRxDesc(struct rtmp_adapter *pAd, RX_CONTEXT *pRxContext);
VOID RTUSBBulkRxHandle(ULONG data);
VOID InitUSBDevice(RT_CMD_USB_INIT *pConfig, VOID *pAd);
#endif /* RTMP_USB_SUPPORT */


#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(struct rtmp_adapter *pAd, TX_BLK *pTxBlk);
VOID RTMPUpdateSwCacheCipherInfo(struct rtmp_adapter *pAd, TX_BLK *pTxBlk, UCHAR *pHdr);
#endif /* SOFT_ENCRYPT */


/*
	OS Related funciton prototype definitions.
	TODO: Maybe we need to move these function prototypes to other proper place.
*/
VOID RTInitializeCmdQ(PCmdQ cmdq);

INT RTPCICmdThread(ULONG Context);

VOID CMDHandler(struct rtmp_adapter *pAd);

VOID RTThreadDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt);

int RTEnqueueInternalCmd(
	IN struct rtmp_adapter *pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN uint32_t 		InformationBufferLength);

#ifdef HOSTAPD_SUPPORT
VOID ieee80211_notify_michael_failure(
	IN	struct rtmp_adapter *pAd,
	IN	PHEADER_802_11   pHeader,
	IN	UINT            keyix,
	IN	INT              report);

const CHAR* ether_sprintf(const UINT8 *mac);
#endif/*HOSTAPD_SUPPORT*/


#ifdef VENDOR_FEATURE3_SUPPORT
VOID RTMP_IO_WRITE32(struct rtmp_adapter *pAd, uint32_t Offset, uint32_t Value);
#endif /* VENDOR_FEATURE3_SUPPORT */


INT AsicGetMacVersion(struct rtmp_adapter *pAd);

INT WaitForAsicReady(struct rtmp_adapter *pAd);

BOOLEAN CHAN_PropertyCheck(struct rtmp_adapter *pAd, uint32_t ChanNum, UCHAR Property);

#ifdef CONFIG_STA_SUPPORT

/* command */
INT Set_SSID_Proc(struct rtmp_adapter *pAd, char *arg);

INT	Set_WmmCapable_Proc(struct rtmp_adapter *pAd, char *arg);

INT Set_NetworkType_Proc(struct rtmp_adapter *pAd, char *arg);

INT Set_AuthMode_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_EncrypType_Proc(struct rtmp_adapter *pAd, char *arg);

INT Set_DefaultKeyID_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_Wep_Key_Proc(struct rtmp_adapter  *pAd, char *Key, INT KeyLen, INT KeyId);
INT Set_Key1_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_Key2_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_Key3_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_Key4_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_WPAPSK_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_PSMode_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef WPA_SUPPLICANT_SUPPORT
INT Set_Wpa_Support(struct rtmp_adapter *pAd, char *arg);
#endif /* WPA_SUPPLICANT_SUPPORT */

int RTMPWPANoneAddKeyProc(struct rtmp_adapter *pAd, VOID *pBuf);

INT Set_FragTest_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef DOT11_N_SUPPORT
INT Set_TGnWifiTest_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* DOT11_N_SUPPORT */

INT Set_LongRetryLimit_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ShortRetryLimit_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_Ieee80211dClientMode_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

INT	Show_Adhoc_MacTable_Proc(struct rtmp_adapter *pAd, char *extra, uint32_t size);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPIoctlRF(
	IN	struct rtmp_adapter *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* RTMP_RF_RW_SUPPORT */


INT Set_BeaconLostTime_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_AutoRoaming_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_SiteSurvey_Proc(struct rtmp_adapter *pAd, char *arg);
INT Set_ForceTxBurst_Proc(struct rtmp_adapter *pAd, char *arg);

VOID RTMPAddKey(struct rtmp_adapter *pAd, PNDIS_802_11_KEY pKey);
VOID StaSiteSurvey(struct rtmp_adapter *pAd, PNDIS_802_11_SSID pSsid, UCHAR ScanType);

VOID MaintainBssTable(
	IN  struct rtmp_adapter *pAd,
	IN OUT	BSS_TABLE *Tab,
	IN  ULONG	MaxRxTimeDiff,
	IN  UCHAR	MaxSameRxTimeCount);
#endif /* CONFIG_STA_SUPPORT */

void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG* fLastTxRxRate);


#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
VOID ApcliSendAssocIEsToWpaSupplicant(
    IN  struct rtmp_adapter *pAd,
    IN UINT ifIndex);

INT	    ApcliWpaCheckEapCode(
	IN  struct rtmp_adapter *  		pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet);

VOID    ApcliWpaSendEapolStart(
	IN	struct rtmp_adapter *pAd,
	IN  PUCHAR          pBssid,
	IN  PMAC_TABLE_ENTRY pMacEntry,
	IN	PAPCLI_STRUCT pApCliEntry);
#endif/*WPA_SUPPLICANT_SUPPORT*/

VOID ApCliRTMPSendNullFrame(
	IN struct rtmp_adapter *pAd,
	IN UCHAR			TxRate,
	IN BOOLEAN 		bQosNull,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN USHORT          PwrMgmt);

#endif/*APCLI_SUPPORT*/


void RTMP_IndicateMediaState(
	IN	struct rtmp_adapter *pAd,
	IN  NDIS_MEDIA_STATE	media_state);

#if defined(RT3350) || defined(RT33xx)
VOID RTMP_TxEvmCalibration(
	IN struct rtmp_adapter *pAd);
#endif /* defined(RT3350) || defined(RT33xx) */

VOID RTMPIoctlShow(
	IN struct rtmp_adapter *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *rq,
	IN uint32_t subcmd,
	IN VOID *pData,
	IN ULONG Data);

INT RTMP_COM_IoctlHandle(
	IN struct rtmp_adapter *pAdSrc,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN INT cmd,
	IN USHORT subcmd,
	IN VOID *pData,
	IN ULONG Data);

#ifdef CONFIG_AP_SUPPORT
INT RTMP_AP_IoctlPrepare(struct rtmp_adapter *pAd, VOID *pCB);
#endif /* CONFIG_AP_SUPPORT */


INT Set_VcoPeriod_Proc(struct rtmp_adapter *pAd, char *arg);
INT	Set_RateAlg_Proc(struct rtmp_adapter *pAd, char *arg);

#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(struct rtmp_adapter *pAd, char *arg);
#endif /* SINGLE_SKU */

VOID RtmpEnqueueNullFrame(
	IN struct rtmp_adapter *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP);

VOID RtmpCleanupPsQueue(
	IN  struct rtmp_adapter *pAd,
	IN  PQUEUE_HEADER   pQueue);

int RtmpInsertPsQueue(
	IN struct rtmp_adapter *pAd,
	IN struct sk_buff *pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx);

VOID RtmpHandleRxPsPoll(struct rtmp_adapter *pAd, UCHAR *pAddr, USHORT Aid, BOOLEAN isActive);
BOOLEAN RtmpPsIndicate(struct rtmp_adapter *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm);


VOID RtmpPrepareHwNullFrame(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR OpMode,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN CHAR Index);


VOID dev_rx_mgmt_frm(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);
VOID dev_rx_ctrl_frm(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN RtmpPktPmBitCheck(struct rtmp_adapter *pAd);
VOID RtmpPsActiveExtendCheck(struct rtmp_adapter *pAd);
VOID RtmpPsModeChange(struct rtmp_adapter *pAd, uint32_t PsMode);
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
void DisplayTxAgg (struct rtmp_adapter *pAd);
#endif /* DOT11_N_SUPPORT */

VOID set_default_ap_edca_param(struct rtmp_adapter *pAd);
VOID set_default_sta_edca_param(struct rtmp_adapter *pAd);

UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps);
UCHAR dot11_max_sup_rate(INT SupRateLen, UCHAR *SupRate, INT ExtRateLen, UCHAR *ExtRate);

VOID mgmt_tb_set_mcast_entry(struct rtmp_adapter *pAd);
VOID set_entry_phy_cfg(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry);
VOID MacTableReset(struct rtmp_adapter *pAd);
MAC_TABLE_ENTRY *MacTableLookup(struct rtmp_adapter *pAd, UCHAR *pAddr);
BOOLEAN MacTableDeleteEntry(struct rtmp_adapter *pAd, USHORT wcid, UCHAR *pAddr);
MAC_TABLE_ENTRY *MacTableInsertEntry(
    IN struct rtmp_adapter *pAd,
    IN UCHAR *pAddr,
    IN struct rtmp_wifi_dev *wdev,
	IN UCHAR apidx,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll);


VOID dump_txwi(struct rtmp_adapter *pAd, TXWI_STRUC *pTxWI);
VOID dump_rxwi(struct rtmp_adapter *pAd, RXWI_STRUC *pRxWI);
VOID dump_txinfo(struct rtmp_adapter *pAd, TXINFO_STRUC *pTxInfo);
VOID dump_rxinfo(struct rtmp_adapter *pAd, RXINFO_STRUC *pRxInfo);
VOID dump_rxblk(struct rtmp_adapter *pAd, RX_BLK *pRxBlk);




#ifdef WFA_VHT_PF
/* for SIGMA */
INT set_vht_nss_mcs_cap(struct rtmp_adapter *pAd, char *arg);
INT set_vht_nss_mcs_opt(struct rtmp_adapter *pAd, char *arg);
INT set_vht_opmode_notify_ie(struct rtmp_adapter *pAd, char *arg);

INT set_force_operating_mode(struct rtmp_adapter *pAd, char *arg);
INT set_force_amsdu(struct rtmp_adapter *pAd, char *arg);
INT set_force_noack(struct rtmp_adapter *pAd, char *arg);
INT set_force_vht_sgi(struct rtmp_adapter *pAd, char *arg);
INT set_force_vht_tx_stbc(struct rtmp_adapter *pAd, char *arg);
INT set_force_ext_cca(struct rtmp_adapter *pAd, char *arg);
INT set_rx_rts_cts(struct rtmp_adapter *pAd, char *arg);
#endif /* WFA_VHT_PF */



#ifdef DROP_MASK_SUPPORT
VOID asic_set_drop_mask(
	struct rtmp_adapter *ad,
	USHORT	wcid,
	BOOLEAN enable);

VOID asic_drop_mask_reset(
	struct rtmp_adapter *ad);

VOID drop_mask_init_per_client(
	struct rtmp_adapter *ad,
	MAC_TABLE_ENTRY *entry);

VOID drop_mask_release_per_client(
	struct rtmp_adapter *ad,
	MAC_TABLE_ENTRY *entry);

VOID drop_mask_per_client_reset(
	struct rtmp_adapter *ad);

VOID set_drop_mask_per_client(
	struct rtmp_adapter *ad,
	MAC_TABLE_ENTRY *entry,
	UINT8 type,
	BOOLEAN enable);

VOID drop_mask_timer_action(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);
#endif /* DROP_MASK_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
VOID Peer_DelBA_Tx_Adapt_Init(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID PeerDelBATxAdaptTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef MULTI_CLIENT_SUPPORT
VOID asic_change_tx_retry(
	IN struct rtmp_adapter *pAd,
	IN USHORT num);

VOID pkt_aggr_num_change(
	IN struct rtmp_adapter *pAd,
	IN USHORT num);

VOID asic_tune_be_wmm(
	IN struct rtmp_adapter *pAd,
	IN USHORT num);
#endif /* MULTI_CLIENT_SUPPORT */

INT set_rf(struct rtmp_adapter *pAd, char *arg);
int write_reg(struct rtmp_adapter *ad, u32 base, u16 offset, u32 value);
int read_reg(struct rtmp_adapter *ad, u32 base, u16 offset, u32 *value);
INT show_pwr_info(struct rtmp_adapter *ad, char *arg);
#ifdef DBG_DIAGNOSE
INT Show_Diag_Proc(struct rtmp_adapter *pAd, char *arg);
#endif


#endif  /* __RTMP_H__ */

