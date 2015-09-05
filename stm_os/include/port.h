#ifndef _PORT_H_
#define _PORT_H_

#include "usart.h"
#include "fcmd.h"

#define CFG_MAX_ACTIVE_OBJECT   2       /* max active object 64 */

#if (CFG_MAX_ACTIVE_OBJECT > 64)
#error  "CFG_MAX_ACTIVE_OBJECT is 64"
#endif

#define CFG_LOWEST_PRIO         63u

#define CFG_RDY_TBL_SIZE        ((CFG_LOWEST_PRIO) / 8u + 1u)

#define POWER_SAVING

typedef unsigned int CPU_SR;

typedef unsigned char signal_t;

CPU_SR  CPU_SR_Save (void);
void    CPU_SR_Restore (CPU_SR sr);

#define PORT_SR_ALLOC()         CPU_SR cpu_sr = (CPU_SR)0
#define PORT_CPU_DISABLE()      { cpu_sr = CPU_SR_Save();}      /* disable cpu interrupt */
#define PORT_CPU_ENABLE()       { CPU_SR_Restore(cpu_sr);}      /* enable cpu interrupt */


#define PORT_ASSERT(EX)                                                       \
	if (!(EX))                                                                \
	{                                                                         \
		volatile char dummy = 0;                                              \
		CPU_SR_Save();                                                        \
		PRINTF("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__);\
		while (dummy == 0);                                                   \
	}

#define PORT_DEBUG_LOG(type, message)                                         \
	do                                                                        \
	{                                                                         \
		if (type)                                                             \
			PRINTF message;                                                   \
	}while (0)

#define MEM_ALIGN_UP(size, align)	(((size) + (align) - 1) & ~((align) - 1))

#define MEM_ALIGN_DOWN(size, align)	((size) & ~((align) - 1))

#define A_ATTR                  /*rom para 1*/
#define B_ATTR                  /*rom para 2*/

#endif


