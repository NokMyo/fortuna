.section .rdata,"dr"
thermal_low: .double 0.02
thermal_high: .double 64.0
entropy_target: .double 0.90
geometry_weight: .double 0.05
crowd_weight: .double 0.10
.section .bss
field_energies: .skip TOP_FINAL*8
field_z: .skip TOP_FINAL*8
field_entropy: .skip 8
field_max: .skip 8
field_sum: .skip 8
field_center: .skip 8
field_scale: .skip 8
field_hash_data: .skip 8448
mixture_marginals: .skip 45*8
challenger_marginals: .skip 45*8
.section .text
FN BuildThermal,64
    xor esi,esi
.bt_energy:
    lea rax,[rip+final_masks]
    mov rcx,[rax+rsi*8]
    call ComputeNormalized
    lea rbx,[rip+norm_z]
    imul eax,esi,MODEL_COUNT*8
    lea rcx,[rip+final_raw]
    add rcx,rax
    mov rdx,rbx
    mov r8d,MODEL_COUNT*8
    call Copy
    xor edi,edi
    xorpd xmm0,xmm0
    xorpd xmm4,xmm4
    xorpd xmm5,xmm5
    lea rdx,[rip+model_weights]
1:  movsd xmm1,[rbx+rdi*8]
    addsd xmm4,xmm1
    addsd xmm5,[rdx+rdi*8]
    movsd xmm1,[rbx+rdi*8]
    mulsd xmm1,[rdx+rdi*8]
    addsd xmm0,xmm1
    inc edi
    cmp edi,MODEL_COUNT-1
    jb 1b
    # Diagnostic challenger energy stays inspectable when evidence gates close.
    comisd xmm5,[rip+zero]
    jne 2f
    movapd xmm0,xmm4
    mov eax,MODEL_COUNT-1
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
2:  mulsd xmm0,[rip+evidence]
    movsd xmm1,[rbx+88]
    mulsd xmm1,[rip+crowd_weight]
    addsd xmm0,xmm1
    # Geometry has no independent evidence; it scales with the same budget.
    imul eax,esi,32
    lea rdx,[rip+geometry]
    movsd xmm1,[rdx+rax]
    addsd xmm1,[rdx+rax+8]
    mulsd xmm1,[rip+geometry_weight]
    mulsd xmm1,[rip+evidence]
    addsd xmm0,xmm1
    lea rdx,[rip+world_summary]
    movsd xmm1,[rdx+rax]
    addsd xmm1,[rdx+rax+16]
    mulsd xmm1,[rip+half]
    mulsd xmm1,[rip+evidence]
    addsd xmm0,xmm1
    lea rdx,[rip+field_energies]
    movsd [rdx+rsi*8],xmm0
    inc esi
    cmp esi,[rip+field_count]
    jb .bt_energy
    lea rcx,[rip+field_energies]
    mov edx,[rip+field_count]
    call Median
    movsd [rip+field_center],xmm0
    lea rbx,[rip+field_energies]
    lea rdi,[rip+world_temp]
    xor esi,esi
1:  movsd xmm1,[rbx+rsi*8]
    subsd xmm1,xmm0
    andpd xmm1,[rip+abs_bits]
    movsd [rdi+rsi*8],xmm1
    inc esi
    cmp esi,[rip+field_count]
    jb 1b
    mov rcx,rdi
    mov edx,[rip+field_count]
    call Median
    mulsd xmm0,[rip+mad_factor]
    maxsd xmm0,[rip+epsilon]
    movsd [rip+field_scale],xmm0
    lea rdi,[rip+field_z]
    xor esi,esi
    movsd xmm5,[rip+neg_zlimit]
1:  movsd xmm1,[rbx+rsi*8]
    subsd xmm1,[rip+field_center]
    divsd xmm1,xmm0
    maxsd xmm1,[rip+neg_zlimit]
    minsd xmm1,[rip+zlimit]
    maxsd xmm5,xmm1
    movsd [rdi+rsi*8],xmm1
    inc esi
    cmp esi,[rip+field_count]
    jb 1b
    movsd [rip+field_max],xmm5
    cvtsi2sd xmm0,dword ptr [rip+field_count]
    call Log
    mulsd xmm0,[rip+entropy_target]
    movsd [rsp+32],xmm0
    movsd xmm0,[rip+thermal_low]
    movsd [rsp+40],xmm0
    movsd xmm0,[rip+thermal_high]
    movsd [rsp+48],xmm0
    mov ebp,24
.bt_search:
    movsd xmm0,[rsp+40]
    addsd xmm0,[rsp+48]
    mulsd xmm0,[rip+half]
    movsd [rip+temperature],xmm0
    call ThermalProbabilities
    movsd xmm0,[rip+field_entropy]
    comisd xmm0,[rsp+32]
    jb 1f
    movsd xmm0,[rip+temperature]
    movsd [rsp+48],xmm0
    jmp 2f
1:  movsd xmm0,[rip+temperature]
    movsd [rsp+40],xmm0
2:  dec ebp
    jnz .bt_search
    call BuildMarginals
    lea rcx,[rip+field_hash_data]
    mov edx,8448
    call Zero
    lea rcx,[rip+field_hash_data]
    lea rdx,[rip+final_masks]
    mov r8d,TOP_FINAL*8
    call Copy
    lea rcx,[rip+field_hash_data+4096]
    lea rdx,[rip+final_prob]
    mov r8d,TOP_FINAL*8
    call Copy
    lea rcx,[rip+field_hash_data+8192]
    lea rdx,[rip+hash_dataset]
    mov r8d,32
    call Copy
    lea rcx,[rip+field_hash_data+8224]
    lea rdx,[rip+hash_config]
    mov r8d,32
    call Copy
    mov rax,[rip+rho]
    mov [rip+field_hash_data+8256],rax
    mov rax,[rip+temperature]
    mov [rip+field_hash_data+8264],rax
    lea rcx,[rip+field_hash_data+8272]
    lea rdx,[rip+model_weights]
    mov r8d,MODEL_COUNT*8
    call Copy
    lea rcx,[rip+field_hash_data]
    mov edx,8448
    lea r8,[rip+hash_field]
    call Sha256
    mov dword ptr [rip+field_ready],1
    END 64

FN ThermalProbabilities,32
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],0
    xor esi,esi
    lea rbx,[rip+field_z]
    lea rdi,[rip+final_prob]
.tp_exp:
    movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rip+field_max]
    divsd xmm0,[rip+temperature]
    movsd [rsp+48],xmm0
    call Exp
    movsd [rdi+rsi*8],xmm0
    movapd xmm1,xmm0
    addsd xmm1,[rsp+32]
    movsd [rsp+32],xmm1
    mulsd xmm0,[rsp+48]
    addsd xmm0,[rsp+40]
    movsd [rsp+40],xmm0
    inc esi
    cmp esi,[rip+field_count]
    jb .tp_exp
    movsd xmm0,[rsp+32]
    movsd [rip+field_sum],xmm0
    call Log
    movsd xmm1,[rsp+40]
    divsd xmm1,[rsp+32]
    subsd xmm0,xmm1
    movsd [rip+field_entropy],xmm0
    xor esi,esi
    xorpd xmm5,xmm5
    lea rbx,[rip+final_cdf]
1:  movsd xmm0,[rdi+rsi*8]
    divsd xmm0,[rsp+32]
    movsd [rdi+rsi*8],xmm0
    addsd xmm5,xmm0
    movsd [rbx+rsi*8],xmm5
    inc esi
    cmp esi,[rip+field_count]
    jb 1b
    dec esi
    movsd xmm0,[rip+one]
    movsd [rbx+rsi*8],xmm0
    END 32

FN BuildMarginals
    lea rcx,[rip+persistence]
    mov edx,45*8
    call Zero
    lea rcx,[rip+challenger_marginals]
    mov edx,45*8
    call Zero
    xor esi,esi
    lea rbx,[rip+final_masks]
    lea rdi,[rip+final_prob]
    lea r12,[rip+persistence]
    lea r13,[rip+challenger_marginals]
1:  mov rax,[rbx+rsi*8]
    movsd xmm0,[rdi+rsi*8]
2:  bsf rcx,rax
    btr rax,rcx
    movsd xmm1,[r12+rcx*8]
    addsd xmm1,[rip+one]
    movsd [r12+rcx*8],xmm1
    movsd xmm1,[r13+rcx*8]
    addsd xmm1,xmm0
    movsd [r13+rcx*8],xmm1
    test rax,rax
    jnz 2b
    inc esi
    cmp esi,[rip+field_count]
    jb 1b
    xor esi,esi
    lea r14,[rip+mixture_marginals]
    movsd xmm5,[rip+one]
    subsd xmm5,[rip+rho]
1:  movsd xmm0,[r12+rsi*8]
    cvtsi2sd xmm1,dword ptr [rip+field_count]
    divsd xmm0,xmm1
    movsd [r12+rsi*8],xmm0
    movsd xmm0,[r13+rsi*8]
    mulsd xmm0,xmm5
    movsd xmm1,[rip+rho]
    mulsd xmm1,[rip+p_one]
    addsd xmm0,xmm1
    movsd [r14+rsi*8],xmm0
    inc esi
    cmp esi,45
    jb 1b
    END

# Session returns zero on OS RNG failure. It never silently selects a fixed mask.
FN SampleMask
    cmp dword ptr [rip+field_ready],1
    jne .sm_uniform
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .sm_fail
    comisd xmm0,[rip+rho]
    jb .sm_uniform
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .sm_fail
    lea rdx,[rip+final_cdf]
    xor ecx,ecx
1:  comisd xmm0,[rdx+rcx*8]
    jb 2f
    inc ecx
    cmp ecx,[rip+field_count]
    jb 1b
    dec ecx
2:  lea rax,[rip+final_masks]
    mov rax,[rax+rcx*8]
    jmp .sm_out
.sm_uniform:
    call GenerateUniformMask
    jmp .sm_out
.sm_fail: xor eax,eax
.sm_out: END

FN SelectMask
    mov [rip+selected_mask],rcx
    lea rdx,[rip+selected_nums]
    call MaskToNumbers
    test eax,eax
    jz .sel_out
    mov dword ptr [rip+selected_index],0
    mov qword ptr [rip+selected_score],0
    cmp dword ptr [rip+model_ready],1
    jne .sel_done
    mov rcx,[rip+selected_mask]
    call ScoreDeep
    mov [rip+selected_score],rax
    # Honest percentile against the fixed 512 uniform reference combinations,
    # not a fabricated score-to-probability conversion.
    mov rbx,rax
    xor esi,esi
    xor edi,edi
1:  lea rax,[rip+null_masks]
    mov rcx,[rax+rsi*8]
    call ScoreDeep
    cmp rax,rbx
    jg 2f
    inc edi
2:  inc esi
    cmp esi,NULL_COUNT
    jb 1b
    imul eax,edi,1000
    shr eax,9
    mov [rip+selected_index],eax
.sel_done: mov eax,1
.sel_out: END
FN SampleCandidate
    mov dword ptr [rip+rng_error],0
    call SampleMask
    test rax,rax
    jz 1f
    mov rcx,rax
    call SelectMask
1:  END

# FCA_1 policy-sampled greedy overlap objective for small ticket bundles:
# draw 64 policy candidates for each new ticket; maximize -sum overlap^2.
# Exact duplicates forbidden; sequential diversification changes the joint law.
FN GenerateTickets,32
    mov ebp,ecx
    cmp ebp,1
    jb .gt_fail
    cmp ebp,10
    ja .gt_fail
    mov dword ptr [rip+session_count],0
    mov dword ptr [rip+rng_error],0
    xor esi,esi
.gt_ticket:
    mov r15,0x7fffffffffffffff
    xor r14d,r14d
    mov r12d,64
.gt_trial:
    call SampleMask
    test rax,rax
    jz .gt_fail
    mov rbx,rax
    xor edi,edi
    xor r13d,r13d
1:  cmp edi,esi
    jae .gt_compare
    lea rax,[rip+session_masks]
    mov rcx,[rax+rdi*8]
    cmp rcx,rbx
    je .gt_nexttrial
    and rcx,rbx
    call PopCount64
    imul eax,eax
    add r13d,eax
    inc edi
    jmp 1b
.gt_compare:
    cmp r13,r15
    jae .gt_nexttrial
    mov r15,r13
    mov r14,rbx
.gt_nexttrial:
    dec r12d
    jnz .gt_trial
    test r14,r14
    jz .gt_fail
    lea rax,[rip+session_masks]
    mov [rax+rsi*8],r14
    inc esi
    mov [rip+session_count],esi
    cmp esi,ebp
    jb .gt_ticket
    mov rcx,[rip+session_masks]
    call SelectMask
    mov eax,1
    jmp .gt_out
.gt_fail:
    mov dword ptr [rip+session_count],0
    xor eax,eax
.gt_out: END 32
