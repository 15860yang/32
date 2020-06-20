#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_esp8266.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./ov7725/bsp_ov7725.h"
#include "./led/bsp_led.h" 

/********************************** �û���Ҫ���õĲ���**********************************/
#define      macUser_ESP8266_ApSsid                       "HUAWEI P30 Pro"                //Ҫ���ӵ��ȵ������
#define      macUser_ESP8266_ApPwd                        "yanghao.0"           //Ҫ���ӵ��ȵ����Կ
#define      macUser_ESP8266_TcpServer_IP                 "192.168.43.1"      //Ҫ���ӵķ������� IP
#define      macUser_ESP8266_TcpServer_Port               "22222"               //Ҫ���ӵķ������Ķ˿�

void initWifi(void);
void connectToServer(void);
void reConnectToServer(void);
void left_WriteString(char *str);
void right_WriteString(char *str);
unsigned int Task_Delay[NumOfTask]; 

extern uint8_t Ov7725_vsync;
extern OV7725_MODE_PARAM cam_mode;

int main ( void ){
	int frame_count = 0;
	char cStr [ 100 ] = { 0 };
	char frameData[15];
	
	char temp[12];
	
	LED_GPIO_Config();
	lcdPrepare();
	
	
	left_WriteString("lcd init ok!");
	
	SysTick_Init ();   //���� SysTick Ϊ 1ms �ж�һ�� 
	
	//b8 - b11
	
	//c8 c9 c10 c11
	
	//SCL(C6)��SDA(C7)  oe(a3)  wrst(c4)  rrst(a2)  rclk(c5)  we(d3)  data(b8 - b15) ʱ��  A8û��
	ov7725Prepare();
	Ov7725_vsync = 0;
	left_WriteString("ov7725 init ok!");
	ESP8266_Init ();   //��ʼ��WiFiģ��ʹ�õĽӿں�����
	left_WriteString("ESP8266 init ok!");
	
	initWifi();
	left_WriteString("wifi init ok!");
	connectToServer();
	left_WriteString("conn init ok!");
	while (1){	
		if (ucTcpClosedFlag ) {      //����Ƿ�ʧȥ����
			LED_BLUE;
			reConnectToServer();
		}else{
			LED_YELLOW;

			//right_WriteString(cStr);
			/*���յ���ͼ�������ʾ*/
			if( Ov7725_vsync == 2 ){
				frame_count++;
				FIFO_PREPARE;  			/*FIFO׼��*/					
				ImagDisp(cam_mode.lcd_sx,
									cam_mode.lcd_sy,
									cam_mode.cam_width,
									cam_mode.cam_height);			/*�ɼ�����ʾ*/
				Ov7725_vsync = 0;		
			}
			Delay_ms(1000);
			
			ILI9341_Clear(0,LINE(1),cam_mode.cam_width,cam_mode.cam_height);
			
			/*ÿ��һ��ʱ�����һ��֡��*/
			if(Task_Delay[0] == 0)  
			{			
				sprintf(frameData,"frame %d",frame_count);
				left_WriteString(frameData);
				frame_count = 0;
				Task_Delay[0] = 1000;
			}
			
		}
		
	}	
}

void initWifi(void){
	macESP8266_CH_ENABLE();
	ESP8266_AT_Test ();
	ESP8266_Net_Mode_Choose ( STA );
}

void connectToServer(void){
	//����WiFi
  while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	ESP8266_Enable_MultipleId ( DISABLE );
	while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) ){};
	while ( ! ESP8266_UnvarnishSend () ){};
}

void reConnectToServer(void){
	uint8_t ucStatus;
	ESP8266_ExitUnvarnishSend ();                                    //�˳�͸��ģʽ
	do ucStatus = ESP8266_Get_LinkStatus ();                         //��ȡ����״̬
	while ( ! ucStatus );
	if ( ucStatus == 4 ){                                             //ȷ��ʧȥ���Ӻ�����
		while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );
		while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	}
	while ( ! ESP8266_UnvarnishSend () );	
}

void left_WriteString(char *str){
	ILI9341_Clear(0,0,160,LINE(1));
	ILI9341_DispString_EN(10,LINE(0),str);
}

void right_WriteString(char *str){
	ILI9341_Clear(160,0,160,LINE(1));
	ILI9341_DispString_EN(170,LINE(0),str);
}

