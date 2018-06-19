

/* http://zq281598585.taobao.com/  
启光电子	  ELH    enlighten  sunny

OV7670模块 STM32函数

选用STM32芯片   
TFT驱动IC可支持 9325 9320 9328 及9341

  程序中 用到的TFT 为 我们店的 2.4寸TFT  驱动芯片为 R61505V	
  TFT用到的GPIO  PB为数据口	PC.6  CS   PC.7  RS	  PC.8  WR   PC.9  RD	PC.10 RES
  此函数 用寄存器方式配置GPIO  在TFT的数据 命令脚配置上用的是寄存器  这样能大幅度提高输出速度
 
摄像头方面
摄像头 数据口 为PA口  AO-A7
SCL    PC11
SDA    PC12
OE	   PC3		 //片选信号(OE)
WRST   PC4		 //写指针复位
RRST   PC2		 //读指针复位
RCLK   PC1		 //读数据时钟
WEN	   PC5		 //写入FIFO使能
VSY	   PD2  	 //同步信号检测IO

整个程序流程介绍

初始化TFT   
初始化ov7670  ov7670是通过SCCB 初始化的  即 SCL 和SDA就可以初始化OV7670  SDA SCL类似于I2C效果
              所以在调模块前 一定要确定OV7670是否已经初始化
开启中断 判断帧数据是否接收
死循环 读取并显示 摄像数据

	   
*/

#include  "stm32f10x.h"
#include  "delay.h"
#include  "led.h"
#include  "tft.h"
#include  "led.h"
#include "ov7670.h"
#include  "gui.h"
#include  "usart.h"

ErrorStatus HSEStartUpStatus;
//截取的范围
u16 gray_bx=0;
u16 gray_by=0;
u16 gray_w=300;//256;//320;//256;
u16 gray_h=52;

//有效位数
u8 gray_n=8;//可取1，2，4，6，8
//1(8字节转1字节),2(4字节转1字节),4(2字节转1字节),6(4字节转3字节),8(不压缩)
//所以宽度要满足条件，gray_n为1时宽度要8的倍数，如可以80X200
u32 gray_cnd=0;
u16 gray_ex=0;
u16 gray_ey=0;
u8 gray_mask=0;
u8 gray_shift=0;
u32 grayvalue=0;
u8 shiftcnd=0;
u8 colorsdata[16000]={0};//存的最大数据
u32 sdata_cnd=0;
u8 posflag=0;
u16 drow[320+1]={0};//存一行数据
u16 drowt[320+1]={0};//存一行中有用值
u8 gao=0,di=0;
u16 color=0;
u8 r,g,b=0;
u8 Y,U,V=0;

extern u8 ov_sta;	//帧次数 置位标志位

u16 convert_yuv_to_rgb_pixel(u8 Y, u8 U, u8 V)  
{  
	u16 pixel32 = 0;  
	unsigned char *pixel = (unsigned char *)&pixel32;  
	int r, g, b;  
	int y, u, v;  

//if(Y>128)	Y=255;
//if(Y<=128)	Y=0;
	
	
  U=128;//U=0;
  V=128;
	
	
	v=V-128;
	u=U-128;
	y=Y;
	r=y+ v+ (v*103>>8);
	g=y- (u*88>>8) - (v*183>>8);
	b=y+ u + (u*198>>8); 
	if(r > 255) r = 255;  
	if(g > 255) g = 255;  
	if(b > 255) b = 255;  
	if(r < 0) r = 0;  
	if(g < 0) g = 0;  
	if(b < 0) b = 0;  
		
	pixel[0] = ( ((g & 0x1C) << 3) | ( b >> 3) );  //低  
	pixel[1] = ( (r & 0xF8) | ( g >> 5) );     //高
	return pixel32;  
} 

void Demo_Init(void)
{
//		u8 lightmode=0,saturation=2,brightness=2,contrast=2;
//		u8 effect=0;
	
		delay_init();	    	 //延时函数初始化
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	  LED_Init();		  	 //初始化与LED连接的硬件接口			
	  Lcd_Init();			 //LCD  初始化
	  TFT_CS(0);			 //打开LCD片选使能 
    GUI_Clear(White);		//清屏
		uart_init(115200);	 //串口初始化为115200  

		printf("\r\n启光电子 STM32紫电a \r\n\r\n");		
	 

	  while(OV7670_Init())//初始化OV7670
		{
			GUI_sprintf_hzstr16x(20,150,"OV7670 Error!!",Blue,White);
			delay_ms(200);
			GUI_sprintf_hzstr16x(20,150,"              ",Blue,White);
			delay_ms(200);
		}

	  GUI_Clear(Red);		//清屏		初始化成功后 红色清屏 

		OV7670_Window_Set(10,174,240,320);	//设置窗口


		TFT_SCAN(5);   //设置TFT扫描方向 为从下到上 从右到左
	 //这里说明一下  摄像头的成像扫描是 从上到下 从左到右

	                                         //其他屏设置方法 具体要看手册

		Address_set(0,0,239,319,0,0);		   //9325 设置显示范围		全屏显示
}

void send_byte(u8 d)
{
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		USART_SendData(USART1,d);
}

void send_data(void)
{
		u32 i;
	
		send_byte(0xff);
		send_byte(0xaa);		
		send_byte(0x55);		
		send_byte(gray_w>>1);	// 除2	
		send_byte(gray_h>>1);	// 除2
		send_byte(gray_n);		
	
		for(i=0;i<sdata_cnd;i++)		
		{
				send_byte(colorsdata[i]);						
		}
}

void set_mask(void)
{
		switch (gray_n)
		{
		case 1:
				gray_mask=0x80;
				break;
		
		case 2:
				gray_mask=0xc0;
				break;

		case 4:
				gray_mask=0xf0;
				break;

		case 6:
				gray_mask=0xfc;
				break;

		case 8:
		default:
				gray_mask=0xff;
				break;
		}	
}

void process_data(void)
{
		u32 i=0;	
	
		gray_cnd--;//先减一，减少for里面的计算量
					
		for(i=0;i<=gray_cnd;i++)
		{
			di=drowt[i]&0xff;
			gao=(drowt[i]&0xff00) >> 8;
			r= (gao & 0xF8);
			g= (gao << 5) | ((di & 0xE0) >> 3);
			b=  di << 3;
			
			Y = (1*r + 150*g + 29*b)>>8;//Y = (r + g + b)/3;//Y = (77*r + 150*g + 29*b)>>8;

			{
					u32 temp;
				
					temp = Y & gray_mask;
					shiftcnd++;
					grayvalue|=temp<<24;			
				
				#if 1	
					switch(gray_n)
					{

					case 1:
					case 2:
					case 4:
							if(shiftcnd==32/gray_n)
							{
									colorsdata[sdata_cnd++]=grayvalue&0xff;
									colorsdata[sdata_cnd++]=(grayvalue&0xff00)>>8;
									colorsdata[sdata_cnd++]=(grayvalue&0xff0000)>>16;
									colorsdata[sdata_cnd++]=(grayvalue&0xff000000)>>24;
									grayvalue=0;
									shiftcnd=0;
							}
							else
							{
									grayvalue>>=gray_n;
							}
							break;

					case 6:
							if(shiftcnd==4)
							{
									colorsdata[sdata_cnd++]=(grayvalue&0xff00)>>8;
									colorsdata[sdata_cnd++]=(grayvalue&0xff0000)>>16;
									colorsdata[sdata_cnd++]=(grayvalue&0xff000000)>>24;
									grayvalue=0;
									shiftcnd=0;
							}
							else
							{
									grayvalue>>=6;
							}			
							break;

					case 8:
					default:
							colorsdata[sdata_cnd++]=Y;
							break;
					}					
					#endif
			}

		}
}

int main(void)
{

	u16 pixcnt=0;				//????
	u16 linecnt=0;				//????

	
	Demo_Init();
	TFT_RS_1;		 //写数据线拉高		为提高写入速度		
	
	gray_ex=gray_bx+gray_w-1;
	gray_ey=gray_by+gray_h-1;
	set_mask();
	
    while(1) 
    {
				while(OV7670_VSYNC==0);//0-1
				while(OV7670_VSYNC==1);//1-0		???VSYNC???,?????	 
				gray_cnd=0;
				grayvalue=0;
				shiftcnd=0;
				sdata_cnd=0;
			
				for(linecnt=0;linecnt<240;linecnt++)		
				{

					while(OV7670_HREF==0);
					
					for(pixcnt=0;pixcnt<320;pixcnt++)
					{
						while(OV7670_PCLK==1);
						//OV7670_CS=0;
						drow[pixcnt]=GPIOC->IDR&0XFF; 
						while(OV7670_PCLK==0); 
						//OV7670_CS=1;
						drow[pixcnt]<<=8;
									
						while(OV7670_PCLK==1);  
						//OV7670_CS=0;
						drow[pixcnt]|=GPIOC->IDR&0XFF; 
						while(OV7670_PCLK==0);	
						//OV7670_CS=1;
					} 
					while(OV7670_HREF==1);
					
					//OV7670_CS=0;//示波器看时间用
					
					gray_cnd=0;
					for(pixcnt=0;pixcnt<320;pixcnt++)
					{
						GPIOB->ODR=drow[pixcnt];
						TFT_WR_0;		 
						TFT_WR_1;						
						if((linecnt>=gray_by) && (linecnt<=gray_ey))
						{
							if((pixcnt>=gray_bx) && (pixcnt<=gray_ex))
							{
								drowt[gray_cnd++]=drow[pixcnt];//有用值
							}
						}
					}
					if(gray_cnd>0)
					{
						process_data();
					}
					
					//OV7670_CS=1;//示波器看时间用
					
				}	
				
			#if 1
				while(OV7670_VSYNC==0);//0-1
				while(OV7670_VSYNC==1);//1-0		
				//发送数据,可能要占用两帧的时间发完,争取一帧发完
				send_data();
			#endif
				
		}	
						
 }

