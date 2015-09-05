#include "event_driven.h"

static uint8_t  rdy_grp;
static uint8_t  rdy_tbl[CFG_RDY_TBL_SIZE];
static list_t   tick_head = {&tick_head, &tick_head};
static const uint8_t  A_ATTR B_ATTR map_tbl[256] =
{
    0u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x00 to 0x0F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x10 to 0x1F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x20 to 0x2F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x30 to 0x3F */
    6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x40 to 0x4F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x50 to 0x5F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x60 to 0x6F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x70 to 0x7F */
    7u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x80 to 0x8F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x90 to 0x9F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xA0 to 0xAF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xB0 to 0xBF */
    6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xC0 to 0xCF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xD0 to 0xDF */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xE0 to 0xEF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u  /* 0xF0 to 0xFF */
};
TEvt STMGlobalEvent[4] =
{
    { STM_EMPTY_SIG, 0},
    { STM_ENTRY_SIG, 0},
    { STM_EXIT_SIG,  0},
    { STM_INIT_SIG,  0}
};

/*
 * Initialize the activeCBs[]
 *
 * Note(s)  active object's prio is the index of activeCBs[]
 */
void event_init (void)
{
    TActive *act;
    uint8_t i;

    for (i = 0; i < CFG_MAX_ACTIVE_OBJECT; i++)
    {
        act = activeCBs[i].act;
        PORT_ASSERT (act != 0);

        act->prio   = i;
        act->head   = 0;
        act->tail   = 0;
        act->used   = 0;

        act->prio_x = i & 0x7;
        act->prio_y = i >> 3;
        act->prio_bit_y = (uint8_t) (1 << act->prio_y);
        act->prio_bit_x = (uint8_t) (1 << act->prio_x);
    }
}

/*
 * Post event to specify active object
 *
 * @act   active object
 * @sig   signal num
 * @para  signal param
 * @opt   send to front or back of msg queue, SEND_TO_FRONT  SEND_TO_END
 *
 * Note(s) it might be called in interrupt.
 */
uint8_t event_post (TActive *act, signal_t sig, void *para, uint8_t opt_send)
{
    TActiveCB *acb = &activeCBs[act->prio];
    PORT_SR_ALLOC();

    PORT_CPU_DISABLE();

    if (act->used >= acb->end)
    {
        PORT_CPU_ENABLE();
        return ERR_EVENT_EXHAUSTED;
    }

    if (opt_send == SEND_TO_END)
    {
        acb->queue[act->tail].sig = sig;
        acb->queue[act->tail].para = para;

        act->tail++;
        if (act->tail >= acb->end)
        {
            act->tail = 0;
        }
    }
    else
    {
        if (act->head == 0)
        {
            act->head = acb->end;
        }
        act->head--;

        acb->queue[act->head].sig = sig;
        acb->queue[act->head].para = para;
    }

    act->used++;
    if (act->used == 1)
    {
        rdy_grp |= acb->act->prio_bit_y;
        rdy_tbl[acb->act->prio_y] |= acb->act->prio_bit_x;
    }

    PORT_CPU_ENABLE();

    return ERR_SUCCESS;
}
/*
 * Post an event (FIFO) to active object
 */
uint8_t event_end_post (TActive *act, signal_t sig, void *para)
{
    return event_post (act, sig, para, SEND_TO_END);
}
/*
 * Post an event (LIFO) to active object
 */
uint8_t event_front_post (TActive *act, signal_t sig, void *para)
{
    return event_post (act, sig, para, SEND_TO_FRONT);
}

/*
 * STM_TIMEOUT_SIG tick list isr
 *
 * Note(s)  when active object's tick note timeout, STM_TIMEOUT_SIG will be trigger
 */
void event_tick_isr (void)
{
    TActive *act;
    list_t *head;
    list_t *iter;
    list_t *iter_temp;

    head = &tick_head;
    iter = head->next;

    /* if list is not empty, tick list is a bi-link circulate list */
    while (iter != head)
    {
        act =  list_entry (iter, TActive, tick_list);
        iter_temp =  iter->next;

        if (act->tick_ctr)
        {
            act->tick_ctr--;
            if (act->tick_ctr == 0)
            {
                list_delete (iter);
                event_end_post (act, STM_TIMEOUT_SIG, NULL);
            }
        }

        iter = iter_temp;
    }
}

/*
 * attach specify active object's tick node to tick_head list
 *
 * @act     specify active object
 * @ticks  ticks be timeout
 *
 * Returns  ERR_SUCCESS or ERR_TICK_DELETE_FAILED
 *
 * Note(s)  when ticks timeout, @act's STM_TIMEOUT_SIG will trigger
 */
uint8_t event_tick_arm (TActive *act, tick_t ticks)
{
    uint8_t r;
    PORT_SR_ALLOC();

    if (ticks == 0)
    {
        return ERR_TICK_ADD_FAILED;
    }

    PORT_CPU_DISABLE();

    if (act->tick_ctr == 0)
    {
        act->tick_ctr = ticks;
        list_insert (&tick_head, &act->tick_list);
        r = ERR_SUCCESS;
    }
    else
    {
        r = ERR_TICK_ADD_FAILED;
    }

    PORT_CPU_ENABLE();

    return r;
}

/*
 * detach specify active object's tick node from tick_head list
 *
 * @act     specify active object
 * @ticks  ticks be timeout
 *
 * Returns  ERR_SUCCESS or ERR_TICK_DELETE_FAILED
 */
uint8_t event_tick_disarm (TActive *act)
{
    uint8_t r;
    PORT_SR_ALLOC();

    PORT_CPU_DISABLE();

    if (act->tick_ctr)
    {
        list_delete (&act->tick_list);
        act->tick_ctr = 0;
        r = ERR_SUCCESS;
    }
    else
    {
        r = ERR_TICK_DELETE_FAILED;
    }

    PORT_CPU_ENABLE();

    return r;
}
void event_dummy (void) {}
void event_sched (void)
{
    TActiveCB *acb;
    TActive *act;
    TEvt evt;
    uint8_t y;
    uint8_t prio_highest_rdy;
    uint8_t i;
    PORT_SR_ALLOC();

    /* init control block */
    event_init();

    /* init state machine */
    for (i = 0; i < CFG_MAX_ACTIVE_OBJECT; i++)
    {
        act = activeCBs[i].act;
        act->super.vptr->init(&act->super, 0);
    }

    while (1)
    {
        /* update timer(evtimer & cbtimer) list. */
        evtimer_update();

        PORT_CPU_DISABLE();

        if (rdy_grp)                /* anyone active object get events. */
        {
            /* find the highest ready active object and activeCB. */
            y = map_tbl[rdy_grp];
            prio_highest_rdy = ( (y << 3) + map_tbl[rdy_tbl[y]]);

            acb = &activeCBs[prio_highest_rdy];
            act = activeCBs[prio_highest_rdy].act;

            /* pick a event from queue head. */
            act->used--;
            if (act->used == 0) /* no event in queue, clear active object's rdy bit */
            {
                rdy_tbl[act->prio_y] &= ~act->prio_bit_x;
                if (rdy_tbl[act->prio_y] == 0)
                {
                    rdy_grp &= ~act->prio_bit_y;
                }
            }

            evt.sig = acb->queue[act->head].sig;
            evt.para = acb->queue[act->head].para;
            act->head++;
            if (act->head >= acb->end)
            {
                act->head = 0;
            }

            PORT_CPU_ENABLE();

            /* process this event */
            act->super.vptr->dispatch(&act->super, &evt);
        }
#ifdef POWER_SAVING
        else
        {
            PORT_CPU_ENABLE();

            event_dummy();      /* ensure interrupt enable. */

            PORT_CPU_DISABLE();

            if (rdy_grp == 0)
            {
                PORT_CPU_ENABLE();

                event_user();
            }
            else
            {
                PORT_CPU_ENABLE();
            }
        }
#endif
    }
}
