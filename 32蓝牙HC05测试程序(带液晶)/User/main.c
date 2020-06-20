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

	//初始化systick
	SysTick_Init();

	//lcd屏幕初始化
	lcdPrepare();
	LED_GPIO_Config();
	ILI9341_DispString_EN ( 40, 20, "HC05 BlueTooth Demo" );
	
	//蓝牙模块的初始化工作 
	hc05Prepare(hc05_name);

	while(1){
		//搜索蓝牙模块，并进行连接
		//这个IS_HC05_CONNECTED（）函数的功能是读取URAST2的输出口的状态值
		if(Task_Delay[2]==0 && !IS_HC05_CONNECTED() ) {
				//从模式
			ILI9341_DispString_EN( 20, 60, "wait conn" );
				HC05_Send_CMD("AT+INQ\r\n",1);//模块在查询状态，才能容易被其它设备搜索到
				delay_ms(1000);
				HC05_Send_CMD("AT+INQC\r\n",1);//中断查询，防止查询的结果干扰串口透传的接收
				ILI9341_Clear(0,80,240,240);		
				ILI9341_DispString_EN( 20, 60, "stop Wait" );
			Task_Delay[2]=2000; //此值每1ms会减1，减到0才可以重新进来这里，所以执行的周期是2s	
		}
		//连接后每隔一段时间检查接收缓冲区
		if(Task_Delay[0]==0 && IS_HC05_CONNECTED())  {
				uint16_t linelen;
				
				LCD_SetColors(YELLOW,BLACK);
				
				ILI9341_Clear(0,80,240,20);
				ILI9341_DispString_EN( 5, 80,"Bluetooth connected!"  );

				/*获取数据*/
				redata = get_rebuff(&len); 
				linelen = get_line(linebuff,redata,len);
			
				/*检查数据是否有更新*/
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
						/*这里只演示显示单行的数据，如果想显示完整的数据，可直接使用redata数组*/
						
						ILI9341_Clear(0,120,240,200);
						
						LCD_SetColors(RED,BLACK);

						ILI9341_DispString_EN( 5, 120,"receive data:" );
						
						LCD_SetColors(YELLOW,BLACK);
						ILI9341_DispString_EN( 5, 140,linebuff );
					}
					/*处理数据后，清空接收蓝牙模块数据的缓冲区*/
					clean_rebuff();
				}
			Task_Delay[0]=500;//此值每1ms会减1，减到0才可以重新进来这里，所以执行的周期是500ms
		}
		
		//连接后每隔一段时间通过蓝牙模块发送字符串
		if(Task_Delay[1]==0 && IS_HC05_CONNECTED())
		{
			static uint8_t testdata=0;
		
			sprintf(sendData,"<%s> send data test,testdata=%d\r\n",hc05_name,testdata++);
			HC05_SendString(sendData);			

			Task_Delay[1]=5000;//此值每1ms会减1，减到0才可以重新进来这里，所以执行的周期是5000ms
		}		
	}
}

void hc05Prepare(char hc05_name[]){
	
	char hc05_nameCMD[40];
	char disp_buff[200];
	//蓝牙准备
	if(HC05_Init() == 0){
		ILI9341_DispString_EN ( 40, 40, "HC05 module detected!" );
	}else{
		ILI9341_DispString_EN ( 20, 40, "No HC05 module detected!"  );
		ILI9341_DispString_EN ( 5, 60, "Please check the hardware connection and reset the system." );
		while(1);
	}

	/*复位、恢复默认状态*/
	if(HC05_Send_CMD("AT+RESET\r\n",1) == 1){
		ov7725_WriteString("1 enter error");
		delay_ms(100);
		ov7725_WriteString("1 error");
	}
	HC05_Send_CMD("AT+ORGL\r\n",1);
	delay_ms(200);

	/*各种命令测试演示，默认不显示。
	 *在bsp_hc05.h文件把HC05_DEBUG_ON 宏设置为1，
	 *即可通过串口调试助手接收调试信息*/	

	//HC05_Send_CMD("AT+VERSION?\r\n",1);
	//HC05_Send_CMD("AT+ADDR?\r\n",1);
	//HC05_Send_CMD("AT+UART?\r\n",1);
	//HC05_Send_CMD("AT+CMODE?\r\n",1);
	//HC05_Send_CMD("AT+STATE?\r\n",1);
	//HC05_Send_CMD("AT+ROLE=0\r\n",1);

	/*初始化SPP规范*/
	HC05_Send_CMD("AT+INIT\r\n",1);
	HC05_Send_CMD("AT+CLASS=0\r\n",1);
	HC05_Send_CMD("AT+INQM=1,9,48\r\n",1);
	
	/*设置模块名字*/
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

