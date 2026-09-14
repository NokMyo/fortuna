# Microsoft x64 ABI throughout. All nonvolatile GPRs are preserved. SSE2 only.
# Frame local storage starts at rsp+32; n must be a multiple of 16.
.macro FN name, n=0
    .globl \name
.ifdef WINDOWS
    .seh_proc \name
.endif
\name:
    push rbp
.ifdef WINDOWS
    .seh_pushreg rbp
.endif
    push rbx
.ifdef WINDOWS
    .seh_pushreg rbx
.endif
    push rsi
.ifdef WINDOWS
    .seh_pushreg rsi
.endif
    push rdi
.ifdef WINDOWS
    .seh_pushreg rdi
.endif
    push r12
.ifdef WINDOWS
    .seh_pushreg r12
.endif
    push r13
.ifdef WINDOWS
    .seh_pushreg r13
.endif
    push r14
.ifdef WINDOWS
    .seh_pushreg r14
.endif
    push r15
.ifdef WINDOWS
    .seh_pushreg r15
.endif
    sub rsp, 40+\n
.ifdef WINDOWS
    .seh_stackalloc 40+\n
    .seh_endprologue
.endif
.endm
.macro END n=0
    add rsp, 40+\n
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
.ifdef WINDOWS
    .seh_endproc
.endif
.endm
