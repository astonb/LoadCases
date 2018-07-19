#include "ad7545.h"
#include "delay.h"
#include "test.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "semphr.h"

volatile u16 DA_Now = 0;  //AD7545µ±Ç°¶ÔÓ¦µÄÂëÖµ

extern volatile double FZ_bfb_gt;

void ad7545_FsmcConfig(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
		FSMC_NORSRAMTimingInitTypeDef  timing; 
			
		RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOG,ENABLE);

		//PORTD
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;	//	//PORTD¸
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
		GPIO_Init(GPIOD, &GPIO_InitStructure); 
	
		//PORTE
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD¸´ÓÃÍÆÍìÊä³ö  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		
		GPIO_Init(GPIOE, &GPIO_InitStructure);   
		
		//PG12-NE4
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		
		GPIO_Init(GPIOG, &GPIO_InitStructure); 
		
		
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);//D2
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);//D3
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);//NOE
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);//NWE
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);//D13
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);//D14
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);//D15
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);//D0
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);//D1
		
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);//D4
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);//D5
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);//D6
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);//D7
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);//D8
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);//D9
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);//D10
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);//D11
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//D12
 
		GPIO_PinAFConfig(GPIOG,GPIO_PinSource12,GPIO_AF_FSMC);//NE4
		
	/* FSMC_Bank1_NORSRAM4 configuration */
		timing.FSMC_AddressSetupTime = 10; 	            //µØÖ·½¨Á¢Ê±¼ä(¶ÔÓ¦RDµÍµçÆ½³ÖÐøÊ±¼ä)  ,¶ÔÓÚSTM32F1 HCLK=13.8ns(1/72M) Ôò40*13.8=552ns  
		timing.FSMC_AddressHoldTime  = 5;                //µØÖ·±£³ÖÊ±¼ä                                                AD7606 RDµÍµçÆ½Tmin= 
		timing.FSMC_DataSetupTime    =	15;               // 6*13.8ns=82.8ns ¶ÔÓ¦RD¸ßµçÆ½³ÖÐøÊ±¼ä (Tmin=15ns)																			Vdrive>4.75V  --- 16ns
		timing.FSMC_BusTurnAroundDuration= 0;						//			×ÜÏß»Ö¸´Ê±¼ä																																			Vdrive>3.3V   --- 21ns
		timing.FSMC_CLKDivision = 0;										//																																						Vdrive>2.7V   --- 25ns
		timing.FSMC_DataLatency = 0;										//																																						Vdrive>2.3V		--- 32ns			
		timing.FSMC_AccessMode = FSMC_AccessMode_A;	 //Ä£Ê½A  ¶ÁÐ´Ê±Ðò¿É²»Í¬
		
   	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;  //Ê¹ÓÃNE4£¬¶ÔÓ¦BTCR[6][7]:																														//BTCR[6]=FSMC_BCR4; BTCR[7]=FSMC_BTR4
		FSMC_NORSRAMInitStructure.FSMC_DataAddressMux  = FSMC_DataAddressMux_Disable; //²»¸´ÓÃµØÖ·Ïß
		FSMC_NORSRAMInitStructure.FSMC_MemoryType      = FSMC_MemoryType_SRAM;
		FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
		FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; //Í»·¢·ÃÎÊÄ£Ê½¹Ø±Õ
		FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
		FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait   = FSMC_AsynchronousWait_Disable; //ÊÇ·ñÊ¹ÓÃÒì²½µÈ´ýÐÅºÅ
		FSMC_NORSRAMInitStructure.FSMC_WrapMode  = FSMC_WrapMode_Disable;     //ÊÇ·ñÊ¹ÓÃ»Ø»·Ä£Ê½
		FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState; //ÉèÖÃWAITÐÅºÅµÄÓÐÐ§Ê±»ú
		FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;   //´æ´¢Æ÷Ð´Ê¹ÄÜ
		FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   //WAITÐÅºÅ²»Ê¹ÄÜ
		FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;  //¶ÁÐ´Ê±Ðò²»Í¬  Disable : ²»Ê¹ÄÜ¶ÁÐ´Ê±Ðò²»Í¬
		FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
		
		FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &timing;        //ÏÂÍ¬£¬¶ÁÐ´Ê±ÐòÏàÍ¬
		FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &timing;
		
		FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //³õÊ¼»¯FSMCÅäÖÃ
		
		/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */ 
		FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4,ENABLE);   //Ê¹ÄÜBANK1
//		FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3,ENABLE);

		AD7545_RESULT()=0x0;
}
static void mdelay_ms(uint32_t ms)
{
	uint32_t i=0,j=0;
	
	for(i=0;i<ms;i++)
		for(j=0;j<168000;j++);
}
/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºstatic enum status void Out_7545_To(u16 DA_To)
*	¹¦ÄÜËµÃ÷ : µç×ÓÔ´µ×²ãÊä³öº¯Êý
*	ÐÎ    ²Î : ÓÃ»§Éè¶¨µÄ°Ù·Ö±í
*	·µ »Ø Öµ : ×´Ì¬status: 
* Ê¹ÓÃËµÃ÷ £º
*********************************************************************************************************
*/
static enum state Out_7545_To(u16 DA_To)
{
//	enum state sta = Normal;
	vTaskSuspendAll();
	while(DA_Now != DA_To)
	{
		if(DA_Now<DA_To)
		{
			if(DA_Now == DA_Full)	return	IsTop;   //µ½¶¥
			DA_Now++;
			AD7545_RESULT() = DA_Now;
			if(DA_Now == DA_Full)
			{
				if(DA_Now<DA_To)
					return ErrIsTop;			//Òì³££¬µ½¶¥
			}
		}
		if(DA_Now>DA_To)
		{
			if(DA_Now == 0)	return	IsBom;	//µ½µ×
			DA_Now--;
			AD7545_RESULT() = DA_Now;
			if(DA_Now == 0)
			{				
				if(DA_Now>DA_To)
					return	ErrIsBom;					//Òì³££¬µ½µ×
			}
		}
		mdelay_ms(1);
	}
	xTaskResumeAll();
	
	AD7545_RESULT() = DA_Now;
	
	return (Normal);
}
/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºenum state Out_Bfb_To(float usr_bfb)
*	¹¦ÄÜËµÃ÷ : µç×ÓÔ´Êä³ö´Öµ÷
*	ÐÎ    ²Î : ÓÃ»§Éè¶¨µÄ°Ù·Ö±í
*	·µ »Ø Öµ : 
* Ê¹ÓÃËµÃ÷ £º
*********************************************************************************************************
*/
enum state Out_Bfb_To(float usr_bfb)
{
	enum state sta = Normal;
	u16	DA_Val = 0;
	
	if(usr_bfb >120.0f)		//°Ù·Ö±íÇ¯Î»±£»¤
		usr_bfb = 120.0f;  
	
	DA_Val = (int)((double)usr_bfb/120.0f*6.0f/DAC7545_Vref*4096)&0xfff;
	
	sta = Out_7545_To(DA_Val);
	
	return (sta);
	
}
/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºenum state Out_Bfb_Adj(float usr_bfb,u8 test_mode)
*	¹¦ÄÜËµÃ÷ : µç×ÓÔ´Êä³ö¾«È·Êä³ö£¨´øµ÷½Ú£©
*	ÐÎ    ²Î : usr_bfb £º ÓÃ»§Éè¶¨µÄ°Ù·Ö±í   test_mode £º ×è¿¹²âÊÔ or µ¼ÄÉ²âÊÔ  Adjust_mode £ºROUGH ´Öµ÷  ACCURATE Ï¸µ÷
*	·µ »Ø Öµ : 
* Ê¹ÓÃËµÃ÷ £ºcnt ÓÃÓÚµ÷Õû´ÎÊý¼ÇÂ¼£¬µ±´óÓÚµ÷Õû9´ÎÊ±£¬Ôòµ÷Õû³¬Ê±£¬ÍË³öµ÷Õû
*********************************************************************************************************
*/
extern float uppre_val;
extern u8 gSystemReset;

enum state Out_Bfb_Adj(const float usr_bfb,const u8 test_mode,const enum DZY_Adjust Adjust_mode)
{
	enum state sta = Normal;
	float bfb_temp = 0;
	u8 cnt=0;
	float upper = 0;
	
	cnt = 0;
	upper = usr_bfb/uppre_val*10;
	sta = Out_Bfb_To(upper);  
	
	if(Adjust_mode == ACCURATE)
	{
		while(((bfb_temp<0.98f*usr_bfb) || (bfb_temp>1.02f*usr_bfb))&&(cnt<9))
		{
			if(gSystemReset == 1)
			{
				Out_Bfb_To(0);
				sta = AbnormalReset;
				gSystemReset = 0;
				break;
			}
//			while(FZX_Test(test_mode)!=TURE);
			while(FZX_Test(test_mode)!=TURE);
			while(FZX_Test(test_mode)!=TURE);				//¶à´Î²âÁ¿µÃµ½ÎÈ¶¨Ê±µÄµ±Ç°°Ù·Ö±íÊý¾Ý
			
			printf("bfb=%.4f    cnt = %d\r\n",FZ_bfb_gt,cnt);
			
			bfb_temp = FZ_bfb_gt;
			
			if((bfb_temp>=0.98f*usr_bfb)&&(bfb_temp<=1.02f*usr_bfb))	
			{
				cnt = 0;
				printf("µ÷ÕûÍê³É\r\n");
				return	Normal;     //µ±Ç°°Ù·Ö±íÂú×ãÉè¶¨ÒªÇó£¬ÍË³öµ÷Õû
			}
			
			upper *= (usr_bfb/bfb_temp);
			
			sta = Out_Bfb_To(upper);
			
			cnt++;
			
			if(cnt == 9)
			{
				cnt = 0;
				printf("µ÷Õû³¬Ê±\r\n");
				return TimeOut;
			}
		}
	}
	
	return (sta);
}
