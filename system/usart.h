#ifndef _USART_H_
#define _USART_H_
#include "sys.h"
#include "xprintf.h"


#define RX_BUF_LEN  64



typedef struct
{
	u8 buf[RX_BUF_LEN];
	u16 rx_len;
	u16 cnt;
}uart_msg_t;


#if 1
#define dprintf(fmt, args...)  xprintf(fmt, ##args)
#define dputs(s)               xputs(s)
#define dputc(c)               xputc(c)
#else
#define dprintf(fmt, args...)
#define dputs(s)
#define dputc(c)
#endif


void uart_init(u32 bound);
void uart_sendchar(u8 ch);
u8 uart_getchar(void);
void uart_set_handler(void (*hanlder)(u8));

#endif

