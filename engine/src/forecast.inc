# FJP1 append-only full-distribution forecasts. Local integrity only: a trusted
# copy is needed to detect wholesale rewriting; clock is the local Windows clock.
.equ FORECAST_BODY,144+JOINT_BYTES
.equ FORECAST_RECORD,FORECAST_BODY+32
.section .bss
forecast_record: .skip FORECAST_RECORD
forecast_prev: .skip 32
forecast_digest: .skip 32
forecast_backup: .skip JOINT_BYTES
forecast_backup_logz: .skip 8
forecast_time: .skip 8
.section .text
# rcx path, edx append (0 = audit existing, 1 = audit and append next forecast).
# Return 1 appended/audited, 2 already pending, 0 failed; restores joint model.
FN ForecastLedger,64
    mov r12,rcx
    mov r13d,edx
    mov dword ptr [rip+forecast_valid],0
    mov dword ptr [rip+forecast_evaluated],0
    mov dword ptr [rip+forecast_pending],0
    mov qword ptr [rip+forecast_log_e],0
    mov qword ptr [rip+forecast_max_log_e],0
    cmp dword ptr [rip+history_count],0
    je .fl_fail_early
    test r13d,r13d
    jz 1f
    cmp dword ptr [rip+research_ready],1
    jne .fl_fail_early
1:  lea rcx,[rip+forecast_backup]
    lea rdx,[rip+joint_coef]
    mov r8d,JOINT_BYTES
    call Copy
    mov rax,[rip+joint_logz]
    mov [rip+forecast_backup_logz],rax
    lea rcx,[rip+forecast_prev]
    mov edx,32
    call Zero
    lea rcx,[rip+forecast_time]
    call GetSystemTimeAsFileTime
    mov rcx,r12
    mov edx,0x80000000
    mov eax,3
    test r13d,r13d
    jz 2f
    mov edx,0xc0000000
    mov eax,4
2:  mov qword ptr [rsp+32],rax
    mov qword ptr [rsp+40],0x80
    mov qword ptr [rsp+48],0
    mov r8d,1
    xor r9d,r9d
    call CreateFileW
    cmp rax,-1
    je .fl_fail_restore
    mov rbx,rax
    mov rcx,rax
    lea rdx,[rip+io_size]
    call GetFileSizeEx
    test eax,eax
    jz .fl_close_fail
    mov rax,[rip+io_size]
    xor edx,edx
    mov ecx,FORECAST_RECORD
    div rcx
    test rdx,rdx
    jnz .fl_close_fail
    cmp rax,MAX_DRAWS
    ja .fl_close_fail
    mov r15,rax
    mov qword ptr [rsp+56],0
    xor r14d,r14d
.fl_read:
    cmp r14,r15
    jae .fl_verified
    mov rcx,rbx
    lea rdx,[rip+forecast_record]
    mov r8d,FORECAST_RECORD
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call ReadFile
    test eax,eax
    jz .fl_close_fail
    cmp dword ptr [rip+io_bytes],FORECAST_RECORD
    jne .fl_close_fail
    lea rdi,[rip+forecast_record]
    cmp dword ptr [rdi],0x31504a46
    jne .fl_close_fail
    cmp dword ptr [rdi+4],1
    jne .fl_close_fail
    mov eax,[rdi+8]
    cmp eax,[rsp+56]
    jbe .fl_close_fail
    mov [rsp+56],eax
    lea rsi,[rip+forecast_prev]
    xor ecx,ecx
1:  mov rax,[rsi+rcx*8]
    cmp rax,[rdi+96+rcx*8]
    jne .fl_close_fail
    inc ecx
    cmp ecx,4
    jb 1b
    mov rcx,rdi
    mov edx,FORECAST_BODY
    lea r8,[rip+forecast_digest]
    call Sha256
    lea rsi,[rip+forecast_digest]
    xor ecx,ecx
1:  mov rax,[rsi+rcx*8]
    cmp rax,[rdi+FORECAST_BODY+rcx*8]
    jne .fl_close_fail
    inc ecx
    cmp ecx,4
    jb 1b
    lea rcx,[rip+forecast_prev]
    mov rdx,rsi
    mov r8d,32
    call Copy
    mov ebp,[rdi+12]
    cmp ebp,60
    jb .fl_close_fail
    cmp ebp,[rip+history_count]
    ja .fl_close_fail
    lea rcx,[rip+history]
    imul edx,ebp,RECORD_SIZE
    lea r8,[rip+forecast_digest]
    call Sha256
    lea rsi,[rip+forecast_digest]
    xor ecx,ecx
1:  mov rax,[rsi+rcx*8]
    cmp rax,[rdi+32+rcx*8]
    jne .fl_close_fail
    inc ecx
    cmp ecx,4
    jb 1b
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov eax,[rdx+rax-RECORD_SIZE]
    inc eax
    cmp eax,[rdi+8]
    jne .fl_close_fail
    # Recorded forecast must precede that round's official scheduled deadline.
    dec eax
    mov rdx,[rip+week_ticks]
    mul rdx
    add rax,[rip+first_deadline]
    cmp [rdi+16],rax
    jae .fl_close_fail
    cmp ebp,[rip+history_count]
    je .fl_pending
    # Reject nonfinite/out-of-contract coefficients even in a rehashed file.
    xor ecx,ecx
1:  movsd xmm0,[rdi+144+rcx*8]
    andpd xmm0,[rip+abs_bits]
    ucomisd xmm0,xmm0
    jp .fl_close_fail
    movsd xmm1,[rip+joint_caps]
    cmp ecx,45
    jb 2f
    movsd xmm1,[rip+joint_caps+8]
    cmp ecx,2070
    jb 2f
    movsd xmm1,[rip+joint_caps+16]
2:  comisd xmm0,xmm1
    ja .fl_close_fail
    inc ecx
    cmp ecx,JOINT_FEATURES
    jb 1b
    movsd xmm0,[rdi+24]
    ucomisd xmm0,xmm0
    jp .fl_close_fail
    comisd xmm0,[rip+zero]
    jbe .fl_close_fail
    # Conservative bound: log Z <= log(C(45,6)) + 0.8 < 17.
    mov eax,17
    cvtsi2sd xmm1,eax
    comisd xmm0,xmm1
    ja .fl_close_fail
    # Load archived coefficients only after structure/hash/prefix validation.
    lea rcx,[rip+joint_coef]
    lea rdx,[rdi+144]
    mov r8d,JOINT_BYTES
    call Copy
    mov rax,[rdi+24]
    mov [rip+joint_logz],rax
    mov qword ptr [rip+joint_space],TOTAL_COMBINATIONS
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov eax,[rdx+rax]
    cmp eax,[rdi+8]
    jne .fl_close_fail
    imul eax,ebp,RECORD_SIZE
    mov rcx,[rdx+rax+40]
    call JointForecastLog
    movsd [rsp+64],xmm0
    mov eax,TOTAL_COMBINATIONS
    cvtsi2sd xmm0,eax
    call Log
    addsd xmm0,[rsp+64]
    addsd xmm0,[rip+forecast_log_e]
    ucomisd xmm0,xmm0
    jp .fl_close_fail
    movsd [rip+forecast_log_e],xmm0
    maxsd xmm0,[rip+forecast_max_log_e]
    movsd [rip+forecast_max_log_e],xmm0
    inc dword ptr [rip+forecast_evaluated]
    jmp .fl_next
.fl_pending:
    inc dword ptr [rip+forecast_pending]
.fl_next:
    inc r14
    jmp .fl_read
.fl_verified:
    test r13d,r13d
    jz .fl_audit_ok
    cmp dword ptr [rip+forecast_pending],0
    jne .fl_reused
    cmp r15,MAX_DRAWS
    jae .fl_close_fail
    mov eax,[rip+last_round]
    mov rdx,[rip+week_ticks]
    mul rdx
    add rax,[rip+first_deadline]
    cmp rax,[rip+forecast_time]
    jbe .fl_close_fail
    lea rcx,[rip+forecast_record]
    mov edx,FORECAST_RECORD
    call Zero
    lea rdi,[rip+forecast_record]
    mov dword ptr [rdi],0x31504a46
    mov dword ptr [rdi+4],1
    mov eax,[rip+last_round]
    inc eax
    mov [rdi+8],eax
    mov eax,[rip+history_count]
    mov [rdi+12],eax
    mov rax,[rip+forecast_time]
    mov [rdi+16],rax
    mov rax,[rip+forecast_backup_logz]
    mov [rdi+24],rax
    lea rcx,[rdi+32]
    lea rdx,[rip+hash_dataset]
    mov r8d,32
    call Copy
    lea rcx,[rdi+64]
    lea rdx,[rip+hash_config]
    mov r8d,32
    call Copy
    lea rcx,[rdi+96]
    lea rdx,[rip+forecast_prev]
    mov r8d,32
    call Copy
    mov rax,[rip+forecast_log_e]
    mov [rdi+128],rax
    mov eax,[rip+forecast_evaluated]
    mov [rdi+136],eax
    lea rcx,[rdi+144]
    lea rdx,[rip+forecast_backup]
    mov r8d,JOINT_BYTES
    call Copy
    mov rcx,rdi
    mov edx,FORECAST_BODY
    lea r8,[rdi+FORECAST_BODY]
    call Sha256
    mov rcx,rbx
    mov rdx,rdi
    mov r8d,FORECAST_RECORD
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call WriteFile
    test eax,eax
    jz .fl_close_fail
    cmp dword ptr [rip+io_bytes],FORECAST_RECORD
    jne .fl_close_fail
    mov rcx,rbx
    call FlushFileBuffers
    test eax,eax
    jz .fl_close_fail
    mov dword ptr [rip+forecast_pending],1
.fl_audit_ok: mov ebp,1; jmp .fl_close
.fl_reused: mov ebp,2; jmp .fl_close
.fl_close_fail: xor ebp,ebp
.fl_close:
    mov rcx,rbx
    call CloseHandle
    test ebp,ebp
    jz .fl_restore
    mov dword ptr [rip+forecast_valid],1
    jmp .fl_restore
.fl_fail_restore: xor ebp,ebp
.fl_restore:
    lea rcx,[rip+joint_coef]
    lea rdx,[rip+forecast_backup]
    mov r8d,JOINT_BYTES
    call Copy
    mov rax,[rip+forecast_backup_logz]
    mov [rip+joint_logz],rax
    mov eax,ebp
    jmp .fl_out
.fl_fail_early: xor eax,eax
.fl_out:
    test eax,eax
    jnz 1f
    mov dword ptr [rip+forecast_evaluated],0
    mov dword ptr [rip+forecast_pending],0
    mov qword ptr [rip+forecast_log_e],0
    mov qword ptr [rip+forecast_max_log_e],0
1:  END 64
