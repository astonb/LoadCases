#ifndef __ADS1274_H_

#define __ADS1274_H_


#include "sys.h"


#define ReadAdcChx		2        //��Ҫ��ȡADS1274����ͨ��������

#define SYNC 	PAout(7)


void ADS1274_Init(void);
void DclkSource(void);
void ADS1274_ReadNowAdc(void);

//u32 ADS1274_ReadNowAdc(void);


#endif 
