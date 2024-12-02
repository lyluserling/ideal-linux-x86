#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "types.h"

//读端口
static inline uint8_t inb(uint16_t port) { 
    uint8_t data;
    //inb al dx inb 是一个汇编指令，用于从指定的 I/O 端口读取一个字节数据。语法为 inb <端口>, <数据>，这里数据将从端口 port 读取到 data。
    __asm__ __volatile__("inb %[p], %[v]" : [v]"=a"(data) : [p]"d"(port)); //用汇编指令inb从端口port读数据，并将数据保存到data变量中
    return data;//返回数据
}



static inline uint16_t inw(uint16_t port) { 
    uint16_t data;
   //in ax dx inw 是一个汇编指令，用于从指定的 I/O 端口读取一个字数据。语法为 inw <端口>, <数据>，这里数据将从端口 port 读取到 data。
    __asm__ __volatile__("in %[p], %[v]" : [v]"=a"(data) : [p]"d"(port)); //用汇编指令inb从端口port读数据，并将数据保存到data变量中
    return data;//返回数据
}


static inline void outb (uint16_t port, uint8_t data){//写端口
    //out al dx
    //    __asm__ __volatile__("outb %[v], %[p]::[p]"d"(port), [v]"a"(data)); //用汇编指令outb将数据data写入端口port中)
    __asm__ __volatile__ (
    "outb %[v], %[p]" //outb 是一个汇编指令，用于将字节数据写入指定的 I/O 端口。语法为 outb <数据>, <端口>，这里 data 将写入到 port。
    : // 没有输出
    : [p] "d" (port), [v] "a" (data) 
);
// 输入[p] 和 [v] 是命名约束，分别表示端口和数据。,"d" 表示将 port 的值放入 EDX 寄存器。"a" 表示将 data 的值放入 EAX 寄存器。
}

static inline void cli(void) {  //cli关中断
    __asm__ __volatile__("cli");
}

static inline void sti(void) {  //sti开中断
    __asm__ __volatile__("sti");
}

static inline void lgdt(uint32_t start, uint32_t size){

    struct {
        uint16_t limit;//段界限
        uint16_t start15_0;//段基址
        uint16_t start32_16;//段基址
    }gdt;
    //initial gdt
    gdt.start15_0 = start & 0xffff;
    gdt.start32_16 = (start >> 16) & 0xffff;
    gdt.limit = size - 1;//段界限
    
   

    
    __asm__ __volatile__("lgdt %[g]" ::[g]"m"(gdt)); //用汇编指令lgdt加载全局描述符表gdt
}

static inline void lidt(uint32_t start, uint32_t size){

    struct {
        uint16_t limit;//段界限
        uint16_t start15_0;//段基址
        uint16_t start32_16;//段基址
    }idt;
    //initial gdt
    idt.start15_0 = start & 0xffff;
    idt.start32_16 = (start >> 16) & 0xffff;
    idt.limit = size - 1;//段界限
    
   

    
    __asm__ __volatile__("lidt %[g]" ::[g]"m"(idt)); //用汇编指令lgdt加载全局描述符表gdt
}

static inline uint16_t read_cr0(void) {
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %[v]" : [v] "=r" (cr0));//将 CR0 寄存器的值移动到指定的操作数 [v] 中,%%cr0 使用了双百分号 (%%) 来转义，这样编译器不会将其视为 C 语言中的格式字符串。具体的寄存器不指定，用[v]表示。
    return cr0;
}
static inline void write_cr0(uint32_t cr0) {
    __asm__ __volatile__("mov %[v], %%cr0" :: [v] "r" (cr0));
}



static inline void far_jump(uint16_t sel, uint32_t offset){//远跳转 offse会被设置为start.S里面protected_mode_entry函数的地址
    uint32_t addr[] = {offset, sel};
    __asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));//用汇编指令ljmp跳转到指定段选择子和偏移地址。

}

static inline void hlt(void){
    __asm__ __volatile__("hlt");
}

#endif