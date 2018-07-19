#ifndef _RTC_H__
#define _RTC_H__

#include "sys.h" 

typedef struct
{
	u8 year;
	u8 month;
	u8 date;
	u8 week;
	u8 hour;
	u8 min;
	u8 sec;
	u8 ampm;
}MyRTC;


u8 My_RTC_Init(void);
void usr_set_dat_time(MyRTC *myRTC);

#endif


