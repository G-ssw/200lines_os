#include "tools/log.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"

void log_init(void) {
    // 初始化日志系统，例如设置日志级别、打开日志文件等
    outb(COM1_PORT + 1, 0x00); // 禁止所有中断
    outb(COM1_PORT + 3, 0x80); // 设置波特率分频器访问
    outb(COM1_PORT + 0, 0x03); // 设置波特率为 38400
    outb(COM1_PORT + 1, 0x00); // 设置数据位为 8 位
    outb(COM1_PORT + 3, 0x03); // 设置数据位为 8 位，无校验位，1 个停止位
    outb(COM1_PORT + 2, 0xC7); // 设置 FIFO 控制寄存器

    outb(COM1_PORT + 4, 0x0F); // 设置调制解调器控制寄存器，启用数据终端准备好信号
}

void log_printf(const char *format, ...) {
    // 实现一个简单的日志打印函数，支持格式化输出
    char buffer[128];
    va_list args; 

    kernel_memset(buffer, 0, sizeof(buffer)); // 清空缓冲区

    va_start(args, format); // 初始化可变参数列表
    kernel_vsnprintf(buffer, sizeof(buffer), format, args); // 格式化输出到缓冲区
    va_end(args); // 结束可变参数处理
    
    const char *p = buffer;
    while(*p != '\0') {
        // 处理格式化字符串
        while((inb(COM1_PORT + 5) & (1 << 6)) == 0); // 等待发送缓冲区空
        outb(COM1_PORT, *p); // 发送字符到 COM1
        p++;
    }
    outb(COM1_PORT, '\r'); // 发送回车符
    outb(COM1_PORT, '\n'); // 发送换行符

}