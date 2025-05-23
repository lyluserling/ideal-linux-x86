/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 * 
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
//	#include "boot.h"

  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start
	.extern loader_entry
_start:

	jmp loader_entry
	
	.code32
	.text
	.global protected_mode_entry//全局，相当于extern
// 保护模式入口点(函数名称)
protected_mode_entry:// 保护模式入口点(函数名称)
	mov $16, %ax // 切换到32位模式
	mov %ax, %ds // 设置数据段寄存器
	mov %ax, %ss // 设置堆栈段寄存器
	mov %ax, %es // 设置附加段寄存器
	mov %ax, %fs // 设置附加段寄存器
	mov %ax, %gs // 设置附加段寄存器

	jmp $8, $load_kernel // 跳转至内核加载地址,跳转到运行load_kernel这个函数
