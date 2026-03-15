s[BITS 32]

MBALIGN  equ 1 << 0
MEMINFO  equ 1 << 1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Stack pour le kernel

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

;;;;;;; Code du kernel ;;;;;;

section .text
global start
extern kernel_main

start:
    mov esp, stack_top

    push ebx
    push eax

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang