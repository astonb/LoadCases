//////////////////////////////////////////////////////////////////////////////////	 
//��Ŀ����: �¸����������
//����    : ��־��
//�޸�����: 2018-7-21	
//�޸�����: ����˵���ĵ�
//�汾    : 1.1.0
//////////////////////////////////////////////////////////////////////////////////  
#include "./includes.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
/********************������ض��������******************/
#define  START_TASK_PRIO 	3
#define START_STK_SIZE		128
TaskHandle_t StartTaskHandle;
void Start_Task(void const *argumnet);

#define UART1_TASK_PRIO		7
#define	UART1_STK_SIZE		128
TaskHandle_t Uart1TaskHandle;
void Uart1_Task(void const *argument);

#define TEST_TASK_PRIO	5
#define TEST_STK_SIZE		256
TaskHandle_t TestTaskHandle;
void Test_Task(void const *argument);

#define LEDFLASH_TASK_PRIO	4
#define	LEDFLASH_STK_SZIE		128
TaskHandle_t LedFlashTaskHandle;
void LedFlash_Task(void const *argument);

#define DORESET_TASK_PRIO		6
#define DORESET_STK_SIZE 		128
TaskHandle_t DoResetTaskHandle;
void DoReset_Task(void const *argument);

SemaphoreHandle_t ResetSyncSemphr;
//SemaphoreHandle_t DoResetCmdSemphr;
/*********************end******************/
extern MyRTC myRTC;
extern u8 read_flag;    //ADS1274�ɶ���־λ
extern int AD_val[ReadAdcChx];

extern volatile double FZ_gt;    						
extern volatile double FZ_x_gt;		           				
extern volatile double FZ_y_gt;		
extern volatile double FZ_bfb_gt;  
extern volatile double SZGL_gt;		
extern volatile double GLYS_gt;		

extern double Temperature,Humidity; 

extern DN DN_Struct;
extern ZK ZK_Struct;
/**********�������ò�������*************/
typedef struct
{
	u8 test_sel;					//����ѡ�� 0�� �迹  1�� ����
	u8 test_mode;					//����ģʽ   0�� �Զ�  1�� �ֶ�
	float Rated_sec;			//�����: �迹ģʽ�£� 5A,2A,1A  ���ɲ���ģʽ�£� 100V��100/3, 100/��3
	u8 Res_range;					//�������赲λ�� ���ɲ���ģʽ����Ч�� 0: �Զ�����   1: 20R  2: 2R  3: 0.2R
	float test_dot;				//���Ե�
	u8 start_test;				//��������   0:  �ر�	 1: ����
	u8 jdq_flag;					//��ѹ��������ͷ ��־λ��1��200V  2: 100V  3�� 50V ......
	float VA_val;					//���Եķ�����
	float dy_test;				//�ⶨ���ʵ�ʵ�ѹ
}test;

test FHX_test;
/**************************************/


u8 gSystemReset =0;   //ϵͳ��λ��־λ
//float Current_TestDot =0;  //��¼��ǰ���Ե�
float uppre_val =10.0;
u8 IsTestuppre = 0;

float	array_to_int(unsigned char *array);


int main(void)
{ 
	
#if RTC_EN > 0	
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
#endif	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
//	delay_init(168);		  //��ʼ����ʱ����	
	uart_init(115200);	//���ڳ�ʼ��������Ϊ115200

	ad7545_FsmcConfig();  //��ʼ�� AD7545����
	
#if LCD_FSA506_EN > 0
	LCD_Init();
	LCD_FSMC_Init();
#endif

#if RTC_EN > 0
	My_RTC_Init();
#endif	
	
	System_GPIO_Init();
	Test_Param_Init();
	
	SPI1_Init();
	ADS1274_Init();

#if	SHT11_EN > 0
	ConnectionReset();
	SHT_GPIO_Config();
#endif
	//Out_Bfb_To(0);
	AD7545_RESULT() = 0;
	FHX_test.start_test = 0;   //��ʼ��0���رղ���
	
	FZX_Test_Init(0);						//Ĭ���迹ģʽ���迹���Գ�ʼ��
	//������ʼ����
	xTaskCreate((TaskFunction_t)Start_Task, "start task",START_STK_SIZE, NULL, START_TASK_PRIO, &StartTaskHandle);
	//�����������
	vTaskStartScheduler();
	
	while(1)
	{	
		
#if SHT11_EN > 0
		measure_wsd();
		printf("tmpe=%.1f  humi=%.1f\n",Temperature,Humidity);
#endif		

#if RTC_EN	> 0
		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		printf("20%02d-%02d-%02d      %02d:%02d:%02d\r\n",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
#endif	
		
	}

}
//��ʼ����
void Start_Task(void const *argumnet)
{
	taskENTER_CRITICAL();
	
	ResetSyncSemphr = xSemaphoreCreateBinary();
//	DoResetCmdSemphr = xSemaphoreCreateBinary();
	
	xTaskCreate((TaskFunction_t)Uart1_Task, "uart1 task", UART1_STK_SIZE, NULL, UART1_TASK_PRIO,&Uart1TaskHandle);
	xTaskCreate((TaskFunction_t)Test_Task,"test task", TEST_STK_SIZE, NULL, TEST_TASK_PRIO, &TestTaskHandle);
	xTaskCreate((TaskFunction_t)LedFlash_Task, "ledflash test",LEDFLASH_STK_SZIE, NULL, LEDFLASH_TASK_PRIO, &LedFlashTaskHandle);
	xTaskCreate((TaskFunction_t)DoReset_Task, "doreset task", DORESET_STK_SIZE, NULL, DORESET_TASK_PRIO, &DoResetTaskHandle);
	
	vTaskSuspend(StartTaskHandle);
	
	taskEXIT_CRITICAL();
}
//���ڽ���ָ��
void Uart1_Task(void const *argument)
{
	u8 i=0;
	u8 rated_erci = 0;
	
	u8 gRx_bfb[4] = {'0','0','0','0'};  //���ڽ��յ� �ٷֱ����� �ݴ�
	u8 gRx_va[4]  = {'0','0','0','0'};	 //���ڽ��յ� ��������   �ݴ�
	
	while(1)
	{
		if(USART_RX_STA&0x8000)
		{
			if(USART_RX_BUF[0]== '#')
			{
				FHX_test.test_sel   = (USART_RX_BUF[1]&(1<<7))>>7;  //�õ�����ѡ�� 0�� �迹   1�� ����
				FHX_test.test_mode  = (USART_RX_BUF[1]&(1<<6))>>6;  //�õ���ʱģʽ�� 0�� �ֶ�   1�� �Զ�
				FHX_test.start_test = (USART_RX_BUF[1]&(1<<0));			//�õ�������� 0:  �ر�		1�� ����	
				rated_erci = (USART_RX_BUF[1]&((7<<3)))>>3;  
				//�õ������
				if(!FHX_test.test_sel)   //�迹
				{
					On_C_Mod_Sel();      //�̵����е��迹��·
					switch(rated_erci)
					{
						case 0x0:	FHX_test.Rated_sec = 5.0;  	break;
						case 0x1: FHX_test.Rated_sec = 2.0;		break;
						case 0x2: FHX_test.Rated_sec = 1.0;		break;
						default : break;
					}
					ZK_Struct.EC_dl = FHX_test.Rated_sec;
				}
				else									//����
				{
					Off_C_Mod_Sel();		//�̵����е����ɻ�·
					switch(rated_erci)		
					{
						case 0:	FHX_test.Rated_sec = 220.0;	FHX_test.jdq_flag = 1;	break;
						case 1: FHX_test.Rated_sec = 150.0;	FHX_test.jdq_flag = 1;	break;
						case 2: FHX_test.Rated_sec = 100.0;	FHX_test.jdq_flag = 1;	break;
						case 3: FHX_test.Rated_sec = 57.7350;	FHX_test.jdq_flag = 2;	break;
						case 4: FHX_test.Rated_sec = 33.3333;	FHX_test.jdq_flag = 3;	break;
						default : break;
					}
					DN_Struct.EC_dy = FHX_test.Rated_sec;
				}
				//�õ���������
				FHX_test.Res_range = (USART_RX_BUF[1]& (3<<1))>>1;
				//�洢���ڽ��յ��ٷֱ�����
				for(i=0;i<=3;i++)										
				{
					gRx_bfb[i] = USART_RX_BUF[2+i];     
					gRx_va[i] = USART_RX_BUF[6+i];
				}
				//�õ����Ե�Ͳ��Է���ֵ
				FHX_test.test_dot = array_to_int(gRx_bfb); 
				FHX_test.VA_val = array_to_int(gRx_va); 
			}
			USART_RX_STA=0; 
		}
		vTaskDelay(500);
	}
}
//��������
void Test_Task(void const *argument)
{
	enum state TestDotAdjResult = Normal;
	u8 IsBfbArrive =0;			//��¼�ٷֱ��Ƿ����趨ֵ
	u8 per_flag = 0;     	 	// ��� ����ģʽ��һ��״̬����ʼ��0���迹����ģʽ
	u8 per_jflag = 0;    		//  ��ŵ�ǰ��ѹ�����̵�����λ ״̬
	float pertest_dot = 0;	//��¼�ϴεĲ��Ե�
	u8 DoJDQSwitch = 0;
	u8 num = 0;
	
	while(1)
	{
		if(FHX_test.start_test)		//�Ƿ�������
		{
			if(FHX_test.test_mode == 0)	//�ֶ�ģʽ
			{
				if(FHX_test.test_sel)  //ֻ���ڵ��ɲ���ģʽ�²Ž��иò���
				{
					/**************���������л�******************/
					if(DN_Struct.liangch_flag != FHX_test.Res_range)  //�жϵ�ǰ�������������Ƿ��趨�ĵ�λ��
					{
						switch(FHX_test.Res_range)
						{
							case 0 : 	DN_Struct.liangch_flag=0;  break;         //�Զ�����	
							
							case 1 : 	DN_Struct.liangch_flag = 1;
												On_C_Sampl_Res1();  //����
												vTaskDelay(500);
												Off_C_Sampl_Res2();  //�Ͽ�
												Off_C_Sampl_Res3();   //�Ͽ�
												DN_Struct.liangch_zdlc = 1;
												break;
							
							case 2 : 	DN_Struct.liangch_flag = 2;
												On_C_Sampl_Res2();  //����
												vTaskDelay(500);
												Off_C_Sampl_Res1();  //�Ͽ�
												Off_C_Sampl_Res3();   //�Ͽ�
												DN_Struct.liangch_zdlc = 2;		
												break;
							
							case 3 : 	DN_Struct.liangch_flag=3;
												On_C_Sampl_Res3();  //����
												vTaskDelay(500);
												Off_C_Sampl_Res1();  //�Ͽ�
												Off_C_Sampl_Res2();   //�Ͽ�
												DN_Struct.liangch_zdlc=3;	
												break;
												
							default : break;
						}
					}
					/*************��ѹ��������ͷѡ��************/
					if(per_jflag != FHX_test.jdq_flag)
					{
						taskENTER_CRITICAL();
						Out_Bfb_To(0);
						taskEXIT_CRITICAL();
						while(FZX_Test(FHX_test.test_sel)!=TURE);
						if(FZ_bfb_gt<0.3)
						{
							switch(FHX_test.jdq_flag)
							{
								case 1:	On_JDQ_200V();	vTaskDelay(1000);	per_jflag = 1;break;   	
								case 2:	On_JDQ_100V();	vTaskDelay(1000);	per_jflag = 2;  break;		
								case 3:	On_JDQ_50V();		vTaskDelay(1000);	per_jflag = 3;	break;		
								case 4:	On_JDQ_20V();	 	vTaskDelay(1000);	per_jflag = 4;	break;		
								case 5:	On_JDQ_10V(); 	vTaskDelay(1000);	per_jflag	= 5;	break;		
								case 6: On_JDQ_5V();   	vTaskDelay(1000);	per_jflag	= 6;	break; 		
								case 7: On_JDQ_1V();   	vTaskDelay(1000);	per_jflag = 7;	break;		
								default: break;
							}
						}
					}
				}
				else 	//�迹ģʽ
				{
						/*************��ѹ��������ͷѡ��************/
					FHX_test.dy_test = FHX_test.VA_val/FHX_test.Rated_sec*FHX_test.test_dot/100;
					if(FHX_test.dy_test<=0.42f)
					{
						FHX_test.jdq_flag = 7;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_1V();
							vTaskDelay(1000);
							per_jflag = 7;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}
					}
					else if(FHX_test.dy_test<=2.2f)
					{
						FHX_test.jdq_flag = 6;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_5V();
							vTaskDelay(1000);
							per_jflag = 6;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}	
					}
					else if(FHX_test.dy_test<=5.2f)
					{
						FHX_test.jdq_flag = 5;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_10V();
							vTaskDelay(1000);
							per_jflag = 5;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}				
					}
					else if(FHX_test.dy_test<=10.2f)
					{
						FHX_test.jdq_flag = 4;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_20V();
							vTaskDelay(1000);
							per_jflag = 4;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}			
					}
					else if(FHX_test.dy_test<=30.2f)
					{
						FHX_test.jdq_flag = 3;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_50V();
							vTaskDelay(1000);
							per_jflag = 3;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}			
					}
					else if(FHX_test.dy_test<=80.2f)
					{
						FHX_test.jdq_flag = 2;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_100V();
							vTaskDelay(1000);
							per_jflag = 2;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}					
					}
					else 
					{
						FHX_test.jdq_flag = 1;
						if(per_jflag != FHX_test.jdq_flag)
						{
							taskENTER_CRITICAL();
							Out_Bfb_To(0);
							taskEXIT_CRITICAL();
							while(num<3)
							{
									FZX_Test(FHX_test.test_sel);
									if(NowBfb<0.3f)
										num++;
							}
							num = 0;
							On_JDQ_200V();
							vTaskDelay(1000);
							per_jflag = 1;
							IsTestuppre = 0;
							DoJDQSwitch = 1;
						}
					}
			/*********************end(��ͷѡ��)**************************/
				}
			/**********�жϵ�ǰ����ģʽ�Ƿ��趨ģʽ**********************/
				if(FHX_test.test_sel != per_flag)		
				{
					per_flag = FHX_test.test_sel;
							
					FZX_Test_Init(FHX_test.test_sel);	
				}
				/**************�жϵ�ǰ���Ե��Ƿ����趨ֵ***************/
				if((IsBfbArrive == 0) || (pertest_dot != FHX_test.test_dot))
				{
					if((DoJDQSwitch == 1)||(IsTestuppre == 0))	//�̵����Ƿ������л� or  ֮ǰû��Ԥ����
					{
						Out_Bfb_To(10);
						while(FZX_Test(FHX_test.test_sel)!=TURE);
						while(FZX_Test(FHX_test.test_sel)!=TURE);
						uppre_val = FZ_bfb_gt;		//�õ�10%ʵ�ʰٷֱ�
						IsTestuppre = 1;		
						DoJDQSwitch = 0;		//���״̬
					}
					TestDotAdjResult = Out_Bfb_Adj(FHX_test.test_dot,FHX_test.test_sel,ACCURATE);
					if(TestDotAdjResult==Normal)	
						IsBfbArrive =1;
					else	
						IsBfbArrive = 0;	
					pertest_dot = FHX_test.test_dot;
				}
				/***********************����******************************/
				if(FZX_Test(FHX_test.test_sel)== TURE)
				{
					printf("bfb=%.4f    bc=%.4f     jc=%.4f\r\n",FZ_bfb_gt,FZ_x_gt,FZ_y_gt);
				}
			}
		}
		vTaskDelay(100);
	}
}
//LED��˸����
void LedFlash_Task(void const *argument)
{
	while(1)
	{
		LED0 = ~LED0;
		vTaskDelay(1000);
	}
}
//ִ�и�λ����
void DoReset_Task(void const *argument)
{
	BaseType_t	err = pdFAIL;
	
	while(1)
	{
		if(ResetSyncSemphr != NULL)
		{
			err = xSemaphoreTake(ResetSyncSemphr,0);   //����
			if(err == pdTRUE)
			{
				vTaskSuspendAll();  
				AD7545_RESULT() = 0;
				xTaskResumeAll();
				gSystemReset = 1;  //ϵͳ������λ����
//			xSemaphoreGive(DoResetCmdSemphr);
				FHX_test.test_dot = 0;
//				Current_TestDot = 0;
				memset(USART_RX_BUF,0,sizeof(USART_RX_BUF));
			}
		}
		vTaskDelay(500);
	}
}
/***********************************************************
��������:   array_to_int(unsigned char *array)	
��������:   ����ת����
��ڲ���:   *array
���ڲ���:   fz
***********************************************************/
float	array_to_int(unsigned char *array)		 
{																								
		char i;
		float fz=0;
		for(i=0;i<=3;i++)
		{
				fz=fz*10+(*(array+i)-'0');
		}
		return(fz/10);
}

/***********************************************************
��������:  void BC_to_chara(float data)	
��������:  ���Ȳ�����ת�����ַ���
��ڲ���:   date
���ڲ���:   
***********************************************************/
/*
void BC_to_chara(float data)
{
		float d;
	
		if(fabs(data)>=10000)
		{
				d=fabs(data);
			
				temp[1]=(int)d/10000+'0';
				temp[2]=(int)d%10000/1000+'0';
				temp[3]=(int)d%1000/100+'0';
				temp[4]=(int)d%100/10+'0';
				temp[5]=(int)d%10+'0';
				temp[6]='.';
		}
		else if((fabs(data)>=1000)&&(fabs(data)<10000))
		{
				d=fabs(data)*10;
			
				temp[1]=(int)d/10000+'0';
				temp[2]=(int)d%10000/1000+'0';
				temp[3]=(int)d%1000/100+'0';
				temp[4]=(int)d%100/10+'0';
				temp[5]='.';
				temp[6]=(int)d%10+'0';
				
		}
		else if((fabs(data)>=100)&&(fabs(data)<1000))
		{
				d=fabs(data)*100;
			
				temp[1]=(int)d/10000+'0';
				temp[2]=(int)d%10000/1000+'0';
				temp[3]=(int)d%1000/100+'0';
				temp[4]='.';
				temp[5]=(int)d%100/10+'0';
				temp[6]=(int)d%10+'0';
		}
		else if((fabs(data)>=10)&&(fabs(data)<100))
		{
				d=fabs(data)*1000;
			
				temp[1]=(int)d/10000+'0';
				temp[2]=(int)d%10000/1000+'0';
				temp[3]='.';
				temp[4]=(int)d%1000/100+'0';
				temp[5]=(int)d%100/10+'0';
				temp[6]=(int)d%10+'0';
		}
		else if((fabs(data)>=1)&&(fabs(data)<10))                                           
		{
				d=fabs(data)*10000;			
				
				temp[1]=(int)d/10000+'0';
				temp[2]='.';
				temp[3]=(int)d%10000/1000+'0';
				temp[4]=(int)d%1000/100+'0';
				temp[5]=(int)d%100/10+'0';
				temp[6]=(int)d%10+'0';
		}
		else if((fabs(data)>=0.1)&&((fabs(data)<1)))                                           //((fabs(data)>=0)&&(fabs(data)<1)) 
		{
				d=fabs(data)*10000;
			
				temp[1]='0';
				temp[2]='.';
				temp[3]=(int)d/1000+'0';
				temp[4]=(int)d%1000/100+'0';
				temp[5]=(int)d%100/10+'0';
				temp[6]=(int)d%10+'0';
		}
		else if((fabs(data)>=0.01)&&((fabs(data)<0.1)))
		{
				d=fabs(data)*10000;
			
				temp[1]='0';
				temp[2]='.';
				temp[3]='0';
				temp[4]=(int)d/100+'0';
				temp[5]=(int)d%100/10+'0';
				temp[6]=(int)d%10+'0';
		}
		else if((fabs(data)>=0.001)&&((fabs(data)<0.01)))
		{
				d=fabs(data)*10000;
			
				temp[1]='0';
				temp[2]='.';
				temp[3]='0';
				temp[4]='0';
				temp[5]=(int)d/10+'0';
				temp[6]=(int)d%10+'0';
		}
		else if((fabs(data)>=0.0001)&&((fabs(data)<0.001)))
		{
				d=fabs(data)*10000;
			
				temp[1]='0';
				temp[2]='.';
				temp[3]='0';
				temp[4]='0';
				temp[5]='0';
				temp[6]=(int)d%10+'0';
		}
		else
		{
				temp[1]='0';
				temp[2]='.';
				temp[3]='0';
				temp[4]='0';
				temp[5]='0';
				temp[6]='0';
		}
		if(data>=0)
			temp[0]='+';
		else
			temp[0]='-';

}
*/
