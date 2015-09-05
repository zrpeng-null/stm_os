/*******************************************************************************
                Copyright (C) 2012-2022, zrpeng
                     All rights reserved

                       led指示灯模块程序
--文件名    : led.c
--版本      : V1.0
--作者      : zrpeng
--创建日期  : 2013/2/14
--修改日期  : 2013/2/14
--注明      : jtag和swd的IO口
			  JTDO   B3
              JTRST  B4
			  JTMS   A13   SWDIO
			  JTCK   A14   SWCLK
			  JTDI   A15
********************************************************************************
--修改历史：
*******************************************************************************/
#include "led.h"


/*******************************************************************************
--函数功能：LED指示灯IO口初始化
--参数列表：无
--返 回 值：无
--注    明：1) 先使能LED所在的PORTx时钟
            2) 输入模式: 0 模拟输入
                         4 浮空输入
                         8 上下拉输入
        
               输出模式: 1 推挽输出，10Mhz
                         2 推挽输出， 2Mhz
                         3 推挽输出，50Mhz
        
                         5 开漏输出，10Mhz
                         6 开漏输出， 2Mhz
                         7 开漏输出，50Mhz
        
                         9 复用推挽，10Mhz
                         a 复用推挽,  2Mhz
                         b 复用推挽, 50Mhz
        
                         d 复用开漏, 10Mhz
                         e 复用开漏,  2Mhz
                         f 复用开漏, 50Mhz
*******************************************************************************/
void led_init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
 * led控制
 * sel    led选择，enum LED_SELECT里的值
 * mode   led的状态，enum LED_MODE里的值
 */
void led_ctrl(u8 sel, u8 mode)
{
	switch (sel)
	{
	case LEDWork:
		switch (mode)
		{
			case LED_ON:	LED0_L;		break;
			case LED_OFF:	LED0_H;		break;
			case LED_TOGGLE:LED0_TOGGLE;break;
		}
		break;
		
	}
}
