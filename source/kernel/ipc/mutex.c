#include "ipc/mutex.h"
#include "cpu/irq.h"
#include "core/task.h"
void mutex_init(mutex_t * mutex){
    mutex->owner = (task_t *)0;
    mutex->locked_count = 0;
    list_init(&mutex->wait_list);
}

void mutex_lock(mutex_t * mutex){
    irq_state_t state = irq_enter_protection();
    task_t * curr = task_current();
    if(mutex->locked_count == 0){ // mutex is not locked
        mutex->locked_count++;
        mutex->owner = curr;
    }else if(mutex->owner == curr){
        mutex->locked_count++;//重复锁定
    }else{
        task_set_block(curr);
        list_insert_last(&mutex->wait_list,&curr->wait_node);//wait_node是task_t的一个成员变量，用于记录等待的任务,是在哪赋值的呢
        task_dispatch();
    }
    irq_leave_protection(state);
}

void mutex_unlock(mutex_t *mutex){
    irq_state_t state = irq_enter_protection();

    task_t * curr = task_current();
    if(mutex->owner == curr){
        mutex->locked_count--;
        if(mutex->locked_count == 0){//解锁完毕
           mutex->owner = (task_t *)0;//解锁后，owner置空

  // 如果队列中有任务等待，则立即唤醒并占用锁
           if(list_count(&mutex->wait_list)){//有任务在等待
            list_node_t * node = list_remove_first(&mutex->wait_list);//返回的被删除的节点wait_node
            task_t * task = list_node_parent(node,task_t,wait_node); //***************s视频的node是task_node 我觉得部队 */
            task_set_ready(task);//再将这个节点设为ready，让其重新运行
            // //**********************我认为还得考虑readylist 里面有没有其他任务，如果有，就需要调度器来调度了，********************************** */
            // //**********************  自己的理解  
            // task_dispatch();//会更新curr_task，然后调度器会调度这个任务
            // task_t * curr2 = task_next_run();
            // mutex->owner = curr2;
            // mutex->locked_count = 1;
            
           //原 视频里面的代码
           // 在这里占用，而不是在任务醒后占用，因为可能抢不到
           mutex->locked_count = 1;
           mutex->owner = task;
           
           task_dispatch();
           
           }
        }
    }
    irq_leave_protection(state);
}