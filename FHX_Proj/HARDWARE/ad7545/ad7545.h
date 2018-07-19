#ifndef _ad7545_h
#define _ad7545_h

#include "sys.h"


/*  AD7545 FSMC总线地址 只能写，不能读  */
#define AD7545_RESULT()	*(__IO uint16_t *)0x6C400000   //BANK1 NE4 地址 0x6C00 0000 ~0x6fff ffff

#define DAC7545_Vref 6.1895 //AD7545的 基准
#define DA_Full		0xfff

#define MAX_USR_Bfb		153.3 	 	//电子源最大输出电压 
#define MAX_Vol_DC		5.0			//电子源最大输出电压(AC) ----> 对应 直流电压 多少
#define Bfb_pre				100.0		//预升百分表


enum state
{
	IsTop,
	IsBom,
	Normal,
	ErrIsTop,
	ErrIsBom,
	TimeOut,
	AbnormalReset,
};

enum DZY_Adjust
{
	ACCURATE,
	ROUGH,
};

void ad7545_FsmcConfig(void);

enum state Out_Bfb_To(float usr_bfb);
enum state Out_Bfb_Adj(const float usr_bfb,const u8 test_mode,const enum DZY_Adjust Adjust_mode);

#endif
