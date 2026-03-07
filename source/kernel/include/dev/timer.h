#ifndef TIMER_C
#define TIMER_C

#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

#define PIS_BASE_FREQ 1193180 // PIT的输入时钟频率

#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT 0x43

#define PIT_CHANNEL0  (0 << 6) // 选择通道0
#define PIT_LOAD_MODE (3 << 1) // 访问方式：先访问低字节再访问高字节
#define PIT_MODE_ZERO (3 << 4) // 工作模式：频率发生器

void timer_init(void);
void timer_eoi(int irq_num);
void exception_handler_timer (void);

#endif