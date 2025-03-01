#ifndef BITMAP_H
#define BITMAP_H
#include"comm/types.h"
typedef struct _bitmap_t {
    int     bitcount;//位图的总位数
    uint8_t * bits;//字节数组，外部参数传过来
}bitmap_t;
int bitmap_byte_count(int bitcount);
void bitmap_init(bitmap_t * bitmap, uint8_t * bits, int bitcount , int init_bit);
int bitmap_get_bit(bitmap_t * bitmap, int index);//获取某个位的状态值
void bitmap_set_bit(bitmap_t * bitmap, int index, int count, int bit);//从index开始，设置count个位的状态值
int bitmap_is_set(bitmap_t * bitmap, int index);
int bitmap_alloc_nbits(bitmap_t * bitmap, int bit, int count);//分配多个页，
#endif