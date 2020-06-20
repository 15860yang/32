#include "stm32f10x.h"
#include "./usart/bsp_usart_blt.h"
#include "./systick/bsp_SysTick.h"
#include "./hc05/bsp_hc05.h"
#include "./led/bsp_led.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include <string.h>
#include <stdlib.h>

void hc05Prepare(char hc05_name[]);
void ov7725_WriteString(char *str);
unsigned int Task_Delay[NumOfTask]; 

BLTDev bltDevList;

char sendData[1024];
char linebuff[1024];

int main(void){	
	char* redata;
	uint16_t len;

	char hc05_name[30]="HC05_SLAVE";

	//��ʼ��systick
	SysTick_Init();

	//lcd��Ļ��ʼ��
	lcdPrepare();
	LED_GPIO_Config();
	ILI9341_DispString_EN ( 40, 20, "HC05 BlueTooth Demo" );
	
	//����ģ��ĳ�ʼ������ 
	hc05Prepare(hc05_name);

	while(1){
		//��������ģ�飬����������
		//���IS_HC05_CONNECTED���������Ĺ����Ƕ�ȡURAST2������ڵ�״ֵ̬
		if(Task_Delay[2]==0 && !IS_HC05_CONNECTED() ) {
				//��ģʽ
			ILI9341_DispString_EN( 20, 60, "wait conn" );
				HC05_Send_CMD("AT+INQ\r\n",1);//ģ���ڲ�ѯ״̬���������ױ������豸������
				delay_ms(1000);
				HC05_Send_CMD("AT+INQC\r\n",1);//�жϲ�ѯ����ֹ��ѯ�Ľ�����Ŵ���͸���Ľ���
				ILI9341_Clear(0,80,240,240);		
				ILI9341_DispString_EN( 20, 60, "stop Wait" );
			Task_Delay[2]=2000; //��ֵÿ1ms���1������0�ſ������½����������ִ�е�������2s	
		}
		//���Ӻ�ÿ��һ��ʱ������ջ�����
		if(Task_Delay[0]==0 && IS_HC05_CONNECTED())  {
				uint16_t linelen;
				
				LCD_SetColors(YELLOW,BLACK);
				
				ILI9341_Clear(0,80,240,20);
				ILI9341_DispString_EN( 5, 80,"Bluetooth connected!"  );

				/*��ȡ����*/
				redata = get_rebuff(&len); 
				linelen = get_line(linebuff,redata,len);
			
				/*��������Ƿ��и���*/
				if(linelen<200 && linelen != 0)	{
					LCD_SetColors(YELLOW,BLACK);
					if(strcmp(redata,"AT+LED1=ON")==0){
						LED1_ON;						
						HC05_SendString("+LED1:ON\r\nOK\r\n");	

						ILI9341_Clear(0,100,240,20);						
						ILI9341_DispString_EN ( 5, 100, "receive cmd: AT+LED1=ON" );
					}else if(strcmp(redata,"AT+LED1=OFF")==0){
						LED1_OFF;
						HC05_SendString("+LED1:OFF\r\nOK\r\n");

						ILI9341_Clear(0,100,240,20);
						ILI9341_DispString_EN ( 5, 100, "receive cmd: AT+LED1=OFF" );
					}else{
						/*����ֻ��ʾ��ʾ���е����ݣ��������ʾ���������ݣ���ֱ��ʹ��redata����*/
						
						ILI9341_Clear(0,120,240,200);
						
						LCD_SetColors(RED,BLACK);

						ILI9341_DispString_EN( 5, 120,"receive data:" );
						
						LCD_SetColors(YELLOW,BLACK);
						ILI9341_DispString_EN( 5, 140,linebuff );
					}
					/*�������ݺ���ս�������ģ�����ݵĻ�����*/
					clean_rebuff();
				}
			Task_Delay[0]=500;//��ֵÿ1ms���1������0�ſ������½����������ִ�е�������500ms
		}
		
		//���Ӻ�ÿ��һ��ʱ��ͨ������ģ�鷢���ַ���
		if(Task_Delay[1]==0 && IS_HC05_CONNECTED())
		{
			static uint8_t testdata=0;
		
			sprintf(sendData,"<%s> send data test,testdata=%d\r\n",hc05_name,testdata++);
			HC05_SendString(sendData);			

			Task_Delay[1]=5000;//��ֵÿ1ms���1������0�ſ������½����������ִ�е�������5000ms
		}		
	}
}

void hc05Prepare(char hc05_name[]){
	
	char hc05_nameCMD[40];
	char disp_buff[200];
	//����׼��
	if(HC05_Init() == 0){
		ILI9341_DispString_EN ( 40, 40, "HC05 module detected!" );
	}else{
		ILI9341_DispString_EN ( 20, 40, "No HC05 module detected!"  );
		ILI9341_DispString_EN ( 5, 60, "Please check the hardware connection and reset the system." );
		while(1);
	}

	/*��λ���ָ�Ĭ��״̬*/
	if(HC05_Send_CMD("AT+RESET\r\n",1) == 1){
		ov7725_WriteString("1 enter error");
		delay_ms(100);
		ov7725_WriteString("1 error");
	}
	HC05_Send_CMD("AT+ORGL\r\n",1);
	delay_ms(200);

	/*�������������ʾ��Ĭ�ϲ���ʾ��
	 *��bsp_hc05.h�ļ���HC05_DEBUG_ON ������Ϊ1��
	 *����ͨ�����ڵ������ֽ��յ�����Ϣ*/	

	//HC05_Send_CMD("AT+VERSION?\r\n",1);
	//HC05_Send_CMD("AT+ADDR?\r\n",1);
	//HC05_Send_CMD("AT+UART?\r\n",1);
	//HC05_Send_CMD("AT+CMODE?\r\n",1);
	//HC05_Send_CMD("AT+STATE?\r\n",1);
	//HC05_Send_CMD("AT+ROLE=0\r\n",1);

	/*��ʼ��SPP�淶*/
	HC05_Send_CMD("AT+INIT\r\n",1);
	HC05_Send_CMD("AT+CLASS=0\r\n",1);
	HC05_Send_CMD("AT+INQM=1,9,48\r\n",1);
	
	/*����ģ������*/
	sprintf(hc05_nameCMD,"AT+NAME=%s\r\n",hc05_name);
	HC05_Send_CMD(hc05_nameCMD,1);

	sprintf(disp_buff,"Device name:%s",hc05_name);
	ILI9341_DispString_EN( 20, 60, disp_buff );
}

void ov7725_WriteString(char *str){
	ILI9341_Clear(0,0,160,LINE(1));
	ILI9341_DispString_EN(10,LINE(0),str);
}
/*********************************************END OF FILE**********************/

