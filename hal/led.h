#ifndef _LED_H_
#define _LED_H_
#include "sys.h"

enum LED_MODE
{
	LED_OFF=0,
	LED_ON,
	LED_TOGGLE=2,
};

enum LED_SELECT
{
	LEDWork=0,
};

#define LED0_H			GPIOA->BSRR|=(1<<1)
#define LED0_L			GPIOA->BRR|=(1<<1)
#define LED0_TOGGLE		GPIOA->ODR^=(1<<1)


void led_init(void);//³õÊ¼»¯
void led_ctrl(u8 sel, u8 mode);


#endif

