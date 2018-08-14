#include "platform_common.h"

#include "mid_interface.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_packdata_manage.h"
#include "app_scenedata_manage.h"
#include "app_rtc.h"
#include "app_task.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_action_recongnition.h"




#define  LongSitTimeInterval            30    //久坐提醒时间：单位：min 
#define  COUTER_5MIN_TIME               300   //计步、睡眠分包数据存储时间间隔：单位：秒
#define  COUTER_1MIN_TIME               60   //心率分包数据存储时间间隔：单位：秒
#define  BAT_CHECK_TIME                 600   //电池电量检测时间间隔：单位：秒
#define  HRM_LOG_CHECK_TIME             2   //心率检测时间间隔：单位：min
#define  SCENE_PROCESS_TIME             10  //场景处理时间：　单位：秒

#define  WEATHER_REQ_TIME               2   //请求ＡＰＰ下发天气信息的时间间隔，单位：min

#define  STEP_STATE_CHANGE_MOTION_VAL   30  //计步状态切换动作阀值
#define  STEP_STATE_3_SEC_TIME          6  
#define  STEP_STATE_30_SEC_TIME         60  
#define  STEP_STATE_60_SEC_TIME         120 



static uint32 appRtcSecCnt  = 0;
static uint16 appStepSecCnt = 0;
static uint8  appHourCnt    = 0;


void App_Rtc_Movt(void);
void App_Rtc_MovtRecover(void);
void App_AlarmProcess(rtc_time_s *timeTemp);
void App_LongSitRemindProcesss(rtc_time_s *timeTemp);


//**********************************************************************
// 函数功能：    半秒定时处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_RtcHalfSec_Process(void)
{
    uint16 motionValTemp;
    multimodule_task_msg_t  msg;

    switch(stepState)
    {
        case STEP_IDLE_STATE:
        motionValTemp = Mid_UsualScene_ReadMotion();
        if (motionValTemp > STEP_STATE_CHANGE_MOTION_VAL)
        {
			appStepSecCnt = 0;
			
            Mid_SportScene_SecStepClear();
            //运动关闭
            msg.id                       = SPORT_ID;
            msg.module.sportEvent.id     = SPORT_STEP_START;
            MultiModuleTask_EventSet(msg);

            //手势关闭
            msg.id                        = GESTURE_ID;
            msg.module.gestureEvent.id    = GESTURE_SCENCE_OPEN;
            MultiModuleTask_EventSet(msg);  

            stepState = STEP_PREPAIRE_STATE;
        }
        break;

        case STEP_PREPAIRE_STATE:
        appStepSecCnt ++;
        if (appStepSecCnt >= STEP_STATE_30_SEC_TIME)
        {
			 appStepSecCnt = 0;
            if(Mid_SportScene_SecStepRead())
            {
                stepState = STEP_PROCESS_STATE;
                Mid_SportScene_SecStepClear();
            }
            else
            {
                stepState = STEP_IDLE_STATE;
                //运动关闭
                msg.id                       = SPORT_ID;
                msg.module.sportEvent.id     = SPORT_STEP_STOP;
                MultiModuleTask_EventSet(msg);

                //手势关闭
                msg.id                        = GESTURE_ID;
                msg.module.gestureEvent.id    = GESTURE_SCENCE_CLOSE;
                MultiModuleTask_EventSet(msg);  
            }
        }
        break;

        case STEP_PROCESS_STATE:
        appStepSecCnt ++;
        if (appStepSecCnt >= STEP_STATE_30_SEC_TIME) //每分钟判断一次是否产生计步
        {
            appStepSecCnt = 0;

            if (Mid_SportScene_SecStepRead())
            {
                Mid_SportScene_SecStepClear();
                 stepState = STEP_PROCESS_STATE;
            }
            else
            {
                stepState = STEP_PREPAIRE_STATE;
            }
        }
        break;

        default:
        break;
    }
}

//**********************************************************************
// 函数功能：    1秒定时处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_RtcSec_Process(void)
{
    menuMessage        message;
    uint16_t          staystillTimeTemp;
    uint32_t          curUtcTimeTemp;
    uint16            stepTemp;  
    scene_data_s       sceneDataTemp;
    multimodule_task_msg_t  msg;


    appRtcSecCnt ++;
    backstageTimeCounterSec ++ ;

    //每秒检查走针
    if (movtState.state == MOVT_STATE_NORMAL)
    {
        App_Rtc_Movt();
    }else if (movtState.state == MOVT_STATE_ADJUST)
    {
        movtState.timeCnt ++;
        //校针超时处理
        if (movtState.timeCnt > movtState.timeMax)
        {
            App_Rtc_MovtRecover();
            App_Rtc_Movt();
            movtState.timeCnt = 0;
            movtState.state = MOVT_STATE_NORMAL;
        }      
    }
    else
        ;

    //时间刷新
    if (WinDowHandle == WIN_TIME_HANDLE || WinDowHandle == WIN_RUN_HANDLE || 
        WinDowHandle == WIN_CLIMB_HANDLE ||WinDowHandle == WIN_SWING_HANDLE)
    {
        message.op   = RTC_SEC;
        WinDowHandle = App_Window_Process(WinDowHandle,TAG_TIME,message);
    }  

    //可执行静置事件(后台运行日常场景) 
    staystillTimeTemp = Mid_UsualScene_StandstillTimeRead();//静置时间获取
   
    //后台处理
    //每秒检测运动时长
    if (systermConfig.stepCountSwitch == SWITCH_ON)
    {
        msg.id                       = SPORT_ID;
        msg.module.sportEvent.id     = SPORT_STEP_DUARATION_PROCESS;
        MultiModuleTask_EventSet(msg); 
    }

    //每秒更新存储后台监测心率，滤波处理
    if (systermConfig.heartrateSwitch == SWITCH_ON)
    {
        msg.id                        = HEARTRATE_LOG_ID;
        msg.module.hrmLogEvent.id     = HRM_LOG_STORAGE_PROCESS;
        MultiModuleTask_EventSet(msg);
    }

    //运动场景处理每SCENE_PROCESS_TIME秒处理一次
    if (sceneSwitch != SCENE_NULL && (appRtcSecCnt % SCENE_PROCESS_TIME == 0))
    {
		curUtcTimeTemp  = Mid_Rtc_ReadCurUtc();
        switch(sceneSwitch)
        {
            case SCENE_RUNNING:  
            //存储上一次的数据
            Mid_Scene_LogDataRead(RUN_SCENE_TYPE, &sceneDataTemp);
            App_SceneData_SaveDynamic(curUtcTimeTemp, RUN_SCENE_DATA_CLASSIFY, &sceneDataTemp);
            //启动一次新统计
            msg.id                       = SCENE_ID;
            msg.module.sceneEvent.id     = RUN_SCENE_PROCESS;
            MultiModuleTask_EventSet(msg); 
            break;
			
			case SCENE_CLIMB:
            //存储上一次的数据
            Mid_Scene_LogDataRead(CLIMB_SCENE_TYPE, &sceneDataTemp);
            App_SceneData_SaveDynamic(curUtcTimeTemp, CLIMB_SCENE_DATA_CLASSIFY, &sceneDataTemp);
            //启动一次新统计
            msg.id                       = SCENE_ID;
            msg.module.sceneEvent.id     = CLIMB_SCENE_PROCESS;
            MultiModuleTask_EventSet(msg); 
            break;

            case SCENE_SWING:
            //存储上一次的数据
            Mid_Scene_LogDataRead(SWING_SCENE_TYPE, &sceneDataTemp);
            App_SceneData_SaveDynamic(curUtcTimeTemp, SWING_SCENE_DATA_CLASSIFY, &sceneDataTemp);
            //启动一次新统计
            msg.id                       = SCENE_ID;
            msg.module.sceneEvent.id     = SWING_SCENE_PROCESS;
            MultiModuleTask_EventSet(msg); 
            break;
        }
    }

    //每5分钟存储计步、睡眠分包数据
    if (backstageTimeCounterSec % COUTER_5MIN_TIME == 0 && systermConfig.stepCountSwitch == SWITCH_ON)
    {
        curUtcTimeTemp  = Mid_Rtc_ReadCurUtc();
        stepTemp        = Mid_SportScene_5minStepRead();
        App_PackData_Step5MinSave(curUtcTimeTemp - 300, stepTemp);

        #ifndef SLEEP_PACK_DUMMY
        App_PackData_Sleep5MinSave(curUtcTimeTemp - 300, stepTemp, Mid_UsualScene_SleepMotionRead());
        #endif
        //存储后清除
        Mid_SportScene_5minStepClear();
        Mid_UsualScene_SleepMotionClear();
    }

    //每BAT_CHECK_TIME分钟检测一次电量
    if ((WinDowHandle != WIN_STORE_HANDLE) && (backstageTimeCounterSec % BAT_CHECK_TIME == 0))
    {
        msg.id                     = BAT_ID;
        msg.module.batEvent.id     = BAT_CHECK;
        MultiModuleTask_EventSet(msg);
    }

    //有与手机强关联状态
    if (phoneState.state != PHONE_STATE_NORMAL)
    {
        phoneState.timeCnt ++; //状态维持计时
        //状态超时，退出机制
        if (phoneState.timeCnt > phoneState.timeMax)
        {
             switch(phoneState.state)
            {
                case PHONE_STATE_PHOTO:
                message.state   = EXIT_TAKE_PHOTO_MODE;
                message.op      = TAKE_PHOTO_REMIND;
                WinDowHandle    = App_Window_Process(WinDowHandle,TAG_REMIND,message);
                break;

                case PHONE_STATE_AUTHOR:

                break;

                case PHONE_STATE_PAIRE:
                // message.state = EXIT_PAIRE_STATE;
                // message.op    = PAIRE_REMIND;
                // WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
                break;

                case PHONE_STATE_HRM:
                //关闭实时心率场景（启动怀关闭需要成对出现）
                msg.id                                  = HEARTRATE_ID;     
                msg.module.hrmEvent.id                  = HRM_STOP;
                MultiModuleTask_EventSet(msg); 
                break;
            }
            phoneState.state    = PHONE_STATE_NORMAL,
            phoneState.timeCnt  = 0;
        } 
    }
	
    //fix:app和蓝牙连接不同步的时候，需要重启蓝牙广播,及OTA接收超时监测
    App_Protocal_Monitor();
    //fix: 2018.6.6
}

//**********************************************************************
// 函数功能：    1分定时处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_RtcMin_Process(void)
{
    uint32              u32Temp;
    rtc_time_s          timeTemp;  
    multimodule_task_msg_t  msg;
    uint32_t            curUtcTimeTemp;
    uint8              logHR;
    uint8              resRHR;


    backstageTimeCounterMin ++;

    //后台事件
    Mid_Rtc_TimeRead(&timeTemp);

    //1.闹钟检查
    App_AlarmProcess(&timeTemp);

    //每HRM_LOG_CHECK_TIME分钟启动一次心率检测
    if ((backstageTimeCounterMin % HRM_LOG_CHECK_TIME == 0) && systermConfig.heartrateSwitch == SWITCH_ON)
    {  
        curUtcTimeTemp  = Mid_Rtc_ReadCurUtc();  
        Mid_HeartRateScene_LogHR_Read(&logHR, &resRHR);   
        App_PackData_HrmSave(curUtcTimeTemp, logHR);
        //心率监测周期性处理-周期性启动
        msg.id                        = HEARTRATE_LOG_ID;
        msg.module.hrmLogEvent.id     = HRM_LOG_PERIOND_PROCESS;
        MultiModuleTask_EventSet(msg); 
        //静息心率周期性处理
        if (backstageTimeCounterMin % LOG_RESTING_PERIOD == 0)
        {
            Mid_SportScene_StepRead(&u32Temp);
            //静息心率检测
            msg.id                        = HEARTRATE_LOG_ID;
            msg.module.hrmLogEvent.para   = (uint16)u32Temp;
            msg.module.hrmLogEvent.id     = HRM_LOG_RESTING_JUDGE;
            MultiModuleTask_EventSet(msg); 
        }
    }
    
    #if(AIR_PRESSURE_SUPPORT == 1)
    //天气处理(带气压才开启)
    if ((appHourCnt % WEATHER_REQ_TIME == 0) && systermConfig.weatherSwitch == SWITCH_ON)
    {
        //每分钟采样一次气温
        msg.id                     = WEATHER_ID;
        msg.module.weatherEvent.id = WEATHER_FORECAST_TEMP_SAMPLE;
        MultiModuleTask_EventSet(msg);  
    }
	#endif

    //久坐判断处理。计步和久坐提醒开关同时开的时候，才处理久坐提醒
    if((systermConfig.stepCountSwitch == SWITCH_ON) && (systermConfig.longSitRemindSwitch == SWITCH_ON))
    {
        App_LongSitRemindProcesss(&timeTemp);
    }
}

//**********************************************************************
// 函数功能：    1小时定时处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_RtcHour_Process(void)
{ 
    appHourCnt ++;

    #if(AIR_PRESSURE_SUPPORT == 0)
     //主动向app获取天气信息
    if ((appHourCnt % WEATHER_REQ_TIME == 0) && (bleState == BLE_CONNECT))
    {
       App_Protocal_GetWeatherProcess();
    }
    #endif
    
}

//**********************************************************************
// 函数功能：    1天定时处理
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_RtcDay_Process(void)
{ 
    app_task_msg_t  appMsg;
    
    //每天清计运动数据
     Mid_SportScene_ClearInfo();

    if (bleState == BLE_CONNECT)
    {
        App_Protocal_AdjustTimeprocess();
    }

    //每24小时封存一次计步分包目录
    if(systermConfig.stepCountSwitch == SWITCH_ON)
    {
        App_PackData_StepSaveCache();
        App_PackData_SleepSaveCache();       
    }

    //每24小时封存一次心率分包数据目录
    if (systermConfig.heartrateSwitch == SWITCH_ON)
    {
        App_PackData_HrmSaveCache();
        Mid_HeartRateScene_Clear();
    }

    //设置保存事件                                        
    appMsg.id   = APP_CACEH_DATA_SAVE;
    appMsg.para = CACHE_SPORT_TYPE;
    App_Task_EventSet(appMsg);
}

//**********************************************************************
// 函数功能：    转换时间成秒
// 输入参数：    时、分、秒
// 返回参数：    秒数
//**********************************************************************
uint16 App_Rtc_ExchangeTimeforCount(uint8 Hour, uint8 Min, uint8 Sec)
{
    uint16 z;
    uint8 i;
    i = Hour;
    if(Hour > 11)
    {
        i = Hour - 12;
    }
    z = (uint16)i *3600;
    z += (uint16)Min *60;
    z += Sec ;
    
    return z;
}

/********************************************************

                 H001系列没有秒针(两针)

分针一次走一度，一小时(3600秒)要走360度，也就是10秒走一下

********************************************************/
void App_Rtc_Movt(void)
{
#if 0
    movt_task_msg_t  movtMsg;
    rtc_time_s       timeTemp;

    Mid_Rtc_TimeRead(&timeTemp);
    movtMsg.id          = MOVT_MSG_MC_SET_AIM;
    movtMsg.aim         = App_Rtc_ExchangeTimeforCount(timeTemp.hour, timeTemp.min, timeTemp.sec)/10;
    MovtTask_EventSet(movtMsg);
#endif
}

//**********************************************************************
// 函数功能：    走针恢复
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
void App_Rtc_MovtRecover(void)
{
#if 0
    movt_task_msg_t  movtMsg;

    movtMsg.id  = MOVT_MSG_MC_RECOVER;
    MovtTask_EventSet(movtMsg);
#endif
}

//**********************************************************************
// 函数功能：    勿扰时间检测
// 输入参数：    无
// 返回参数：    无
//**********************************************************************
uint16_t App_NotDisturdTimeCheck(void)
{
    rtc_time_s  timeTemp;

    uint32_t i,j,n;
    
    Mid_Rtc_TimeRead(&timeTemp);
    //current rtc
    n = timeTemp.hour*3600 + timeTemp.min*60;
    
    i = appNotDisturdTimeInfo.StartHour*3600 + appNotDisturdTimeInfo.StartMin*60;
    j = appNotDisturdTimeInfo.StopHour *3600 + appNotDisturdTimeInfo.StopMin *60;
    
    if(TimePeriodJudge(i,j,n))
    return 0xff;
    
    return 0;
}

//**********************************************************************
// 函数功能：    久坐时间检测
// 输入参数：    rtc时间
// 返回参数：    无
//**********************************************************************
uint16_t App_LongSitTimeCheck(rtc_time_s *timeTemp)
{
    uint32_t i,j,k,m,n;
    
    //当前rtc
    n = timeTemp->hour*3600 + timeTemp->min*60;
    //久坐提醒生效时间段
    i = appLongSitInfo.StartTimeHour*3600 + appLongSitInfo.StartTimeMin*60;
    j = appLongSitInfo.StopTimeHour *3600 + appLongSitInfo.StopTimeMin*60;
    //久坐提醒勿扰时间段
    k = appLongSitInfo.DisturdStartTimehour*3600 + appLongSitInfo.DisturdStartTimeMin*60;
    m = appLongSitInfo.DisturdStopTimehour *3600 + appLongSitInfo.DisturdStopTimeMin*60;
    if(TimePeriodJudge(i,j,n)) //在生效时间内
    {
        if(TimePeriodJudge(k,m,n))//在勿扰时间段
        {
            return 0;
        }
        else
        return 0xff;
    }
    return 0;
}

//**********************************************************************
// 函数功能：    久坐提醒判断处理
// 输入参数：    rtc时间
// 返回参数：    无
//**********************************************************************
void App_LongSitRemindProcesss(rtc_time_s *timeTemp)
{
    uint16_t     LongSitTime;
    uint32_t     LongSitMotion;
    menuMessage  message;
   
    //long sit
    LongSitTime   = Mid_UsualScene_SedentaryTimeRead();
    LongSitMotion = Mid_UsualScene_SedentaryMotionRead();          
    if(LongSitTime >= appLongSitInfo.intv_mimute)                //久坐时间大于设置的时间间隔
    {
        if((LongSitMotion / appLongSitInfo.intv_mimute / 60) > 2)//久坐动作除以间隔时间大于2
        {
            if(App_LongSitTimeCheck(timeTemp))      //是否在久坐可提醒时间段
            {
                message.op   = LONG_SIT_REMIND;
                WinDowHandle = App_Window_Process(WinDowHandle,TAG_REMIND,message);
            }
        }
        Mid_UsualScene_SedentaryClear();
    } 
}
                        
//**********************************************************************
// 函数功能：    闹钟响闹检测，每分钟检测一次
// 输入参数：    rtc时间
// 返回参数：    无
//**********************************************************************
void App_AlarmProcess(rtc_time_s *timeTemp)
{
    menuMessage         message;

    if(Mid_AlarmClock_Check(timeTemp))
    { 
        //拍照不来闹钟提醒
        if (phoneState.state != PHONE_STATE_PHOTO)
        {
             message.op   = ALARM_REMIND;
            WinDowHandle = App_Window_Process(WinDowHandle,TAG_REMIND,message);
        }
    }
}


