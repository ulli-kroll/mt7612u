#ifndef __WSC_NFC_H__
#define __WSC_NFC_H__

VOID	NfcParseRspCommand(
	IN  Pstruct rtmp_adapter pAd,
	IN  u8 *pData,
	IN  unsigned short DataLen);

VOID	NfcCommand(
	IN	Pstruct rtmp_adapter pAd,
	IN	u8 Action,
	IN  u8 Type,
	IN  SHORT DataLen,
	IN  u8 *pData);

VOID	NfcGenRandomPasswd(
	IN  Pstruct rtmp_adapter pAd,
	IN  PWSC_CTRL       pWscCtrl);

INT 	NfcBuildWscProfileTLV(
	IN	Pstruct rtmp_adapter pAd,
	IN  PWSC_CTRL pWscCtrl,
	OUT	u8 *pbuf,
	OUT unsigned short *pBufLen);

INT 	NfcBuildOOBDevPasswdTLV(
	IN	Pstruct rtmp_adapter pAd,
	IN  PWSC_CTRL pWscCtrl,
	IN	u8 HandoverType,
	OUT	u8 *pbuf,
	OUT unsigned short *pBufLen);

INT		Set_NfcStatus_Proc(
	IN  struct rtmp_adapter 	*pAd,
	IN  char *		arg);

INT 	Set_NfcPasswdToken_Proc(
	IN  struct rtmp_adapter 	*pAd,
	IN  char *		arg);

INT 	Set_NfcConfigurationToken_Proc(
	IN  struct rtmp_adapter 	*pAd,
	IN  char *		arg);

INT 	Get_NfcStatus_Proc(
	IN	Pstruct rtmp_adapter pAd,
	IN	char *		arg);

INT		Set_DoWpsByNFC_Proc(
	IN  Pstruct rtmp_adapter 	pAd,
	IN  char *			arg);

INT Set_NfcRegenPK_Proc(
	IN Pstruct rtmp_adapter 	pAd,
	IN char *			arg);


#endif /* __WSC_NFC_H__ */
