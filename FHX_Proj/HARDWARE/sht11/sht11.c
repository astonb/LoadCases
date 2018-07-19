#include "sys.h"
#include "delay.h"
#include "sht11.h"

u8 wd[2];    //存放温度
u8 sd[2];    //存放湿度


double Temperature,Humidity;  

/********************************************************
               SHT11初始化IO口配置
********************************************************/
void SHT_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10|GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	
		GPIO_Init(GPIOB,&GPIO_InitStructure);
}

/********************************************************
               SHT11写字节程序
********************************************************/
static char WriteByte(unsigned char value)
{
		unsigned char i,error=0;
		 for(i=0x80;0<i;i/=2)
		 {
				if(i&value)
				 SDAH;
				else
				 SDAL;
				SCKH;
//				delay_us(1);
				SCKL;
		 }
		 SDAH;
		 SCKH;
//		 delay_us(1);
		 ReadState();
		 error=DATA();
		 WriteState();
//		 delay_us(1);
		 SCKL;
		 SDAH;
		 return error;
}
/********************************************************
						SHT11读字节程序
********************************************************/
static char ReadByte(unsigned char ack)
{
		 unsigned char i,val=0;
		 SDAH;
		 ReadState();
		 for(i=0x80;0<i;i/=2)
		 { 
				SCKH;
//				delay_us(1);
				if(DATA())
				val=(val|i);
//				delay_us(1);
				SCKL; 
//				delay_us(1);
		 }
		 WriteState();
		 if(ack==1)
			SDAL;
		 else
			SDAH;
//		 delay_us(1);
		 SCKH;
//		 delay_us(1);
		 SCKL;  
//		 delay_us(1);
		 SDAH;
		 return val;
}

/********************************************************
										SHT11启动传输
********************************************************/
static void TransStart(void)
{
		 SDAH;
		 SCKL;
//		 delay_us(1);
		 SCKH;
//		 delay_us(1);
		 SDAL;
//		 delay_us(1);
		 SCKL;
//		 delay_us(1);
		 SCKH;
//		 delay_us(1);
		 SDAH;
//		 delay_us(1);
		 SCKL;
//		 delay_us(1);

}
/********************************************************
									SHT11连接复位
********************************************************/
void ConnectionReset(void)
{
		 unsigned char i;
		 SDAH;
		 SCKL;
//		 delay_us(1);
		 for(i=0;i<9;i++)
		 {
				SCKH;
//				delay_us(1);
				SCKL;
//				delay_us(1);
		 }
		 TransStart(); 
}
/********************************************************
										SHT11温湿度检测
********************************************************/
static char Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
		 unsigned char error=0;
		 TransStart();
		 switch(mode)
		 {
				case TEMP:
					error+=WriteByte(MEASURE_TEMP);
				  break;
				case HUMI:
					error+=WriteByte(MEASURE_HUMI);
					break;
				default:
					break;
		 }
		 ReadState();
//		 delay_us(320000);
		 if(DATA())
				error+=1;  
		 WriteState();
		 *(p_value)=ReadByte(ACK);
		 *(p_value+1)=ReadByte(ACK);
		 *p_checksum=ReadByte(noACK);
		 return error;

}

/********************************************************
									SHT11温湿度补偿及测量
********************************************************/

void measure_wsd(void)
{
		u8 error;
		u8 checksum; //CRC
		double rh_lin;
		double rh_true;
		error=0;
		error+=Measure((unsigned char*)&sd,&checksum,HUMI);
		error+=Measure((unsigned char*)&wd,&checksum,TEMP);
		if(error!=0)
			ConnectionReset();
		Temperature=((wd[0]<<8)+wd[1]);
		Temperature=(float)(Temperature*0.01 - 40);
		Humidity=((sd[0]<<8)+sd[1]);
		rh_lin=-0.0000028*Humidity*Humidity + 0.0405*Humidity -4.0; 
		rh_true=(Temperature-25)*(0.01+0.00008*Humidity )+rh_lin;
		Humidity=rh_true;
}
