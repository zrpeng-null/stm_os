#include "event_driven.h"

/*
 * 内存配置
 */
#define MEM_STATS				/* 内存统计 */
#define MEM_ALIGN_SIZE	8		/* 内存对齐字节数 */
#define DEBUG_MEM		0		/* 内存调试 */
//#define MEM_SAFETY				/* 内存线程安全 */

#define MIN_SIZE		12		/* 最小内存分配大小 */
#define MIN_SIZE_ALIGNED     MEM_ALIGN_UP(MIN_SIZE, MEM_ALIGN_SIZE)		/* 16字节 */
#define SIZEOF_STRUCT_MEM    MEM_ALIGN_UP(sizeof(struct heap_mem), MEM_ALIGN_SIZE)	/* 16字节 */

/*
 * 内存块结构定义, 12字节
 */
#define HEAP_MAGIC 0x1ea0
struct heap_mem
{
    /* magic and used flag */
    uint16_t magic;
    uint16_t used;

    uint32_t next;
    uint32_t prev;
};

/** pointer to the heap: for alignment, heap_ptr is now a pointer instead of an array */
static uint8_t *heap_ptr;			/* memory pool pointer. */

static struct heap_mem *heap_end;/* the last entry, always unused! */
static struct heap_mem *lfree;   /* pointer to the lowest free block */

#ifdef MEM_SAFETY
static struct rt_semaphore heap_sem; /* 内存信号量 */
#endif

static uint32_t mem_size_aligned;	/* 当前最大可用内存块 */

#ifdef MEM_STATS
static uint32_t used_mem, max_mem;
#endif

static void plug_holes(struct heap_mem *mem)
{
    struct heap_mem *nmem;
    struct heap_mem *pmem;

    PORT_ASSERT((uint8_t *)mem >= heap_ptr);
    PORT_ASSERT((uint8_t *)mem < (uint8_t *)heap_end);
    PORT_ASSERT(mem->used == 0);

    /* plug hole forward */
    nmem = (struct heap_mem *)&heap_ptr[mem->next];
    if (mem != nmem &&
            nmem->used == 0 &&
            (uint8_t *)nmem != (uint8_t *)heap_end)
    {
        /* if mem->next is unused and not end of heap_ptr,
         * combine mem and mem->next
         */
        if (lfree == nmem)
        {
            lfree = mem;
        }
        mem->next = nmem->next;
        ((struct heap_mem *)&heap_ptr[nmem->next])->prev = (uint8_t *)mem - heap_ptr;
    }

    /* plug hole backward */
    pmem = (struct heap_mem *)&heap_ptr[mem->prev];
    if (pmem != mem && pmem->used == 0)
    {
        /* if mem->prev is unused, combine mem and mem->prev */
        if (lfree == mem)
        {
            lfree = pmem;
        }
        pmem->next = mem->next;
        ((struct heap_mem *)&heap_ptr[mem->next])->prev = (uint8_t *)pmem - heap_ptr;
    }
}

/**
 * This function will initialize system heap memory.
 *
 * @begin_addr the beginning address of heap memory.
 * @end_addr   the end address of heap memory.
 */
void heap_init(void *begin_addr, void *end_addr)
{
    struct heap_mem *mem;
    uint32_t begin_align = MEM_ALIGN_UP((uint32_t)begin_addr, MEM_ALIGN_SIZE);
    uint32_t end_align = MEM_ALIGN_DOWN((uint32_t)end_addr, MEM_ALIGN_SIZE);

    /*RT_DEBUG_NOT_IN_INTERRUPT;*/

    /* alignment addr */
    if ((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
            ((end_align - 2 * SIZEOF_STRUCT_MEM) >= begin_align))
    {
        /* calculate the aligned memory size */
        mem_size_aligned = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    }
    else
    {
        PRINTF("mem init, error begin address 0x%x, and end address 0x%x\n",
               (uint32_t)begin_addr, (uint32_t)end_addr);

        return;
    }

    /* point to begin address of heap */
    heap_ptr = (uint8_t *)begin_align;

    PORT_DEBUG_LOG(DEBUG_MEM, ("mem init, heap begin address 0x%x, size %d\n",
                               (uint32_t)heap_ptr, mem_size_aligned));

    /* initialize the start of the heap */
    mem        = (struct heap_mem *)heap_ptr;
    mem->magic = HEAP_MAGIC;
    mem->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev  = 0;
    mem->used  = 0;

    /* initialize the end of the heap */
    heap_end        = (struct heap_mem *)&heap_ptr[mem->next];
    heap_end->magic = HEAP_MAGIC;
    heap_end->used  = 1;
    heap_end->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    heap_end->prev  = mem_size_aligned + SIZEOF_STRUCT_MEM;

#ifdef MEM_SAFETY
    rt_sem_init(&heap_sem, "heap", 1, RT_IPC_FLAG_FIFO);
#endif

    /* initialize the lowest-free pointer to the start of the heap */
    lfree = (struct heap_mem *)heap_ptr;
}

/**
 * Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @size is the minimum size of the requested block in bytes.
 *
 * @return pointer to allocated memory or NULL if no free memory was found.
 */
void *mmalloc(uint32_t size)
{
    uint32_t ptr, ptr2;
    struct heap_mem *mem, *mem2;

    /* Note:shall not used in ISR, I never do this. */
    /*RT_DEBUG_NOT_IN_INTERRUPT;*/

    if (size == 0)
    {
        return NULL;
    }

    if (size != MEM_ALIGN_UP(size, MEM_ALIGN_SIZE))
        PORT_DEBUG_LOG(DEBUG_MEM, ("malloc size %d, but align to %d\n",
                                   size, MEM_ALIGN_UP(size, MEM_ALIGN_SIZE)));
    else
    {
        PORT_DEBUG_LOG(DEBUG_MEM, ("malloc size %d\n", size));
    }

    /* alignment size */
    size = MEM_ALIGN_UP(size, MEM_ALIGN_SIZE);

    if (size > mem_size_aligned)
    {
        PORT_DEBUG_LOG(DEBUG_MEM, ("no memory\n"));

        return NULL;
    }

    /* every data block must be at least MIN_SIZE_ALIGNED long */
    if (size < MIN_SIZE_ALIGNED)
    {
        size = MIN_SIZE_ALIGNED;
    }

#ifdef MEM_SAFETY
    /* take memory semaphore */
    rt_sem_take(&heap_sem, RT_WAITING_FOREVER);
#endif

    for (ptr = (uint8_t *)lfree - heap_ptr;
            ptr < mem_size_aligned - size;
            ptr = ((struct heap_mem *)&heap_ptr[ptr])->next)
    {
        mem = (struct heap_mem *)&heap_ptr[ptr];

        if ((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
        {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

            if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
                    (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
            {
                /* (in addition to the above, we test if another struct heap_mem (SIZEOF_STRUCT_MEM) containing
                 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
                 * -> split large block, create empty remainder,
                 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
                 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
                 * struct heap_mem would fit in but no data between mem2 and mem2->next
                 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
                 *       region that couldn't hold data, but when mem->next gets freed,
                 *       the 2 regions would be combined, resulting in more free memory
                 */
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;

                /* create mem2 struct */
                mem2       = (struct heap_mem *)&heap_ptr[ptr2];
                mem2->used = 0;
                mem2->next = mem->next;
                mem2->prev = ptr;

                /* and insert it between mem and mem->next */
                mem->next = ptr2;
                mem->used = 1;

                if (mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM)
                {
                    ((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
                }
#ifdef MEM_STATS
                used_mem += (size + SIZEOF_STRUCT_MEM);
                if (max_mem < used_mem)
                {
                    max_mem = used_mem;
                }
#endif
            }
            else
            {
                /* (a mem2 struct does no fit into the user data space of mem and mem->next will always
                 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
                 * take care of this).
                 * -> near fit or excact fit: do not split, no mem2 creation
                 * also can't move mem->next directly behind mem, since mem->next
                 * will always be used at this point!
                 */
                mem->used = 1;
#ifdef MEM_STATS
                used_mem += mem->next - ((uint8_t *)mem - heap_ptr);
                if (max_mem < used_mem)
                {
                    max_mem = used_mem;
                }
#endif
            }
            /* set memory block magic */
            mem->magic = HEAP_MAGIC;

            if (mem == lfree)
            {
                /* Find next free block after mem and update lowest free pointer */
                while (lfree->used && lfree != heap_end)
                {
                    lfree = (struct heap_mem *)&heap_ptr[lfree->next];
                }

                PORT_ASSERT(((lfree == heap_end) || (!lfree->used)));
            }
#ifdef MEM_SAFETY
            rt_sem_release(&heap_sem);
#endif
            PORT_ASSERT((uint32_t)mem + SIZEOF_STRUCT_MEM + size <= (uint32_t)heap_end);
            PORT_ASSERT((uint32_t)((uint8_t *)mem + SIZEOF_STRUCT_MEM) % MEM_ALIGN_SIZE == 0);
            PORT_ASSERT((((uint32_t)mem) & (MEM_ALIGN_SIZE - 1)) == 0);

            PORT_DEBUG_LOG(DEBUG_MEM,
                           ("allocate memory at 0x%x, size: %d\n",
                            (uint32_t)((uint8_t *)mem + SIZEOF_STRUCT_MEM),
                            (uint32_t)(mem->next - ((uint8_t *)mem - heap_ptr))));

            /* return the memory data except mem struct */
            return (uint8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }
#ifdef MEM_SAFETY
    rt_sem_release(&heap_sem);
#endif
    return NULL;
}

/**
 * This function will change the previously allocated memory block.
 *
 * @rmem pointer to memory allocated by mmalloc
 * @newsize the required new size
 *
 * @return the changed memory block address
 */
void *mrealloc(void *rmem, uint32_t newsize)
{
    uint32_t size;
    uint32_t ptr, ptr2;
    struct heap_mem *mem, *mem2;
    void *nmem;

    /*RT_DEBUG_NOT_IN_INTERRUPT;*/

    /* alignment size */
    newsize = MEM_ALIGN_UP(newsize, MEM_ALIGN_SIZE);
    if (newsize > mem_size_aligned)
    {
        PORT_DEBUG_LOG(DEBUG_MEM, ("realloc: out of memory\n"));

        return NULL;
    }

    /* allocate a new memory block */
    if (rmem == NULL)
    {
        return mmalloc(newsize);
    }

#ifdef MEM_SAFETY
    rt_sem_take(&heap_sem, RT_WAITING_FOREVER);
#endif

    if ((uint8_t *)rmem < (uint8_t *)heap_ptr ||
            (uint8_t *)rmem >= (uint8_t *)heap_end)
    {
#ifdef MEM_SAFETY
        /* illegal memory */
        rt_sem_release(&heap_sem);
#endif

        return rmem;
    }

    mem = (struct heap_mem *)((uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    ptr = (uint8_t *)mem - heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;
    if (size == newsize)
    {
#ifdef MEM_SAFETY
        /* the size is the same as */
        rt_sem_release(&heap_sem);
#endif
        return rmem;
    }

    if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size)
    {
        /* split memory block */
#ifdef MEM_STATS
        used_mem -= (size - newsize);
#endif

        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct heap_mem *)&heap_ptr[ptr2];
        mem2->magic = HEAP_MAGIC;
        mem2->used = 0;
        mem2->next = mem->next;
        mem2->prev = ptr;
        mem->next = ptr2;
        if (mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM)
        {
            ((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
        }

        plug_holes(mem2);
#ifdef MEM_SAFETY
        rt_sem_release(&heap_sem);
#endif
        return rmem;
    }
#ifdef MEM_SAFETY
    rt_sem_release(&heap_sem);
#endif

    /* expand memory */
    nmem = mmalloc(newsize);
    if (nmem != NULL) /* check memory */
    {
        memcpy(nmem, rmem, size < newsize ? size : newsize);
        mfree(rmem);
    }

    return nmem;
}

/**
 * This function will contiguously allocate enough space for count objects
 * that are size bytes of memory each and returns a pointer to the allocated
 * memory.
 *
 * The allocated memory is filled with bytes of value zero.
 *
 * @count number of objects to allocate
 * @size size of the objects to allocate
 *
 * @return pointer to allocated memory / NULL pointer if there is an error
 */
void *mcalloc(uint32_t count, uint32_t size)
{
    void *p;

    /*RT_DEBUG_NOT_IN_INTERRUPT;*/

    /* allocate 'count' objects of size 'size' */
    p = mmalloc(count * size);

    /* zero the memory */
    if (p)
    {
        memset(p, 0, count * size);
    }

    return p;
}

/**
 * This function will release the previously allocated memory block by
 * mmalloc. The released memory block is taken back to system heap.
 *
 * @rmem the address of memory which will be released
 */
void mfree(void *rmem)
{
    struct heap_mem *mem;

    /*RT_DEBUG_NOT_IN_INTERRUPT;*/

    if (rmem == NULL)
    {
        return;
    }
    PORT_ASSERT((((uint32_t)rmem) & (MEM_ALIGN_SIZE - 1)) == 0);
    PORT_ASSERT((uint8_t *)rmem >= (uint8_t *)heap_ptr &&
                (uint8_t *)rmem < (uint8_t *)heap_end);

    if ((uint8_t *)rmem < (uint8_t *)heap_ptr ||
            (uint8_t *)rmem >= (uint8_t *)heap_end)
    {
        PORT_DEBUG_LOG(DEBUG_MEM, ("illegal memory\n"));

        return;
    }

    /* Get the corresponding struct heap_mem ... */
    mem = (struct heap_mem *)((uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    PORT_DEBUG_LOG(DEBUG_MEM,
                   ("release memory 0x%x, size: %d\n",
                    (uint32_t)rmem,
                    (uint32_t)(mem->next - ((uint8_t *)mem - heap_ptr))));

#ifdef MEM_SAFETY
    /* protect the heap from concurrent access */
    rt_sem_take(&heap_sem, RT_WAITING_FOREVER);
#endif

    /* ... which has to be in a used state ... */
    PORT_ASSERT(mem->used);
    PORT_ASSERT(mem->magic == HEAP_MAGIC);
    /* ... and is now unused. */
    mem->used  = 0;
    mem->magic = 0;

    if (mem < lfree)
    {
        /* the newly freed struct is now the lowest */
        lfree = mem;
    }

#ifdef MEM_STATS
    used_mem -= (mem->next - ((uint8_t *)mem - heap_ptr));
#endif

    /* finally, see if prev or next are free also */
    plug_holes(mem);
#ifdef MEM_SAFETY
    rt_sem_release(&heap_sem);
#endif
}

#ifdef MEM_STATS
void mem_info(void)
{
    PRINTF("struct heap_mem=%d, MIN_SIZE_ALIGNED=%d\n", sizeof(struct heap_mem), MIN_SIZE_ALIGNED);
    PRINTF("total memory: %d\n", mem_size_aligned);
    PRINTF("used memory : %d\n", used_mem);
    PRINTF("maximum allocated memory: %d\n", max_mem);
}
#endif

