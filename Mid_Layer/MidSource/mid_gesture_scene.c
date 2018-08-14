#include "platform_common.h"
#include "algorithm_gesture.h"
#include "mid_gesture_scene.h"
#include "multimodule_task.h"



/*************** func declaration ********************************/
static void   GestureAccelDataProcess(int16 *xyzData);
static uint16 GestureSceneOpen(void);
static uint16 GestureSceneClose(void);

void (*GestureFunIsrCb)(uint8 gesturetye);
static void   (*ActionIsrCb)(uint8 *xyzData);


/*************** macro define ********************************/


/************** variable define *****************************/
static uint16     gestureAccelReadId;
static uint8 	  freqCnt;



/************** function define ******`***********************/

//**********************************************************************
// 函数功能：   计步算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
static void GestureAccelDataProcess(int16 *xyzData)
{	
	uint16    accelRateTemp;
	uint8     accelRangeTemp   = 0;
    gesture_type gestureAct = GESTURE_NULL;
    uint8     actionBuf[3];
	freqCnt ++;
	//检测重力计量程并实现更新
    Mid_Accel_SettingRead(&accelRateTemp, &accelRangeTemp);


    if (accelRateTemp == ACCEL_25HZ)
    {
        if (freqCnt % 5 == 0)
        {
            gestureAct = gesture_process(xyzData);
            freqCnt    = 0;
        }
    }else if (accelRateTemp >= ACCEL_50HZ)
    {
        if (freqCnt % 10 == 0)
        {
            gestureAct = gesture_process(xyzData);
            freqCnt    = 0;
        }
    }
    else
    {
        gestureAct = gesture_process(xyzData);
        freqCnt    = 0;
    }

    if(gestureAct == RAISE_HAND && GestureFunIsrCb != NULL)
    {
        GestureFunIsrCb(GESTURE_HAND_LIFT);
    }	

    //动作识别回调
    for (int i = 0; i < 3; i++)
    {
        actionBuf[i] = (uint8)(xyzData[i] >> 8);
    }
    if (ActionIsrCb != NULL)
    {
        ActionIsrCb(actionBuf);
    }
}

//**********************************************************************
// 函数功能：   手势识别信息初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
uint16 Mid_GestureScene_Init(void(*Cb)(uint8 gesturetye),void(*actionCb)(uint8 *xyzdata))
{
	freqCnt  		= 0;
	GestureFunIsrCb = Cb;
    ActionIsrCb     = actionCb;
	return 0;
}

//**********************************************************************
// 函数功能：   手势识别开启
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 GestureSceneOpen(void)
{
	multimodule_task_msg_t  msg;
    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_SET;
    msg.module.accelEvent.readId      = &gestureAccelReadId;
    msg.module.accelEvent.rate        = ACCEL_25HZ;
    msg.module.accelEvent.scaleRange  = ACCEL_2G;
    msg.module.accelEvent.Cb          = GestureAccelDataProcess;
    MultiModuleTask_EventSet(msg);

    return 0;
}

//**********************************************************************
// 函数功能：   手势识别关闭
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
static uint16 GestureSceneClose(void)
{
	multimodule_task_msg_t  msg;

    msg.id                            = ACCEL_ID;
    msg.module.accelEvent.id          = ACCEL_READ_DELETE;
    msg.module.accelEvent.readId      = &gestureAccelReadId;
    MultiModuleTask_EventSet(msg);
    return 0;
}


//**********************************************************************
// 函数功能:    心率模块事件处理
// 输入参数：    无
// 返回参数：    0x00: success
//              0xff: fail
uint16 Mid_GestureScene_EventProcess(gesture_event_s *msg)
{
     switch(msg->id)
     {
        case GESTURE_SCENCE_OPEN:
         GestureSceneOpen();
         break;

         case GESTURE_SCENCE_CLOSE:
         GestureSceneClose();
         break;

         default:
         break;
    }
    return 0x00;
}
