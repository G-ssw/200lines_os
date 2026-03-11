#ifndef LOG_H
#define LOG_H

#define COM1_PORT 0x3F8 // COM1 端口地址

void log_init(void);
void log_printf(const char *format, ...);
void panic(const char *file, int line, const char *func, const char *cond);

#ifndef RELEASE
#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            panic(__FILE__, __LINE__, __func__, #cond); \
        } \
    } while (0)
#else
#define ASSERT(cond) do { } while (0)
#endif

#endif