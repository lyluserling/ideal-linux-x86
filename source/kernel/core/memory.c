#include "core/memory.h"

// static void addr_alloc_init(addr_alloc_t * alloc , uint8_t * bits,
//     uint32_t start, uint32_t size, uint32_t page_size){//初始化地址分配器
//         mutex_init(&alloc->mutex);
//         alloc->start = start;// start address of the memory
//         alloc->size = size;// size of the memory
//         alloc->page_size = page_size;
//         bitmap_init(&alloc->bitmap, bits, alloc->size / page_size, 0);//初始化位图为0
//     }

// static uint32_t addr_alloc_page(addr_alloc_t * alloc, int page_count){//分配page_count个页
//     uint32_t addr = 0;
//     mutex_lock(&alloc->mutex);

//     int page_index =bitmap_alloc_nbits(&alloc->bitmap, 0,page_count);//分配page_count个页
//     if(page_index >= 0) //分配成功
//     {
//         addr = alloc->start + page_index * alloc->page_size;
//     }
//     mutex_unlock(&alloc->mutex);
//     return addr;
// }    
/**
 * @brief 初始化地址分配结构
 * 以下不检查start和size的页边界，由上层调用者检查
 */
static void addr_alloc_init (addr_alloc_t * alloc, uint8_t * bits,
                    uint32_t start, uint32_t size, uint32_t page_size) {
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, alloc->size / page_size, 0);
}

/**
 * @brief 分配多页内存
 */
static uint32_t addr_alloc_page (addr_alloc_t * alloc, int page_count) {
    uint32_t addr = 0;

    mutex_lock(&alloc->mutex);

    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);//分配page_count个页
    if (page_index >= 0) {
        addr = alloc->start + page_index * alloc->page_size;//page_index代表第几页，乘以page_size得到该页的起始地址
    }

    mutex_unlock(&alloc->mutex);
    return addr;
}

static void addr_free_page(addr_alloc_t * alloc, uint32_t addr, int page_count){
    mutex_lock(&alloc->mutex);
    uint32_t pg_index = (addr - alloc->start)/alloc->page_size; //得到了它在第几页
    bitmap_set_bit(&alloc->bitmap,pg_index,page_count,0);
    mutex_unlock(&alloc->mutex); 
}

void memory_init(boot_info_t *boot_info){
    addr_alloc_t addr_alloc;
    uint8_t bits[8];//位图大小为8字节 8x8=64位  int bitmap_size_in_bytes = (64 + 7) / 8;  // 需要 8 字节来表示 64 页的位图
    //kernel_memset(bits,0,8);//*bits=0;

    addr_alloc_init(&addr_alloc, bits,0x1000,64*4096,4096); //初始化地址分配器
    for (int i = 0; i<32 ; i++){
        uint32_t addr = addr_alloc_page(&addr_alloc,2); //分配2页内存
        log_printf("alloc addr:0x%x", addr);
    }

    uint32_t addr = 0x1000;//从开始的地方开始释放
    for(int i = 0; i<32; i++){
        addr_free_page(&addr_alloc,addr,2);//释放2页内存
        addr += 2*4096;
    }
}