/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   2.4g无线模块/nrf24l01+/单板 测试  只使用一块开发板，
	*					 对2个NRF模块进行收发测试
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103 指南者开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
 #include <stdio.h>
#include "stm32f10x.h"
#include "bsp_spi_nrf.h"
#include "./lcd/bsp_ili9341_lcd.h"

u8 status;	               // 用于判断接收/发送状态
u8 status2;                // 用于判断接收/发送状态
u8 txbuf[5]={1,2,3,4};	   // 发送缓冲
u8 rxbuf[32];			         // 接收缓冲
int i=0;
char rx_buff[38];


void Self_Test(void);
void Delay_ms(u8 z);

 /**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)                  
{  	
	ILI9341_Init ();         //LCD 初始化
 
	ILI9341_GramScan ( 6 );
	LCD_SetFont(&Font8x16);
	LCD_SetColors(RED,BLACK);
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */

	/* 初始化NRF1 */
  SPI_NRF_Init();
	
  /* 串口1初始化 */
  //USART1_Config();
  
	ILI9341_DispStringLine_EN(LINE(0),"NRF24L01 TEST ");

	Self_Test();	

}

void Delay_ms(u8 z)
{
  u8 x,y;
	for(x = 0;x < z;x++)
	  for(y = 0;y < 110;y++);
}

 /**
  * @brief  NRF模块测试函数，NRF1和NRF2之间循环发送数据
  * @param  无
  * @retval 无
  */
void Self_Test(void)
{
  /*检测 NRF 模块与 MCU 的连接*/
  status = NRF_Check(); 

  /*判断连接状态*/  
  if(status == SUCCESS){		
		ILI9341_DispStringLine_EN(LINE(1),"NRF AND MCU CONNECT SUCCESS ");
	}else{ 
		ILI9341_DispStringLine_EN(LINE(1),"NRF AND MCU CONNECT ERROR ");
		while(1);
	}
  NRF_RX_Mode();     // NRF1 进入接收模式
	
  while(1)
  {
    /* 等待 NRF1 接收数据 */
		if(NRF_Read_IRQ()==0){
			status = NRF_Rx_Dat(rxbuf);
			/* 判断接收状态 */
			if(status == RX_DR){
				for(i=0;i<32/4;i++){	
					sprintf(rx_buff,"line %d data %d,%d,%d,%d",i,rxbuf[i*4],rxbuf[i*4+1],rxbuf[i*4+2],rxbuf[i*4+3]);
					ILI9341_DispStringLine_EN(LINE(i+2),rx_buff);
				}
			}
			NRF_CE_HIGH();	 //进入接收状态			
		} 
  }
}
/*********************************************END OF FILE**********************/
