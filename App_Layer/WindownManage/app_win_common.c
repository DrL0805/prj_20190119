#include "platform_common.h"
#include "app_win_common.h"



//虚拟窗口无需显示界面，但在该虚拟窗口下，按键、甩手需特别处理
uint8 	virtualHandle 		= Virtual_Handle_None;	

//窗口显示时间
uint32 	winTimeCnt;

//子窗口循环显示标记
uint8 	cycleFlag;

//窗口下存储存储状态标记（需要显示存储状态的窗口中有用）
uint8 	saveState;
