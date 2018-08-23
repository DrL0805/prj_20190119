#include "mid_gesture_scene.h"
#include "mid_accelerate.h"
#include "main.h"

Mid_GestureScene_Param_t	Mid_GestureScene;

//**********************************************************************
// 函数功能：   算法处理，读取重力数据回调处理
// 输入参数：   重力三轴数据
// 返回参数：	无
// 调用：睡眠算法，1秒1次
void Mid_GestureScene_algorithm(int16 *xyzData, uint32_t Interval)
{
	if(false == Mid_GestureScene.EnableFlg)
		return;	
	
	Mid_GestureScene.IntervalMs += Interval;
	if(Mid_GestureScene.IntervalMs >= GESTURE_SCENE_PERIOD_MS)
	{
		Mid_GestureScene.IntervalMs -= GESTURE_SCENE_PERIOD_MS;

		/* 调用动作识别算法 */ 
		switch(gesture_process(xyzData))
		{
			case GESTURE_NULL: 
//				MID_GESTURE_RTT_LOG(0, "GESTURE_NULL \r\n");
				break;
			case RAISE_HAND:	/* 抬手 */
				MID_GESTURE_RTT_LOG(0, "RAISE_HAND \r\n");
				break;
			case FREE_HAND:		 /* 放手 */
				MID_GESTURE_RTT_LOG(0, "FREE_HAND \r\n");
				break;
			case TURN_WRIST:	 /* 翻腕 */
				MID_GESTURE_RTT_LOG(0, "TURN_WRIST \r\n");
				break;
			case SHAKE_HAND:	 /* 甩手 */	
				MID_GESTURE_RTT_LOG(0, "SHAKE_HAND \r\n");
				switch(phoneState.state)
				{
					case PHONE_STATE_NORMAL:		
						break;					
					case PHONE_STATE_PHOTO:
						App_Protocal_TakePhoto();
						break;
					case PHONE_STATE_AUTHOR:
						phoneState.state = PHONE_STATE_NORMAL;
						App_Protocal_AuthorPass();			
						break;
					case PHONE_STATE_PAIRE:
						break;
					case PHONE_STATE_HRM:
						break;
					default: break;
				}			
				break;
			default: break;
		}
	}
}


//**********************************************************************
// 函数功能：   初始化
// 输入参数：   无
// 返回参数：	0x00:成功
// 				0xff:失败
// 调用：初始化调用一次
uint16 Mid_GestureScene_Init(void)
{
	
	return 0;
}

//**********************************************************************
// 函数功能：   
// 输入参数：   
// 返回参数：	
void Mid_GestureScene_Start(void)
{
	Mid_Accel_StartSample(&Mid_GestureScene.SampleId, eMidAccelSampleRate25HZ, eMidAccelSampleRange2G);
	
	Mid_GestureScene.IntervalMs = 0;
	Mid_GestureScene.EnableFlg = true;
}


//**********************************************************************
// 函数功能：   
// 输入参数：
// 返回参数:
void Mid_GestureScene_Stop(void)
{
	Mid_Accel_StopSample(Mid_GestureScene.SampleId);
	
	Mid_GestureScene.EnableFlg = false;
}






















