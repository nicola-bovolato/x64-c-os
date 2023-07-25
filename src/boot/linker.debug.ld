ENTRY(start)

KERNEL_ALIGN = 4K;
KERNEL_START = 1M;

SECTIONS {
    . = KERNEL_START;

    .boot :
    {
        /* ensure that the multiboot header is at the beginning of the binary */
        KEEP(*(.multiboot_header))
        *(.rodata .rodata.*)
        . = ALIGN(KERNEL_ALIGN);
    }

    .text :
    {
        *(.text .text.*)
        . = ALIGN(KERNEL_ALIGN);
    }

    .data :
    {
        *(.data .data.*)
        . = ALIGN(KERNEL_ALIGN);
    }

    .bss :
    {
        *(.bss .bss.*)
        . = ALIGN(KERNEL_ALIGN);
    }
}
