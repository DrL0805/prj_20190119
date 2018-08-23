#ifndef PXIALG_RR_H__
#define PXIALG_RR_H__

#include <stdint.h>

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


PXIALG_API void RR_Open(int fps, int fps_biopac, int flag_lowRR);
PXIALG_API void RR_Close(void);       
PXIALG_API int RR_Process(float raw_data, int* RR, float* Reliability, char* memory_alloc);	
PXIALG_API int RR_Biopac(float raw_data, int* RR, float* Reliability, char* memory_alloc);
PXIALG_API void RR_Filtered(float* raw_data);
PXIALG_API int RR_Version(void);
PXIALG_API int RR_Memory_Size(void);

PXIALG_API void RR8002_Open(int fps, int fps_biopac);
PXIALG_API void RR8002_Close(void);       
PXIALG_API int RR8002_Process(float* raw_data, float* raw_data2, float* IR_data, float* mems_data, int* RR, float* Reliability, int num_ppg, int num_mems, char* memory_alloc);	
//PXIALG_API int RR8002_Process(float* raw_data, float* raw_data2, float* IR_data, float* mems_data, int* RR, float* Reliability, int num_ppg, int num_mems);	

PXIALG_API void RR8002_Biopac(float raw_data, int* RR, float* Reliability);
PXIALG_API void RR8002_Filtered(float* raw_data, float* raw_data2, int num_ppg);
PXIALG_API int RR8002_Version(void);
PXIALG_API int RR8002_Memory_Size(void);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif
