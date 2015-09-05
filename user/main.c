#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "event_driven.h"
#include "bsp.h"

#include "app_task.h"
#include "hal_driver.h"


#define MEM_POOL_SIZE   1024*20
__align(4) uint8_t mem_pool[MEM_POOL_SIZE];

TActiveCB activeCBs[] =
{
    { &g_HalDriver.act, queue_HalDriver, ARRAY_SIZE(queue_HalDriver)},
	{ &g_bomb.act, queue_bomb, ARRAY_SIZE(queue_bomb)},
	
};

//Ӳ��������
void HardFault_Handler(void)
{
	u32 i;
	u8 t=0;
	//fault״̬�Ĵ���(@0XE000ED28)����:MMSR,BFSR,UFSR
 	xprintf("CFSR:%8X\r\n",SCB->CFSR);	//��ʾ����ֵ
 	xprintf("HFSR:%8X\r\n",SCB->HFSR);	//��ʾ����ֵ
 	xprintf("DFSR:%8X\r\n",SCB->DFSR);	//��ʾ����ֵ
 	xprintf("AFSR:%8X\r\n",SCB->AFSR);	//��ʾ����ֵ
 	while(t<5)
	{
        t++;
		for(i=0; i<0X1FFFFF; i++);
 	}
}


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//16��������ռ���ȼ�
	
	delay_init(100);//�δ�ʱ�ӣ�1��100���ж�
	uart_init(115200);
    jtag_set(JTAG_SWD_DISABLE);//JTAG_SWD_DISABLE
	
	heap_init(mem_pool, mem_pool+MEM_POOL_SIZE);
	
    PORT_ASSERT((ARRAY_SIZE(activeCBs)) == CFG_MAX_ACTIVE_OBJECT);

	
	//��������
	HalDriver_ctor(&g_HalDriver);
    Bomb_ctor(&g_bomb, 0x0d);
	
	
	//�¼�ѭ��
	event_sched();
}


void SYSCLKConfig_STOP(void)
{
	ErrorStatus HSEStartUpStatus;
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
}

void event_user(void)
{
	if (g_HalDriver.wfi_enable)
	{
		dprintf("goto stop mode\n");
		
		PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
		SYSCLKConfig_STOP();
		
		g_HalDriver.wfi_enable = 0;
		
		dprintf("wake up from stop mode\n");
	}
}


