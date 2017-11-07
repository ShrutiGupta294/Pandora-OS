section .bss


section .text

[GLOBAL switcher_ksp_t]
[GLOBAL switcher_end_t]

switcher_ksp_t:
    cli 
    pusha
    mov dword eax, [0x4284ACD3]         ;   Thread Structure
    mov dword [eax], esp                ;   Save Esp

    mov dword esp, 0x4284CD00           ;   Change to some temporary stack, I dont want to take risk

    call [0x4284ACD5]                   ;   call our scheduler
    
    mov dword eax, [0x4284ACD3]         ;   Get New Task Entry
 ;   hlt
    mov dword esp, [eax]                ;   Load New Stack

    popa                                ;   Unload all Registers

    push dword eax                            ;   Safety Purpose
    push dword edx
    mov dx, 0x20                      ; PIC Timer End Of Interrupt
    mov ax, 0x20
    out dx, ax
    mov dword eax, 0xFEE00380               ; Reset LVT Timer counter
    mov dword edx, 0x4000
    mov dword [eax], edx
    pop dword edx
    pop dword eax

    iretd

switcher_end_t:


