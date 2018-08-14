#include "platform_common.h"
#include "platform_debugcof.h"
/* FreeRTOS includes */
#include "rtos.h"

/* FreeRTOS includes */
#include "rtos.h"

#include "mid_interface.h"
#include "mid_common.h"

#include "drv_movt.h"
#include "movt_task.h"

#if(SUPPORT_MOVT == 1)

static QueueHandle_t		movt_task_msg_queue;				/**< Queue handler for movt task */

uint16 MovtTask_EventSet(movt_task_msg_t msg)
{
	uint16 ret = 0;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	switch(msg.id)
	{

// clock MIDDLE
		case MOVT_MSG_MC_FORWARD:
		// break;
		
		case MOVT_MSG_MC_REVERSE:
		// break;

		case MOVT_MSG_MC_FORWARDING:
		// break;

		case MOVT_MSG_MC_REVERSEING:
		// break;

		case MOVT_MSG_MC_SET_CUR:
		// break;

		case MOVT_MSG_MC_SET_AIM:
		// break;

		case MOVT_MSG_MC_SET_CUR_AIM:
		// break;
		case MOVT_MSG_MC_SET_CUR_FORWARD:
		case MOVT_MSG_MC_SET_AIM_FORWARD:
		case MOVT_MSG_MC_SET_CUR_AIM_FORWARD:
		case MOVT_MSG_MC_SET_CUR_REVERSE:
		case MOVT_MSG_MC_SET_AIM_REVERSE:
		case MOVT_MSG_MC_SET_CUR_AIM_REVERSE:

		case MOVT_MSG_MC_STOP:
		case MOVT_MSG_MC_RECOVER:
		case MOVT_MSG_MC_RECOVER_FORWARD:
		case MOVT_MSG_MC_RECOVER_REVERSE:
		xQueueSendToBack(movt_task_msg_queue, &msg, 1);
		break;

		case MOVT_MSG_MC_READ_CUR:
		return (Drv_Movt_ReadPositionInfo(MOVT_M_CLOCK,MOVT_SET_MC_READ_CUR));


		case MOVT_MSG_MC_READ_AIM:
		return (Drv_Movt_ReadPositionInfo(MOVT_M_CLOCK,MOVT_SET_MC_READ_AIM));


		case MOVT_MSG_MC_FORWARD_FINISH:
		// break;

		case MOVT_MSG_MC_REVERSE_FINISH:
		if (errQUEUE_FULL == xQueueSendToFrontFromISR( movt_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
		{
			// ("errQUEUE_FULL");
		}
		if( pdTRUE ==  xHigherPriorityTaskWoken )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		break;


// clock 3
		case MOVT_MSG_3C_FORWARD:
		// break;
		
		case MOVT_MSG_3C_REVERSE:
		// break;

		case MOVT_MSG_3C_FORWARDING:
		// break;

		case MOVT_MSG_3C_REVERSEING:
		// break;

		case MOVT_MSG_3C_SET_CUR:
		// break;

		case MOVT_MSG_3C_SET_AIM:
		// break;

		case MOVT_MSG_3C_SET_CUR_AIM:
		// break;

		case MOVT_MSG_3C_SET_CUR_FORWARD:
		case MOVT_MSG_3C_SET_AIM_FORWARD:
		case MOVT_MSG_3C_SET_CUR_AIM_FORWARD:
		case MOVT_MSG_3C_SET_CUR_REVERSE:
		case MOVT_MSG_3C_SET_AIM_REVERSE:
		case MOVT_MSG_3C_SET_CUR_AIM_REVERSE:

		case MOVT_MSG_3C_STOP:
        case MOVT_MSG_3C_RECOVER:
        case MOVT_MSG_3C_RECOVER_FORWARD:
		case MOVT_MSG_3C_RECOVER_REVERSE:
		xQueueSendToBack(movt_task_msg_queue, &msg, 1);
		break;

		case MOVT_MSG_3C_READ_CUR:
		return (Drv_Movt_ReadPositionInfo(MOVT_3_CLOCK,MOVT_SET_3C_READ_CUR));


		case MOVT_MSG_3C_READ_AIM:
		return (Drv_Movt_ReadPositionInfo(MOVT_3_CLOCK,MOVT_SET_3C_READ_AIM));


		case MOVT_MSG_3C_FORWARD_FINISH:
		// break;

		case MOVT_MSG_3C_REVERSE_FINISH:
		if (errQUEUE_FULL == xQueueSendToFrontFromISR( movt_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
		{
			// ("errQUEUE_FULL");
		}
		if( pdTRUE ==  xHigherPriorityTaskWoken )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		break;

// clock 6
		case MOVT_MSG_6C_FORWARD:
		// break;
		
		case MOVT_MSG_6C_REVERSE:
		// break;

		case MOVT_MSG_6C_FORWARDING:
		// break;

		case MOVT_MSG_6C_REVERSEING:
		// break;

		case MOVT_MSG_6C_SET_CUR:
		// break;

		case MOVT_MSG_6C_SET_AIM:
		// break;

		case MOVT_MSG_6C_SET_CUR_AIM:
		// break;

		case MOVT_MSG_6C_SET_CUR_FORWARD:
		case MOVT_MSG_6C_SET_AIM_FORWARD:
		case MOVT_MSG_6C_SET_CUR_AIM_FORWARD:
		case MOVT_MSG_6C_SET_CUR_REVERSE:
		case MOVT_MSG_6C_SET_AIM_REVERSE:
		case MOVT_MSG_6C_SET_CUR_AIM_REVERSE:

		case MOVT_MSG_6C_STOP:
        case MOVT_MSG_6C_RECOVER:
        case MOVT_MSG_6C_RECOVER_FORWARD:
		case MOVT_MSG_6C_RECOVER_REVERSE:
		xQueueSendToBack(movt_task_msg_queue, &msg, 1);
		break;

		case MOVT_MSG_6C_READ_CUR:
		return (Drv_Movt_ReadPositionInfo(MOVT_6_CLOCK,MOVT_SET_6C_READ_CUR));


		case MOVT_MSG_6C_READ_AIM:
		return (Drv_Movt_ReadPositionInfo(MOVT_6_CLOCK,MOVT_SET_6C_READ_AIM));


		case MOVT_MSG_6C_FORWARD_FINISH:
		// break;

		case MOVT_MSG_6C_REVERSE_FINISH:
		if (errQUEUE_FULL == xQueueSendToFrontFromISR( movt_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
		{
			// ("errQUEUE_FULL");
		}
		if( pdTRUE ==  xHigherPriorityTaskWoken )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		break;

		// clock 9
		case MOVT_MSG_9C_FORWARD:
		// break;
		
		case MOVT_MSG_9C_REVERSE:
		// break;

		case MOVT_MSG_9C_FORWARDING:
		// break;

		case MOVT_MSG_9C_REVERSEING:
		// break;

		case MOVT_MSG_9C_SET_CUR:
		// break;

		case MOVT_MSG_9C_SET_AIM:
		// break;

		case MOVT_MSG_9C_SET_CUR_AIM:
		// break;
		case MOVT_MSG_9C_SET_CUR_FORWARD:
		case MOVT_MSG_9C_SET_AIM_FORWARD:
		case MOVT_MSG_9C_SET_CUR_AIM_FORWARD:
		case MOVT_MSG_9C_SET_CUR_REVERSE:
		case MOVT_MSG_9C_SET_AIM_REVERSE:
		case MOVT_MSG_9C_SET_CUR_AIM_REVERSE:

		case MOVT_MSG_9C_STOP:
        case MOVT_MSG_9C_RECOVER:
        case MOVT_MSG_9C_RECOVER_FORWARD:
		case MOVT_MSG_9C_RECOVER_REVERSE:
		xQueueSendToBack(movt_task_msg_queue, &msg, 1);
		break;

		case MOVT_MSG_9C_READ_CUR:
		return (Drv_Movt_ReadPositionInfo(MOVT_9_CLOCK,MOVT_SET_9C_READ_CUR));


		case MOVT_MSG_9C_READ_AIM:
		return (Drv_Movt_ReadPositionInfo(MOVT_9_CLOCK,MOVT_SET_9C_READ_AIM));


		case MOVT_MSG_9C_FORWARD_FINISH:
		// break;

		case MOVT_MSG_9C_REVERSE_FINISH:
		if (errQUEUE_FULL == xQueueSendToFrontFromISR( movt_task_msg_queue, (void *) &msg, &xHigherPriorityTaskWoken ))
		{
			// ("errQUEUE_FULL");
		}
		if( pdTRUE ==  xHigherPriorityTaskWoken )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		break;


		default:
		break;
	}
	return ret;
}

void ClockMFrowardFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_MC_FORWARD_FINISH;
	MovtTask_EventSet(msg);
}

void Clock3FrowardFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_3C_FORWARD_FINISH;
	MovtTask_EventSet(msg);
}

void Clock6FrowardFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_6C_FORWARD_FINISH;
	MovtTask_EventSet(msg);
}

void Clock9FrowardFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_9C_FORWARD_FINISH;
	MovtTask_EventSet(msg);
}

void ClockMResverseFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_MC_REVERSE_FINISH;
	MovtTask_EventSet(msg);
}

void Clock3ResverseFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_3C_REVERSE_FINISH;
	MovtTask_EventSet(msg);
}

void Clock6ResverseFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_6C_REVERSE_FINISH;
	MovtTask_EventSet(msg);
}

void Clock9ResverseFinishIsr(void)
{
	movt_task_msg_t msg;

	msg.id		= MOVT_MSG_9C_REVERSE_FINISH;
	MovtTask_EventSet(msg);
}

// TimerHandle_t movtTaskWatchdogTimer;

// void MovtTaskWdtPeriodProcess(TimerHandle_t xTimer)
// {
// 	movt_task_msg_t msg;

// 	msg.id		= MOVT_TASK_WATCHDOG;
// 	xQueueSendToBack(movt_task_msg_queue, &msg, 1);
// }

// init the task, the drive interface
void MovtTask(void *pvParameters)
{
	movt_task_msg_t msg;

	Drv_Movt_Init();
	Drv_Movt_SetFrowardFinishCb(MOVT_M_CLOCK,ClockMFrowardFinishIsr);	
	Drv_Movt_SetReverseFinishCb(MOVT_M_CLOCK,ClockMResverseFinishIsr);

	Drv_Movt_SetFrowardFinishCb(MOVT_3_CLOCK,Clock3FrowardFinishIsr);	
	Drv_Movt_SetReverseFinishCb(MOVT_3_CLOCK,Clock3ResverseFinishIsr);

	Drv_Movt_SetFrowardFinishCb(MOVT_6_CLOCK,Clock6FrowardFinishIsr);	
	Drv_Movt_SetReverseFinishCb(MOVT_6_CLOCK,Clock6ResverseFinishIsr);

	Drv_Movt_SetFrowardFinishCb(MOVT_9_CLOCK,Clock9FrowardFinishIsr);	
	Drv_Movt_SetReverseFinishCb(MOVT_9_CLOCK,Clock9ResverseFinishIsr);


	movt_task_msg_queue = xQueueCreate(MOVT_TASK_QUEUE_SIZE, sizeof(movt_task_msg_t));

	for(;;)
	{
		if(xQueueReceive(movt_task_msg_queue, &msg, portMAX_DELAY))
		{
			switch(msg.id)
			{
// clock M
				case MOVT_MSG_MC_FORWARD:
				Drv_Movt_Forward(MOVT_M_CLOCK);
				break;
				
				case MOVT_MSG_MC_REVERSE:
				Drv_Movt_Reverse(MOVT_M_CLOCK);
				break;

				case MOVT_MSG_MC_FORWARDING:
				Drv_Movt_Forwarding(MOVT_M_CLOCK);
				break;

				case MOVT_MSG_MC_REVERSEING:
				Drv_Movt_Reversing(MOVT_M_CLOCK);
				break;

				case MOVT_MSG_MC_SET_CUR:
				case MOVT_MSG_MC_SET_CUR_FORWARD:
				case MOVT_MSG_MC_SET_CUR_REVERSE:
				Drv_Movt_SetCurAction(MOVT_M_CLOCK,msg.id,msg.cur);
				break;

				case MOVT_MSG_MC_SET_AIM:
				case MOVT_MSG_MC_SET_AIM_FORWARD:
				case MOVT_MSG_MC_SET_AIM_REVERSE:
				Drv_Movt_SetAimAction(MOVT_M_CLOCK,msg.id,msg.aim);
				break;

				case MOVT_MSG_MC_SET_CUR_AIM:
				case MOVT_MSG_MC_SET_CUR_AIM_FORWARD:
				case MOVT_MSG_MC_SET_CUR_AIM_REVERSE:
				Drv_Movt_SetCurAimAction(MOVT_M_CLOCK,msg.id,msg.cur,msg.aim);
				break;

				case MOVT_MSG_MC_FORWARD_FINISH:
				Drv_Movt_MClockForwardFinish();
				break;

				case MOVT_MSG_MC_REVERSE_FINISH:
				Drv_Movt_MClockReverseFinish();
				break;

				case MOVT_MSG_MC_STOP:
				Drv_Movt_SetStopAction(MOVT_M_CLOCK);
				break;
                case MOVT_MSG_MC_RECOVER:
                case MOVT_MSG_MC_RECOVER_FORWARD:
				case MOVT_MSG_MC_RECOVER_REVERSE:
				Drv_Movt_SetRecoverAction(MOVT_M_CLOCK,msg.id);
                break;

//clock 3
				case MOVT_MSG_3C_FORWARD:
				Drv_Movt_Forward(MOVT_3_CLOCK);
				break;
				
				case MOVT_MSG_3C_REVERSE:
				Drv_Movt_Reverse(MOVT_3_CLOCK);
				break;

				case MOVT_MSG_3C_FORWARDING:
				Drv_Movt_Forwarding(MOVT_3_CLOCK);
				break;

				case MOVT_MSG_3C_REVERSEING:
				
				Drv_Movt_Reversing(MOVT_3_CLOCK);
				break;

				case MOVT_MSG_3C_SET_CUR:
				case MOVT_MSG_3C_SET_CUR_FORWARD:
				case MOVT_MSG_3C_SET_CUR_REVERSE:
				Drv_Movt_SetCurAction(MOVT_3_CLOCK,msg.id,msg.cur);
				break;

				case MOVT_MSG_3C_SET_AIM:
				case MOVT_MSG_3C_SET_AIM_FORWARD:
				case MOVT_MSG_3C_SET_AIM_REVERSE:
				Drv_Movt_SetAimAction(MOVT_3_CLOCK,msg.id,msg.aim);
				break;

				case MOVT_MSG_3C_SET_CUR_AIM:
				case MOVT_MSG_3C_SET_CUR_AIM_FORWARD:
				case MOVT_MSG_3C_SET_CUR_AIM_REVERSE:
				Drv_Movt_SetCurAimAction(MOVT_3_CLOCK,msg.id,msg.cur,msg.aim);
				break;

				case MOVT_MSG_3C_FORWARD_FINISH:
				Drv_Movt_3ClockForwardFinish();
				break;

				case MOVT_MSG_3C_REVERSE_FINISH:
				Drv_Movt_3ClockReverseFinish();
				break;

				case MOVT_MSG_3C_STOP:
				Drv_Movt_SetStopAction(MOVT_3_CLOCK);
				break;
                case MOVT_MSG_3C_RECOVER:
                case MOVT_MSG_3C_RECOVER_FORWARD:
				case MOVT_MSG_3C_RECOVER_REVERSE:
				Drv_Movt_SetRecoverAction(MOVT_3_CLOCK,msg.id);
                break;

//clock 6
				case MOVT_MSG_6C_FORWARD:
				Drv_Movt_Forward(MOVT_6_CLOCK);
				break;
				
				case MOVT_MSG_6C_REVERSE:
				Drv_Movt_Reverse(MOVT_6_CLOCK);
				break;

				case MOVT_MSG_6C_FORWARDING:
				Drv_Movt_Forwarding(MOVT_6_CLOCK);
				break;

				case MOVT_MSG_6C_REVERSEING:
				Drv_Movt_Reversing(MOVT_6_CLOCK);
				break;

				case MOVT_MSG_6C_SET_CUR:
				case MOVT_MSG_6C_SET_CUR_FORWARD:
				case MOVT_MSG_6C_SET_CUR_REVERSE:
				Drv_Movt_SetCurAction(MOVT_6_CLOCK,msg.id,msg.cur);
				break;

				case MOVT_MSG_6C_SET_AIM:
				case MOVT_MSG_6C_SET_AIM_FORWARD:
				case MOVT_MSG_6C_SET_AIM_REVERSE:
				Drv_Movt_SetAimAction(MOVT_6_CLOCK,msg.id,msg.aim);
				break;

				case MOVT_MSG_6C_SET_CUR_AIM:
				case MOVT_MSG_6C_SET_CUR_AIM_FORWARD:
				case MOVT_MSG_6C_SET_CUR_AIM_REVERSE:
				Drv_Movt_SetCurAimAction(MOVT_6_CLOCK,msg.id,msg.cur,msg.aim);
				break;

				case MOVT_MSG_6C_FORWARD_FINISH:
				Drv_Movt_6ClockForwardFinish();
				break;

				case MOVT_MSG_6C_REVERSE_FINISH:
				Drv_Movt_6ClockReverseFinish();
				break;

				case MOVT_MSG_6C_STOP:
				
				Drv_Movt_SetStopAction(MOVT_6_CLOCK);
				break;

                case MOVT_MSG_6C_RECOVER:
                case MOVT_MSG_6C_RECOVER_FORWARD:
				case MOVT_MSG_6C_RECOVER_REVERSE:
				Drv_Movt_SetRecoverAction(MOVT_6_CLOCK,msg.id);
                break;

//clock 9
				case MOVT_MSG_9C_FORWARD:
				Drv_Movt_Forward(MOVT_9_CLOCK);
				break;
				
				case MOVT_MSG_9C_REVERSE:
				Drv_Movt_Reverse(MOVT_9_CLOCK);
				break;

				case MOVT_MSG_9C_FORWARDING:
				Drv_Movt_Forwarding(MOVT_9_CLOCK);
				break;

				case MOVT_MSG_9C_REVERSEING:
				Drv_Movt_Reversing(MOVT_9_CLOCK);
				break;

				case MOVT_MSG_9C_SET_CUR:
				case MOVT_MSG_9C_SET_CUR_FORWARD:
				case MOVT_MSG_9C_SET_CUR_REVERSE:
				Drv_Movt_SetCurAction(MOVT_9_CLOCK,msg.id,msg.cur);
				break;

				case MOVT_MSG_9C_SET_AIM:
				case MOVT_MSG_9C_SET_AIM_FORWARD:
				case MOVT_MSG_9C_SET_AIM_REVERSE:
				Drv_Movt_SetAimAction(MOVT_9_CLOCK,msg.id,msg.aim);
				break;

				case MOVT_MSG_9C_SET_CUR_AIM:
				case MOVT_MSG_9C_SET_CUR_AIM_FORWARD:
				case MOVT_MSG_9C_SET_CUR_AIM_REVERSE:
				Drv_Movt_SetCurAimAction(MOVT_9_CLOCK,msg.id,msg.cur,msg.aim);
				break;

				case MOVT_MSG_9C_FORWARD_FINISH:
				Drv_Movt_9ClockForwardFinish();
				break;

				case MOVT_MSG_9C_REVERSE_FINISH:
				Drv_Movt_9ClockReverseFinish();
				break;

				case MOVT_MSG_9C_STOP:
				Drv_Movt_SetStopAction(MOVT_9_CLOCK);
				break;
                case MOVT_MSG_9C_RECOVER:
                case MOVT_MSG_9C_RECOVER_FORWARD:
				case MOVT_MSG_9C_RECOVER_REVERSE:
				Drv_Movt_SetRecoverAction(MOVT_9_CLOCK,msg.id);
                break;

                // case MOVT_TASK_WATCHDOG://任务看门狗计时
                // TaskWatchdogSet(MOVT_TASK_ID);
                // break;

				default:
				break;
			}
		}
	}
}


uint16 Mid_Movt_SelfTest(void)
{
	return Drv_Movt_SelfTest();
}

// 函数功能:	设置齿轮箱极性
// 输入参数：	无
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16 Mid_Movt_PolarSet(movt_num_t  clock, uint8 polar)
{
	return Drv_Movt_PolarSet(clock, polar);
}


// 函数功能:	读取齿轮箱极性
// 输入参数：	clock：指定指针
// 				polar：极性返回指针
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16 Mid_Movt_PolarRead(movt_num_t clock, uint8 *polar)
{
	return Drv_Movt_PolarRead(clock, polar);
}

// 函数功能:	设置正转波形宽度，中间表盘为每1个计数30.05uS，其他每个计数为61uS
// 输入参数：	clock：指定指针
// 				polar：波形数据指针，长度为24个字节
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16 Mid_Movt_ForwardWaveformSet(movt_num_t  clock, uint8 *waveform)
{
	return Drv_Movt_ForwardWaveformSet(clock, waveform);
}

// 函数功能:	设置反转波形宽度，中间表盘为每1个计数30.05uS，其他每个计数为61uS
// 输入参数：	clock：指定指针
// 				polar：波形数据指针，长度为24个字节
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16 Mid_Movt_ReverseWaveformSet(movt_num_t  clock, uint8 *waveform)
{
	return Drv_Movt_ReverseWaveformSet(clock, waveform);
}


static uint8 movtRecoverDirection[MOVT_MAX_NUM];

// 函数功能:	设置因单口驱动引起的停针，回复走针后已哪种方式恢复走针
// 输入参数：	clock：指定指针
// 				directionSet：	MOVT_DIRECTION_AUTO:	自动判断方向
// 								MOVT_DIRECTION_FORWARD：只向前追时
// 								MOVT_DIRECTION_RESERVE：只向后追时
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16 Mid_Movt_RecoverDirectionSet(movt_num_t  clock, uint8 directionSet)
{
	if(clock >= MOVT_MAX_NUM)
		return 0xff;
	movtRecoverDirection[clock]	= directionSet;
	return 0x00;
}

// 函数功能:	读取因单口驱动导致停针后针的恢复方向；
// 输入参数：	clock：指定指针
// 				directionSet：	MOVT_DIRECTION_AUTO:	自动判断方向
// 								MOVT_DIRECTION_FORWARD：只向前追时
// 								MOVT_DIRECTION_RESERVE：只向后追时
// 返回参数：	0x00:操作成功
// 				0xff:操作失败
uint16 Mid_Movt_RecoverDirectionRead(movt_num_t  clock, uint8 *directionSet)
{
	if(clock >= MOVT_MAX_NUM)
		return 0xff;
	*directionSet	= movtRecoverDirection[clock];
	return 0x00;
}


// 函数功能:	按照设置的方向进行指针恢复
// 输入参数：	none
// 返回参数：	none

void Mid_Movt_AllRecover(void)
{
    movt_task_msg_t msg;
    uint8 direction;

	Mid_Movt_RecoverDirectionRead(MOVT_3_CLOCK, &direction);
	switch(direction)
	{
		case MOVT_DIRECTION_AUTO:
		msg.id			= MOVT_MSG_3C_RECOVER;
		break;

		case MOVT_DIRECTION_FORWARD:
		msg.id			= MOVT_MSG_3C_RECOVER_FORWARD;
		break;

		case MOVT_DIRECTION_RESERVE:
		msg.id			= MOVT_MSG_3C_RECOVER_REVERSE;
		break;
	}
	MovtTask_EventSet(msg);

	Mid_Movt_RecoverDirectionRead(MOVT_6_CLOCK, &direction);
	switch(direction)
	{
		case MOVT_DIRECTION_AUTO:
		msg.id			= MOVT_MSG_6C_RECOVER;
		break;

		case MOVT_DIRECTION_FORWARD:
		msg.id			= MOVT_MSG_6C_RECOVER_FORWARD;
		break;

		case MOVT_DIRECTION_RESERVE:
		msg.id			= MOVT_MSG_6C_RECOVER_REVERSE;
		break;
	}
	MovtTask_EventSet(msg);

	Mid_Movt_RecoverDirectionRead(MOVT_9_CLOCK, &direction);
	switch(direction)
	{
		case MOVT_DIRECTION_AUTO:
		msg.id			= MOVT_MSG_9C_RECOVER;
		break;

		case MOVT_DIRECTION_FORWARD:
		msg.id			= MOVT_MSG_9C_RECOVER_FORWARD;
		break;

		case MOVT_DIRECTION_RESERVE:
		msg.id			= MOVT_MSG_9C_RECOVER_REVERSE;
		break;
	}
	MovtTask_EventSet(msg);

	Mid_Movt_RecoverDirectionRead(MOVT_M_CLOCK, &direction);
	switch(direction)
	{
		case MOVT_DIRECTION_AUTO:
		msg.id			= MOVT_MSG_MC_RECOVER;
		break;

		case MOVT_DIRECTION_FORWARD:
		msg.id			= MOVT_MSG_MC_RECOVER_FORWARD;
		break;

		case MOVT_DIRECTION_RESERVE:
		msg.id			= MOVT_MSG_MC_RECOVER_REVERSE;
		break;
	}
	MovtTask_EventSet(msg);
}

void MovtTask_CreateTask(void)
{
	xTaskCreate(MovtTask, "MovtTask", 128, NULL, TASK_PRIORITY_MIDDLE, NULL);
}
#endif

