#include "tools/bitmap.h"
#include "tools/klib.h"
int bitmap_byte_count(int bitcount){
    return (bitcount+7)/8;
}
void bitmap_init(bitmap_t * bitmap, uint8_t * bits, int bitcount , int init_bit){//初始化位图
    bitmap->bits = bits;//bits 目标地址
    bitmap->bitcount = bitcount;
    
    int bytes = bitmap_byte_count(bitmap->bitcount);//计算需要多少字节
    kernel_memset(bitmap->bits,init_bit ? 0xff : 0, bytes);//
}

int bitmap_get_bit(bitmap_t * bitmap, int index){
    return (bitmap->bits[index/8] & (1<<(index%8)))? 1 : 0;; //bitmap->bits[index/8]表示第index/8个字节，& (1<<(index%8))表示第index%8位是否为1，代码当前的返回值是一个 int，但是返回的可能是一个 非零整数（即按位与后的结果），而不一定是 0 或 1。
 }
void bitmap_set_bit(bitmap_t * bitmap, int index, int count, int bit){
    for(int i = 0;(i<count && index<bitmap->bitcount);i++,index++){
        if(bit == 1){
            bitmap->bits[index/8] |= (1<<(index%8));
        }
        else{
            bitmap->bits[index/8] &= ~(1<<(index%8));
        }
    }
}
int bitmap_is_set(bitmap_t * bitmap, int index){
    return bitmap_get_bit(bitmap, index)? 1 : 0;
}
/**
 * @brief 连续分配若干指定比特位，返回起始索引
 */
int bitmap_alloc_nbits (bitmap_t * bitmap, int bit, int count) {
    int search_idx = 0;
    int ok_idx = -1;

    while (search_idx < bitmap->bitcount) {
        // 定位到第一个相同的索引处
        if (bitmap_get_bit(bitmap, search_idx) != bit) {
            // 不同，继续寻找起始的bit
            search_idx++
            continue;
        }

        // 记录起始索引
        ok_idx = search_idx;

        // 继续计算下一部分
        int i;
        for (i = 1; (i < count) && (search_idx < bitmap->bitcount); i++) {
            log_printf("Checking bit at search_idx  %d\n", search_idx);
            if (bitmap_get_bit(bitmap, search_idx) != bit) {
                // 不足count个，退出，重新进行最外层的比较
                search_idx++;
                log_printf("不足count个，退出，重新进行最外层的比较\n");
                ok_idx = -1;
                break;
            }
            search_idx++;
        }

        // 找到，设置各位，然后退出
        if (i >= count) {
            log_printf("Found %d consecutive free bits starting at index %d\n", count, ok_idx);
            bitmap_set_bit(bitmap, ok_idx, count, ~bit);
            return ok_idx;
        }
    }
    log_printf("No free space found for %d bits\n", count);
    return -1;
}
