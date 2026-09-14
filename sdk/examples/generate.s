# Independent client: imports ONLY FortunaOracle.dll and Kernel32.
# clang --target=x86_64-pc-windows-msvc -c generate.s -o generate.obj
# link /nodefaultlib /entry:start /subsystem:console generate.obj FortunaOracle.lib kernel32.lib
# Place FortunaOracle.dll next to the resulting generate.exe; exit 0 = success.
.intel_syntax noprefix
.section .bss
.p2align 3
masks: .skip 40
.section .text
.globl start
.seh_proc start
start:
    sub rsp,40
    .seh_stackalloc 40
    .seh_endprologue
    call FortunaOracleGetAbiVersion
    cmp eax,0x10000
    jne failed
    call FortunaOracleInitialize
    test eax,eax
    jnz failed
    mov ecx,5
    lea rdx,[rip+masks]
    mov r8d,5
    call FortunaOracleGenerate
    test eax,eax
    jnz failed
    xor ecx,ecx
    call ExitProcess
failed:
    mov ecx,1
    call ExitProcess
.seh_endproc
