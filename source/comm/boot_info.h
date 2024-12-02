#ifndef BOOT_INFO_H
#define BOOT_INFO_H
#define BOOT_RAM_REGION_MAX 10
#define SECTOR_SIZE 512//扇区大小
#define SYS_KERNEL_ADDR (1024*1024)//系统内核地址
#include "types.h"
typedef struct boot_info_t
{
    //内存相关的配置信息
    struct 
    {
        uint32_t start;
        uint32_t size;
    }ram_region_cfg[BOOT_RAM_REGION_MAX];//可用的内存是一块一块的
    
    int ram_region_count;//有多少片段是有效的

}boot_info_t; //结构体在——16c作为一个全局变量

#endif