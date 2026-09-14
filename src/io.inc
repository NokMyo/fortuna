.section .bss
ledger_path: .skip 2048
io_size: .skip 8
io_bytes: .skip 4
.section .rdata,"dr"
app_title: .short 70,111,114,0
.section .text
FN LoadCsvFile,32
    mov rbx,rcx
    mov dword ptr [rip+parse_error],6
    mov dword ptr [rip+parse_line],0
    mov edx,0x80000000
    mov r8d,1
    xor r9d,r9d
    mov qword ptr [rsp+32],3
    mov qword ptr [rsp+40],0x80
    mov qword ptr [rsp+48],0
    call CreateFileW
    cmp rax,-1
    je .lf_fail
    mov rsi,rax
    mov rcx,rax
    lea rdx,[rip+io_size]
    call GetFileSizeEx
    test eax,eax
    jz .lf_close
    mov rdi,[rip+io_size]
    cmp rdi,16777216
    ja .lf_close
    test rdi,rdi
    jz .lf_close
    call GetProcessHeap
    mov rcx,rax
    xor edx,edx
    mov r8,rdi
    call HeapAlloc
    test rax,rax
    jz .lf_close
    mov rbx,rax
    mov rcx,rsi
    mov rdx,rbx
    mov r8d,edi
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call ReadFile
    xor ebp,ebp
    test eax,eax
    jz .lf_free
    cmp edi,[rip+io_bytes]
    jne .lf_free
    mov rcx,rbx
    mov rdx,rdi
    call ParseCsv
    mov ebp,eax
.lf_free:
    call GetProcessHeap
    mov rcx,rax
    xor edx,edx
    mov r8,rbx
    call HeapFree
    mov rcx,rsi
    call CloseHandle
    mov eax,ebp
    jmp .lf_out
.lf_close:
    mov rcx,rsi
    call CloseHandle
.lf_fail: xor eax,eax
.lf_out: END 32

FN WriteBytes,32
    mov rbx,rdx
    mov edi,r8d
    mov edx,0x40000000
    xor r8d,r8d
    xor r9d,r9d
    mov qword ptr [rsp+32],2
    mov qword ptr [rsp+40],0x80
    mov qword ptr [rsp+48],0
    call CreateFileW
    cmp rax,-1
    je .wb_fail
    mov rsi,rax
    mov rcx,rax
    mov rdx,rbx
    mov r8d,edi
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call WriteFile
    xor ebp,ebp
    test eax,eax
    jz .wb_close
    cmp edi,[rip+io_bytes]
    jne .wb_close
    mov rcx,rsi
    call FlushFileBuffers
    mov ebp,eax
.wb_close:
    mov rcx,rsi
    call CloseHandle
    mov eax,ebp
    jmp .wb_out
.wb_fail: xor eax,eax
.wb_out: END 32

