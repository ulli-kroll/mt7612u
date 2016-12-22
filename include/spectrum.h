
#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include "rtmp_type.h"
#include "spectrum_def.h"


UINT8 GetRegulatoryMaxTxPwr(
	IN struct rtmp_adapter *pAd,
	IN UINT8 channel);

CHAR RTMP_GetTxPwr(
	IN struct rtmp_adapter *pAd,
	IN HTTRANSMIT_SETTING HTTxMode);

/*
	==========================================================================
	Description:
		Prepare Measurement request action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID MakeMeasurementReqFrame(
	IN struct rtmp_adapter *pAd,
	OUT PUCHAR pOutBuffer,
	OUT PULONG pFrameLen,
	IN UINT8 TotalLen,
	IN UINT8 Category,
	IN UINT8 Action,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT16 NumOfRepetitions);

/*
	==========================================================================
	Description:
		Prepare Measurement report action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID EnqueueMeasurementRep(
	IN struct rtmp_adapter *pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT8 ReportInfoLen,
	IN uint8_t * pReportInfo);

/*
	==========================================================================
	Description:
		Prepare TPC Request action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID EnqueueTPCReq(
	IN struct rtmp_adapter *pAd,
	IN PUCHAR pDA,
	IN UCHAR DialogToken);

/*
	==========================================================================
	Description:
		Prepare TPC Report action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID EnqueueTPCRep(
	IN struct rtmp_adapter *pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin);


/*
	==========================================================================
	Description:
		Spectrun action frames Handler such as channel switch annoucement,
		measurement report, measurement request actions frames.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
VOID PeerSpectrumAction(
    IN struct rtmp_adapter *pAd,
    IN MLME_QUEUE_ELEM *Elem);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_MeasureReq_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	PSTRING			arg);

INT Set_TpcReq_Proc(
	IN	struct rtmp_adapter *pAd,
	IN	PSTRING			arg);

INT Set_PwrConstraint(
	IN	struct rtmp_adapter *pAd,
	IN	PSTRING			arg);


int MeasureReqTabInit(
	IN struct rtmp_adapter *pAd);

VOID MeasureReqTabExit(
	IN struct rtmp_adapter *pAd);

PMEASURE_REQ_ENTRY MeasureReqLookUp(
	IN struct rtmp_adapter *pAd,
	IN UINT8			DialogToken);

PMEASURE_REQ_ENTRY MeasureReqInsert(
	IN struct rtmp_adapter *pAd,
	IN UINT8			DialogToken);

VOID MeasureReqDelete(
	IN struct rtmp_adapter *pAd,
	IN UINT8			DialogToken);

VOID InsertChannelRepIE(
	IN struct rtmp_adapter *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PSTRING pCountry,
	IN UINT8 RegulatoryClass);

VOID InsertTpcReportIE(
	IN struct rtmp_adapter *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin);

VOID InsertDialogToken(
	IN struct rtmp_adapter *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 DialogToken);

int TpcReqTabInit(
	IN struct rtmp_adapter *pAd);

VOID TpcReqTabExit(
	IN struct rtmp_adapter *pAd);

VOID NotifyChSwAnnToPeerAPs(
	IN struct rtmp_adapter *pAd,
	IN PUCHAR pRA,
	IN PUCHAR pTA,
	IN UINT8 ChSwMode,
	IN UINT8 Channel);

VOID RguClass_BuildBcnChList(
	IN struct rtmp_adapter *pAd,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen);
#endif /* __SPECTRUM_H__ */

