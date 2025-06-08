#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "comm/types.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"
#include "comm/boot_info.h"

#define MEM_EBDA_START              0x00080000
#define MEM_EXT_START               (1024*1024)
#define MEM_EXT_END                 (128*1024*1024 - 1)
#define MEM_PAGE_SIZE               4096        // 和页表大小一致

#define MEMORY_TASK_BASE            (0x80000000)        // 进程起始地址空间
#define MEM_TASK_STACK_TOP          (0xE0000000)        // 初始栈的位置  
#define MEM_TASK_STACK_SIZE         (MEM_PAGE_SIZE * 500)   // 初始500KB栈
#define MEM_TASK_ARG_SIZE           (MEM_PAGE_SIZE * 4)     // 参数和环境变量占用的大小


typedef struct _addr_alloc_t{//寻找起始空闲地址
    mutex_t mutex;//互斥锁
    bitmap_t bitmap;//位图

    uint32_t start;
    uint32_t size; //总大小
    uint32_t page_size;//页(快）大小

}addr_alloc_t;

typedef struct _memory_map_t{
    void * vstart;//虚拟地址
    void * vend;//虚拟结束地址
    void * pstart;//物理地址
    uint32_t perm;

}memory_map_t;


void memory_init(boot_info_t *boot_info);//初始化内存管理
#endif