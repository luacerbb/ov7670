

/* http://zq281598585.taobao.com/  
�������	  ELH    enlighten  sunny

OV7670ģ�� STM32����

ѡ��STM32оƬ   
TFT����IC��֧�� 9325 9320 9328 ��9341

  ������ �õ���TFT Ϊ ���ǵ�� 2.4��TFT  ����оƬΪ R61505V	
  TFT�õ���GPIO  PBΪ���ݿ�	PC.6  CS   PC.7  RS	  PC.8  WR   PC.9  RD	PC.10 RES
  �˺��� �üĴ�����ʽ����GPIO  ��TFT������ ������������õ��ǼĴ���  �����ܴ�����������ٶ�
 
����ͷ����
����ͷ ���ݿ� ΪPA��  AO-A7
SCL    PC11
SDA    PC12
OE	   PC3		 //Ƭѡ�ź�(OE)
WRST   PC4		 //дָ�븴λ
RRST   PC2		 //��ָ�븴λ
RCLK   PC1		 //������ʱ��
WEN	   PC5		 //д��FIFOʹ��
VSY	   PD2  	 //ͬ���źż��IO

�����������̽���

��ʼ��TFT   
��ʼ��ov7670  ov7670��ͨ��SCCB ��ʼ����  �� SCL ��SDA�Ϳ��Գ�ʼ��OV7670  SDA SCL������I2CЧ��
              �����ڵ�ģ��ǰ һ��Ҫȷ��OV7670�Ƿ��Ѿ���ʼ��
�����ж� �ж�֡�����Ƿ����
��ѭ�� ��ȡ����ʾ ��������

	   
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
//��ȡ�ķ�Χ
u16 gray_bx=0;
u16 gray_by=0;
u16 gray_w=300;//256;//320;//256;
u16 gray_h=52;

//��Чλ��
u8 gray_n=8;//��ȡ1��2��4��6��8
//1(8�ֽ�ת1�ֽ�),2(4�ֽ�ת1�ֽ�),4(2�ֽ�ת1�ֽ�),6(4�ֽ�ת3�ֽ�),8(��ѹ��)
//���Կ��Ҫ����������gray_nΪ1ʱ���Ҫ8�ı����������80X200
u32 gray_cnd=0;
u16 gray_ex=0;
u16 gray_ey=0;
u8 gray_mask=0;
u8 gray_shift=0;
u32 grayvalue=0;
u8 shiftcnd=0;
u8 colorsdata[16000]={0};//����������
u32 sdata_cnd=0;
u8 posflag=0;
u16 drow[320+1]={0};//��һ������
u16 drowt[320+1]={0};//��һ��������ֵ
u8 gao=0,di=0;
u16 color=0;
u8 r,g,b=0;
u8 Y,U,V=0;

extern u8 ov_sta;	//֡���� ��λ��־λ

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
		
	pixel[0] = ( ((g & 0x1C) << 3) | ( b >> 3) );  //��  
	pixel[1] = ( (r & 0xF8) | ( g >> 5) );     //��
	return pixel32;  
} 

void Demo_Init(void)
{
//		u8 lightmode=0,saturation=2,brightness=2,contrast=2;
//		u8 effect=0;
	
		delay_init();	    	 //��ʱ������ʼ��
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	  LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ�			
	  Lcd_Init();			 //LCD  ��ʼ��
	  TFT_CS(0);			 //��LCDƬѡʹ�� 
    GUI_Clear(White);		//����
		uart_init(115200);	 //���ڳ�ʼ��Ϊ115200  

		printf("\r\n������� STM32�ϵ�a \r\n\r\n");		
	 

	  while(OV7670_Init())//��ʼ��OV7670
		{
			GUI_sprintf_hzstr16x(20,150,"OV7670 Error!!",Blue,White);
			delay_ms(200);
			GUI_sprintf_hzstr16x(20,150,"              ",Blue,White);
			delay_ms(200);
		}

	  GUI_Clear(Red);		//����		��ʼ���ɹ��� ��ɫ���� 

		OV7670_Window_Set(10,174,240,320);	//���ô���


		TFT_SCAN(5);   //����TFTɨ�跽�� Ϊ���µ��� ���ҵ���
	 //����˵��һ��  ����ͷ�ĳ���ɨ���� ���ϵ��� ������

	                                         //���������÷��� ����Ҫ���ֲ�

		Address_set(0,0,239,319,0,0);		   //9325 ������ʾ��Χ		ȫ����ʾ
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
		send_byte(gray_w>>1);	// ��2	
		send_byte(gray_h>>1);	// ��2
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
	
		gray_cnd--;//�ȼ�һ������for����ļ�����
					
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
	TFT_RS_1;		 //д����������		Ϊ���д���ٶ�		
	
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
					
					//OV7670_CS=0;//ʾ������ʱ����
					
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
								drowt[gray_cnd++]=drow[pixcnt];//����ֵ
							}
						}
					}
					if(gray_cnd>0)
					{
						process_data();
					}
					
					//OV7670_CS=1;//ʾ������ʱ����
					
				}	
				
			#if 1
				while(OV7670_VSYNC==0);//0-1
				while(OV7670_VSYNC==1);//1-0		
				//��������,����Ҫռ����֡��ʱ�䷢��,��ȡһ֡����
				send_data();
			#endif
				
		}	
						
 }

