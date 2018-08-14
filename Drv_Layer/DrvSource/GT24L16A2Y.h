#ifndef _GT24L16A2Y_H_
#define _GT24L16A2Y_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef  unsigned char  u8;
//typedef  unsigned long  u32;
typedef  unsigned short  u16;
typedef  unsigned long   ulong;
typedef  unsigned char   uchar;
typedef  unsigned char   BYTE;
typedef  unsigned short  WORD;
typedef  unsigned long   DWORD;

static DWORD hfmbdd;

extern void r_dat_bat(unsigned long TAB_addr,unsigned int Num,unsigned char *p_arr);


#define ASCII_5X7              1
#define ASCII_7X8              2
#define ASCII_6X12             3
#define ASCII_12_B_A           4
#define ASCII_12_B_T           5
#define ASCII_8X16             6
#define ASCII_8X16_BOLD        7
#define ASCII_16_A             8
#define ASCII_16_T             9

#define B_11X16_A              13
#define B_18X24_A              14
#define B_22X32_A              15
#define B_34X48_A              16
#define B_40X64_A              17
#define B_11X16_T              18
#define B_18X24_T              19
#define B_22X32_T              20
#define B_34X48_T              21
#define B_40X64_T              22
#define T_FONT_20X24           23
#define T_FONT_24X32           24
#define T_FONT_34X48           25
#define T_FONT_48X64           26
#define F_FONT_816             27
#define F_FONT_1624            28
#define F_FONT_1632            29
#define F_FONT_2448            30
#define F_FONT_3264            31
#define KCD_UI_32              32     

#define SEL_GB                 33
#define SEL_JIS                34
#define SEL_KSC                35

void Uncompress12(unsigned char *des,unsigned char *src);
void ram_map_W2Y( unsigned char *getdate,unsigned char *putdata,unsigned char high,unsigned char width);
unsigned char ASCII_GetData(unsigned char ASCIICode,unsigned long ascii_kind,unsigned char *DZ_Data);
unsigned long LATIN_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long CYRILLIC_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long GREECE_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long HEBREW_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long LATIN_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long CYRILLIC_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long GREECE_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
unsigned long ALB_B_GetData(unsigned int unicode_alb,unsigned char *DZ_Data);

//Unicode ת GBK18030
unsigned long  U2GBK(unsigned int  unicode);
unsigned int BIG52GBK( unsigned char h,unsigned char l);
unsigned int U2J(unsigned short Unicode);
unsigned int U2K(unsigned short Unicode);
unsigned int SJIS2JIS(unsigned short code);
void zz_zf(unsigned char Sequence,unsigned char kind,unsigned char *DZ_Data);
unsigned long GB18030_12_GetData(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4,unsigned char *DZ_Data);
#define ntohs(s)             
void hzbmp16(unsigned char charset, unsigned int hzcode, unsigned int codeex, unsigned char lsize,unsigned char *buf);

#endif
