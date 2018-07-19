#ifndef _ad7545_h
#define _ad7545_h

#include "sys.h"


/*  AD7545 FSMC���ߵ�ַ ֻ��д�����ܶ�  */
#define AD7545_RESULT()	*(__IO uint16_t *)0x6C400000   //BANK1 NE4 ��ַ 0x6C00 0000 ~0x6fff ffff

#define DAC7545_Vref 6.1895 //AD7545�� ��׼
#define DA_Full		0xfff

#define MAX_USR_Bfb		153.3 	 	//����Դ��������ѹ 
#define MAX_Vol_DC		5.0			//����Դ��������ѹ(AC) ----> ��Ӧ ֱ����ѹ ����
#define Bfb_pre				100.0		//Ԥ���ٷֱ�


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
