#ifndef _DELAY_H_
#define _DELAY_H_

#include "sys.h"

void delay_init(u16 ticks_per_sec);
void delay_ms(u32 nms);
void delay_us(u32 nus);

#endif
