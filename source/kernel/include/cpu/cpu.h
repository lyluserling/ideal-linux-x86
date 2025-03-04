#ifndef __CPU_H__
#define __CPU_H__


#include "comm/types.h"

#define EFLAGS_DEFAULT     (1 << 1) //固定为1的位
#define EFLAGS_IF          (1 << 9) //中断标志位,不能把中断全都关了




#pragma pack(1)// byte alignment没有任何填充字节。
typedef struct _segment_desc_t{// 16 bytes
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;// 23:16
    uint16_t attr;
    uint8_t base31_24;
}segment_desc_t;

/**
 * tss描述符,
 */
typedef struct _tss_t { //help 任务切换 
    uint32_t pre_link;
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;//段寄存器
    uint32_t ldt;
    uint32_t iomap;
}tss_t;

#pragma pack()// restore default alignment


/*
 * 调用门描述符
 */
typedef struct _gate_desc_t {
	uint16_t offset15_0;
	uint16_t selector;
	uint16_t attr;
	uint16_t offset31_16;
}gate_desc_t;


#define SEG_G				(1 << 15)		// 设置段界限的单位，1-4KB，0-字节
#define SEG_D				(1 << 14)		// 控制是否是32位、16位的代码或数据段
#define SEG_P_PRESENT	    (1 << 7)		// 段是否存在

#define SEG_DPL0			(0 << 5)		// 特权级0，最高特权级
#define SEG_DPL3			(3 << 5)		// 特权级3，最低权限

#define SEG_S_SYSTEM		(0 << 4)		// 是否是系统段，如调用门或者中断
#define SEG_S_NORMAL		(1 << 4)		// 普通的代码段或数据段

#define SEG_TYPE_CODE		(1 << 3)		// 指定其为代码段
#define SEG_TYPE_DATA		(0 << 3)		// 数据段

#define SEG_TYPE_RW			(1 << 1)		// 是否可写可读，不设置为只读

#define SEG_TYPE_TSS      	(9 << 0)		// 32位TSS

#define GATE_TYPE_IDT		(0xE << 8)		// 中断32位门描述符
#define GATE_TYPE_SYSCALL	(0xC << 8)		// 调用门
#define GATE_P_PRESENT		(1 << 15)		// 是否存在
#define GATE_DPL0			(0 << 13)		// 特权级0，最高特权级
#define GATE_DPL3			(3 << 13)		// 特权级3，最低权限

#define SEG_RPL0                (0 << 0)
#define SEG_RPL3                (3 << 0)

#define EFLAGS_IF           (1 << 9)
#define EFLAGS_DEFAULT      (1 << 1)


void init_gdt(void);
void cpu_init(void);
void segment_desc_init(int selector, uint32_t base, uint32_t limit, uint16_t attr);
void gate_desc_set(gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr) ;
int gdt_alloc_desc();//分配一个新的描述符
void switch_to_tss(int tss_sel);

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);



#endif