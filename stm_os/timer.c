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


list_t evtimer_head={&evtimer_head, &evtimer_head};	//ȫ�ֵĶ�ʱ������
list_t cbtimer_head={&cbtimer_head, &cbtimer_head};	//cbtimer����Ķ�ʱ������
uint32_t tick_previous;

/******************************************************************************
*   event timer
*/
/*
 * ����һ���¼���ʱ��
 * @me		״̬������
 * @sig		�¼�
 * @para	�¼�����
 * @ms		��ʱʱ��
 * @flag	��ʱ����־�����������ظ���������ʱ��������ֹͣ��־
 *
 * return 
 */
uint8_t evtimer_add(TMsm *me, signal_t sig, void *para, uint16_t ms, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

	//���ж���û���ظ����¼���ʱ��
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
	
	//����һ���¼���ʱ�����,����
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
 * ɾ���¼���ʱ��
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
	
	return ERR_TICK_DELETE_FAILED;	//û������¼���ʱ��������ɾ��ʧ��
}

/*
 * �¼���ʱ�����ñ�־
 * @me		Ҫ���õ�״̬������
 * @sig		Ҫ���õ��¼�
 * @flag	���ñ�־:start,stop,one_shot,periodic
 */
uint8_t evtimer_set(TMsm *me, signal_t sig, uint8_t flag)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;

	//������ָ�����¼���ʱ�����
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
	return ERR_NULL_OBJECT;		//û������¼���ʱ��
}

void evtimer_update(void)
{
    evtimer_t *t;
    list_t *head;
    list_t *iter;
    list_t *iter_temp;
	uint32_t elapse_ms;

    
	//�������ŵ�ʱ��
	if (sys_tick != tick_previous)
	{
		elapse_ms = sys_tick - tick_previous;
		tick_previous = sys_tick;
	}
	else
	{
		return;	//û���ۼ�����ʱ�䣬ֱ�ӷ���
	}

	//�¼���ʱ�����������elapse_ms��ʱ��
	head = &evtimer_head;
    iter = head->next;
    while (iter != head)
    {
        t =  list_entry (iter, evtimer_t, list);
		iter_temp = iter->next;

		if (t->flag & TIMER_FLAG_START)		//��ʱ���Ѿ�����
		{
			if (t->timeout > elapse_ms)
			{
				t->timeout -= elapse_ms;
			}
			else
			{
				event_end_post((TActive*)t->me, t->e.sig, t->e.para);//�����Ѿ���ʱ���¼�
				if (t->flag & TIMER_FLAG_PERIODIC)		//�ظ�������ʱ��������һ��
				{
					t->timeout = t->reload_timeout;
				}
				else									//��������ʱ������ɾ��������
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










