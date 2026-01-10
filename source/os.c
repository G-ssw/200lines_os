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

// 任务栈定义（每个任务需要两个栈：DPL0和DPL3）
uint32_t task0_dpl0_stack[1024], task0_dpl3_stack[1024];
uint32_t task1_dpl0_stack[1024], task1_dpl3_stack[1024];



struct{uint16_t offset_l, selector, attr, offset_h;}idt_table[256] __attribute__((aligned(8))) = {1};

struct {uint16_t limit_l, base_l, basehl_attr, base_limit;}gdt_table[256] __attribute__((aligned(8))) = {
    [KERNEL_CODE_SEG / 8] = {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    [KERNEL_DATA_SEG / 8] = {0xFFFF, 0x0000, 0x9200, 0x00CF},
    
    [APP_CODE_SEG / 8]    = {0xFFFF, 0x0000, 0xFA00, 0x00CF},
    [APP_DATA_SEG / 8]    = {0xFFFF, 0x0000, 0xF300, 0x00CF},

    [TASK0_TSS_SEL / 8]   = {0x0068, 0, 0xE900, 0x0},  
    [TASK1_TSS_SEL / 8]   = {0x0068, 0, 0xE900, 0x0},
};

void outb(uint8_t val, uint16_t port){
    __asm__ __volatile__ (
        "outb %0, %1"
        :
        : "a"(val), "Nd"(port)
    );
}

void task_0 (void) {
    uint8_t color = 0;
    for (;;) {
        color++;
    }
}

// 任务1函数（任务0已经有了）
void task_1(void) {
    uint8_t color = 0xff;

    for (;;) {
        color--;
    }
}

// 任务调度函数
void task_sched(void) {
    static int current_task = TASK0_TSS_SEL;
    
    // 切换任务
    if (current_task == TASK0_TSS_SEL) {
        current_task = TASK1_TSS_SEL;
    } else {
        current_task = TASK0_TSS_SEL;
    }
    
    // 使用远跳转切换到新任务
    // 这个数组[0, 选择子]构成一个远指针
    uint32_t jump_addr[] = {0, current_task};
    
    // ljmpl指令：跳转到TSS选择子，触发任务切换
    __asm__ __volatile__("ljmpl *(%0)" : : "r"(jump_addr));
}

// 任务0的TSS定义
uint32_t task0_tss[] = {
    // prelink, esp0, ss0, esp1, ss1, esp2, ss2
    0,  (uint32_t)task0_dpl0_stack + 4*1024, KERNEL_DATA_SEG, 
    0x0, 0x0, 0x0, 0x0,  // esp1-ss2不用
    
    // cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi
    (uint32_t)pg_dir,  (uint32_t)task_0, 0x202,  // 页目录、入口点、标志
    0xa, 0xc, 0xd, 0xb,                         // eax, ecx, edx, ebx
    (uint32_t)task0_dpl3_stack + 4*1024,        // 用户栈（esp）
    0x1, 0x2, 0x3,                              // ebp, esi, edi
    
    // es, cs, ss, ds, fs, gs, ldt, iomap
    APP_DATA_SEG, APP_CODE_SEG, APP_DATA_SEG, 
    APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, 
    0x0, 0x0,                                   // ldt, iomap
};

// 任务1的TSS定义
uint32_t task1_tss[] = {
    // prelink, esp0, ss0, esp1, ss1, esp2, ss2
    0,  (uint32_t)task1_dpl0_stack + 4*1024, KERNEL_DATA_SEG,
    0x0, 0x0, 0x0, 0x0,
    
    // cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi
    (uint32_t)pg_dir,  (uint32_t)task_1, 0x202,
    0xa, 0xc, 0xd, 0xb,
    (uint32_t)task1_dpl3_stack + 4*1024,        // 用户栈
    0x1, 0x2, 0x3,
    
    // es, cs, ss, ds, fs, gs, ldt, iomap
    APP_DATA_SEG, APP_CODE_SEG, APP_DATA_SEG,
    APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG,
    0x0, 0x0,
};


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

    // 初始化TSS描述符的基址
    gdt_table[TASK0_TSS_SEL / 8].base_l = (uint16_t)((uint32_t)task0_tss & 0xFFFF);
    gdt_table[TASK0_TSS_SEL / 8].basehl_attr |= ((uint32_t)os_init >> 16) & 0xFF;
    gdt_table[TASK0_TSS_SEL / 8].base_limit |= ((uint32_t)task0_tss >> 24) << 24;
    
    gdt_table[TASK1_TSS_SEL / 8].base_l = (uint16_t)((uint32_t)task1_tss & 0xFFFF);
    gdt_table[TASK1_TSS_SEL / 8].basehl_attr |= ((uint32_t)task1_tss >> 16) & 0xFF;
    gdt_table[TASK1_TSS_SEL / 8].base_limit |= ((uint32_t)task1_tss >> 24) << 24;
}