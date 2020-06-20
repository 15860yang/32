/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   2.4g����ģ��/nrf24l01+/���� ����  ֻʹ��һ�鿪���壬
	*					 ��2��NRFģ������շ�����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103 ָ���߿����� 
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
 #include <stdio.h>
#include "stm32f10x.h"
#include "bsp_spi_nrf.h"
#include "./lcd/bsp_ili9341_lcd.h"

u8 status;	               // �����жϽ���/����״̬
u8 status2;                // �����жϽ���/����״̬
u8 txbuf[5]={1,2,3,4};	   // ���ͻ���
u8 rxbuf[32];			         // ���ջ���
int i=0;
char rx_buff[38];


void Self_Test(void);
void Delay_ms(u8 z);

 /**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)                  
{  	
	ILI9341_Init ();         //LCD ��ʼ��
 
	ILI9341_GramScan ( 6 );
	LCD_SetFont(&Font8x16);
	LCD_SetColors(RED,BLACK);
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */

	/* ��ʼ��NRF1 */
  SPI_NRF_Init();
	
  /* ����1��ʼ�� */
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
  * @brief  NRFģ����Ժ�����NRF1��NRF2֮��ѭ����������
  * @param  ��
  * @retval ��
  */
void Self_Test(void)
{
  /*��� NRF ģ���� MCU ������*/
  status = NRF_Check(); 

  /*�ж�����״̬*/  
  if(status == SUCCESS){		
		ILI9341_DispStringLine_EN(LINE(1),"NRF AND MCU CONNECT SUCCESS ");
	}else{ 
		ILI9341_DispStringLine_EN(LINE(1),"NRF AND MCU CONNECT ERROR ");
		while(1);
	}
  NRF_RX_Mode();     // NRF1 �������ģʽ
	
  while(1)
  {
    /* �ȴ� NRF1 �������� */
		if(NRF_Read_IRQ()==0){
			status = NRF_Rx_Dat(rxbuf);
			/* �жϽ���״̬ */
			if(status == RX_DR){
				for(i=0;i<32/4;i++){	
					sprintf(rx_buff,"line %d data %d,%d,%d,%d",i,rxbuf[i*4],rxbuf[i*4+1],rxbuf[i*4+2],rxbuf[i*4+3]);
					ILI9341_DispStringLine_EN(LINE(i+2),rx_buff);
				}
			}
			NRF_CE_HIGH();	 //�������״̬			
		} 
  }
}
/*********************************************END OF FILE**********************/
