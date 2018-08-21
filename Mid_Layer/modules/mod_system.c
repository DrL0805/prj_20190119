
#include "mod_system.h"

void Mod_Sys_Init(void)
{
	// 开机后默认进入仓储模式
	App_Window_Init(eStoreWinHandle);
	
	// 在这里开启计步场景时为了能使用Apollo3的RTT打印，原因未知
	Mid_SportScene_Start();	
}

// 长按开机上电
void Mod_Sys_PwrOn(void)
{
	App_Protocal_BleStateSet(BLE_POWERON);
	
	Mid_Rtc_Start();
	
//	Mid_SportScene_Start();	
	Mid_SleepScene_Start();		
}

// 长按进入仓储
void Mod_Sys_PwrOff(void)
{
	
}




