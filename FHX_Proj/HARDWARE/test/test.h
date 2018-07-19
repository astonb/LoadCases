#ifndef __TEST_H_
#define __TEST_H_

#include "sys.h"

#define Pi					3.14159265358979
#define H_J  				3437.746771
#define ADC_Vref		2.500000
/****************************************/
#define XS_CT0			1.000000
#define BZ_DifRto  	4.000000													//标准通道差分衰减比
#define CZ_DifRto		4.000000													//差值通道差分衰减比
#define Fy_ratio  (((200.000/5.00000)	+1))   //阻抗大电压 分压比
//#define Fy_ratio  (((5.000/1.00000)	+1))   //阻抗大电压 分压比
#define VolIslationRatio	(280.000/7.000)							//导纳电压采样隔离变比
/****************************************/
#define Test_Per		(5)						//连续测5个周期
#define Sampl_fre		(12500)   	  //采样频率12.5K
#define Target_fre	(50)					//采样信号50HZ

#define Dot_P_Cyc		(Sampl_fre/Target_fre)								//250
#define 	N			(Dot_P_Cyc*Test_Per+1)   //总采样点数250*5+1


/*继电器控制(C_Mod_Sel, C_Sampl_Res1(22R), C_Sampl_Res2(2R), C_Sampl_Res1(0.2R))*/
#define On_C_Mod_Sel()   		GPIOC->BSRRL = GPIO_Pin_6;  //置高，吸合
#define Off_C_Mod_Sel()  		GPIOC->BSRRH  = GPIO_Pin_6;   //置低，断开
#define On_C_Sampl_Res1()   GPIOC->BSRRL = GPIO_Pin_7;
#define Off_C_Sampl_Res1()  GPIOC->BSRRH  = GPIO_Pin_7;
#define On_C_Sampl_Res2()   GPIOC->BSRRL = GPIO_Pin_8;
#define Off_C_Sampl_Res2()  GPIOC->BSRRH  = GPIO_Pin_8;
#define On_C_Sampl_Res3()   GPIOC->BSRRL = GPIO_Pin_9;
#define Off_C_Sampl_Res3()  GPIOC->BSRRH  = GPIO_Pin_9;

/**********************DG409通道初始化****************************************/
#define VC0_H()		GPIOE->BSRRL=GPIO_Pin_2;
#define VC0_L()		GPIOE->BSRRH=GPIO_Pin_2;

#define VC1_H()		GPIOE->BSRRL=GPIO_Pin_3;
#define VC1_L()		GPIOE->BSRRH=GPIO_Pin_3;

#define VC2_H()		GPIOC->BSRRL=GPIO_Pin_2;
#define VC2_L()		GPIOC->BSRRH=GPIO_Pin_2;

#define VC3_H()		GPIOC->BSRRL=GPIO_Pin_3;
#define VC3_L()		GPIOC->BSRRH=GPIO_Pin_3;

#define CH1_BIAO()		{GPIOE->BSRRH=GPIO_Pin_2;GPIOE->BSRRH=GPIO_Pin_3;}
#define CH2_BIAO()		{GPIOE->BSRRL=GPIO_Pin_2;GPIOE->BSRRH=GPIO_Pin_3;}
#define CH3_BIAO()		{GPIOE->BSRRH=GPIO_Pin_2;GPIOE->BSRRL=GPIO_Pin_3;}
#define CH4_BIAO()		{GPIOE->BSRRL=GPIO_Pin_2;GPIOE->BSRRL=GPIO_Pin_3;}

#define CH1_CHA()			{GPIOC->BSRRH=GPIO_Pin_2;GPIOC->BSRRH=GPIO_Pin_3;}
#define CH2_CHA()			{GPIOC->BSRRL=GPIO_Pin_2;GPIOC->BSRRH=GPIO_Pin_3;}
#define CH3_CHA()			{GPIOC->BSRRH=GPIO_Pin_2;GPIOC->BSRRL=GPIO_Pin_3;}
#define CH4_CHA()			{GPIOC->BSRRL=GPIO_Pin_2;GPIOC->BSRRL=GPIO_Pin_3;}       
/*******************PGA204放大倍数选择****************************************/
#define CK204_Biao_1()			{GPIOE->BSRRH=GPIO_Pin_4;GPIOE->BSRRH=GPIO_Pin_5;}
#define CK204_Biao_10()			{GPIOE->BSRRH=GPIO_Pin_4;GPIOE->BSRRL=GPIO_Pin_5;}
#define CK204_Biao_100()		{GPIOE->BSRRL=GPIO_Pin_4;GPIOE->BSRRH=GPIO_Pin_5;}
#define CK204_Biao_1000()		{GPIOE->BSRRL=GPIO_Pin_4;GPIOE->BSRRL=GPIO_Pin_5;}


#define CK204_Cha_1()				{GPIOA->BSRRH=GPIO_Pin_0;GPIOA->BSRRH=GPIO_Pin_1;}
#define CK204_Cha_10()			{GPIOA->BSRRH=GPIO_Pin_0;GPIOA->BSRRL=GPIO_Pin_1;}
#define CK204_Cha_100()			{GPIOA->BSRRL=GPIO_Pin_0;GPIOA->BSRRH=GPIO_Pin_1;}
#define CK204_Cha_1000()		{GPIOA->BSRRL=GPIO_Pin_0;GPIOA->BSRRL=GPIO_Pin_1;}

/**************************end************************************************/
#define NowBfb	ZK_Struct.ZK_bfb

enum result
{
	TURE,
	FALSE,
};

typedef struct
{
	volatile double a1[Test_Per];			
	volatile double b1[Test_Per];
	
	volatile double a2[Test_Per];				
	volatile double b2[Test_Per];
	
	volatile int Sum_Biao[Test_Per*Dot_P_Cyc+1];	//标准通道	
	volatile int Sum_Cha[Test_Per*Dot_P_Cyc+1];		//差值通道
	
	volatile double Avg_Biao[Test_Per];					//存放标准通道信号幅值
	volatile double Avg_Cha[Test_Per];					//存放差值通道实际幅值
	
	volatile double Arctan_Biao[Test_Per];			//存放标准通道信号相位
	volatile double Arctan_Cha[Test_Per];				//存放差值通道信号相位
	
	volatile double Arctan_Cha_Biao[Test_Per];  //存放标准通道和差值通道的相位差
				
	volatile double Cos_jiao[Test_Per];      		//存放相位差余弦值
	volatile double Sin_jiao[Test_Per];    		  //存放相位差正弦值
	
	volatile double Cos_jiao_zhi;								//存放最终的相位 cos值
	volatile double Sin_jiao_zhi;								//存放最终的相位 sin值
	
}FFT;

typedef struct
{
	volatile u8 EC_dl;
	volatile u8 Div_flag;
	volatile double ZK;
	volatile float ZK_bfb;
	volatile double ZK_x;
	volatile double ZK_y;
	volatile float UzLimit;
}ZK;
typedef struct
{
	volatile u8 liangch_flag;   //量程标记(0,1,2,3)
	volatile u8 liangch_zdlc;   //自动量程档位标志(,1,2,3(10量程))
	volatile float EC_dy;
	volatile double DN;
	volatile float DN_bfb;
	volatile double DN_x;
	volatile double DN_y;
	volatile u8 Div_flag;
	volatile float UzLimit;
	volatile double SZGL;
	volatile double GLYS;
}DN;



void Test_Param_Init(void);

//void ZK_Test_Init(void);
//void DN_Test_Init(void);
void FZX_Test_Init(u8 mode_flag);

//void FZX_Test(const u8 mode_flag);
enum result FZX_Test(const u8 mode_flag);

void DC_test(void);
#endif
