#ifndef _EVENT_H_
#define _EVENT_H_

#include "port.h"

typedef struct
{
    TMsm super;         /* derive from TMsm */

    tick_t tick_ctr;
    list_t tick_list;

    uint8_t head;
    uint8_t tail;
    uint8_t used;

    uint8_t prio;
    uint8_t prio_bit_x;
    uint8_t prio_bit_y;
    uint8_t prio_x;
    uint8_t prio_y;
} TActive;

typedef struct
{
    TActive *act;
    TEvt    *queue;
    uint8_t  end;
} TActiveCB;


#define  ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define SEND_TO_FRONT     1
#define SEND_TO_END       2

extern TEvt         STMGlobalEvent[4];
extern TActiveCB    activeCBs[CFG_MAX_ACTIVE_OBJECT];

void        event_init      (void);
uint8_t     event_end_post  (TActive *act, signal_t sig, void *para);
uint8_t     event_front_post (TActive *act, signal_t sig, void *para);
void        event_tick_isr  (void);
uint8_t     event_tick_arm  (TActive *act, tick_t ticks);
uint8_t     event_tick_disarm (TActive *act);
void        event_sched     (void);
void        event_user      (void);

#endif
