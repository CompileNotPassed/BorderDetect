/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		main
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.24
 * @Target core		NXP RT1064DVL6A
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-04-30
 * @note
					接线定义：
					------------------------------------ 
						1.8TFT模块管脚      单片机管脚
						SCL                 查看SEEKFREE_18TFT.H文件中的TFT_SCL  	宏定义     硬件SPI引脚不可随意切换
						SDA                 查看SEEKFREE_18TFT.H文件中的TFT_SDA		宏定义     硬件SPI引脚不可随意切换
						RES                 查看SEEKFREE_18TFT.H文件中的REST_PIN	宏定义   
						DC                  查看SEEKFREE_18TFT.H文件中的DC_PIN		宏定义 
						CS                 	查看SEEKFREE_18TFT.H文件中的TFT_CS		宏定义     硬件SPI引脚不可随意切换
						
						电源引脚
						BL  3.3V电源（背光控制引脚，也可以接PWM来控制亮度）
						VCC 3.3V电源
						GND 电源地
						最大分辨率128*160
					------------------------------------ 
						总钻风模块管脚      单片机管脚
						SDA(51的RX)         查看SEEKFREE_MT9V03X_CSI.h文件中的MT9V03X_CSI_COF_UART_TX	宏定义
						SCL(51的TX)         查看SEEKFREE_MT9V03X_CSI.h文件中的MT9V03X_CSI_COF_UART_RX	宏定义
						场中断(VSY)         查看SEEKFREE_MT9V03X_CSI.h文件中的MT9V03X_CSI_VSYNC_PIN		宏定义
						行中断(HREF)	    不需要使用
						像素中断(PCLK)      查看SEEKFREE_MT9V03X_CSI.h文件中的MT9V03X_CSI_PCLK_PIN		宏定义
						数据口(D0-D7)		B31-B24 B31对应摄像头接口D0
					------------------------------------ 
	
						默认分辨率是        188*120
						默认FPS             50帧
 ********************************************************************************************************************/


//整套推荐IO查看Projecct文件夹下的TXT文本



//打开新的工程或者工程移动了位置务必执行以下操作
//第一步 关闭上面所有打开的文件
//第二步 project  clean  等待下方进度条走完



#include "headfile.h"
int i,j;
uint8 frame[128][160];

//Frame Process
void picGamma(uint8 in_array[][168], uint8 out_array[][168], long height, long width)
{
    for (int i = 0; i < height; i++){
        for (int j = 0; j <width; j++)
             out_array[i][j] = (uint8)pow(in_array[i][j], 1.02);
    }
}

#define Gourd 256
uint8 OTSU(uint8 *pic,uint16 num)
{
   uint16 i=0;
   uint16 Histogram[Gourd];
   for (i=0;i<Gourd;i++)
       Histogram[i]=0;

   for (i=0;i<num;i++)
   {
       Histogram[(int)pic[i]*Gourd/256]++;
   }

  float pt[Gourd],w[Gourd],u[Gourd],o[Gourd],Ut;

  pt[0]=(float)Histogram[0]/num;
  w[0]=pt[0];
  u[0]=w[0];

  for(i=1;i<Gourd;i++)
  {
    pt[i]=(float)Histogram[i]/num; 
    w[i]=w[i-1]+pt[i];
    u[i]=u[i-1]+i*pt[i];
  };
  Ut=u[Gourd-1];

  for(i=0;i<Gourd;i++)
  {
    o[i]=(1-pt[i])*(u[i]*u[i]/w[i]+(u[i]-Ut)*(u[i]-Ut)/(1-w[i]));
  };

  int maxi=0;
  float maxo=0;

  for(i=0;i<Gourd;i++)
  {
    if(o[i]!=0x7FC0000)
    if(o[i]>maxo){maxo=o[i];maxi=i;}

  }
  return maxi*256/Gourd;
}

//Border Detect
void borderDetect(uint8 in[][160],uint8 out[][160]){
		uint8 i,left,right,color=in[120][80];
		for(i=128;i>120;i--){
			for(left=80;(in[i][left]==in[i][left-1])&&(left>1);left--){
			}
			for(right=80;(in[i][right]==in[i][right+1])&&(right<158);right++){
			}
			out[i][left]=103;
			out[i][left+1]=103;
			out[i][left/2+right/2]=100;
			out[i][right]=106;
			out[i][right-1]=106;
		}
		for(;i>1;i--){
			if(in[i-1][left]!=color){
				for(;in[i][left]==in[i][left+1];left++){
				}
			}
			else{
				for(;(in[i][left]==in[i][left-1])&&(left>1);left--){
				}
			}
			if(in[i-1][right]!=color){
				for(;in[i][right]==in[i][right-1];right--){
				}
			}
			else{
				for(;(in[i][right]==in[i][left+1])&&(right<158);right++){
				}
			}
			out[i][left]=103;
			out[i][left+1]=103;
			out[i][left/2+right/2]=100;
			out[i][right]=106;
			out[i][right-1]=106;
		}
}




int main(void)
{
    DisableGlobalIRQ();
    board_init();   	//务必保留，本函数用于初始化MPU 时钟 调试串口
    
	systick_delay_ms(300);	//延时300ms，等待主板其他外设上电成功
	
    //显示模式设置为3  竖屏模式
    //显示模式在SEEKFREE_18TFT.h文件内的TFT_DISPLAY_DIR宏定义设置
    lcd_init();     	//初始化TFT屏幕
    lcd_showstr(0,0,"SEEKFREE MT9V03x");
    lcd_showstr(0,1,"Initializing...");
    //如果屏幕没有任何显示，请检查屏幕接线
    
    mt9v03x_csi_init();	//初始化摄像头 使用CSI接口
    //如果屏幕一直显示初始化信息，请检查摄像头接线
    //如果使用主板，一直卡在while(!uart_receive_flag)，请检查是否电池连接OK?
    //如果图像只采集一次，请检查场信号(VSY)是否连接OK?
    
    lcd_showstr(0,1,"     OK...     ");
    systick_delay_ms(500);
    
    EnableGlobalIRQ(0);
    while(1)
    {
        while(!mt9v03x_csi_finish_flag){} 
    
					mt9v03x_csi_finish_flag = 0;
						

					picGamma(mt9v03x_csi_image,mt9v03x_csi_image,MT9V03X_CSI_H,MT9V03X_CSI_W);
					
					for(i=0;i<128;i++){
						for(j=0;j<160;j++){
							if(mt9v03x_csi_image[i][j]<130){
								frame[i][j]=255;
							}
							else{
								frame[i][j]=0;
							}	
						}
					}
					
					borderDetect(frame,frame);
            lcdOutput(frame[0], 160, 128, 160, 128);
						
        }
}





