#ifndef _TASK_H
#define _TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"

#define TASK_TIME_SLICE_DEFAULT 10 // 默认时间片长度，单位为时钟节拍

typedef struct _task_t{
    enum {
        TASK_CREATED,
        TASK_READY,
        TASK_RUNNING,
        TASK_BLOCKED,
        TASK_ZOMBIE,
        TASK_SLEEP,
    }state;

    char name[32]; // 任务名称

    uint16_t time_slice; // 时间片长度，单位为时钟节拍
    uint16_t ticks; // 已经使用的时间片时钟节拍数
    uint16_t sleep_ticks; //睡眠时间长度

    // uint32_t * stack;
    ListNode RunNode; // 运行相关节点
    ListNode AllNode; // 所有任务列表节点

    tss_t tss; // 任务的TSS结构体
    uint16_t tss_sel;		// tss选择子
} task_t;

typedef struct _task_manager_t{
    List* ready_list; // 就绪队列，保存所有就绪的任务
    List* tasks_list; // 所有任务列表，保存系统中所有的任务
    List* sleep_list; //

    task_t* current_task;
    task_t* first_task;
    task_t* idle_task;
} task_manager_t;

void task_switch_from_to(task_t *from, task_t *to);
int task_create(task_t *task, const char* name, uint32_t entry_point, uint32_t stack_top);
void simple_task_switch (uint32_t ** from_stack, uint32_t * to_stack);

void task_manager_init(void);
void first_task_init(void);
void task_set_ready(task_t* task);
void task_set_blocked(task_t* task);
void task_yield(void);
void task_schedule(void);
void sys_sleep(uint16_t time_ms);
void task_timeslice_tick(void);
task_t* task_get_current(void);
task_t* task_get_rdyto_run(void);

#endif