#include "event_driven.h"

#include "hal_driver.h"
#include "usart.h"
#include "app_task.h"
#include "fcmd.h"


//驱动层活动对象
HalDriver g_HalDriver;
TEvt queue_HalDriver[50];


uint8_t HalDriver_init(HalDriver *me, TEvt  *e);
uint8_t HalDriver_active(HalDriver *me, TEvt *e);

void HalDriver_ctor(HalDriver *me)
{
	fsm_ctor(&me->act.super, (state_handler)HalDriver_init);
}

uint8_t HalDriver_init(HalDriver *me, TEvt  *e)
{
	return STM_TRAN(HalDriver_active);
}


uint8_t HalDriver_active(HalDriver *me, TEvt *e)
{
	uint8_t ret;
	
	switch (e->sig)
	{
	case RX_LINE_SIG:
		{
			/*
			 * SecureCRT的键盘字符对应的十进制
			 * 上:27 91 65  下:27 91 66  右:27 91 67  左:27 91 68
			 * 退格:8(\b)	水平制表:9(\t)	删除del:127		取消:3(ctrl+c)
			 * 回车:0x0d 0x0a(\r\n)
			 */
			uart_msg_t *pmsg = (uart_msg_t *)e->para;
			
			fcmd_exec(pmsg->buf);
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	case STM_TIMEOUT_SIG:
		{
			ret = STM_RET_HANDLED;
		}
		break;
		
	case STM_ENTRY_SIG:
		{
			RTC_Configuration();
			
			ret = STM_RET_HANDLED;
		}
		break;
	
	case STM_EXIT_SIG:
		{
			ret = STM_RET_HANDLED;
		}
		break;
	
	case STM_INIT_SIG:
		{
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_RET_HANDLED;
		break;
	
	}
	
	return ret;
}

void RTC_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable the LSE OSC */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* RTC configuration -------------------------------------------------------*/
	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();

	/* Set the RTC time base to 1s */
	RTC_SetPrescaler(32767);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Alarm interrupt */
	RTC_ITConfig(RTC_IT_ALR, ENABLE);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	{
		NVIC_InitTypeDef NVIC_InitStructure;

		NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	
	{
		EXTI_InitTypeDef EXTI_InitStructure;

		/* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
		EXTI_ClearITPendingBit(EXTI_Line17);
		EXTI_InitStructure.EXTI_Line = EXTI_Line17;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
	}
}

void RTCAlarm_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
		/* Clear EXTI line17 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line17);

		/* Check if the Wake-Up flag is set */
		if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
		{
			/* Clear Wake Up flag */
			PWR_ClearFlag(PWR_FLAG_WU);
		}

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
		/* Clear RTC Alarm interrupt pending bit */
		RTC_ClearITPendingBit(RTC_IT_ALR);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}
}



/******************************************************************************
 * 测试函数
 */
void wfi_enable(uint8_t en)
{
	g_HalDriver.wfi_enable = en;
	
	/* 每隔3秒唤醒一次 */
	RTC_SetAlarm(RTC_GetCounter()+ 3);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}


