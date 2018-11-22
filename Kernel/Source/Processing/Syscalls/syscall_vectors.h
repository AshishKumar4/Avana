#ifndef SYSCALLVECTS_H
#define SYSCALLVECTS_H 

#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Memory/mem.h"

void _sys_kill(uint32_t* syscall_stack);
void _sys_malloc(uint32_t* syscall_stack);
void _sys_free(uint32_t* syscall_stack);
void _sys_stdout(uint32_t* syscall_stack);
void __attribute__((optimize("O2"))) _sys_stdin(uint32_t* syscall_stack);
void _sys_fork(uint32_t* syscall_stack);

void _sys_openFile(uint32_t* syscall_stack);
void _sys_closeFile(uint32_t* syscall_stack);
void _sys_fileSeek(uint32_t* syscall_stack);
void _sys_fileTell(uint32_t* syscall_stack);
void _sys_readFile(uint32_t* syscall_stack);
void _sys_writeFile(uint32_t* syscall_stack);

#endif 