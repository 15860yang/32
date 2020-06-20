/*************************************DoWell电子******************************************************
@文件名    : nRF24L01.c
@描述		   : nRF24l01无线模块底层驱动
@实验平台  : nRF24l01无线模块+STC89C52最小系统板（带有nRF24L01模块接口）
@
@作者      :镁天   QQ:1402330827		技术讨论/支持QQ群:121597570(加群请通过旺旺向店主获得验证码)
@
@电子邮箱  :wshww123@yeah.net
@
@
@版权归属  : 部分代码来源网络，由DoWell电子整理修改，供给学习参考使用
****************************************************************************************************/
#include <reg52.h>
#include "NRF24L01.h"
#include "Type_Define.h"

uchar const code TX_ADDRESS[TX_ADR_WIDTH]= {0x30,0x30,0x30,0x30,0x30};	//本地地址
uchar const code RX_ADDRESS[RX_ADR_WIDTH]= {0x30,0x30,0x30,0x30,0x30};	//接收地址

	
//*************************************中断标志********************************************
//以下任意一个中断都会使IRQ引脚编程低电平，判断IRQ引脚就可以知道当前接收或者发送的状态了
uchar 	bdata NRF24L01_Sta;        //状态标志
sbit	  RX_DR	=NRF24L01_Sta^6;			//接收中断标志
sbit	  TX_DS	=NRF24L01_Sta^5;			//发送中断标志
sbit	  MAX_RT	=NRF24L01_Sta^4;		//重发次数中断标志，当重发次数超过了最大允许的重发次数该中断产生

/********************************************************************************
@函数名称:	Delay_us()
@描述：			普通延时函数，us级延时函数
@输入：		  n  延时的时间长度
@输出：			无
@返回：			无
@注意事项：	无
********************************************************************************/
void Delay_us(uint n)
{
	for(;n>0;n--) ;
}
/********************************************************************************
@函数名称:	Init_NRF24L01()
@描述：			24L01初始化函数
@输入：			无
@输出：			无
@返回：			无
@注意事项：	无
********************************************************************************/
void Init_NRF24L01(void)
{
  	Delay_us(100);
	IRQ = 1; //中断端口初始化为高
 	CE=0;    // 芯片使能
 	CSN=1;   // SPI 失能，即禁止2401芯片的SPI功能
 	SCK=0;   // 初始化SPI时钟
	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    // 设置本地地址	
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // 设置接收地址
	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);      //  频道0自动	ACK应答允许	
	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);  //  允许接收地址只有频道0，如果需要多频道可以参考Page21 
	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0xF5); //  设置自动重发的延时和次数  延时大小4000+86us 次数5次 
	SPI_RW_Reg(WRITE_REG + RF_CH, 0x88);         //   设置信道工作为2.4GHZ，收发必须一致
	SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //设置接收数据长度，本次设置为32字节
	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x0f);   //设置发射速率为2Mbps，发射功率为最大值0dB
	//相对于接收，初始化的时候只有下面这句不同
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x5e);   	// IRQ收发完成中断响应，16位CRC	，主发送

	SPI_RW_Reg(WRITE_REG+STATUS,0x71);     		//清除通道0中断标志
	CE = 1; 																	//片选失能
	Delay_us(400);
}
/********************************************************************************
@函数名称:	SPI_RW()
@描述：			24L01的读写函数
@输入：			dat  发送的数据
@输出：			无
@返回：			dat  接收到的数据
@注意事项：	根据需要决定是发送数据有效，还是接收数据有效
********************************************************************************/
uchar SPI_RW(uchar dat)		//发送dat数据，并返回接收到得数据
{
	uchar Bit_Count;				//位计数器，用于确定接收或者发送的位数，这里是一个字节即8位
 	for(Bit_Count=0;Bit_Count<8;Bit_Count++) 
 	{
		MOSI = (dat & 0x80);         // 输出数据, MSB to MOSI，即先送高位后送低位
		dat = (dat << 1);            // 右移一位，准备传送下一个数据
		SCK = 1;                     // 设置 SCK 为高
		dat |= MISO;       		       // 接收2401传出的数据，也是先送高位后送低位
		SCK = 0;            		     // 拉低SCK时钟产生下降沿，从MOSI送出数据，在下一次SCK为高时读入NISO数据
 	}
	return(dat);						
}
/********************************************************************************
@函数名称:	SPI_RW()
@描述：			24L01的读写函数
@输入：			dat  发送的数据
@输出：			无
@返回：			dat  接收到的数据
@注意事项：	根据需要决定是发送数据有效，还是接收数据有效
********************************************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
	
	CSN = 0;                // CSN 拉低, 启动SPI通信
	SPI_RW(reg);            // 设置读取的寄存器
	reg_val = SPI_RW(0xff); // 读出寄存器中的数据
	CSN = 1;                // CSN 拉高, 终止SPI通信
	
	return(reg_val);        // 返回接收到的数据
}
/********************************************************************************
@函数名称:	SPI_RW_Reg()
@描述：			24L01的读写寄存器函数
@输入：			reg   寄存器地址
						value 写入的数据
@输出：			无
@返回：			status 当前寄存器的状态
@注意事项：	先写寄存器地址，即选择寄存器，然后再写入数据
********************************************************************************/
uchar SPI_RW_Reg(uchar reg, uchar value)
{
	uchar status;
	
	CSN = 0;                   // CSN 拉低, 启动SPI通信
	status = SPI_RW(reg);      // 选择要操作的寄存器
	SPI_RW(value);             // 向选择的寄存器中写入数据
	CSN = 1;                   // CSN 拉高, 终止SPI通信
	
	return(status);            // 返回2401 reg寄存器的状态
}
/********************************************************************************
@函数名称:	SPI_Read_Buf()
@描述：			读取2401接收数据寄存器的数据
@输入：			reg      寄存器地址
						pBuf     接收数据指针
						Byte_Num 接收数据的字节数
@输出：			无
@返回：			status 当前寄存器的状态
@注意事项：	先设置读寄存器地址，然后再读数据
********************************************************************************/
uchar SPI_Read_Buf(uchar reg, uchar *pBuf, uchar Byte_Num)
{
	uchar status,Byte_Count;
	
	CSN = 0;                    		   // CSN 拉低, 启动SPI通信
	status = SPI_RW(reg);       		   // 设置读取的寄存器和该寄存器的状态	
	for(Byte_Count=0;Byte_Count<Byte_Num;Byte_Count++)
	{
		pBuf[Byte_Count] = SPI_RW(0xff);  // 连续接收uchars个数据 
	}
	CSN = 1;                           
	
	return(status);                    // 返回24L01的状态
}
/********************************************************************************
@函数名称:	SPI_Write_Buf()
@描述：			向nRF24L01数据寄存器写入数据
@输入：			reg      寄存器地址
						pBuf     待写入数据地址指针
						Byte_Num 接收数据的字节数
@输出：			无
@返回：			status 当前寄存器的状态
@注意事项：	先写寄存器地址，即选择寄存器，然后再写入数据
********************************************************************************/
uchar SPI_Write_Buf(uchar reg, uchar *pBuf, uchar Byte_Num)
{
	uchar status,Byte_Count;
	
	CSN = 0;            //SPI使能       
	status = SPI_RW(reg);   
	for(Byte_Count = 0; Byte_Count < Byte_Num; Byte_Count++) 
	{
		SPI_RW(*pBuf++); //指向下一个数据
	}
	CSN = 1;           //关闭SPI
	return(status);    //返回24L01的状态 
}
/********************************************************************************
@函数名称:	SetRX_Mode()
@描述：			nRF24L01接收配置 
@输入：			无
@输出：			无
@返回：			无
@注意事项：	无
********************************************************************************/
void SetRX_Mode(void)
{
	CE=0;																	//片选使能
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x3F); // IRQ收发完成中断响应，16位CRC	，上电，接收模式
	CE = 1; 															//片选失能
	Delay_us(400);
}
/********************************************************************************
@函数名称:	nRF24L01_RxPacket()
@描述：			检测nRF24L01接收数据状态，把接收数据放到rx_buf缓冲区指针中
@输入：			rx_buf  接收数据缓冲地址指针
@输出：			无																										  
@返回：			ReceiveComplete_Flag  完成数据接收标志  0  未接收数据  	1 数据接收成功
@注意事项：	无
********************************************************************************/
uchar NRF24L01_RxPacket(uchar *rx_buf)
{
  uchar ReceiveComplete_Flag = 0; //接收数据完成且成功标志位

	if(IRQ == 0)				  // 判断是否接收到数据
	{
		NRF24L01_Sta = SPI_Read(STATUS);	// 读取状态寄存其来判断数据接收状况
		if(RX_DR == 1)
	  {
			CE = 0; 				      //SPI使能
			SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// 从接收缓冲器中读出数据
			ReceiveComplete_Flag = 1;			      //读取数据完成标志
		}
		SPI_RW_Reg(WRITE_REG+STATUS,0x71);    //接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来清除中断标志		
		CE = 1;
		//while(!IRQ);													//等待传输完毕
		SetRX_Mode();			  									//设置为接收模式
		
	}	
	
	return ReceiveComplete_Flag;					//返回接收状态
}
/********************************************************************************
@函数名称:	nRF24L01_TxPacket()
@描述：			装载数据到nRF24L01
@输入：			tx_buf  发送数据缓冲地址指针
@输出：			无																										  
@返回：			无
@注意事项： 先设置接收端的地址，应答时需要，接收端地址需要与发送端一致。接着装载
					  要发送的数据，最后设置为发送模式，把CE设置为高电平启动发送，时间最少为10us
********************************************************************************/
void NRF24L01_TxPacket(uchar * tx_buf)
{
	CE=0;			              //待机 I模式	
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 装载接收端地址
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf,TX_PLOAD_WIDTH); 			       // 装载数据	
	SPI_RW_Reg(WRITE_REG + CONFIG,0x5e);   		                     // IRQ收发完成中断响应，16位CRC，主发送
	CE=1;		                //置高CE，激发数据发送
	Delay_us(1000);				  //在下一次操作前需要延时一段时间，10us即可	
 	//while(IRQ == 1);
	SPI_RW_Reg(WRITE_REG+STATUS,0X71); // 清除通道0状态标志 
}
