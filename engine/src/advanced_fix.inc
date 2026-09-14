# Runtime-corrected AEL scoring/search path. Keeps the first AEL module focused
# on estimators and diagnostics while making the hot-path combination explicit.
.section .text

# Preserve validated base energy across the fourth-order scorer call.
FN AdvancedCandidateEnergyV2,32
    mov rbx,rcx
    call ComputeNormalized
    lea rdx,[rip+norm_z]
    lea r8,[rip+model_weights]
    xorpd xmm5,xmm5
    xorpd xmm4,xmm4
    xor esi,esi
1:  movsd xmm0,[r8+rsi*8]
    addsd xmm4,xmm0
    mulsd xmm0,[rdx+rsi*8]
    addsd xmm5,xmm0
    inc esi
    cmp esi,MODEL_COUNT-1
    jb 1b
    comisd xmm4,[rip+epsilon]
    ja 2f
    xorpd xmm5,xmm5
    xor esi,esi
3:  addsd xmm5,[rdx+rsi*8]
    inc esi
    cmp esi,MODEL_COUNT-1
    jb 3b
    mov eax,MODEL_COUNT-1
    cvtsi2sd xmm0,eax
    divsd xmm5,xmm0
    mulsd xmm5,[rip+adv_quarter]
2:  movsd [rsp+32],xmm5
    mov rcx,rbx
    call AdvancedQuadScore
    mulsd xmm0,[rip+advanced_quad_weight]
    addsd xmm0,[rsp+32]
    END 32

# Doubling the original 4,096 fast stage to MAX_FAST (8,192 in ABI v1), then
# rescoring into a dedicated 2,048 advanced stage before the final 512 field.
FN AdvancedExpandCandidatesV2,16
    mov eax,[rip+scan_capacity]
    mov [rsp+32],eax
    mov dword ptr [rip+scan_capacity],MAX_FAST
    call ScanField
    test eax,eax
    jz .aec2_fail
    mov dword ptr [rip+advanced_stage_count],0
    xor esi,esi
.aec2_stage:
    cmp esi,[rip+fast_count]
    jae .aec2_final
    cmp dword ptr [rip+cancel_flag],0
    jne .aec2_fail
    lea rax,[rip+fast_masks]
    mov rbx,[rax+rsi*8]
    mov rcx,rbx
    call AdvancedCandidateEnergyV2
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rdx,xmm0
    mov rcx,rbx
    lea r8,[rip+advanced_stage_masks]
    lea r9,[rip+advanced_stage_scores]
    lea r10,[rip+advanced_stage_count]
    mov r11d,ADV_STAGE_COUNT
    call HeapInsert
    inc esi
    jmp .aec2_stage
.aec2_final:
    mov dword ptr [rip+final_count],0
    xor esi,esi
1:  cmp esi,[rip+advanced_stage_count]
    jae 2f
    lea rax,[rip+advanced_stage_masks]
    mov rcx,[rax+rsi*8]
    lea rax,[rip+advanced_stage_scores]
    mov rdx,[rax+rsi*8]
    call FinalHeapInsert
    inc esi
    jmp 1b
2:  mov eax,[rip+final_count]
    mov [rip+field_count],eax
    mov eax,1
    jmp .aec2_restore
.aec2_fail: xor eax,eax
.aec2_restore:
    mov edx,[rsp+32]
    mov [rip+scan_capacity],edx
    END 16

# Full two-number replacement neighborhood for the strongest candidates.
# Normal mode checks 8 seeds; deep mode checks 32 seeds.
FN AdvancedTwoSwapRefineV2,64
    mov r12d,8
    cmp dword ptr [rip+advanced_mode],0
    je 1f
    mov r12d,32
1:  xor esi,esi
.ats2_select:
    cmp esi,r12d
    jae .ats2_refine
    mov r13,0x8000000000000000
    mov r14d,-1
    xor edi,edi
.ats2_scan:
    cmp edi,[rip+field_count]
    jae .ats2_store
    xor ebp,ebp
2:  cmp ebp,esi
    jae 3f
    lea rax,[rip+advanced_elite_indices]
    cmp edi,[rax+rbp*4]
    je .ats2_next
    inc ebp
    jmp 2b
3:  lea rax,[rip+final_scores]
    mov rbx,[rax+rdi*8]
    cmp rbx,r13
    jle .ats2_next
    mov r13,rbx
    mov r14d,edi
.ats2_next:
    inc edi
    jmp .ats2_scan
.ats2_store:
    cmp r14d,0
    jl .ats2_done
    lea rax,[rip+advanced_elite_indices]
    mov [rax+rsi*4],r14d
    inc esi
    jmp .ats2_select
.ats2_refine:
    xor esi,esi
.ats2_each:
    cmp esi,r12d
    jae .ats2_done
    cmp dword ptr [rip+cancel_flag],0
    jne .ats2_fail
    lea rax,[rip+advanced_elite_indices]
    mov r15d,[rax+rsi*4]
    lea rax,[rip+final_masks]
    mov rbx,[rax+r15*8]
    mov rcx,rbx
    lea rdx,[rip+advanced_nums]
    call MaskToNumbers
    test eax,eax
    jz .ats2_fail
    xor edi,edi
    xor ebp,ebp
.ats2_outside:
    cmp edi,45
    jae .ats2_base
    bt rbx,rdi
    jc 4f
    lea rax,[rip+advanced_outside]
    mov [rax+rbp*4],edi
    inc ebp
4:  inc edi
    jmp .ats2_outside
.ats2_base:
    mov rcx,rbx
    call AdvancedCandidateEnergyV2
    movsd [rsp+32],xmm0
    mov [rsp+40],rbx
    xor edi,edi
.ats2_i:
    lea ebp,[rdi+1]
.ats2_j:
    xor r13d,r13d
.ats2_u:
    lea r14d,[r13+1]
.ats2_v:
    mov rax,rbx
    lea rdx,[rip+advanced_nums]
    mov ecx,[rdx+rdi*4]
    dec ecx
    btr rax,rcx
    mov ecx,[rdx+rbp*4]
    dec ecx
    btr rax,rcx
    lea rdx,[rip+advanced_outside]
    mov ecx,[rdx+r13*4]
    bts rax,rcx
    mov ecx,[rdx+r14*4]
    bts rax,rcx
    mov [rsp+48],rax
    mov rcx,rax
    call AdvancedCandidateEnergyV2
    comisd xmm0,[rsp+32]
    jbe 5f
    mov rcx,[rsp+48]
    mov edx,r15d
    call AdvancedFieldContains
    test eax,eax
    jnz 5f
    mov rcx,[rsp+48]
    call AdvancedCandidateEnergyV2
    movsd [rsp+32],xmm0
    mov rax,[rsp+48]
    mov [rsp+40],rax
5:  inc r14d
    cmp r14d,39
    jb .ats2_v
    inc r13d
    cmp r13d,38
    jb .ats2_u
    inc ebp
    cmp ebp,6
    jb .ats2_j
    inc edi
    cmp edi,5
    jb .ats2_i
    lea rax,[rip+final_masks]
    mov rdx,[rsp+40]
    mov [rax+r15*8],rdx
    movsd xmm0,[rsp+32]
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rdx,xmm0
    lea rax,[rip+final_scores]
    mov [rax+r15*8],rdx
    inc esi
    jmp .ats2_each
.ats2_done: mov eax,1; jmp .ats2_out
.ats2_fail: xor eax,eax
.ats2_out: END 64

# Final integration used by the app-facing advanced API. Existing validation is
# never weakened: AEL can only shrink evidence or add a separately validated
# bounded fourth-order term.
FN AdvancedFinalizeCommonV2
    mov [rip+advanced_mode],ecx
    mov dword ptr [rip+advanced_ready],0
    cmp dword ptr [rip+field_ready],1
    jne .af2_fail
    cmp dword ptr [rip+model_ready],1
    jne .af2_fail
    mov dword ptr [rip+analysis_stage],11
    call AdvancedValidateQuad
    test eax,eax
    jz .af2_fail
    call AdvancedTemporalStability
    test eax,eax
    jz .af2_fail
    call AdvancedSyntheticNull
    test eax,eax
    jz .af2_fail
    call AdvancedSetQuadWeight
    call AdvancedShrinkWeights
    mov dword ptr [rip+analysis_stage],12
    call AdvancedExpandCandidatesV2
    test eax,eax
    jz .af2_fail
    mov dword ptr [rip+analysis_stage],13
    call AdvancedTwoSwapRefineV2
    test eax,eax
    jz .af2_fail
    call BuildGeometry
    test eax,eax
    jz .af2_fail
    call BuildWorlds
    test eax,eax
    jz .af2_fail
    call BuildThermal
    call AdvancedAdjustField
    test eax,eax
    jz .af2_fail
    mov dword ptr [rip+advanced_ready],1
    mov dword ptr [rip+analysis_stage],7
    mov dword ptr [rip+progress],100
    mov dword ptr [rip+rng_mode],0
    mov eax,1
    jmp .af2_out
.af2_fail:
    mov dword ptr [rip+advanced_ready],0
    mov dword ptr [rip+field_ready],0
    xor eax,eax
.af2_out: END

FN AdvancedFinalizeNormalV2
    xor ecx,ecx
    call AdvancedFinalizeCommonV2
    END
FN AdvancedFinalizeDeepV2
    mov ecx,1
    call AdvancedFinalizeCommonV2
    END

# Deep sampling keeps the exact joint model as proposal distribution and only
# applies the AEL rejection term when the new evidence layer itself survived.
FN AdvancedJointSampleV2,32
    cmp dword ptr [rip+advanced_ready],1
    jne .ajs2_base
    movsd xmm0,[rip+advanced_global_support]
    comisd xmm0,[rip+epsilon]
    jbe .ajs2_base
    mov ebp,32
    xor r12,r12
.ajs2_try:
    call JointSample
    test rax,rax
    jz .ajs2_fail
    mov r12,rax
    mov rcx,rax
    call AdvancedCandidateEnergyV2
    maxsd xmm0,[rip+adv_neg_four]
    minsd xmm0,[rip+adv_four]
    mulsd xmm0,[rip+adv_joint_accept_weight]
    subsd xmm0,[rip+adv_joint_accept_cap]
    call Exp
    movsd [rsp+32],xmm0
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .ajs2_fail
    comisd xmm0,[rsp+32]
    jbe .ajs2_accept
    dec ebp
    jnz .ajs2_try
.ajs2_accept:
    mov rax,r12
    jmp .ajs2_out
.ajs2_base:
    call JointSample
    jmp .ajs2_out
.ajs2_fail: xor eax,eax
.ajs2_out: END 32
