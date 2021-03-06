#ifndef RTOS_H
#define RTOS_H

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"


#define TASK_PRIORITY_IDLE          (tskIDLE_PRIORITY)                  /**< Idle priority */
//#define TASK_PRIORITY_LOW           (tskIDLE_PRIORITY + 1)              /**< Low priority */
#define TASK_PRIORITY_MIDDLE_LOW    (tskIDLE_PRIORITY + 2)              /**< Middle low priority */
//#define TASK_PRIORITY_MIDDLE        (tskIDLE_PRIORITY + 3)             /**< Middle priority */
#define TASK_PRIORITY_MIDDLE_HIGH   (tskIDLE_PRIORITY + 4)             /**< Middle high priority */
//#define TASK_PRIORITY_HIGH          (tskIDLE_PRIORITY + 5)             /**< High priority */
#define TASK_PRIORITY_REALTIME      (tskIDLE_PRIORITY + 6)             /**< Highest priority */

#define TASK_PRIORITY_START			(configMAX_PRIORITIES-1)
#define TASK_PRIORITY_HIGH			(configMAX_PRIORITIES-2)
#define TASK_PRIORITY_MIDDLE		(configMAX_PRIORITIES-3)
#define TASK_PRIORITY_LOW           (configMAX_PRIORITIES-4)    

#define TASK_PRIORITY_KEY			TASK_PRIORITY_HIGH
#define TASK_STACKDEPTH_KEY			(256)
#define TASK_PRIORITY_MOTOR			TASK_PRIORITY_LOW
#define TASK_STACKDEPTH_MOROT		(256)
#define TASK_PRIORITY_MAG			TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MAG			(256)
#define TASK_PRIORITY_ACCEL			TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_ACCEL		(256)
#define TASK_PRIORITY_GYRO			TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_GYRO		(256)

#define TASK_PRIORITY_MID_SCHD		TASK_PRIORITY_HIGH
#define TASK_STACKDEPTH_MID_SCHD	(1024)
#define TASK_PRIORITY_MID_BLE		TASK_PRIORITY_HIGH
#define TASK_STACKDEPTH_MID_BLE		(512)
#define TASK_PRIORITY_MID_GPS		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MID_GPS		(512)
#define TASK_PRIORITY_MID_MIC		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MID_MIC		(512)

#define TASK_PRIORITY_MOD_PWR		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MOD_PWR		(512)
#define TASK_PRIORITY_MOD_ALGO		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MOD_ALGO	(512)
#define TASK_PRIORITY_MOD_PDU		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MOD_PDU		(512)
#define TASK_PRIORITY_MOD_FLASH		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MOD_FLASH	(512)
#define TASK_PRIORITY_MOD_TIME		TASK_PRIORITY_MIDDLE
#define TASK_STACKDEPTH_MOD_TIME	(512)

#define TASK_PRIORITY_APP_WIN		TASK_PRIORITY_LOW
#define TASK_STACKDEPTH_APP_WIN		(512)
#define TASK_PRIORITY_APP_LCD		TASK_PRIORITY_LOW
#define TASK_STACKDEPTH_APP_LCD		(1024)

#define	APP_1SEC_TICK				configTICK_RATE_HZ

#endif // RTOS_H

