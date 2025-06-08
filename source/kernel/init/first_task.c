
#include "applib/lib_syscall.h"
#include "dev/tty.h"

int first_task_main (void) {
    for(;;){
        log_printf("Hello, world!\n");
        sys_sleep(1000);
    }
    return 0;
} 