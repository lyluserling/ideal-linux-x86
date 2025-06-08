#include "core/memory.h"
#include "tools/klib.h"

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



static addr_alloc_t paddr_alloc;        // 物理地址分配结构
static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE))); // 内核页目录表


/**
 * @brief 初始化地址分配结构
 * 以下不检查start和size的页边界，由上层调用者检查
 */
static void addr_alloc_init (addr_alloc_t * alloc, uint8_t * bits,
                    uint32_t start, uint32_t size, uint32_t page_size) { // 初始化地址分配器
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, alloc->size / page_size, 0);
}

/**
 * @brief 分配多页内存
 */
static uint32_t addr_alloc_page (addr_alloc_t * alloc, int page_count) {//分配page_count个页
    uint32_t addr = 0;

    mutex_lock(&alloc->mutex);

    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);//分配page_count个页
    log_printf("alloc start  0x%x", alloc->start);
    log_printf("bitmap_alloc_nbits de return page_index  %d\n", page_index);
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

void show_mem_info(boot_info_t *boot_info) {
    log_printf("memory region:");
    for (int i = 0; i<boot_info->ram_region_count; i++){
        log_printf("[%d]:0x%x-0x%x",i,
            boot_info->ram_region_cfg[i].start,
            boot_info->ram_region_cfg[i].size);//打印出内存段的起始地址和大小
    }
    log_printf("\n");
}

/**
 * @brief 将指定的地址空间进行一页的映射
 */
int memory_create_map (pde_t * page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm) {
    for (int i = 0; i < count; i++) {
        // log_printf("create map: v-0x%x p-0x%x, perm: 0x%x", vaddr, paddr, perm);

        pte_t * pte = find_pte(page_dir, vaddr, 1);
        if (pte == (pte_t *)0) {
            // log_printf("create pte failed. pte == 0");
            return -1;
        }

        // 创建映射的时候，这条pte应当是不存在的。
        // 如果存在，说明可能有问题
        // log_printf("\tpte addr: 0x%x", (uint32_t)pte);
        ASSERT(pte->present == 0);

        pte->v = paddr | perm | PTE_P;

        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }

    return 0;
}

/**
 * @brief 根据内存映射表，构造内核页表
 */
void create_kernel_table (void) {
    extern uint8_t s_text[], e_text[], s_data[], e_data[];
    extern uint8_t kernel_base[];

    // 地址映射表, 用于建立内核级的地址映射
    // 地址不变，但是添加了属性
    static memory_map_t kernel_map[] = {
        {kernel_base,   s_text,         0,              PTE_W},         // 内核栈区
        {s_text,        e_text,         s_text,         0},         // 内核代码区
        {s_data,        (void *)(MEM_EBDA_START - 1),   s_data,        PTE_W},      // 内核数据区
        {(void *)CONSOLE_DISP_ADDR, (void *)(CONSOLE_DISP_END - 1), (void *)CONSOLE_VIDEO_BASE, PTE_W},

        // 扩展存储空间一一映射，方便直接操作
        {(void *)MEM_EXT_START, (void *)MEM_EXT_END,     (void *)MEM_EXT_START, PTE_W},
    };

    // 清空页目录表
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));

    // 清空后，然后依次根据映射关系创建映射表
    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
        memory_map_t * map = kernel_map + i;

        // 可能有多个页，建立多个页的配置
        // 简化起见，不考虑4M的情况
        int vstart = down2((uint32_t)map->vstart, MEM_PAGE_SIZE);
        int vend = up2((uint32_t)map->vend, MEM_PAGE_SIZE);
        int page_count = (vend - vstart) / MEM_PAGE_SIZE;

        memory_create_map(kernel_page_dir, vstart, (uint32_t)map->pstart, page_count, map->perm);
    }
}



/**
 * @brief 获取可用的物理内存大小
 */
static uint32_t total_mem_size(boot_info_t * boot_info) {
    int mem_size = 0;

    // 简单起见，暂不考虑中间有空洞的情况
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        mem_size += boot_info->ram_region_cfg[i].size;
    }
    return mem_size;
}

/**
 * @brief 创建进程的初始页表
 * 主要的工作创建页目录表，然后从内核页表中复制一部分
 */
uint32_t memory_create_uvm (void) {
    pde_t * page_dir = (pde_t *)addr_alloc_page(&paddr_alloc, 1);
    if (page_dir == 0) {
        return 0;
    }
    kernel_memset((void *)page_dir, 0, MEM_PAGE_SIZE);

    // 复制整个内核空间的页目录项，以便与其它进程共享内核空间
    // 用户空间的内存映射暂不处理，等加载程序时创建
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    for (int i = 0; i < user_pde_start; i++) {
        page_dir[i].v = kernel_page_dir[i].v;
    }

    return (uint32_t)page_dir;
}

/**
 * @brief 销毁用户空间内存
 */
void memory_destroy_uvm (uint32_t page_dir) {
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t * pde = (pde_t *)page_dir + user_pde_start;

    ASSERT(page_dir != 0);

    // 释放页表中对应的各项，不包含映射的内核页面
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++) {
        if (!pde->present) {
            continue;
        }

        // 释放页表对应的物理页 + 页表
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++) {
            if (!pte->present) {
                continue;
            }

            addr_free_page(&paddr_alloc, pte_paddr(pte), 1);
        }

        addr_free_page(&paddr_alloc, (uint32_t)pde_paddr(pde), 1);
    }

    // 页目录表
    addr_free_page(&paddr_alloc, page_dir, 1);
}

void memory_init(boot_info_t *boot_info){
    extern uint8_t * mem_free_start;
    //测试代码
    // addr_alloc_t addr_alloc;
    // uint8_t bits[8];//位图大小为8字节 8x8=64位  int bitmap_size_in_bytes = (64 + 7) / 8;  // 需要 8 字节来表示 64 页的位图
    // //kernel_memset(bits,0,8);//*bits=0;

    // addr_alloc_init(&addr_alloc, bits,0x1000,64*4096,4096); //初始化地址分配器
    // for (int i = 0; i<32 ; i++){
    //     uint32_t addr = addr_alloc_page(&addr_alloc,2); //分配2页内存
    //     log_printf("alloc addr:0x%x", addr);  
    // }

    // uint32_t addr = 0x1000;//从开始的地方开始释放
    // for(int i = 0; i<32; i++){
    //     addr_free_page(&addr_alloc,addr,2);//释放2页内存
    //     addr += 2*4096;
    // }

    log_printf("memory init success");
    show_mem_info(boot_info); 

    uint8_t * mem_free = (uint8_t *)&mem_free_start; // 得到空闲内存的起始地址

    uint32_t   mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START;//得到1MB以上的空闲内存大小
    mem_up1MB_free = down2(mem_up1MB_free,MEM_PAGE_SIZE); // 向下取整到页大小
    log_printf("free memory: 0x%x, size:0x%x",MEM_EXT_START,mem_up1MB_free);//打印出剩余的内存大小

    // 4GB大小需要总共4*1024*1024*1024/4096/8=128KB的位图, 使用低1MB的RAM空间中足够
    // 该部分的内存仅跟在mem_free_start开始放置
    addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE);
    mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    ASSERT(mem_free < (uint8_t *)MEM_EBDA_START); // 确保空闲内存不超过1MB

    creat_kernel_tables(); // 创建内核表
}