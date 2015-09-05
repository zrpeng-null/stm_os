#include "sys.h"

//ʱ�����
void mco_out(void)
{
    RCC->APB2ENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA;

    GPIOA->CRH &= 0xFFFFFFF0;
    GPIOA->CRH |= 0x0000000B;//PA8
    GPIOA->ODR |= 0x0100;
    RCC->CFGR &= 0xF0FFFFFF;
    RCC->CFGR |= 0x05000000; //4:sysclk���   72MHz
    //5:�ڲ�8MHz��RC 8MHz
    //6:�ⲿ4~25MHz  8MHz
    //7:PLLʱ��2��Ƶ 36MHz
}

/*
 * �������ܣ�JTAGģʽ����      (�û�����)
 * �����б�mode :  00, JTAG_SWD_ENABLE;  01,SWD_ENABLE;  10,JTAG_SWD_DISABLE;
 * �� �� ֵ����
 * ע    ����1) JTAG_Set(JTAG_SWD_ENABLE);
 */
void jtag_set(u8 mode)
{
    u32 temp;
    temp = mode;
    temp <<= 25;
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;     //��������ʱ��
    AFIO->MAPR &= 0XF8FFFFFF; //���MAPR��[26:24]
    AFIO->MAPR |= temp;       //����jtagģʽ
}


