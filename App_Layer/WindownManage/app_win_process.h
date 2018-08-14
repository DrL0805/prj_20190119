#ifndef APP_WIN_PROCESS_H
#define APP_WIN_PROCESS_H



typedef 	uint16 WinInitFunc(void);
typedef 	uint16 CallBackFunc(uint16 sysHandle,uint16 tagMenu,menuMessage message);

typedef struct
{
	uint16 			HandleIndex;
	WinInitFunc 		*WinInit;
	CallBackFunc 		*CallBack;
}SysWin;


uint16 App_Window_Register(uint16 SysHandleNew,WinInitFunc *InitFunc,CallBackFunc *SysCallBack);
uint16 App_Window_Process(uint16 sysHandle,MenuTag tagMenu,menuMessage message);
uint16 App_Window_Create(SysWinHandle sysHandleNew);

#endif

