#ifndef __GPIO_H_
#define __GPIO_H_


#include "sys.h"

#define BEEP	PAout(11)
#define LED0  PAout(12)

#define SYNC  PAout(7)


#define On_JDQ_200V()		{GPIOF->ODR = 0x80;}

#define On_JDQ_100V()		{GPIOF->ODR = 0x40;}

#define On_JDQ_50V()		{GPIOF->ODR = 0x20;}

#define On_JDQ_20V()		{GPIOF->ODR = 0x10;}

#define On_JDQ_10V()		{GPIOF->ODR = 0x08;}

#define On_JDQ_5V()			{GPIOF->ODR = 0x04;}

#define On_JDQ_1V()			{GPIOF->ODR = 0x02;}


void System_GPIO_Init(void);

void Beep_hz(u32 f_hz,u32 time);






#endif 
