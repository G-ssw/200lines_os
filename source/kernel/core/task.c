#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

static int tss_init(task_t *task, uint32_t entry_point, uint32_t stack_top) {
    uint16_t tss_selector = gdt_alloc_desc();
    if (tss_selector < 0) {
        log_printf("Failed to allocate GDT descriptor for TSS");
        return -1; // 分配失败
    }
    // 设置 TSS 描述符，类型为 32 位可用 TSS，特权级 0
    segment_desc_set(tss_selector, (uint32_t)&task->tss, sizeof(tss_t) - 1,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_SYSTEM | SEG_TYPE_TSS, 0);

    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.esp = stack_top; // 用户栈顶地址
    task->tss.esp0 = stack_top; // 内核栈顶地址
    task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.eip = entry_point;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF; // IF=1，允许中断
    task->tss.es = KERNEL_SELECTOR_DS;
    task->tss.cs = KERNEL_SELECTOR_CS;
    task->tss.ss = KERNEL_SELECTOR_DS;
    task->tss.ds = KERNEL_SELECTOR_DS;
    task->tss.fs = KERNEL_SELECTOR_DS;
    task->tss.gs = KERNEL_SELECTOR_DS;

    task->tss_sel = tss_selector; // 保存 TSS 选择子
    return 0;
}

int task_create(task_t *task, uint32_t entry_point, uint32_t stack_top) {
    ASSERT(task);
    // 手动构造初始栈帧，模拟任务切换时 CPU 自动压入的寄存器
    uint32_t * pesp = (uint32_t *)stack_top;
    if (pesp) {
        *(--pesp) = entry_point;
        *(--pesp) = 0;
        *(--pesp) = 0;
        *(--pesp) = 0;
        *(--pesp) = 0;
        task->stack = pesp;
    }
    //tss_init(task, entry_point, stack_top)
    return 1;
}

void task_switch_from_to(task_t *from, task_t *to) {
    ASSERT(from);
    ASSERT(to);
    //switch_to_tss(to->tss_sel); // 使用保存的 TSS 选择子并切换
    simple_task_switch(&from->stack, to->stack); //手动保存和恢复 TSS 寄存器，模拟任务切换
}
