/**
 * 功能：16位与32位的启动混合代码
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
	#include "os.h"

	// 声明本地以下符号是全局的，在其它源文件中可以访问
	.global _start, irq0_handler

	// 指定以下的代码生成16位的机器指令，这样才能在启动时的实模式下运行
  	.code16

	// 以下是代码区
 	.text
_start: 
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs
	mov $_start, %esp
	

read_self_all:
	mov $start_32, %bx
	mov $0x02, %cx
	mov $0x240, %ax
	mov $0x80, %dx
	int $0x13 
	jc read_self_all

	// 下面的代码将处理器从实模式切换到保护模式
	cli                     // 关中断
	lgdt gdt_desc      // 加载全局描述符表
	lidt idt_desc      // 加载中断描述符表
	mov $1, %eax          // 设置CR0寄存器的PE位
	lmsw %ax
	jmp $KERNEL_CODE_SEG, $start_32  // 跳转到32位代码段

	.org 0x1fe
	.byte 0x55, 0xaa  // 引导扇区的结束标志    

	// 32位保护模式, 位于512字节后
	.code32
	.text
start_32:
	// 下面的代码将段寄存器重新加载为保护模式下的段选择子
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs
	mov $_start, %esp

	call os_init  // 调用os_init函数初始化操作系统
	
	// 下面的代码启用分页机制
	mov $pg_dir, %eax
	mov %eax, %cr3         // 加载页目录基地址到CR3寄存器

	mov %cr4, %eax
	orl $(1 << 4), %eax  // 设置CR4寄存器的PSE位
	mov %eax, %cr4        // 启用分页机制

	mov %cr0, %eax
	orl $(1 << 31), %eax  // 设置CR0寄存器的PG位
	mov %eax, %cr0        // 启用分页机制

	sti					 // 开中断
	jmp .

irq0_handler:
	push %ds
	pusha
	mov $0x20, %al
	outb %al, $0x20       // 通知PIC芯片IRQ0中断处理完毕
	popa
	pop %ds
	iret

gdt_desc:
	.word (256 * 8) - 1	  // GDT表界限
	.long gdt_table                 // GDT表基地址

idt_desc:
	.word (256 * 8) - 1	  // IDT表界限
	.long idt_table                 // IDT表基地址