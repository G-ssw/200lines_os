#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
void kernel_init(boot_info_t* boot_info) {
    cpu_init();
}

void init_main(){
    irq_global_enable(); // 使能时钟中断
    //int a = 3 / 0; // 故意制造一个除零错误，测试异常处理
    for (;;) {}
}