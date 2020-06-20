#include "stm32f10x.h"
#include "./ov7725/bsp_ov7725.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./led/bsp_led.h" 
#include "./systick/bsp_SysTick.h"

void left_WriteString(char *str);
void right_WriteString(char *str);

extern uint8_t Ov7725_vsync;

unsigned int Task_Delay[NumOfTask]; 

extern OV7725_MODE_PARAM cam_mode;

int main(void) 	{		
	float frame_count = 0;

	/* 液晶初始化 D7 D11 D4 E1 D12 D14 D15 D0 D1 E7-E15 D8-D10*/
	lcdPrepare();
	
	LED_GPIO_Config();
	SysTick_Init();

	//b8 - b11
	
	//c8 c9 c10 c11
	
	//SCL(C6)、SDA(C7)  oe(a3)  wrst(c4)  rrst(a2)  rclk(c5)  we(d3)  data(b8 - b15) 时钟  A8没用
	ov7725Prepare();
	
	left_WriteString("OV7725 initialize success!");
	
	Ov7725_vsync = 0;

	while(1){
		/*接收到新图像进行显示*/
		if( Ov7725_vsync == 2 )
		{
			frame_count++;
			FIFO_PREPARE;  			/*FIFO准备*/					
			ImagDisp(cam_mode.lcd_sx,
								cam_mode.lcd_sy,
								cam_mode.cam_width,
								cam_mode.cam_height);			/*采集并显示*/
			Ov7725_vsync = 0;		
		}
		/*每隔一段时间计算一次帧率*/
		if(Task_Delay[0] == 0)  
		{			
			frame_count = 0;
			Task_Delay[0] = 1000;
		}
	}
}

void left_WriteString(char *str){
	ILI9341_Clear(0,0,160,LINE(1));
	ILI9341_DispString_EN(10,LINE(0),str);
}

void right_WriteString(char *str){
	ILI9341_Clear(160,0,160,LINE(1));
	ILI9341_DispString_EN(170,LINE(0),str);
}


/*********************************************END OF FILE**********************/

