.section .bss
norm_temp: .skip NULL_COUNT*8
model_null_mean: .skip MODEL_COUNT*8
model_null_sd: .skip MODEL_COUNT*8
norm_z: .skip MODEL_COUNT*8
.section .text
FN BuildNormalization
    mov rbx,[rip+rng_state]
    mov ebp,[rip+rng_mode]
    mov dword ptr [rip+rng_mode],1
    mov qword ptr [rip+rng_state],20260913
    xor esi,esi
.bn_samples:
    call GenerateUniformMask
    lea rdx,[rip+null_masks]
    mov [rdx+rsi*8],rax
    mov rcx,rax
    call ComputeModels
    imul eax,esi,MODEL_COUNT*8
    lea rcx,[rip+null_models]
    add rcx,rax
    lea rdx,[rip+raw_models]
    mov r8d,MODEL_COUNT*8
    call Copy
    inc esi
    cmp esi,NULL_COUNT
    jb .bn_samples
    mov [rip+rng_state],rbx
    mov [rip+rng_mode],ebp
    xor esi,esi
.bn_columns:
    xor edi,edi
    lea r12,[rip+norm_temp]
    lea r13,[rip+null_models]
1:  imul eax,edi,MODEL_COUNT
    add eax,esi
    movsd xmm0,[r13+rax*8]
    movsd [r12+rdi*8],xmm0
    inc edi
    cmp edi,NULL_COUNT
    jb 1b
    mov rcx,r12
    mov edx,NULL_COUNT
    call Median
    lea rax,[rip+norm_center]
    movsd [rax+rsi*8],xmm0
    xor edi,edi
1:  movsd xmm1,[r12+rdi*8]
    subsd xmm1,xmm0
    andpd xmm1,[rip+abs_bits]
    movsd [r12+rdi*8],xmm1
    inc edi
    cmp edi,NULL_COUNT
    jb 1b
    mov rcx,r12
    mov edx,NULL_COUNT
    call Median
    mulsd xmm0,[rip+mad_factor]
    maxsd xmm0,[rip+epsilon]
    lea rax,[rip+norm_scale]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,MODEL_COUNT
    jb .bn_columns
    xor esi,esi
.bn_standardize:
    imul eax,esi,MODEL_COUNT*8
    lea rbx,[rip+null_models]
    add rbx,rax
    xor edi,edi
    lea r12,[rip+norm_center]
    lea r13,[rip+norm_scale]
1:  movsd xmm0,[rbx+rdi*8]
    subsd xmm0,[r12+rdi*8]
    divsd xmm0,[r13+rdi*8]
    maxsd xmm0,[rip+neg_zlimit]
    minsd xmm0,[rip+zlimit]
    movsd [rbx+rdi*8],xmm0
    inc edi
    cmp edi,MODEL_COUNT
    jb 1b
    inc esi
    cmp esi,NULL_COUNT
    jb .bn_standardize
    call ModelCorrelation
    call PrepareFast
    END

FN ModelCorrelation
    lea rbx,[rip+null_models]
    xor esi,esi
.mc_moments:
    xorpd xmm0,xmm0
    xorpd xmm1,xmm1
    xor edi,edi
1:  imul eax,edi,MODEL_COUNT
    add eax,esi
    movsd xmm2,[rbx+rax*8]
    addsd xmm0,xmm2
    mulsd xmm2,xmm2
    addsd xmm1,xmm2
    inc edi
    cmp edi,NULL_COUNT
    jb 1b
    mov eax,NULL_COUNT
    cvtsi2sd xmm2,eax
    divsd xmm0,xmm2
    divsd xmm1,xmm2
    movapd xmm3,xmm0
    mulsd xmm3,xmm3
    subsd xmm1,xmm3
    maxsd xmm1,[rip+epsilon]
    sqrtsd xmm1,xmm1
    lea rax,[rip+model_null_mean]
    movsd [rax+rsi*8],xmm0
    lea rax,[rip+model_null_sd]
    movsd [rax+rsi*8],xmm1
    inc esi
    cmp esi,MODEL_COUNT
    jb .mc_moments
    xor esi,esi
.mc_i:
    xor r12d,r12d
.mc_j:
    xorpd xmm0,xmm0
    xor edi,edi
1:  imul eax,edi,MODEL_COUNT
    lea ecx,[rax+rsi]
    add eax,r12d
    movsd xmm1,[rbx+rcx*8]
    mulsd xmm1,[rbx+rax*8]
    addsd xmm0,xmm1
    inc edi
    cmp edi,NULL_COUNT
    jb 1b
    mov eax,NULL_COUNT
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    lea rax,[rip+model_null_mean]
    movsd xmm1,[rax+rsi*8]
    mulsd xmm1,[rax+r12*8]
    subsd xmm0,xmm1
    lea rax,[rip+model_null_sd]
    divsd xmm0,[rax+rsi*8]
    divsd xmm0,[rax+r12*8]
    imul eax,esi,MODEL_COUNT
    add eax,r12d
    lea rdx,[rip+model_corr]
    movsd [rdx+rax*8],xmm0
    inc r12d
    cmp r12d,MODEL_COUNT
    jb .mc_j
    inc esi
    cmp esi,MODEL_COUNT
    jb .mc_i
    END

FN ComputeNormalized
    call ComputeModels
    lea rax,[rip+raw_models]
    lea rdx,[rip+norm_center]
    lea r8,[rip+norm_scale]
    lea r9,[rip+norm_z]
    xor ecx,ecx
1:  movsd xmm0,[rax+rcx*8]
    subsd xmm0,[rdx+rcx*8]
    divsd xmm0,[r8+rcx*8]
    maxsd xmm0,[rip+neg_zlimit]
    minsd xmm0,[rip+zlimit]
    movsd [r9+rcx*8],xmm0
    inc ecx
    cmp ecx,MODEL_COUNT
    jb 1b
    END
# Descriptive pre-validation score. Validated sampling weights are separately
# limited by OCSG/FECL; displaying a high descriptive index is not evidence.
FN ScoreDeep
    call ComputeNormalized
    lea rax,[rip+norm_z]
    xorpd xmm0,xmm0
    xor ecx,ecx
1:  addsd xmm0,[rax+rcx*8]
    inc ecx
    cmp ecx,MODEL_COUNT
    jb 1b
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rax,xmm0
    END

FN PrepareFast
    lea rbx,[rip+number_models]
    lea rdi,[rip+norm_scale]
    lea r12,[rip+number_fast]
    xor esi,esi
.pf_num:
    xor ecx,ecx
    xorpd xmm0,xmm0
1:  imul eax,ecx,45
    add eax,esi
    movsd xmm1,[rbx+rax*8]
    divsd xmm1,[rdi+rcx*8]
    maxsd xmm1,[rip+neg_zlimit]
    minsd xmm1,[rip+zlimit]
    addsd xmm0,xmm1
    inc ecx
    cmp ecx,4
    jb 1b
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rax,xmm0
    mov [r12+rsi*8],rax
    inc esi
    cmp esi,45
    jb .pf_num
    lea rbx,[rip+pair_models]
    lea r12,[rip+pair_fast]
    xor esi,esi
1:  movsd xmm0,[rbx+rsi*8]
    divsd xmm0,[rdi+32]
    movsd xmm1,[rbx+rsi*8+45*45*8]
    divsd xmm1,[rdi+40]
    addsd xmm0,xmm1
    maxsd xmm0,[rip+neg_zlimit]
    minsd xmm0,[rip+zlimit]
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rax,xmm0
    mov [r12+rsi*8],rax
    inc esi
    cmp esi,45*45
    jb 1b
    END

# Fixed point hot path: six node lookups and 15 pair lookups. No divisions,
# feature normalization, allocation, RNG or OS calls inside the global scan.
FN ScoreFast,32
    mov rbx,rcx
    lea rdi,[rip+number_fast]
    xor r12d,r12d
    xor esi,esi
1:  bsf rax,rbx
    btr rbx,rax
    mov [rsp+32+rsi*4],eax
    add r12,[rdi+rax*8]
    inc esi
    cmp esi,6
    jb 1b
    lea rdi,[rip+pair_fast]
    xor esi,esi
2:  lea r8d,[rsi+1]
    imul r9d,dword ptr [rsp+32+rsi*4],45
3:  mov eax,[rsp+32+r8*4]
    add eax,r9d
    add r12,[rdi+rax*8]
    inc r8d
    cmp r8d,6
    jb 3b
    inc esi
    cmp esi,5
    jb 2b
    mov rax,r12
    END 32

# Min-heap ordering is (score ascending, mask descending). Thus tied survivors
# use a deterministic low-mask tie break, independent of insertion/worker order.
FN HeapInsert
    # rcx mask, rdx signed score, r8 masks, r9 scores;
    # count pointer and capacity supplied in r10/r11 by wrappers.
    mov r12,rcx
    mov r13,rdx
    mov r14,r8
    mov r15,r9
    mov ebx,[r10]
    cmp ebx,r11d
    jb .hi_append
    cmp rdx,[r9]
    jl .hi_out
    jg 1f
    cmp rcx,[r8]
    jae .hi_out
1:  xor ebx,ebx
    mov [r8],rcx
    mov [r9],rdx
.hi_down:
    lea esi,[rbx*2+1]
    cmp esi,r11d
    jae .hi_out
    lea edi,[rsi+1]
    cmp edi,r11d
    jae .hi_child
    mov rax,[r15+rdi*8]
    cmp rax,[r15+rsi*8]
    jl .hi_right
    jg .hi_child
    mov rax,[r14+rdi*8]
    cmp rax,[r14+rsi*8]
    jbe .hi_child
.hi_right: mov esi,edi
.hi_child:
    mov rax,[r15+rbx*8]
    cmp rax,[r15+rsi*8]
    jl .hi_out
    jg .hi_swapdown
    mov rax,[r14+rbx*8]
    cmp rax,[r14+rsi*8]
    jae .hi_out
.hi_swapdown:
    mov rax,[r14+rbx*8]
    mov rdx,[r14+rsi*8]
    mov [r14+rbx*8],rdx
    mov [r14+rsi*8],rax
    mov rax,[r15+rbx*8]
    mov rdx,[r15+rsi*8]
    mov [r15+rbx*8],rdx
    mov [r15+rsi*8],rax
    mov ebx,esi
    jmp .hi_down
.hi_append:
    mov [r14+rbx*8],r12
    mov [r15+rbx*8],r13
    inc dword ptr [r10]
.hi_up:
    test ebx,ebx
    jz .hi_out
    lea esi,[rbx-1]
    shr esi,1
    mov rax,[r15+rbx*8]
    cmp rax,[r15+rsi*8]
    jg .hi_out
    jl .hi_swapup
    mov rax,[r14+rbx*8]
    cmp rax,[r14+rsi*8]
    jbe .hi_out
.hi_swapup:
    mov rax,[r14+rbx*8]
    mov rdx,[r14+rsi*8]
    mov [r14+rbx*8],rdx
    mov [r14+rsi*8],rax
    mov rax,[r15+rbx*8]
    mov rdx,[r15+rsi*8]
    mov [r15+rbx*8],rdx
    mov [r15+rsi*8],rax
    mov ebx,esi
    jmp .hi_up
.hi_out: END
FastHeapInsert:
    lea r8,[rip+fast_masks]
    lea r9,[rip+fast_scores]
    lea r10,[rip+fast_count]
    mov r11d,TOP_FAST
    jmp HeapInsert
FinalHeapInsert:
    lea r8,[rip+final_masks]
    lea r9,[rip+final_scores]
    lea r10,[rip+final_count]
    mov r11d,TOP_FINAL
    jmp HeapInsert

FN ScanField
    mov dword ptr [rip+fast_count],0
    mov dword ptr [rip+final_count],0
    mov dword ptr [rip+scan_count],0
    mov dword ptr [rip+analysis_stage],2
    mov dword ptr [rip+rank_truth_position],1
    cmp qword ptr [rip+rank_truth_mask],0
    je 1f
    mov rcx,[rip+rank_truth_mask]
    call ScoreFast
    mov [rip+rank_truth_score],rax
1:  mov ebx,63
    mov r15,1
    shl r15,45
.sf_loop:
    mov rcx,rbx
    call ScoreFast
    cmp qword ptr [rip+rank_truth_mask],0
    je 1f
    cmp rax,[rip+rank_truth_score]
    jl 1f
    jg 2f
    cmp rbx,[rip+rank_truth_mask]
    jae 1f
2:  inc dword ptr [rip+rank_truth_position]
1:  mov rdx,rax
    mov rcx,rbx
    call FastHeapInsert
    inc dword ptr [rip+scan_count]
    test dword ptr [rip+scan_count],8191
    jnz .sf_next
    cmp dword ptr [rip+cancel_flag],0
    jne .sf_cancel
    mov eax,[rip+scan_count]
    imul eax,eax,10
    xor edx,edx
    mov ecx,814506
    div ecx
    mov [rip+progress],eax
.sf_next:
    # Gosper's successor on fixed-popcount masks; no duplicate combinations.
    mov rax,rbx
    neg rax
    and rax,rbx
    lea rdx,[rax+rbx]
    bsf rcx,rax
    xor rbx,rdx
    shr rbx,2
    shr rbx,cl
    or rbx,rdx
    cmp rbx,r15
    jb .sf_loop
    mov dword ptr [rip+analysis_stage],3
    xor esi,esi
.sf_deep:
    cmp esi,[rip+fast_count]
    jae .sf_done
    cmp dword ptr [rip+cancel_flag],0
    jne .sf_cancel
    lea rax,[rip+fast_masks]
    mov rbx,[rax+rsi*8]
    mov rcx,rbx
    call ScoreDeep
    mov rdx,rax
    mov rcx,rbx
    call FinalHeapInsert
    inc esi
    jmp .sf_deep
.sf_done:
    mov eax,[rip+final_count]
    mov [rip+field_count],eax
    mov eax,1
    jmp .sf_out
.sf_cancel:
    mov dword ptr [rip+field_ready],0
    xor eax,eax
.sf_out: END
