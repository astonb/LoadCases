#ifndef _LCD_H_

#define _LCD_H_

#include "sys.h"

/***Çý¶¯Ä£Ê½Ñ¡Ôñ£º GPIO or FSMC**************/
//#define Dervice_Mode_GPIO	
#define Dervice_Mode_FSMC
/*******************************************/
//Define lcd interface
#define Mode80		
//Define BUS wide ->8bit
#define C80_8B
//Define Landscap/Portait  
#define Landscap
//Define Resolution
#ifdef Landscap
#define Resolution_X	320
#define Resolution_Y	240
#endif

#define Rising   (0<<2)
#define Falling  (1<<2)
//Select DCLK Frequency  MHZ 
#define LCD_DCLK	10    

#define LCD_DCLK_Latch	Rising

#define H_Sync_Pluse_Wide  10	//Hsync  Pluse  wide
#define H_Sync_to_DE 	68		//DE  horizontal start position
#define H_Sync_total 	440		//horizontal totao
#define V_Sync_Pluse_Wide	8	//Vsync pluse wide
#define V_Sync_to_DE	16		//DE vertical position
#define V_Sync_total	265		//vertical total

#if LCD_DCLK==10
	#define R41      1
	#define R42      2
	#define R10_B10  2
#endif

#define _DisplayRAM_WriteEnable		0xc1
#define _DisplayRAM_WriteDisable	0x80

#define XSIZE			Resolution_X
#define YSIZE			Resolution_Y


#define RS	PBout(5)
#define CS	PBout(6)
#define RD	PBout(7)
#define WR	PBout(8)

#define NOP() __asm{NOP}


void LCD_Init(void);

#ifdef Dervice_Mode_GPIO
void LCD_GPIO_Init(void);
#else 
void LCD_FSMC_Init(void);
#endif

void AMP506_80Mode_Command_SendAddress(u8 Addr);
void AMP506_80Mode_Command_SendData(u8 Data);
void AMP506_80Mode_8Bit_Memory_SendData(u8 Dat8bit);
void AMP506_Command_Write(u8 CMD_Address,u8 CMD_Value);
void AMP506_WindowSet(u16 S_x,u16 S_Y,u16 end_X,u16 end_Y);
void AMP506_DispRAM_WriteEnable(void);
void AMP506_DispRAM_WriteDisable(void);
void GUI_RectangleFill(u32 x0,u32 y0,u32 x1,u32 y1,u16 color);
void Full_LCD(u16 Dat16bit);
void LCD_Pixel(u16 x,u16 y,u16 color);


//void LCD_DrawLine(u16 x,u16 y,u16 x1,u16 y1,u16 color);
//»­Ïß
void glib_line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color);
//»­Ô²
void draw_circular(unsigned short centerX, unsigned short centerY, unsigned short radius, unsigned short color);

//Ð´×ÖÄ¸
void draw_ascii_ok(unsigned short x, unsigned short y, unsigned short color, unsigned char *str);
//dispaly picture
void lcd_draw_picture(const unsigned char *pData);
void LCD_Draw_Cinese1616(unsigned short x, unsigned short y, unsigned short color,const unsigned char *data);
#endif

