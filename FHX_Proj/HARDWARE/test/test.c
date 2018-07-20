#include "test.h"
#include "ads1274.h"
#include "delay.h"
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"

extern u32 AD_val[ReadAdcChx];   //�洢��ȡAD1274��4ͨ����AD��ֵ

const double Fy_table[2]={1,Fy_ratio};


const double Table_Biao_JXS[4]={	//Biao_204�ǲ�ϵ��
0.0, 			//1 
0.0, 			//10
0.0,			//100  
0.0,			//1000
};
const double Table_Cha_JXS[4]={	//Cha_204�ǲ�ϵ��
0.0, 			//1 
0.0, 			//10
0.0,			//100  
0.0,			//1000 
};


const float XS_K[4]={	
	(22),               //����
	(22.0*1.00000),		  //22  ŷķ��������(0.1����)
	(2.2*1.00000), 		  //2.2   ŷķ��������(1����)
	(0.2*1.00000), 		  //0.2 ŷķ��������(10����)
	};


FFT FFT_Struct;
DN 	DN_Struct;
ZK  ZK_Struct;

u8 read_flag=0;    //ADS1274�ɶ���־λ

//����ģʽѡ��
volatile u8 Mod_Sel=0;  //0���迹����  1�����ɲ���

//ADS1274ͨ�������ĵ�ѹ
volatile double U_Biao=0.0;		
volatile double U_Cha=0.0;

//�����ر�������
static volatile double WC_Peak=0.0; //�ۺ����
volatile double U_Iu=0.0;					//��׼ͨ����Դ�ź�
volatile double U_i=0.0;					//��ֵͨ����Դ�ź�

//�Ŵ�����־λ :  0 ->1��  1->10��  2->100��   3->1000��
volatile u8 CK204_Biao=0; 
volatile u8 CK204_Cha=0;  
const float Table_Biao[4]={1.0,10,100,1000};
const float Table_Cha[4]= {1.0,10,100,1000};

//�����м������ݴ�
volatile double fz_Tab[8]= {0.0};
volatile double fz_x_Tab[8] = {0.0};
volatile double fz_y_Tab[8] = {0.0};
volatile double fz_bfb_Tab[8] = {0.0};
volatile double SZGL_Tab[8] = {0.0};
volatile double glys_Tab[8] = {0.0};

//���մ�Ų��Խ������
volatile double FZ_gt=0.0;    //						
volatile double FZ_x_gt=0.0;		//             				
volatile double FZ_y_gt=0.0;		//
volatile double FZ_bfb_gt =0.0;   //
volatile double SZGL_gt = 0.0;			//
volatile double GLYS_gt = 0.0;		//

/*
*********************************************************************************************************
*	�� �� �� �dvoid Test_Param_Init(void)
*	����˵�� : ����������ز�����ʼ��
*	��    �� : ��
*	�� �� ֵ : ��
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
	DN_Struct.liangch_flag = 3;   //���̱��(0,1,2,3)
	DN_Struct.liangch_zdlc = 3;   //�Զ����̵�λ��־(1,2,3(10����))
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
*	�� �� �� �dstatic double SZLB(u8 n,volatile double *DATA)
*	����˵�� : �����˲�
*	��    �� : n : ������   DATA�� Ԥ���������
*	�� �� ֵ : Result
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
		if(n<=3)	Result=(Result/n);  //���n<=3,��ֱ��ȡƽ��ֵ
		else 			Result=(Result-Max_Dat-Min_Dat)/(n-2);
		return(Result);
}
/*
*********************************************************************************************************
*	�� �� �� ��void ZK_Test_Init(void)
*	����˵�� : �迹���Գ�ʼ��
*	��    �� : ��
*	�� �� ֵ : ��
*********************************************************************************************************
*/
/*
static void ZK_Test_Init(void)
{
		CH1_BIAO();           //SVin1+
		CH3_CHA();						//CVin3+  ����ѹ
		CK204_Biao_1();				//�̿�
		CK204_Cha_1();
}
*/
/*
*********************************************************************************************************
*	�� �� �� ��void DN_Test_Init(void)
*	����˵�� : ���ɲ��Գ�ʼ��
*	��    �� : ��
*	�� �� ֵ : ��
*********************************************************************************************************
*/
/*
static void DN_Test_Init(void)
{
		CH3_BIAO();           //SVin3+	����ѹ
		CH1_CHA();					  //CVin1+
		CK204_Biao_1();				//�̿�
		CK204_Cha_1();
}
*/
/*
*********************************************************************************************************
*	�� �� �� ��void DN_Test_Init(void)
*	����˵�� : ���ɲ��Գ�ʼ��
*	��    �� : ��
*	�� �� ֵ : ��
*********************************************************************************************************
*/
void FZX_Test_Init(u8 mode_flag)
{
	if(mode_flag)   //���ɲ��Գ�ʼ��
	{
//	CH3_BIAO();           //SVin3+	����ѹ
		CH2_BIAO();						//SVin2+    ��ѹ
		CH1_CHA();					  //CVin1+
		DN_Struct.Div_flag = 1;
		CK204_Biao_1();				//�̿�
		CK204_Cha_1();
	}
	else					//�迹���Գ�ʼ��
	{
		CH1_BIAO();           //SVin1+
//	CH3_CHA();						//CVin3+  ����ѹ
		CH2_CHA();						//CVin2+    ��ѹ
		ZK_Struct.Div_flag = 1;
		CK204_Biao_1();				//�̿�
		CK204_Cha_1();
	}
}
/*
*********************************************************************************************************
*	�� �� �� 	 static enum result AC_Test(void)
*	����˵�� : ����FFT����
*	��    �� : ��
*	�� �� ֵ : Test_result �� FALSE������ʧ��  TURE: �����ɹ�
*********************************************************************************************************
*/
static enum result AC_Test(void)
{
	enum result Test_result=FALSE;
	u16 i=0,j=0;
	u32 m=0;
	
	m=0;
	
	SYNC =1;
	vTaskSuspendAll();				//�������������
	do
	{
		if(read_flag==1)
		{
			ADS1274_ReadNowAdc();
			FFT_Struct.Sum_Biao[m]=AD_val[0];  //��׼ͨ��
			FFT_Struct.Sum_Cha[m] =AD_val[1];  //��ֵͨ��
			read_flag = 0;
			m++;
		}
	}while(m<N);
	xTaskResumeAll();				//����
	SYNC =0;
	
	Test_result = TURE;				//�������
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
		
		/***************��׼ͨ���ź���λ����**************************************/
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
				
		/****************��ֵͨ���ź���λ����**************************************/
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
	U_Biao=SZLB(Test_Per,FFT_Struct.Avg_Biao);  //�����˲�;
	U_Cha =SZLB(Test_Per,FFT_Struct.Avg_Cha);
		
	for(i=0;i<Test_Per;i++)
	{
		FFT_Struct.Arctan_Biao[i]-= (Table_Biao_JXS[CK204_Biao]/H_J);
		FFT_Struct.Arctan_Cha[i] -= (Table_Biao_JXS[CK204_Cha]/H_J);
		
		FFT_Struct.Arctan_Cha_Biao[i] = FFT_Struct.Arctan_Cha[i]-FFT_Struct.Arctan_Biao[i];
																												
		FFT_Struct.Cos_jiao[i] = cos(FFT_Struct.Arctan_Cha_Biao[i]);
		FFT_Struct.Sin_jiao[i] = sin(FFT_Struct.Arctan_Cha_Biao[i]);
	}	
	FFT_Struct.Cos_jiao_zhi=SZLB(Test_Per,(double*)FFT_Struct.Cos_jiao);  //�����˲�
	FFT_Struct.Sin_jiao_zhi=SZLB(Test_Per,(double*)FFT_Struct.Sin_jiao);
	
	U_Biao/=Table_Biao[CK204_Biao]; //U_Biaoת��Ϊ�̿�֮ǰ�ĵ�ѹ
	U_Cha/=Table_Cha[CK204_Cha];
	
	return Test_result;
}

/*
*********************************************************************************************************
*	�� �� �� ��static enum result CK_PGA(void)
*	����˵�� : �̿طŴ��л�����
*	��    �� : ��
*	�� �� ֵ : 
*********************************************************************************************************
*/
static enum result CK_PGA(void)
{
	enum result Test_result=FALSE;

	Test_result=AC_Test();

/********************��׼ͨ���̿ط� ********************* */
	if(U_Biao>0.16)
	{
			CK204_Biao_1();     //�Ŵ�1��
			CK204_Biao = 0;	 
	}
	else if((U_Biao>0.016)&&(U_Biao<=0.16))
	{
			CK204_Biao_10();    //�Ŵ�10��
			CK204_Biao = 1;
	}
	else if((U_Biao>0.0016)&&(U_Biao<=0.016))
	{
			CK204_Biao_100();   //�Ŵ�100��
			CK204_Biao = 2;
	}
	else   //<=0.0016
	{
			CK204_Biao_1000();    //�Ŵ�1000��
			CK204_Biao = 3;
	}
/*************************end***************************************/	
		
/**********************��ֵͨ���̿ط� **********************/
	if(U_Cha>0.16)
	{
			CK204_Cha_1();     //�Ŵ�1��
			CK204_Cha = 0;
	}
	else if((U_Cha>0.016)&&(U_Cha<=0.16))
	{
			CK204_Cha_10();     //�Ŵ�10��
			CK204_Cha = 1;
	}
	else if((U_Cha>0.0016)&&(U_Cha<=0.016))
	{
			CK204_Cha_100();     //�Ŵ�100��
			CK204_Cha = 2;
	}
	else   //<=0.0016
	{
			CK204_Cha_1000();     //�Ŵ�1000��
			CK204_Cha = 3;
	}
/****************************end************************************/
	return(Test_result);
}

/*
*********************************************************************************************************
*	�� �� �� ��static void Swith_LC(u8 lc)
*	����˵�� : ��������ת������
*	��    �� : ��
*	�� �� ֵ :
*********************************************************************************************************
*/
static void Swith_LC(u8 lc)
{
	static u8 lc_tem=0;
	
	lc_tem=lc;
	
	if(lc_tem==DN_Struct.liangch_zdlc)     //�Ѿ����ڸ����̣��������л�
		 return;            
	
	if(lc_tem>3)       //����
			lc_tem=3;
	if(lc_tem==1)
	{
			On_C_Sampl_Res1();  //����
			vTaskDelay(500);
			Off_C_Sampl_Res2();  //�Ͽ�
			Off_C_Sampl_Res3();   //�Ͽ�
			DN_Struct.liangch_zdlc=1;
	}
	else if(lc_tem==2)
	{
			On_C_Sampl_Res2();  //����
			vTaskDelay(500);
			Off_C_Sampl_Res1() ;  //�Ͽ�
			Off_C_Sampl_Res3();   //�Ͽ�
			DN_Struct.liangch_zdlc=2;
	}
	else   //3
	{
			On_C_Sampl_Res3();  //����
			vTaskDelay(500);
			Off_C_Sampl_Res1();  //�Ͽ�
			Off_C_Sampl_Res2();   //�Ͽ�
			DN_Struct.liangch_zdlc=3;
	}
	vTaskDelay(500);
}


/*
*********************************************************************************************************
*	�� �� �� ��static void Auto_LC_DN(void)
*	����˵�� : ���ɲ���ʱ�Զ������л�����
*	��    �� : ��
*	�� �� ֵ : ��
*********************************************************************************************************
*/
static void Auto_LC_DN(void)
{
	if(DN_Struct.liangch_flag!=0)  return;    //�����Զ�����

	WC_Peak=U_i/U_Iu;    //����
	
	if(WC_Peak>3.3)
			Swith_LC(3);
	else if(WC_Peak>0.33)
			Swith_LC(2);
	else
			Swith_LC(1);
}

/*
*********************************************************************************************************
*	�� �� �� ��void vol_div(u8 flag_fy)
*	����˵�� : �迹�����Ƿ��ѹ�жϺ���
*	��    �� : u8 flag_fy    0�� ����ѹ   1����ѹ
*	�� �� ֵ : ��
*********************************************************************************************************
*/
static void vol_div(const u8 Mod_Sel,const u8 flag_fy)
{
	if(Mod_Sel == 0)											//�迹��ѹ
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
	else if(Mod_Sel == 1)								//���ɷ�ѹ
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
*	�� �� �� �  static enum result Zukang_test(void)
*	����˵�� : �迹���Ժ���
*	��    �� : ��
*	�� �� ֵ : ����ֵTest_result ΪTURE   ���һ�β��ԣ��������ݣ�  ΪFALSE ����������
*********************************************************************************************************
*/
static enum result Zukang_test(void)
{
	enum result Test_result=FALSE;
	
	Test_result=CK_PGA();
	
	U_Iu = U_Biao/XS_CT0*BZ_DifRto;
	
	U_i = U_Cha*Fy_table[ZK_Struct.Div_flag]*CZ_DifRto;
	
	/*************��ѹͨ���ж�************************/
	
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
	
	ZK_Struct.ZK = U_i/U_Iu;                   //�迹
	
	ZK_Struct.ZK_x = ZK_Struct.ZK*FFT_Struct.Cos_jiao_zhi;   //�迹ͬ������� ʵ������
	ZK_Struct.ZK_y = ZK_Struct.ZK*FFT_Struct.Sin_jiao_zhi;   //�迹���������� �鲿�翹
		
	ZK_Struct.ZK_bfb=(U_Iu/ZK_Struct.EC_dl)*100;  //�迹����ʱ����ǰ�ٷֱ�
	
	return (Test_result);
}

/*
*********************************************************************************************************
*	�� �� �� ��static enum result Daona_test(void)
*	����˵�� : �迹���Ժ���
*	��    �� : ��
*	�� �� ֵ : ����ֵTest_result ΪTURE   ���һ�β��ԣ��������ݣ�  ΪFALSE ����������
*********************************************************************************************************
*/
static enum result Daona_test(void)
{
	enum result Test_result=FALSE;
	volatile double CH_U_Iu=0.0;					//�����ϵ�ѹ���������ж��Ƿ��ѹ
	
	Test_result = CK_PGA();
	
	U_Iu = U_Biao*Fy_table[DN_Struct.Div_flag]*VolIslationRatio*BZ_DifRto;    //��׼ͨ��	�� ���ɵ�ѹ
	CH_U_Iu = U_Iu/VolIslationRatio;
	
	U_i  = U_Cha/(XS_K[DN_Struct.liangch_zdlc])*BZ_DifRto*1000;  	//��ֵͨ��  :   ���ɵ���
	
	Auto_LC_DN();								//�����ۺ�����Զ��л����̣��������裩
	
	/*************��ѹͨ���ж�************************/
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
		
		DN_Struct.DN = U_i/U_Iu;             //����(ms)
		
		DN_Struct.DN_x = DN_Struct.DN*FFT_Struct.Cos_jiao_zhi;   //����ͬ������� �絼
		DN_Struct.DN_y = DN_Struct.DN*FFT_Struct.Sin_jiao_zhi;   //�������������� ����
			
		DN_Struct.DN_bfb=(U_Iu/DN_Struct.EC_dy)*100;    //���ɲ���ʱ����ǰ�ٷֱ�
		
		DN_Struct.SZGL = DN_Struct.EC_dy*DN_Struct.EC_dy*DN_Struct.DN /1000;      			//���ڹ���
		
		DN_Struct.GLYS = FFT_Struct.Cos_jiao_zhi;    	//��������
		
		return (Test_result);	
}

/*
*********************************************************************************************************
*	�� �� �� ��void FZX_Test(const u8 mode_flag)
*	����˵�� : �迹�͵��ɲ���
*	��    �� : u8 mode_flag ;  0: �迹����   1�����ɲ���
*	�� �� ֵ : ��
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
	if(i>=3)    //�ٷֱ�ȡ3��ƽ��
	{
		FZ_bfb_gt=0;
		for(k=0;k<i;k++)
		{
				FZ_bfb_gt+=fz_bfb_Tab[k];
		}
		FZ_bfb_gt/=i;
		i=0;
	}
	if(j>=5)   //�������ȡ5��ƽ��
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
*	�� �� �� ��void DC_Test(void)
*	����˵�� : ֱ���źŲ��� (���Բ�����)
*	��    �� : 
*	�� �� ֵ : 
* ʹ��˵�� �� ������ֱ����ѹ����SVin3+  �� CVin3+ ͨ����
*********************************************************************************************************
*/
void DC_test(void)
{
	CH3_BIAO();   //SVin3+  SVin3-
	CH3_CHA();    //CVin3+  CVin3-
	
	CK204_Biao_1();
	CK204_Cha_1();
}
