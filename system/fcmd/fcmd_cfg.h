#ifndef _FCMD_CFG_H_
#define _FCMD_CFG_H_
/*******************************************************************************
* 用户函数命令头文件包含，函数声明
*/

#include "cmd_mem.h"
#include "mem.h"

void wfi_enable(uint8_t en);



/*******************************************************************************
 * 自定义函数命令表
 */
CmdTbl_t CmdTbl[] =
{
    //系统命令, SYSTEM_CMD_NUM和系统命令个数保持一致
    "ls",  0,
    "addr", 0,
    "help", 0,


    //用户命令
    "void md(int addr, int elem_cnt, int elem_size)", (void(*)(void))md,
    "int cmp(void *addr1, void *addr2, int elem_cnt, int elem_size)", (void(*)(void))cmp,
    "void memset(void *s, int c, size_t n)", (void(*)(void))memset,
    "void *memcpy(void *s1, const void *s2, size_t n)", (void(*)(void))memcpy,
    "long strtol(const char *nptr, char **endptr, int base)", (void(*)(void))strtol,
    "char *strstr(const char *s1, const char *s2)", (void(*)(void))strstr,

    "void *mmalloc(uint32_t size)",                    (void(*)(void))mmalloc,
    "void *mrealloc(void *rmem, uint32_t newsize)",    (void(*)(void))mrealloc,
    "void *mcalloc(uint32_t count, uint32_t size)",    (void(*)(void))mcalloc,
    "void mfree(void *rmem)",                          (void(*)(void))mfree,
    "void mem_info(void)",                               (void(*)(void))mem_info,
    "void wfi_enable(uint8_t en)",                       (void(*)(void))wfi_enable,

};
uint8_t CmdTblSize = sizeof(CmdTbl) / sizeof(CmdTbl[0]);

#endif
