#ifdef VIDEO_TURBINE_SUPPORT
extern AP_VIDEO_STRUCT GLOBAL_AP_VIDEO_CONFIG;

VOID VideoModeUpdate(IN Pstruct rtmp_adapter pAd);
VOID VideoModeDynamicTune(IN Pstruct rtmp_adapter pAd);
uint32_t GetAsicDefaultRetry(IN Pstruct rtmp_adapter pAd);
UCHAR GetAsicDefaultTxBA(IN Pstruct rtmp_adapter pAd);
uint32_t GetAsicVideoRetry(IN Pstruct rtmp_adapter pAd);
UCHAR GetAsicVideoTxBA(IN Pstruct rtmp_adapter pAd);
VOID VideoConfigInit(IN Pstruct rtmp_adapter pAd);
#endif /* VIDEO_TURBINE_SUPPORT */

