/*

uint8_t etimer_add(TMsm *me, signal_t sig, void *para, uint16_t ms)
{
	
}

uint8_t etimer_delete(TMsm *me, signal_t sig)
{

}

typedef uint8_t (*pfCBHandler)(void *param);

typedef struct
{
	uint8_t timer_id;
	TMsm *me;
	uint16_t timeout;
	uint8_t flag;
	pfCBHandler func;
	
	list_t cbtimer_list;
}cbtimer_t


list_t cbtimer_head={&cbtimer_head, &cbtimer_head};

uint8_t cbtimer_add(TMsm *me, uint8_t *timer_id, pfTimerHandler func, uint16_t timeout)

uint8_t cbtimer_delete(TMsm *me, uint8_t timer_id)

uint8_t cbtimer_set(TMsm *me, uint8_t timer_id)

*/
#include "event_driven.h"


list_t evtimer_head={&evtimer_head, &evtimer_head};	//全局的定时器链表
list_t cbtimer_head={&cbtimer_head, &cbtimer_head};	//cbtimer任务的定时器链表
uint32_t tick_previous;

/******************************************************************************
*   event timer
*/
/*
 * 增加一个事件定时器
 * @me		状态机对象
 * @sig		事件
 * @para	事件参数
 * @ms		超时时间
 * @flag	定时器标志：单触发和重复触发，定时器启动和停止标志
 *
 * return 
 */
uint8_t evtimer_add(TMsm *me, signal_t sig, void *para, uint16_t ms, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

	//先判断有没有重复的事件定时器
    head = &evtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t = list_entry(iter, evtimer_t, list);

        if ((t->me == me) && (t->e.sig == sig))
        {
            return ERR_EXISTING_TIMER;
        }

        iter = iter->next;
    }
	
	//申请一个事件定时器结点,插入
	t = (evtimer_t*)mmalloc(sizeof(evtimer_t));
	if (t == NULL)
	{
		return ERR_NO_MEMORY;
	}
	t->me      = me;
	t->e.sig   = sig;
	t->e.para  = para;
	t->timeout = ms;
	t->flag    = flag;
	list_insert(head, &t->list);

	return ERR_SUCCESS;
}

/*
 * 删除事件定时器
 */
uint8_t evtimer_del(TMsm *me, signal_t sig)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

    head = &evtimer_head;
    iter = head->next;

    while (iter != head)
    {
        t =  list_entry (iter, evtimer_t, list);

        if ((t->me == me) && (t->e.sig == sig))
        {
			list_delete(&t->list);
			mfree(t);
            return ERR_SUCCESS;
        }

        iter = iter->next;
    }
	
	return ERR_TICK_DELETE_FAILED;	//没有这个事件定时器，返回删除失败
}

/*
 * 事件定时器设置标志
 * @me		要设置的状态机对象
 * @sig		要设置的事件
 * @flag	设置标志:start,stop,one_shot,periodic
 */
uint8_t evtimer_set(TMsm *me, signal_t sig, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

	//遍历到指定的事件定时器结点
    head = &evtimer_head;
    iter = head->next;
	
    while (iter != head)
    {
        t =  list_entry (iter, evtimer_t, list);

        if ((t->me == me) && (t->e.sig == sig))
        {
			t->flag = flag;
            return ERR_SUCCESS;
        }

        iter = iter->next;
    }
	return ERR_NULL_OBJECT;		//没有这个事件定时器
}

void evtimer_update(void)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;
    list_t *iter_temp;
	uint32_t elapse_ms;

    
	//计算消逝的时间
	if (sys_tick != tick_previous)
	{
		elapse_ms = sys_tick - tick_previous;
		tick_previous = sys_tick;
	}
	else
	{
		return;	//没有累计消逝时间，直接返回
	}

	//事件定时器结点消逝了elapse_ms的时间
	head = &evtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t =  list_entry (iter, evtimer_t, list);
		iter_temp = iter->next;

		if (t->flag & TIMER_FLAG_START)		//定时器已经启动
		{
			if (t->timeout > elapse_ms)
			{
				t->timeout -= elapse_ms;
			}
			else
			{
				event_end_post((TActive*)t->me, t->e.sig, t->e.para);//发出已经超时的事件
				if (t->flag & TIMER_FLAG_PERIODIC)		//重复触发定时器，再来一次
				{
					t->timeout = t->reload_timeout;
				}
				else									//单触发定时器，就删除这个结点
				{
					list_delete(&t->list);
					mfree(t);
				}
			}
		}
		
        iter = iter_temp;
    }
}


/******************************************************************************
*  callback function timer
*/
uint8_t cbtimer_add(TMsm *me)
{
	return 0;
}










