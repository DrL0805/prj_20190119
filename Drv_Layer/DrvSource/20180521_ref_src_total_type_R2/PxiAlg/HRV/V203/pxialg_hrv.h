#ifndef _PXIALG_HRV_
#define _PXIALG_HRV_

#if defined(WIN32) && !defined(PXIALG_STATIC_LIB)
#   ifdef PXIALG_EXPORTS
#       define PXIALG_API   __declspec(dllexport)
#   else
#       define PXIALG_API   __declspec(dllimport)
#   endif
#else
#   define PXIALG_API
#endif	// WIN32


#ifdef __cplusplus
extern "C"
{
#endif
	PXIALG_API int HRVReset(void);
	PXIALG_API int HRVGetMemorySize(int enable_lhr, int test_seconds, int sampling_rate);
	PXIALG_API int HRVOpen(void *p_buffer);
	PXIALG_API int HRVProcess(int data_ready, float data);
	PXIALG_API int HRVClose(void);

	PXIALG_API int HRVGetVersion(void);
	PXIALG_API float HRVGetSDNN(void);
	PXIALG_API int HRVGetSDNNLevel(int m_iAge, float dSDNN);
	PXIALG_API float HRVGetLHRatio(float *nlf, float *nhf);
	PXIALG_API float HRVGetRMSSD(void);
	PXIALG_API int HRVGetSleepQuality(int age);
	PXIALG_API int HRVGetOnlineRRIReportFlag(void);
	PXIALG_API float HRVGetOnlineRRI(void);
	PXIALG_API int HRVGetRawRRINum(void);
	PXIALG_API float * HRVGetRawRRIBuf(void);
	PXIALG_API float HRVGetNoiseRate(void);
	PXIALG_API float HRVGetRRIStability(void);
	PXIALG_API float HRVGetRRIQuality(void);
	PXIALG_API int HRVGetReadyFlag(void);
	PXIALG_API int HRVGetMotionFlag(void);
	PXIALG_API int HRVGetHRAvg(void);

	PXIALG_API void HRVSetSkipCounterTh(int SkipCounterTh);
	PXIALG_API void HRVSetFreqOffset(float FreqOffset);
	PXIALG_API void HRVSetDiffTime(float DiffTime, int SampleNum);
	PXIALG_API void HRVSetPeakProcessReadyCounterTh(int PeakProcessReady3Counter_Th0,
									                int PeakProcessReady3Counter_Th1,
									                int PeakProcessReady3Counter_Th2,
									                int PeakProcessReady3Counter_Th3,
									                int m_PeakProcessReady3Counter_HeartRate_Th1,
									                int m_PeakProcessReady3Counter_HeartRate_Th2,
									                int m_PeakProcessReady3Counter_HeartRate_Th3);
#ifdef __cplusplus
}
#endif

#endif /* _PXIALG_HRV_ */

