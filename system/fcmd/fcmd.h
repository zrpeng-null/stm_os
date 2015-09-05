#ifndef _FCMD_H_
#define _FCMD_H_

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "usart.h"
#define PRINTF  dprintf	//如果是内核打印，可以改成printk
#define PUTC	dputc

#ifdef __cplusplus
extern "C" {
#endif

void fcmd_exec(uint8_t *cmd);

#ifdef __cplusplus
}
#endif

#endif


