#ifndef __SYSTICK_H
#define __SYSTICK_H



#include "stm32f10x.h"

#define TASK_ENABLE 0
#define NumOfTask 3

#define Delay_ms(x) Delay_us(x)	 //µ¥Î»ms

void mdelay(unsigned long nTime);


int get_tick_count(unsigned long *count);
void SysTick_Init( void );
void TimingDelay_Decrement( void );
void Delay_us ( __IO u32 nTime );

void TimeStamp_Increment(void);
#endif /* __SYSTICK_H */
