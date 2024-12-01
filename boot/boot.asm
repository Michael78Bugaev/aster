BITS 32

section .text
    ALIGN 4
    DD 0x1BADB002
    DD 0x00000000
    DD -(0x1BADB002 + 0x00000000)

    dd 0 ; skip some flags
    dd 0
    dd 0
    dd 0
    dd 0

    dd 0 ; sets it to graphical mode
    dd 800 ; sets the width
    dd 600 ; sets the height
    dd 32 ; sets the bits per pixel

global start
global stack_space
extern kentr
extern vga_80x50
extern enter_real_mode
extern enter_protected_mode

push ebx

start:
    CLI
    MOV esp, [esp]
    CALL kentr
    HLT
HaltKernel:
    HLT
    JMP HaltKernel

section .bss
RESB 8192
stack_space: