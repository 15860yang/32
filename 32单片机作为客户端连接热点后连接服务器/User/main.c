#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_esp8266.h"

/********************************** �û���Ҫ���õĲ���**********************************/
#define      macUser_ESP8266_ApSsid                       "HUAWEI P30 Pro"                //Ҫ���ӵ��ȵ������
#define      macUser_ESP8266_ApPwd                        "yanghao.0"           //Ҫ���ӵ��ȵ����Կ

#define      macUser_ESP8266_TcpServer_IP                 "192.168.43.1"      //Ҫ���ӵķ������� IP
#define      macUser_ESP8266_TcpServer_Port               "22222"               //Ҫ���ӵķ������Ķ˿�

void initWifi(void);
void connectToServer(void);
void reConnectToServer(void);
int main ( void ){
	char cStr [ 100 ] = { 0 };
	SysTick_Init ();                                                               //���� SysTick Ϊ 1ms �ж�һ�� 
	ESP8266_Init ();                                                               //��ʼ��WiFiģ��ʹ�õĽӿں�����
	
	initWifi();
	connectToServer();
	while ( 1 ){		
		sprintf ( cStr,"Hello World!" );
		ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );               //��������
		Delay_ms ( 1000 );
		if (ucTcpClosedFlag ) {                                            //����Ƿ�ʧȥ����
			reConnectToServer();
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
	while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	while ( ! ESP8266_UnvarnishSend () );
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
