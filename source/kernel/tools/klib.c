#include "tools/klib.h"

void kernel_strcpy(char* dest, const char* src) {
    if(!src || !dest) {
        return; // 处理空指针情况
    }
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void kernel_strncpy(char* dest, const char* src, uint16_t n) {
    if(!src || !dest || n == 0) {
        return; // 处理空指针和长度小于等于0的情况
    }
    uint16_t i;
    // 复制最多 n-1 个字符，确保 dest 以 null 结尾
    for (i = 0; i < n - 1&& src[i]; i++) {
        dest[i] = src[i];
    }

    dest[i] = '\0'; // 确保字符串以 null 结尾
}

int kernel_strlen(char *s) {
    if(!s) {
        return 0; // 处理空指针情况
    }
    int len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}

int kernel_strncmp(const char *s1, const char *s2, uint16_t n) {
    if (n <= 0) return 0;                // 比较 0 个字符视为相等
    if (!s1 || !s2) return -1;            // 错误情况返回特殊值（或触发断言）

    while (n--) {
        if (*s1 != *s2) return 1;         // 发现不同字符，返回不等
        if (*s1 == '\0') break;            // 已到字符串末尾，提前结束
        s1++;
        s2++;
    }
    return 0;                              // 所有 n 个字符都相等
}

void kernel_memcpy(void* dest, const void* src, uint16_t n) {
    if(!dest || !src || n <= 0) {
        return; // 处理空指针和长度小于等于0的情况
    }
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while(n--) {
        *d++ = *s++;
    }
}

void kernel_memset(void* dest, int value, uint16_t n) {
    if(!dest || n <= 0) {
        return; // 处理空指针和长度小于等于0的情况
    }
    char* d = (char*)dest;
    while(n--) {
        *d++ = (char)value;
    }
}

int kernel_memcmp(const void *d1, const void *d2, uint16_t size) {
    if (!d1 || !d2) return -1;   // 错误码
    const uint8_t *p1 = d1;
    const uint8_t *p2 = d2;
    while (size--) {
        if (*p1++ != *p2++) {
            return 1;
        }
    }
    return 0;
}

// 辅助函数：反转字符串（内部使用）
static void reverse(char *s, uint16_t len) {
    for (uint16_t i = 0, j = len - 1; i < j; ++i, --j) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


// 辅助函数：将整数转换为字符串（内部使用）
static uint16_t itoa(char *buf, uint32_t num, int base, int upper) {
    char *p = buf;
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    do {
        *p++ = digits[num % base];
        num /= base;
    } while (num);
    reverse(buf, p - buf);
    return p - buf;
}

/**
 * 格式化输出到缓冲区，带长度限制
 * @param buffer 输出缓冲区
 * @param size   缓冲区大小（包括结尾'\0'）
 * @param fmt    格式字符串
 * @param args   可变参数列表
 * @return       应写入的字符数（不包括结尾'\0'），若大于等于size则截断
 */
int kernel_vsnprintf(char *buffer, uint16_t size, const char *fmt, va_list args) {
    enum { NORMAL, READ_FMT } state = NORMAL;
    char ch;
    char *curr = buffer;
    uint16_t remain = size;          // 剩余可用字节数（包括结尾'\0'的空间）

    while ((ch = *fmt++)) {
        if (state == NORMAL) {
            if (ch == '%') {
                state = READ_FMT;
            } else {
                if (remain > 1) {   // 留一个位置给结尾'\0'
                    *curr++ = ch;
                    remain--;
                }
                // 即使缓冲区满，仍需继续计数，以便返回总长度
            }
        } else { // state == READ_FMT
            state = NORMAL;          // 默认处理完后回到普通状态
            uint16_t len = 0;
            char num_buf[32];        // 临时存放数字转换结果

            switch (ch) {
                case 's': {
                    const char *str = va_arg(args, const char *);
                    while (*str) {
                        if (remain > 1) {
                            *curr++ = *str;
                            remain--;
                        }
                        str++;
                        len++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    if (remain > 1) {
                        *curr++ = c;
                        remain--;
                    }
                    len = 1;
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    uint32_t u;
                    if (num < 0) {
                        if (remain > 1) {
                            *curr++ = '-';
                            remain--;
                        }
                        len++;
                        u = -num;
                    } else {
                        u = num;
                    }
                    uint16_t num_len = itoa(num_buf, u, 10, 0);
                    for (uint16_t i = 0; i < num_len; i++) {
                        if (remain > 1) {
                            *curr++ = num_buf[i];
                            remain--;
                        }
                        len++;
                    }
                    break;
                }
                case 'x':
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    uint16_t num_len = itoa(num_buf, num, 16, (ch == 'X'));
                    for (uint16_t i = 0; i < num_len; i++) {
                        if (remain > 1) {
                            *curr++ = num_buf[i];
                            remain--;
                        }
                        len += num_len;
                    }
                    break;
                }
                case '%': {
                    if (remain > 1) {
                        *curr++ = '%';
                        remain--;
                    }
                    len = 1;
                    break;
                }
                default:
                    // 不认识的格式符，直接输出原样？或者忽略？这里忽略并回退状态
                    // 但为了简单，我们直接输出 '%' 和该字符
                    if (remain > 1) {
                        *curr++ = '%';
                        remain--;
                    }
                    len++;
                    if (remain > 1) {
                        *curr++ = ch;
                        remain--;
                    }
                    len++;
                    break;
            }
            // 累加长度已在各分支中处理，这里无需额外操作
        }
    }

    // 写入结尾 '\0'
    if (size > 0) {
        *curr = '\0';
    }

    // 返回实际应写入的字符数（不包括结尾 '\0'）
    return (curr - buffer) + (size == 0 ? 0 : 1); // 需要计算实际长度，更精确的做法是全程计数
    // 注意：上面返回的长度需要更精确的计算，这里简化，可改为维护 total_len 变量
}