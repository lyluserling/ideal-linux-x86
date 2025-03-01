#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"

#define TASK_NAME_SIZE 32
#define TASK_TIME_SLICE_DEFAULT 10 //每次定时中断是10ms，10次定时中断就是100ms
typedef struct _task_t{
 //   uint32_t * stack;
 enum{
    TASK_CREATED,
    TASK_RUNNING,
    TASK_SLEEP,
    TASK_READY,
    TASK_WAITTING,
 }state;
 
    int sleep_ticks;//睡眠计数器
    int time_ticks;//时间片计数器ticks时间片
    int slice_ticks;//递减计数器

    char name[TASK_NAME_SIZE];
    list_node_t run_node;//     就绪任务链表节点
    list_node_t all_node;//     所有任务链表节点
    list_node_t wait_node;//     等待任务链表节点

    tss_t tss;
    int tss_sel;
}task_t;//任务结构

void task_time_tick(void);

int task_init (task_t * task, uint32_t entry, uint32_t esp,const char * name);
void task_switch (task_t * from, task_t * to);//切换任务

typedef struct _task_manager_t {
    task_t * curr_task;

    list_t ready_list;//     就绪任务链表
    list_t task_list;//所有任务链表
    list_t sleep_list;//睡眠队列
    task_t first_task;
    task_t idle_task;//空闲任务
}task_manager_t;//任务管理器

void task_manager_init(void);
void task_first_init(void);
task_t * task_first_task(void);
void task_set_block(task_t * task);
void task_set_ready(task_t * task);
int sys_sched_yield(void);
void  task_dispatch(void);

void task_set_sleep(task_t * task,uint32_t ticks);//睡眠任务
void task_set_wakeup(task_t * task);//唤醒任务

void sys_sleep(uint32_t ms);

#endif /* __TASK_H__ */