/**
 * 功能：32位代码，完成多任务的运行
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
#include "os.h"

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define PDE_P       (1 << 0)    // Present
#define PDE_W       (1 << 1)    // Read/Write
#define PDE_U       (1 << 2)    // User/Supervisor
#define PDE_PS      (1 << 7)    // Page Size
#define MAP_PHY_ADDR 0x80000000  // 映射的物理地址，1MB

uint8_t map_phy_buffer[4096] __attribute__((aligned(4096))) = {123};

static uint32_t pg_table[1024] __attribute__((aligned(4096))) = {11};

uint32_t pg_dir[1024] __attribute__((aligned(4096)))= {
    [0] = (0x00000000 | PDE_P | PDE_W | PDE_U | PDE_PS),          // 前4MB，4MB大页
};

struct{uint16_t offset_l, selector, attr, offset_h;}idt_table[256] __attribute__((aligned(8))) = {1};

struct {uint16_t limit_l, base_l, basehl_attr, base_limit;}gdt_table[256] __attribute__((aligned(8))) = {
    [KERNEL_CODE_SEG / 8] = {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    [KERNEL_DATA_SEG / 8] = {0xFFFF, 0x0000, 0x9200, 0x00CF},
};

void outb(uint8_t val, uint16_t port){
    __asm__ __volatile__ (
        "outb %0, %1"
        :
        : "a"(val), "Nd"(port)
    );
}

void irq0_handler(void);

void os_init(void){
    outb(0x11, 0x20); // 主片
    outb(0x11, 0xA0); // 从片
    outb(0x20, 0x21); // 主片中断0-7映射到32-39
    outb(0x28, 0xA1); // 从片中断8-15映射到40-47
    outb(0x04, 0x21); // 主片IR2接从片
    outb(0x02, 0xA1); // 从片级联标志
    outb(0x01, 0x21); // 8086模式
    outb(0x01, 0xA1); // 8086模式
    outb(0xff, 0x21); // 屏蔽所有中断
    outb(0xff, 0xA1); // 屏蔽所有中断

    // 设置时钟中断频率为100ms
    int clock_100ms = 1193180 / 100; 
    outb(0x36, 0x43); // 选择通道0，方式3，二进制计数
    outb((uint8_t)clock_100ms, 0x40);
    outb((clock_100ms >> 8) & 0xFF, 0x40);

    idt_table[0x20].offset_l = (uint32_t)irq0_handler & 0xFFFF;
    idt_table[0x20].selector = KERNEL_CODE_SEG;
    idt_table[0x20].attr = 0x8E00; 
    idt_table[0x20].offset_h = (uint32_t)irq0_handler >> 16;

    pg_dir[MAP_PHY_ADDR >> 22] = ((uint32_t)pg_table | PDE_P | PDE_W | PDE_U); 
    pg_table[MAP_PHY_ADDR >> 12 & 0x3FF] = (uint32_t)map_phy_buffer | PDE_P | PDE_W | PDE_U;

    outb(0xFE, 0x21); // 取消中断屏蔽，打开时钟中断IRQ0
}