#ifndef PxiAlg_VP_H_
#define PxiAlg_VP_H_

#include <stdint.h>
#include "report.h"

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
extern "C" {
#endif // __cplusplus

PXIALG_API int PxiAlg_VP_GetMemorySize(void);
PXIALG_API uint8_t PxiAlg_VP_Open(float calibration_ppgsp, float calibration_ppgdp, float true_sbp, float true_dbp, int sample_rate, void *pBuffer);
PXIALG_API uint8_t PxiAlg_VP_Close(void);
PXIALG_API float PxiAlg_VP_Version(void);
PXIALG_API uint8_t PxiAlg_VP_Process(float data, int touch_flag);
PXIALG_API uint8_t PxiAlg_VP_DecodeProcess(uint32_t data, int touch_flag);
PXIALG_API uint8_t PxiAlg_VP_GetReport(struct Report *rpt);

PXIALG_API uint8_t PxiAlg_VP_SetTimeoutLength(int length);
PXIALG_API uint8_t PxiAlg_VP_SetFeatureStabilityTolerance(float tolerance);
PXIALG_API int PxiAlg_VP_SetModel(float slope_ppgsp, float y_intercept_ppgsp, float slope_ppgdp, float y_intercept_ppgdp);
PXIALG_API int PxiAlg_VP_SetOperationMode(int mode);
PXIALG_API int PxiAlg_VP_SetAnomalyTime(int second);
PXIALG_API int PxiAlg_VP_SetPhysicalInfo(int gender, int age, float height, float weight);

PXIALG_API int PxiAlg_VP_GetFeatureNum(void);
PXIALG_API float PxiAlg_VP_GetSignalQuality(void);
PXIALG_API float PxiAlg_VP_GetRegulatePPGSP(float true_sbp);
PXIALG_API float PxiAlg_VP_GetRegulatePPGDP(float true_dbp);
PXIALG_API float PxiAlg_VP_GetEstimatePPGSP(void);
PXIALG_API float PxiAlg_VP_GetEstimatePPGDP(void);
PXIALG_API int PxiAlg_VP_GetModel(float *slope_ppgsp, float *y_intercept_ppgsp, float *slope_ppgdp, float *y_intercept_ppgdp);
PXIALG_API int PxiAlg_VP_GetAnomalyFlag(void);

PXIALG_API float PxiAlg_VP_GetSingleReportDT(void);
PXIALG_API float PxiAlg_VP_GetSingleReportT1(void);
PXIALG_API float PxiAlg_VP_GetSingleReportStaticRawPPGSP(void);
PXIALG_API float PxiAlg_VP_GetSingleReportStaticRawPPGDP(void);
PXIALG_API float PxiAlg_VP_GetSingleReportStaticCalPPGSP(void);
PXIALG_API float PxiAlg_VP_GetSingleReportStaticCalPPGDP(void);
PXIALG_API float PxiAlg_VP_GetSingleReportStaticOutPPGSP(void);
PXIALG_API float PxiAlg_VP_GetSingleReportStaticOutPPGDP(void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* PxiAlg_VP_H_ */
