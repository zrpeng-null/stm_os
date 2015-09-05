#include "usart.h"
#include "xprintf.h"

#include "event_driven.h"
#include "app_task.h"
#include "hal_driver.h"

/*
 * static variable
 */
static uart_msg_t s_uart_msg;
static void (*pf_uart_handler)(u8 ch);

/*
 * 串口1中断函数
 */
void USART1_IRQHandler(void)
{
	u8 ch = USART1->DR;
	
	pf_uart_handler(ch);
}

/*
 * 接收一个字符
 */
void uart_rx_line(u8 ch)
{
	if (ch == '\r')
	{
		s_uart_msg.buf[s_uart_msg.cnt] = 0;
		s_uart_msg.rx_len = s_uart_msg.cnt;
		s_uart_msg.cnt = 0;
		event_end_post(&g_HalDriver.act, RX_LINE_SIG, (void*)&s_uart_msg);
	}
	else if (ch == '\n')
	{
		;
	}
	else
	{
		s_uart_msg.buf[s_uart_msg.cnt] = ch;
		s_uart_msg.cnt = (s_uart_msg.cnt + 1)%RX_BUF_LEN;
	}
}

/*
 * 串口接收一个文件
 */
void uart_rx_file(u8 ch)
{
	;
}

/*
 * 设置串口中断函数调用的函数指针
 */
void uart_set_handler(void (*hanlder)(u8))
{
	pf_uart_handler = hanlder;
}

/*
 * 发送一个字符
 */
void uart_sendchar(u8 ch)
{
	while((USART1->SR&USART_FLAG_TC)==0){}//等待发送完成
	USART1->DR = ch&0x01FF;
}

/*
 * 获取一个字符
 */
u8 uart_getchar(void)
{
	while ((USART1->SR&USART_FLAG_RXNE)==RESET){}//等待接收数据
    USART1->SR &= ~USART_FLAG_RXNE;//clear flag
	return (USART1->DR&0x01ff);
}

/*
 * 串口1初始化
 * @baud 串口波特率
 */
void uart_init(u32 baud)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* USARTx configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
    RCC->APB2ENR|= RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA;
    GPIOA->CRH &= 0xFFFFF00F;//TX  RX
    GPIOA->CRH |= 0x000008B0;//A10 A9
    GPIOA->ODR |= 0x0600;;

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USARTx */
    USART_Init(USART1, &USART_InitStructure);
    /* Enable the USARTx */
    USART_Cmd(USART1, ENABLE);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//接收非空中断

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 13;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	uart_set_handler(uart_rx_line);
    xfunc_out = uart_sendchar;
	xfunc_in = uart_getchar;
}

