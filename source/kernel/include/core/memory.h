#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "comm/types.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"
#include "comm/boot_info.h"

typedef struct _addr_alloc_t{//寻找起始空闲地址
    mutex_t mutex;//互斥锁
    bitmap_t bitmap;//位图

    uint32_t start;
    uint32_t size;
    uint32_t page_size;//页(快）大小

}addr_alloc_t;

void memory_init(boot_info_t *boot_info);//初始化内存管理
#endif