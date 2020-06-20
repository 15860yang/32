#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_esp8266.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./ov7725/bsp_ov7725.h"
#include "./led/bsp_led.h" 
#include "./nrf/bsp_spi_nrf.h"
#include "./dht11/bsp_dht11.h"
#include "string.h"
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
void nrfStatusCheck(void);
int det11GetData(void);
void wifiReceiveData(void);
void redLed(int flag);
void orangeLed(int flag);
void flagLedInit(void);
void greenLed(int flag);
unsigned int Task_Delay[NumOfTask]; 

extern uint8_t Ov7725_vsync;
extern OV7725_MODE_PARAM cam_mode;

DHT11_Data_TypeDef DHT11_Data;
char wifiSendBuff[100];
u8 status;	               // �����жϽ���/����״̬
u8 status2;                // �����жϽ���/����״̬
u8 txbuf[5]={1,2,3,4};	   // ���ͻ���
u8 rxbuf[32];			         // ���ջ���
int i=0;
char rx_buff[38];


int main ( void ){
	int frame_count = 0;

	LED_GPIO_Config();
	flagLedInit();//e5  c13  
	
	/*��ʼ��DTT11������  e6 */
	DHT11_Init ();
	
	//D7 D11 D4 E1 D12 D14 D15 D0 D1 E7-E15 D8-D10
	lcdPrepare();
	left_WriteString("lcd init ok!");
	
	SysTick_Init ();   //���� SysTick Ϊ 1ms �ж�һ�� 
	
	//SCL(a11)��SDA(C7)  oe(a3)  wrst(a12 )  rrst(a2)  rclk( b5)  we(d3)  data(c8 - c11,b12 - b15) ʱ��  A8û��
	ov7725Prepare();
	Ov7725_vsync = 0;
	left_WriteString("ov7725 init ok!");
	ESP8266_Init ();   //��ʼ��WiFiģ��ʹ�õĽӿں�����
	left_WriteString("ESP8266 init ok!");
	
	/* ��ʼ��NRF1  a5 a6 a7 c4 c5 c6 */
  SPI_NRF_Init();
	nrfStatusCheck();
	left_WriteString("nrf init ok!");
	initWifi();
	left_WriteString("wifi init ok!");
	connectToServer();
	left_WriteString("conn init ok!");
	while (1){	
		if (ucTcpClosedFlag ) {      //����Ƿ�ʧȥ����
			LED_BLUE;
			reConnectToServer();
		}else{
			LED_GREEN;

			/*���յ���ͼ�������ʾ*/
			if( Ov7725_vsync == 2 ){
				frame_count++;
				FIFO_PREPARE;  			/*FIFO׼��*/		
				left_WriteString("start play pic");				
				ImagDisp(cam_mode.lcd_sx,
									cam_mode.lcd_sy,
									cam_mode.cam_width,
									cam_mode.cam_height);			/*�ɼ�����ʾ*/
				Ov7725_vsync = 0;		
				left_WriteString("end play pic");				
			}
			//Delay_ms(1000);
			
			//ILI9341_Clear(0,LINE(1),cam_mode.cam_width,cam_mode.cam_height);
			
			/*ÿ��һ��ʱ�����һ��֡��*/
			if(Task_Delay[0] == 0)  
			{			
				//sprintf(frameData,"frame %d",frame_count);
				//left_WriteString(frameData);
				frame_count = 0;
				Task_Delay[0] = 1000;
			}
			det11GetData();	
			/* �ȴ� NRF1 �������� */
			if(NRF_Read_IRQ()==0){
				status = NRF_Rx_Dat(rxbuf);
				/* �жϽ���״̬ */
				if(status == RX_DR){
					char *temp = "*##*%d#%d#%d#%d#%s";
					char ee[] = "T:%d.%d H:%d.%d%%";
					char aa[25];
					left_WriteString((char*)rxbuf);
					sprintf(wifiSendBuff,temp,DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci,(char*)rxbuf);
					sprintf(aa,ee,DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci);
					right_WriteString(aa);
					wifiSendString(wifiSendBuff);
				}
				NRF_CE_HIGH();	 //�������״̬			
			}
			wifiReceiveData();		
		}
	}	
}

void wifiReceiveData(){
	if(strEsp8266_Fram_Record .InfBit .FramFinishFlag == 1){
		LED_YELLOW;
		strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = 0;
		strEsp8266_Fram_Record .InfBit .FramLength = 0;
		strEsp8266_Fram_Record .InfBit .FramFinishFlag = 0;
		//������¶�    �ٴ���ʪ��
		if(strstr(strEsp8266_Fram_Record .Data_RX_BUF,"**TT##")){
			redLed(1);
		}else if(strstr(strEsp8266_Fram_Record .Data_RX_BUF,"**HH##")){
			orangeLed(1);
		}else if(strstr(strEsp8266_Fram_Record .Data_RX_BUF,"**00##")){
			redLed(0);
		}else if(strstr(strEsp8266_Fram_Record .Data_RX_BUF,"**11##")){
			orangeLed(0);
		}else if(strstr(strEsp8266_Fram_Record .Data_RX_BUF,"**CD##")){
			//����  ��һ��
			greenLed(1);
			Delay_ms(300);
			greenLed(0);
		}else if(strstr(strEsp8266_Fram_Record .Data_RX_BUF,"**WS##")){
			//��ˮ  ������
			greenLed(1);
			Delay_ms(300);
			greenLed(0);
			Delay_ms(300);
			greenLed(1);
			Delay_ms(300);
			greenLed(0);
		}
	} 
}

void redLed(int flag){
	if(flag == 1){
		GPIO_SetBits(GPIOC,GPIO_Pin_13);   //����
	}else {
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);   //Ϩ��
	}
}

void orangeLed(int flag){
	if(flag == 1){
		GPIO_SetBits(GPIOE,GPIO_Pin_5);   //����
	}else {
		GPIO_ResetBits(GPIOE,GPIO_Pin_5);  //Ϩ��
	}
}

void greenLed(int flag){
	if(flag == 1){
		GPIO_SetBits(GPIOC,GPIO_Pin_12);   //����
	}else {
		GPIO_ResetBits(GPIOC,GPIO_Pin_12);   //Ϩ��
	}
}

void flagLedInit(void){
	GPIO_InitTypeDef GPIO_InitStructure;//����ṹ�����
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;  //ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	 //�����������ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //���ô�������
	GPIO_Init(GPIOE,&GPIO_InitStructure); 	   /* ��ʼ��GPIO */
	
	GPIO_ResetBits(GPIOE,GPIO_Pin_5);   //��LED�˿����ߣ�Ϩ������LED

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;  //ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	 //�����������ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //���ô�������
	GPIO_Init(GPIOC,&GPIO_InitStructure); 	   /* ��ʼ��GPIO */
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);   //Ϩ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;  //ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	 //�����������ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //���ô�������
	GPIO_Init(GPIOC,&GPIO_InitStructure); 	   /* ��ʼ��GPIO */
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_12);   //Ϩ��
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

void nrfStatusCheck(void){
	/*��� NRF ģ���� MCU ������*/
  status = NRF_Check(); 

  /*�ж�����״̬*/  
  if(status == SUCCESS){		
		left_WriteString("NRF bind MCU");
	}else{ 
		left_WriteString("NRF unbind MCU");
		while(1);
	}
  NRF_RX_Mode();     // NRF1 �������ģʽ
}

int det11GetData(void){
	/*����DHT11_Read_TempAndHumidity��ȡ��ʪ�ȣ����ɹ����������Ϣ*/
	if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS){
		return 1;
	}else{
		return 0;
	}
}
