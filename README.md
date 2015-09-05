# stm_os事件驱动状态机系统

2015-4-5

zrpeng

history
-------------------
版本	说明

0.02	2015-6-6 准备增加内存管理，定时器和电源模块.
		mem_malloc,mem_free
		evtimer_add,evtimer_del,evtimer_set
		cbtimer_add,cbtimer_del,cbtimer_set
		power_sleep, power_set
	
		2015-6-18 正在开发中
	
		2015-7-14 evtimer编写完毕，等待测试
	
0.01a	2015-6-1 层次状态机运行正常，串口消息正常，fcmd模块正常.
		2015-6-3 RTC定时唤醒成功
