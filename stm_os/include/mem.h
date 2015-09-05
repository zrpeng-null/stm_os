#ifndef _MEM_H_
#define _MEM_H_

void heap_init(void *begin_addr, void *end_addr);
void *mmalloc(uint32_t size);
void *mrealloc(void *rmem, uint32_t newsize);
void *mcalloc(uint32_t count, uint32_t size);
void mfree(void *rmem);
void mem_info(void);

#endif


