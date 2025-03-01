#include <stdarg.h>
#include "comm/cpu_instr.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "cpu/irq.h"
#include "ipc/mutex.h"
#define COM1_PORT 0x3f8

static mutex_t log_mutex;

// void kernel_memset(void *dest, uint8_t v, int size){
//     if(!dest || size == 0) return ;//保护程序免于出现由于无效指针访问导致的错误或崩溃

//     uint8_t *d = (uint8_t *)dest;//d指向dest的开始位置
//     while(size--){//当size大于0时，设置字符为v
//          *d++ = v;//将v复制到d中，直到size为0
//          }
// }

void log_init(void){
    mutex_init(&log_mutex);
    outb(COM1_PORT + 1, 0x00); // Disable all interrupts,清除COM1_PORT中断
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0F);
}
void log_printf(const char * fmt, ...){
    char str_buf[128];
    va_list arg_ptr;
    kernel_memset(str_buf, '\0', sizeof(str_buf));//清空缓冲区
    va_start(arg_ptr, fmt);//获取参数列表
    kernel_vsprintf(str_buf, fmt, arg_ptr);//格式化输出到缓冲区
    va_end(arg_ptr);//结束获取参数列 表

//    irq_state_t state =  irq_enter_protection();//进入中断保护

    mutex_lock(&log_mutex);//加锁
    const char *p = str_buf;
    while(*p != '\0'){
        while((inb(COM1_PORT + 5) & 0x20) == 0); // Wait for the transmit buffer to be empty
        outb(COM1_PORT, *p);
        p++;
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
    // irq_leave_protection(state);//退出中断保护
    mutex_unlock(&log_mutex);//解锁
}

