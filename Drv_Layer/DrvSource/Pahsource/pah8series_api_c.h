#ifndef __PAH8SERIES_API_C_H__
#define __PAH8SERIES_API_C_H__


#include "pah8series_data_c.h"


#if defined(WIN32) && !defined(PXIALG_STATIC_LIB)
#   ifdef PXIALG_EXPORTS
#       define PXIALG_API   __declspec(dllexport)
#   else
#       define PXIALG_API   __declspec(dllimport)
#   endif
#else
#    define PXIALG_API
#endif	// WIN32


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


PXIALG_API uint32 pah8series_version(void);
PXIALG_API uint32 pah8series_query_open_size(void);
PXIALG_API uint8  pah8series_open(void *pBuffer);
PXIALG_API uint8  pah8series_close(void);
PXIALG_API uint8  pah8series_reset(void);
PXIALG_API uint8  pah8series_set_param(pah8series_param_idx_t idx, float p1);
PXIALG_API uint8  pah8series_get_param(pah8series_param_idx_t idx, float *p1);
PXIALG_API uint8  pah8series_get_hr(float *hr);
PXIALG_API uint8  pah8series_get_hr_trust_level(int *hr_trust_level);
PXIALG_API uint8  pah8series_entrance(pah8series_data_t *pah8series_data);
PXIALG_API uint8  pah8series_set_data(pah8series_data_t *pah8series_data);
PXIALG_API uint8  pah8series_process(uint8 *is_finished);
PXIALG_API uint8  pah8series_get_signal_grade(int16 *grade);
PXIALG_API uint8  pah8series_get_display_buffer(int32 **buffer1, int32 **buffer2, int32 *size);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif
