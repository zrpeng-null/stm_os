#include "bsp.h"

#include "app_task.h"


//炸弹活动对象
Bomb g_bomb;
TEvt queue_bomb[3];


/******************************************************************************
 *  app任务
 */
static uint8_t Bomb_init(Bomb *me, TEvt  *e);
static uint8_t Bomb_active(Bomb *me, TEvt *e);

static uint8_t Bomb_A(Bomb *me, TEvt  *e);
static uint8_t Bomb_A1(Bomb *me, TEvt  *e);
static uint8_t Bomb_A2(Bomb *me, TEvt  *e);

static uint8_t Bomb_B(Bomb *me, TEvt  *e);
static uint8_t Bomb_B1(Bomb *me, TEvt  *e);
static uint8_t Bomb_B2(Bomb *me, TEvt  *e);

static uint8_t Bomb_C(Bomb *me, TEvt  *e);


void Bomb_ctor(Bomb *me, uint8_t defuse)
{
    hsm_ctor(&me->act.super, (state_handler)Bomb_init);
}

/*..........................................................................*/
static uint8_t Bomb_init(Bomb *me, TEvt  *e)
{
    (void)e;
	xprintf("Bomb_init\n");
    return STM_TRAN(Bomb_active);
}

/*..........................................................................*/
static uint8_t Bomb_active(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
	switch (e->sig)
	{
	
	case ACTIVE_OK_SIG:
		{
			xprintf("Bomb_active: active_ok_sig\n");
			ret = STM_RET_HANDLED;
		}
		break;
	
	case STM_TIMEOUT_SIG:
		{
			xprintf("timeout:%d\n", SysTick->VAL);
			event_tick_arm(&me->act, 100);
			ret = STM_RET_HANDLED;
		}
		break;
		
	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_active entry\n");
			event_tick_arm(&me->act, 100);
			ret = STM_RET_HANDLED;
		}
		break;
	
	case STM_INIT_SIG:
		{
			xprintf("Bomb_active: STM_INIT_SIG\n");
			
			ret = STM_TRAN(Bomb_A);
		}
		break;
		
	case STM_EXIT_SIG:
		{
			xprintf("Bomb_active exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(hsm_top);
		break;
	}
	
	return ret;
}

/*..........................................................................*/
static uint8_t Bomb_A(Bomb *me, TEvt  *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {
	
	case A1_SIG:
		{
			xprintf("Bomb_A: A1_SIG\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
	
	case STM_INIT_SIG:
		{
			xprintf("Bomb_A:STM_INIT_SIG\n");
			
			ret = STM_TRAN(Bomb_A1);
		}
		break;
		
	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_A entry\n");
			ret = STM_RET_HANDLED;
		}
		break;
		
	case STM_EXIT_SIG:
		{
			xprintf("Bomb_A exit\n");
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_active);
		break;
    }
	
    return ret;
}

/*..........................................................................*/
static uint8_t Bomb_A1(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {

	case A11_SIG:
		{
			xprintf("Bomb_A1: A11_SIG\n");
			
			ret = STM_TRAN(Bomb_B);
		}
		break;
		
	case A12_SIG:
		{
			xprintf("Bomb_A1: A12_SIG\n");
			
			ret = STM_TRAN(Bomb_A2);
		}
		break;
		
		
		
	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_A1 enter\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_EXIT_SIG:
		{
			xprintf("Bomb_A1 exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_A);
		break;

    }
	
    return ret;
}

/*..........................................................................*/
static uint8_t Bomb_A2(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {

	case A21_SIG:
		{
			xprintf("Bomb_A2: A21_SIG\n");
			
			ret = STM_TRAN(Bomb_B2);
		}
		break;
		
	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_A2 enter\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_EXIT_SIG:
		{
			xprintf("Bomb_A2 exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_A);
		break;

    }
	
    return ret;
}

/*..........................................................................*/
static uint8_t Bomb_B(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {

	case B1_SIG:
		{
			xprintf("Bomb_B: B1_SIG\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
	
	case STM_INIT_SIG:
		{
			xprintf("Bomb_B:STM_INIT_SIG\n");
			
			ret = STM_TRAN(Bomb_B1);
		}
		break;
		
	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_B enter\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_EXIT_SIG:
		{
			xprintf("Bomb_B exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_active);
		break;

    }
	
    return ret;
}

/*..........................................................................*/
static uint8_t Bomb_B1(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {

	case B11_SIG:
		{
			xprintf("Bomb_B1: B11_SIG\n");

			ret = STM_TRAN(Bomb_B2);
		}
		break;
	
	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_B1 enter\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_EXIT_SIG:
		{
			xprintf("Bomb_B1 exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_B);
		break;

    }
	
    return ret;
}

/*..........................................................................*/
static uint8_t Bomb_B2(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {

	case B21_SIG:
		{
			xprintf("Bomb_B2: B21_SIG\n");

			ret = STM_TRAN(Bomb_C);
		}
		break;

	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_B2 enter\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_EXIT_SIG:
		{
			xprintf("Bomb_B2 exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_B);
		break;

    }
	
    return ret;
}



/*..........................................................................*/
static uint8_t Bomb_C(Bomb *me, TEvt *e)
{
	uint8_t ret;
	
    switch (e->sig)
    {

	case C1_SIG:
		{
			xprintf("Bomb_C: C1_SIG\n");

			ret = STM_TRAN(Bomb_A2);
		}
		break;
		
	case C2_SIG:
		{
			xprintf("Bomb_C: C2_SIG\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_ENTRY_SIG:
		{
			xprintf("Bomb_C enter\n");

			ret = STM_RET_HANDLED;
		}
		break;

	case STM_EXIT_SIG:
		{
			xprintf("Bomb_C exit\n");
			
			ret = STM_RET_HANDLED;
		}
		break;
		
	default:
		ret = STM_FATHER(Bomb_active);
		break;

    }
	
    return ret;
}
