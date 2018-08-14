/**********************************************************************
**
**模块说明: 模块测试单元
**软件版本，修改日志(时间，内容):
**   V1.0   2018.6.22  初版  
**
**********************************************************************/
#include "platform_common.h"
#include "module_test.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "sm_sys.h"

#include "drv_key.h"
#include "drv_common.h"

#include "BLE_Stack.h"  //BLE stack

#include "rtos.h"

#if(MODULE_TEST_ENABLE == 1)


//**********************************************************************
// 函数功能:  测试项2: 蓝牙
// 输入参数：
// 返回参数： 
//**********************************************************************
static void Ble_CB(const uint8 *buf,uint16 len)
{
    uint16  i;

    SEGGER_RTT_printf(0,"BLE Recv:\n");
    for(i = 0; i < len; i++)
    {
        SEGGER_RTT_printf(0,"0x%x,",buf[i]);
    }
    SEGGER_RTT_printf(0,"\n");
    BLE_Stack_Send((uint8 *)buf,len);
}

static void BLE_Test(void)
{
    uint8 newblestate;

    while(1)
    {
        SEGGER_RTT_printf(0,"\n1: Transmiter,with BLE tool...\n");
        SEGGER_RTT_printf(0,"2: ADV On...\n");
        SEGGER_RTT_printf(0,"3: ADV Off...\n");
        SEGGER_RTT_printf(0,"4: Power On...\n");
        SEGGER_RTT_printf(0,"5: Power Off...\n");
        SEGGER_RTT_printf(0,"6: Set ADV Name...\n");
        SEGGER_RTT_printf(0,"0: Exit...\n");
        switch(SEGGER_RTT_WaitKey())
        {
        case '1':
            BLE_Stack_SetRecvCallBack(Ble_CB);
            SMDrv_SYS_DelayMs(50);
            break;
        case '2':
            newblestate = BT_ADV_ON;
            BLE_Stack_Interact(BLE_STATUS_SET,&newblestate,0);
            break;
        case '3':
            newblestate = BT_ADV_OFF;
            BLE_Stack_Interact(BLE_STATUS_SET,&newblestate,0);
            break;
        case '4':
            newblestate = BT_POWERON;
            BLE_Stack_Interact(BLE_POWERONOFF,&newblestate,0);
            break;
        case '5':
            newblestate = BT_POWEROFF;
            BLE_Stack_Interact(BLE_POWERONOFF,&newblestate,0);
            break;
        case '6':
            BLE_Stack_Interact(BLE_AVD_NAME_SET,"ble_test",8);
            break;
        case '0':
            newblestate = BT_ADV_OFF;
            BLE_Stack_Interact(BLE_STATUS_SET,&newblestate,0);
            SMDrv_SYS_DelayMs(50);
            newblestate = BT_POWEROFF;
            BLE_Stack_Interact(BLE_POWERONOFF,&newblestate,0);
            return;
        default:
            break;
        }
    }
}

//**********************************************************************
// 函数功能:  测试项1: 按键和马达
// 输入参数：
// 返回参数： 
//**********************************************************************
static void Key_CB(key_num_t keynum,key_event event_type)
{
    SEGGER_RTT_printf(0,"Key S%d:",keynum);
    if(event_type == KEY_HOLDSHORT_EVENT)
        SEGGER_RTT_printf(0,"short hold\n");
    else if(event_type == KEY_HOLDLONG_EVENT)
        SEGGER_RTT_printf(0,"long hold\n");
    else
        SEGGER_RTT_printf(0,"press...\n");
}

static void Key_Moto_Test(void)
{
    static uint8 u8bInit = FALSE;

    if(u8bInit == FALSE)
    {
        Drv_Key_Init();
        Drv_Common_Init();
        u8bInit = TRUE;
    }
    while(1)
    {
        SEGGER_RTT_printf(0,"1: Key Test.\n");
        SEGGER_RTT_printf(0,"2: Moto Test.\n");
        SEGGER_RTT_printf(0,"0: Exit...\n");
        switch(SEGGER_RTT_WaitKey())
        {
        case '1':
            Drv_Key_SetCallBack(Key_CB);
            break;
        case '2':
            //Drv_Common_MotoOn();
            Drv_Common_GreenLedOn();
            SMDrv_SYS_DelayMs(1500);
            //Drv_Common_MotoOff();
            Drv_Common_GreenLedOff();
            SMDrv_SYS_DelayMs(500);
            break;
        case '0':
            Drv_Key_SetCallBack(NULL);
            Drv_Common_MotoOff();
            return;
        default:
            break;
        }
    }
}

//**********************************************************************
// 函数功能:  打印主测试项
// 输入参数：
// 返回参数： 
//**********************************************************************
static void Module_Print_Case(void)
{
    SEGGER_RTT_printf(0,"Goto H003 Module Test:\n");
    SEGGER_RTT_printf(0,"a: Key,Moto Test...\n");
    SEGGER_RTT_printf(0,"b: BLE Test...\n");
}

//**********************************************************************
// 函数功能:  模块测试总入口
// 输入参数：
// 返回参数： 
//**********************************************************************
void Module_Test_Main(void *pvParameters)
{
//	Drv_Key_Init();
//	Drv_Common_Init();
//	Drv_Key_SetCallBack(Key_CB);
	
//    int32 s32Key;
//    SMDrv_SYS_DelayMs(500);
    while(1)
    {
//        Module_Print_Case();

//        s32Key = SEGGER_RTT_WaitKey();
//        //SEGGER_RTT_printf(0,"s32Key=%c\n",s32Key);
//        switch(s32Key)
//        {
//        case 'a':
//            Key_Moto_Test();
//            break;
//        case 'b':
//            BLE_Test();
//            break;
//        }
//        SMDrv_SYS_DelayMs(50);
//		SEGGER_RTT_printf(0,"b: BLE Test...\n");
		vTaskDelay(100);
    }
}
#endif
