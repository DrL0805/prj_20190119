
#include "mod_system.h"

void Mod_Sys_Init(void)
{
	// ����״̬
	resetStatus = 1;
	
	// ������Ĭ�Ͻ���ִ�ģʽ
	App_Window_Init(eStoreWinHandle);
	
	// �����￪���Ʋ�����ʱΪ����ʹ��Apollo3��RTT��ӡ��ԭ��δ֪
	Mid_SportScene_Start();	
}

// ���������ϵ�
void Mod_Sys_PwrOn(void)
{
	App_Protocal_BleStateSet(BLE_POWERON);
	
	Mid_Rtc_Start();
	
//	Mid_SportScene_Start();	
	Mid_SleepScene_Start();	

	// �������ʾ����
	Mid_Motor_ParamSet(eMidMotorShake4Hz, 2);
	Mid_Motor_ShakeStart();		
}

// ��������ִ�
void Mod_Sys_PwrOff(void)
{
	
}




