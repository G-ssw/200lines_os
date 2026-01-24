/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 * 
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
	#include "boot.h"

  	// 16位代码，务必加上
  	.code16
 	.text
	.extern boot_entry
	.global _start
_start:
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %fs
	mov %ax, %gs
	// 设置堆栈指针
	mov $_start , %sp

	// 显示欢迎信息
	mov $0xE, %ah
	mov $'H', %al
	int $0x10

read_loader:
	// 读取二级引导程序到内存地址0x8000处
	mov $0x8000, %bx       // 读入地址
	mov $0x2, %ah          // 读扇区功能号
	mov $0x64, %al          // 读1个扇区
	mov $0x0, %ch          // 柱面号0
	mov $0x2, %cl          // 起始扇区号1
	mov $0x0080, %dx       // 磁头号0 + 驱动器号0x80
	int $0x13              // 调用BIOS中断
	jc read_loader		   // 读失败则重试

	jmp boot_entry

	jmp .


	
	// 引导结束段
	.section boot_end, "ax"
boot_sig: .byte 0x55, 0xaa
