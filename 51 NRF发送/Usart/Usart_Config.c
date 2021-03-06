/*************************************DoWell电子******************************************************
@文件名    : Usart_Config.c
@描述		   : 51单片机串口配置
@实验平台  : nRF24l01无线模块+STC89C52最小系统板（带有nRF24L01模块接口）
@
@作者      :镁天   QQ:1402330827		技术讨论/支持QQ群:121597570(加群请通过旺旺向店主获得验证码)
@
@电子邮箱  :wshww123@yeah.net
@
@
@版权归属  : 供给学习参考使用，未经允许请勿使用在其他用途
****************************************************************************************************/
#include <reg52.h>
#include "Usart_Config.h"

#define FOSC 11059200L
#define BAUD 9600
/********************************************************************************
@函数名称:	InitUSART()
@描述：			串口初始化
@输入：			tx_buf  发送数据缓冲地址指针
@输出：			无																										  
@返回：			无
@注意事项： 先设置接收端的地址，应答时需要，接收端地址需要与发送端一致。接着装载
					  要发送的数据，最后设置为发送模式，把CE设置为高电平启动发送，时间最少为10us
********************************************************************************/
void InitUSART()	
{  
	PCON = 0x00;							
	SCON = 0x50;	  //选择方式1
	TMOD = 0x20;		//定时器1 工作方式2 8位自动重装		
	TH1 = TL1 = 256 -(FOSC/12/32/BAUD);//设置波特率定时器初值	
	TR1  = 1;				//启动定时器
}
/********************************************************************************
@函数名称:	Rx_Byte()
@描述：			串口发送
@输入：			R_Byte  将要发送的数据
@输出：			无																										  
@返回：			无
@注意事项： 有些人对这点很不明白，它是怎么发送的。我呢就说下吧，只要向SBUF这个寄存
						器放入数据，在串口发送开启的状态下回自动启动发送的，不用CPU的干预，硬
						件自行操作，只需要判断TI这个标志就行了，发送时为0，完成发送会被置1
********************************************************************************/
void Rx_Byte(uchar R_Byte)
{		
	SBUF = R_Byte;  				//装载数据到串口缓冲
	while( TI == 0 );				//查询发送完成标志
	TI = 0;    							//清楚完成标志
}