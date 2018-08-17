
#include "mod_system.h"

void Mod_Sys_Init(void)
{
	// 开机后默认进入仓储模式
	App_Window_Init(eStoreWinHandle);
	
	// 在这里开启计步场景时为了能使用Apollo3的RTT打印，原因未知
	Mid_SportScene_Start();	
}


void Mod_Sys_PwrOn(void)
{
	Mid_Rtc_Start();
	
//	Mid_SportScene_Start();	
	Mid_SleepScene_Start();		
}



