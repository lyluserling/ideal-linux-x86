#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "os_cfg.h"
static boot_info_t * init_boot_info;        // 启动信息

void kernel_init(boot_info_t * boot_info) {
    cpu_init();//初始化GDT_table
    log_init();
    irq_init();
    time_init();
}

void init_main(void) {
  log_printf("kernel is running...");
  log_printf("version %s",OS_VERSION);
  //int a = 3 / 0;
  //irq_enable_global();//开启全局中断
 // printf("%s %d","a",10);
  for(;;){}
}