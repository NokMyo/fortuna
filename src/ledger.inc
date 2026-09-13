.section .rdata,"dr"
first_deadline: .quad 126837351000000000
week_ticks: .quad 6048000000000
.section .bss
seal_record: .skip 256
seal_read: .skip 256
seal_prev: .skip 32
seal_hash: .skip 32
seal_seed_data: .skip 80
seal_seed_hash: .skip 32
seal_time: .skip 8
seal_latest: .skip 4
seal_existing: .skip 4
seal_existing_mask: .skip 8
seal_existing_index: .skip 4
.section .text
# SealPick -> 1 new, 2 already exists (reuses saved pick), 0 failure.
FN SealPick,64
    cmp dword ptr [rip+field_ready],1
    jne .seal_fail
    cmp word ptr [rip+ledger_path],0
    je .seal_fail
    lea rcx,[rip+seal_time]
    call GetSystemTimeAsFileTime
    lea rcx,[rip+seal_prev]
    mov edx,32
    call Zero
    mov dword ptr [rip+seal_latest],0
    mov dword ptr [rip+seal_existing],0
    lea rcx,[rip+ledger_path]
    mov edx,0xc0000000
    mov r8d,1
    xor r9d,r9d
    mov qword ptr [rsp+32],4
    mov qword ptr [rsp+40],0x80
    mov qword ptr [rsp+48],0
    call CreateFileW
    cmp rax,-1
    je .seal_fail
    mov rbx,rax
    mov rcx,rax
    lea rdx,[rip+io_size]
    call GetFileSizeEx
    test eax,eax
    jz .seal_close_fail
    cmp qword ptr [rip+io_size],MAX_DRAWS*256
    ja .seal_close_fail
    test qword ptr [rip+io_size],255
    jnz .seal_close_fail
    mov r15,[rip+io_size]
    xor r14d,r14d
.seal_verify:
    cmp r14,r15
    jae .seal_verified
    mov rcx,rbx
    lea rdx,[rip+seal_read]
    mov r8d,256
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call ReadFile
    test eax,eax
    jz .seal_close_fail
    cmp dword ptr [rip+io_bytes],256
    jne .seal_close_fail
    lea rdi,[rip+seal_read]
    cmp dword ptr [rdi],0x31474c46
    jne .seal_close_fail
    cmp dword ptr [rdi+4],1
    jne .seal_close_fail
    mov eax,[rdi+8]
    cmp eax,[rip+seal_latest]
    jbe .seal_close_fail
    mov [rip+seal_latest],eax
    lea rsi,[rip+seal_prev]
    xor ecx,ecx
1:  mov rax,[rsi+rcx*8]
    cmp rax,[rdi+rcx*8+192]
    jne .seal_close_fail
    inc ecx
    cmp ecx,4
    jb 1b
    mov rcx,rdi
    mov edx,224
    lea r8,[rip+seal_hash]
    call Sha256
    lea rsi,[rip+seal_hash]
    xor ecx,ecx
1:  mov rax,[rsi+rcx*8]
    cmp rax,[rdi+rcx*8+224]
    jne .seal_close_fail
    inc ecx
    cmp ecx,4
    jb 1b
    lea rcx,[rip+seal_prev]
    lea rdx,[rdi+224]
    mov r8d,32
    call Copy
    mov ecx,[rip+last_round]
    inc ecx
    cmp ecx,[rdi+8]
    jne 2f
    mov dword ptr [rip+seal_existing],1
    mov rax,[rdi+128]
    mov [rip+seal_existing_mask],rax
    mov eax,[rdi+136]
    mov [rip+seal_existing_index],eax
2:  add r14,256
    jmp .seal_verify
.seal_verified:
    cmp dword ptr [rip+seal_existing],1
    je .seal_reuse
    mov eax,[rip+last_round]
    mov rdx,[rip+week_ticks]
    mul rdx
    add rax,[rip+first_deadline]
    cmp rax,[rip+seal_time]
    jbe .seal_close_fail
    mov eax,[rip+last_round]
    inc eax
    cmp eax,[rip+seal_latest]
    jbe .seal_close_fail
    lea rcx,[rip+seal_record]
    mov edx,256
    call Zero
    lea rdi,[rip+seal_record]
    mov dword ptr [rdi],0x31474c46
    mov dword ptr [rdi+4],1
    mov eax,[rip+last_round]
    inc eax
    mov [rdi+8],eax
    mov dword ptr [rdi+12],0x10000
    mov rax,[rip+seal_time]
    mov [rdi+16],rax
    lea rcx,[rdi+24]
    lea rdx,[rip+hash_dataset]
    mov r8d,32
    call Copy
    lea rcx,[rdi+56]
    lea rdx,[rip+hash_config]
    mov r8d,32
    call Copy
    lea rcx,[rdi+88]
    lea rdx,[rip+hash_field]
    mov r8d,32
    call Copy
    lea rcx,[rdi+192]
    lea rdx,[rip+seal_prev]
    mov r8d,32
    call Copy
    lea rsi,[rip+seal_seed_data]
    mov rax,0x31616e7574726f46
    mov [rsi],rax
    mov rax,[rdi+8]
    mov [rsi+8],rax
    lea rcx,[rsi+16]
    lea rdx,[rdi+24]
    mov r8d,64
    call Copy
    mov rcx,rsi
    mov edx,80
    lea r8,[rip+seal_seed_hash]
    call Sha256
    mov rax,[rip+seal_seed_hash]
    mov [rdi+120],rax
    mov rsi,[rip+rng_state]
    mov ebp,[rip+rng_mode]
    mov [rip+rng_state],rax
    mov dword ptr [rip+rng_mode],1
    mov dword ptr [rip+rng_error],0
    call SampleMask
    mov [rip+rng_state],rsi
    mov [rip+rng_mode],ebp
    test rax,rax
    jz .seal_close_fail
    mov [rdi+128],rax
    mov rcx,rax
    call SelectMask
    mov eax,[rip+selected_index]
    mov [rdi+136],eax
    movsd xmm0,[rip+rho]
    mov eax,1000000
    cvtsi2sd xmm1,eax
    mulsd xmm0,xmm1
    cvtsd2si eax,xmm0
    mov [rdi+140],eax
    mov rcx,rdi
    mov edx,224
    lea r8,[rdi+224]
    call Sha256
    mov rcx,rbx
    mov rdx,rdi
    mov r8d,256
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call WriteFile
    test eax,eax
    jz .seal_close_fail
    cmp dword ptr [rip+io_bytes],256
    jne .seal_close_fail
    mov rcx,rbx
    call FlushFileBuffers
    test eax,eax
    jz .seal_close_fail
    mov ebp,1
    jmp .seal_close
.seal_reuse:
    mov rcx,[rip+seal_existing_mask]
    call SelectMask
    test eax,eax
    jz .seal_close_fail
    mov eax,[rip+seal_existing_index]
    mov [rip+selected_index],eax
    mov ebp,2
    jmp .seal_close
.seal_close_fail: xor ebp,ebp
.seal_close:
    mov rcx,rbx
    call CloseHandle
    mov eax,ebp
    jmp .seal_out
.seal_fail: xor eax,eax
.seal_out: END 64
