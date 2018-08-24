#include "platform_common.h"
/* FreeRTOS includes */
#include "rtos.h"
#include "mid_common.h"
#include "mid_interface.h"
#include "app_variable.h"
#include "app_protocal.h"
#include "app_remind_manage.h"
#include "app_win_common.h"
#include "app_win_process.h"
#include "app_win_remind.h"
#include "main.h"

/*******************macro define*******************/
#define 	SECT_END 			0x0000 		//详情段结尾判断符

typedef enum 
{
	FLOW_START 			= 0x00,
	FLOW_PROCESS 		= 0x01,
	FLOW_END			= 0xFF,
}flow_state;

typedef enum
{
	PHONE_FAMILIAR	= 0,	//常用联系人
	PHONE_UNFAMILIAR,
}phone_relation_type;

typedef enum
{
	QQ_MESAGE 		= 0,
	WECHAT_MESAGE   = 1,	//微信
	PHONE_CALL_IN 	= 10,	// 来电
	PHONE_MESSAGE   = 11,	// 短信
	MISS_PHONE_CALL = 12,	// 未接来电
	FAMILIAR_PHONE_CALL = 16,	// 联系人来电
    TIM_MESAGE = 17,		// TIM
}detail_remind_type;

app_remind_msg_t appRemindMsg;
uint8_t 	protocalCache[APP_REMIND_MSG_MAX_LENGHT];	// 存储最新utf-8格式的提醒数据
uint8_t 	protocalCacheCnt;	// 提醒数据缓存计数，最大接收 APP_REMIND_MSG_MAX_LENGHT 字节
uint8_t 	flowCnt;			// 提醒数据包流控计数

//**********************************************************************
// 函数功能: 判断消息详情中是否有null:null
// 输入参数：无
// 返回参数：true:消息为null
//**********************************************************************
static uint8 App_RemindManag_IsNULL(void)
{
    //0x6E,0x75,0x6C,0x6C,0x3A,0x6E,0x75,0x6C,0x6C
    uint8 u8null[10] = "null:null";
    uint8 u8i,u8j,u8z;

    for(u8i = 0; u8i < protocalCacheCnt; u8i++)
    {
        u8j = 0;
        if(protocalCache[u8i] == u8null[u8j])
        {
            for(u8z = u8i + 1,u8j = 1; u8z < protocalCacheCnt; u8z++,u8j++)
            {
                if(protocalCache[u8z] != u8null[u8j])
                    break;
                //消息详情中有null:null
                if(u8j >= 8)
                    return TRUE;
            }
        }
    }
    return FALSE;
}

//**********************************************************************
// 函数功能:	计算完整单个字的UTF-8编码长度
// 输入参数：	pInput: UTF-8编码
// 返回参数：	无
static int16_t GetSingleUTF8CodeLen(const uint8_t pInput)
{
	unsigned char c = pInput;
	// 0xxxxxxx 返回0
	// 10xxxxxx 不存在
	// 110xxxxx 返回2
	// 1110xxxx 返回3
	// 11110xxx 返回4
	// 111110xx 返回5
	// 1111110x 返回6
	if(c< 0x80) return 0;
	if(c>=0x80 && c<0xC0) return -1;
	if(c>=0xC0 && c<0xE0) return 2;
	if(c>=0xE0 && c<0xF0) return 3;
	if(c>=0xF0 && c<0xF8) return 4;
	if(c>=0xF8 && c<0xFC) return 5;
	if(c>=0xFC) return 6;
	return -1;
}
	
//**********************************************************************
// 函数功能:	将一个字符的UTF8编码转换成Unicode(UCS-2和UCS-4)编码
// 输入参数：	UTF8code：  以UTF-8编码数据流
//				Unicode：   转换生成的Unicode编码
// 返回参数：	成功则返回该字符的UTF8编码所占用的字节数; 失败则返回0. 
// * 注意: 
// *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
// *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
// *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
static uint16_t SINGLE_UTF8_TO_UNICODE(const uint8_t* UTF8code, uint16_t *Unicode)  
{
//    assert(pInput != NULL && Unic != NULL);  
  
    // b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...  
	char b1, b2, b3, b4, b5, b6;  
  
    *Unicode = 0x0; // 把 *Unic 初始化为全零  
    int utfbytes = GetSingleUTF8CodeLen(*UTF8code);  
    unsigned char *pOutput = (unsigned char *) Unicode;  
  
	switch ( utfbytes )  
    {  
		case 0:  
        *pOutput     = *UTF8code;  
        utfbytes    += 1;  
        break;  

        case 2:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        if ( (b2 & 0xE0) != 0x80 )  
            return 0;  
        *pOutput     = (b1 << 6) + (b2 & 0x3F);  
        *(pOutput+1) = (b1 >> 2) & 0x07;  
        break;  

        case 3:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b2 << 6) + (b3 & 0x3F);  
        *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);  
        break;  

        case 4:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        b4 = *(UTF8code + 3);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                || ((b4 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b3 << 6) + (b4 & 0x3F);  
        *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);  
        *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);  
        break;  

        case 5:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        b4 = *(UTF8code + 3);  
        b5 = *(UTF8code + 4);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b4 << 6) + (b5 & 0x3F);  
        *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);  
        *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);  
        *(pOutput+3) = (b1 << 6);  
        break; 

        case 6:  
        b1 = *UTF8code;  
        b2 = *(UTF8code + 1);  
        b3 = *(UTF8code + 2);  
        b4 = *(UTF8code + 3);  
        b5 = *(UTF8code + 4);  
        b6 = *(UTF8code + 5);  
        if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)  
                || ((b6 & 0xC0) != 0x80) )  
            return 0;  
        *pOutput     = (b5 << 6) + (b6 & 0x3F);  
        *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);  
        *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);  
        *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);  
        break;  

        default:  
        return 0;  
//            break;  
    }   
    return utfbytes;  
}

//**********************************************************************
// 函数功能:	来电解析处理
// 输入参数：	无
// 返回参数：	无
static void App_RemindManage_CallProcess(uint8 phoneSystem)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	phoneType;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes,utfbytes1,utfbytes2;
	uint16_t 	unicodeTemp,unicodeTemp1,unicodeTemp2;

	uint8_t 	endFlagNum = 0;

	/* 清空旧数据，以免对新数据产生影响 */
	memset(appRemindMsg.data,0,sizeof(appRemindMsg));

	/* 把UTF8转化成Unicode编码,使用前三位作为是否在通讯录判断 */
	if (phoneSystem == IOS)
	{
		if (*(protocalCache + utfbytesIndex) == 0xE2)	//ＩＯＳ号码前　Ｅ2　80　ＡＤ
		{
			utfbytesIndex += 3;
		}
	}
	utfbytes 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);
	utfbytesIndex 	+= utfbytes;
	utfbytes1 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp1);
	utfbytesIndex 	+= utfbytes1;
	utfbytes2 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp2);
	utfbytesIndex 	+= utfbytes2;

	if (utfbytes == 1 && utfbytes1 == 1 && utfbytes2 == 1)
	{
		//开始3个均为数字
		if (((unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039) || unicodeTemp == 0x002B) && 
			(unicodeTemp1 >= 0x0030 && unicodeTemp1 <= 0x0039) &&
			(unicodeTemp2 >= 0x0030 && unicodeTemp2 <= 0x0039))
		{
			phoneType = PHONE_UNFAMILIAR;	// 非通信录联系人
		}
		else
		{
			phoneType 		= PHONE_FAMILIAR;	// 通信录联系人
			utfbytesIndex 	= 0;
		}
	}
	else
	{
		phoneType 			= PHONE_FAMILIAR;	// 通信录联系人
		utfbytesIndex 		= 0;
	}

    switch(phoneType)
    {
    	case PHONE_UNFAMILIAR:	// 号码开头，非通信录联系人
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp1;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp2;
			
			for(;;)	//号码解析
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}

				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);

				if (utfbytes == 1)//ACSII码
				{
					if (unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039)
					{
						appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp;
					}
					else if(unicodeTemp == SECT_END)
					{
						utfbytesIndex 	+= utfbytes;
						break;
					}
					utfbytesIndex 	+= utfbytes;
				}
				else
				{
					break;//非ACSII码
				}
			}

			//归属地解析
			for(;;)
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp); 

				if(utfbytes == 0)//异常错误
					break;
				
				switch(endFlagNum)
				{
					case 0://内容-归属地
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.ascription[appRemindMsg.remindMsg.content.u.ascriptionlen++] = unicodeTemp;
						}	    		
						break;
					case 1://内容-呼入电话
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen++] = unicodeTemp;
						}
						break;
					case 2: break;	//无内容
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			
			/* 向上层发送来电提醒消息 */
			APP_REMIND_RTT_LOG(0, "PHONE_UNFAMILIAR \r\n");
			
			#if 1	// debug
			// 电话号码
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.phonenumberlen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.phonenumber[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");

			// 归属地
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.ascriptionlen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.ascription[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			// 消息内容
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");				
			#endif

			break;
    	case PHONE_FAMILIAR:	//非号码开头，是通讯录联系人
			for(;;)
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);  

				if(utfbytes == 0)//异常错误
					break;

				switch(endFlagNum)
				{
					case 0://姓名
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen++] = unicodeTemp;
						}
						break;
					case 1:	//内容-呼入电话
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen++] = unicodeTemp;
						}
						break;
					case 2: break;	//无内容
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			
			APP_REMIND_RTT_LOG(0, "PHONE_FAMILIAR \r\n");
			
			#if 1	// debug
			// 联系人姓名
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.namelen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.name[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			// 消息内容：呼入电话
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");				
			#endif
			break;
			default: break;
    }
}

//**********************************************************************
// 函数功能:	未接来电解析处理
// 输入参数：	无
// 返回参数：	无
// 注：有些安卓手机收不到未接来电提醒
static void App_RemindManage_MissCallProcess(uint8 phoneSystem)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	phoneType;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes,utfbytes1,utfbytes2;
	uint16_t 	unicodeTemp,unicodeTemp1,unicodeTemp2;

	uint8_t 	endFlagNum = 0;

	//清除信息结构体
	memset(appRemindMsg.data,0,sizeof(appRemindMsg));

	//把UTF8转化成Unicode编码,使用前三位作为是否在通讯录判断
	if (phoneSystem == IOS)
	{
		if (*(protocalCache + utfbytesIndex) == 0xE2)//ＩＯＳ号码前　Ｅ2　80　ＡＤ
		{
			utfbytesIndex += 3;
		}
	}
	utfbytes 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);
	utfbytesIndex 	+= utfbytes;
	utfbytes1 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp1);
	utfbytesIndex 	+= utfbytes1;
	utfbytes2 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp2);
	utfbytesIndex 	+= utfbytes2;

	if (utfbytes == 1 && utfbytes1 == 1 && utfbytes2 == 1)
	{
		//开始3个均为数字或有+
		if (((unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039) || unicodeTemp == 0x002B)&& 
			(unicodeTemp1 >= 0x0030 && unicodeTemp1 <= 0x0039) &&
			(unicodeTemp2 >= 0x0030 && unicodeTemp2 <= 0x0039))
		{
			phoneType = PHONE_UNFAMILIAR;
		}
		else
		{
			phoneType 		= PHONE_FAMILIAR;
			utfbytesIndex	= 0;
		}
	}
	else
	{
		phoneType 			= PHONE_FAMILIAR;
		utfbytesIndex		= 0;
	}

    switch(phoneType)
    {
    	case PHONE_UNFAMILIAR: 	//号码开始，不是通讯录联系人
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp1;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp2;
			
			for(;;)//号码解析
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}

				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);

				if (utfbytes == 1)//ACSII码
				{
					if (unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039)
					{
						appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp;
					}
					else if(unicodeTemp == SECT_END)
					{
						utfbytesIndex 	+= utfbytes;
						break;
					}
					utfbytesIndex 	+= utfbytes;
				}
				else
				{
					break;//非ACSII码
				}
			}

			for(;;)	//内容解析
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);   	

				if(utfbytes == 0)//异常错误
					break;

				switch(endFlagNum)
				{
					case 0://内容-归属地
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.ascription[appRemindMsg.remindMsg.content.u.ascriptionlen++] = unicodeTemp;
						}
						break;
					case 1://内容-呼入电话
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen++] = unicodeTemp;							
						}
						break;
					case 2: break;
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			
			/* 向上层发送提醒消息 */
			APP_REMIND_RTT_LOG(0, "PHONE_UNFAMILIAR \r\n");
			
			#if 1	// debug
			// 电话号码
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.phonenumberlen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.phonenumber[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");

			// 归属地
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.ascriptionlen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.ascription[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			// 消息内容
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");				
			#endif
			break;
    	case PHONE_FAMILIAR:	//非号码开头，是通讯录联系人
			for(;;)
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp); 

				if(utfbytes == 0)//异常错误
					break;

				switch(endFlagNum)
				{
					case 0://姓名
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen++] = unicodeTemp;
						}	    		
						break;
					case 1://内容-呼入电话
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen++] = unicodeTemp;
						}
						break;
					case 2: break;
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			
			APP_REMIND_RTT_LOG(0, "PHONE_FAMILIAR \r\n");
			
			#if 1	// debug
			// 联系人姓名
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.namelen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.name[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			// 消息内容：呼入电话
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");				
			#endif			
			break;
    	default: break;
    }
    #if 0
    //解析完，保存
	Mid_RemindMsg_SaveList((uint8_t *)(appRemindMsg.data + 6), APP_REMIND_MSG_VALID_MAX_LENGHT, REMIND_CALL_PHONE);
	Mid_RemindMsg_SaveListEnd(REMIND_CALL_PHONE);
	#endif

}

//**********************************************************************
// 函数功能:	短信信息解析处理
// 输入参数：	无
// 返回参数：	无
static void App_RemindManage_MessgeProcess(uint8 phoneSystem)
{
	uint8_t 	phoneType;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes,utfbytes1,utfbytes2;
	uint16_t 	unicodeTemp,unicodeTemp1,unicodeTemp2;
	uint8_t 	endFlagNum = 0;

	//清除信息结构体
	memset(appRemindMsg.data,0,sizeof(appRemindMsg));

	//把UTF8转化成Unicode编码,使用前三位作为是否在通讯录判断
	if (phoneSystem == IOS)
	{
		if (*(protocalCache + utfbytesIndex) == 0xE2)//ＩＯＳ号码前　Ｅ2　80　ＡＤ
		{
			utfbytesIndex += 3;
		}
	}
	utfbytes 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);
	utfbytesIndex 	+= utfbytes;
	utfbytes1 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp1);
	utfbytesIndex 	+= utfbytes1;
	utfbytes2 		= SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp2);
	utfbytesIndex 	+= utfbytes2;

	if (utfbytes == 1 && utfbytes1 == 1 && utfbytes2 == 1)
	{
		//开始3个均为数字
		if (((unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039) || unicodeTemp == 0x002B)&& 
			(unicodeTemp1 >= 0x0030 && unicodeTemp1 <= 0x0039) &&
			(unicodeTemp2 >= 0x0030 && unicodeTemp2 <= 0x0039))
		{
			phoneType = PHONE_UNFAMILIAR;
		}
		else
		{
			phoneType 		= PHONE_FAMILIAR;
			utfbytesIndex	= 0;
		}
	}
	else
	{
		phoneType 			= PHONE_FAMILIAR;
		utfbytesIndex		= 0;
	}

	switch(phoneType)
	{
		case PHONE_UNFAMILIAR: 	//号码开始，不是通讯录联系人
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp1;
			appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp2;
		
			for(;;)	//号码解析
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}

				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);

				if (utfbytes == 1)//ACSII码
				{
					if (unicodeTemp >= 0x0030 && unicodeTemp <= 0x0039)
					{
						appRemindMsg.remindMsg.phonenumber[appRemindMsg.remindMsg.phonenumberlen++] = unicodeTemp;	
					}
					else if(unicodeTemp == SECT_END)	
					{
						utfbytesIndex 	+= utfbytes;
						break;
					}
					utfbytesIndex 	+= utfbytes;
				}
				else
				{
					break;//非ACSII码
				}
			}

			for(;;)	//信息内容解析
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);  

				if(utfbytes == 0)//异常错误
					break;

				switch(endFlagNum)
				{
					case 0://内容-信息内容
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen++] = unicodeTemp;
							if(appRemindMsg.remindMsg.content.u.detaillen >= APP_REMIND_DATAIL_MAX_LENGHT)
							{
								endFlagNum += 1;
							}
						}	    		
						break;
					case 1: break;
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			
			/* 向上层发送提醒消息 */

			#if 1	// debug
			APP_REMIND_RTT_LOG(0, "PHONE_UNFAMILIAR \r\n");
			
			// 电话号码
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.phonenumberlen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.phonenumber[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			// 消息内容
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");				
			#endif
			break;
		case PHONE_FAMILIAR:	//非号码开头，是通讯录联系人
			for(;;)
			{
				if (utfbytesIndex >= protocalCacheCnt)
				{
					break;
				}
				utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);   		

				if(utfbytes == 0)//异常错误
					break;

				switch(endFlagNum)
				{
					case 0:	//姓名
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen++] = unicodeTemp;
							if(appRemindMsg.remindMsg.namelen >= APP_REMIND_NANE_MAX_LENGHT)
							{
								endFlagNum += 1;
							}
						}	    		
						break;
					case 1:	//内容-信息内容
						if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
						{
							endFlagNum += 1;
						}
						else
						{
							appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen++] = unicodeTemp;
							if(appRemindMsg.remindMsg.content.u.detaillen >= APP_REMIND_DATAIL_MAX_LENGHT)
							{
								endFlagNum += 1;
							}
						}
						break;
					case 2: break;
					default: break;
				}
				utfbytesIndex 	+= utfbytes;
			}
			
			#if 1	// debug
			APP_REMIND_RTT_LOG(0, "PHONE_FAMILIAR \r\n");
			
			// 联系人姓名
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.namelen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.name[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			// 短信内容
			for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
			{
				APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
			}APP_REMIND_RTT_LOG(0,"\r\n");				
			#endif				
			break;
		default: break;
	}

    #if 0
    //解析完，保存
	Mid_RemindMsg_SaveList((uint8_t *)(appRemindMsg.data + 5), APP_REMIND_MSG_VALID_MAX_LENGHT, REMIND_MSG_BRIEF);
	Mid_RemindMsg_SaveListEnd(REMIND_MSG_BRIEF);	
	#endif
}

//**********************************************************************
// 函数功能:	微信信息解析处理
// 输入参数：	无
// 返回参数：	无
static void App_RemindManage_MsgDetailProcess(uint8 phoneSystem,uint8_t msg_type)
{
	// uint32_t 	msgLenghtCnt 	= 0;
	uint8_t 	utfbytesIndex 	= 0;
	uint16_t 	utfbytes;
	uint16_t 	unicodeTemp;
	uint8_t 	endFlagNum = 0;
	uint32_t 	size = 0;
//	menuMessage message={0};

	//清除信息结构体
	size = sizeof(appRemindMsg);
	memset(appRemindMsg.data,0,size);

	for(;;)
	{
		if (utfbytesIndex >= protocalCacheCnt)
		{
			break;
		}
		utfbytes 	 = SINGLE_UTF8_TO_UNICODE((const uint8_t*)(protocalCache + utfbytesIndex),&unicodeTemp);   		
		
		if(utfbytes == 0)//异常错误
			break;

		switch(endFlagNum)
		{
			case 0://微信名
            {
                switch(msg_type)
                {
                    case WECHAT_MESAGE:
                    if (utfbytes == 1 && (unicodeTemp == SECT_END))//冒号
                    {
                        endFlagNum += 1;
                    }
                    else
                    {
                        appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
                        appRemindMsg.remindMsg.namelen ++;

                        if(appRemindMsg.remindMsg.namelen >= APP_REMIND_NANE_MAX_LENGHT)
                        {
                            endFlagNum += 1;
                        }
                    }                   
                    break;
                    
                    default:
                    if (utfbytes == 1 && (unicodeTemp == SECT_END))//冒号
                    {
                        endFlagNum += 1;
                    }
                    else
                    {
                        appRemindMsg.remindMsg.name[appRemindMsg.remindMsg.namelen] = unicodeTemp;
                        appRemindMsg.remindMsg.namelen ++;

                        if(appRemindMsg.remindMsg.namelen >= APP_REMIND_NANE_MAX_LENGHT)
                        {
                            endFlagNum += 1;
                        }
                    }
                    break;
                }
            }  		
			break;

			case 1://内容-信息内容
			if (utfbytes == 1 && (unicodeTemp == SECT_END))//空字符
    		{
    			endFlagNum += 1;
    		}
    		else
    		{
    			appRemindMsg.remindMsg.content.u.detail[appRemindMsg.remindMsg.content.u.detaillen] = unicodeTemp;
				appRemindMsg.remindMsg.content.u.detaillen ++;
				
				if(appRemindMsg.remindMsg.content.u.detaillen >= APP_REMIND_DATAIL_MAX_LENGHT)
				{
					endFlagNum += 1;
				}
    		}
			break;

			case 2://无内容

			break;

			default:
			break;
		}
		utfbytesIndex 	+= utfbytes;
	}

	// 打印消息内容Unicode编码
	for(uint32_t t = 0; t < appRemindMsg.remindMsg.content.u.detaillen;t++)
	{
		APP_REMIND_RTT_LOG(0,"\\u%04X", appRemindMsg.remindMsg.content.u.detail[t]);
	}APP_REMIND_RTT_LOG(0,"\r\n");	
	
	/* 通知上层对消息数据进行处理 */
//	message.state = NEW_REMIND_STATE;           
//    message.op    = UASUAL_MSG_DETAIL_REMIND;
//    WinDowHandle  = App_Window_Process(WinDowHandle,TAG_REMIND,message);
	
	#if 0
    //解析完，保存
	Mid_RemindMsg_SaveList((uint8_t *)(appRemindMsg.data + 5), APP_REMIND_MSG_VALID_MAX_LENGHT, REMIND_MSG_WEIXIN);
	Mid_RemindMsg_SaveListEnd(REMIND_MSG_WEIXIN);
	#endif	
}

//**********************************************************************
// 函数功能:	初始化提醒信息存储
// 输入参数：	无
// 返回参数：	无
void App_RemindManage_Init(void)
{

}

//**********************************************************************
// 函数功能:	未操作的提醒信息总条数获取
// 输入参数： 	msgClassify：       信息类型
// 				listNum： 			信息编号，记录根据接收到的先后顺序从1开始编号
// 返回参数：	无

void App_RemindManage_MissRemindSumRead(uint32_t msgClassify,uint8_t *listSumNum)
{

}

//**********************************************************************
// 函数功能:	提醒信息读取
// 输入参数：	dailyMissRemindMsg: 存储信息的结构体
// 				msgClassify：       信息类型
// 				listNum： 			信息编号，记录根据接收到的先后顺序从1开始编号
// 返回参数：	无

void App_RemindManage_MissRemindRead(app_remind_msg_t *dailyMissRemindMsg,uint32_t msgClassify,uint8_t listNum)
{
//	if (msgClassify > REMIND_CLASSIFY_NUM)
//	{
//		return;
//	}

//	if (misssRemindNum[msgClassify] == 0)
//	{
//		return;
//	}

//	if (listNum == 0 || listNum > APP_REMIND_MSG_MAX_LIST)
//	{
//		return;
//	}

//	Mid_RemindMsg_Read(dailyMissRemindMsg->data,misssRemindCatalog[msgClassify][listNum - 1],msgClassify);	
}

//**********************************************************************
// 函数功能:	删除一条提醒信息
// 输入参数：	msgClassify: 删除的类型
// 				listnum： 	 删除的编号，记录根据接收到的先后顺序从1开始编号
// 返回参数：	无

void App_RemindManage_MissRemindDelete(uint32_t msgClassify,uint8_t listnum)
{
//	uint8_t 	i;

//	if (msgClassify > REMIND_CLASSIFY_NUM)
//	{
//		return;
//	}

//	if (misssRemindNum[msgClassify] == 0)
//	{
//		return;
//	}

//	if (listnum == 0)
//	{
//		return;
//	}
//	//目录更新
//	for (i = listnum - 1; i < misssRemindNum[msgClassify] - 1; ++i)
//	{
//		misssRemindCatalog[msgClassify][i] 	= misssRemindCatalog[msgClassify][i + 1];
//	}
//	misssRemindCatalog[msgClassify][i] 		= 0;
//	misssRemindNum[msgClassify] 			-= 1;	
}

//**********************************************************************
// 函数功能:	读取当前缓存的提醒信息
// 输入参数：	protocal： 协议内容
// 返回参数：	无
void App_RemindManage_DetailRemindRead(app_remind_msg_t *detailRemindMsg)
{
	*detailRemindMsg  = appRemindMsg;
}

//**********************************************************************
// 函数功能:	提醒信息解析处理，先把数据存入CACHE
// 输入参数：	protocal： 协议内容
// 返回参数：	无
// 调用：收到app提醒消息是调用一次
void App_RemindManage_Process(protocal_msg_t protocal,uint8 phoneSystem)
{
	static uint8_t firstPackFlag = 1;

	switch(protocal.att.load.content.parameter[1])
	{
		case FLOW_END:
			if (firstPackFlag)//只有一包的情形需作特别处理
			{
				memset(protocalCache,0,APP_REMIND_MSG_MAX_LENGHT);
				flowCnt 			= 0;
				protocalCacheCnt 	= 0;
			}
			
			for(uint8_t i = 2; i <= protocal.att.loadLength - 4; i ++)
			{
				if (protocalCacheCnt >= APP_REMIND_MSG_MAX_LENGHT)
				{
					break;
				}
				protocalCache[protocalCacheCnt] = protocal.att.load.content.parameter[i];
				protocalCacheCnt ++;    
			}
			firstPackFlag = 1;	//接收到尾包，下一次接收应为第一次包开始

			//fix :如果消息详情是无效的，就不显示,否则会显示"null:null"
			if(App_RemindManag_IsNULL() == TRUE)
				return;
			//fix: 2018.6.26

			// utf-8编码原始数据打印
//			for(uint32_t t = 0; t < protocalCacheCnt;t++)
//			{
//				APP_REMIND_RTT_LOG(0,"%02X ",protocalCache[t]);
//			}APP_REMIND_RTT_LOG(0,"\r\n");
			
			switch(protocal.att.load.content.parameter[0])//信息接收完，分别作处理
			{
				case PHONE_CALL_IN:					
				case FAMILIAR_PHONE_CALL:	// 来电提醒
					App_RemindManage_CallProcess(phoneSystem);
					break;
				case MISS_PHONE_CALL:
					App_RemindManage_MissCallProcess(phoneSystem);
					break;
				case PHONE_MESSAGE:
					App_RemindManage_MessgeProcess(phoneSystem);
					break;
				case TIM_MESAGE:
				case WECHAT_MESAGE:
				case QQ_MESAGE:
					App_RemindManage_MsgDetailProcess(phoneSystem,protocal.att.load.content.parameter[0]);
					break;
				default:
					App_RemindManage_MsgDetailProcess(phoneSystem,protocal.att.load.content.parameter[0]);
					break;
			}
			break;
		case FLOW_START://如果是第一包数据，清0该数组
			memset(protocalCache,0,APP_REMIND_MSG_MAX_LENGHT);
			flowCnt 			= 0;
			protocalCacheCnt 	= 0;
			firstPackFlag 		= 0;//首包接收到，清标识
		default:	//只作数据入缓存		
			if (flowCnt != protocal.att.load.content.parameter[1])
			{
				return;
			}

			for(uint8_t i = 2; i <= protocal.att.loadLength - 4; i ++)
			{	
				if (protocalCacheCnt >= APP_REMIND_MSG_MAX_LENGHT)
				{
					break;
				}
				protocalCache[protocalCacheCnt] = protocal.att.load.content.parameter[i];
				protocalCacheCnt ++;    
			}
			flowCnt ++;
			break;
	}
}




  

