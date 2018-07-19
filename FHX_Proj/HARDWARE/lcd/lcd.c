#include "lcd.h"
#include "sys.h"
#include "ascii.h"
#include "delay.h"
//#include "ChineseLib.h"


typedef struct
{
	u8 REG_Index;
	u8 REG_Value;
}FSA506_REG_Setting;

//FSMC 驱动模式下：RS 选择
#ifdef Dervice_Mode_FSMC
typedef struct
{
	u8 CMD;
	u8 DATA;
}LCD_TypeDef;

#define LCD_BASE	((unsigned int)(0x6C000000 | 0x000003FF))
#define LCD				((LCD_TypeDef *) LCD_BASE)

#endif

#ifdef Landscap 	/*end Dervice_FSMC*/

static FSA506_REG_Setting FSA506_A[]=
{
	{0x40,0x12},
	{0x41,R41},
	{0x42,R42},
	{0x08,(u8)(Resolution_X>>8)},
	{0x09,(u8)(Resolution_X)},
	{0x0a,0x00},
	{0x0b,0x00},
	{0x0c,0x00},
	{0x10,0x0C|R10_B10},
	//{0x10,0x0C|0x02},
	{0x11,0x05},
	{0x12,0x00},
	{0x13,0x00},
	{0x14,(u8)(H_Sync_Pluse_Wide>>8)},
	{0x15,(u8)(H_Sync_Pluse_Wide)},
	{0x16,(u8)(H_Sync_to_DE>>8)},
	{0x17,(u8)(H_Sync_to_DE)},
	{0x18,(u8)(Resolution_X>>8)},
	{0x19,(u8)(Resolution_X)},
	{0x1a,(u8)(H_Sync_total>>8)},
	{0x1b,(u8)(H_Sync_total)},
	{0x1c,0x00},
	{0x1d,0x00},
	{0x1e,(u8)(V_Sync_Pluse_Wide>>8)},
	{0x1f,(u8)(V_Sync_Pluse_Wide)},
	{0x20,(u8)(V_Sync_to_DE>>8)},
	{0x21,(u8)(V_Sync_to_DE)},
	{0x22,(u8)(Resolution_Y>>8)},
	{0x23,(u8)(Resolution_Y)},
	{0x24,(u8)(V_Sync_total>>8)},
	{0x25,(u8)(V_Sync_total)},
	{0x26,0x00},
	{0x27,0x00},
	{0x28,0x00},
	{0x29,0x01}, //bit0: Load output timing related setting (配置参数) to take effect
	{0x2d,LCD_DCLK_Latch|0x08},  // TFTLCD POWER IS ON , The RGB out put data are on the Rising edge of the DCLK.
	{0x30,0x00},
	{0x31,0x00},
	{0x32,0x00},
	{0x33,0x00},
	{0x34,(u8)(Resolution_X>>8)},
	{0x35,(u8)(Resolution_X)},
	{0x36,(u8)((2*Resolution_Y)>>8)},
	{0x37,(u8)(2*Resolution_Y)},
};

#endif
					
#ifdef Dervice_Mode_GPIO

void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//GPIOC0~7 : D0~DB7
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	RS =1;
	CS =1;
	RD =1;
	WR =1;
}

void AMP506_80Mode_Command_SendAddress(u8 Addr)
{
	RD = 1;
	RS = 0;
	
	//Data
	GPIOC->ODR = Addr;
	NOP();
	
	CS =0;
	WR =0;
	
	NOP();NOP();NOP();
	
	WR =1;
	RS =1;
	CS =1;
}

void AMP506_80Mode_Command_SendData(u8 Data)
{
	RD =1;
	RS =1;
	
	GPIOC->ODR = Data;
	NOP();
	
	CS =0;
	WR =0;
	
	NOP();NOP();NOP();
	WR =1;
	RS =1;
	CS =1;
}

void AMP506_80Mode_16Bit_Memory_SendData(u16 Dat16bit)
{
	GPIOC->ODR = Dat16bit>>8;
	NOP();NOP();

	RD =1;
	RS =1;
	
	CS =0;
	WR =0;
	
	NOP();NOP();NOP();
	
	WR =1;
	CS =1;
	
	GPIOC->ODR=(u8)Dat16bit;
	NOP();NOP();
	
	RD =1;
	RS =1;
	
	CS =0;
	WR =0;
	
	NOP();NOP();NOP();
	
	WR =1;
	CS =1;
}
#endif   /*end  Dervice_Mode_GPIO */

#ifdef Dervice_Mode_FSMC
void LCD_FSMC_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
		FSMC_NORSRAMTimingInitTypeDef  timing; 
			
		RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOG,ENABLE);

		//PORTD
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;	//	//PORTD¸
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
		GPIO_Init(GPIOD, &GPIO_InitStructure); 
	
		//PORTE
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD¸´ÓÃÍÆÍìÊä³ö  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOE, &GPIO_InitStructure);   
		
		//  PG0-A10  PG10-NE3
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_10;	 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		
		GPIO_Init(GPIOG, &GPIO_InitStructure); 
		
		
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);//D2
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);//D3
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);//NOE
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);//NWE
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);//D13
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);//D14
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);//D15
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);//D0
		GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);//D1
		
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);//D4
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);//D5
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);//D6
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);//D7
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);//D8
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);//D9
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);//D10
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);//D11
		GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//D12
 
		GPIO_PinAFConfig(GPIOG,GPIO_PinSource0,GPIO_AF_FSMC);//A10
		GPIO_PinAFConfig(GPIOG,GPIO_PinSource10,GPIO_AF_FSMC);//NE3
		

		timing.FSMC_AddressSetupTime = 3; 	           			//地址建立时间ADDSET
		timing.FSMC_AddressHoldTime  = 0;                		//地址保持时间DATSET
		timing.FSMC_DataSetupTime    =	6;          				//DATAST    		
		timing.FSMC_BusTurnAroundDuration= 0;																																					
		timing.FSMC_CLKDivision = 0;									
		timing.FSMC_DataLatency = 0;													
		timing.FSMC_AccessMode = FSMC_AccessMode_A;	 
		
		FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;  											//使用NE3,对应BTCT[4]	[5]																	
		FSMC_NORSRAMInitStructure.FSMC_DataAddressMux  = FSMC_DataAddressMux_Disable;			//不复用地址线
		FSMC_NORSRAMInitStructure.FSMC_MemoryType      = FSMC_MemoryType_SRAM;
		FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
		FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; 	 //突发访问模式
		FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
		FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait   = FSMC_AsynchronousWait_Disable;	//不使用异步等待信号
		FSMC_NORSRAMInitStructure.FSMC_WrapMode  = FSMC_WrapMode_Disable;    								//不使用回环模式
		FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState; //设置WAIT信号的有效时机
		FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;  						//存储器写使能
		FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   								//WAIT信号不使能
		FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;  						//读写时序不同设置				
		FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
		
		FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &timing;       
		FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &timing;
		
		FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  
		
		/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */ 
		FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4,ENABLE);  			//使能BANK1
}

void AMP506_80Mode_Command_SendAddress(u8 Addr)
{
	LCD->CMD = Addr;
}

void AMP506_80Mode_Command_SendData(u8 Data)
{
	LCD->DATA = Data;
}

void AMP506_80Mode_16Bit_Memory_SendData(u16 Dat16bit)
{
	LCD->DATA = Dat16bit>>8;
	LCD->DATA = (u8)Dat16bit;
}

#endif /*end Dervice_Mode_FSMC*/

void AMP506_Command_Write(u8 CMD_Address,u8 CMD_Value)
{
	
	AMP506_80Mode_Command_SendAddress(CMD_Address);
	AMP506_80Mode_Command_SendData(CMD_Value);
}

void LCD_Init(void)
{
	u8 i;
	
	for(i=0;i<(sizeof(FSA506_A)/sizeof(FSA506_A[0]));i++)
	{
		AMP506_Command_Write(FSA506_A[i].REG_Index,FSA506_A[i].REG_Value);
	}
}

//设置窗口大小
void AMP506_WindowSet(u16 S_X,u16 S_Y,u16 E_X,u16 E_Y)
{
	AMP506_80Mode_Command_SendAddress(0x00);
	AMP506_80Mode_Command_SendData(S_X>>8);
	AMP506_80Mode_Command_SendData((u8)S_X);
	AMP506_80Mode_Command_SendData((E_X-1)>>8);
	AMP506_80Mode_Command_SendData((u8)(E_X-1));
	AMP506_80Mode_Command_SendData(S_Y>>8);
	AMP506_80Mode_Command_SendData((u8)S_Y);
	AMP506_80Mode_Command_SendData((E_Y-1)>>8);
	AMP506_80Mode_Command_SendData((u8)(E_Y-1));
}
//Display RAM  write enable
void AMP506_DispRAM_WriteEnable(void)
{
	AMP506_80Mode_Command_SendAddress(_DisplayRAM_WriteEnable);
}
//Display RAM  write disable
void AMP506_DispRAM_WriteDisable(void)
{
	AMP506_80Mode_Command_SendAddress(_DisplayRAM_WriteDisable);
}

/**************FSA506 Set Start & End area function *************************/
void GUI_RectangleFill(u32 x0,u32 y0,u32 x1,u32 y1,u16 color)
{
	u32 k,l;
	
	AMP506_WindowSet(x0,y0,x1,y1);  //该函数结束，地址在0x07,之后写操作起始与0x08
	
	AMP506_DispRAM_WriteEnable();  //打开显存写使能
	
	for(k=y0;k<y1;k++)
	{
		for(l=x0;l<x1;l++)
		{
			AMP506_80Mode_16Bit_Memory_SendData(color);
		}
	}
	AMP506_DispRAM_WriteDisable();  //关闭显存写
}

/**************Full Display function *************************/
void Full_LCD(u16 Dat16bit)
{
	GUI_RectangleFill(0,0,Resolution_X,Resolution_Y,Dat16bit);
}

void LCD_Pixel(u16 x,u16 y,u16 color)
{	
	AMP506_80Mode_Command_SendAddress(0x00); 	//地址递增
	AMP506_80Mode_Command_SendData((x)>>8);   //0x00 MSB_S_X
	AMP506_80Mode_Command_SendData((u8)x);		//0x01 LSB_S_X
	
	AMP506_80Mode_Command_SendData((x)>>8);		//0x02 MSB_E_X
	AMP506_80Mode_Command_SendData((u8)x);		//0x03 LSB_E_X
	
	AMP506_80Mode_Command_SendData(y>>8);			//0x04 MSB_S_Y
	AMP506_80Mode_Command_SendData((u8)y);		//0x05 LSB_S_Y
	
	AMP506_80Mode_Command_SendData((y)>>8);		//0x06 MSB_E_Y
	AMP506_80Mode_Command_SendData((u8)y);		//0x07 LSB_E_Y
	
	
	AMP506_DispRAM_WriteEnable();
	
	AMP506_80Mode_16Bit_Memory_SendData(color);
	
	AMP506_DispRAM_WriteDisable();
	
}

/*
void LCD_DrawLine(u16 x,u16 y,u16 x1,u16 y1,u16 color)
{
	u8 i,j;
	for(j=y;j<=y1;j++)
	{
		for(i=x;i<=x1;i++)
		{
			LCD_Pixel(i,j,color);
		}
	}
}
*/

// glib库中的画线函数，可以画斜线，线两端分别是(x1, y1)和(x2, y2)
void glib_line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color)
{
	int dx,dy,e;
	dx=x2-x1; 
	dy=y2-y1;
    
	if(dx>=0)
	{
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 1/8 octant
			{
				e=dy-dx/2;  
				while(x1<=x2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 2/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 8/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else	 // 7/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
	else //dx<0
	{
		dx=-dx;		//dx=abs(dx)
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 4/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 3/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 5/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 6/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					LCD_Pixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
}

//画圆函数，圆心坐标是(centerX, centerY)，半径是radius，圆的颜色是color
void draw_circular(unsigned short centerX, unsigned short centerY, unsigned short radius, unsigned short color)
{
	int x,y ;
	int tempX,tempY;
  int SquareOfR = radius*radius;
	for(y=0; y<XSIZE; y++)
	{
		for(x=0; x<YSIZE; x++)
		{
			if(y<=centerY && x<=centerX)
			{
				tempY=centerY-y;
				tempX=centerX-x;                        
			}
			else if(y<=centerY&& x>=centerX)
			{
				tempY=centerY-y;
				tempX=x-centerX;                        
			}
			else if(y>=centerY&& x<=centerX)
			{
				tempY=y-centerY;
				tempX=centerX-x;                        
			}
			else
			{
				tempY = y-centerY;
				tempX = x-centerX;
			}
			if ((tempY*tempY+tempX*tempX)<=SquareOfR)
				LCD_Pixel(x, y, color);
		}
	}
}
//写汉字
void LCD_Draw_Cinese1616(unsigned short x, unsigned short y, unsigned short color,const unsigned char *data)  
{  
// count记录当前正在绘制的像素的次序
    int i, j, count = 0;  
	  
    for (j=y; j<(y+16); j++)  
    {  
        for (i=x; i<(x+16); i++)  
        {  
            if (i<XSIZE && j<YSIZE)  
            {  
			// 在坐标(i, j)这个像素处判断是0还是1，如果是1写color；如果是0直接跳过
            	if ((data[count/8]) & (1<<(count%8)))   //chinese_lib[0]
								LCD_Pixel(i, j, color);
            }  
            count++;  
        }  
    }  
} 

// 写字的左上角坐标(x, y)，字的颜色是color，字的字模信息存储在data中
static void show_8_16(unsigned short x, unsigned short y, unsigned short color, unsigned char *data)  
{  
// count记录当前正在绘制的像素的次序
    int i, j, count = 0;  
	  
    for (j=y; j<(y+16); j++)  
    {  
        for (i=x; i<(x+8); i++)  
        {  
            if (i<XSIZE && j<YSIZE)  
            {  
			// 在坐标(i, j)这个像素处判断是0还是1，如果是1写color；如果是0直接跳过
            	if (data[count/8] & (1<<(count%8)))   
								LCD_Pixel(i, j, color);
            }  
            count++;  
        }  
    }  
} 
//显示字符串
void draw_ascii_ok(unsigned short x, unsigned short y, unsigned short color, unsigned char *str)
{
	int i;  
	unsigned char *ch;
	
  for (i=0; str[i]!='\0'; i++)  
  {  
		ch = (unsigned char *)ascii_8_16[(unsigned char)str[i]-0x20];
    show_8_16(x, y, color, ch); 
		
    x += 8;
		if (x >= XSIZE)
		{
			x -= XSIZE;			// 回车
			y += 16;			// 换行
		}
  }  
}
//Display picture
void lcd_draw_picture(const unsigned char *pData)
{
	unsigned short x,y;
	unsigned short color;
	unsigned int n=0;
	for(y=0;y<YSIZE;y++)
	{
		for(x=0;x<XSIZE;x++)
		{
			color = ((pData[n]) | (pData[n+1]<<8));
			LCD_Pixel(x,y,color);
			n+=2;
		}	
	}
	
}
