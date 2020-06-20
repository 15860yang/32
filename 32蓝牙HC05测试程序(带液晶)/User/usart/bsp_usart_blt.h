#ifndef __USART2_H
#define	__USART2_H


#include "stm32f10x.h"
#include <stdio.h>


#define             BLT_USART_BAUD_RATE                       38400

#define             BLT_USARTx                                UART4
#define             BLT_USART_APBxClock_FUN                 RCC_APB1PeriphClockCmd
#define             BLT_USART_CLK                             RCC_APB1Periph_UART4
#define             BLT_USART_GPIO_APBxClock_FUN            RCC_APB2PeriphClockCmd
#define             BLT_USART_GPIO_CLK                       (RCC_APB2Periph_GPIOC )     
#define             BLT_USART_TX_PORT                         GPIOC   
#define             BLT_USART_TX_PIN                          GPIO_Pin_10
#define             BLT_USART_RX_PORT                         GPIOC 
#define             BLT_USART_RX_PIN                          GPIO_Pin_11
#define             BLT_USART_IRQ                             UART4_IRQn
#define             BLT_USART_IRQHandler                      UART4_IRQHandler


void BLT_USART_Config(void);
void Usart_SendStr_length( USART_TypeDef * pUSARTx, uint8_t *str,uint32_t strlen );
void Usart_SendString( USART_TypeDef * pUSARTx, uint8_t *str);

void bsp_USART_Process(void);
char *get_rebuff(uint16_t *len);
void clean_rebuff(void);

#endif /* __USART2_H */
