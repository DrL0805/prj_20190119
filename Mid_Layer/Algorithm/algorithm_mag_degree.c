#include "platform_common.h"
#include "algorithm_mag_degree.h"




const uint16 ArcTanTap[]={0,52,105,158,213,268,325,384,445,510,577,649,727,810,900,1000,
				   1111,1235,1376,1540,1732,1963,2246,2605,3078,3732,4705,6314,
				   9514,19081};

uint8 UserCrctan(int16 data)
{
	uint8 i = 0;
	if(data > 19081)
		return 90;
	for(i=0;i<30;i++)
	{
		if((data>ArcTanTap[i])&&(data<ArcTanTap[i+1]))
			break;
	}
	return i*3;
}

int16 MagmaxXX = -1000;
int16 MagmaxYY = -1000;
int16 MagminXX = 1000;
int16 MagminYY = 1000;

void flat_algorithm(int16 *mag, double *offset, int16 *flag)
{
	static uint8 Tpy = 0;
	if((flag == 0)&&(Tpy == 0)) return ;
	if((flag[0] == 1)&&(Tpy == 0)) //刚进来
	{
		Tpy =1;
		MagmaxXX = -1000;
		MagmaxYY = -1000;
		MagminXX = 1000;
		MagminYY = 1000;

		//setRapidTaskMode(&sLed_M2_APP,&rapidMode20);
		//startRapidTask(&sLed_M2_APP,2);
	}
	else if((flag[0] == 0)&&(Tpy == 1))//要出去了
	{
		Tpy = 0;
	}

	if((flag[0] == 1)&&(Tpy == 1))//校准中
	{
		if(mag[0]>MagmaxXX)
			MagmaxXX = mag[0];
		if(mag[1]>MagmaxYY)
			MagmaxYY = mag[1];
		if(mag[0]< MagminXX)
			MagminXX = mag[0];
		if(mag[1]< MagminYY)
			MagminYY = mag[1];
		float Vx0ff = (MagmaxXX-MagminXX)/2+MagminXX;
		float Vy0ff = (MagmaxYY-MagminYY)/2+MagminYY;

		if((MagmaxXX-MagminXX)>(MagmaxYY-MagminYY))
		{
			offset[2] = 1;
			offset[3] =  (float)(MagmaxXX-MagminXX)/(MagmaxYY-MagminYY);
		}
		else
		{
			offset[3] = 1;
			offset[2] = (float)(MagmaxXX-MagminXX)/(MagmaxYY-MagminYY);
		}
		offset[0] = Vx0ff;
		offset[1] = Vy0ff;
	}
}

uint16 Algorithm_Calculate_Mag_Degree(int16 *uv_uart_buffer,uint16 GfTestModeSwitch)
{
		uint16 Angle = 0;
		int16 Vx = 0,Vy = 0;// Vz = 0;
		static int16 mag[2],flag[1];
		static double offset[4]={0,0,0,0};


		Vx = -1 * uv_uart_buffer[0];
		Vy = uv_uart_buffer[1];


		mag[0]=uv_uart_buffer[0];
		mag[1]=uv_uart_buffer[1];
		flag[0] = GfTestModeSwitch;//GfTestModeSwitch set 1 enter adjust
		flat_algorithm(mag,offset,flag);

		if (GfTestModeSwitch == 1)
		{
			return 0xffff;
		}

		if(GfTestModeSwitch == 0)
		{
			double Hx,Hy;

			Hx = (Vx - offset[0])*offset[2];    //mag[0]是X轴原始数据，offset是校准参数
			Hy = (Vy - offset[1])*offset[3];

			long double tempd = ((Hy*1000)/Hx);

			if ((Hx == 0) && (Hy < 0))
				Angle = 90.0f;
			else if ((Hx == 0) && (Hy > 0))
				Angle = 270.0f;
			else if ((Hx < 0) && (Hy <= 0))
				Angle = 180.0f - (UserCrctan(tempd)); //180 - 270
			else if ((Hx < 0) && (Hy > 0))
				Angle = 180.0f + (UserCrctan(tempd*(-1)));  //90 -  180
			else if ((Hx > 0) && (Hy < 0))
				Angle = (UserCrctan(tempd*(-1)));          //0  -  90
			else if ((Hx > 0) && (Hy > 0))
				Angle = 360.0f - (UserCrctan(tempd));    //270 - 360
			else if ((Hx > 0) && (Hy == 0))
				Angle = 0;
			if(Angle > 360) Angle = Angle -360;

			// return (60 - (Angle / 6));
			return Angle;

		}
		return 0xffff;
}
