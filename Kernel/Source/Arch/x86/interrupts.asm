section .bss

esp_backup: RESD 1

esp_backup2: RESD 1

curr_dir: RESD 1
eax_backup: RESD 1

syscall_stack_back1: RESD 1
syscall_stack_back2: RESD 1
syscall_stack_back3: RESD 1

tmp_stack: RESD 1

tmp_intStack: RESD 4096

tmp_cr3: RESD 1

global t_c
t_c: RESD 1

global pfault_cr2 
pfault_cr2: RESD 1

global pfault_cr3 
pfault_cr3: RESD 1

global interrupts_tmp_esp
interrupts_tmp_esp: RESD 1

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Exception/Fault Handlers ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


[GLOBAL defaultExceptionHandler]
[EXTERN defaultExceptionHandle]

defaultExceptionHandler:
    cli 
    mov eax,0x6655
    hlt
    pusha

    call defaultExceptionHandle

    popa 
    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0

    push edx
    mov dx, 0x20                      ; PIT Timer End Of Interrupt
    mov ax, 0x20
    out dx, ax
    mov dx, 0xA0
    mov ax, 0x20
    out dx, ax
    pop edx
    pop eax

    iretd

[GLOBAL pageFaultHandler]
[EXTERN pageFaultHandle]

pageFaultHandler:
    cli 
    pusha

    mov eax, esp 
    mov [esp_backup], eax 

    mov eax, [interrupts_tmp_esp]
    mov esp, eax
    mov ebp, eax
    mov eax, cr2
    mov [pfault_cr2], eax

    mov eax, cr3 
    mov [pfault_cr3], eax
    call pageFaultHandle

    mov eax, [esp_backup]
    mov esp, eax

    popa 
    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0
    pop eax

    mov [eax_backup], eax 
    pop eax 
    mov eax, [eax_backup]
    iretd


[GLOBAL doubleFault_handler]
;[EXTERN doubleFault_handle]

doubleFault_handler:
    cli 
    mov eax, 0x4284
    hlt
    pusha

    mov eax, esp 
    mov [esp_backup], eax 

    mov eax, [interrupts_tmp_esp]
    mov esp, eax
    mov ebp, eax
    mov eax, cr2
    mov [pfault_cr2], eax

    mov eax, cr3 
    mov [pfault_cr3], eax
    ;call doubleFault_handle

    mov eax, [esp_backup]
    mov esp, eax

    popa 
    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0
    pop eax

    mov [eax_backup], eax 
    pop eax 
    mov eax, [eax_backup]
    iretd



[GLOBAL generalProtectionFault_handler]
;[EXTERN generalProtectionFault_handle]

generalProtectionFault_handler:
    cli 
    mov eax, 0x5544
    hlt
    pusha

    mov eax, esp 
    mov [esp_backup], eax 

    mov eax, [interrupts_tmp_esp]
    mov esp, eax
    mov ebp, eax
    mov eax, cr2
    mov [pfault_cr2], eax

    mov eax, cr3 
    mov [pfault_cr3], eax
    ;;call generalProtectionFault_handle

    mov eax, [esp_backup]
    mov esp, eax

    popa 
    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0
    pop eax

    mov [eax_backup], eax 
    pop eax 
    mov eax, [eax_backup]
    iretd


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; General Interrupt Handlers ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[GLOBAL defaultInterruptHandler]
[EXTERN defaultInterruptHandle]

defaultInterruptHandler:
    cli 
    mov eax,0x7788
    hlt
    pusha

    call defaultInterruptHandle

    popa 
    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0

    push edx
    mov dx, 0x20                      ; PIT Timer End Of Interrupt
    mov ax, 0x20
    out dx, ax
    mov dx, 0xA0
    mov ax, 0x20
    out dx, ax
    pop edx
    pop eax

    iretd


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Some Important Handlers ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[EXTERN PIT_Handle]
[GLOBAL PIT_Handler]


PIT_Handler:
  cli
  pusha
  ;mov eax, 0x5555
   ; hlt
  call PIT_Handle

  popa 
  push eax 
  push edx 
  mov dx, 0x20                      ; PIT Timer End Of Interrupt
  mov ax, 0x20
  out dx, ax
  mov dx, 0xA0
  mov ax, 0x20
  out dx, ax
  pop edx
  pop eax 
  iretd

[GLOBAL kb_handle]
;[EXTERN keyboardInterrupt_handler]

kb_handle:
    cli
    pusha

    mov eax, esp
    mov [esp_backup], eax

    ;call keyboardInterrupt_handler

    mov eax, [esp_backup]
    mov esp, eax

    popa

    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0
    
    push edx
    mov dx, 0x20                      ; PIT Timer End Of Interrupt
    mov ax, 0x20
    out dx, ax
    pop edx
    pop eax

    iretd

[GLOBAL mouse_handle]
;[EXTERN mouse_handler]

mouse_handle:
iretd;
    cli
    pusha

    mov eax, esp
    mov [esp_backup2], eax

    ;call mouse_handler

    mov eax, [esp_backup2]
    mov esp, eax

    popa

    push eax
    mov eax, 0xFEE000B0                ; APIC Timer End Of Interrupt
    mov dword [eax], 0
    ;    pop eax

    ;    push eax
    push edx
    mov dx, 0x20                      ; PIT Timer End Of Interrupt
    mov ax, 0x20
    out dx, ax
    mov dx, 0xA0
    mov ax, 0x20
    out dx, ax
    pop edx
    pop eax

    iretd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;