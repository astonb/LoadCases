#include "./spi.h"
#include "delay.h"
#include "ads1274.h"

/*
*********************************************************************************************************
*	�� �� �� ��void SPI1_Init(void)
*	����˵�� : IOģ��SPI ʱ�� ���� 
*	��    �� : ��
*	�� �� ֵ : 
*********************************************************************************************************
*/
/*
void SPI1_Init(void)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);  //ʹ��GPIOAʱ��
	
	//SCLK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   //���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;   //����
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//DIN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;   //���ù���
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PU; //�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //����
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	SCLK=0;
}   
*/

/*
*********************************************************************************************************
*	�� �� �� ��void SPI1_Init(void)
*	����˵�� : SPI1 ��ʼ�������� SCK ��MISO
*	��    �� : ��
*	�� �� ֵ : 
*********************************************************************************************************
*/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);  //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);   //ʹ��SPI1ʱ��
	
	//GPIOA5: SCLK  GPIOA6: MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;   //���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //����
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//��������
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	
	//����ֻ���SPI�ڳ�ʼ��
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//��λSPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//ֹͣ��λSPI1
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_RxOnly;  //ֻ��ģʽ
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;   //����
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //8bit 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;        //SCLK IDLE STATUE IS High
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; 
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	
	SPI_Init(SPI1,&SPI_InitStructure);
	
	SPI_Cmd(SPI1,DISABLE);   //ʹ��SPI1
}

/*
*********************************************************************************************************
*	�� �� �� ��__inline u32 SPI1_Read_Data(void)
*	����˵�� : ������ȡADC 3�Σ��õ���ͨ����ѹ��Ӧ��AD��ֵ
*	��    �� : ��
*	�� �� ֵ : ad_val �� ��ȡ����AD ��ֵ
* 	note   :	��	SPI	�����ݵ�ʱ�����й̶���ȡ6�Σ���ȡ��1��2ͨ�������ݣ����˴�����forѭ������Ϊ��
*							��������ָ��Ӱ��ads1274�Ƴ����ݣ�û�м�ʱ�Ķ�ȡ�Ӷ�������ݵ���©
*********************************************************************************************************
*/
__inline void SPI1_Read_Data(u32 *ad_val)
{
	u8 i=0;
	u8 data[3*ReadAdcChx]={0};
//	u32 ad_val=0;
	
	SPI_Cmd(SPI1, ENABLE);
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //�ȴ�������һ��byte
	data[0] = SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //�ȴ�������һ��byte
	data[1]=SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //�ȴ�������һ��byte
	data[2]=SPI_I2S_ReceiveData(SPI1);
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //�ȴ�������һ��byte
	data[3] = SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //�ȴ�������һ��byte
	data[4]=SPI_I2S_ReceiveData(SPI1);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);   //�ȴ�������һ��byte
	data[5]=SPI_I2S_ReceiveData(SPI1);

	
	SPI_Cmd(SPI1, DISABLE);
	
	for(i=0;i<ReadAdcChx;i++)
	{
		ad_val[i]= ((data[3*i]<<16) | (data[3*i+1]<<8) | (data[3*i+2]))&0x00ffffff;
	}
	
//	return ad_val;
	
//	return SPI_I2S_ReceiveData(SPI1);
	
}


 










