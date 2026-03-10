#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "tools/log.h"
#include "os_cfg.h"

void kernel_init(boot_info_t* boot_info) {
    cpu_init();
    log_init();
}

void init_main(){
    irq_global_enable(); // 使能时钟中断
    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    //int a = 3 / 0; // 故意制造一个除零错误，测试异常处理
    for (;;) {}
}