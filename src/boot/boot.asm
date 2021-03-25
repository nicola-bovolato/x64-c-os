; Grub has already prepared protected mode for us
; we now need to switch to long mode and enable paging
; to execute 64 bit instructions, a 64 bit GDT is required
; we then far jump to enable long mode

global start

section .text
bits 32

%include "utils/screen.asm"

start:
    mov esp, stack_top  ; Update the stack pointer
    mov edi, ebx        ; Move Multiboot info pointer (stored in ebx at startup) to edi, to read it from kernel_main

    clear_screen

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call set_up_page_tables
    call enable_paging

    lgdt [gdt64.pointer]

    extern long_mode_start
    jmp gdt64.code:long_mode_start

    hlt

check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    print_string MSG_NO_MULTIBOOT, WHITE_ON_RED
    jmp error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    print_string MSG_NO_CPUID, WHITE_ON_RED
    jmp error

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    print_string MSG_NO_LONG_MODE, WHITE_ON_RED
    jmp error

error:
    hlt

set_up_page_tables:
    ; map pml4 table recursively
    mov eax, pml4_table
    or eax, 0b11 ; present + writable
    mov [pml4_table + 511 * 8], eax

    ; map first pml4 entry to pdp table
    mov eax, pdp_table
    or eax, 0b11 ; present + writable
    mov [pml4_table], eax

    ; map first pdp entry to pd table
    mov eax, pd_table
    or eax, 0b11 ; present + writable
    mov [pdp_table], eax

    ; the first gigabyte of memory will be identity mapped
.identity_map_1st_GiB:

    ; map each pd table entry to the respective pt table address
    mov ecx, 0  ; counter variable

.map_pd_table:

    mov eax, 4096       ; the size of a pt table
    mul ecx             ; multiply the size by a counter, to get the index of the nth table
    add eax, pt_table   ; add the offset of the first pt_table
    or eax, 0b11        ; present + writable
    mov [pd_table + ecx * 8], eax

    inc ecx             ; increase counter
    cmp ecx, 512        ; check if all the table entries have been mapped
    jne .map_pd_table

    ; identity map each entry of each pt table to a 4KiB frame
    mov ecx, 0          ; counter variable

.map_512_pt_tables:

    mov eax, 0x1000
    mul ecx             ; start address of ecx-th page
    or eax, 0b11        ; present + writable
    mov [pt_table + ecx * 8], eax   ; map ecx-th entry (the pt tables are declared in a contiguous memory region)

    inc ecx             ; increase counter
    cmp ecx, 512 * 512  ; if counter == 512 * 512, each entry of each pt table is mapped
    jne .map_512_pt_tables   ; else map the next entry

    ret

enable_paging:
    ; load pml4 to cr3 register (cpu uses this to access the Pml4 table)
    mov eax, pml4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

section .bss
align 4096
pml4_table:
    resb 4096
pdp_table:
    resb 4096
pd_table:
    resb 4096
pt_table:
    resb 4096 * 512   ; Reserve space for 512 page tables
stack_bottom:
    resb 4096 * 4     ; Reserve 16 kBytes for the kernel stack
stack_top:

section .data
    MSG_NO_MULTIBOOT                db "Multiboot not supported", 0
    MSG_NO_CPUID                    db "Cpuid not supported", 0
    MSG_NO_LONG_MODE                db "Long mode not supported", 0

section .rodata
gdt64:
    dq 0 ; zero entry
.code: equ $ - gdt64 ; new
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
