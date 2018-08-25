#ifndef PLATFORM_COMMON_H
#define PLATFORM_COMMON_H

#include "platform_debugcof.h"
#include "platform_feature.h"

#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#ifdef keil   //keil
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t  uint64;
typedef uint32_t  uint32;
typedef uint16_t  uint16;
typedef uint8_t   uint8;

typedef int64_t  int64;
typedef int32_t  int32;
typedef int16_t  int16;
typedef int8_t   int8;
#endif

#define TRUE  1
#define FALSE 0

typedef enum
{
	Ret_OK,            
	Ret_Fail,          
	Ret_InvalidParam,  
	Ret_DeviceBusy,    
	Ret_NoInit, 
	Ret_NoDevice,
	Ret_QueueFull,   //?иоивD?y
	Ret_QueueEmpty,  //?иоивD??
	Ret_DeviceError,
}ret_type;

typedef void comm_cb(void);
typedef	void func(void);

#endif

