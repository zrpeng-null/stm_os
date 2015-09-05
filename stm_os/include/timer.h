#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * timer flag
 */
#define TIMER_FLAG_STOP			0x00
#define TIMER_FLAG_START		0x01
#define TIMER_FLAG_ONE_SHOT		0x00
#define TIMER_FLAG_PERIODIC		0x02


typedef struct
{
    TMsm *me;
    list_t list;
    uint8_t flag;
    uint16_t timeout;		/* ms */
    uint16_t reload_timeout;

    TEvt e;
} evtimer_t;

typedef struct
{
    TMsm *me;
    list_t list;
    uint8_t flag;
    uint16_t timeout;

    uint32_t counter;
    uint8_t (*proc)(uint32_t counter);
} cbtimer_t;


extern uint32_t sys_tick;


uint8_t evtimer_add(TMsm *me, signal_t sig, void *para, uint16_t ms, uint8_t flag);
uint8_t evtimer_del(TMsm *me, signal_t sig);
uint8_t evtimer_set(TMsm *me, signal_t sig, uint8_t flag);
void evtimer_update(void);

#endif
