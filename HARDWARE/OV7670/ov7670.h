	


#ifndef _OV7670_H
#define _OV7670_H
#include "stm32f10x.h"
#include "sccb.h"


#define OV7670_VSYNC  PAin(3)				//同步信号检测IO
#define OV7670_HREF		PAin(2)
#define	OV7670_PCLK		PDin(2)
#define OV7670_CS		PAout(1)  		//示波器看时间用
	

#define XCLK_H	        GPIOA->BSRR =  GPIO_Pin_8;
#define XCLK_L		    GPIOA->BRR =   GPIO_Pin_8;

//GPIOC->IDR&0x00FF 
/////////////////////////////////////////
#define CHANGE_REG_NUM 							171			//需要配置的寄存器总数		  
extern const u8 ov7670_init_reg_tbl[CHANGE_REG_NUM][2];		//寄存器及其配置表

void CLK_init_ON(void);
void CLK_init_OFF(void);	    				 
u8   OV7670_Init(void);		  	   		 
void OV7670_Light_Mode(u8 mode);
void OV7670_Color_Saturation(u8 sat);
void OV7670_Brightness(u8 bright);
void OV7670_Contrast(u8 contrast);
void OV7670_Special_Effects(u8 eft);
void OV7670_Window_Set(u16 sx,u16 sy,u16 width,u16 height);
void EXTI2_Init(void);

#endif





















