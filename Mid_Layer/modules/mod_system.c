
#include "mod_system.h"

void Mod_Sys_Init(void)
{
	// ������Ĭ�Ͻ���ִ�ģʽ
	App_Window_Init(eStoreWinHandle);
	
	// �����￪���Ʋ�����ʱΪ����ʹ��Apollo3��RTT��ӡ��ԭ��δ֪
	Mid_SportScene_Start();	
}


void Mod_Sys_PwrOn(void)
{
	Mid_Rtc_Start();
	
//	Mid_SportScene_Start();	
	Mid_SleepScene_Start();		
}


