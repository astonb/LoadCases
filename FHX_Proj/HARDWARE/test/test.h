#ifndef __TEST_H_
#define __TEST_H_

#include "sys.h"

#define Pi					3.14159265358979
#define H_J  				3437.746771
#define ADC_Vref		2.500000
/****************************************/
#define XS_CT0			1.000000
#define BZ_DifRto  	4.000000													//��׼ͨ�����˥����
#define CZ_DifRto		4.000000													//��ֵͨ�����˥����
#define Fy_ratio  (((200.000/5.00000)	+1))   //�迹���ѹ ��ѹ��
//#define Fy_ratio  (((5.000/1.00000)	+1))   //�迹���ѹ ��ѹ��
#define VolIslationRatio	(280.000/7.000)							//���ɵ�ѹ����������
/****************************************/
#define Test_Per		(5)						//������5������
#define Sampl_fre		(12500)   	  //����Ƶ��12.5K
#define Target_fre	(50)					//�����ź�50HZ

#define Dot_P_Cyc		(Sampl_fre/Target_fre)								//250
#define 	N			(Dot_P_Cyc*Test_Per+1)   //�ܲ�������250*5+1


/*�̵�������(C_Mod_Sel, C_Sampl_Res1(22R), C_Sampl_Res2(2R), C_Sampl_Res1(0.2R))*/
#define On_C_Mod_Sel()   		GPIOC->BSRRL = GPIO_Pin_6;  //�øߣ�����
#define Off_C_Mod_Sel()  		GPIOC->BSRRH  = GPIO_Pin_6;   //�õͣ��Ͽ�
#define On_C_Sampl_Res1()   GPIOC->BSRRL = GPIO_Pin_7;
#define Off_C_Sampl_Res1()  GPIOC->BSRRH  = GPIO_Pin_7;
#define On_C_Sampl_Res2()   GPIOC->BSRRL = GPIO_Pin_8;
#define Off_C_Sampl_Res2()  GPIOC->BSRRH  = GPIO_Pin_8;
#define On_C_Sampl_Res3()   GPIOC->BSRRL = GPIO_Pin_9;
#define Off_C_Sampl_Res3()  GPIOC->BSRRH  = GPIO_Pin_9;

/**********************DG409ͨ����ʼ��****************************************/
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
/*******************PGA204�Ŵ���ѡ��****************************************/
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
	
	volatile int Sum_Biao[Test_Per*Dot_P_Cyc+1];	//��׼ͨ��	
	volatile int Sum_Cha[Test_Per*Dot_P_Cyc+1];		//��ֵͨ��
	
	volatile double Avg_Biao[Test_Per];					//��ű�׼ͨ���źŷ�ֵ
	volatile double Avg_Cha[Test_Per];					//��Ų�ֵͨ��ʵ�ʷ�ֵ
	
	volatile double Arctan_Biao[Test_Per];			//��ű�׼ͨ���ź���λ
	volatile double Arctan_Cha[Test_Per];				//��Ų�ֵͨ���ź���λ
	
	volatile double Arctan_Cha_Biao[Test_Per];  //��ű�׼ͨ���Ͳ�ֵͨ������λ��
				
	volatile double Cos_jiao[Test_Per];      		//�����λ������ֵ
	volatile double Sin_jiao[Test_Per];    		  //�����λ������ֵ
	
	volatile double Cos_jiao_zhi;								//������յ���λ cosֵ
	volatile double Sin_jiao_zhi;								//������յ���λ sinֵ
	
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
	volatile u8 liangch_flag;   //���̱��(0,1,2,3)
	volatile u8 liangch_zdlc;   //�Զ����̵�λ��־(,1,2,3(10����))
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
