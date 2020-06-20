#include "stm32f10x.h"
//摄像头模块驱动文件
#include "./ov7725/bsp_ov7725.h"
//lcd液晶显示器文件
#include "./lcd/bsp_ili9341_lcd.h"
//对led灯配置的一个文件
#include "./led/bsp_led.h"
//SysTick_Config主要用来配置，重置STK_VAL寄存器，配置SysTick时钟为AHB 
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
//声明摄像头配置结构体变量
extern OV7725_MODE_PARAM cam_mode;
BLTDev bltDevList;
/**
		ov7725概括：管脚使用SCL(PC6)、SDA(PC7),oe(a3),wrst(c4),rrst(a2)
								rclk(c5),we(d3),data(b8-b15),(外部中断VSYNC帧同步信号)c3
		lcd概括： cs(d7),dc(d11),wr(d5),rd(d4),rst(e1),bk(d12),
							data(d14,d15,d0,d1,e7-e15,d8-d10) 共16根数据线
*/
uint16_t realData[] = {0x55cc};   //  8899 ==>  -103  -120   // 55cc ==>  -52 85

uint8_t realSend[4];


int main(void){		
	int frame_count = 0;
	uint8_t retry = 0;
	
	char btData[30];
	
	char hc05_name[30]="HC05_SLAVE";
	
	//定时器初始化函数
	SysTick_Init();
	//led灯的初始化配置函数
	LED_GPIO_Config();
	//显示屏的准备函数
	initLcdMode(cam_mode);
	lcdPrepare(cam_mode.lcd_scan);

	/* ov7725 摄像头的gpio 初始化 */
	OV7725_GPIO_Config();
	/* ov7725 寄存器默认配置初始化 */
	//OV7725_Init 这个是初始化摄像头的函数，也就是监测摄像头是否存在的函数
	while(OV7725_Init() != SUCCESS){
		retry++;
		if(retry>5){
			ov7725_WriteString("No OV7725 module detected!");
			while(1);
		}
	}
	//启动ov7725之前设置一些初始化类似的东西
	ov7725Prepare();

	Ov7725_vsync = 0;
	
	//蓝牙模块的初始化工作 
	hc05Prepare(hc05_name);
	
	while(1){
		//进来先检查蓝牙连接
		if(Task_Delay[3]==0 && !IS_HC05_CONNECTED() ) {
			ILI9341_Clear(0,LINE(1),320,240 - LINE(1));
			bt_WriteString("wait conn");
			//从模式
			HC05_Send_CMD("AT+INQ\r\n",1);//模块在查询状态，才能容易被其它设备搜索到
			delay_ms(1000);
			HC05_Send_CMD("AT+INQC\r\n",1);//中断查询，防止查询的结果干扰串口透传的接收
			ILI9341_Clear(0,80,240,240);		
			bt_WriteString("stop Wait" );
			Task_Delay[3]=2000; //此值每1ms会减1，减到0才可以重新进来这里，所以执行的周期是2s	
		}
		if(IS_HC05_CONNECTED()){
			/*接收到新图像进行显示*/
			if( Ov7725_vsync == 2){
				frame_count++;
				FIFO_PREPARE;  			/*FIFO准备*/
				LED_RED;
				ImagDisp(cam_mode.lcd_sx,
									cam_mode.lcd_sy,
									cam_mode.cam_width,
									cam_mode.cam_height);			/*采集并显示*/
				Ov7725_vsync = 0;
				LED_GREEN;
				//sprintf(btData,"frame_c = %d",frame_count);
				//btSendString(btData);
				//bt_WriteString("write ok");
			}
			/*每隔一段时间计算一次帧率*/
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
	//蓝牙准备
	if(HC05_Init() == 0){
		bt_WriteString( "HC05 detected!" );
	}else{
		bt_WriteString( "No HC05 detected!"  );
		bt_WriteString("check hardware con" );
		while(1);
	}
	
	/*复位、恢复默认状态*/
	HC05_Send_CMD("AT+RESET\r\n",1);	
	delay_ms(100);
	
	HC05_Send_CMD("AT+ORGL\r\n",1);
	delay_ms(100);

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
	/*获取数据*/
	redata = get_rebuff(&len); 
	linelen = get_line(linebuff,redata,len);

	/*检查数据是否有更新*/
	if(linelen<200 && linelen != 0)	{
		/*这里只演示显示单行的数据，如果想显示完整的数据，可直接使用redata数组*/
		//bt_WriteString(linebuff );
	}
	/*处理数据后，清空接收蓝牙模块数据的缓冲区*/
	clean_rebuff();
}


