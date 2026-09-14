# New ABI-v1 functions; integrated analysis is the product default.
.section .text
.globl FortunaOracleGenerateIntegrated
FN FortunaOracleGenerateIntegrated
    cmp ecx,1
    jb .igi_arg
    cmp ecx,10
    ja .igi_arg
    test rdx,rdx
    jz .igi_arg
    cmp r8d,ecx
    jb .igi_arg
    mov ebp,ecx
    mov rbx,rdx
    call ApiLock
    test eax,eax
    jnz .igi_out
    cmp dword ptr [rip+ti_ready],1
    jne .igi_state
    cmp dword ptr [rip+field_ready],1
    jne .igi_state
    mov dword ptr [rip+rng_mode],0
    mov dword ptr [rip+rng_error],0
    mov dword ptr [rip+session_count],0
    xor esi,esi
.igi_sample:
    call IntegratedSample
    test rax,rax
    jz .igi_random
    lea rdx,[rip+session_masks]
    mov [rdx+rsi*8],rax
    inc esi
    cmp esi,ebp
    jb .igi_sample
    mov [rip+session_count],ebp
    mov rcx,[rip+session_masks]
    call SelectMask
    mov rcx,rbx
    lea rdx,[rip+session_masks]
    lea r8d,[rbp*8]
    call Copy
    xor ecx,ecx
    jmp .igi_finish
.igi_random: mov ecx,-5; jmp .igi_finish
.igi_state: mov ecx,-4
.igi_finish: call ApiFinish; jmp .igi_out
.igi_arg: mov eax,-1
.igi_out: END
.globl FortunaOracleGetIntegratedSnapshot
# 64 bytes: size,ready,profile,completed,folds,null_refits,grid,version;
# doubles null_p,heldout_log_gain,mixture_weight,AIS_relative_error.
FN FortunaOracleGetIntegratedSnapshot
    test rcx,rcx
    jz .igs_arg
    cmp edx,64
    jb .igs_arg
    mov rbx,rcx
    call ApiLock
    test eax,eax
    jnz .igs_out
    mov dword ptr [rbx],64
    mov eax,[rip+ti_ready]
    mov [rbx+4],eax
    mov eax,[rip+ti_profile]
    mov [rbx+8],eax
    mov eax,[rip+ti_completed]
    mov [rbx+12],eax
    mov eax,[rip+ti_folds]
    mov [rbx+16],eax
    mov eax,[rip+ti_null_done]
    mov [rbx+20],eax
    mov eax,[rip+ti_grid_count]
    mov [rbx+24],eax
    mov dword ptr [rbx+28],0x10000
    mov rax,[rip+ti_null_p]
    mov [rbx+32],rax
    mov rax,[rip+ti_gain]
    mov [rbx+40],rax
    mov rax,[rip+ti_alpha]
    mov [rbx+48],rax
    mov rax,[rip+ti_ais_error]
    mov [rbx+56],rax
    xor ecx,ecx
    call ApiFinish
    jmp .igs_out
.igs_arg: mov eax,-1
.igs_out: END

.globl FortunaOracleGetIntegratedProbability
FN FortunaOracleGetIntegratedProbability
    test rdx,rdx
    jz .igp_arg
    mov rbx,rcx
    mov rsi,rdx
    mov rax,rcx
    shr rax,45
    jnz .igp_arg
    call PopCount64
    cmp eax,6
    jne .igp_arg
    call ApiLock
    test eax,eax
    jnz .igp_out
    cmp dword ptr [rip+ti_ready],1
    jne .igp_state
    cmp dword ptr [rip+field_ready],1
    jne .igp_state
    mov rcx,rbx
    call IntegratedProbability
    movsd [rsi],xmm0
    xor ecx,ecx
    jmp .igp_finish
.igp_state: mov ecx,-4
.igp_finish: call ApiFinish; jmp .igp_out
.igp_arg: mov eax,-1
.igp_out: END
