/*******************************************************************************
                Copyright (C) 2012-2022, zrpeng
                     All rights reserved

                       ledָʾ��ģ�����
--�ļ���    : led.c
--�汾      : V1.0
--����      : zrpeng
--��������  : 2013/2/14
--�޸�����  : 2013/2/14
--ע��      : jtag��swd��IO��
			  JTDO   B3
              JTRST  B4
			  JTMS   A13   SWDIO
			  JTCK   A14   SWCLK
			  JTDI   A15
********************************************************************************
--�޸���ʷ��
*******************************************************************************/
#include "led.h"


/*******************************************************************************
--�������ܣ�LEDָʾ��IO�ڳ�ʼ��
--�����б���
--�� �� ֵ����
--ע    ����1) ��ʹ��LED���ڵ�PORTxʱ��
            2) ����ģʽ: 0 ģ������
                         4 ��������
                         8 ����������
        
               ���ģʽ: 1 ���������10Mhz
                         2 ��������� 2Mhz
                         3 ���������50Mhz
        
                         5 ��©�����10Mhz
                         6 ��©����� 2Mhz
                         7 ��©�����50Mhz
        
                         9 �������죬10Mhz
                         a ��������,  2Mhz
                         b ��������, 50Mhz
        
                         d ���ÿ�©, 10Mhz
                         e ���ÿ�©,  2Mhz
                         f ���ÿ�©, 50Mhz
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
 * led����
 * sel    ledѡ��enum LED_SELECT���ֵ
 * mode   led��״̬��enum LED_MODE���ֵ
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
