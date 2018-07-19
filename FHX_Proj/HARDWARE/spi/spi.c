#include "./spi.h"
#include "delay.h"
#include "ads1274.h"

/*
*********************************************************************************************************
*	函 数 名 ：void SPI1_Init(void)
*	功能说明 : IO模拟SPI 时序 操作 
*	形    参 : 无
*	返 回 值 : 
*********************************************************************************************************
*/
/*
void SPI1_Init(void)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);  //使能GPIOA时钟
	
	//SCLK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   //复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;   //上拉
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//DIN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;   //复用功能
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PU; //推挽输出
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	SCLK=0;
}   
*/

/*
*********************************************************************************************************
*	函 数 名 ：void SPI1_Init(void)
*	功能说明 : SPI1 初始化函数， SCK 、MISO
*	形    参 : 无
*	返 回 值 : 
*********************************************************************************************************
*/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);  //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);   //使能SPI1时钟
	
	//GPIOA5: SCLK  GPIOA6: MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;   //复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//复用配置
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	
	//这里只针对SPI口初始化
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//停止复位SPI1
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_RxOnly;  //只读模式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;   //主机
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //8bit 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;        //SCLK IDLE STATUE IS High
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; 
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	
	SPI_Init(SPI1,&SPI_InitStructure);
	
	SPI_Cmd(SPI1,DISABLE);   //使能SPI1
}

/*
*********************************************************************************************************
*	函 数 名 ：__inline u32 SPI1_Read_Data(void)
*	功能说明 : 连续读取ADC 3次，得到该通道电压对应的AD码值
*	形    参 : 无
*	返 回 值 : ad_val ： 读取到的AD 码值
* 	note   :	在	SPI	读数据的时候，下列固定读取6次（读取第1、2通道的数据），此处不加for循环，是为了
*							避免多余的指令影响ads1274移除数据，没有及时的读取从而造成数据的遗漏
*********************************************************************************************************
*/
__inline void SPI1_Read_Data(u32 *ad_val)
{
	u8 i=0;
	u8 data[3*ReadAdcChx]={0};
//	u32 ad_val=0;
	
	SPI_Cmd(SPI1, ENABLE);
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //等待接收完一个byte
	data[0] = SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //等待接收完一个byte
	data[1]=SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //等待接收完一个byte
	data[2]=SPI_I2S_ReceiveData(SPI1);
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //等待接收完一个byte
	data[3] = SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //等待接收完一个byte
	data[4]=SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //等待接收完一个byte
	data[5]=SPI_I2S_ReceiveData(SPI1);

	
	SPI_Cmd(SPI1, DISABLE);
	
	for(i=0;i<ReadAdcChx;i++)
	{
		ad_val[i]= ((data[3*i]<<16) | (data[3*i+1]<<8) | (data[3*i+2]))&0x00ffffff;
	}
	
//	return ad_val;
	
//	return SPI_I2S_ReceiveData(SPI1);
	
}


 










