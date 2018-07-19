#ifndef _SHT11_H

#define _SHT11_H

#include "sys.h"

#define noACK 0
#define ACK 1
#define STATUS_REG_W 0x06
#define STATUS_REG_R 0x07
#define MEASURE_TEMP 0x03
#define MEASURE_HUMI 0x05
#define RESET 0x1e

enum {TEMP,HUMI};   //TEMP=0; HUMI=1


#define SCKL GPIOB->ODR&=~(1<<10)   //ʱ��������   PB10
#define SCKH GPIOB->ODR|=1<<10      //ʱ��������
#define SDAL GPIOB->ODR&=~(1<<11)   //����������   PB11
#define SDAH GPIOB->ODR|=1<<11      //����������

#define ReadState()  {GPIOB->MODER&=~(3<<22); GPIOB->PUPDR|=(1<<22);}   //����������Ϊ���룬����ģʽ
#define WriteState() {GPIOB->MODER&=~(3<<22); GPIOB->MODER|=(1<<22); GPIOB->OTYPER&=~(1<<11); GPIOB->OSPEEDR&=~(3<<22);GPIOB->OSPEEDR|=(2<<22);} 
#define DATA() PBin(11)           //��ȡ �������ϵ�����


void SHT_GPIO_Config(void);

void ConnectionReset(void);

void measure_wsd(void);

#endif
