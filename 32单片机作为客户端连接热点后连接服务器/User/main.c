#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_esp8266.h"

/********************************** 用户需要设置的参数**********************************/
#define      macUser_ESP8266_ApSsid                       "HUAWEI P30 Pro"                //要连接的热点的名称
#define      macUser_ESP8266_ApPwd                        "yanghao.0"           //要连接的热点的密钥

#define      macUser_ESP8266_TcpServer_IP                 "192.168.43.1"      //要连接的服务器的 IP
#define      macUser_ESP8266_TcpServer_Port               "22222"               //要连接的服务器的端口

void initWifi(void);
void connectToServer(void);
void reConnectToServer(void);
int main ( void ){
	char cStr [ 100 ] = { 0 };
	SysTick_Init ();                                                               //配置 SysTick 为 1ms 中断一次 
	ESP8266_Init ();                                                               //初始化WiFi模块使用的接口和外设
	
	initWifi();
	connectToServer();
	while ( 1 ){		
		sprintf ( cStr,"Hello World!" );
		ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );               //发送数据
		Delay_ms ( 1000 );
		if (ucTcpClosedFlag ) {                                            //检测是否失去连接
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
	//连接WiFi
  while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	ESP8266_Enable_MultipleId ( DISABLE );
	while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	while ( ! ESP8266_UnvarnishSend () );
}

void reConnectToServer(void){
	uint8_t ucStatus;
	ESP8266_ExitUnvarnishSend ();                                    //退出透传模式
	do ucStatus = ESP8266_Get_LinkStatus ();                         //获取连接状态
	while ( ! ucStatus );
	if ( ucStatus == 4 ){                                             //确认失去连接后重连
		while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );
		while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	}
	while ( ! ESP8266_UnvarnishSend () );	
}
