#include "init.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "tools/log.h"
#include "os_cfg.h"
#include "core/task.h"
#include "tools/list.h"


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
        log_printf("Init task has been running for %d seconds...", count++);
        //task_yield(); // 主动放弃 CPU，切换到其他就绪的任务
        sys_sleep(1000);
    }
}

void list_test() {
    List list;
    ListNode Node[5];
    
    // 插入节点 头
    List_init(&list);
    log_printf("List is empty: %d", List_isEmpty(&list));
    for (int i = 0; i < 5; i++) {
        ListNode_init(&Node[i], (void*)(i + 1));
        List_insert_head(&list, &Node[i]);
    }
    log_printf("List Size: %d, List Head: %d, List Tail: %d", List_size(&list), List_Front(&list), List_Tail(&list));

    // 插入节点
    List_init(&list);
    log_printf("List is empty: %d", List_isEmpty(&list));
    for (int i = 0; i < 5; i++) {
        ListNode_init(&Node[i], (void*)(i + 1));
        List_insert_tail(&list, &Node[i]);
    }
    log_printf("List Size: %d, List Head: %d, List Tail: %d", List_size(&list), List_Front(&list), List_Tail(&list));

    // 删除节点
    for(int i = 0; i < 5; i++){
        ListNode* node = List_remove_head(&list);
        log_printf("Removed Node: %d, List Size: %d", node->data, List_size(&list));
    }

    // test offset_func
    struct Test_struct{
        int a;
        ListNode node;
    } test_struct = {0x123456};
    log_printf("Test_struct a: %x", test_struct.a);
    log_printf("Relative offset of node: %x", relative_offset(struct Test_struct, node));
    ListNode* node = &test_struct.node;
    log_printf("Absolute offset of node: %x", absolute_offset(node, struct Test_struct, node));
    if(absolute_offset(node, struct Test_struct, node)->a == 0x123456){
        log_printf("Offset function works correctly!");
    } else {
        log_printf("Offset function is incorrect!");
    }
}

void init_main(){
    
    //list_test();
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    task_create(&init_task, "Init Task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1023]); // 创建 init 任务
    first_task_init(); // 初始化第一个用户任务

    //int a = 3;
    //ASSERT(a < 2); // 触发断言测试
    //int a = 3 / 0; // 故意制造一个除零错误，测试异常处理
    int count = 0;
    irq_global_enable(); // 使能时钟中断
    for (;;) { 
        log_printf("first task: %d", count++);
        sys_sleep(1000);
        //task_yield(); // 主动放弃 CPU，切换到其他就绪的任务
    }
}