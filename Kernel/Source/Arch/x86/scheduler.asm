section .bss

section .text

[GLOBAL Scheduler_t]
[GLOBAL Scheduler_end_t]


;Symbol Table->
;      top_queue           ->        0x4284ABD1
;      reached_bottom      ->        0x4284ABD2
;      bottom_task         ->        0x4284ABD3
;      current_task        ->        0x4284ABD4
;      LAST_QUEUE          ->        0x4284ABD5
;      time_slice          ->        0x4284ABD6

;The Scheduler Rewritten in Assembly
; C code -->
; uint32_t Scheduler_t() 
; {
;   if (top_queue == LAST_QUEUE) 
;   {
;     if (bottom_task >= top_queue[0]) 
;     {
;       bottom_task = 1;
;     }
;     else 
;     {
;       bottom_task += 1;
;     }
;     while (&top_queue[bottom_task] == 0)
;     {
  ;     if (bottom_task >= top_queue[0]) 
  ;     {
  ;       bottom_task = 1;
  ;     }
  ;     else 
  ;     {
  ;       bottom_task += 1;
  ;     }
;     }

;     current_task = &top_queue[bottom_task];
;     current_task->task->active = top_queue;
;   } 
;   else if (top_queue[0] != 1) 
;   {
;     uint32_t* currentQueue = top_queue;
;     uint32_t* firstElemTopQueue = &currentQueue[1];
;     uint32_t* lastElemTopQueue = &currentQueue[currentQueue[0]];
;     currentQueue[0] -= 1;
;     uint32_t* lowerQueue = currentQueue + 1024;
;     lowerQueue[0] += 1;
;     uint32_t lastElemBelowQueue = &lowerQueue[lowerQueue[0]];
;     *lastElemBelowQueue = *firstElemTopQueue;
;     *firstElemTopQueue = *lastElemTopQueue;
;     *lastElemTopQueue = 0;
;     current_task = *lastElemBelowQueue;
;     current_task->task->active = lowerQueue;
;   } 
;   else 
;   {
;     uint32_t* currentQueue = top_queue;
;     uint32_t* firstElemTopQueue = &currentQueue[1];
;     currentQueue[0] -= 1;
;     uint32_t* lowerQueue = currentQueue + 1024;
;     lowerQueue[0] += 1;
;     uint32_t lastElemBelowQueue = &lowerQueue[lowerQueue[0]];
;     *lastElemBelowQueue = *firstElemTopQueue;
;     *firstElemTopQueue = 0;
;     current_task = *lastElemBelowQueue;
;     current_task->task->active = lowerQueue;
;     top_queue = lowerQueue;
;   }
; }

Scheduler_t:
  ; mov dword edx, [0x4284ABD1]
  ; mov dword eax, [0x4284ABD2]
  ; cmp dword eax, 0
  ; jne Bottom_Schedule_t                         ; if (reached_bottom) { Bottom_Schedule_t() }

  mov dword edx, [0x4284ABD1]

  mov dword ebx, edx
  mov dword eax, [0x4284ABD5]
  cmp dword eax, ebx
  je Bottom_Schedule_t                         ; if (reached_bottom) { Bottom_Schedule_t() }
  
  mov dword eax, [edx]
  cmp dword eax, 1
  je StepDown_Schedule_t                        ; if (top_queue[0] == 1) { StepDown_Schedule_t() }
                                                  ; GENERAL
  mov eax, edx                                    ; first_curr  = _q + 1
  add dword eax, 4                                ; as firstElementInQueue = eax = &top_queue[1]

  mov dword ebx, [edx]                                    ; last_curr; ebx = top_queue[0]
  shl dword ebx, 2
  add dword ebx, edx                              ; ebx = top_queue + top_queue[0]*4 

  sub dword [edx], 1                              ; top_queue[0] =- 1
  add dword edx, 4096                             ; top_queue = top_queue + 4096
  add dword [edx], 1                              ; top_queue[0] =+ 1

  mov dword ecx, [edx]                                    ; last_lower; ecx = top_queue[0]
  shl dword ecx, 2                               
  add dword ecx, edx                               ; ecx = top_queue + top_queue[0]*4

  mov dword edx, [eax]                                  ; *last_lower = *first_curr
  mov dword [ecx], edx

  mov dword edx, [ebx]                                  ; *first_curr = *last_curr
  mov dword [eax], edx

  mov dword [ebx], 0                                    ; *last_curr = 0

  mov dword ebx, [ecx]
  mov dword [0x4284ABD4], ebx                         ; 0x4284ABD4 = *last_lower
  mov dword ebx, [ebx]                                ; Get Task Structure from ThreadTableEntry
  mov dword [ebx+24], ecx                               ; 0x4284ABD4->active = last_lower

  mov dword [0x4284ABD6], 0xf0000


  ret

HALT:
  hlt


StepDown_Schedule_t:
  mov dword eax, edx                                    ; first_curr  = _q + 1
  add dword eax, 4

  sub dword [edx], 1
  add dword edx, 4096
  add dword [edx], 1

  mov dword ecx, [edx]                                    ; last_lower
  shl dword ecx, 2
  add dword ecx, edx

  mov dword ebx, [eax]                                  ; *last_lower = *first_curr
  mov dword [ecx], ebx

  mov dword [eax], 0                              ; *first_curr = 0

  mov dword [0x4284ABD4], ebx                         ; 0x4284ABD4 = *last_lower
  mov dword ebx, [ebx]                                ; Get Task Structure from ThreadTableEntry
  mov dword [ebx+24], ecx                               ; 0x4284ABD4->active = last_lower

  mov dword [0x4284ABD1], edx                            ; 0x4284ABD1 = _q

  mov dword ebx, edx
  mov dword eax, [0x4284ABD5]
  cmp dword eax, ebx
  je bottomReached_exit_t

  ;mov dword eax, [0x4284ABD1]
  ;and dword eax, 0xfffff
  ;shr dword eax, 5
  mov dword [0x4284ABD6], 0xf0000

  ret

bottomReached_exit_t:
  ; mov dword [0x4284ABD2], 1

  ;mov dword eax, [0x4284ABD1]
  ;and dword eax, 0xfffff
  ;shr dword eax, 5
  mov dword [0x4284ABD6], 0xf0000

  ret

Bottom_Schedule_t:
  mov dword eax, [edx]
  mov dword ebx, [0x4284ABD3]
  cmp dword ebx, eax
  jl alternate_flow_t

  mov dword [0x4284ABD3], 1
  jmp back_t

back_t:
  shl dword ebx, 2
  add dword ebx, edx
  cmp dword [ebx], 0
  jne option1_t                                      ; if(*tmp)
                                                   ; else
  mov dword eax, [edx]
  mov dword ebx, [0x4284ABD3]
  cmp dword ebx, eax 
  jl back_reset_t

  add dword [0x4284ABD3], 1
  jmp back_t

back_reset_t:
  mov dword [0x4284ABD3], 1
  jmp back_t

option1_t:
  mov dword eax, [ebx]
  mov dword [0x4284ABD4], eax
  mov dword eax, [eax]                                ; Get Task Structure from ThreadTableEntry
  mov dword [eax + 24], ebx

  ;mov dword eax, [0x4284ABD1]
  ;and dword eax, 0xfffff
  ;shr dword eax, 5
  mov dword [0x4284ABD6], 0xf0000

  ret

alternate_flow_t:
  add dword [0x4284ABD3], 1
  jmp back_t

Scheduler_end_t:


;********************CODE TO STOP SPECIFIC CPU WHEN REQUIRED***********************

;  mov dword eax, 0xfee00020                 ; LAPIC ID entry
;  mov dword eax, [eax]
;
;  shr eax, 24
;  cmp eax, 0
;  jne HALT
