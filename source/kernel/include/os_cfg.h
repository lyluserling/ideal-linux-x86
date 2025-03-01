#ifndef OS_CFG_H
#define OS_CFG_H

#define GDT_TABLE_SIZE 256
#define KERNEL_SELECTOR_CS (1 * 8)
#define KERNEL_SELECTOR_DS (2 * 8)
#define KERNEL_STACK_SIZE (8*1024)//8kb
#define OS_TICKS_MS 10//10ms

#define IDLE_TASK_STACK_SIZE (1024)//1kb
#define OS_VERSION "1.0.0"
#endif