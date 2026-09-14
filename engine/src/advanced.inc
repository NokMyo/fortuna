# Advanced Evidence Layer (AEL): higher-order interaction, synthetic-null stress,
# temporal stability, hierarchical uncertainty shrinkage and deeper candidate search.
# Pure x86-64 assembly; no external runtime. These diagnostics never create
# evidence when the base ORACLE gates are closed.
.equ ADV_QUAD_COUNT,148995
.equ ADV_STAGE_COUNT,2048
.equ ADV_NULL_NORMAL,4096
.equ ADV_NULL_DEEP,8192
.equ ADV_RECENT_ROWS,256

.section .rdata,"dr"
adv_p_quad: .double 0.00010067451927917044
adv_prior_quad: .double 480.0
adv_prior_quad_mass: .double 0.04832376925400181
adv_quad_var_num: .double 0.00010066438392033835
adv_quad_k: .double 16.0
adv_fifteen: .double 15.0
adv_twenty: .double 20.0
adv_quarter: .double 0.25
adv_three_quarter: .double 0.75
adv_baseline_brier: .double 0.11555555555555556
adv_null_alpha: .double 0.20
adv_sixty: .double 60.0
adv_stability_normal: .double 0.10
adv_stability_deep: .double 0.20
adv_quad_cap_normal: .double 0.08
adv_quad_cap_deep: .double 0.15
adv_joint_accept_weight: .double 0.08
adv_joint_accept_cap: .double 0.32
adv_four: .double 4.0
adv_neg_four: .double -4.0
adv_coprime45: .long 1,2,4,7,8,11,13,14,16,17,19,22,23,26,28,29,31,32,34,37,38,41,43,44

.section .bss
.p2align 4
advanced_ready: .skip 4
advanced_mode: .skip 4
advanced_quad_train_count: .skip 4
advanced_quad_counts: .skip ADV_QUAD_COUNT*4
advanced_nums: .skip 6*4
advanced_outside: .skip 39*4
advanced_quad_validation: .skip 8
advanced_quad_support: .skip 8
advanced_quad_weight: .skip 8
advanced_null_p: .skip 8
advanced_null_support: .skip 8
advanced_temporal_stability: .skip 8
advanced_global_support: .skip 8
advanced_model_uncertainty: .skip MODEL_COUNT*8
advanced_segment_hits: .skip 4*8
advanced_segment_brier: .skip 4*8
advanced_segment_count: .skip 4*4
advanced_stage_count: .skip 4
advanced_stage_masks: .skip ADV_STAGE_COUNT*8
advanced_stage_scores: .skip ADV_STAGE_COUNT*8
advanced_elite_indices: .skip 32*4
advanced_candidate_stability: .skip TOP_FINAL*8

.section .text
# combinadic C(d,4)+C(c,3)+C(b,2)+a for zero-based a<b<c<d.
FN AdvancedQuadIndex,32
    mov [rsp+32],ecx
    mov [rsp+36],edx
    mov [rsp+40],r8d
    mov [rsp+44],r9d
    mov ecx,r9d
    mov edx,4
    call Choose
    mov ebx,eax
    mov ecx,[rsp+40]
    mov edx,3
    call Choose
    add ebx,eax
    mov ecx,[rsp+36]
    mov edx,2
    call Choose
    add eax,ebx
    add eax,[rsp+32]
    END 32

# Add the fifteen 4-subsets contained in one valid six-mask.
FN AdvancedAddQuadMask
    mov rbx,rcx
    lea rdx,[rip+advanced_nums]
    call MaskToNumbers
    test eax,eax
    jz .aaqm_fail
    xor esi,esi
.aaqm_i:
    lea edi,[rsi+1]
.aaqm_j:
    lea r12d,[rdi+1]
.aaqm_k:
    lea r13d,[r12+1]
.aaqm_l:
    lea r14,[rip+advanced_nums]
    mov ecx,[r14+rsi*4]
    dec ecx
    mov edx,[r14+rdi*4]
    dec edx
    mov r8d,[r14+r12*4]
    dec r8d
    mov r9d,[r14+r13*4]
    dec r9d
    call AdvancedQuadIndex
    lea rdx,[rip+advanced_quad_counts]
    inc dword ptr [rdx+rax*4]
    inc r13d
    cmp r13d,6
    jb .aaqm_l
    inc r12d
    cmp r12d,5
    jb .aaqm_k
    inc edi
    cmp edi,4
    jb .aaqm_j
    inc esi
    cmp esi,3
    jb .aaqm_i
    mov eax,1
    jmp .aaqm_out
.aaqm_fail: xor eax,eax
.aaqm_out: END

FN AdvancedBuildQuadCounts
    lea rcx,[rip+advanced_quad_counts]
    mov edx,ADV_QUAD_COUNT*4
    call Zero
    xor esi,esi
.abqc_loop:
    cmp esi,[rip+history_count]
    jae .abqc_done
    imul eax,esi,RECORD_SIZE
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    call AdvancedAddQuadMask
    test eax,eax
    jz .abqc_fail
    inc esi
    jmp .abqc_loop
.abqc_done:
    mov eax,[rip+history_count]
    mov [rip+advanced_quad_train_count],eax
    mov eax,1
    jmp .abqc_out
.abqc_fail: xor eax,eax
.abqc_out: END

# Prefix-safe fourth-order residual score. No lower-order full-history state used.
FN AdvancedQuadResidualScore,64
    mov rbx,rcx
    lea rdx,[rip+advanced_nums]
    call MaskToNumbers
    test eax,eax
    jz .aqrs_zero
    xorpd xmm5,xmm5
    xor esi,esi
.aqrs_i:
    lea edi,[rsi+1]
.aqrs_j:
    lea r12d,[rdi+1]
.aqrs_k:
    lea r13d,[r12+1]
.aqrs_l:
    lea r14,[rip+advanced_nums]
    mov ecx,[r14+rsi*4]
    dec ecx
    mov edx,[r14+rdi*4]
    dec edx
    mov r8d,[r14+r12*4]
    dec r8d
    mov r9d,[r14+r13*4]
    dec r9d
    call AdvancedQuadIndex
    lea rdx,[rip+advanced_quad_counts]
    cvtsi2sd xmm0,dword ptr [rdx+rax*4]
    movsd [rsp+32],xmm0
    addsd xmm0,[rip+adv_prior_quad_mass]
    cvtsi2sd xmm1,dword ptr [rip+advanced_quad_train_count]
    addsd xmm1,[rip+adv_prior_quad]
    movsd [rsp+40],xmm1
    divsd xmm0,xmm1
    subsd xmm0,[rip+adv_p_quad]
    movsd xmm2,[rip+adv_quad_var_num]
    divsd xmm2,xmm1
    sqrtsd xmm2,xmm2
    maxsd xmm2,[rip+epsilon]
    divsd xmm0,xmm2
    movsd xmm1,[rsp+32]
    movsd xmm2,xmm1
    addsd xmm2,[rip+adv_quad_k]
    divsd xmm1,xmm2
    mulsd xmm0,xmm1
    addsd xmm5,xmm0
    inc r13d
    cmp r13d,6
    jb .aqrs_l
    inc r12d
    cmp r12d,5
    jb .aqrs_k
    inc edi
    cmp edi,4
    jb .aqrs_j
    inc esi
    cmp esi,3
    jb .aqrs_i
    movapd xmm0,xmm5
    divsd xmm0,[rip+adv_fifteen]
    jmp .aqrs_out
.aqrs_zero: xorpd xmm0,xmm0
.aqrs_out: END 64

# Hierarchical fourth-order score: 75% validated quad residual + 25% parent triple mean.
FN AdvancedQuadScore,64
    mov rbx,rcx
    call AdvancedQuadResidualScore
    movsd [rsp+32],xmm0
    mov rcx,rbx
    lea rdx,[rip+advanced_nums]
    call MaskToNumbers
    test eax,eax
    jz .aqs_residual
    xorpd xmm5,xmm5
    xor esi,esi
.aqs_i:
    lea edi,[rsi+1]
.aqs_j:
    lea r12d,[rdi+1]
.aqs_k:
    lea r13,[rip+advanced_nums]
    mov ecx,[r13+rsi*4]
    dec ecx
    mov edx,[r13+rdi*4]
    dec edx
    mov r8d,[r13+r12*4]
    dec r8d
    call TripleIndex
    lea rdx,[rip+triple_scores]
    addsd xmm5,[rdx+rax*8]
    inc r12d
    cmp r12d,6
    jb .aqs_k
    inc edi
    cmp edi,5
    jb .aqs_j
    inc esi
    cmp esi,4
    jb .aqs_i
    divsd xmm5,[rip+adv_twenty]
    movsd xmm0,[rsp+32]
    mulsd xmm0,[rip+adv_three_quarter]
    mulsd xmm5,[rip+adv_quarter]
    addsd xmm0,xmm5
    jmp .aqs_out
.aqs_residual:
    movsd xmm0,[rsp+32]
.aqs_out: END 64

# Walk-forward validation dedicated to the new fourth-order residual. At each
# cutoff the score sees only the previous draws. Uniform matched candidates form
# a foldwise percentile baseline. The final counts are then complete for N draws.
FN AdvancedValidateQuad,48
    mov rax,[rip+rng_state]
    mov [rsp+32],rax
    mov eax,[rip+rng_mode]
    mov [rsp+40],eax
    mov dword ptr [rip+rng_mode],1
    mov rax,[rip+hash_dataset]
    xor rax,0x51414434
    mov [rip+rng_state],rax
    mov dword ptr [rip+rng_error],0
    lea rcx,[rip+advanced_quad_counts]
    mov edx,ADV_QUAD_COUNT*4
    call Zero
    mov eax,[rip+history_count]
    cmp eax,61
    jb .avq_neutral
    xor esi,esi
.avq_seed:
    cmp esi,60
    jae .avq_folds
    imul eax,esi,RECORD_SIZE
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    call AdvancedAddQuadMask
    test eax,eax
    jz .avq_fail
    inc esi
    jmp .avq_seed
.avq_folds:
    mov ebp,60
    xorpd xmm4,xmm4
    xor r14d,r14d
.avq_fold:
    cmp ebp,[rip+history_count]
    jae .avq_done
    cmp dword ptr [rip+cancel_flag],0
    jne .avq_fail
    mov [rip+advanced_quad_train_count],ebp
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov rbx,[rdx+rax+40]
    mov rcx,rbx
    call AdvancedQuadResidualScore
    movsd [rsp+48],xmm0
    mov r12d,32
    cmp dword ptr [rip+advanced_mode],0
    je 1f
    mov r12d,96
1:  xor r13d,r13d
    xor edi,edi
.avq_null:
    call GenerateUniformMask
    test rax,rax
    jz .avq_fail
    mov rcx,rax
    call AdvancedQuadResidualScore
    comisd xmm0,[rsp+48]
    jb 2f
    inc edi
2:  inc r13d
    cmp r13d,r12d
    jb .avq_null
    lea eax,[rdi+1]
    cvtsi2sd xmm0,eax
    lea eax,[r12+1]
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    movsd xmm1,[rip+one]
    subsd xmm1,xmm0
    addsd xmm4,xmm1
    inc r14d
    mov rcx,rbx
    call AdvancedAddQuadMask
    test eax,eax
    jz .avq_fail
    inc ebp
    jmp .avq_fold
.avq_done:
    mov eax,[rip+history_count]
    mov [rip+advanced_quad_train_count],eax
    test r14d,r14d
    jz .avq_neutral
    cvtsi2sd xmm0,r14d
    divsd xmm4,xmm0
    movsd [rip+advanced_quad_validation],xmm4
    movapd xmm0,xmm4
    subsd xmm0,[rip+half]
    addsd xmm0,xmm0
    maxsd xmm0,[rip+zero]
    minsd xmm0,[rip+one]
    movsd [rip+advanced_quad_support],xmm0
    mov eax,1
    jmp .avq_restore
.avq_neutral:
    call AdvancedBuildQuadCounts
    movsd xmm0,[rip+half]
    movsd [rip+advanced_quad_validation],xmm0
    mov qword ptr [rip+advanced_quad_support],0
    mov eax,1
    jmp .avq_restore
.avq_fail: xor eax,eax
.avq_restore:
    mov rdx,[rsp+32]
    mov [rip+rng_state],rdx
    mov edx,[rsp+40]
    mov [rip+rng_mode],edx
    END 48

# Affine permutation of 45 labels. The temporal-block-dependent parameters keep
# within-draw cardinality and coarse structural properties while breaking the
# original number identities used by the prefix forecasts.
FN AdvancedPermuteMask,32
    mov rbx,rcx
    mov r12d,edx
    mov r13d,r8d
    mov eax,r13d
    imul eax,eax,977
    add eax,r12d
    xor edx,edx
    mov ecx,24
    div ecx
    lea rcx,[rip+adv_coprime45]
    mov r14d,[rcx+rdx*4]
    imul eax,r12d,7
    imul ecx,r13d,11
    add eax,ecx
    xor edx,edx
    mov ecx,45
    div ecx
    mov r15d,edx
    xor esi,esi
.apm_bits:
    test rbx,rbx
    jz .apm_done
    bsf rax,rbx
    btr rbx,rax
    imul eax,r14d
    add eax,r15d
    xor edx,edx
    mov ecx,45
    div ecx
    bts rsi,rdx
    jmp .apm_bits
.apm_done:
    mov rax,rsi
    END 32

# 4,096/8,192 structure-preserving synthetic-null histories against the most
# recent 256 prefix-only validation rows. This is a stress test, not a proof of
# lottery predictability.
FN AdvancedSyntheticNull,48
    mov ebp,[rip+validation_count]
    cmp ebp,16
    jb .asn_neutral
    mov r12d,ADV_RECENT_ROWS
    cmp ebp,r12d
    jae 1f
    mov r12d,ebp
1:  mov r13d,ebp
    sub r13d,r12d
    xor r14d,r14d
    xor esi,esi
.asn_observed:
    cmp esi,r12d
    jae .asn_rep_setup
    lea eax,[r13+rsi]
    imul eax,eax,64
    lea rdx,[rip+validation_rows]
    add rdx,rax
    mov rcx,[rdx+8]
    and rcx,[rdx+16]
    call PopCount64
    add r14d,eax
    inc esi
    jmp .asn_observed
.asn_rep_setup:
    mov r15d,ADV_NULL_NORMAL
    cmp dword ptr [rip+advanced_mode],0
    je 2f
    mov r15d,ADV_NULL_DEEP
2:  xor ebx,ebx
    xor edi,edi
.asn_rep:
    cmp edi,r15d
    jae .asn_done
    cmp dword ptr [rip+cancel_flag],0
    jne .asn_fail
    xor esi,esi
    xor ebp,ebp
.asn_row:
    cmp esi,r12d
    jae .asn_compare
    lea eax,[r13+rsi]
    imul eax,eax,64
    lea rdx,[rip+validation_rows]
    add rdx,rax
    mov rax,rsi
    imul eax,eax,4
    xor edx,edx
    div r12d
    mov r8d,eax
    lea rax,[rip+validation_rows]
    lea ecx,[r13+rsi]
    imul ecx,ecx,64
    add rax,rcx
    mov rcx,[rax+16]
    mov edx,edi
    call AdvancedPermuteMask
    lea rdx,[rip+validation_rows]
    lea ecx,[r13+rsi]
    imul ecx,ecx,64
    add rdx,rcx
    mov rcx,[rdx+8]
    and rcx,rax
    call PopCount64
    add ebp,eax
    inc esi
    jmp .asn_row
.asn_compare:
    cmp ebp,r14d
    jb 3f
    inc ebx
3:  inc edi
    jmp .asn_rep
.asn_done:
    lea eax,[rbx+1]
    cvtsi2sd xmm0,eax
    lea eax,[r15+1]
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    movsd [rip+advanced_null_p],xmm0
    divsd xmm0,[rip+adv_null_alpha]
    minsd xmm0,[rip+one]
    movsd xmm1,[rip+one]
    subsd xmm1,xmm0
    maxsd xmm1,[rip+zero]
    movsd [rip+advanced_null_support],xmm1
    mov eax,1
    jmp .asn_out
.asn_neutral:
    movsd xmm0,[rip+one]
    movsd [rip+advanced_null_p],xmm0
    mov qword ptr [rip+advanced_null_support],0
    mov eax,1
    jmp .asn_out
.asn_fail: xor eax,eax
.asn_out: END 48

# Four chronological blocks from the recent prefix-only folds. A block earns
# half credit for lower Brier than uniform and half for at least uniform mean hit.
FN AdvancedTemporalStability,32
    lea rcx,[rip+advanced_segment_hits]
    mov edx,4*8
    call Zero
    lea rcx,[rip+advanced_segment_brier]
    mov edx,4*8
    call Zero
    lea rcx,[rip+advanced_segment_count]
    mov edx,4*4
    call Zero
    mov ebp,[rip+validation_count]
    cmp ebp,8
    jb .ats_neutral
    mov r12d,ADV_RECENT_ROWS
    cmp ebp,r12d
    jae 1f
    mov r12d,ebp
1:  mov r13d,ebp
    sub r13d,r12d
    xor esi,esi
.ats_rows:
    cmp esi,r12d
    jae .ats_score
    mov eax,esi
    imul eax,eax,4
    xor edx,edx
    div r12d
    mov edi,eax
    lea eax,[r13+rsi]
    imul eax,eax,64
    lea rbx,[rip+validation_rows]
    add rbx,rax
    lea rdx,[rip+advanced_segment_hits]
    movsd xmm0,[rdx+rdi*8]
    addsd xmm0,[rbx+24]
    movsd [rdx+rdi*8],xmm0
    lea rdx,[rip+advanced_segment_brier]
    movsd xmm0,[rdx+rdi*8]
    addsd xmm0,[rbx+40]
    movsd [rdx+rdi*8],xmm0
    lea rdx,[rip+advanced_segment_count]
    inc dword ptr [rdx+rdi*4]
    inc esi
    jmp .ats_rows
.ats_score:
    xorpd xmm5,xmm5
    xor esi,esi
.ats_seg:
    lea rdx,[rip+advanced_segment_count]
    mov eax,[rdx+rsi*4]
    test eax,eax
    jz .ats_next
    cvtsi2sd xmm3,eax
    lea rdx,[rip+advanced_segment_brier]
    movsd xmm0,[rdx+rsi*8]
    divsd xmm0,xmm3
    comisd xmm0,[rip+adv_baseline_brier]
    jae 2f
    addsd xmm5,[rip+half]
2:  lea rdx,[rip+advanced_segment_hits]
    movsd xmm0,[rdx+rsi*8]
    divsd xmm0,xmm3
    movsd xmm1,[rip+expected_hits]
    comisd xmm0,xmm1
    jb .ats_next
    addsd xmm5,[rip+half]
.ats_next:
    inc esi
    cmp esi,4
    jb .ats_seg
    mov eax,4
    cvtsi2sd xmm0,eax
    divsd xmm5,xmm0
    movsd [rip+advanced_temporal_stability],xmm5
    mov eax,1
    jmp .ats_out
.ats_neutral:
    movsd xmm0,[rip+half]
    movsd [rip+advanced_temporal_stability],xmm0
    mov eax,1
.ats_out: END 32

# Empirical-Bayes-style shrinkage of already-gated model weights. Sample size,
# synthetic-null survivability, temporal stability and each model's BH q-value
# propagate uncertainty into both evidence magnitude and the uniform-mixture rho.
FN AdvancedShrinkWeights,32
    cvtsi2sd xmm0,dword ptr [rip+validation_count]
    movapd xmm1,xmm0
    addsd xmm1,[rip+adv_sixty]
    divsd xmm0,xmm1
    movsd xmm1,[rip+advanced_null_support]
    addsd xmm1,[rip+one]
    mulsd xmm1,[rip+half]
    mulsd xmm0,xmm1
    movsd xmm1,[rip+advanced_temporal_stability]
    addsd xmm1,[rip+one]
    mulsd xmm1,[rip+half]
    mulsd xmm0,xmm1
    maxsd xmm0,[rip+zero]
    minsd xmm0,[rip+one]
    movsd [rip+advanced_global_support],xmm0
    xor esi,esi
.asw_model:
    cmp esi,MODEL_COUNT-1
    jae .asw_global
    lea rdx,[rip+model_qvalue]
    movsd xmm1,[rdx+rsi*8]
    minsd xmm1,[rip+half]
    movsd xmm2,[rip+one]
    subsd xmm2,xmm1
    mulsd xmm2,xmm0
    lea rdx,[rip+model_weights]
    movsd xmm3,[rdx+rsi*8]
    mulsd xmm3,xmm2
    movsd [rdx+rsi*8],xmm3
    movsd xmm4,[rip+one]
    subsd xmm4,xmm2
    lea rdx,[rip+advanced_model_uncertainty]
    movsd [rdx+rsi*8],xmm4
    inc esi
    jmp .asw_model
.asw_global:
    movsd xmm1,[rip+evidence]
    mulsd xmm1,xmm0
    movsd [rip+evidence],xmm1
    movsd xmm1,[rip+one]
    subsd xmm1,[rip+rho]
    mulsd xmm1,xmm0
    movsd xmm2,[rip+one]
    subsd xmm2,xmm1
    movsd [rip+rho],xmm2
    mov eax,1
    END 32

FN AdvancedSetQuadWeight
    movsd xmm0,[rip+advanced_quad_support]
    mulsd xmm0,[rip+advanced_null_support]
    movsd xmm1,[rip+advanced_temporal_stability]
    addsd xmm1,[rip+one]
    mulsd xmm1,[rip+half]
    mulsd xmm0,xmm1
    cmp dword ptr [rip+advanced_mode],0
    jne 1f
    mulsd xmm0,[rip+adv_quad_cap_normal]
    jmp 2f
1:  mulsd xmm0,[rip+adv_quad_cap_deep]
2:  movsd [rip+advanced_quad_weight],xmm0
    mov eax,1
    END

# Weighted validated energy plus the separately validated hierarchical quad term.
FN AdvancedCandidateEnergy,32
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
2:  mov rcx,rbx
    call AdvancedQuadScore
    mulsd xmm0,[rip+advanced_quad_weight]
    addsd xmm0,xmm5
    END 32

# Exact score sensitivity to removal of the two largest absolute model
# contributions. 1 means diffuse support; 0 means two models dominate.
FN AdvancedCandidateStability,32
    call ComputeNormalized
    lea rdx,[rip+norm_z]
    lea r8,[rip+model_weights]
    xorpd xmm5,xmm5
    xorpd xmm3,xmm3
    xorpd xmm4,xmm4
    xor esi,esi
1:  movsd xmm0,[rdx+rsi*8]
    mulsd xmm0,[r8+rsi*8]
    andpd xmm0,[rip+abs_bits]
    addsd xmm5,xmm0
    comisd xmm0,xmm3
    jbe 2f
    movapd xmm4,xmm3
    movapd xmm3,xmm0
    jmp 3f
2:  comisd xmm0,xmm4
    jbe 3f
    movapd xmm4,xmm0
3:  inc esi
    cmp esi,MODEL_COUNT-1
    jb 1b
    comisd xmm5,[rip+epsilon]
    ja 4f
    movsd xmm0,[rip+half]
    jmp 5f
4:  movsd xmm0,[rip+one]
    addsd xmm3,xmm4
    divsd xmm3,xmm5
    subsd xmm0,xmm3
    maxsd xmm0,[rip+zero]
    minsd xmm0,[rip+one]
5:  END 32

# Expand 4,096 fast survivors to MAX_FAST, then retain a 2,048-candidate
# advanced stage before the existing 512 thermal field.
FN AdvancedExpandCandidates,16
    mov eax,[rip+scan_capacity]
    mov [rsp+32],eax
    mov dword ptr [rip+scan_capacity],MAX_FAST
    call ScanField
    test eax,eax
    jz .aec_fail
    mov dword ptr [rip+advanced_stage_count],0
    xor esi,esi
.aec_stage:
    cmp esi,[rip+fast_count]
    jae .aec_final
    cmp dword ptr [rip+cancel_flag],0
    jne .aec_fail
    lea rax,[rip+fast_masks]
    mov rbx,[rax+rsi*8]
    mov rcx,rbx
    call AdvancedCandidateEnergy
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rdx,xmm0
    mov rcx,rbx
    lea r8,[rip+advanced_stage_masks]
    lea r9,[rip+advanced_stage_scores]
    lea r10,[rip+advanced_stage_count]
    mov r11d,ADV_STAGE_COUNT
    call HeapInsert
    inc esi
    jmp .aec_stage
.aec_final:
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
    jmp .aec_restore
.aec_fail: xor eax,eax
.aec_restore:
    mov edx,[rsp+32]
    mov [rip+scan_capacity],edx
    END 16

FN AdvancedFieldContains
    xor eax,eax
    xor r8d,r8d
    lea r9,[rip+final_masks]
1:  cmp r8d,[rip+field_count]
    jae 3f
    cmp r8d,edx
    je 2f
    cmp rcx,[r9+r8*8]
    jne 2f
    mov eax,1
    jmp 3f
2:  inc r8d
    jmp 1b
3:  END

# Identify the strongest 8 (normal) or 32 (deep) stage-B candidates and examine
# their complete C(6,2)*C(39,2)=11,115 two-swap neighborhoods.
FN AdvancedTwoSwapRefine,64
    mov r12d,8
    cmp dword ptr [rip+advanced_mode],0
    je 1f
    mov r12d,32
1:  xor esi,esi
.atsr_select:
    cmp esi,r12d
    jae .atsr_refine
    mov r13,0x8000000000000000
    mov r14d,-1
    xor edi,edi
.atsr_scan:
    cmp edi,[rip+field_count]
    jae .atsr_store
    xor ebp,ebp
2:  cmp ebp,esi
    jae 3f
    lea rax,[rip+advanced_elite_indices]
    cmp edi,[rax+rbp*4]
    je .atsr_next
    inc ebp
    jmp 2b
3:  lea rax,[rip+final_scores]
    mov rbx,[rax+rdi*8]
    cmp rbx,r13
    jle .atsr_next
    mov r13,rbx
    mov r14d,edi
.atsr_next:
    inc edi
    jmp .atsr_scan
.atsr_store:
    cmp r14d,0
    jl .atsr_done
    lea rax,[rip+advanced_elite_indices]
    mov [rax+rsi*4],r14d
    inc esi
    jmp .atsr_select
.atsr_refine:
    xor esi,esi
.atsr_each:
    cmp esi,r12d
    jae .atsr_done
    cmp dword ptr [rip+cancel_flag],0
    jne .atsr_fail
    lea rax,[rip+advanced_elite_indices]
    mov r15d,[rax+rsi*4]
    lea rax,[rip+final_masks]
    mov rbx,[rax+r15*8]
    mov rcx,rbx
    lea rdx,[rip+advanced_nums]
    call MaskToNumbers
    test eax,eax
    jz .atsr_fail
    xor edi,edi
    xor ebp,ebp
.atsr_outside:
    cmp edi,45
    jae .atsr_base
    bt rbx,rdi
    jc 4f
    lea rax,[rip+advanced_outside]
    mov [rax+rbp*4],edi
    inc ebp
4:  inc edi
    jmp .atsr_outside
.atsr_base:
    mov rcx,rbx
    call AdvancedCandidateEnergy
    movsd [rsp+32],xmm0
    mov [rsp+40],rbx
    xor edi,edi
.atsr_i:
    lea ebp,[rdi+1]
.atsr_j:
    xor r13d,r13d
.atsr_u:
    lea r14d,[r13+1]
.atsr_v:
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
    call AdvancedCandidateEnergy
    comisd xmm0,[rsp+32]
    jbe 5f
    mov rcx,[rsp+48]
    mov edx,r15d
    call AdvancedFieldContains
    test eax,eax
    jnz 5f
    mov rcx,[rsp+48]
    call AdvancedCandidateEnergy
    movsd [rsp+32],xmm0
    mov rax,[rsp+48]
    mov [rsp+40],rax
5:  inc r14d
    cmp r14d,39
    jb .atsr_v
    inc r13d
    cmp r13d,38
    jb .atsr_u
    inc ebp
    cmp ebp,6
    jb .atsr_j
    inc edi
    cmp edi,5
    jb .atsr_i
    lea rax,[rip+final_masks]
    mov rdx,[rsp+40]
    mov [rax+r15*8],rdx
    movsd xmm0,[rsp+32]
    mulsd xmm0,[rip+scale_q]
    cvtsd2si rdx,xmm0
    lea rax,[rip+final_scores]
    mov [rax+r15*8],rdx
    inc esi
    jmp .atsr_each
.atsr_done: mov eax,1; jmp .atsr_out
.atsr_fail: xor eax,eax
.atsr_out: END 64

FN AdvancedRehashField
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
    END

# Recalibrate thermal temperature after candidate-specific quad and leave-two
# stability corrections, preserving the same entropy target as the base field.
FN AdvancedRethermal,32
    movsd xmm5,[rip+neg_zlimit]
    xor esi,esi
1:  cmp esi,[rip+field_count]
    jae 2f
    lea rdx,[rip+field_z]
    movsd xmm0,[rdx+rsi*8]
    maxsd xmm5,xmm0
    inc esi
    jmp 1b
2:  movsd [rip+field_max],xmm5
    cvtsi2sd xmm0,dword ptr [rip+field_count]
    call Log
    mulsd xmm0,[rip+entropy_target]
    movsd [rsp+32],xmm0
    movsd xmm0,[rip+thermal_low]
    movsd [rsp+40],xmm0
    movsd xmm0,[rip+thermal_high]
    movsd [rsp+48],xmm0
    mov ebp,24
3:  movsd xmm0,[rsp+40]
    addsd xmm0,[rsp+48]
    mulsd xmm0,[rip+half]
    movsd [rip+temperature],xmm0
    call ThermalProbabilities
    movsd xmm0,[rip+field_entropy]
    comisd xmm0,[rsp+32]
    jb 4f
    movsd xmm0,[rip+temperature]
    movsd [rsp+48],xmm0
    jmp 5f
4:  movsd xmm0,[rip+temperature]
    movsd [rsp+40],xmm0
5:  dec ebp
    jnz 3b
    call ThermalProbabilities
    call BuildMarginals
    call AdvancedRehashField
    mov dword ptr [rip+field_ready],1
    mov eax,1
    END 32

FN AdvancedAdjustField
    xor esi,esi
.aaf_loop:
    cmp esi,[rip+field_count]
    jae .aaf_thermal
    lea rax,[rip+final_masks]
    mov rbx,[rax+rsi*8]
    mov rcx,rbx
    call AdvancedQuadScore
    mulsd xmm0,[rip+advanced_quad_weight]
    movsd [rsp+32],xmm0
    mov rcx,rbx
    call AdvancedCandidateStability
    lea rdx,[rip+advanced_candidate_stability]
    movsd [rdx+rsi*8],xmm0
    subsd xmm0,[rip+half]
    cmp dword ptr [rip+advanced_mode],0
    jne 1f
    mulsd xmm0,[rip+adv_stability_normal]
    jmp 2f
1:  mulsd xmm0,[rip+adv_stability_deep]
2:  addsd xmm0,[rsp+32]
    lea rdx,[rip+field_z]
    addsd xmm0,[rdx+rsi*8]
    maxsd xmm0,[rip+neg_zlimit]
    minsd xmm0,[rip+zlimit]
    movsd [rdx+rsi*8],xmm0
    inc esi
    jmp .aaf_loop
.aaf_thermal:
    call AdvancedRethermal
    END

FN AdvancedFinalizeCommon
    mov [rip+advanced_mode],ecx
    mov dword ptr [rip+advanced_ready],0
    cmp dword ptr [rip+field_ready],1
    jne .afc_fail
    cmp dword ptr [rip+model_ready],1
    jne .afc_fail
    mov dword ptr [rip+analysis_stage],11
    call AdvancedValidateQuad
    test eax,eax
    jz .afc_fail
    call AdvancedTemporalStability
    test eax,eax
    jz .afc_fail
    call AdvancedSyntheticNull
    test eax,eax
    jz .afc_fail
    call AdvancedSetQuadWeight
    call AdvancedShrinkWeights
    mov dword ptr [rip+analysis_stage],12
    call AdvancedExpandCandidates
    test eax,eax
    jz .afc_fail
    mov dword ptr [rip+analysis_stage],13
    call AdvancedTwoSwapRefine
    test eax,eax
    jz .afc_fail
    call BuildGeometry
    test eax,eax
    jz .afc_fail
    call BuildWorlds
    test eax,eax
    jz .afc_fail
    call BuildThermal
    call AdvancedAdjustField
    test eax,eax
    jz .afc_fail
    mov dword ptr [rip+advanced_ready],1
    mov dword ptr [rip+analysis_stage],7
    mov dword ptr [rip+progress],100
    mov dword ptr [rip+rng_mode],0
    mov eax,1
    jmp .afc_out
.afc_fail:
    mov dword ptr [rip+advanced_ready],0
    mov dword ptr [rip+field_ready],0
    xor eax,eax
.afc_out: END

FN AdvancedFinalizeNormal
    xor ecx,ecx
    call AdvancedFinalizeCommon
    END
FN AdvancedFinalizeDeep
    mov ecx,1
    call AdvancedFinalizeCommon
    END

# Research sampling retains the exact joint model as the proposal distribution,
# then applies bounded rejection using validated standard-model + quad evidence.
# Full support remains because the acceptance probability is strictly positive.
FN AdvancedJointSample,32
    cmp dword ptr [rip+advanced_ready],1
    jne .ajs_base
    mov ebp,32
    xor r12,r12
.ajs_try:
    call JointSample
    test rax,rax
    jz .ajs_fail
    mov r12,rax
    mov rcx,rax
    call AdvancedCandidateEnergy
    maxsd xmm0,[rip+adv_neg_four]
    minsd xmm0,[rip+adv_four]
    mulsd xmm0,[rip+adv_joint_accept_weight]
    subsd xmm0,[rip+adv_joint_accept_cap]
    call Exp
    movsd [rsp+32],xmm0
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .ajs_fail
    comisd xmm0,[rsp+32]
    jbe .ajs_accept
    dec ebp
    jnz .ajs_try
.ajs_accept:
    mov rax,r12
    jmp .ajs_out
.ajs_base:
    call JointSample
    jmp .ajs_out
.ajs_fail: xor eax,eax
.ajs_out: END 32
