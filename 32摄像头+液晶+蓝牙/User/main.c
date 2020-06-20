#include "stm32f10x.h"
//����ͷģ�������ļ�
#include "./ov7725/bsp_ov7725.h"
//lcdҺ����ʾ���ļ�
#include "./lcd/bsp_ili9341_lcd.h"
//��led�����õ�һ���ļ�
#include "./led/bsp_led.h"
//SysTick_Config��Ҫ�������ã�����STK_VAL�Ĵ���������SysTickʱ��ΪAHB 
#include "./systick/bsp_SysTick.h"

#include "./usart/bsp_usart_blt.h"
#include "./hc05/bsp_hc05.h"
#include <stdio.h>
#include<string.h>

extern uint8_t Ov7725_vsync;

unsigned int Task_Delay[NumOfTask]; 

void hc05Prepare(char hc05_name[]);
void ov7725_WriteString(char *str);
void bt_WriteString(char *str);
void btSendString(char* str);
void btReceiveData(void);
//��������ͷ���ýṹ�����
extern OV7725_MODE_PARAM cam_mode;
BLTDev bltDevList;
/**
		ov7725�������ܽ�ʹ��SCL(PC6)��SDA(PC7),oe(a3),wrst(c4),rrst(a2)
								rclk(c5),we(d3),data(b8-b15),(�ⲿ�ж�VSYNC֡ͬ���ź�)c3
		lcd������ cs(d7),dc(d11),wr(d5),rd(d4),rst(e1),bk(d12),
							data(d14,d15,d0,d1,e7-e15,d8-d10) ��16��������
*/
uint16_t realData[] = {0x55cc};   //  8899 ==>  -103  -120   // 55cc ==>  -52 85

uint8_t realSend[4];


int main(void){		
	int frame_count = 0;
	uint8_t retry = 0;
	
	char btData[30];
	
	char hc05_name[30]="HC05_SLAVE";
	
	//��ʱ����ʼ������
	SysTick_Init();
	//led�Ƶĳ�ʼ�����ú���
	LED_GPIO_Config();
	//��ʾ����׼������
	initLcdMode(cam_mode);
	lcdPrepare(cam_mode.lcd_scan);

	/* ov7725 ����ͷ��gpio ��ʼ�� */
	OV7725_GPIO_Config();
	/* ov7725 �Ĵ���Ĭ�����ó�ʼ�� */
	//OV7725_Init ����ǳ�ʼ������ͷ�ĺ�����Ҳ���Ǽ������ͷ�Ƿ���ڵĺ���
	while(OV7725_Init() != SUCCESS){
		retry++;
		if(retry>5){
			ov7725_WriteString("No OV7725 module detected!");
			while(1);
		}
	}
	//����ov7725֮ǰ����һЩ��ʼ�����ƵĶ���
	ov7725Prepare();

	Ov7725_vsync = 0;
	
	//����ģ��ĳ�ʼ������ 
	hc05Prepare(hc05_name);
	
	while(1){
		//�����ȼ����������
		if(Task_Delay[3]==0 && !IS_HC05_CONNECTED() ) {
			ILI9341_Clear(0,LINE(1),320,240 - LINE(1));
			bt_WriteString("wait conn");
			//��ģʽ
			HC05_Send_CMD("AT+INQ\r\n",1);//ģ���ڲ�ѯ״̬���������ױ������豸������
			delay_ms(1000);
			HC05_Send_CMD("AT+INQC\r\n",1);//�жϲ�ѯ����ֹ��ѯ�Ľ�����Ŵ���͸���Ľ���
			ILI9341_Clear(0,80,240,240);		
			bt_WriteString("stop Wait" );
			Task_Delay[3]=2000; //��ֵÿ1ms���1������0�ſ������½����������ִ�е�������2s	
		}
		if(IS_HC05_CONNECTED()){
			/*���յ���ͼ�������ʾ*/
			if( Ov7725_vsync == 2){
				frame_count++;
				FIFO_PREPARE;  			/*FIFO׼��*/
				LED_RED;
				ImagDisp(cam_mode.lcd_sx,
									cam_mode.lcd_sy,
									cam_mode.cam_width,
									cam_mode.cam_height);			/*�ɼ�����ʾ*/
				Ov7725_vsync = 0;
				LED_GREEN;
				//sprintf(btData,"frame_c = %d",frame_count);
				//btSendString(btData);
				//bt_WriteString("write ok");
			}
			/*ÿ��һ��ʱ�����һ��֡��*/
			if(Task_Delay[0] == 0)  {	
				char sendData[50];
				sprintf(sendData,"frame_count =%d",frame_count);
				ov7725_WriteString(sendData);			
				frame_count = 0;
				Task_Delay[0] = 1000;
			}
			if(Task_Delay[4] == 0){
				btReceiveData();
				Task_Delay[4] = 500;
			}
		}	
	}
}

void ov7725_WriteString(char *str){
	ILI9341_Clear(0,0,160,LINE(1));
	ILI9341_DispString_EN(10,LINE(0),str);
}

void bt_WriteString(char *str){
	ILI9341_Clear(160,0,160,LINE(1));
	ILI9341_DispString_EN(170,LINE(0),str);
}

void hc05Prepare(char hc05_name[]){
	
	char hc05_nameCMD[40];
	char disp_buff[200];
	//����׼��
	if(HC05_Init() == 0){
		bt_WriteString( "HC05 detected!" );
	}else{
		bt_WriteString( "No HC05 detected!"  );
		bt_WriteString("check hardware con" );
		while(1);
	}
	
	/*��λ���ָ�Ĭ��״̬*/
	HC05_Send_CMD("AT+RESET\r\n",1);	
	delay_ms(100);
	
	HC05_Send_CMD("AT+ORGL\r\n",1);
	delay_ms(100);

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

	sprintf(disp_buff,"Dev n:%s",hc05_name);
	bt_WriteString( disp_buff );
}

void btSendString(char* str){
	HC05_SendString(str);
}

void btReceiveData(void ){
	uint16_t linelen;
	char* redata;
	char linebuff[1024];
	uint16_t len = 0;
	/*��ȡ����*/
	redata = get_rebuff(&len); 
	linelen = get_line(linebuff,redata,len);

	/*��������Ƿ��и���*/
	if(linelen<200 && linelen != 0)	{
		/*����ֻ��ʾ��ʾ���е����ݣ��������ʾ���������ݣ���ֱ��ʹ��redata����*/
		//bt_WriteString(linebuff );
	}
	/*�������ݺ���ս�������ģ�����ݵĻ�����*/
	clean_rebuff();
}


