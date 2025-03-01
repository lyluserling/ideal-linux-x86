#include "ipc/sem.h"
#include "tools/list.h"
#include "core/task.h"
#include "cpu/irq.h "
//Inter-Process Communication（进程间通信）的缩写
void sem_init(sem_t * sem,int init_count){
    
    sem->count = init_count;
    list_init(&sem->wait_list);
    
}
void sem_wait(sem_t * sem){
    irq_state_t state = irq_enter_protection();
    if(sem->count > 0){
        sem->count--;
    }else{
        task_t * curr = task_current();
        task_set_block(curr);//从就绪队列中移除
        list_insert_last(&sem->wait_list,&curr->wait_node);//curr是task_t结构体，wait_node是list_node_t结构体，将当前任务插入到等待队列末尾 
        task_dispatch();//就绪队列发生变化，就需要调度（这个函数）
    }
    irq_leave_protection(state);
}

void sem_notify(sem_t * sem){//通知信号量
    irq_state_t state = irq_enter_protection();

    if(list_count(&sem->wait_list)){
        list_node_t * node = list_remove_first(&sem->wait_list);
        task_t * task = list_node_parent(node,task_t,wait_node);//将list_node_t * node 转为task_t * task
        task_set_ready(task);//将task设置为就绪状态
        task_dispatch();
    }else{
        sem->count++; 
    }
    irq_leave_protection(state);
}

void sem_count(sem_t * sem){
    irq_state_t state = irq_enter_protection();
    int count = sem->count;
    irq_leave_protection(state);   
    return count;
}