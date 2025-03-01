#include "os_cfg.h"  
    .text
    .extern kernel_init
    .global _start
    .extern init_main
_start:
    push %ebp
    mov %esp, %ebp
    mov 0x8(%ebp), %eax
    # 偏移量是0x8（ebp向上两个格子），即boot_info参数的位置
    # kernel_init(boot_info)
    push %eax
    call kernel_init

    jmp $KERNEL_SELECTOR_CS, $gdt_reload
gdt_reload:    # 重新加载GDT
	mov $KERNEL_SELECTOR_DS, %ax		// 16为数据段选择子
	mov %ax, %ds 
	mov %ax, %ss 
	mov %ax, %es 
	mov %ax, %fs 
	mov %ax, %gs 
/* 栈设置
	mov $(stack + KERNEL_STACK_SIZE), %esp
*/
# 将 stack 的地址加载到 %eax 寄存器中
movl $stack, %eax

# 将偏移 KERNEL_STACK_SIZE 加到 %eax，然后加载到 %esp
addl $KERNEL_STACK_SIZE, %eax
movl %eax, %esp
	# 栈和段等沿用之前的设置
	jmp init_main

	.bss
.comm stack, KERNEL_STACK_SIZE    # comm 声明未初始化的通用内存区域，以字节计

.text
.macro exception_handler name num with_error_code
    
    .extern do_handler_\name
    .global exception_handler_\name
exception_handler_\name:
    .if \with_error_code == 0
        push $0 //如果没有错误码，压入0
    .endif

    push $\num //保存异常号(压栈)num是一个数值，要加一个$符号
    pusha
    push %ds
    push %es
    push %fs
    push %gs

    push %esp//保存异常发生时的栈指针(压栈)
    call do_handler_\name //调用
    add $(1*4), %esp		// 丢掉esp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    add $(2*4), %esp  // 丢掉异常号
    iret
.endm
 
exception_handler unknown, -1,0 //未知异常处理程序，异常号为-1，不带错误码
exception_handler divider, 0,0 //未知异常处理程序，异常号为0，不带错误码
exception_handler Debug, 1, 0
exception_handler NMI, 2, 0
exception_handler breakpoint, 3, 0
exception_handler overflow, 4, 0
exception_handler bound_range, 5, 0
exception_handler invalid_opcode, 6, 0
exception_handler device_unavailable, 7, 0
exception_handler double_fault, 8, 1
exception_handler invalid_tss, 10, 1
exception_handler segment_not_present, 11, 1
exception_handler stack_segment_fault, 12, 1
exception_handler general_protection, 13, 1
exception_handler page_fault, 14, 1
exception_handler fpu_error, 16, 0
exception_handler alignment_check, 17, 1
exception_handler machine_check, 18, 0
exception_handler smd_exception, 19, 0
exception_handler virtual_exception, 20, 0
exception_handler time, 0x20,  0     //关联到了芯片IRQ0的位置，也就是start


    // simple_switch(&from,to)
    .text
    .global simple_switch
simple_switch:
    mov 4(%esp),%eax
    mov 8(%esp),%edx
    push %ebp
    push %ebx
    push %esi
    push %edi
    mov %esp,(%eax)

    mov %edx,%esp
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp

    ret