#include "platform_common.h"

#include "mid_interface.h"




/*************** func declaration ********************************/
static uint16 ClimbFuncOpen(void);
static uint16 ClimbFuncClose(void);
static uint16 ClimbFloorCheck(uint32 curStep);

/*************** macro define ********************************/
#define 	STEP_VALID 				10//判断爬梯计步门限
#define 	CLIMB_CHECK_P1 			30//30秒首次检测判断

#define 	FLOOR_HEIGHT 			400//楼层高度，单位cm

/************** variable define *****************************/
int32 	curClimbHeight; 		 //记录当前高度
int32 	baseFloorHeight; 		 //记录当前楼层基点高度
uint8 	floorTotal; 			 //总楼层数
uint32  	logStep; 				 //计步日志

static uint8 climbFuncFlag; 		//功能打开标记

static uint8 firstData; 			//初始数据标志
 
static uint8 	climbFuncId; 			//注册压力功能的id


/************** function define *****************************/

//**********************************************************************
// 函数功能：  爬楼场景初始化
// 输入参数：   
// 返回参数：  无
uint16 Mid_ClimbScene_Init(void)
{
	curClimbHeight 		= 0;
	baseFloorHeight 	= 0;
	floorTotal 			= 0;
	logStep 			= 0;
	firstData 			= 1;
	climbFuncFlag 	 	= 0;

	return 0;
}

//**********************************************************************
// 函数功能：  爬楼功能打开
// 输入参数：   
// 返回参数：  无
static uint16 ClimbFuncOpen(void)
{
	multimodule_task_msg_t msg;

	firstData 			= 1;
	climbFuncFlag 		= 1;

	//新注册压力功能事件
    msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_REGISTER;
	msg.module.pressureEvent.funcId 		= &climbFuncId;
	msg.module.pressureEvent.cvtfrq 		= PRESSURE_8HZ;//转换频率
	msg.module.pressureEvent.osr 			= RATE_128;//采样率
	msg.module.pressureEvent.paratype 		= ALL_PARA;//获取参数
    MultiModuleTask_EventSet(msg);

	return 0;
}

//**********************************************************************
// 函数功能：  爬楼功能关闭
// 输入参数：   
// 返回参数：  无
static uint16 ClimbFuncClose(void)
{
	multimodule_task_msg_t msg;

	climbFuncFlag 		= 0;

	//注销压力功能事件
	msg.id                                  = PRESSURE_ID;     
    msg.module.pressureEvent.id             = PRESSURE_FUNC_UN_REGISTER;
	msg.module.pressureEvent.funcId 		= &climbFuncId;
	MultiModuleTask_EventSet(msg); 
	return 0;
}

//**********************************************************************
// 函数功能：  检测楼层处理
// 输入参数：   
//curStep  :   计步值
// 返回参数：  无
static uint16 ClimbFloorCheck(uint32 curStep)
{
	int32  	heightJistter;
	uint8  	floorJistter;
	int32  	stepJistter;

	if (climbFuncFlag)
	{
		if (firstData)
		{
			Mid_Pressure_ReadAltitude(&curClimbHeight);
			baseFloorHeight 	= curClimbHeight;
			floorTotal 			= 0;
			logStep 			= curStep;
			firstData 			= 0;
		}
		else
		{
			Mid_Pressure_ReadAltitude(&curClimbHeight);

			heightJistter 		= curClimbHeight - baseFloorHeight;//高度偏离量

			if (heightJistter <= 0)//下
			{
				baseFloorHeight 	= curClimbHeight;//更新基础高度
			}
			else//上
			{
				stepJistter 		= curStep - logStep;

				if (heightJistter >= FLOOR_HEIGHT)
				{
					if (stepJistter > STEP_VALID)//判断因运动产生的高度变化 
					{
						floorJistter 		= (heightJistter / FLOOR_HEIGHT);//楼层增量
						floorTotal 			+= floorJistter;
						baseFloorHeight 	+= floorJistter * FLOOR_HEIGHT;
					}
					else
					{
						baseFloorHeight 	= curClimbHeight;//更新基础高度					
					}			
				}	
			}
			logStep 				= curStep;
		}
		return 0;
	}
	return 0xff;
}

//**********************************************************************
// 函数功能：  爬楼场景事件处理
// 输入参数：   
// 返回参数：  无
uint16 Mid_ClimbScene_EventProcess(climb_event_s* msg)
{
	switch(msg->id)
	{
		case CLIMB_FUNC_OPEN:
		ClimbFuncOpen();
		break;

		case CLIMB_FUNC_CLOSE:
		ClimbFuncClose();
		break;

		case CLIMB_FLOOR_CHECK:
		ClimbFloorCheck(msg->para);
		break;

		default:
		break;
	}
	return 0;
}

//**********************************************************************
// 函数功能：  读取爬楼层数
// 输入参数：   
// 返回参数：  无
uint16 Mid_ClimbScene_ReadFloor(uint8 *floor)
{
	*floor  = floorTotal;

	return 0;
}

