#ifndef __SPI_H_
#define __SPI_H_

#include "sys.h"


#define SCLK  PAout(5)
#define DIN   PAin(6)


void SPI1_Init(void);
//u8 SPI1_Read_Data(void);
//u32 SPI1_Read_Data(void);
void SPI1_Read_Data(u32 *ad_val);



#endif
