#include "stm32f10x_it.h"
#include <stdio.h>
#include <string.h> 
#include "bsp_SysTick.h"
#include "bsp_esp8266.h"
#include "./ov7725/bsp_ov7725.h"

extern uint8_t Ov7725_vsync;
extern unsigned int Task_Delay[];
extern void TimingDelay_Decrement(void);
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	unsigned char i;
	
	TimingDelay_Decrement();  
	TimeStamp_Increment(); 
	
	for(i=0;i<NumOfTask;i++)
	{
		if(Task_Delay[i])
		{
			Task_Delay[i]--;
		}
	}
}


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/**
  * @brief  This function handles macESP8266_USARTx Handler.
  * @param  None
  * @retval None
  */

void macESP8266_USART_INT_FUN ( void )
{	
	uint8_t ucCh;
	
	if ( USART_GetITStatus ( macESP8266_USARTx, USART_IT_RXNE ) != RESET ){
		//ÿ�ν���һ��8�ֽڵ�����
		ucCh  = USART_ReceiveData( macESP8266_USARTx );
		if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )      //Ԥ��1���ֽ�д������
			strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ucCh;
	}
	if ( USART_GetITStatus( macESP8266_USARTx, USART_IT_IDLE ) == SET )    //����֡�������
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
		ucCh = USART_ReceiveData( macESP8266_USARTx );             //�������������жϱ�־λ(�ȶ�USART_SR��Ȼ���USART_DR)
		ucTcpClosedFlag = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;
  }	
}


void OV7725_VSYNC_EXTI_INT_FUNCTION ( void )
{
    if ( EXTI_GetITStatus(OV7725_VSYNC_EXTI_LINE) != RESET ) 	//���EXTI_Line0��·�ϵ��ж������Ƿ��͵���NVIC 
    {
        if( Ov7725_vsync == 0 )
        {
            FIFO_WRST_L(); 	                      //����ʹFIFOд(����from����ͷ)ָ�븴λ
            FIFO_WE_H();	                        //����ʹFIFOд����
            
            Ov7725_vsync = 1;	   	
            FIFO_WE_H();                          //ʹFIFOд����
            FIFO_WRST_H();                        //����ʹFIFOд(����from����ͷ)ָ���˶�
        }
        else if( Ov7725_vsync == 1 )
        {
            FIFO_WE_L();                          //����ʹFIFOд��ͣ
            Ov7725_vsync = 2;
        }        
        EXTI_ClearITPendingBit(OV7725_VSYNC_EXTI_LINE);		    //���EXTI_Line0��·�����־λ        
    }    
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
