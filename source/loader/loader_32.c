#include "loader.h"
#include "comm/elf.h"

static void read_disk(uint32_t sector, uint32_t sector_count, void *buffer){ // read disk function,sector is the starting sector, count is the number of sectors to read, buffer 数据存放的位置
outb(0x1f6,0xE0);//0x1f6寄存器的第5六7位设置为1
outb(0x1f2,(uint8_t)(sector_count>>8));//0x1f2寄存器的第8位到第15位存放要读取的扇区数的高8位
outb(0x1f3,(uint8_t)(sector>>24));//0x1f3寄存器的第24位到第31位存放要读取的扇区数的低24位
outb(0x1f4,0);
outb(0x1f5,0);

outb(0x1f2,(uint8_t)(sector_count));//0x1f2寄存器的第0位到第7位存放要读取的扇区数的低8位
outb(0x1f3,(uint8_t)(sector));//0x1f2寄存器的第0位到第7位存放要读取的扇区数的低8位
outb(0x1f4,(uint8_t)(sector>>8));//
outb(0x1f5,(uint8_t)(sector>>16));

outb(0x1f7,0x24);////0x1f7寄存器的第5位设置为1，表示要读取扇区数据

uint16_t *data_buf = (uint16_t *)buffer;
while(sector_count--){
    while((inb(0x1f7) & 0x88) != 0x8){};

    for(int i=0;i<SECTOR_SIZE/2;i++){
        *data_buf++ = inw(0x1f0);
}
}
}


static uint32_t reload_elf_file (uint8_t * file_buffer) {
    // 读取的只是ELF文件，不像BIN那样可直接运行，需要从中加载出有效数据和代码
    // 简单判断是否是合法的ELF文件
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    if ((elf_hdr->e_ident[0] != ELF_MAGIC) || (elf_hdr->e_ident[1] != 'E')
        || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')) {
        return 0;
    }

    // 然后从中加载程序头，将内容拷贝到相应的位置
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

		// 全部使用物理地址，此时分页机制还未打开
        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }

		// memsz和filesz不同时，后续要填0
		dest= (uint8_t *)phdr->p_paddr + phdr->p_filesz;
		for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
			*dest++ = 0;
		}
    }

    return elf_hdr->e_entry;
}


static void dile_error(char *msg){
    while(1){
        for(;;);
    }
}

#define PDE_P  (1 << 0) // Present
#define PDE_W  (1 << 1) // Writable
#define PDE_PS (1 << 7) // Page size
#define CR4_PSE   (1 << 4) // Page size extension
#define CR0_PG    (1 << 31) // paging enable
void enable_page_mode(void){
    static uint32_t page_dir[1024] __attribute__((aligned(4096))) = {//
        [0] = PDE_P |PDE_W | PDE_PS | 0
    }; //高10位地址对表进行索引 页目录表
    
    uint32_t cr4 = read_cr4();
    write_cr4(cr4 | CR4_PSE); //设置CR4的PSE位
    
    write_cr3((uint32_t)page_dir); //设置CR3的页目录表地址

    write_cr0(read_cr0() | CR0_PG); 
}


void load_kernel(void){
    read_disk(100,500,(uint8_t *)SYS_KERNEL_ADDR);//将内核放在100扇区开始的地方(1M)，500个扇区大小

    uint32_t kernel_entry = reload_elf_file((uint8_t *)SYS_KERNEL_ADDR);//加载内核，返回入口地址
    if(kernel_entry == 0){
        dile_error("Invalid kernel file");
    }

    enable_page_mode(); //开启分页机制
      ((void (*)(boot_info_t *))kernel_entry)(&boot_info);//跳转到内核入口地址
    for(;;){}
}