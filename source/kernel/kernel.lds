SECTIONS{
    PROVIDE(kernel_base = .);
    . = 0x10000;

    PROVIDE(s_text = .);

    .text : { *(.text) }
    .rodata : { *(.rodata) } 

    PROVIDE(e_text = .);// start of text section

    PROVIDE(s_data = .);// start of data section
    .data : { *(.data) }    
    .bss : { *(.bss) }
 
    PROVIDE(mem_free_start =.);
}