#include "init.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "tools/log.h"
#include "os_cfg.h"
#include "core/task.h"

void kernel_init(boot_info_t* boot_info) {
    cpu_init();
    log_init();
}

task_t init_task;
task_t first_task;
uint32_t init_task_stack[1024]; // 为 init 任务分配一个栈

void init_task_entry() {
    int count = 0;
    for(;;){
        log_printf("Init task has been running for %d seconds...", count);
        task_switch_from_to(&init_task, &first_task);
        count++;
    }
}

void init_main(){
    //irq_global_enable(); // 使能时钟中断
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    task_create(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1023]); // 创建 init 任务
    task_create(&first_task, 0, 0); // 创建第一个用户任务
    write_tr(first_task.tss_sel); // 直接使用 ltr 指令加载 TSS 描述符
    //int a = 3;
    //ASSERT(a < 2); // 触发断言测试
    //int a = 3 / 0; // 故意制造一个除零错误，测试异常处理
    int count = 0;
    for (;;) { 
        log_printf("first task: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}