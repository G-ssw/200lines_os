#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "tools/list.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

static task_manager_t task_manager; // 全局任务管理器实例
static task_t first_task_struct;  // 定义实际的任务控制块(因为成员为指针类型，需要特地指向一个实际的变量，否则为空触发断言)
static List ready_list; // 就绪队列，保存所有就绪的任务
static List tasks_list; // 所有任务列表，保存系统中所有的任务
static List sleep_list; // 所有任务列表，保存系统中所有的任务
static task_t idle_task;

static uint32_t idle_task_stack[IDLE_STACK_SIZE];

static void task_idle_entry(){
    while(1){
        hlt();
    }
}

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

int task_create(task_t *task, const char* name, uint32_t entry_point, uint32_t stack_top) {
    ASSERT(task != (task_t *)0);
    int ret = tss_init(task, entry_point, stack_top);
    if (ret < 0) {
        log_printf("Failed to initialize TSS for new task");
        return -1; // TSS 初始化失败
    }

    // 初始化任务状态和名称
    if (name) {
        kernel_strncpy(task->name, name, sizeof(task->name));
    } else {
        kernel_strncpy(task->name, "Unnamed Task", sizeof(task->name));
    }
    task->state = TASK_CREATED;
    task->time_slice = TASK_TIME_SLICE_DEFAULT; // 设置默认时间片长度
    task->ticks = 0; // 初始化已使用时间片计数

    // 将新任务添加到任务管理器的就绪队列和所有任务列表中
    ListNode_init(&task->RunNode, 0);
    ListNode_init(&task->AllNode, 0);

    task_set_ready(task); // 将任务状态设置为就绪并加入就绪队列
    List_insert_tail(task_manager.tasks_list, &task->AllNode);
    // 手动构造初始栈帧，模拟任务切换时 CPU 自动压入的寄存器
    // uint32_t * pesp = (uint32_t *)stack_top;
    // if (pesp) {
    //     *(--pesp) = entry_point;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     task->stack = pesp;
    // }
    
    return 0;
}

void task_switch_from_to(task_t *from, task_t *to) {
    ASSERT(from);
    ASSERT(to);
    switch_to_tss(to->tss_sel); // 使用保存的 TSS 选择子并切换
    //simple_task_switch(&from->stack, to->stack); //手动保存和恢复 TSS 寄存器，模拟任务切换
}

void task_manager_init(void){
    List_init(task_manager.ready_list);
    List_init(task_manager.tasks_list);
    task_manager.ready_list = &ready_list;
    task_manager.tasks_list = &tasks_list;
    task_manager.sleep_list = &sleep_list;
    task_manager.current_task = NULL;
    task_manager.first_task = NULL;
    task_manager.idle_task = &idle_task;

    task_create(task_manager.idle_task, "idle_task", (uint32_t)task_idle_entry, (uint32_t)&idle_task_stack[IDLE_STACK_SIZE-1]);
}

void first_task_init(void){
    task_manager.first_task = &first_task_struct;
    task_create(task_manager.first_task, "First Task", 0, 0); // 创建第一个用户任务
    write_tr(task_manager.first_task->tss_sel); // 直接使用 ltr 指令加载 TSS 描述符
    task_manager.current_task = task_manager.first_task; // 设置当前运行的任务为第一个任务
}

void task_set_ready(task_t* task){
    if(task->state != TASK_READY && task != task_manager.idle_task){
        task->state = TASK_READY;
        List_insert_tail(task_manager.ready_list, &task->RunNode);
    }
}

void task_set_blocked(task_t* task){
    if(task->state == TASK_RUNNING || task->state == TASK_READY){
        task->state = TASK_BLOCKED;
        if(task != task_manager.idle_task)List_remove_node(task_manager.ready_list, &task->RunNode);
    }
}

void task_set_sleep(task_t* task, uint16_t ticks){
    if(ticks <= 0){
        return ;
    }
    task->state = TASK_SLEEP;
    task->sleep_ticks = ticks;
    List_insert_tail(task_manager.sleep_list, &task->RunNode);
}

void task_set_wakeup(task_t* task){
    List_remove_node(task_manager.sleep_list, &task->RunNode);
}


task_t* task_get_current(void){
    return task_manager.current_task;
}

task_t* task_get_rdyto_run(void){
    if(List_isEmpty(task_manager.ready_list)){
        return task_manager.idle_task; // 返回空闲任务
    }
    ListNode* node = List_Front(task_manager.ready_list);

    return absolute_offset(node, task_t, RunNode); // 获取包含 RunNode 的 task_t 结构体指针
}

void task_yield(void){
    if(List_size(task_manager.ready_list) > 1){ // 如果就绪队列中有多个任务，才进行主动放弃
        task_t * current = task_get_current();
        task_set_blocked(current); // 将当前任务设置为阻塞状态并从就绪队列中移除
        task_set_ready(current); // 将当前任务重新加入就绪队列
        task_schedule(); // 调度器选择下一个任务运行
    }
}

void task_schedule(void){
    task_t* next = task_get_rdyto_run();
    if(next && next != task_manager.current_task){
        task_t* current = task_get_current();
        task_manager.current_task = next; // 切换当前任务指针
        next->state = TASK_RUNNING; // 设置下一个任务状态为运行
        task_switch_from_to(current, next); // 切换到下一个任务
    }
}

void task_timeslice_tick(void){
    task_t * curr = task_get_current();

    // 进入保护
    uint32_t state = irq_enter_protection();
    if(curr){
        curr->ticks++;
        if(curr->ticks >= curr->time_slice){
            curr->ticks = 0; // 重置时间片计数
            task_set_blocked(curr); // 将当前任务设置为阻塞状态并从就绪队列中移除
            task_set_ready(curr); // 将当前任务重新加入就绪队列
        }
    }

    // 睡眠任务扫描处理
    ListNode * tasknode_at_sleep = List_Front(task_manager.sleep_list);
    while(tasknode_at_sleep){
        task_t *task_at_sleep = absolute_offset(tasknode_at_sleep, task_t, RunNode);
        if(--task_at_sleep->sleep_ticks == 0){
            task_set_wakeup(task_at_sleep);
            task_set_ready(task_at_sleep);
        }
        tasknode_at_sleep = tasknode_at_sleep->next;
    }

    task_schedule();
    irq_exit_protection(state);
}

void sys_sleep(uint16_t time_ms){
    if(time_ms <= OS_TICK_MS){
        time_ms = OS_TICK_MS;
    }
    uint32_t state = irq_enter_protection();

    task_set_blocked(task_manager.current_task);
    task_set_sleep(task_manager.current_task, ((time_ms + OS_TICK_MS - 1) / OS_TICK_MS));

    task_schedule();

    irq_exit_protection(state);
}

