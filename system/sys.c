#include "sys.h"

//时钟输出
void mco_out(void)
{
    RCC->APB2ENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA;

    GPIOA->CRH &= 0xFFFFFFF0;
    GPIOA->CRH |= 0x0000000B;//PA8
    GPIOA->ODR |= 0x0100;
    RCC->CFGR &= 0xF0FFFFFF;
    RCC->CFGR |= 0x05000000; //4:sysclk输出   72MHz
    //5:内部8MHz的RC 8MHz
    //6:外部4~25MHz  8MHz
    //7:PLL时钟2分频 36MHz
}

/*
 * 函数功能：JTAG模式设置      (用户调用)
 * 参数列表：mode :  00, JTAG_SWD_ENABLE;  01,SWD_ENABLE;  10,JTAG_SWD_DISABLE;
 * 返 回 值：无
 * 注    明：1) JTAG_Set(JTAG_SWD_ENABLE);
 */
void jtag_set(u8 mode)
{
    u32 temp;
    temp = mode;
    temp <<= 25;
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;     //开启辅助时钟
    AFIO->MAPR &= 0XF8FFFFFF; //清除MAPR的[26:24]
    AFIO->MAPR |= temp;       //设置jtag模式
}


