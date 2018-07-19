#ifndef __ADS1274_H_

#define __ADS1274_H_


#include "sys.h"


#define ReadAdcChx		2        //需要读取ADS1274几个通道的数据

#define SYNC 	PAout(7)


void ADS1274_Init(void);
void DclkSource(void);
void ADS1274_ReadNowAdc(void);

//u32 ADS1274_ReadNowAdc(void);


#endif 
