#include "dev/timer.h"

static uint32_t sys_tick;

/*
 * EOI（End of Interrupt）信号发送函数
*/
void timer_eoi(int irq_num) {
	irq_num -= IRQ_PIC_START; // 调整为从片的IRQ号
	if (irq_num >= 8) {
		outb(PIC1_OCW2, PIC_OCW2_EOI); // 发送EOI给从片
	}
	outb(PIC0_OCW2, PIC_OCW2_EOI); // 发送EOI给主片
}

void do_handler_timer (exception_frame_t *frame) {
    sys_tick++;
    //先发 EOI
    timer_eoi(IRQ0_TIMER);
}

static void init_pit (void){
    uint32_t reload_count = PIS_BASE_FREQ / (1000 / OS_TICK_MS); // 计算重装载值
    // 发送命令字，设置 PIT 工作模式
    outb(PIT_COMMAND_PORT, PIT_CHANNEL0 | PIT_LOAD_MODE | PIT_MODE_ZERO);
    // 发送重装载值的低字节和高字节
    outb(PIT_CHANNEL0_PORT, reload_count & 0xFF); // 低字节
    outb(PIT_CHANNEL0_PORT, (reload_count >> 8) & 0xFF); // 高字节

    irq_install(IRQ0_TIMER, (irq_handler_t)exception_handler_timer); // 安装时钟中断处理函数
    irq_enable(IRQ0_TIMER); // 使能时钟中断
}

/*
 * 初始化定时器
 * 1. 配置 PIT 定时器，设置合适的频率
 * 2. 安装时钟中断处理函数
 */
void timer_init(void) {
    sys_tick = 0; // 初始化系统时钟计数器
    init_pit();
}





