#ifndef     MID_INTERFACE_H
#define     MID_INTERFACE_H

//#include "bsp_interface.h"

/* FreeRTOS includes */
#include "rtos.h"

#include "mid_rtc.h"
#include "mid_alarm.h"
#include "mid_countdown.h"
#include "mid_extflash.h"
#include "mid_front.h"
#include "mid_accelerate.h"
#include "mid_gyroscope.h"
#include "mid_magnetism.h"
#include "mid_hrm.h"
#include "mid_key.h"
#include "mid_singleport.h"
#include "mid_stopwatch.h"
#include "mid_uart.h"
#include "mid_pressure.h"
#include "mid_bat.h"

#include "algorithm_lis3dh.h"
#include "algorithm_mag_degree.h"
#include "algorithm_sleep.h"
#include "algorithm_usual.h"
#include "algorithm_running.h"

#include "mid_compass.h"
#include "mid_sport_scene.h"
#include "mid_gesture_scene.h"
#include "mid_heartrate_scene.h"
#include "mid_usual_scene.h"
#include "mid_weather_scene.h"
#include "mid_sunset_scene.h"
#include "mid_climb_scene.h"
#include "mid_heartrate_scene.h"
#include "mid_sleep_scene.h"
#include "mid_scene.h"


#include "system_task.h"
#include "movt_task.h"
#include "flash_task.h"
#include "ble_task.h"
#include "multimodule_task.h"

#include "mid_packdata_manage.h"
#include "mid_remind_manage.h"
#include "mid_sleepdata_manage.h"
#include "mid_cachedata_manage.h"
#include "mid_scenedata_manage.h"

#include "mid_ble_linkInv_manage.h"

#include "mid_screen.h"
#include "BLE_Stack.h" 

#endif      //MID_INTERFACE_H


