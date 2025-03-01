#include "tools/klib.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include <stdint.h>
#include "tools/log.h"
#include "comm/cpu_instr.h"


// #include"comm/types.h"
// #include "tools/klib.h"
// #include "comm/cpu_instr.h"
void kernel_strcpy(char *dest, const char *src){
     if(!dest ||!src) return;//保护程序免于出现由于无效指针访问导致的错误或崩溃
     while(*dest && *src){//当dest和src都不为'\0'时，复制字符
         *dest++ = *src++;//将src中的字符复制到dest中，直到遇到'\0'或src的末尾
     }

     *dest = '\0';//将dest的末尾添加'\0'
}
void kernel_strncpy(char *dest, const char *src, int size){//
    if(!dest ||!src || size == 0) return;//保护程序免于出现由于无效指针访问导致的错误或崩溃

    char *d = dest;//d指向dest的开始位置,dest 是 char *，表示它指向的内存是可以修改的,建立指针副本 d s
    const char *s = src;//s指向src的开始位置,const char *，意味着它指向的内容是只读的
    while(size-- && *s){//当size大于0且s不为'\0'时，复制字符
         *d++ = *s++;//将s中的字符复制到d中，直到遇到'\0'或s的末尾或size为0
     }
     //dest是字符串数组，size是字符串数组的大小包含了'\0' 所以size-1
    if(size == 0)
    *(d-1) = '\0';//将dest的末尾添加'\0'
    else
    *d = '\0';//将dest的末尾添加'\0'
    

}
int kernel_strncmp(const char *s1, const char *s2, int size){
    if(!s1 ||!s2) return 0;//保护程序免于出现由于无效指针访问导致的错误或崩溃\

    while(*s1 && *s2 && size--){//当s1和s2都不为'\0'且size大于0时，比较字符
         if(*s1 != *s2) return 1;//如果字符不相等，返回
         s1++;//指向下一个字符
         s2++;//指向下一个字符
}
    if(size == 0) return 0;//如果size为0，说明s1和s2相同，返回
}

int kernel_strlen(const char *s){
    if(!s) return 0;//保护程序免于出现由于无效指针访问导致的错误或崩溃
    const char *p = s;//p指向s的开始位置
    int len = 0;//len记录字符串的长度
    while(*p!='\0'){//当p不为'\0'时，len加1
         p++;
         len++;
    }
    return len;//返回字符串的长度
}
void kernel_memcpy(void *dest, void *src, int size){
    if(!dest || !src || size == 0) return ;//保护程序免于出现由于无效指针访问导致的错误或崩溃
    uint8_t *d = (uint8_t *)dest;//d指向dest的开始位置
    uint8_t *s = (uint8_t *)&src;//s指向c的开始位置
    while(size--){//当size大于0时，复制字符
         *d++ = *s++;//将s中的字符复制到d中，直到size为0
    }
}
void kernel_memset(void *dest, uint8_t v, int size){
    if(!dest || size == 0) return ;//保护程序免于出现由于无效指针访问导致的错误或崩溃

    uint8_t *d = (uint8_t *)dest;//d指向dest的开始位置
    while(size--){//当size大于0时，设置字符为v
         *d++ = v;//将v复制到d中，直到size为0
         }
}

int kernel_memcmp(void *d1, void *d2, int size){
    if(!d1 || !d2 || size == 0) return 0;//保护程序免于出现由于无效指针访问导致的错误或崩溃
    uint8_t *s1 = (uint8_t *)d1;//s1指向d1的开始位置
    uint8_t *s2 = (uint8_t *)d2;//s2指向d2的开始位置
    while(size--){//当size大于0时，比较字符
         if(*s1 != *s2) return 1;//如果字符不相等，返回
         s1++;//指向下一个字符
         s2++;//指向下一个字符
    }
    if(size == 0) return 0;//如果size为0，说明d1和d2相同，返回

}

void kernel_itoa(int num, char *buf, int base) {
    const char * numchar = "FEDCBA9876543210123456789ABCDEF";
    char * str = buf;//str指向buf的开始位置
    char * start = buf;//记录字符串开始位置
    if(base != 2 && base != 8 && base != 10 && base != 16){
        *str = '\0';
        return;
    }
    else if(num < 0 && base == 10){//负数
        *str++ = '-';  
        start++;
    }
    do
    {
        
        *str++ = numchar[num%base+15];
        num = num/base;
    } while (num != 0);
    *str-- = '\0';//将字符串的末尾添加'\0'
       while(start < str)//将字符串反转
       {
           char temp = *start;
           *start = *str;
           *str = temp;
           start++;
           str--;
       }
       
    
}

void kernel_vsprintf(char *str_buf, const char *fmt, va_list arg_ptr){
    enum{NORMAL,READ_FMT} state = NORMAL;
    char *p = str_buf;
    char ch;//char ch = *fmt;
    while((ch = *fmt++)){
        // *p++ = ch;
        switch (state)
        {
        case NORMAL:
            if(ch == '%'){state = READ_FMT;}
            else{
                *p++ = ch;
            }
            break;
        case READ_FMT:
            if(ch == 's'){
                const char * str = va_arg(arg_ptr,char *); //获取参数
                int len = kernel_strlen(str);
                while(len--){
                    *p++ = *str++;
                }
            }
            else if(ch == 'd'){
                int num_d = va_arg(arg_ptr,int);
                kernel_itoa(num_d,p,10);//void kernel_itoa(int num, char *str, int base) {
                p = p + kernel_strlen(p);
            }
            else if(ch == 'c'){
                char c = va_arg(arg_ptr,int);
                *p++ = c;
            }
            else if(ch == 'x'){
                int num_d = va_arg(arg_ptr,int);
                kernel_itoa(num_d,p,16);//void kernel_itoa(int num, char *str, int base) {
                p = p + kernel_strlen(p);
            }
            state = NORMAL;//回到正常状态
            break;
        
        default:
            break;
        }
    }

}

void kernel_sprintf(char * buffer, const char * fmt, ...){
    va_list arg_ptr;
  //  kernel_memset(str_buf, '\0', sizeof(str_buf));//清空缓冲区
    va_start(arg_ptr, fmt);//获取参数列表
    kernel_vsprintf(buffer, fmt, arg_ptr);//格式化输出到缓冲区
    va_end(arg_ptr);//结束获取参数列 表

}

    void panic(const char * file, int line, const char *func,const char * cond){
    log_printf("assert failed! %s", cond);//输出断言失败的信息
    log_printf("file:%s\n,line:%d\n,func:%s", file, line, func);//输出断言失败的位置信息
    for(;;)
    hlt();
}