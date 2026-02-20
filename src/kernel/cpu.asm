;; cpu.asm
;; version 0.0.3
;; Low-level CPU functions: GDT, IDT, ISR, IRQ

bits 32

section .text

; ============================================
; GDT Functions
; ============================================

; Load GDT
global gdt_flush
extern gp
gdt_flush:
    lgdt [gp]           ; Load GDT with our gp
    mov ax, 0x10        ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2     ; 0x08 is the offset to our code segment: Far jump!
flush2:
    ret

; ============================================
; IDT Functions
; ============================================

; Load IDT
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

; ============================================
; ISR Handlers (0-31)
; ============================================

%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        cli
        push dword 0            ; Push dummy error code
        push dword %1           ; Push interrupt number
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli
        push dword %1           ; Push interrupt number (error code already pushed by CPU)
        jmp isr_common_stub
%endmacro

; ISRs without error code
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; ISR common stub
; Stack layout at this point:
; [esp+0]  = gs
; [esp+4]  = fs
; [esp+8]  = es
; [esp+12] = ds
; [esp+16] = edi
; [esp+20] = esi
; [esp+24] = ebp
; [esp+28] = esp (old)
; [esp+32] = ebx
; [esp+36] = edx
; [esp+40] = ecx
; [esp+44] = eax
; [esp+48] = error code
; [esp+52] = interrupt number
; [esp+56] = eip (return address)
extern isr_handler
isr_common_stub:
    pusha                   ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10            ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Pass register values to C handler
    ; We'll pass a pointer to a struct on stack
    mov eax, [esp + 52]     ; Get interrupt number
    push eax                ; arg1: int number
    
    mov eax, [esp + 56]     ; Get EIP (return address)
    push eax                ; arg2: eip
    
    mov eax, [esp + 36]     ; Get EAX (after pusha, offset changed)
    push eax                ; arg3: eax
    
    mov eax, [esp + 44]     ; Get EBX
    push eax                ; arg4: ebx
    
    mov eax, [esp + 40]     ; Get ESP (from pusha)
    push eax                ; arg5: esp
    
    call isr_handler
    add esp, 20             ; Clean up 5 arguments
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8              ; Clean up error code and int number
    iret

; ============================================
; IRQ Handlers (32-47)
; ============================================

%macro IRQ 2
    global irq%1
    irq%1:
        cli
        push dword 0
        push dword %2
        jmp irq_common_stub
%endmacro

IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; IRQ common stub
extern irq_handler
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; Stack layout after pushes:
    ; [esp+0] = gs, [esp+4] = fs, [esp+8] = es, [esp+12] = ds
    ; [esp+16-44] = pusha registers
    ; [esp+48] = interrupt number (pushed by IRQ macro)
    ; [esp+52] = error code (pushed by IRQ macro)
    mov eax, [esp + 48]     ; Get interrupt number
    push eax
    call irq_handler
    add esp, 4              ; Clean up argument
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
