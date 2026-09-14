# Signal model preparation. All counts are null-centered; the candidate model
# vector is subsequently normalized against a deterministic uniform sample.
FN PrepareModels,64
    cvtsi2sd xmm0,dword ptr [rip+draw_count]
    movsd [rsp+32],xmm0
    movapd xmm1,xmm0
    addsd xmm1,[rip+prior]
    movsd [rsp+40],xmm1
    movsd xmm2,[rip+one]
    subsd xmm2,[rip+p_one]
    mulsd xmm2,[rip+p_one]
    divsd xmm2,xmm1
    sqrtsd xmm2,xmm2
    movsd [rsp+48],xmm2
    xor esi,esi
    lea rbx,[rip+number_models]
.pm_numbers:
    lea rax,[rip+freq_total]
    cvtsi2sd xmm0,dword ptr [rax+rsi*4]
    movsd xmm1,[rip+prior]
    mulsd xmm1,[rip+p_one]
    addsd xmm0,xmm1
    divsd xmm0,[rsp+40]
    subsd xmm0,[rip+p_one]
    divsd xmm0,[rsp+48]
    movsd [rbx+rsi*8],xmm0
    # OTRK: weighted first differences + second differences, scales 4..256.
    xor edi,edi
    xor ebp,ebp
    xorpd xmm5,xmm5
.pm_temporal:
    imul eax,edi,45
    add eax,esi
    lea rdx,[rip+freq_ewma]
    movsd xmm0,[rdx+rax*8]
    lea rcx,[rip+ewma_mass]
    divsd xmm0,[rcx+rdi*8]
    movsd xmm1,[rdx+rax*8+45*8]
    divsd xmm1,[rcx+rdi*8+8]
    movapd xmm2,xmm0
    subsd xmm2,xmm1
    addsd xmm5,xmm2
    cmp edi,5
    jae 1f
    movsd xmm3,[rdx+rax*8+90*8]
    divsd xmm3,[rcx+rdi*8+16]
    subsd xmm0,xmm1
    subsd xmm0,xmm1
    addsd xmm0,xmm3
    mulsd xmm0,[rip+half]
    addsd xmm5,xmm0
1:  subsd xmm1,[rip+p_one]
    comisd xmm1,[rip+zero]
    je 3f
    jb 2f
    inc ebp
    jmp 3f
2:  dec ebp
3:  inc edi
    cmp edi,6
    jb .pm_temporal
    movsd [rbx+rsi*8+45*8],xmm5
    mov eax,ebp
    neg eax
    cmovs eax,ebp
    cvtsi2sd xmm0,eax
    divsd xmm0,[rip+six]
    movsd [rbx+rsi*8+90*8],xmm0
    # Discrete hazard, censored initial intervals excluded by ProcessRecord.
    lea rax,[rip+last_seen]
    mov ecx,[rip+draw_count]
    sub ecx,[rax+rsi*4]
    cmp ecx,63
    jbe 4f
    mov ecx,63
4:  mov eax,esi
    shl eax,6
    add eax,ecx
    lea rdx,[rip+hazard_events]
    cvtsi2sd xmm0,dword ptr [rdx+rax*4]
    movsd xmm1,[rip+prior]
    mulsd xmm1,[rip+p_one]
    addsd xmm0,xmm1
    lea rdx,[rip+hazard_risk]
    cvtsi2sd xmm1,dword ptr [rdx+rax*4]
    addsd xmm1,[rip+prior]
    divsd xmm0,xmm1
    subsd xmm0,[rip+p_one]
    movsd [rbx+rsi*8+135*8],xmm0
    inc esi
    cmp esi,45
    jb .pm_numbers
    lea rbx,[rip+pair_models]
    xor esi,esi
.pm_pair:
    mov eax,esi
    xor edx,edx
    mov ecx,45
    div ecx
    cmp eax,edx
    je .pm_pair_zero
    lea rax,[rip+pair_counts]
    cvtsi2sd xmm0,dword ptr [rax+rsi*4]
    movsd xmm1,[rsp+32]
    mulsd xmm1,[rip+p_pair]
    subsd xmm0,xmm1
    movsd xmm2,[rip+one]
    subsd xmm2,[rip+p_pair]
    mulsd xmm1,xmm2
    addsd xmm1,[rip+one]
    sqrtsd xmm1,xmm1
    divsd xmm0,xmm1
    movsd xmm1,[rsp+32]
    addsd xmm1,[rip+prior_pair]
    movsd xmm2,[rsp+32]
    divsd xmm2,xmm1
    mulsd xmm0,xmm2
    movsd [rbx+rsi*8],xmm0
    lea rax,[rip+spectral_a]
    movsd [rax+rsi*8],xmm0
    lea rax,[rip+pair_decay]
    movsd xmm0,[rax+rsi*8]
    divsd xmm0,[rip+ewma_mass+32]
    subsd xmm0,[rip+p_pair]
    movsd [rbx+rsi*8+45*45*8],xmm0
    jmp .pm_pair_next
.pm_pair_zero:
    mov qword ptr [rbx+rsi*8],0
    mov qword ptr [rbx+rsi*8+45*45*8],0
    lea rax,[rip+spectral_a]
    mov qword ptr [rax+rsi*8],0
.pm_pair_next:
    inc esi
    cmp esi,45*45
    jb .pm_pair
    call SpectralLattice
    # Triple interaction information with explicit finite-without-replacement
    # null centering and 240-draw prior; empirical shrinkage c/(c+8).
    xor esi,esi
.pm_ta:
    lea edi,[rsi+1]
.pm_tb:
    lea r12d,[rdi+1]
.pm_tc:
    mov ecx,esi
    mov edx,edi
    mov r8d,r12d
    call TripleIndex
    mov r13d,eax
    lea rax,[rip+triple_counts]
    cvtsi2sd xmm0,dword ptr [rax+r13*4]
    movsd [rsp+56],xmm0
    movsd xmm1,[rip+prior_triple]
    mulsd xmm1,[rip+p_triple]
    addsd xmm0,xmm1
    movsd xmm1,[rsp+32]
    addsd xmm1,[rip+prior_triple]
    divsd xmm0,xmm1
    divsd xmm0,[rip+p_triple]
    movsd [rsp+64],xmm0
    mov [rsp+72],esi
    mov [rsp+76],edi
    mov [rsp+80],r12d
    xor ebp,ebp
.pm_tnum:
    mov eax,[rsp+72+rbp*4]
    lea rdx,[rip+freq_total]
    cvtsi2sd xmm0,dword ptr [rdx+rax*4]
    movsd xmm1,[rip+prior]
    mulsd xmm1,[rip+p_one]
    addsd xmm0,xmm1
    divsd xmm0,[rsp+40]
    divsd xmm0,[rip+p_one]
    mulsd xmm0,[rsp+64]
    movsd [rsp+64],xmm0
    inc ebp
    cmp ebp,3
    jb .pm_tnum
    mov ebp,0
.pm_tpair:
    mov eax,[rsp+72+rbp*4]
    lea ecx,[rbp+1]
    cmp ecx,3
    jb 1f
    xor ecx,ecx
1:  imul eax,eax,45
    add eax,[rsp+72+rcx*4]
    lea rdx,[rip+pair_counts]
    cvtsi2sd xmm1,dword ptr [rdx+rax*4]
    movsd xmm2,[rip+prior_pair]
    mulsd xmm2,[rip+p_pair]
    addsd xmm1,xmm2
    movsd xmm2,[rsp+32]
    addsd xmm2,[rip+prior_pair]
    divsd xmm1,xmm2
    divsd xmm1,[rip+p_pair]
    movsd xmm0,[rsp+64]
    divsd xmm0,xmm1
    movsd [rsp+64],xmm0
    inc ebp
    cmp ebp,3
    jb .pm_tpair
    call Log
    movsd xmm1,[rsp+56]
    mov eax,8
    cvtsi2sd xmm2,eax
    addsd xmm2,xmm1
    divsd xmm1,xmm2
    mulsd xmm0,xmm1
    lea rax,[rip+triple_scores]
    movsd [rax+r13*8],xmm0
    inc r12d
    cmp r12d,45
    jb .pm_tc
    inc edi
    cmp edi,44
    jb .pm_tb
    inc esi
    cmp esi,43
    jb .pm_ta
    # Covariance + scale-aware diagonal regularization; no inverse of singular
    # raw moments. Raw feature units preserved through ridge diagonal.
    lea rbx,[rip+cov_inverse]
    lea rdi,[rip+feature_m2]
    xor esi,esi
.pm_cov:
    mov eax,esi
    xor edx,edx
    mov ecx,FEATURE_COUNT
    div ecx
    movsd xmm0,[rdi+rsi*8]
    movsd xmm1,[rsp+32]
    subsd xmm1,[rip+one]
    maxsd xmm1,[rip+one]
    divsd xmm0,xmm1
    cmp eax,edx
    jne 1f
    movapd xmm2,xmm0
    mulsd xmm2,[rip+ridge]
    maxsd xmm2,[rip+ridge]
    addsd xmm0,xmm2
1:  movsd [rbx+rsi*8],xmm0
    inc esi
    cmp esi,FEATURE_COUNT*FEATURE_COUNT
    jb .pm_cov
    mov rcx,rbx
    mov edx,FEATURE_COUNT
    mov r8,rbx
    call MatrixInverse
    test eax,eax
    jz .pm_out
    mov dword ptr [rip+model_ready],1
    mov eax,1
.pm_out:
    END 64

# ComputeModels(mask) -> raw_models[12]. No dependence on future history.
FN ComputeModels,64
    mov rbx,rcx
    call ExtractFeatures
    lea rcx,[rip+raw_models]
    mov edx,MODEL_COUNT*8
    call Zero
    lea rdi,[rip+extract_buf]
    lea rbp,[rip+raw_models]
    xor esi,esi
.cm_num:
    mov eax,[rdi+rsi*4]
    dec eax
    lea rdx,[rip+number_models]
    xor ecx,ecx
1:  movsd xmm0,[rdx+rax*8]
    addsd xmm0,[rbp+rcx*8]
    movsd [rbp+rcx*8],xmm0
    add rdx,45*8
    inc ecx
    cmp ecx,4
    jb 1b
    lea rdx,[rip+spectral_l]
    imul eax,eax,46
    movsd xmm0,[rdx+rax*8]
    addsd xmm0,[rbp+48]
    movsd [rbp+48],xmm0
    inc esi
    cmp esi,6
    jb .cm_num
    xor esi,esi
.cm_pi:
    lea r12d,[rsi+1]
.cm_pj:
    mov r13d,[rdi+rsi*4]
    dec r13d
    mov r14d,[rdi+r12*4]
    dec r14d
    imul eax,r13d,45
    add eax,r14d
    lea rdx,[rip+pair_models]
    movsd xmm0,[rdx+rax*8]
    addsd xmm0,[rbp+32]
    movsd [rbp+32],xmm0
    movsd xmm0,[rdx+rax*8+45*45*8]
    addsd xmm0,[rbp+40]
    movsd [rbp+40],xmm0
    lea rdx,[rip+spectral_l]
    movsd xmm0,[rdx+rax*8]
    addsd xmm0,xmm0
    addsd xmm0,[rbp+48]
    movsd [rbp+48],xmm0
    lea r15d,[r12+1]
.cm_t:
    cmp r15d,6
    jae .cm_pnext
    mov ecx,r13d
    mov edx,r14d
    mov r8d,[rdi+r15*4]
    dec r8d
    call TripleIndex
    lea rdx,[rip+triple_scores]
    movsd xmm0,[rdx+rax*8]
    addsd xmm0,[rbp+56]
    movsd [rbp+56],xmm0
    inc r15d
    jmp .cm_t
.cm_pnext:
    inc r12d
    cmp r12d,6
    jb .cm_pj
    inc esi
    cmp esi,5
    jb .cm_pi
    # OHPS: regularized Mahalanobis shell, d0=sqrt(d-1).
    lea rbx,[rip+feature_buf]
    lea rdi,[rip+feature_mean]
    xor esi,esi
1:  movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rdi+rsi*8]
    movsd [rsp+32+rsi*8],xmm0
    inc esi
    cmp esi,FEATURE_COUNT
    jb 1b
    lea rdi,[rip+cov_inverse]
    xorpd xmm5,xmm5
    xor esi,esi
.cm_mahal:
    xor ecx,ecx
1:  mov eax,esi
    imul eax,eax,FEATURE_COUNT
    add eax,ecx
    movsd xmm0,[rdi+rax*8]
    mulsd xmm0,[rsp+32+rsi*8]
    mulsd xmm0,[rsp+32+rcx*8]
    addsd xmm5,xmm0
    inc ecx
    cmp ecx,FEATURE_COUNT
    jb 1b
    inc esi
    cmp esi,FEATURE_COUNT
    jb .cm_mahal
    maxsd xmm5,[rip+zero]
    sqrtsd xmm5,xmm5
    mov eax,FEATURE_COUNT-1
    cvtsi2sd xmm0,eax
    sqrtsd xmm0,xmm0
    subsd xmm5,xmm0
    mulsd xmm5,xmm5
    mulsd xmm5,[rip+half]
    xorpd xmm5,[rip+sign_bits]
    movsd [rbp+64],xmm5
    # FEB: history-relative entropy in five partitions, no maximize-entropy bias.
    lea rbx,[rip+entropy_buf]
    lea rdi,[rip+entropy_mean]
    lea rdx,[rip+entropy_m2]
    xor esi,esi
    xorpd xmm5,xmm5
1:  movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rdi+rsi*8]
    mulsd xmm0,xmm0
    movsd xmm1,[rdx+rsi*8]
    cvtsi2sd xmm2,dword ptr [rip+draw_count]
    maxsd xmm2,[rip+two]
    subsd xmm2,[rip+one]
    divsd xmm1,xmm2
    addsd xmm1,[rip+ridge]
    divsd xmm0,xmm1
    addsd xmm5,xmm0
    inc esi
    cmp esi,5
    jb 1b
    xorpd xmm5,[rip+sign_bits]
    mulsd xmm5,[rip+half]
    movsd [rbp+72],xmm5
    # OIS = interaction enhancement over the independent-number base.
    movsd xmm0,[rbp+32]
    addsd xmm0,[rbp+48]
    addsd xmm0,[rbp+56]
    movsd [rbp+80],xmm0
    # HSSM heuristic: birthdays, multiples, ending collisions, arithmetic runs.
    lea rdi,[rip+extract_buf]
    xor esi,esi
    xor ebx,ebx
1:  mov eax,[rdi+rsi*4]
    cmp eax,31
    jbe 2f
    add ebx,2
2:  xor edx,edx
    mov ecx,5
    div ecx
    test edx,edx
    jnz 3f
    dec ebx
3:  cmp esi,4
    jae 4f
    mov eax,[rdi+rsi*4+4]
    sub eax,[rdi+rsi*4]
    mov edx,[rdi+rsi*4+8]
    sub edx,[rdi+rsi*4+4]
    cmp eax,edx
    jne 4f
    dec ebx
4:  inc esi
    cmp esi,6
    jb 1b
    cvtsi2sd xmm0,ebx
    subsd xmm0,[rip+feature_buf+48]
    movsd [rbp+88],xmm0
    END 64
