#include "test.h"
#include "ads1274.h"
#include "delay.h"
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"

extern u32 AD_val[ReadAdcChx];   //存储读取AD1274的4通道的AD码值

const double Fy_table[2]={1,Fy_ratio};


const double Table_Biao_JXS[4]={	//Biao_204角差系数
0.0, 			//1 
0.0, 			//10
0.0,			//100  
0.0,			//1000
};
const double Table_Cha_JXS[4]={	//Cha_204角差系数
0.0, 			//1 
0.0, 			//10
0.0,			//100  
0.0,			//1000 
};


const float XS_K[4]={	
	(22),               //不用
	(22.0*1.00000),		  //22  欧姆采样电阻(0.1量程)
	(2.2*1.00000), 		  //2.2   欧姆采样电阻(1量程)
	(0.2*1.00000), 		  //0.2 欧姆采样电阻(10量程)
	};


FFT FFT_Struct;
DN 	DN_Struct;
ZK  ZK_Struct;

u8 read_flag=0;    //ADS1274可读标志位

//测试模式选择
volatile u8 Mod_Sel=0;  //0：阻抗测试  1：导纳测试

//ADS1274通道采样的电压
volatile double U_Biao=0.0;		
volatile double U_Cha=0.0;

//误差相关变量定义
static volatile double WC_Peak=0.0; //综合误差
volatile double U_Iu=0.0;					//标准通道上源信号
volatile double U_i=0.0;					//差值通道上源信号

//放大倍数标志位 :  0 ->1倍  1->10倍  2->100倍   3->1000倍
volatile u8 CK204_Biao=0; 
volatile u8 CK204_Cha=0;  
const float Table_Biao[4]={1.0,10,100,1000};
const float Table_Cha[4]= {1.0,10,100,1000};

//测试中间数据暂存
volatile double fz_Tab[8]= {0.0};
volatile double fz_x_Tab[8] = {0.0};
volatile double fz_y_Tab[8] = {0.0};
volatile double fz_bfb_Tab[8] = {0.0};
volatile double SZGL_Tab[8] = {0.0};
volatile double glys_Tab[8] = {0.0};

//最终存放测试结果数据
volatile double FZ_gt=0.0;    //						
volatile double FZ_x_gt=0.0;		//             				
volatile double FZ_y_gt=0.0;		//
volatile double FZ_bfb_gt =0.0;   //
volatile double SZGL_gt = 0.0;			//
volatile double GLYS_gt = 0.0;		//

/*
*********************************************************************************************************
*	函 数 名 void Test_Param_Init(void)
*	功能说明 : 测试所有相关参数初始化
*	形    参 : 无
*	返 回 值 : 无
*********************************************************************************************************
*/
void Test_Param_Init(void)
{
	u32 i;
	
	//FFT_Struct Param  Init
	for(i=0;i<Test_Per;i++)
	{
		FFT_Struct.a1[i] = 0.0;
		FFT_Struct.a2[i] = 0.0;
		FFT_Struct.b1[i] = 0.0;
		FFT_Struct.b2[i] = 0.0;
		FFT_Struct.Avg_Biao[i] = 0.0;
		FFT_Struct.Avg_Cha[i] = 0.0;
		FFT_Struct.Arctan_Biao[i] = 0.0;
		FFT_Struct.Arctan_Cha[i] = 0.0;
		FFT_Struct.Arctan_Cha_Biao[i] = 0.0;
		FFT_Struct.Cos_jiao[i] = 0.0;
		FFT_Struct.Sin_jiao[i] = 0.0;	
	}
	for(i=0;i<Test_Per*Dot_P_Cyc+1;i++)
	{
		FFT_Struct.Sum_Biao[i] = 0;
		FFT_Struct.Sum_Cha[i] = 0;
	}
	FFT_Struct.Cos_jiao_zhi = 0.0;
	FFT_Struct.Sin_jiao_zhi =0.0;
	
	//DN_Struct Param  Init
	DN_Struct.liangch_flag = 3;   //量程标记(0,1,2,3)
	DN_Struct.liangch_zdlc = 3;   //自动量程档位标志(1,2,3(10量程))
	DN_Struct.EC_dy = 100.0;
	DN_Struct.DN = 0.0;
	DN_Struct.DN_bfb = 0.0;
	DN_Struct.DN_x = 0.0;
	DN_Struct.DN_y = 0.0;
	DN_Struct.Div_flag = 1;
	DN_Struct.UzLimit = 3.9;
	DN_Struct.SZGL = 0.0;
	DN_Struct.GLYS = 0.0;
	
	//ZK_Struct Param  Init
	ZK_Struct.EC_dl = 5;
	ZK_Struct.Div_flag = 1;
	ZK_Struct.ZK = 0.0;
	ZK_Struct.ZK_bfb = 0.0;
	ZK_Struct.ZK_x = 0.0;
	ZK_Struct.ZK_y = 0.0;
	ZK_Struct.UzLimit = 3.9;
}
/*
*********************************************************************************************************
*	函 数 名 static double SZLB(u8 n,volatile double *DATA)
*	功能说明 : 数字滤波
*	形    参 : n : 周期数   DATA： 预处理的数据
*	返 回 值 : Result
*********************************************************************************************************
*/
static double SZLB(u8 n,volatile double *DATA)
{
		u16 i;
		volatile double Max_Dat;
		volatile double Min_Dat;
		volatile double Result;
	
		Max_Dat=Min_Dat=Result=*DATA;
	
		for(i=1;i<n;i++)
		{
				Result+=(*(DATA+i));
				if(*(DATA+i)<Min_Dat)
				{
						Min_Dat=*(DATA+i);
				}
				if(*(DATA+i)>Max_Dat)
				{
						Max_Dat=*(DATA+i);
				}
		}
		if(n<=3)	Result=(Result/n);  //如果n<=3,则直接取平均值
		else 			Result=(Result-Max_Dat-Min_Dat)/(n-2);
		return(Result);
}
/*
*********************************************************************************************************
*	函 数 名 ：void ZK_Test_Init(void)
*	功能说明 : 阻抗测试初始化
*	形    参 : 无
*	返 回 值 : 无
*********************************************************************************************************
*/
/*
static void ZK_Test_Init(void)
{
		CH1_BIAO();           //SVin1+
		CH3_CHA();						//CVin3+  不分压
		CK204_Biao_1();				//程控
		CK204_Cha_1();
}
*/
/*
*********************************************************************************************************
*	函 数 名 ：void DN_Test_Init(void)
*	功能说明 : 导纳测试初始化
*	形    参 : 无
*	返 回 值 : 无
*********************************************************************************************************
*/
/*
static void DN_Test_Init(void)
{
		CH3_BIAO();           //SVin3+	不分压
		CH1_CHA();					  //CVin1+
		CK204_Biao_1();				//程控
		CK204_Cha_1();
}
*/
/*
*********************************************************************************************************
*	函 数 名 ：void DN_Test_Init(void)
*	功能说明 : 导纳测试初始化
*	形    参 : 无
*	返 回 值 : 无
*********************************************************************************************************
*/
void FZX_Test_Init(u8 mode_flag)
{
	if(mode_flag)   //导纳测试初始化
	{
//	CH3_BIAO();           //SVin3+	不分压
		CH2_BIAO();						//SVin2+    分压
		CH1_CHA();					  //CVin1+
		DN_Struct.Div_flag = 1;
		CK204_Biao_1();				//程控
		CK204_Cha_1();
	}
	else					//阻抗测试初始化
	{
		CH1_BIAO();           //SVin1+
//	CH3_CHA();						//CVin3+  不分压
		CH2_CHA();						//CVin2+    分压
		ZK_Struct.Div_flag = 1;
		CK204_Biao_1();				//程控
		CK204_Cha_1();
	}
}
/*
*********************************************************************************************************
*	函 数 名 	 static enum result AC_Test(void)
*	功能说明 : 交流FFT采样
*	形    参 : 无
*	返 回 值 : Test_result ： FALSE：采样失败  TURE: 采样成功
*********************************************************************************************************
*/
static enum result AC_Test(void)
{
	enum result Test_result=FALSE;
	u16 i=0,j=0;
	u32 m=0;
	
	m=0;
	
	SYNC =1;
	vTaskSuspendAll();				//任务调度器上锁
	do
	{
		if(read_flag==1)
		{
			ADS1274_ReadNowAdc();
			FFT_Struct.Sum_Biao[m]=AD_val[0];  //标准通道
			FFT_Struct.Sum_Cha[m] =AD_val[1];  //差值通道
			read_flag = 0;
			m++;
		}
	}while(m<N);
	xTaskResumeAll();				//解锁
	SYNC =0;
	
	Test_result = TURE;				//测试完成
	m = 0;
	
	for(i=0;i<Test_Per;i++)
	{
		FFT_Struct.a1[i]=0;
		FFT_Struct.b1[i]=0;
		FFT_Struct.a2[i]=0;
		FFT_Struct.b2[i]=0;
	}
	
	for (j=0;j<Test_Per;j++) 
	{
		for(i=(j*Dot_P_Cyc+1);i<((j+1)*Dot_P_Cyc);i++)
		{
			FFT_Struct.a1[j]+=(((double)FFT_Struct.Sum_Biao[(i/Dot_P_Cyc)*Dot_P_Cyc]/(Dot_P_Cyc-1)+2*(double)FFT_Struct.Sum_Biao[i]*cos(2*Pi*i/Dot_P_Cyc)+(double)FFT_Struct.Sum_Biao[(i/Dot_P_Cyc+1)*Dot_P_Cyc]/(Dot_P_Cyc-1))/Dot_P_Cyc);
			FFT_Struct.b1[j]+=((2*(double)FFT_Struct.Sum_Biao[i]*sin(2*Pi*i/Dot_P_Cyc))/Dot_P_Cyc);  
					
			FFT_Struct.a2[j]+=(((double)FFT_Struct.Sum_Cha[(i/Dot_P_Cyc)*Dot_P_Cyc]/(Dot_P_Cyc-1)+2*(double)FFT_Struct.Sum_Cha[i]*cos(2*Pi*i/Dot_P_Cyc)+(double)FFT_Struct.Sum_Cha[(i/Dot_P_Cyc+1)*Dot_P_Cyc]/(Dot_P_Cyc-1))/Dot_P_Cyc);
			FFT_Struct.b2[j]+=((2*(double)FFT_Struct.Sum_Cha[i]*sin(2*Pi*i/Dot_P_Cyc))/Dot_P_Cyc);  
		 }
	}	
	for(i=0;i<Test_Per;i++)
	{
		FFT_Struct.Avg_Biao[i] = sqrt(FFT_Struct.a1[i]*FFT_Struct.a1[i]+FFT_Struct.b1[i]*FFT_Struct.b1[i]);
		FFT_Struct.Avg_Biao[i] = ((2*ADC_Vref)*(FFT_Struct.Avg_Biao[i]/16777215.0))/1.414213562;
		
		FFT_Struct.Avg_Cha[i] = sqrt(FFT_Struct.a2[i]*FFT_Struct.a2[i]+FFT_Struct.b2[i]*FFT_Struct.b2[i]);
		FFT_Struct.Avg_Cha[i] = ((2*ADC_Vref)*(FFT_Struct.Avg_Cha[i]/16777215.0))/1.414213562;
		
		/***************标准通道信号相位处理**************************************/
		if((FFT_Struct.a1[i]>0)&&(-FFT_Struct.b1[i]>0))
		{
				FFT_Struct.Arctan_Biao[i]=(atan(-FFT_Struct.b1[i]/FFT_Struct.a1[i]));
		}		      
		else if((FFT_Struct.a1[i]<0)&&(-FFT_Struct.b1[i]>0))
		{
				FFT_Struct.Arctan_Biao[i]=((Pi+atan(-FFT_Struct.b1[i]/FFT_Struct.a1[i])));
		}   
		else if((FFT_Struct.a1[i]<0)&&(-FFT_Struct.b1[i]<0))
		{
				FFT_Struct.Arctan_Biao[i]=((Pi+atan(-FFT_Struct.b1[i]/FFT_Struct.a1[i])));
		} 	              
		else if((FFT_Struct.a1[i]>0)&&(-FFT_Struct.b1[i]<0))
		{
				FFT_Struct.Arctan_Biao[i]=(2*Pi+(atan(-FFT_Struct.b1[i]/FFT_Struct.a1[i])));
		}	        
		else if(FFT_Struct.b1[i]==0)
		{
				if (FFT_Struct.a1[i]>0) FFT_Struct.Arctan_Biao[i]=0;
				if (FFT_Struct.a1[i]<0) FFT_Struct.Arctan_Biao[i]=Pi;
		}          
				
		/****************差值通道信号相位处理**************************************/
		if((FFT_Struct.a2[i]>0)&&(-FFT_Struct.b2[i]>0))
		{
				FFT_Struct.Arctan_Cha[i]=(atan(-FFT_Struct.b2[i]/FFT_Struct.a2[i]));
		}  
		else if((FFT_Struct.a2[i]<0)&&(-FFT_Struct.b2[i]>0))
		{
				FFT_Struct.Arctan_Cha[i]=((Pi+atan(-FFT_Struct.b2[i]/FFT_Struct.a2[i])));
		}  
		else if((FFT_Struct.a2[i]<0)&&(-FFT_Struct.b2[i]<0))
		{
				FFT_Struct.Arctan_Cha[i]=((Pi+atan(-FFT_Struct.b2[i]/FFT_Struct.a2[i])));
		}               
		else if((FFT_Struct.a2[i]>0)&&(-FFT_Struct.b2[i]<0))
		{
				FFT_Struct.Arctan_Cha[i]=(2*Pi+(atan(-FFT_Struct.b2[i]/FFT_Struct.a2[i])));
		}  
		else if(FFT_Struct.b2[i]==0)
		{
				if (FFT_Struct.a2[i]>0) FFT_Struct.Arctan_Cha[i]=0;
				if (FFT_Struct.a2[i]<0) FFT_Struct.Arctan_Cha[i]=Pi;
		}        	             
			/**********************end****************************************************/
	}
	U_Biao=SZLB(Test_Per,FFT_Struct.Avg_Biao);  //数字滤波;
	U_Cha =SZLB(Test_Per,FFT_Struct.Avg_Cha);
		
	for(i=0;i<Test_Per;i++)
	{
		FFT_Struct.Arctan_Biao[i]-= (Table_Biao_JXS[CK204_Biao]/H_J);
		FFT_Struct.Arctan_Cha[i] -= (Table_Biao_JXS[CK204_Cha]/H_J);
		
		FFT_Struct.Arctan_Cha_Biao[i] = FFT_Struct.Arctan_Cha[i]-FFT_Struct.Arctan_Biao[i];
																												
		FFT_Struct.Cos_jiao[i] = cos(FFT_Struct.Arctan_Cha_Biao[i]);
		FFT_Struct.Sin_jiao[i] = sin(FFT_Struct.Arctan_Cha_Biao[i]);
	}	
	FFT_Struct.Cos_jiao_zhi=SZLB(Test_Per,(double*)FFT_Struct.Cos_jiao);  //数字滤波
	FFT_Struct.Sin_jiao_zhi=SZLB(Test_Per,(double*)FFT_Struct.Sin_jiao);
	
	U_Biao/=Table_Biao[CK204_Biao]; //U_Biao转化为程控之前的电压
	U_Cha/=Table_Cha[CK204_Cha];
	
	return Test_result;
}

/*
*********************************************************************************************************
*	函 数 名 ：static enum result CK_PGA(void)
*	功能说明 : 程控放大切换函数
*	形    参 : 无
*	返 回 值 : 
*********************************************************************************************************
*/
static enum result CK_PGA(void)
{
	enum result Test_result=FALSE;

	Test_result=AC_Test();

/********************标准通道程控放 ********************* */
	if(U_Biao>0.16)
	{
			CK204_Biao_1();     //放大1倍
			CK204_Biao = 0;	 
	}
	else if((U_Biao>0.016)&&(U_Biao<=0.16))
	{
			CK204_Biao_10();    //放大10倍
			CK204_Biao = 1;
	}
	else if((U_Biao>0.0016)&&(U_Biao<=0.016))
	{
			CK204_Biao_100();   //放大100倍
			CK204_Biao = 2;
	}
	else   //<=0.0016
	{
			CK204_Biao_1000();    //放大1000倍
			CK204_Biao = 3;
	}
/*************************end***************************************/	
		
/**********************差值通道程控放 **********************/
	if(U_Cha>0.16)
	{
			CK204_Cha_1();     //放大1倍
			CK204_Cha = 0;
	}
	else if((U_Cha>0.016)&&(U_Cha<=0.16))
	{
			CK204_Cha_10();     //放大10倍
			CK204_Cha = 1;
	}
	else if((U_Cha>0.0016)&&(U_Cha<=0.016))
	{
			CK204_Cha_100();     //放大100倍
			CK204_Cha = 2;
	}
	else   //<=0.0016
	{
			CK204_Cha_1000();     //放大1000倍
			CK204_Cha = 3;
	}
/****************************end************************************/
	return(Test_result);
}

/*
*********************************************************************************************************
*	函 数 名 ：static void Swith_LC(u8 lc)
*	功能说明 : 采样电阻转化函数
*	形    参 : 无
*	返 回 值 :
*********************************************************************************************************
*/
static void Swith_LC(u8 lc)
{
	static u8 lc_tem=0;
	
	lc_tem=lc;
	
	if(lc_tem==DN_Struct.liangch_zdlc)     //已经处于该量程，无需再切换
		 return;            
	
	if(lc_tem>3)       //限制
			lc_tem=3;
	if(lc_tem==1)
	{
			On_C_Sampl_Res1();  //吸合
			vTaskDelay(500);
			Off_C_Sampl_Res2();  //断开
			Off_C_Sampl_Res3();   //断开
			DN_Struct.liangch_zdlc=1;
	}
	else if(lc_tem==2)
	{
			On_C_Sampl_Res2();  //吸合
			vTaskDelay(500);
			Off_C_Sampl_Res1() ;  //断开
			Off_C_Sampl_Res3();   //断开
			DN_Struct.liangch_zdlc=2;
	}
	else   //3
	{
			On_C_Sampl_Res3();  //吸合
			vTaskDelay(500);
			Off_C_Sampl_Res1();  //断开
			Off_C_Sampl_Res2();   //断开
			DN_Struct.liangch_zdlc=3;
	}
	vTaskDelay(500);
}


/*
*********************************************************************************************************
*	函 数 名 ：static void Auto_LC_DN(void)
*	功能说明 : 导纳测试时自动量程切换函数
*	形    参 : 无
*	返 回 值 : 无
*********************************************************************************************************
*/
static void Auto_LC_DN(void)
{
	if(DN_Struct.liangch_flag!=0)  return;    //不是自动量程

	WC_Peak=U_i/U_Iu;    //导纳
	
	if(WC_Peak>3.3)
			Swith_LC(3);
	else if(WC_Peak>0.33)
			Swith_LC(2);
	else
			Swith_LC(1);
}

/*
*********************************************************************************************************
*	函 数 名 ：void vol_div(u8 flag_fy)
*	功能说明 : 阻抗导纳是否分压判断函数
*	形    参 : u8 flag_fy    0： 不分压   1：分压
*	返 回 值 : 无
*********************************************************************************************************
*/
static void vol_div(const u8 Mod_Sel,const u8 flag_fy)
{
	if(Mod_Sel == 0)											//阻抗分压
	{
		switch(flag_fy)
		{
			case 0: 
							CH3_CHA();
							ZK_Struct.Div_flag = 0;
//							delay_ms(100);
							break;
			
			case 1: 
							CH2_CHA();
							ZK_Struct.Div_flag = 1;
//							delay_ms(100);
							break;
			
			default: 
							break;	
		}
	}
	else if(Mod_Sel == 1)								//导纳分压
	{
		switch(flag_fy)
		{
			case 0: 
							CH3_BIAO();
							DN_Struct.Div_flag = 0;
//							delay_ms(100);
							break;
			
			case 1: 
							CH2_BIAO();
							DN_Struct.Div_flag = 1;
//							delay_ms(100);
							break;
			
			default: 
							break;	
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名 �  static enum result Zukang_test(void)
*	功能说明 : 阻抗测试函数
*	形    参 : 无
*	返 回 值 : 返回值Test_result 为TURE   完成一次测试（正常数据）  为FALSE ：测试有误
*********************************************************************************************************
*/
static enum result Zukang_test(void)
{
	enum result Test_result=FALSE;
	
	Test_result=CK_PGA();
	
	U_Iu = U_Biao/XS_CT0*BZ_DifRto;
	
	U_i = U_Cha*Fy_table[ZK_Struct.Div_flag]*CZ_DifRto;
	
	/*************分压通道判断************************/
	
	if(U_i>ZK_Struct.UzLimit)
	{
			if(ZK_Struct.Div_flag == 0)
					vol_div(0,1);
	}
	else 
	{
			if(ZK_Struct.Div_flag == 1)
				vol_div(0,0);
	}
	
/****************end****************************/	
	
	ZK_Struct.ZK = U_i/U_Iu;                   //阻抗
	
	ZK_Struct.ZK_x = ZK_Struct.ZK*FFT_Struct.Cos_jiao_zhi;   //阻抗同向分量： 实部电阻
	ZK_Struct.ZK_y = ZK_Struct.ZK*FFT_Struct.Sin_jiao_zhi;   //阻抗正交分量： 虚部电抗
		
	ZK_Struct.ZK_bfb=(U_Iu/ZK_Struct.EC_dl)*100;  //阻抗测试时，当前百分表
	
	return (Test_result);
}

/*
*********************************************************************************************************
*	函 数 名 ：static enum result Daona_test(void)
*	功能说明 : 阻抗测试函数
*	形    参 : 无
*	返 回 值 : 返回值Test_result 为TURE   完成一次测试（正常数据）  为FALSE ：测试有误
*********************************************************************************************************
*/
static enum result Daona_test(void)
{
	enum result Test_result=FALSE;
	volatile double CH_U_Iu=0.0;					//端子上电压量，用于判断是否分压
	
	Test_result = CK_PGA();
	
	U_Iu = U_Biao*Fy_table[DN_Struct.Div_flag]*VolIslationRatio*BZ_DifRto;    //标准通道	： 导纳电压
	CH_U_Iu = U_Iu/VolIslationRatio;
	
	U_i  = U_Cha/(XS_K[DN_Struct.liangch_zdlc])*BZ_DifRto*1000;  	//差值通道  :   导纳电流
	
	Auto_LC_DN();								//根据综合误差自动切换量程（采样电阻）
	
	/*************分压通道判断************************/
		if(CH_U_Iu>DN_Struct.UzLimit)		//U_Iu>DN_Struct.UzLimit
		{
				if(DN_Struct.Div_flag == 0)
						vol_div(1,1);
		}
		else 
		{
				if(DN_Struct.Div_flag == 1)
					vol_div(1,0);
		}
/****************end****************************/	
		
		DN_Struct.DN = U_i/U_Iu;             //导纳(ms)
		
		DN_Struct.DN_x = DN_Struct.DN*FFT_Struct.Cos_jiao_zhi;   //导纳同向分量： 电导
		DN_Struct.DN_y = DN_Struct.DN*FFT_Struct.Sin_jiao_zhi;   //导纳正交分量： 电纳
			
		DN_Struct.DN_bfb=(U_Iu/DN_Struct.EC_dy)*100;    //导纳测试时，当前百分表
		
		DN_Struct.SZGL = DN_Struct.EC_dy*DN_Struct.EC_dy*DN_Struct.DN /1000;      			//视在功率
		
		DN_Struct.GLYS = FFT_Struct.Cos_jiao_zhi;    	//功率因数
		
		return (Test_result);	
}

/*
*********************************************************************************************************
*	函 数 名 ：void FZX_Test(const u8 mode_flag)
*	功能说明 : 阻抗和导纳测试
*	形    参 : u8 mode_flag ;  0: 阻抗测试   1：导纳测试
*	返 回 值 : 无
*********************************************************************************************************
*/
enum result FZX_Test(const u8 mode_flag)
{
	enum result Test_result = FALSE;
	static u8 i=0;
	static u8 j=0;
	u8 k;

	switch(mode_flag)
	{
		case 0:
						if(Zukang_test()==TURE)    
						{				
							fz_bfb_Tab[i] = ZK_Struct.ZK_bfb;
							fz_Tab[j]= ZK_Struct.ZK;
							fz_x_Tab[j] = ZK_Struct.ZK_x;
							fz_y_Tab[j] = ZK_Struct.ZK_y;
						}
						break;
		case 1:
						if(Daona_test()==TURE)    
						{				
							fz_bfb_Tab[i] = DN_Struct.DN_bfb;
							fz_Tab[j]= DN_Struct.DN;
							fz_x_Tab[j] = DN_Struct.DN_x;
							fz_y_Tab[j] = DN_Struct.DN_y;
						}
						break;
		default:
						break;
	}
						
	i++;
	j++;
	if(i>=3)    //百分表取3次平均
	{
		FZ_bfb_gt=0;
		for(k=0;k<i;k++)
		{
				FZ_bfb_gt+=fz_bfb_Tab[k];
		}
		FZ_bfb_gt/=i;
		i=0;
	}
	if(j>=5)   //其余参数取5次平均
	{					
		FZ_gt = 0;
		FZ_x_gt = 0;
		FZ_y_gt = 0;
	
		for(k=0;k<j;k++)
		{
				FZ_gt+=fz_Tab[k];
				FZ_x_gt+=fz_x_Tab[k];
				FZ_y_gt+=fz_y_Tab[k];
		}											
		FZ_gt/=j;
		FZ_x_gt/=j;
		FZ_y_gt/=j;			
		
		j=0;
		
		Test_result = TURE;
	}
	
	return (Test_result);
}

/*
*********************************************************************************************************
*	函 数 名 ：void DC_Test(void)
*	功能说明 : 直流信号测试 (调试测试用)
*	形    参 : 
*	返 回 值 : 
* 使用说明 ： 将测试直流电压接入SVin3+  和 CVin3+ 通道上
*********************************************************************************************************
*/
void DC_test(void)
{
	CH3_BIAO();   //SVin3+  SVin3-
	CH3_CHA();    //CVin3+  CVin3-
	
	CK204_Biao_1();
	CK204_Cha_1();
}
