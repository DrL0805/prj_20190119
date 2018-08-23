#ifndef PXIALG_H__
#define PXIALG_H__
#include "stdint.h"


#define PXIALG_API


typedef enum {
    FLAG_DATA_READY = 0,
    FLAG_DATA_NOT_READY,
    FLAG_DATA_LOSS,
    FLAG_NO_TOUCH,
    FLAG_DATA_ERROR,
    FLAG_POOR_SIGNAL,
    FLAG_FIFO_ERROR,
    FLAG_TIMMING_ERROR,
    FLAG_MEMS_ERROR,
    FLAG_HR_UPDATE = 16,
    FLAG_SG_UPDATE,
} PXI_STATUS_FLAG;

/**
* @brief When HRD and MEMS data are ready, call this function to do the algorithm processing
* @param[in] HRD_Data Pointer to the buffer where HRD data (13 Bytes) is stored.
* @param[in] MEMS_Data Pointer to the buffer where MEMS data (3*sizeof(float) Bytes) is stored.
* @return one of the PXI_STATUS_FLAG types.
*/
PXIALG_API int32_t PxiAlg_Process(unsigned char *HRD_Data, float *MEMS_Data);

/**
* @brief Call this function to get Heart Rate
* @param[out] hr Pointer to a float variable where heart rate is stored
* @return None
*/
PXIALG_API void PxiAlg_HrGet(float *hr);

/**
* @brief Call this function to determine the version of the algorithm
* @return Version of the algorithm
*/
//Note: Recommend call PxiAlg_Version() function to get the version of the algorithm first. If return value is 1025, it means the version is 25th version. If return value is 0, it means unconnected the library.
PXIALG_API int32_t PxiAlg_Version(void) ;

/**
* @brief Call this function to get Ready_Flag
* @return Ready_Flag
*/
PXIALG_API unsigned char PxiAlg_GetReadyFlag(void);

/**
* @brief Call this function to get Motion_Flag
* @return Motion_Flag
*/
PXIALG_API unsigned char PxiAlg_GetMotionFlag(void);

/**
* @brief Call this function to notify algorithm the MEMS Scale of Motion Sensor
* @param[in] scale The MEMS Scale of Motion Sensor. Only 0(±2G), 1(±4G, ±8G, ±16G) are supported.
* @return 1 for success. 0 for failure.
*/
PXIALG_API uint8_t PxiAlg_SetMemsScale(int scale);

/**
* @brief Call this function to get PPG Signal Grade. High signal grade is meant high signal at steady state.
* @param[out] grade Pointer to a float variable where signal grade is stored.
* @return Return 1 when Signal Grade is ready. Otherwise, return 0.
*/
PXIALG_API uint8_t PxiAlg_GetSigGrade(float *grade);

/**
* @brief Call this function to set PPG Signal Grade Threshold. Default Signal Grade Threshold is 40.
* @param[in] threshold The PPG Signal Grade Threshold. Its value ranges from 0 to 100.
* @return 1 for success. 0 for failure.
*/
PXIALG_API uint8_t PxiAlg_SetSigGradeThrd(float thrd);

/**
* @brief Call this function to enable or disable fast output mode
* @param[in] enable The flag of fast output mode.
* @when enable fast output mode, 1st HR value is output about 6 sec.
* @For getting high accuracy, please don’t move in the beginning.
*/
PXIALG_API void PxiAlg_EnableFastOutput(uint8_t en);

/**
* @brief Call this function to enable or disable motion mode. Default is disable motion mode.
* @param[in] enable The flag of motion mode.
* @enable motion mode for whole body movement: such as running
* @disable motion mode for parts body movement: such as car driving, weight lifting, slow walking, bicycling
*/
PXIALG_API void PxiAlg_EnableMotionMode(uint8_t en);

/**
* @brief Call this function to enable or disable auto mode. Default is enable auto mode.
* @param[in] enable The flag of auto mode.
*/
PXIALG_API void PxiAlg_EnableAutoMode(uint8_t en);

/**
* @brief Call this function to open algorithm
*/
PXIALG_API void PxiAlg_Open(void);

/**
* @brief Call this function to close/reset algorithm
*/
PXIALG_API void PxiAlg_Close(void);

/**
* @brief Call this function to get the alarm flag of fast output
* @param[out] get the alarm flag of fast output when signal is poor during 60 seconds in the beginning.
* @return Return 1 when the flag is set. Otherwise, return 0.
*/
PXIALG_API uint8_t PxiAlg_GetFastOutAlarmFlag(void);

/**
* @brief Call this function to enable or disable mems0 signal grade mode for factory testing
* @param[in] 1 : enable. 0 : disable.
*/
PXIALG_API void PxiAlg_EnableMEMS0SigGrade(uint8_t en);

/**
* @brief Call this function to get the alarm flag of signal bad
* @param[out] get the alarm flag of bad signal
* @return Return 1 when the flag is set. Otherwise, return 0.
* @when get this flag=1, host could reset algorithm and stop heart calculation.
*/
PXIALG_API uint8_t PxiAlg_GetSignalBadAlarm(void);

/**
* @brief Call this function to set PPG buffer pointer and size
* @param[in] ppg_buffer Pointer to a float buffer where the PPG data would be stored
* @param[in] size PPG buffer size
* @return None
*/
PXIALG_API void PxiAlg_SetPPGBuffer(float *ppg_buffer, int32_t size);

/**
 * @brief Call this function to feed Algorithm raw data(19 Bytes), and get Signal Grade and Heart Rate
 *
 * @param[in] pixart_19_byte_data   Pointer to a arrag where raw data is stored
 *  Format: 13 bytes PPG + G-sensor X_L[13] + X_H[14] + Y_L[15] + Y_H[16] + Z_L[17] + Z_H[18]
 * @param[out] grade    Pointer to a float variable where signal grade is stored.
 * @param[out] heart_rate   Pointer to a float variable where heart rate is stored
 *
 * @return status bit field, combinatoin of the list below
 * 1 << FLAG_DATA_READY,
 * 1 << FLAG_DATA_NOT_READY,
 * 1 << FLAG_DATA_LOSS,
 * 1 << FLAG_NO_TOUCH,
 * 1 << FLAG_DATA_ERROR,
 * 1 << FLAG_POOR_SIGNAL,
 * 1 << FLAG_FIFO_ERROR,
 * 1 << FLAG_TIMMING_ERROR,
 * 1 << FLAG_MEMS_ERROR,
 * 1 << FLAG_HR_UPDATE,
 * 1 << FLAG_SG_UPDATE,
 */
PXIALG_API uint32_t PxiAlg_HRM(uint8_t *pixart_19_byte_data, float *grade, float * heart_rate);

/**
 * @brief Call this function to enable/disable Check_Not_Stable_First_HR
 *
 * @param[in] 1 : enable, 0: disable
 */
PXIALG_API void PxiAlg_Check_Not_Stable_First_HR(uint8_t check); 

/**
 * @brief Call this function to set Signal_Grade_Large_Energy_Num_Th_Lo
 *
 * @param[in] Threshold. The higher input value, the less stronger de-ambient light effect, but faster HR convergence time. Default 95.
 *
 */
PXIALG_API void PxiAlg_Set_Signal_Grade_Large_Energy_Num_Th_Lo(uint8_t th) ;


PXIALG_API uint8_t PxiAlg_DecodeProcess(uint32_t data, int touch_flag, float time_in);
PXIALG_API void PxiAlg_Open_SamplingRate(int32_t sampling_rate);

#endif /* PXIALG_H__ */
