#include "os_cfg.h"

.text
.extern kernel_init
.extern init_main
.global _start

_start:
    push %ebp
    mov %esp, %ebp
    mov 0x8(%ebp), %eax

    push %eax
    call kernel_init
    // 重新加载GDT
	jmp $KERNEL_SELECTOR_CS, $gdt_reload

gdt_reload:
	mov $KERNEL_SELECTOR_DS, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

	// 栈设置
	mov $(stack + KERNEL_STACK_SIZE), %esp

	// 栈和段等沿用之前的设置
	jmp init_main

	.bss
.comm stack, KERNEL_STACK_SIZE    // comm 声明未初始化的通用内存区域，以字节计

// 中断发生时，会自动切换到特权级0对应的栈中去执行
// 并且只保存ss,esp,cs,eip,flags寄存器
// 所以需要在中断中自行保存其它寄存器
	.text
.macro exception_handler name num error_code 
	.extern do_handler_\name
	.global exception_handler_\name
	exception_handler_\name:

	// 如果错误码不存在，为了保证统一的调用方式，仍然压入一个0作为错误码
	.if \error_code == 0
		push $\error_code
	.endif

	// 压入中断号
	push $\num

	// 保存所有寄存器
	pushal
	push %ds
	push %es
	push %fs
	push %gs

	push %esp // 将用户栈指针压入栈中，供中断处理函数使用
	call do_handler_\name // 调用中断处理函数
	pop %esp // 恢复用户栈指针

	// 恢复保存的寄存器
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa

	// 直接跳过栈中的错误码和中断号，返回到中断发生前的状态
	add $8, %esp

	iret
.endm

exception_handler unknown, -1, 0
exception_handler divider, 0, 0
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

// 硬件中断
exception_handler timer, 32, 0

.text
.global simple_task_switch

simple_task_switch:
	movl 4(%esp), %eax // 获取当前任务的 TSS 选择子
	movl 8(%esp), %edx // 获取目标任务的 TSS 选择子

	push %ebp
	push %ebx
	push %esi
	push %edi

	mov %esp, (%eax)
	mov %edx, %esp

	pop %edi
	pop %esi
	pop %ebx
	pop %ebp
	ret
