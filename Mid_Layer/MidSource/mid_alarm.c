#include "platform_common.h"
#include "mid_rtc.h"
#include "mid_alarm.h"


static Mid_Alarm_Param_t	Mid_Alarm;


//**********************************************************************
// ��������:	����̰˯��顣ÿ��10��������һ�Σ���������Ρ�
// ���������	
//      ��
// ���ز�����
//	    0xff �������壻0x00 ���Ӳ�����
// ���ã�Mid_AlarmClock_Check
uint8 Mid_AlarmClock_DelayRing(void)
{
	if(Mid_Alarm.RingSwitch)
	{
		Mid_Alarm.accTimer++;	
		if(Mid_Alarm.accTimer == 10)
		{
			Mid_Alarm.RingCnt++;
			if(Mid_Alarm.RingCnt > 4)//���������Ч�����ӵ�һ����Ӧû�м���
			{
				Mid_Alarm.RingSwitch = 0;
			}
			else
			{
				Mid_Alarm.accTimer = 0;
				return 0xff;
			}
		}
	}
	return 0x00;
}

//**********************************************************************
// ��������:	����̰˯���ܴ�
// ���������	
//      ��
// ���ز�����
//	    ��
// ����:Mid_AlarmClock_Check
 void Mid_AlarmClock_DelayRing_Open(void)
{
	Mid_Alarm.RingSwitch 	        = 1;
	Mid_Alarm.accTimer			= 0;
	Mid_Alarm.RingCnt			= 0;

}

//**********************************************************************
// ��������:	����̰˯���ܹر�
// ���������	
//      ��
// ���ز�����
//	    ��
 void Mid_AlarmClock_DelayRing_Close(void)
{
	Mid_Alarm.RingSwitch = 0;
}

//**********************************************************************
// ��������:�������״̬��RTCÿ���ӵ���һ��
// ���������	
//      curtime����ǰ��RTC time
// ���ز����� 0xff �������壻0x00 ���Ӳ�����
// ����:�ⲿ���ã�ÿ���ӵ���һ�Σ�����Ƿ������Ӳ���
uint8 Mid_AlarmClock_Check(rtc_time_s *curtime)
{
	uint8 i;
	uint8 ret = 0;
	
	for(i = 0; i < eMidAlarmGroupNum; i++)
	{
		// �жϵ�ǰ�����Ƿ���Ч
		if(Mid_Alarm.Clock[i].alarmswitch)			//alarm is open
		{
			// �жϵ�ǰ�������ظ����ӻ��ǵ�������
			if (Mid_Alarm.Clock[i].reptswitch & 0x80)	//repeat
			{
				// ��Ϊ�ظ����ӣ����ݵ�ǰ���ڼ��ж���������Ӧ
				if((Mid_Alarm.Clock[i].reptswitch & (0x01 << curtime->week)) && Mid_Alarm.Clock[i].hour == curtime->hour
					&& Mid_Alarm.Clock[i].min == curtime->min) 
				{	
					Mid_AlarmClock_DelayRing_Open();	
                    CurRingAlarmIdSet(i);					
					return 0xff;	
				}
			}
			else//single
			{
				// ��Ϊ�������ӣ��������ڼ����졣
                if(Mid_Alarm.Clock[i].hour == curtime->hour && Mid_Alarm.Clock[i].min == curtime->min)				
			    {
					Mid_AlarmClock_DelayRing_Open();
					CurRingAlarmIdSet(i);
					Mid_Alarm.Clock[i].alarmswitch = 0;	// ֻ��һ�κ�ر�����
                    return 0xff;					
				}						
			}
		}
	}
	
	// �жϵ�ǰ�Ƿ���̰˯״̬����̰˯ʱ�䵽
	ret |= Mid_AlarmClock_DelayRing();
	
	return ret;
}

// ���õ�ǰ����ID
// ���� Mid_AlarmClock_Check
void CurRingAlarmIdSet(eMidAlarmGroup alarmid)
{
    Mid_Alarm.curRingAlarmId = alarmid;
}

// ��ȡ��ǰ����ID
// ���ã��ⲿ
eMidAlarmGroup CurRingAlarmIdGet(void)
{
    return Mid_Alarm.curRingAlarmId;
}

//in case phone setting
//**********************************************************************
// ��������:ĳ������������Ϣд��
// ���������	
//      configinfo�����ӵ�������Ϣ
//      group ������ID��0-4��
// ���ز�������
//	  ���ã��ⲿ
 void Mid_AlarmClock_Write(alarm_clock_t *configinfo, eMidAlarmGroup group)
{
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}

	Mid_Alarm.Clock[group].reptswitch	= configinfo->reptswitch;
	Mid_Alarm.Clock[group].hour			= configinfo->hour;
	Mid_Alarm.Clock[group].min			= configinfo->min;
	Mid_Alarm.Clock[group].delayswitch	= configinfo->delayswitch;
	Mid_Alarm.Clock[group].alarmswitch	= configinfo->alarmswitch;

	if ((Mid_Alarm.curRingAlarmId == group) && (Mid_Alarm.Clock[group].alarmswitch == 0))
	{
		Mid_AlarmClock_DelayRing_Close();//���õ�ǰ��������ӹر�ʱ��̰˯��ر�
	}	
}

//in case phone reading
//**********************************************************************
// ��������:ĳ������������Ϣ��ȡ
// ���������	
//      configinfo�����ӵ�������Ϣ�����������������Ϣ��
//      group ������ID��0-4��
// ���ز�������
//	   ���ã��ⲿ
 void Mid_AlarmClock_Read(alarm_clock_t *configinfo, eMidAlarmGroup group)
{
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}

	configinfo->reptswitch		= Mid_Alarm.Clock[group].reptswitch;
	configinfo->hour            = Mid_Alarm.Clock[group].hour;
	configinfo->min				= Mid_Alarm.Clock[group].min;
	configinfo->delayswitch		= Mid_Alarm.Clock[group].delayswitch;
	configinfo->alarmswitch		= Mid_Alarm.Clock[group].alarmswitch;
}

//set a alarm groupn status,state =0, the alarm group function is disable, no alarm ring
//**********************************************************************
// ��������:����һ�����ӵĿ���״̬
// ���������	
//      group ������ID��0-4��
//      state ������״̬��0�رգ�1�򿪣�
// ���ز�������
//	  ���ã�  ��
 void Mid_AlarmClock_Switch_Set(uint8 group, uint8 state)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}
	Mid_Alarm.Clock[group].alarmswitch = state;
	#endif
}

//read a alarm groupn status,state =0, the alarm group function is disable, no alarm ring
//**********************************************************************
// ��������:��һ�����ӵĿ���״̬
// ���������
//      group ������ID��0-4��
// ���ز�����
//     ����״̬��0�رգ�1�򿪣�
// ���� ����
 uint8 Mid_AlarmClock_Switch_Read(uint8 group)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		return 0x00;
	}
	return Mid_Alarm.Clock[group].alarmswitch;
	#endif
}


//read a alarm group delay ring function
//**********************************************************************
// ��������:��ĳ�����ӵ�̰˯״̬
// ���������	
//      group ������ID��0-4��
// ���ز�����
//     ����̰˯״̬�� 1��̰˯����  0��̰˯�ر�  ��
// ���ã���
 uint8 Mid_AlarmClock_DelayRing_Read(uint8 group)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		return 0x00;
	}
	return Mid_Alarm.Clock[group].delayswitch;
	#endif
}

//set a alarm group delay ring function
//**********************************************************************
// ��������:����ĳ�����ӵ�̰˯��״̬
// ���������	
//      group ������ID��0-4��
//      state �����õ�״̬
// ���ز�������
//   ���ã�   ��

 void Mid_AlarmClock_DelayRing_Set(uint8 group, uint8 state)
{
	#if 0
	if(group >= eMidAlarmGroupNum)
	{
		group = 0;
	}
	Mid_Alarm.Clock[group].delayswitch = state;
	#endif
}



//for watch device(key) setting
//**********************************************************************
// ��������:ĳ�����ӵ�Hourʮλ��һ		
// ���������	
//      group ������ID��0-4��
// ���ز����� ��
// ���ã���
 void Mid_AlarmClock_HighHour_Add(uint8 group)
{
	#if 0
	Mid_Alarm.Clock[group].hour += 10;
	if(Mid_Alarm.Clock[group].hour >= 30)
	{
		Mid_Alarm.Clock[group].hour %= 10;
 	}
	#endif
}

//for watch device(key) setting
//**********************************************************************
// ��������:ĳ�����ӵ�Hourʮλ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//  ���ã�   ��
 void Mid_AlarmClock_HighHour_Less(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour < 10)
	{
		Mid_Alarm.Clock[group].hour += 20;
 	}
	else
	{
		Mid_Alarm.Clock[group].hour -= 10;
	}
	#endif
}

//**********************************************************************
// ��������:���ӵ�Hourʮλ��Ч��Χ��Ⲣ����������23�Զ���Ϊ20
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_HighHour_Check(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour > 23)
	{
		Mid_Alarm.Clock[group].hour = 20;
 	}
	#endif
}

//**********************************************************************
// ��������:ĳ�����ӵ�Hour��λ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_LowHour_Add(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour == 23)
	{
		Mid_Alarm.Clock[group].hour = 20;
 	}
	else if(Mid_Alarm.Clock[group].hour == 19)
	{
		Mid_Alarm.Clock[group].hour = 10;
	}
	else if(Mid_Alarm.Clock[group].hour == 9)
	{
		Mid_Alarm.Clock[group].hour = 0;
	}
	else 
	{
		Mid_Alarm.Clock[group].hour++;
	}
	#endif
}

//**********************************************************************
// ��������:ĳ�����ӵ�Hour��λ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_LowHour_Less(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].hour == 0)
	{
		Mid_Alarm.Clock[group].hour = 9;
	}
	else if(Mid_Alarm.Clock[group].hour == 10)
	{
		Mid_Alarm.Clock[group].hour = 19;
	}
	else if(Mid_Alarm.Clock[group].hour == 20)
	{
		Mid_Alarm.Clock[group].hour = 23;
	}
	else
	{
		Mid_Alarm.Clock[group].hour--;
	}
	#endif
}

//**********************************************************************
// ��������:ĳ�����ӵ�Minʮλ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_HighMin_Add(uint8 group)
{
	#if 0
	Mid_Alarm.Clock[group].min += 10;
	if(Mid_Alarm.Clock[group].min >= 60)
	{
		Mid_Alarm.Clock[group].min %= 10;
 	}
	#endif
}

//**********************************************************************
// ��������:ĳ�����ӵ�Minʮλ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_HighMin_Less(uint8 group)
{
	#if 0
	if(Mid_Alarm.Clock[group].min < 10)
	{
		Mid_Alarm.Clock[group].min += 50;
 	}
	else
	{
		Mid_Alarm.Clock[group].min -= 10;
	}
	#endif
}

//**********************************************************************
// ��������:ĳ�����ӵ�Min��λ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_LowMin_Add(uint8 group)
{
	#if 0
	uint8 temp;

	temp = Mid_Alarm.Clock[group].min / 10;
	Mid_Alarm.Clock[group].min = Mid_Alarm.Clock[group].min % 10 + 1;
	if(Mid_Alarm.Clock[group].min > 9)
	{
		Mid_Alarm.Clock[group].min = 0;
	}
	Mid_Alarm.Clock[group].min = temp * 10 + Mid_Alarm.Clock[group].min;
	#endif
}

//**********************************************************************
// ��������:ĳ�����ӵ�Min��λ��һ
// ���������	
//      group ������ID��0-4��
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_LowMin_Less(uint8 group)
{
	#if 0
	uint8 temp;

	temp = Mid_Alarm.Clock[group].min / 10;
	Mid_Alarm.Clock[group].min = Mid_Alarm.Clock[group].min % 10;
	if(Mid_Alarm.Clock[group].min == 0)
	{
		Mid_Alarm.Clock[group].min = 9;
	}
	else 
	{
		Mid_Alarm.Clock[group].min--;
	}
	Mid_Alarm.Clock[group].min = temp * 10 + Mid_Alarm.Clock[group].min;
	#endif
}


//**********************************************************************
// ��������:����ĳ�����ӵ��ظ�����
// ���������	
//      group ������ID��0-4��
//      reptMainSwitch ������ģʽ��0�����Σ� 1���ظ���
// ���ز�������
//     ���ã� ��
 void Mid_AlarmClock_ReptMainSwtich_Set(uint8 group, uint8 reptMainSwitch)
{
	#if 0
	if(reptMainSwitch)
	{
		Mid_Alarm.Clock[group].reptswitch |= 0x80;
	}
	else
	{
		Mid_Alarm.Clock[group].reptswitch &= 0x7F;
	}
	#endif
}

//**********************************************************************
// ��������:��ĳ�����ӵ��ظ�����״̬
// ���������	
//      group ������ID��0-4��
// ���ز�����
//     �����ظ�����״̬�� 0�����Σ� 1���ظ���
// ���ã���
 uint8 Mid_AlarmClock_ReptMainSwtich_Read(uint8 group)
{
	#if 0
	return Mid_Alarm.Clock[group].reptswitch & 0x80;
	#endif
	return 0;
}

//retDay: sun~sat
//**********************************************************************
// ��������:�����ظ�����ĳ��Ŀ���״̬
// ���������	
//      group ������ID��0-4��
//      reptDay ������
//      reptDaySwitch �������ظ�����״̬�� 0�����Σ� 1���ظ���
// ���ز�������
//    ���ã� ��
 void Mid_AlarmClock_ReptDaySwtich_Set(uint8 group, uint8 reptDay, uint8 reptDaySwitch)
{
	#if 0
	if (reptDay > 6)
	{
		reptDay = 0;
	}

	if (reptDaySwitch)//open (day)repeat alarm function
	{
		Mid_Alarm.Clock[group].reptswitch |= (0x01 << reptDay);
	}
	else
	{
		Mid_Alarm.Clock[group].reptswitch &= ~(0x01 << reptDay);
	}
	#endif
}

//**********************************************************************
// ��������:���ظ�����һ��ĳ��Ŀ���״̬
// ���������	
//      group ������ID��0-4��
//      reptDay ������      
// ���ز�����
//     reptDaySwitch �������ظ�����״̬�� 0�����Σ� 1���ظ���
// ���ã���
 uint8 Mid_AlarmClock_ReptDaySwtich_Read(uint8 group, uint8 reptDay)
{
	#if 0
	uint8 switchtemp = 0;

	if (reptDay > 6)
	{
		return 0x00;
	}
	
	switchtemp	= Mid_Alarm.Clock[group].reptswitch & (0x01 << reptDay);

	return switchtemp;
	#endif
	
	return 0;
}

//**********************************************************************
// ��������:��ȡ���ӵ���Ч����
// �����������    
// ���ز��������ӵ���Ч����
// ���ã��ⲿ
uint8 Mid_AlarmClock_ValidNunRead(void)
{
	uint8 numCnt;
	uint8 validAlarmNun = 0;

	for (numCnt = 0; numCnt < eMidAlarmGroupNum; numCnt ++)
	{
		//ȫ��Ϊ0�����ӱ�����Ϊ��Ч
		if (!(Mid_Alarm.Clock[numCnt].reptswitch == 0 && Mid_Alarm.Clock[numCnt].alarmswitch == 0  && 
			 Mid_Alarm.Clock[numCnt].hour == 0 && Mid_Alarm.Clock[numCnt].min == 0))
		{
				validAlarmNun ++;
		}
	}	
	return validAlarmNun;
}
