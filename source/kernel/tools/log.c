#include "tools/log.h"
#include "comm/cpu_instr.h"
#define COM1_PORT 0x3f8
void log_init(void){
    outb(COM1_PORT + 1, 0x00); // Disable all interrupts,清除COM1_PORT中断
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0F);
}
void log_printf(const char * fmt, ...){
    const char *p = fmt;
    while(*p != '\0'){
        while((inb(COM1_PORT + 5) & 0x20) == 0); // Wait for the transmit buffer to be empty
        outb(COM1_PORT, *p);
        p++;
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
}