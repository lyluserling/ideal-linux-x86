
## **ideal- x86 OS 系统架构文档**

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/95f4f0628e444bf9ac38758572f8cd29.png)

# 一、项目概述

本项目是一个自研的基于x86架构的类Linux操作系统内核，完全使用C语言与x86汇编语言开发，支持32位保护模式，具备基础的内核功能：任务调度、内存分页管理、中断处理、系统调用、文件系统、设备驱动、终端交互以及用户程序加载执行。

目标：构建一个从裸机启动到运行用户shell的最小但功能完整的操作系统环境。

# 二、系统模块架构
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/46065463364240a88583ae0af80e6c16.png)




# 三、各核心模块说明

## 1. 启动与内核初始化

文件：`start.S, init.c, kernel_init()`

流程：

汇编设置段寄存器、栈、跳转到 C 函数 kernel_init()；

初始化 GDT/IDT、开启中断；

初始化内存管理器（分页、物理页管理）；

初始化文件系统，挂载 devfs 和 FAT16 根目录；

初始化任务管理器，创建第一个用户进程（init）

## 2. 中断与异常机制

文件：`irq.c, start.S`

支持32个CPU异常、16个IRQ硬件中断、系统调用陷入（int 0x80/lcall）

每个中断入口保存上下文，调用对应的处理函数，最后 iret 恢复

支持 page fault、divide error 等调试信息输出

## 3. 内存管理

文件：`memory.c, bitmap.c`

使用页目录+页表实现分页机制，支持最大4GB空间

实现用户/内核空间隔离，内核地址高1GB映射共享

提供 fork 时页表复制，支持写时复制（COW 可拓展）

内核堆管理通过 sbrk() 系统调用，页面粒度分配

## 4. 进程管理与调度

文件：`task.c`

支持 fork, execve, exit, wait, yield

时间片轮转调度，基于PIT定时器中断触发

每个任务有独立内核栈 + 页表 + PCB，使用TSS进行上下文切换

init 进程自动拉起多个 shell 实例，绑定到 TTY0~7

## 5. 文件系统

文件：`fs.c, file.c, fatfs.c, devfs.c`

多文件系统支持（通过 fs_op_t 抽象）：

FAT16：支持 open/read/write/lseek/close，遍历根目录、文件创建与删除；

devfs：抽象设备文件如 /dev/tty0、/dev/sda1；

支持 dup, isatty, fstat 等常规调用；

文件描述符管理使用引用计数与 file_alloc() 实现；

## 6. 设备驱动框架

文件：`dev.c, tty.c, kbd.c, disk.c, console.c`

抽象设备描述符 dev_desc_t，统一注册到 devfs 文件系统中

支持字符设备（TTY、键盘）与块设备（IDE磁盘，FAT16分区）

使用 dev_read, dev_write, dev_ioctl 封装调用路径

## 7. 系统调用机制

文件：`syscall.c, start.S, lib_syscall.c`

用户通过 lcall 指令陷入系统调用门（调用门描述符在GDT中）

do_handler_syscall() 提取参数，查表调用内核服务函数

参数通过压栈传递，最多支持5个参数

## 8. 用户态接口

文件：`lib_syscall.c`

实现用户库函数：fork, execve, open, read, write, msleep, sbrk 等

提供 shell 使用的 print_msg()、系统级API封装

## 9. 工具与通用模块

文件：`klib.c, list.c, log.c, bitmap.c`

实现 memcpy, vsprintf, log_printf, 双向链表, 位图分配器 等

日志支持串口/TTY输出，可用于调试与 panic 断点

## 四、系统初始化流程

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/96db807e6aaf437ca5fa3d3034e5ab58.png)


## 五、系统调用路径示意

以 `write(fd, buf, size)` 为例：
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/d8d548f2a8164e8d9b7da447eabd2d93.png)

## 六、结语

本项目从零实现了一个具备基本Unix特性的微内核操作系统，完成了从裸机引导到多进程shell运行的全流程。架构清晰，模块分离，适合教学、自研系统开发与操作系统原理深入学习。

下一步可扩展方向：

写时复制支持；

更复杂的调度算法（如优先级、多队列）

用户态 ELF 加载器优化

多核支持（SMP）

网络协议栈（如 loopback + ICMP）
