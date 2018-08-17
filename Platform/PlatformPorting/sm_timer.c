/**********************************************************************
**
**ģ��˵��: �Խ�MCU ctimer�����ӿ�
**����汾���޸���־(ʱ�䣬����),�޸���:
**   V1.0   2018.4.18  ����  ZSL  
**
**********************************************************************/
#include "am_mcu_apollo.h"
#include "string.h"
#include "sm_timer.h"

#define CTIMER_MAX_NUM   8

//����ctimer���ò���
typedef struct
{
    uint32 u32tm_id;   //timer number
    uint32 u32tm_seg;  //timer segment
    uint32 u32tm_irq;  //timer�ж�����
    uint32 u32tm_conf; //����Timer����
    uint32 u32tm_delay;//delayֵ
}ctimer_info_s;

static comm_cb *ctimer_irq_cb[CTIMER_MAX_NUM];

//**********************************************************************
// ��������:    ��ʱ���жϷ�����
// ���������    ��
// ���ز�����    ��
//**********************************************************************
void am_ctimer_isr(void)
{
    uint32 timer_int_status;

    timer_int_status = am_hal_ctimer_int_status_get(true);
    am_hal_ctimer_int_clear(timer_int_status);

    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA0)    //��0,A��С����
    {
        if(ctimer_irq_cb[0] != NULL)
        {      
            (ctimer_irq_cb[0])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB0)     //��0,B���๦�ܶ�ʱ��
    {
        if(ctimer_irq_cb[1] != NULL)
        {      
            (ctimer_irq_cb[1])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA1)     //��1,A��ϵͳTICKʱ��
    {
        if(ctimer_irq_cb[2] != NULL)
        {      
            (ctimer_irq_cb[2])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB1)     //��1,B������
    {
        if(ctimer_irq_cb[3] != NULL)
        {      
            (ctimer_irq_cb[3])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA2)     //��2,A��stopwatch
    {
        if(ctimer_irq_cb[4] != NULL)
        {      
            (ctimer_irq_cb[4])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB2)     //��2,B��countdown
    {
        if(ctimer_irq_cb[5] != NULL)
        {      
            (ctimer_irq_cb[5])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERA3)     //��3,A��RTC
    {
        if(ctimer_irq_cb[6] != NULL)
        {      
            (ctimer_irq_cb[6])();
        }
    }
    if(timer_int_status & AM_HAL_CTIMER_INT_TIMERB3)     //��3,B��monitor
    {
        if(ctimer_irq_cb[7] != NULL)
        {
            (ctimer_irq_cb[7])();
        }
    }
}

//**********************************************************************
// ��������: ���ϲ�ctimer moduleת��Ϊ����Timer���ò�����
//           �˽ӿ��еľ������������ݾ�����Ŀ����������ú͵�����
// ���������
//    modul: driver module ID,ֵ�ο�ctimer_module
//    u16freq: timerƵ�ʣ���open Timer��ʱ�����ʹ��ģ�鴫�룬�����ӿڴ���Ϊ0
//    timer_callback:�ϲ�ע����жϻص�����
// ���ز�����
//    ptm_info:ctimer��ʱ�����ò���
//    FALSE:����modul��������ȷ
//    TRUE:����
//**********************************************************************
static uint8 ctimer_modul2time(ctimer_module modul,uint16 u16freq,comm_cb *timer_callback,ctimer_info_s *ptm_info)
{
    uint8 u8cb_id;

    if(ptm_info == NULL)
        return FALSE;
    //��ֹu16freq����0ֵ���ڼ���ʱ����
    if(u16freq == 0)
        u16freq = 1;
    
    if(modul == MOVT_CTIMER_MODULE)         //timer:(0,A)
    {
        ptm_info->u32tm_id=0;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA0;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
        ptm_info->u32tm_delay=1;
        u8cb_id = 0;
    }
    else if(modul == MULTI_CTIMER_MODULE)    //timer:(0,B)
    {
		#if 0	// H001
        ptm_info->u32tm_id=0;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB0;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_16_384KHZ;
        ptm_info->u32tm_delay=(uint32)(16384 / u16freq - 1);
        u8cb_id = 1;
		#endif 
		
		#if 1 // H003
        ptm_info->u32tm_id=0;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB0;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
        ptm_info->u32tm_delay = (uint32)((32768 * u16freq / 1000) -1);	// תΪms 
        u8cb_id = 1;
		#endif
    }
    else if(modul == SYSTICK_CTIMER_MODULE)  //timer:(1,A)
    {
        ptm_info->u32tm_id=1;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA1;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
        ptm_info->u32tm_delay= u16freq;  //1023; //u16freq     100Hz
        u8cb_id = 2;
    }
    else if(modul == KEY_CTIMER_MODULE)      //timer:(1,B)
    {    
        ptm_info->u32tm_id=1;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB1;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_256HZ;
        ptm_info->u32tm_delay= 7;
        u8cb_id = 3;
    }
    else if(modul == STOPWATCH_CTIMER_MODULE) //timer:(2,A)
    {
        ptm_info->u32tm_id=2;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA2;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_32_768KHZ;
//        ptm_info->u32tm_delay= 328;	// �����	
		ptm_info->u32tm_delay = (uint32)((32768 * u16freq / 1000) - 1);	// תΪms 
        u8cb_id = 4;
    }
    else if(modul == COUNTDOWN_CTIMER_MODULE) //timer:(2,B)
    {
        ptm_info->u32tm_id=2;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB2;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_256HZ;
        ptm_info->u32tm_delay= (uint32)((256 * u16freq / 1000) - 1);
        u8cb_id = 5;
    }
    else if(modul == RTC_CTIMER_MODULE)       //timer:(3,A)
    {
        ptm_info->u32tm_id=3;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERA;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERA3;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_256HZ;
        ptm_info->u32tm_delay= 127;
        u8cb_id = 6;
    }
    else if(modul == MONITOR_CTIMER_MODULE)   //timer:(3,B)
    {
        ptm_info->u32tm_id=3;
        ptm_info->u32tm_seg=AM_HAL_CTIMER_TIMERB;
        ptm_info->u32tm_irq=AM_HAL_CTIMER_INT_TIMERB3;
        ptm_info->u32tm_conf=AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_XT_16_384KHZ;
        ptm_info->u32tm_delay=(16384/(u16freq * 2));
        u8cb_id = 7;
    }
    else if(modul == MOVT_CTIMER_MODULE) //С����
    {
        return FALSE;  //��Ŀ����
    }
    else
    {
        return FALSE;
    }

    //set callback
    if(timer_callback != NULL)
    {
        ctimer_irq_cb[u8cb_id] = timer_callback;
    }
    return TRUE;
}

//**********************************************************************
// ��������: ��ʼ��counter/timer
// ���������
// ���ز�����
//**********************************************************************
void SMDrv_CTimer_Init(void)
{
    memset(ctimer_irq_cb,NULL,sizeof(ctimer_irq_cb));
}

//**********************************************************************
// ��������: ����driver module ID��Ӳ����Ӧ��ctimer
// ���������	
//    modul: driver module ID,ֵ�ο�ctimer_module
//    u16freq: timerƵ��
//    timer_callback:�ϲ�ע����жϻص�����
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Open(ctimer_module modul,uint16 u16freq,comm_cb *timer_callback)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,u16freq,timer_callback,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

    //init ctimer
    BspTimerInitCom(stm_info.u32tm_id,stm_info.u32tm_seg,stm_info.u32tm_irq,stm_info.u32tm_conf,stm_info.u32tm_delay);
    return Ret_OK;
}

//**********************************************************************
// ��������:  ����CTimer�ж����ȼ�
// ��������� prio:�ж����ȼ���bit0~2λֵ��Ч
// ���ز����� ��
//**********************************************************************
void SMDrv_CTimer_SetIsrPrio(uint32 prio)
{
#if AM_CMSIS_REGS
    NVIC_SetPriority(CTIMER_IRQn,prio);
#else
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, AM_HAL_INTERRUPT_PRIORITY(prio));
#endif
}

//**********************************************************************
// �������� : ����timer�Ƚ�ֵ
// ������� ��
//    modul : driver module ID,ֵ�ο�ctimer_module
//u32cmp_cnt: �Ƚ�ֵ
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_SetCmpValue(ctimer_module modul,uint32 u32cmp_cnt)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

	am_hal_ctimer_compare_set(stm_info.u32tm_id,stm_info.u32tm_seg,0,u32cmp_cnt);	
    return Ret_OK;
}

//**********************************************************************
// ��������: ����timer
// ���������modul: driver module ID,ֵ�ο�ctimer_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Start(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

	am_hal_ctimer_int_enable(stm_info.u32tm_irq);
	am_hal_ctimer_start(stm_info.u32tm_id, stm_info.u32tm_seg);	
    return Ret_OK;
}

//**********************************************************************
// ��������: ֹͣtimer
// �������: modul: driver module ID,ֵ�ο�ctimer_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Stop(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

	am_hal_ctimer_stop(stm_info.u32tm_id, stm_info.u32tm_seg);
	am_hal_ctimer_int_disable(stm_info.u32tm_irq);
    return Ret_OK;
}

//**********************************************************************
// ��������: ����timer
// ���������modul: driver module ID,ֵ�ο�ctimer_module
// ���ز�����Ret_InvalidParam��Ret_OK
//**********************************************************************
ret_type SMDrv_CTimer_Clear(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

	am_hal_ctimer_clear(stm_info.u32tm_id, stm_info.u32tm_seg);
    return Ret_OK;
}

//**********************************************************************
// ��������: ��ȡtimer����ֵ
// �������: modul: driver module ID,ֵ�ο�ctimer_module
// ���ز���: ����timer����ֵ 
//**********************************************************************
uint16 SMDrv_CTimer_ReadCount(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

	return am_hal_ctimer_read(stm_info.u32tm_id, stm_info.u32tm_seg);
}

//**********************************************************************
// ��������: ��ȡģ��ʹ�õ�ctimer��number
// �������: modul: driver module ID,ֵ�ο�ctimer_module
// ���ز���: ctimer��number
//**********************************************************************
uint32 SMDrv_CTimer_GetTimerNumber(ctimer_module modul)
{
    ctimer_info_s stm_info;

    if(ctimer_modul2time(modul,0,NULL,&stm_info) == FALSE)
    {
        return Ret_InvalidParam ;//��������
    }

	return stm_info.u32tm_id;
}

