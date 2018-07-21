//////////////////////////////////////////////////////////////////////////////////	 
//项目名称: 新负荷箱测试仪
//作者    : 黄志宝
//修改日期: 2018-7-21	
//修改内容: 增加说明文档
//版本    : 1.1.0
//////////////////////////////////////////////////////////////////////////////////  
#include "./includes.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
/********************任务相关定义和申明******************/
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
extern u8 read_flag;    //ADS1274可读标志位
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
/**********测试设置参数定义*************/
typedef struct
{
	u8 test_sel;					//测试选择： 0： 阻抗  1： 导纳
	u8 test_mode;					//测试模式   0： 自动  1： 手动
	float Rated_sec;			//额定二次: 阻抗模式下： 5A,2A,1A  导纳测试模式下： 100V，100/3, 100/根3
	u8 Res_range;					//采样电阻挡位： 导纳测试模式下有效， 0: 自动量程   1: 20R  2: 2R  3: 0.2R
	float test_dot;				//测试点
	u8 start_test;				//启动测试   0:  关闭	 1: 开启
	u8 jdq_flag;					//升压升流器车头 标志位，1：200V  2: 100V  3： 50V ......
	float VA_val;					//测试的伏安数
	float dy_test;				//测定点的实际电压
}test;

test FHX_test;
/**************************************/


u8 gSystemReset =0;   //系统复位标志位
//float Current_TestDot =0;  //记录当前测试点
float uppre_val =10.0;
u8 IsTestuppre = 0;

float	array_to_int(unsigned char *array);


int main(void)
{ 
	
#if RTC_EN > 0	
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
#endif	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
//	delay_init(168);		  //初始化延时函数	
	uart_init(115200);	//串口初始化波特率为115200

	ad7545_FsmcConfig();  //初始化 AD7545配置
	
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
	FHX_test.start_test = 0;   //初始化0，关闭测试
	
	FZX_Test_Init(0);						//默认阻抗模式，阻抗测试初始化
	//创建开始任务
	xTaskCreate((TaskFunction_t)Start_Task, "start task",START_STK_SIZE, NULL, START_TASK_PRIO, &StartTaskHandle);
	//开启任务调度
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
//开始任务
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
//串口接收指令
void Uart1_Task(void const *argument)
{
	u8 i=0;
	u8 rated_erci = 0;
	
	u8 gRx_bfb[4] = {'0','0','0','0'};  //串口接收到 百分表数据 暂存
	u8 gRx_va[4]  = {'0','0','0','0'};	 //串口接收到 伏安数据   暂存
	
	while(1)
	{
		if(USART_RX_STA&0x8000)
		{
			if(USART_RX_BUF[0]== '#')
			{
				FHX_test.test_sel   = (USART_RX_BUF[1]&(1<<7))>>7;  //得到测试选择： 0： 阻抗   1： 导纳
				FHX_test.test_mode  = (USART_RX_BUF[1]&(1<<6))>>6;  //得到测时模式： 0： 手动   1： 自动
				FHX_test.start_test = (USART_RX_BUF[1]&(1<<0));			//得到启动命令： 0:  关闭		1： 开启	
				rated_erci = (USART_RX_BUF[1]&((7<<3)))>>3;  
				//得到额定二次
				if(!FHX_test.test_sel)   //阻抗
				{
					On_C_Mod_Sel();      //继电器切到阻抗回路
					switch(rated_erci)
					{
						case 0x0:	FHX_test.Rated_sec = 5.0;  	break;
						case 0x1: FHX_test.Rated_sec = 2.0;		break;
						case 0x2: FHX_test.Rated_sec = 1.0;		break;
						default : break;
					}
					ZK_Struct.EC_dl = FHX_test.Rated_sec;
				}
				else									//导纳
				{
					Off_C_Mod_Sel();		//继电器切到导纳回路
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
				//得到采样量程
				FHX_test.Res_range = (USART_RX_BUF[1]& (3<<1))>>1;
				//存储串口接收到百分表数据
				for(i=0;i<=3;i++)										
				{
					gRx_bfb[i] = USART_RX_BUF[2+i];     
					gRx_va[i] = USART_RX_BUF[6+i];
				}
				//得到测试点和测试伏安值
				FHX_test.test_dot = array_to_int(gRx_bfb); 
				FHX_test.VA_val = array_to_int(gRx_va); 
			}
			USART_RX_STA=0; 
		}
		vTaskDelay(500);
	}
}
//测试任务
void Test_Task(void const *argument)
{
	enum state TestDotAdjResult = Normal;
	u8 IsBfbArrive =0;			//记录百分表是否在设定值
	u8 per_flag = 0;     	 	// 存放 测试模式上一次状态，初始化0：阻抗测试模式
	u8 per_jflag = 0;    		//  存放当前升压升流继电器挡位 状态
	float pertest_dot = 0;	//记录上次的测试点
	u8 DoJDQSwitch = 0;
	u8 num = 0;
	
	while(1)
	{
		if(FHX_test.start_test)		//是否开启测试
		{
			if(FHX_test.test_mode == 0)	//手动模式
			{
				if(FHX_test.test_sel)  //只有在导纳测试模式下才进行该操作
				{
					/**************采样电阻切换******************/
					if(DN_Struct.liangch_flag != FHX_test.Res_range)  //判断当前采样电阻量程是否设定的挡位上
					{
						switch(FHX_test.Res_range)
						{
							case 0 : 	DN_Struct.liangch_flag=0;  break;         //自动量程	
							
							case 1 : 	DN_Struct.liangch_flag = 1;
												On_C_Sampl_Res1();  //吸合
												vTaskDelay(500);
												Off_C_Sampl_Res2();  //断开
												Off_C_Sampl_Res3();   //断开
												DN_Struct.liangch_zdlc = 1;
												break;
							
							case 2 : 	DN_Struct.liangch_flag = 2;
												On_C_Sampl_Res2();  //吸合
												vTaskDelay(500);
												Off_C_Sampl_Res1();  //断开
												Off_C_Sampl_Res3();   //断开
												DN_Struct.liangch_zdlc = 2;		
												break;
							
							case 3 : 	DN_Struct.liangch_flag=3;
												On_C_Sampl_Res3();  //吸合
												vTaskDelay(500);
												Off_C_Sampl_Res1();  //断开
												Off_C_Sampl_Res2();   //断开
												DN_Struct.liangch_zdlc=3;	
												break;
												
							default : break;
						}
					}
					/*************升压升流器车头选择************/
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
				else 	//阻抗模式
				{
						/*************升压升流器车头选择************/
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
			/*********************end(车头选择)**************************/
				}
			/**********判断当前测试模式是否设定模式**********************/
				if(FHX_test.test_sel != per_flag)		
				{
					per_flag = FHX_test.test_sel;
							
					FZX_Test_Init(FHX_test.test_sel);	
				}
				/**************判断当前测试点是否处于设定值***************/
				if((IsBfbArrive == 0) || (pertest_dot != FHX_test.test_dot))
				{
					if((DoJDQSwitch == 1)||(IsTestuppre == 0))	//继电器是否发生过切换 or  之前没有预升过
					{
						Out_Bfb_To(10);
						while(FZX_Test(FHX_test.test_sel)!=TURE);
						while(FZX_Test(FHX_test.test_sel)!=TURE);
						uppre_val = FZ_bfb_gt;		//得到10%实际百分表
						IsTestuppre = 1;		
						DoJDQSwitch = 0;		//清除状态
					}
					TestDotAdjResult = Out_Bfb_Adj(FHX_test.test_dot,FHX_test.test_sel,ACCURATE);
					if(TestDotAdjResult==Normal)	
						IsBfbArrive =1;
					else	
						IsBfbArrive = 0;	
					pertest_dot = FHX_test.test_dot;
				}
				/***********************测试******************************/
				if(FZX_Test(FHX_test.test_sel)== TURE)
				{
					printf("bfb=%.4f    bc=%.4f     jc=%.4f\r\n",FZ_bfb_gt,FZ_x_gt,FZ_y_gt);
				}
			}
		}
		vTaskDelay(100);
	}
}
//LED闪烁任务
void LedFlash_Task(void const *argument)
{
	while(1)
	{
		LED0 = ~LED0;
		vTaskDelay(1000);
	}
}
//执行复位任务
void DoReset_Task(void const *argument)
{
	BaseType_t	err = pdFAIL;
	
	while(1)
	{
		if(ResetSyncSemphr != NULL)
		{
			err = xSemaphoreTake(ResetSyncSemphr,0);   //阻塞
			if(err == pdTRUE)
			{
				vTaskSuspendAll();  
				AD7545_RESULT() = 0;
				xTaskResumeAll();
				gSystemReset = 1;  //系统发生复位操作
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
函数名称:   array_to_int(unsigned char *array)	
函数功能:   数组转浮点
入口参数:   *array
出口参数:   fz
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
函数名称:  void BC_to_chara(float data)	
函数功能:  将比差数据转化成字符串
入口参数:   date
出口参数:   
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
