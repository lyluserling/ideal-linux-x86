#include "dev/time.h"
#include"comm/types.h"
#include "cpu/irq.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

static uint32_t sys_tick;

void do_handler_time (exception_frame_t *frame){
    sys_tick++;
    pic_send_eoi(IRQ0_TIMER); //send end of interrupt signal to PIC
}

void init_pit(void){
    uint32_t reload_count = PIT_OSC_FREQ*OS_TICKS_MS/1000.0;//calculate reload count for PIT
    outb(PIT_COMMAND_MODE_PORT,PIT_CHANNEL |PIT_LOAD_LOHI |PIT_MODE3 );
    outb(PIT_CHANNEL0_DATA_PORT,reload_count & 0xff);
    outb(PIT_CHANNEL0_DATA_PORT,(reload_count>>8)&0xff);

    irq_install(IRQ0_TIMER,(irq_handler_t)exception_handler_time );
    irq_enable(IRQ0_TIMER);
}

void time_init(void){
    sys_tick = 0;//initialize system tick to 0
    init_pit();//initialize programmable interval timer
}