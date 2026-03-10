#ifndef LOG_H
#define LOG_H

#define COM1_PORT 0x3F8 // COM1 端口地址

void log_init(void);
void log_printf(const char *format, ...);

#endif