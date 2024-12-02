__asm__(".code16gcc");

#include "loader.h"

boot_info_t boot_info;

static void show_msg(const char * msg){
    char c;

    while ((c = *msg++) != '\0')
    {
        /* code */
        asm(
            "mov $0xe, %%ah\n\t" //
            "mov %[ch], %%al\n\t" 
            "int $0x10" ::[ch]"r"(c)
        );
    }
    
}

void int_to_str(int num, char *buffer, int base) {
    char *p = buffer;
    int i;

    // 处理负数（可选，通常对于无符号数不需要）
    if (num < 0) {
        *p++ = '-';
        num = -num;
    }

    // 将数字转换为字符串
    do {
        i = num % base;
        *p++ = (i > 9) ? (i - 10) + 'a' : i + '0'; // 支持十进制和十六进制
        num /= base;
    } while (num);

    // 反转字符串
    *p-- = '\0'; // 终止字符串
    char temp;
    while (buffer < p) {
        temp = *buffer;
        *buffer++ = *p;
        *p-- = temp;
    }
}

static void detect_memory (void) {
        uint32_t signature, bytes, contID;
        SMAP_entry_t SMAP_entry;
        contID = 0;
        show_msg("try todetect memory:");
        char buffer[256];
        boot_info.ram_region_count = 0;
        for(int i = 0; i < BOOT_RAM_REGION_MAX; i++){
            SMAP_entry_t * entry = &SMAP_entry;
            //显示BOOT_RAM_REGION_MAX的大小
            
            int_to_str(i, buffer, 10); // 将 BOOT_RAM_REGION_MAX 转换为字符串
            show_msg("BOOT_RAM_REGION_MAX size = ");
            show_msg(buffer);
            show_msg("\r\n");
           // show_msg("BOOT_RAM_REGION_MAX\r\n");
            __asm__ __volatile__ ("int $0x15"
                     : "=a"(signature), "=c"(bytes), "=b"(contID)
                     : "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry));
            if (signature != 0x534D4150){
                show_msg("SMAP signature error\r\n");
                 return; // error
            }
           
            if (bytes > 20 && (entry->ACPI & 0x0001) == 0) {
            //    show_msg("RAM region %d: start 0x%08x, length 0x%08x, type 0x%02x\r\n", i, entry->base_addr, entry->length, entry->type);//ACPI bit 0 is 0 means it is not a reserved region
            }
            if(entry->Type == 1){
                boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
                boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
                boot_info.ram_region_count++;
            }
            if(contID == 0){
                break;
            }
        }
        show_msg("memory detect done\r\n");
}

uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF,0x0000,0x9A00,0x00CF},
    {0xFFFF,0x0000,0x9200,0x00CF},
};

static void enter_protected_mode(void) {
    cli();//关中断

    uint8_t v = inb(0x92);
    outb(0x92, v|0x2); // 进入保护模式

    // 加载GDT
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    uint32_t cr0 = read_cr0();
    cr0 |= 0x1; // 启用保护模式
    write_cr0(cr0);

    far_jump(8,(uint32_t)protected_mode_entry); // 跳转到保护模式的入口点
}


void loader_entry(void){
    show_msg(".....lodaDING......\n\r");
    detect_memory();
    enter_protected_mode();
    for (;;) {}
}