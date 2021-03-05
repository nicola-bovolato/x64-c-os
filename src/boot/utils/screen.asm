; EDX = memory address of a 0 terminated string
; Manipulates video memory directly instead of using interrupts
; Each character uses two bytes: first for color (3 high bits background, 4 low foreground), second for character
; To access a character on the screen (80 x 25 grid): 0xb8000 + 2 * (row * 80 + col)

%ifndef SCREEN_ASM
%define SCREEN_ASM

VGA_ADDRESS     equ 0xb8000         ; BIOS VGA memory address
VGA_ADDRESS_END equ 0xb8fa0         ; (80 rows * 25 col * 2 char size) + 0xb8000 start address
WHITE_ON_BLACK  equ 0x0f            ; Background = 0x00, Foreground = 0x0f
WHITE_ON_GREEN  equ 0x2f
WHITE_ON_RED    equ 0x4f

%macro print_string 2
    pusha

    mov edx, %1
    mov ebx, VGA_ADDRESS            ; EBX = video memory address

.print_string_loop:

    mov ah, %2                      ; AH = character color
    mov al, [edx]                   ; AL = character to print

    cmp al, 0                       ; check if string is terminated
    je .print_string_end            ; if true end

    mov [ebx], ax                   ; AX = color + char to print
    add ebx, 2                      ; Go to the next screen character
    add edx, 1                      ; Go to the next character of the string

    jmp .print_string_loop

.print_string_end:
    popa

%endmacro

%macro clear_screen 0
    pusha

    mov ebx, VGA_ADDRESS            ; EBX = video memory address

.clear_screen_loop:
    mov ah, WHITE_ON_BLACK          ; AH = character color
    mov al, 0x20                    ; AL = character to print (whitespace)

    cmp ebx, VGA_ADDRESS_END        ; check end of vga memory is reached
    jge .clear_screen_end           ; if ebx > end of vga memory then end

    mov [ebx], ax                   ; AX = color + char to print
    add ebx, 2                      ; Go to the next screen character

    jmp .clear_screen_loop

.clear_screen_end:
    popa

%endmacro

%endif
