#include "init.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "os_cfg.h"
#include "tools/klib.h"
#include "core/task.h"
#include "tools/list.h"
#include "ipc/sem.h"
#include "ipc/mutex.h"
#include "core/memory.h"

static boot_info_t * init_boot_info;        // 启动信息
static sem_t sem;


void kernel_init(boot_info_t * boot_info) {
  // ASSERT(boot_info->ram_region_count != 0);
  // ASSERT(3<2);
    cpu_init();

    memory_init(boot_info);//初始化内存管理

    log_init();
    irq_init();
    time_init();
    task_manager_init();
}

//static task_t first_task; //转移到task.h中的manager_t结构体中
static uint32_t init_task_stack[1024];
static task_t init_task;

 void init_task_entry(void){     //这个的选择子放在init_task的tss_sel中
    int count = 0;
    for(;;){
      //sem_wait(&sem);//等待信号量
        log_printf("init task:%d",count++);  ////任务2
        //sys_sleep(1000);
        //log_printf("init_task slice_ticks: %d", init_task.slice_ticks);
       

        //task_switch(&init_task,task_first_task);
        //sys_sched_yield();//释放cpu资源，让其他任务运行
  }
 }


void init_main(void) {
  //list_test();

  log_printf("kernel is running...");
  log_printf("version %s %s",OS_VERSION,"shabi");
  log_printf("%d %d %x %c",-123456,123456,0x12345,'c');//%x表示以16进制输出

   
  task_init(&init_task,(uint32_t)init_task_entry,(uint32_t)&init_task_stack[1024],"init task");//栈顶指针（高地址到低地址寻找的）
  task_first_init();
  // task_init(&first_task,0,0);//空任务
  // write_tr(first_task.tss_sel);//这个的选择子放在first_task的tss_sel中

  sem_init(&sem,0);
  //如果只执行了first main，可能由于中断的原因，导致任务切换失败， 因此需要开启全局中断
  irq_enable_global();//开启全局中断
 
  int count = 0;
  // int a = 3 / 0;
  //irq_enable_global();//开启全局中断
 // printf("%s %d","a",10);
  for(;;){
         log_printf("first main:%d",count++);//任务1
        //sem_notify(&sem);
        //sys_sleep(1000);
        //log_printf("init_task slice_ticks: %d", curr_task->slice_ticks);
       

        //task_switch(task_first_task,&init_task);
        //sys_sched_yield();
  }

  //init_task_entry();//任务2
}





// void list_test(void){
//   list_t list;
//   list_node_t nodes[5];


//   list_init(&list);//初始化列表
//   log_printf("list:first=0x%x, last=0x%x, count=%d",
//       list_first(&list), list_last(&list), list_count(&list));

//   for(int i=0;i<5;i++){
//     list_node_t * node = nodes + i;

//     log_printf("insert first to list: %d, 0x%x",i,(uint32_t)node);//插入到第一个位置,打印id和地址
//     list_insert_first(&list, node);
//   }
//   log_printf("list:first=0x%x, last=0x%x, count=%d",
//       list_first(&list), list_last(&list), list_count(&list));


//  list_init(&list);//清空列表
//   for(int i=0;i<5;i++){
//     list_node_t * node = nodes + i;

//     log_printf("insert first to list: %d, 0x%x",i,(uint32_t)node);//插入到第一个位置,打印id和地址
//     list_insert_last(&list, node);
//   }
//   log_printf("list:first=0x%x, last=0x%x, count=%d",
//       list_first(&list), list_last(&list), list_count(&list));

//    //remove first node from list
//   for(int i=0;i<5;i++){
//     list_node_t * node = list_remove_first(&list);
//     log_printf("remove first from list: %d, 0x%x",i,(uint32_t)node);//移除第一个节点,打印id和地址
//   } 
//    log_printf("list:first=0x%x, last=0x%x, count=%d",
//       list_first(&list), list_last(&list), list_count(&list));
//   //remove node
//   for(int i=0;i<5;i++){
//     list_node_t * node = nodes + i;
//     log_printf("insert first to list: %d, 0x%x",i,(uint32_t)node);//插入到第一个位置,打印id和地址
//     list_insert_last(&list, node);    
//   }
//   log_printf("list:first=0x%x, last=0x%x, count=%d",
//       list_first(&list), list_last(&list), list_count(&list));

//  for(int i=0;i<5;i++){
//     list_node_t * node = nodes + i;
//     log_printf("remove first from list: %d, 0x%x",i,(uint32_t)node);//移除第一个节点,打印id和地址
//     list_remove(&list, node);
//   } 
//    log_printf("list:first=0x%x, last=0x%x, count=%d",
//       list_first(&list), list_last(&list), list_count(&list));

//   struct type_t{
//     int i;
//     list_node_t node;
//   }v = {0x123456};

//   list_node_t * v_node = &v.node;//结构体指针
//   struct type_t * v_parent = list_node_parent(v_node,struct type_t,node);//结构体指针的父节点
//   if(v_parent->i == 0x123456){
//     log_printf("parent is ok");
//   }else{
//     log_printf("parent is not ok");
//   }
// }