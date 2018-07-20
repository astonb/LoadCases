#include "test.h"
#include "ads1274.h"
#include "delay.h"
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"

extern u32 AD_val[ReadAdcChx];   //´æ´¢¶ÁÈ¡AD1274µÄ4Í¨µÀµÄADÂëÖµ

const double Fy_table[2]={1,Fy_ratio};


const double Table_Biao_JXS[4]={	//Biao_204½Ç²îÏµÊý
0.0, 			//1 
0.0, 			//10
0.0,			//100  
0.0,			//1000
};
const double Table_Cha_JXS[4]={	//Cha_204½Ç²îÏµÊý
0.0, 			//1 
0.0, 			//10
0.0,			//100  
0.0,			//1000 
};


const float XS_K[4]={	
	(22),               //²»ÓÃ
	(22.0*1.00000),		  //22  Å·Ä·²ÉÑùµç×è(0.1Á¿³Ì)
	(2.2*1.00000), 		  //2.2   Å·Ä·²ÉÑùµç×è(1Á¿³Ì)
	(0.2*1.00000), 		  //0.2 Å·Ä·²ÉÑùµç×è(10Á¿³Ì)
	};


FFT FFT_Struct;
DN 	DN_Struct;
ZK  ZK_Struct;

u8 read_flag=0;    //ADS1274¿É¶Á±êÖ¾Î»

//²âÊÔÄ£Ê½Ñ¡Ôñ
volatile u8 Mod_Sel=0;  //0£º×è¿¹²âÊÔ  1£ºµ¼ÄÉ²âÊÔ

//ADS1274Í¨µÀ²ÉÑùµÄµçÑ¹
volatile double U_Biao=0.0;		
volatile double U_Cha=0.0;

//Îó²îÏà¹Ø±äÁ¿¶¨Òå
static volatile double WC_Peak=0.0; //×ÛºÏÎó²î
volatile double U_Iu=0.0;					//±ê×¼Í¨µÀÉÏÔ´ÐÅºÅ
volatile double U_i=0.0;					//²îÖµÍ¨µÀÉÏÔ´ÐÅºÅ

//·Å´ó±¶Êý±êÖ¾Î» :  0 ->1±¶  1->10±¶  2->100±¶   3->1000±¶
volatile u8 CK204_Biao=0; 
volatile u8 CK204_Cha=0;  
const float Table_Biao[4]={1.0,10,100,1000};
const float Table_Cha[4]= {1.0,10,100,1000};

//²âÊÔÖÐ¼äÊý¾ÝÔÝ´æ
volatile double fz_Tab[8]= {0.0};
volatile double fz_x_Tab[8] = {0.0};
volatile double fz_y_Tab[8] = {0.0};
volatile double fz_bfb_Tab[8] = {0.0};
volatile double SZGL_Tab[8] = {0.0};
volatile double glys_Tab[8] = {0.0};

//×îÖÕ´æ·Å²âÊÔ½á¹ûÊý¾Ý
volatile double FZ_gt=0.0;    //						
volatile double FZ_x_gt=0.0;		//             				
volatile double FZ_y_gt=0.0;		//
volatile double FZ_bfb_gt =0.0;   //
volatile double SZGL_gt = 0.0;			//
volatile double GLYS_gt = 0.0;		//

/*
*********************************************************************************************************
*	º¯ Êý Ãû £dvoid Test_Param_Init(void)
*	¹¦ÄÜËµÃ÷ : ²âÊÔËùÓÐÏà¹Ø²ÎÊý³õÊ¼»¯
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ÎÞ
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
	DN_Struct.liangch_flag = 3;   //Á¿³Ì±ê¼Ç(0,1,2,3)
	DN_Struct.liangch_zdlc = 3;   //×Ô¶¯Á¿³ÌµµÎ»±êÖ¾(1,2,3(10Á¿³Ì))
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
*	º¯ Êý Ãû £dstatic double SZLB(u8 n,volatile double *DATA)
*	¹¦ÄÜËµÃ÷ : Êý×ÖÂË²¨
*	ÐÎ    ²Î : n : ÖÜÆÚÊý   DATA£º Ô¤´¦ÀíµÄÊý¾Ý
*	·µ »Ø Öµ : Result
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
		if(n<=3)	Result=(Result/n);  //Èç¹ûn<=3,ÔòÖ±½ÓÈ¡Æ½¾ùÖµ
		else 			Result=(Result-Max_Dat-Min_Dat)/(n-2);
		return(Result);
}
/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºvoid ZK_Test_Init(void)
*	¹¦ÄÜËµÃ÷ : ×è¿¹²âÊÔ³õÊ¼»¯
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ÎÞ
*********************************************************************************************************
*/
/*
static void ZK_Test_Init(void)
{
		CH1_BIAO();           //SVin1+
		CH3_CHA();						//CVin3+  ²»·ÖÑ¹
		CK204_Biao_1();				//³Ì¿Ø
		CK204_Cha_1();
}
*/
/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºvoid DN_Test_Init(void)
*	¹¦ÄÜËµÃ÷ : µ¼ÄÉ²âÊÔ³õÊ¼»¯
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ÎÞ
*********************************************************************************************************
*/
/*
static void DN_Test_Init(void)
{
		CH3_BIAO();           //SVin3+	²»·ÖÑ¹
		CH1_CHA();					  //CVin1+
		CK204_Biao_1();				//³Ì¿Ø
		CK204_Cha_1();
}
*/
/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºvoid DN_Test_Init(void)
*	¹¦ÄÜËµÃ÷ : µ¼ÄÉ²âÊÔ³õÊ¼»¯
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ÎÞ
*********************************************************************************************************
*/
void FZX_Test_Init(u8 mode_flag)
{
	if(mode_flag)   //µ¼ÄÉ²âÊÔ³õÊ¼»¯
	{
//	CH3_BIAO();           //SVin3+	²»·ÖÑ¹
		CH2_BIAO();						//SVin2+    ·ÖÑ¹
		CH1_CHA();					  //CVin1+
		DN_Struct.Div_flag = 1;
		CK204_Biao_1();				//³Ì¿Ø
		CK204_Cha_1();
	}
	else					//×è¿¹²âÊÔ³õÊ¼»¯
	{
		CH1_BIAO();           //SVin1+
//	CH3_CHA();						//CVin3+  ²»·ÖÑ¹
		CH2_CHA();						//CVin2+    ·ÖÑ¹
		ZK_Struct.Div_flag = 1;
		CK204_Biao_1();				//³Ì¿Ø
		CK204_Cha_1();
	}
}
/*
*********************************************************************************************************
*	º¯ Êý Ãû 	 static enum result AC_Test(void)
*	¹¦ÄÜËµÃ÷ : ½»Á÷FFT²ÉÑù
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : Test_result £º FALSE£º²ÉÑùÊ§°Ü  TURE: ²ÉÑù³É¹¦
*********************************************************************************************************
*/
static enum result AC_Test(void)
{
	enum result Test_result=FALSE;
	u16 i=0,j=0;
	u32 m=0;
	
	m=0;
	
	SYNC =1;
	vTaskSuspendAll();				//ÈÎÎñµ÷¶ÈÆ÷ÉÏËø
	do
	{
		if(read_flag==1)
		{
			ADS1274_ReadNowAdc();
			FFT_Struct.Sum_Biao[m]=AD_val[0];  //±ê×¼Í¨µÀ
			FFT_Struct.Sum_Cha[m] =AD_val[1];  //²îÖµÍ¨µÀ
			read_flag = 0;
			m++;
		}
	}while(m<N);
	xTaskResumeAll();				//½âËø
	SYNC =0;
	
	Test_result = TURE;				//²âÊÔÍê³É
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
		
		/***************±ê×¼Í¨µÀÐÅºÅÏàÎ»´¦Àí**************************************/
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
				
		/****************²îÖµÍ¨µÀÐÅºÅÏàÎ»´¦Àí**************************************/
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
	U_Biao=SZLB(Test_Per,FFT_Struct.Avg_Biao);  //Êý×ÖÂË²¨;
	U_Cha =SZLB(Test_Per,FFT_Struct.Avg_Cha);
		
	for(i=0;i<Test_Per;i++)
	{
		FFT_Struct.Arctan_Biao[i]-= (Table_Biao_JXS[CK204_Biao]/H_J);
		FFT_Struct.Arctan_Cha[i] -= (Table_Biao_JXS[CK204_Cha]/H_J);
		
		FFT_Struct.Arctan_Cha_Biao[i] = FFT_Struct.Arctan_Cha[i]-FFT_Struct.Arctan_Biao[i];
																												
		FFT_Struct.Cos_jiao[i] = cos(FFT_Struct.Arctan_Cha_Biao[i]);
		FFT_Struct.Sin_jiao[i] = sin(FFT_Struct.Arctan_Cha_Biao[i]);
	}	
	FFT_Struct.Cos_jiao_zhi=SZLB(Test_Per,(double*)FFT_Struct.Cos_jiao);  //Êý×ÖÂË²¨
	FFT_Struct.Sin_jiao_zhi=SZLB(Test_Per,(double*)FFT_Struct.Sin_jiao);
	
	U_Biao/=Table_Biao[CK204_Biao]; //U_Biao×ª»¯Îª³Ì¿ØÖ®Ç°µÄµçÑ¹
	U_Cha/=Table_Cha[CK204_Cha];
	
	return Test_result;
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºstatic enum result CK_PGA(void)
*	¹¦ÄÜËµÃ÷ : ³Ì¿Ø·Å´óÇÐ»»º¯Êý
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : 
*********************************************************************************************************
*/
static enum result CK_PGA(void)
{
	enum result Test_result=FALSE;

	Test_result=AC_Test();

/********************±ê×¼Í¨µÀ³Ì¿Ø·Å ********************* */
	if(U_Biao>0.16)
	{
			CK204_Biao_1();     //·Å´ó1±¶
			CK204_Biao = 0;	 
	}
	else if((U_Biao>0.016)&&(U_Biao<=0.16))
	{
			CK204_Biao_10();    //·Å´ó10±¶
			CK204_Biao = 1;
	}
	else if((U_Biao>0.0016)&&(U_Biao<=0.016))
	{
			CK204_Biao_100();   //·Å´ó100±¶
			CK204_Biao = 2;
	}
	else   //<=0.0016
	{
			CK204_Biao_1000();    //·Å´ó1000±¶
			CK204_Biao = 3;
	}
/*************************end***************************************/	
		
/**********************²îÖµÍ¨µÀ³Ì¿Ø·Å **********************/
	if(U_Cha>0.16)
	{
			CK204_Cha_1();     //·Å´ó1±¶
			CK204_Cha = 0;
	}
	else if((U_Cha>0.016)&&(U_Cha<=0.16))
	{
			CK204_Cha_10();     //·Å´ó10±¶
			CK204_Cha = 1;
	}
	else if((U_Cha>0.0016)&&(U_Cha<=0.016))
	{
			CK204_Cha_100();     //·Å´ó100±¶
			CK204_Cha = 2;
	}
	else   //<=0.0016
	{
			CK204_Cha_1000();     //·Å´ó1000±¶
			CK204_Cha = 3;
	}
/****************************end************************************/
	return(Test_result);
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºstatic void Swith_LC(u8 lc)
*	¹¦ÄÜËµÃ÷ : ²ÉÑùµç×è×ª»¯º¯Êý
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ :
*********************************************************************************************************
*/
static void Swith_LC(u8 lc)
{
	static u8 lc_tem=0;
	
	lc_tem=lc;
	
	if(lc_tem==DN_Struct.liangch_zdlc)     //ÒÑ¾­´¦ÓÚ¸ÃÁ¿³Ì£¬ÎÞÐèÔÙÇÐ»»
		 return;            
	
	if(lc_tem>3)       //ÏÞÖÆ
			lc_tem=3;
	if(lc_tem==1)
	{
			On_C_Sampl_Res1();  //ÎüºÏ
			vTaskDelay(500);
			Off_C_Sampl_Res2();  //¶Ï¿ª
			Off_C_Sampl_Res3();   //¶Ï¿ª
			DN_Struct.liangch_zdlc=1;
	}
	else if(lc_tem==2)
	{
			On_C_Sampl_Res2();  //ÎüºÏ
			vTaskDelay(500);
			Off_C_Sampl_Res1() ;  //¶Ï¿ª
			Off_C_Sampl_Res3();   //¶Ï¿ª
			DN_Struct.liangch_zdlc=2;
	}
	else   //3
	{
			On_C_Sampl_Res3();  //ÎüºÏ
			vTaskDelay(500);
			Off_C_Sampl_Res1();  //¶Ï¿ª
			Off_C_Sampl_Res2();   //¶Ï¿ª
			DN_Struct.liangch_zdlc=3;
	}
	vTaskDelay(500);
}


/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºstatic void Auto_LC_DN(void)
*	¹¦ÄÜËµÃ÷ : µ¼ÄÉ²âÊÔÊ±×Ô¶¯Á¿³ÌÇÐ»»º¯Êý
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ÎÞ
*********************************************************************************************************
*/
static void Auto_LC_DN(void)
{
	if(DN_Struct.liangch_flag!=0)  return;    //²»ÊÇ×Ô¶¯Á¿³Ì

	WC_Peak=U_i/U_Iu;    //µ¼ÄÉ
	
	if(WC_Peak>3.3)
			Swith_LC(3);
	else if(WC_Peak>0.33)
			Swith_LC(2);
	else
			Swith_LC(1);
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºvoid vol_div(u8 flag_fy)
*	¹¦ÄÜËµÃ÷ : ×è¿¹µ¼ÄÉÊÇ·ñ·ÖÑ¹ÅÐ¶Ïº¯Êý
*	ÐÎ    ²Î : u8 flag_fy    0£º ²»·ÖÑ¹   1£º·ÖÑ¹
*	·µ »Ø Öµ : ÎÞ
*********************************************************************************************************
*/
static void vol_div(const u8 Mod_Sel,const u8 flag_fy)
{
	if(Mod_Sel == 0)											//×è¿¹·ÖÑ¹
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
	else if(Mod_Sel == 1)								//µ¼ÄÉ·ÖÑ¹
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
*	º¯ Êý Ãû £  static enum result Zukang_test(void)
*	¹¦ÄÜËµÃ÷ : ×è¿¹²âÊÔº¯Êý
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ·µ»ØÖµTest_result ÎªTURE   Íê³ÉÒ»´Î²âÊÔ£¨Õý³£Êý¾Ý£©  ÎªFALSE £º²âÊÔÓÐÎó
*********************************************************************************************************
*/
static enum result Zukang_test(void)
{
	enum result Test_result=FALSE;
	
	Test_result=CK_PGA();
	
	U_Iu = U_Biao/XS_CT0*BZ_DifRto;
	
	U_i = U_Cha*Fy_table[ZK_Struct.Div_flag]*CZ_DifRto;
	
	/*************·ÖÑ¹Í¨µÀÅÐ¶Ï************************/
	
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
	
	ZK_Struct.ZK = U_i/U_Iu;                   //×è¿¹
	
	ZK_Struct.ZK_x = ZK_Struct.ZK*FFT_Struct.Cos_jiao_zhi;   //×è¿¹Í¬Ïò·ÖÁ¿£º Êµ²¿µç×è
	ZK_Struct.ZK_y = ZK_Struct.ZK*FFT_Struct.Sin_jiao_zhi;   //×è¿¹Õý½»·ÖÁ¿£º Ðé²¿µç¿¹
		
	ZK_Struct.ZK_bfb=(U_Iu/ZK_Struct.EC_dl)*100;  //×è¿¹²âÊÔÊ±£¬µ±Ç°°Ù·Ö±í
	
	return (Test_result);
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºstatic enum result Daona_test(void)
*	¹¦ÄÜËµÃ÷ : ×è¿¹²âÊÔº¯Êý
*	ÐÎ    ²Î : ÎÞ
*	·µ »Ø Öµ : ·µ»ØÖµTest_result ÎªTURE   Íê³ÉÒ»´Î²âÊÔ£¨Õý³£Êý¾Ý£©  ÎªFALSE £º²âÊÔÓÐÎó
*********************************************************************************************************
*/
static enum result Daona_test(void)
{
	enum result Test_result=FALSE;
	volatile double CH_U_Iu=0.0;					//¶Ë×ÓÉÏµçÑ¹Á¿£¬ÓÃÓÚÅÐ¶ÏÊÇ·ñ·ÖÑ¹
	
	Test_result = CK_PGA();
	
	U_Iu = U_Biao*Fy_table[DN_Struct.Div_flag]*VolIslationRatio*BZ_DifRto;    //±ê×¼Í¨µÀ	£º µ¼ÄÉµçÑ¹
	CH_U_Iu = U_Iu/VolIslationRatio;
	
	U_i  = U_Cha/(XS_K[DN_Struct.liangch_zdlc])*BZ_DifRto*1000;  	//²îÖµÍ¨µÀ  :   µ¼ÄÉµçÁ÷
	
	Auto_LC_DN();								//¸ù¾Ý×ÛºÏÎó²î×Ô¶¯ÇÐ»»Á¿³Ì£¨²ÉÑùµç×è£©
	
	/*************·ÖÑ¹Í¨µÀÅÐ¶Ï************************/
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
		
		DN_Struct.DN = U_i/U_Iu;             //µ¼ÄÉ(ms)
		
		DN_Struct.DN_x = DN_Struct.DN*FFT_Struct.Cos_jiao_zhi;   //µ¼ÄÉÍ¬Ïò·ÖÁ¿£º µçµ¼
		DN_Struct.DN_y = DN_Struct.DN*FFT_Struct.Sin_jiao_zhi;   //µ¼ÄÉÕý½»·ÖÁ¿£º µçÄÉ
			
		DN_Struct.DN_bfb=(U_Iu/DN_Struct.EC_dy)*100;    //µ¼ÄÉ²âÊÔÊ±£¬µ±Ç°°Ù·Ö±í
		
		DN_Struct.SZGL = DN_Struct.EC_dy*DN_Struct.EC_dy*DN_Struct.DN /1000;      			//ÊÓÔÚ¹¦ÂÊ
		
		DN_Struct.GLYS = FFT_Struct.Cos_jiao_zhi;    	//¹¦ÂÊÒòÊý
		
		return (Test_result);	
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû £ºvoid FZX_Test(const u8 mode_flag)
*	¹¦ÄÜËµÃ÷ : ×è¿¹ºÍµ¼ÄÉ²âÊÔ
*	ÐÎ    ²Î : u8 mode_flag ;  0: ×è¿¹²âÊÔ   1£ºµ¼ÄÉ²âÊÔ
*	·µ »Ø Öµ : ÎÞ
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
	if(i>=3)    //°Ù·Ö±íÈ¡3´ÎÆ½¾ù
	{
		FZ_bfb_gt=0;
		for(k=0;k<i;k++)
		{
				FZ_bfb_gt+=fz_bfb_Tab[k];
		}
		FZ_bfb_gt/=i;
		i=0;
	}
	if(j>=5)   //ÆäÓà²ÎÊýÈ¡5´ÎÆ½¾ù
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
*	º¯ Êý Ãû £ºvoid DC_Test(void)
*	¹¦ÄÜËµÃ÷ : Ö±Á÷ÐÅºÅ²âÊÔ (µ÷ÊÔ²âÊÔÓÃ)
*	ÐÎ    ²Î : 
*	·µ »Ø Öµ : 
* Ê¹ÓÃËµÃ÷ £º ½«²âÊÔÖ±Á÷µçÑ¹½ÓÈëSVin3+  ºÍ CVin3+ Í¨µÀÉÏ
*********************************************************************************************************
*/
void DC_test(void)
{
	CH3_BIAO();   //SVin3+  SVin3-
	CH3_CHA();    //CVin3+  CVin3-
	
	CK204_Biao_1();
	CK204_Cha_1();
}
