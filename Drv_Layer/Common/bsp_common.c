#include "platform_common.h"
#include "bsp_common.h"




//多传感器芯片管理
MultiSensor_s multisensor =
{
	.accelstate = uinit,
	.gyrostate 	= uinit,
	.magstate 	= uinit,
};

//共享总线管理
ShareBusSpi_s sharebusspi =
{
	.ExtFlash 	= BusUninit,
	.NandFlash 	= BusUninit,
	.WordStack 	= BusUninit,
};    

ShareBusI2c_s sharebusi2c =
{
	.Accels 	= BusUninit,
	.Gyros 		= BusUninit,
	.Mags 		= BusUninit,
	.Pressure	= BusUninit,
	.Hrm 		= BusUninit,
};

