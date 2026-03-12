#ifndef _TASK_H
#define _TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"

typedef struct _task_t{
    uint32_t * stack;
    tss_t tss; // 任务的TSS结构体
    uint16_t tss_sel;		// tss选择子
} task_t;

void task_switch_from_to(task_t *from, task_t *to);
int task_create(task_t *task, uint32_t entry_point, uint32_t stack_top);
void simple_task_switch (uint32_t ** from_stack, uint32_t * to_stack);

#endif