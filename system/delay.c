#include "delay.h"
#include "event_driven.h"

static u8  fac_us = 0; //1us延时,SysTick计数多少
static u16 fac_ms = 0; //1ms延时,SysTick计数多少

uint32_t sys_tick;

void SysTick_Handler(void)
{
    event_tick_isr();
    sys_tick++;
}

/*
 * 设置滴答时钟
 * @ticks_per_sec  1秒中断多少次
 * note 1) SysTick是向下计数, SysTick->VAL计到0时，
 *         又会从SysTick->LOAD中取出重装值，再继续向下计数.
 */
void delay_init(u16 ticks_per_sec)
{
    RCC_ClocksTypeDef rcc_clocks;
    u32 cnts;

    RCC_GetClocksFreq(&rcc_clocks);
    cnts = (u32)rcc_clocks.SYSCLK_Frequency / 8 / ticks_per_sec;
    SysTick_Config(cnts);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);//HCLK/8

    fac_us = rcc_clocks.SYSCLK_Frequency / 1000000l / 8;
    fac_ms = fac_us * 1000;
}

/*
 * @total_ticks  需要延时多少个systick
 * note  保证SysTick已经在正常计数工作了
 */
static void delay_ticks(u32 total_ticks)
{
    u32 cnt, tnew, told;
    u32 reload = SysTick->LOAD;

    cnt = 0;
    told = SysTick->VAL;

    for (;;)
    {
        tnew = SysTick->VAL;
        if (tnew != told)
        {
            if (tnew < told)
            {
                cnt += told - tnew;
            }
            else
            {
                cnt += reload + told - tnew;
            }
            told = tnew;
            if (cnt > total_ticks)
            {
                break;
            }
        }
    }
}

/*
 * @nms  延时多少ms
 */
void delay_ms(u32 nms)
{
    if (nms)
    {
        delay_ticks(nms * fac_ms - 1);
    }
}

/*
 * @nus 延时多少us
 */
void delay_us(u32 nus)
{
    if (nus)
    {
        delay_ticks(nus * fac_us - 1);
    }
}



