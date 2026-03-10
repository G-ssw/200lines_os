#ifndef KLIB_H
#define KLIB_H

#include "comm/types.h"

#include "stdarg.h"

void kernel_strcpy(char* dest, const char* src);
void kernel_strncpy(char* dest, const char* src, uint16_t n);
int kernel_strlen(char *s);
int kernel_strncmp(const char *s1, const char *s2, uint16_t n);
void kernel_memcpy(void* dest, const void* src, uint16_t n);
void kernel_memset(void* dest, int value, uint16_t n);
int kernel_memcmp(const void *d1, const void *d2, uint16_t size);
int kernel_vsnprintf(char *buffer, uint16_t size, const char *fmt, va_list args);


#endif