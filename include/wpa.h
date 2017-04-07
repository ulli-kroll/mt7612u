/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wpa.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__WPA_H__
#define	__WPA_H__

#ifndef ROUND_UP
#define ROUND_UP(__x, __y) \
	(((ULONG)((__x)+((__y)-1))) & ((ULONG)~((__y)-1)))
#endif

#define	SET_UINT16_TO_ARRARY(_V, _LEN)		\
{											\
	_V[0] = ((uint16_t)_LEN) >> 8;			\
	_V[1] = ((uint16_t)_LEN & 0xFF);					\
}

#define	INC_UINT16_TO_ARRARY(_V, _LEN)			\
{												\
	uint16_t var_len;							\
												\
	var_len = (_V[0]<<8) | (_V[1]);				\
	var_len += _LEN;							\
												\
	_V[0] = (var_len & 0xFF00) >> 8;			\
	_V[1] = (var_len & 0xFF);					\
}

#define	CONV_ARRARY_TO_UINT16(_V)	((_V[0]<<8) | (_V[1]))

#define	ADD_ONE_To_64BIT_VAR(_V)		\
{										\
	u8 cnt = LEN_KEY_DESC_REPLAY;	\
	do									\
	{									\
		cnt--;							\
		_V[cnt]++;						\
		if (cnt == 0)					\
			break;						\
	}while (_V[cnt] == 0);				\
}

#define INC_TX_TSC(_tsc, _cnt)                          \
{                                                       \
    INT i=0;                                            \
	while (++_tsc[i] == 0x0)                            \
    {                                                   \
        i++;                                            \
		if (i == (_cnt))                                \
			break;                                      \
	}                                                   \
}

#define IS_WPA_CAPABILITY(a)       (((a) >= Ndis802_11AuthModeWPA) && ((a) <= Ndis802_11AuthModeWPA1PSKWPA2PSK))

/*
	WFA recommend to restrict the encryption type in 11n-HT mode.
 	So, the WEP and TKIP shall not be allowed to use HT rate.
 */
#define IS_INVALID_HT_SECURITY(_mode)		\
	(((_mode) == Ndis802_11Encryption1Enabled) || \
	 ((_mode) == Ndis802_11Encryption2Enabled))

#define MIX_CIPHER_WPA_TKIP_ON(x)       (((x) & 0x08) != 0)
#define MIX_CIPHER_WPA_AES_ON(x)        (((x) & 0x04) != 0)
#define MIX_CIPHER_WPA2_TKIP_ON(x)      (((x) & 0x02) != 0)
#define MIX_CIPHER_WPA2_AES_ON(x)       (((x) & 0x01) != 0)

/* Some definition are different between Keneral mode and Daemon mode */
#ifdef WPA_DAEMON_MODE
/* The definition for Daemon mode */
#define WPA_GET_BSS_NUM(_pAd)		(_pAd)->mbss_num

#define WPA_GET_PMK(_pAd, _pEntry, _pmk)					\
{															\
	_pmk = _pAd->MBSS[_pEntry->apidx].PMK;					\
}

#define WPA_GET_GTK(_pAd, _pEntry, _gtk)					\
{															\
	_gtk = _pAd->MBSS[_pEntry->apidx].GTK;					\
}

#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)		\
{															\
	_cipher = (_pAd)->MBSS[_pEntry->apidx].GroupEncrypType;	\
}

#define WPA_GET_DEFAULT_KEY_ID(_pAd, _pEntry, _idx)			\
{															\
	_idx = (_pAd)->MBSS[_pEntry->apidx].DefaultKeyId;		\
}

#define WPA_GET_BMCST_TSC(_pAd, _pEntry, _tsc)				\
{															\
	_tsc = 1;												\
}

#define WPA_BSSID(_pAd, _apidx)		(_pAd)->MBSS[_apidx].wlan_addr

#define WPA_OS_MALLOC(_p, _s)		\
{									\
	_p = os_malloc(_s);			\
}

#define WPA_OS_FREE(_p)		\
{								\
	os_free(_p);				\
}

#define WPA_GET_CURRENT_TIME(_time)		\
{										\
	struct timeval tv;					\
	gettimeofday(&tv, NULL);			\
	*(_time) = tv.tv_sec;					\
}

#else
/* The definition for Driver mode */

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define WPA_GET_BSS_NUM(_pAd)		(((_pAd)->OpMode == OPMODE_AP) ? (_pAd)->ApCfg.BssidNum : 1)
#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)					\
	{																	\
	_cipher = Ndis802_11WEPDisabled;								\
		if ((_pAd)->OpMode == OPMODE_AP)								\
		{																\
		if (IS_ENTRY_APCLI(_pEntry) && 								\
			((_pEntry)->wdev_idx < MAX_APCLI_NUM))			\
			_cipher = (_pAd)->ApCfg.ApCliTab[(_pEntry)->wdev_idx].GroupCipher;	\
			else if ((_pEntry)->apidx < (_pAd)->ApCfg.BssidNum)			\
				_cipher = (_pAd)->ApCfg.MBSSID[_pEntry->apidx].GroupKeyWepStatus;\
		}																\
		else															\
			_cipher = (_pAd)->StaCfg.GroupCipher;						\
	}

#define WPA_BSSID(_pAd, _apidx) 	(((_pAd)->OpMode == OPMODE_AP) ?\
									(_pAd)->ApCfg.MBSSID[_apidx].Bssid :\
									(_pAd)->CommonCfg.Bssid)
#elif defined(CONFIG_AP_SUPPORT)
#define WPA_GET_BSS_NUM(_pAd)		(_pAd)->ApCfg.BssidNum
#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)				\
	{																\
	_cipher = Ndis802_11WEPDisabled;							\
	if (IS_ENTRY_APCLI(_pEntry) && 								\
		((_pEntry)->wdev_idx < MAX_APCLI_NUM))			\
		_cipher = (_pAd)->ApCfg.ApCliTab[(_pEntry)->wdev_idx].GroupCipher;	\
		else if ((_pEntry)->apidx < (_pAd)->ApCfg.BssidNum)			\
			_cipher = (_pAd)->ApCfg.MBSSID[_pEntry->apidx].GroupKeyWepStatus;\
	}

#define WPA_BSSID(_pAd, _apidx) 	(_pAd)->ApCfg.MBSSID[_apidx].Bssid

#elif defined(CONFIG_STA_SUPPORT)
#define WPA_GET_BSS_NUM(_pAd)		1
#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)				\
	{																\
		_cipher = (_pAd)->StaCfg.GroupCipher;						\
	}
#define WPA_BSSID(_pAd, _apidx) 	(_pAd)->CommonCfg.Bssid
#endif /* defined(CONFIG_STA_SUPPORT) */

#define WPA_GET_CURRENT_TIME(_time)		NdisGetSystemUpTime(_time);

#endif /* End of Driver Mode */

#ifdef CONFIG_AP_SUPPORT
/*========================================
	The prototype is defined in ap_wpa.c
  ========================================*/
VOID WPA_APSetGroupRekeyAction(
	IN struct rtmp_adapter *pAd);

#endif /* CONFIG_AP_SUPPORT */

/*========================================
	The prototype is defined in cmm_wpa.c
  ========================================*/
void inc_iv_byte(
	u8 *iv,
	UINT len,
	UINT cnt);

bool WpaMsgTypeSubst(
	IN u8 EAPType,
	OUT INT *MsgType);

VOID PRF(
	IN u8 *key,
	IN INT key_len,
	IN u8 *prefix,
	IN INT prefix_len,
	IN u8 *data,
	IN INT data_len,
	OUT u8 *output,
	IN INT len);

int RtmpPasswordHash(
	char *password,
	unsigned char *ssid,
	int ssidlength,
	unsigned char *output);

	VOID KDF(
	IN uint8_t * key,
	IN INT key_len,
	IN uint8_t * label,
	IN INT label_len,
	IN uint8_t * data,
	IN INT data_len,
	OUT uint8_t * output,
	IN unsigned short len);

uint8_t * WPA_ExtractSuiteFromRSNIE(
	IN uint8_t * rsnie,
	IN UINT rsnie_len,
	IN UINT8 type,
	OUT UINT8 *count);

VOID WpaShowAllsuite(
	IN uint8_t * rsnie,
	IN UINT rsnie_len);

VOID RTMPInsertRSNIE(
	IN u8 *pFrameBuf,
	OUT PULONG pFrameLen,
	IN uint8_t * rsnie_ptr,
	IN UINT8 rsnie_len,
	IN uint8_t * pmkid_ptr,
	IN UINT8 pmkid_len);

/*
 =====================================
 	function prototype in cmm_wpa.c
 =====================================
*/
VOID WpaStateMachineInit(
    IN  struct rtmp_adapter *  pAd,
    IN  STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID RTMPToWirelessSta(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN u8 *pHeader802_3,
	IN UINT HdrLen,
	IN u8 *pData,
	IN UINT DataLen,
	IN bool bClearFrame);

VOID WpaDerivePTK(
	IN struct rtmp_adapter *pAd,
	IN u8 *PMK,
	IN u8 *ANonce,
	IN u8 *AA,
	IN u8 *SNonce,
	IN u8 *SA,
	OUT u8 *output,
	IN UINT len);

VOID WpaDeriveGTK(
	IN u8 *PMK,
	IN u8 *GNonce,
	IN u8 *AA,
	OUT u8 *output,
	IN UINT len);

VOID GenRandom(
	IN struct rtmp_adapter *pAd,
	IN u8 *macAddr,
	OUT u8 *random);

bool RTMPCheckWPAframe(
	IN struct rtmp_adapter *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN u8 *pData,
	IN ULONG DataByteCount,
	IN u8 FromWhichBSSID);


bool RTMPParseEapolKeyData(
	IN struct rtmp_adapter *pAd,
	IN u8 *pKeyData,
	IN u8 KeyDataLen,
	IN u8 GroupKeyIndex,
	IN u8 MsgType,
	IN bool bWPA2,
	IN MAC_TABLE_ENTRY *pEntry);

VOID WPA_ConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT u8 *pBuf);

VOID ConstructEapolMsg(
	IN PMAC_TABLE_ENTRY pEntry,
	IN u8 GroupKeyWepStatus,
	IN u8 MsgType,
	IN u8 DefaultKeyIdx,
	IN u8 *KeyNonce,
	IN u8 *TxRSC,
	IN u8 *GTK,
	IN u8 *RSNIE,
	IN u8 RSNIE_Len,
	OUT PEAPOL_PACKET pMsg);

PCIPHER_KEY RTMPSwCipherKeySelection(
	IN struct rtmp_adapter *pAd,
	IN u8 *pIV,
	IN RX_BLK *pRxBlk,
	IN PMAC_TABLE_ENTRY pEntry);

int RTMPSoftDecryptionAction(
	IN struct rtmp_adapter *pAd,
	IN u8 *pHdr,
	IN u8 UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT u8 *pData,
	INOUT uint16_t *DataByteCnt);

VOID RTMPSoftConstructIVHdr(
	IN u8 CipherAlg,
	IN u8 key_id,
	IN u8 *pTxIv,
	OUT u8 *pHdrIv,
	OUT UINT8 *hdr_iv_len);

VOID RTMPSoftEncryptionAction(
	IN struct rtmp_adapter *pAd,
	IN u8 CipherAlg,
	IN u8 *pHdr,
	IN u8 *pSrcBufData,
	IN uint32_t SrcBufLen,
	IN u8 KeyIdx,
	IN PCIPHER_KEY pKey,
	OUT UINT8 *ext_len);

VOID RTMPMakeRSNIE(
	IN struct rtmp_adapter *pAd,
	IN UINT AuthMode,
	IN UINT WepStatus,
	IN u8 apidx);

VOID WPAInstallPairwiseKey(
	struct rtmp_adapter *pAd,
	UINT8 BssIdx,
	PMAC_TABLE_ENTRY pEntry,
	bool bAE);

VOID WPAInstallSharedKey(
	struct rtmp_adapter *pAd,
	UINT8 GroupCipher,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 Wcid,
	bool bAE,
	uint8_t * pGtk,
	UINT8 GtkLen);

VOID RTMPSetWcidSecurityInfo(
	struct rtmp_adapter *pAd,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 CipherAlg,
	UINT8 Wcid,
	UINT8 KeyTabFlag);

VOID CalculateMIC(
	IN u8 KeyDescVer,
	IN u8 *PTK,
	OUT PEAPOL_PACKET pMsg);

bool rtmp_chk_tkip_mic(struct rtmp_adapter *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk);

#ifdef WPA_SUPPLICANT_SUPPORT
INT WpaCheckEapCode(
	IN  struct rtmp_adapter *pAd,
	IN  u8 *pFrame,
	IN  unsigned short FrameLen,
	IN  unsigned short OffSet);
#endif /* WPA_SUPPLICANT_SUPPORT */


char *GetEapolMsgType(CHAR msg);

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

/*
 =====================================
 	function prototype in cmm_wep.c
 =====================================
*/
UINT RTMP_CALC_FCS32(
	IN UINT Fcs,
	IN u8 *Cp,
	IN INT Len);

VOID RTMPConstructWEPIVHdr(
	IN UINT8 key_idx,
	IN u8 *pn,
	OUT u8 *iv_hdr);

bool RTMPSoftEncryptWEP(
	IN struct rtmp_adapter *pAd,
	IN u8 *pIvHdr,
	IN PCIPHER_KEY pKey,
	INOUT u8 *pData,
	IN ULONG DataByteCnt);

bool RTMPSoftDecryptWEP(
	IN struct rtmp_adapter *pAd,
	IN PCIPHER_KEY pKey,
	INOUT u8 *pData,
	INOUT uint16_t *DataByteCnt);

/*
 =====================================
 	function prototype in cmm_tkip.c
 =====================================
*/
bool RTMPSoftDecryptTKIP(
	IN struct rtmp_adapter *pAd,
	IN u8 *pHdr,
	IN u8 UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT u8 *pData,
	IN uint16_t *DataByteCnt);

VOID TKIP_GTK_KEY_WRAP(
	IN u8 *key,
	IN u8 *iv,
	IN u8 *input_text,
	IN uint32_t input_len,
	OUT u8 *output_text);

VOID TKIP_GTK_KEY_UNWRAP(
	IN u8 *key,
	IN u8 *iv,
	IN u8 *input_text,
	IN uint32_t input_len,
	OUT u8 *output_text);

/*
 =====================================
 	function prototype in cmm_aes.c
 =====================================
*/
bool RTMPSoftDecryptAES(
	IN struct rtmp_adapter *pAd,
	IN u8 *pData,
	IN ULONG DataByteCnt,
	IN PCIPHER_KEY pWpaKey);

VOID RTMPConstructCCMPHdr(
	IN UINT8 key_idx,
	IN u8 *pn,
	OUT u8 *ccmp_hdr);

bool RTMPSoftEncryptCCMP(
	IN struct rtmp_adapter *pAd,
	IN u8 *pHdr,
	IN u8 *pIV,
	IN u8 *pKey,
	INOUT u8 *pData,
	IN uint32_t DataLen);

bool RTMPSoftDecryptCCMP(
	IN struct rtmp_adapter *pAd,
	IN u8 *pHdr,
	IN PCIPHER_KEY pKey,
	INOUT u8 *pData,
	INOUT uint16_t *DataLen);

VOID CCMP_test_vector(
	IN struct rtmp_adapter *pAd,
	IN INT input);

void inc_byte_array(u8 *counter, int len);

#endif
