section .multiboot
ALIGN 4
dd 0x1BADB002
dd 3
dd -(0x1BADB005)

ALIGN 32
section .text
[EXTERN kernel_start]
[GLOBAL _start]
_start:
    call kernel_start 
    hlt
