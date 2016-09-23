#ifndef __WSC_NFC_H__
#define __WSC_NFC_H__

VOID	NfcParseRspCommand(
	IN  Pstruct rtmp_adapter pAd,
	IN  PUCHAR pData,
	IN  USHORT DataLen);

VOID	NfcCommand(
	IN	Pstruct rtmp_adapter pAd,
	IN	UCHAR Action,
	IN  UCHAR Type,
	IN  SHORT DataLen,
	IN  PUCHAR pData);

VOID	NfcGenRandomPasswd(
	IN  Pstruct rtmp_adapter pAd,
	IN  PWSC_CTRL       pWscCtrl);

INT 	NfcBuildWscProfileTLV(
	IN	Pstruct rtmp_adapter pAd,
	IN  PWSC_CTRL pWscCtrl,
	OUT	UCHAR *pbuf,
	OUT USHORT *pBufLen);

INT 	NfcBuildOOBDevPasswdTLV(
	IN	Pstruct rtmp_adapter pAd,
	IN  PWSC_CTRL pWscCtrl,
	IN	UCHAR	HandoverType,
	OUT	UCHAR *pbuf,
	OUT USHORT *pBufLen);

INT		Set_NfcStatus_Proc(
	IN  struct rtmp_adapter 	*pAd,
	IN  PSTRING			arg);

INT 	Set_NfcPasswdToken_Proc(
	IN  struct rtmp_adapter 	*pAd,
	IN  PSTRING			arg);

INT 	Set_NfcConfigurationToken_Proc(
	IN  struct rtmp_adapter 	*pAd,
	IN  PSTRING			arg);

INT 	Get_NfcStatus_Proc(
	IN	Pstruct rtmp_adapter pAd,
	IN	PSTRING			arg);

INT		Set_DoWpsByNFC_Proc(
	IN  Pstruct rtmp_adapter 	pAd,
	IN  PSTRING 			arg);

INT Set_NfcRegenPK_Proc(
	IN Pstruct rtmp_adapter 	pAd,
	IN PSTRING 			arg);


#endif /* __WSC_NFC_H__ */
