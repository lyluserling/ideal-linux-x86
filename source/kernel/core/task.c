#include "core/task.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "cpu/cpu.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "cpu/mmu.h"

static uint32_t idle_task_stack[IDLE_TASK_STACK_SIZE];//空闲任务栈

static task_manager_t task_manager;
static int tss_init (task_t * task, uint32_t entry, uint32_t esp){
    int tss_sel = gdt_alloc_desc();//分配一个新的TSS描述符
    if(tss_sel < 0){
        log_printf("alloc tss failed\n");
        return -1;
    }
    segment_desc_set(tss_sel,(uint32_t)&task->tss,sizeof(tss_t),//设置TSS段描述符
        SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS
        );


    kernel_memset(&task->tss,0,sizeof(tss_t));
    task->tss.eip = entry;
    task->tss.esp = task->tss.esp0 = esp;//esp0 is the same as esp
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;
    task->tss.cs = KERNEL_SELECTOR_CS;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    
    task->tss_sel = tss_sel;//设置task的TSS选择子
     //其他字段不需要设置
    return 0;
}

int task_init (task_t * task, uint32_t entry, uint32_t esp,const char *name){
    ASSERT(task != (task_t *)0);
    tss_init(task, entry, esp);
//    uint32_t *pesp = (uint32_t *)esp;//栈顶指针
//    if (pesp){
//     *(--pesp) = entry;
//       *(--pesp) = 0;
//       *(--pesp) = 0;
//       *(--pesp) = 0;
//       *(--pesp) = 0;
//       task->stack = (uint32_t)pesp;
//    }
    kernel_strncpy(task->name,name,TASK_NAME_SIZE);//设置task的名字
    task->state = TASK_CREATED;//设置task的状态为创建
    task->sleep_ticks = 0;//init task的睡眠计数器
    task->time_ticks = TASK_TIME_SLICE_DEFAULT;//设置task的运行计数器初始值
    task->slice_ticks = task->time_ticks;//设置task的运行时间片

    list_node_init(&task->all_node);//初始化task的all_node
    list_node_init(&task->run_node);//初始化task的run_node
    list_node_init(&task->wait_node);//初始化task的wait_node

    irq_state_t state = irq_enter_protection();
    task_set_ready(task);
    list_insert_last(&task_manager.task_list,&task->all_node);//将task加入到所有任务链表中
    irq_leave_protection(state);
    return 0;
}

void simple_switch (uint32_t *from, uint32_t * to);

void  task_switch (task_t * from, task_t * to){
    switch_to_tss(to->tss_sel);
   //simple_switch(&from->stack,to->stack);
}

void task_first_init(void){
    void first_task_entry(void);
    uint32_t first_start = (uint32_t)first_task_entry;
    task_init(&task_manager.first_task,0,0,"first task");//空任务
    write_tr(task_manager.first_task.tss_sel);//这个的选择子放在first_task的tss_sel中,初始化完成后
    log_printf("first_task_init slice_ticks: %d", task_manager.first_task.slice_ticks);
    task_manager.curr_task = &task_manager.first_task;

    mmu_set_page_dir(task_manager.first_task.tss.cr3);//设置页目录
}


task_t * task_first_task(void){
    return &task_manager.first_task;
}

static void idle_task_entry(void){
    for(;;){
        hlt();
    }
}

void task_manager_init(void){
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);
    task_init(&task_manager.idle_task,(uint32_t)idle_task_entry,(uint32_t)(idle_task_stack + IDLE_TASK_STACK_SIZE), "idle task");
    task_manager.curr_task = (task_t *)0;
}

void task_set_ready(task_t * task){
    if(task == &task_manager.idle_task){ //空闲任务
        return;
    }
    list_insert_last(&task_manager.ready_list,&task->run_node);//将task加入到就绪任务链表中
    task->state = TASK_READY;
}

void task_set_block(task_t * task){
    if(task == &task_manager.idle_task){ //空闲任务的话就不进行移除操作
        return;
    }
    list_remove(&task_manager.ready_list,&task->run_node);
}

task_t * task_next_run(void){
    if(list_count(&task_manager.ready_list) == 0){ //如果就绪队列为空
        return &task_manager.idle_task; //返回空闲任务
    }
    list_node_t * task_node = list_first(&task_manager.ready_list);//获取就绪队列的第一个节点
    return list_node_parent(task_node,task_t,run_node);//获取头部的run_node
}
task_t * task_current(void){
 return task_manager.curr_task;
}

int sys_sched_yield(void){

irq_state_t state = irq_enter_protection();

    if(list_count(&task_manager.ready_list) > 1){ 
        task_t * current_task = task_current();//当前任务
        task_set_block(current_task);//将当前任务从就绪任务链表中移除
        task_set_ready(current_task);//(但是加到了尾部)

        task_dispatch();
    }
    irq_leave_protection(state);
    return 0;
}

void  task_dispatch(void){//调度器
    irq_state_t state = irq_enter_protection();
    task_t * to = task_next_run();//获取头部的run任务

    if(to != task_manager.curr_task){//如果头部的任务不是当前任务
        task_t * from = task_current();//当前任务
        task_manager.curr_task = to;//更新当前任务5
        to->state = TASK_RUNNING;//设置新任务的状态为运行
        task_switch(from,to);//切换任务
        log_printf("Dispatching task: %s", to->name);

    }
    irq_leave_protection(state);
}

void task_time_tick(void){
    task_t * curr_task = task_current();

    if(--curr_task->slice_ticks == 0){//如果时间片用完
        log_printf(" ******************time slice end*************************** ");//任务1 
        curr_task->slice_ticks = curr_task->time_ticks;//重置时间片
        task_set_block(curr_task);//将当前任务从就绪任务链表中移除
        task_set_ready(curr_task);//(但是加到了尾部)
        task_dispatch();
    }

    list_node_t * curr = list_first(&task_manager.sleep_list);
    while(curr){//遍历睡眠任务链表
        list_node_t * next = list_node_next(curr);
        task_t * task = list_node_parent(curr,task_t,run_node);
        if(--task->sleep_ticks == 0){
            task_set_wakeup(task);
            task_set_ready(task);
        }
        curr = next;
    }
    task_dispatch(); //每次更新run_node后都要调度一次
}

void task_set_sleep(task_t * task,uint32_t ticks){
    if(ticks == 0){
        return ;
    }
    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    //***********************************************这一行错了，导致后面睡眠任务不能被唤醒 */
    //list_insert_last(&task_manager.sleep_list,&task->all_node);//将task加入到睡眠任务链表中
    list_insert_last(&task_manager.sleep_list,&task->run_node);
}

void task_set_wakeup(task_t * task){
    list_remove(&task_manager.sleep_list,&task->run_node);

}

void sys_sleep(uint32_t ms){
    irq_state_t state = irq_enter_protection();

    task_set_block(task_manager.curr_task);//将当前任务从就绪任务链表中移除
   // task_set_sleep(task_manager.curr_task, ms+5 / OS_TICKS_MS); //OS_TICKS_MS时钟节拍
    task_set_sleep(task_manager.curr_task, (ms+(OS_TICKS_MS-1)) / OS_TICKS_MS); //向上取整

    task_dispatch();//切换进程
    irq_leave_protection(state);
}