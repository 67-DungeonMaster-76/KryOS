;; boot.asm
;; version 0.0.9
;; Bootloader with VBE graphics mode request and multiboot info passing
;; Fixed Multiboot header for ELF format with video mode

bits 32

section .text
    ; Multiboot header - must be within first 8KB of kernel
    align 4
    dd 0x1BADB002           ; magic
    dd 0x00000007           ; flags: page align (0), memory info (1), video mode (2)
    dd - (0x1BADB002 + 0x00000007) ; checksum (m+f+c should be zero)
    
    ; Placeholder fields to shift video settings to correct offset
    ; (for ELF format, these are ignored by GRUB)
    dd 0                    ; header_addr placeholder
    dd 0                    ; load_addr placeholder
    dd 0                    ; load_end_addr placeholder
    dd 0                    ; bss_end_addr placeholder
    dd 0                    ; entry_addr placeholder
    
    ; Video mode info (for VBE) - now at correct offset
    dd 0                    ; mode_type (0 = linear graphics)
    dd 800                  ; width
    dd 600                  ; height
    dd 32                   ; depth

global start
extern k_main               ; k_main is defined in kernel.c

start:
    ; Set up the stack
    mov esp, stack_top      ; Set stack pointer to top of stack
    
    ; Clear direction flag
    cld
    
    ; Call kernel main with (magic, mbi)
    ; EAX = magic number (0x2BADB002)
    ; EBX = multiboot info pointer
    ; C calling convention: push args right to left
    push ebx                ; arg2: multiboot info pointer
    push eax                ; arg1: magic number
    call k_main
    add esp, 8              ; Clean up stack
    
    ; Halt the CPU
    cli                     ; Disable interrupts
    hlt                     ; Halt

; Infinite loop as fallback
.hang:
    jmp .hang

section .bss
    ; Reserve 16KB for stack
    align 16
stack_bottom:
    resb 16384              ; 16 KB stack
stack_top:
