#ifndef KLIB_H
#define KLIB_H
#include <stdarg.h>
#include"comm/types.h"
// void kernel_strcpy(char *dest, const char *src);
// void kernel_strncpy(char *dest, const char *src, int size);//c操作系统
// int kernel_strncmp(const char *s1, const char *s2, int size);
// int kernel_strlen(const char *s);
// void kernel_memcpy(void *dest, void *src, int size);
// void kernel_memset(void *dest, uint8_t v, int size);
// int kernel_memcmp(void *d1, void *d2, int size);
// void kernel_vsprintf(char *str_buf, const char *fmt, va_list arg_ptr);


void kernel_strcpy (char * dest, const char * src);
void kernel_strncpy(char * dest, const char * src, int size);
int kernel_strncmp (const char * s1, const char * s2, int size);
int kernel_strlen(const char * str);
void kernel_memcpy (void * dest, void * src, int size);
void kernel_memset(void * dest, uint8_t v, int size);
int kernel_memcmp (void * d1, void * d2, int size);
void kernel_itoa(int num, char *str, int base);
void kernel_sprintf(char * buffer, const char * fmt, ...);

void kernel_vsprintf(char * buffer, const char * fmt, va_list args);

// #ifndef RELEASE
// #define ASSERT(expr)   \
//     if(!(expr)) panic(__FILE__, __LINE__, __func__, #expr)//应该是 !(expr) 来检查 expr 是否为假。
// void panic (const char * file, int line, const char *func,const char * cond);//宏定义，用于在调试模式下输出错误信息。
//     #else
// #define ASSERT(expr)    ((void)0)
// #endif

// #endif

#ifndef RELEASE
#define ASSERT(condition)    \
    if (!(condition)) panic(__FILE__, __LINE__, __func__, #condition)
void panic (const char * file, int line, const char * func, const char * cond);
#else
#define ASSERT(condition)    ((void)0)
#endif

#endif //KLIB_H