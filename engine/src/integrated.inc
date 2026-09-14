# Integrated inference: finite empirical-Bayes posterior, four model families,
# causal validation/stacking, KL-penalized robustness, and full null refits.
# All probability normalization enumerates the six-of-N support exactly.
.equ TI_FAMILIES,4
.equ TI_GRID,5
.equ TI_FOLDS,16
.section .data
ti_grid_count: .long 3
ti_fold_limit: .long 2
ti_null_limit: .long 2
ti_ais_runs: .long 8
ti_ais_steps: .long 8
.section .rdata,"dr"
ti_scales: .double 0.0,0.5,1.0,1.5,2.0
ti_small: .double 0.000000000001
ti_q: .double 0.0001
ti_cap: .double 0.05
ti_negcap: .double -0.05
ti_rate: .double 0.02
ti_prior: .double 0.2
ti_margin: .double 0.000001
ti_report_title: .asciz "\r\nINTEGRATED TEN-FUNCTION INFERENCE 1.0 (empirical-Bayes finite grid)\r\nprofile,completed_mask,folds,null_refits,null_p,heldout_log_gain,AIS_relative_error,Z_lower,Z_upper\r\n"
ti_report_limits: .asciz "\r\nFour normalized families: high-order, rank-one tensor, parity changepoint, marginal Kalman. Finite-grid posterior is conditional on fitted empirical-Bayes bases, not a continuous full posterior. Bounds have a floating-point safety margin, not machine-certified interval arithmetic. AIS is checked against enumeration. No demonstrated lottery advantage. Legacy forecast ledger records the legacy joint model, not this mixture.\r\n"
ti_report_weights: .asciz "\r\nFamily,mixture_weight,posterior_weights_at_scales_0_0.5_1_1.5_2\r\n"
.section .bss
.p2align 4
ti_begin:
ti_ready: .skip 4
ti_profile: .skip 4
ti_completed: .skip 4
ti_folds: .skip 4
ti_null_done: .skip 4
ti_n: .skip 4
ti_real_n: .skip 4
ti_null_active: .skip 4
ti_space: .skip 8
ti_logspace: .skip 8
ti_null_p: .skip 8
ti_gain: .skip 8
ti_real_gain: .skip 8
ti_alpha: .skip 8
ti_ais_error: .skip 8
ti_z_lower: .skip 8
ti_z_upper: .skip 8
ti_coef: .skip TI_FAMILIES*JOINT_BYTES
ti_logz: .skip TI_FAMILIES*TI_GRID*8
ti_post: .skip TI_FAMILIES*TI_GRID*8
ti_weights: .skip 5*8
ti_rows: .skip TI_FOLDS*5*8
ti_end:
.equ TI_BYTES,ti_end-ti_begin
ti_saved_joint: .skip JOINT_BYTES
ti_saved_moments: .skip JOINT_BYTES
ti_saved_logz: .skip 8
.equ TI_JOINT_STATE_BYTES,joint_marginal_error+8-joint_coef
ti_saved_joint_state: .skip TI_JOINT_STATE_BYTES
ti_saved_order: .skip 4
ti_saved_shrink: .skip 8
ti_history: .skip 8192*8
ti_cp_prefix: .skip 8193*4
ti_logfact: .skip 8194*8
ti_cp_prob: .skip 8192*8
ti_counts: .skip 45*8
ti_state: .skip 45*8
ti_state_var: .skip 45*8
ti_tensor_u: .skip 45*8
ti_tensor_next: .skip 45*8
ti_energies: .skip TI_GRID*8
ti_z: .skip TI_GRID*8
ti_pred: .skip 5*8
ti_gradient: .skip 5*8
ti_losses: .skip TI_FOLDS*8
ti_ais_sum: .skip 8
ti_enum_count: .skip 8
.section .text
# Uniform mask over the configured test/production universe, never a modulo map.
FN TiUniform
    xor ebx,ebx
    xor esi,esi
1:  mov ecx,[rip+joint_universe]
    call RandomBelow
    cmp dword ptr [rip+rng_error],0
    jne 3f
    bt rbx,rax
    jc 1b
    bts rbx,rax
    inc esi
    cmp esi,6
    jb 1b
    mov rax,rbx
    jmp 4f
3:  xor eax,eax
4:  END

# rcx=mask, edx=family. Energy is bounded by the learned coefficient caps.
FN TiEnergy
    mov ebx,edx
    call JointIndices
    imul ebx,JOINT_BYTES
    lea rdx,[rip+ti_coef]
    add rdx,rbx
    lea r8,[rip+joint_indices]
    xorpd xmm0,xmm0
    xor ecx,ecx
1:  mov eax,[r8+rcx*4]
    addsd xmm0,[rdx+rax*8]
    inc ecx
    cmp ecx,41
    jb 1b
    END

# Tensor rank-one alternating contractions, followed by least-squares amplitude.
# Only distinct triples enter the objective. A zero tensor remains zero.
FN TiTensor,64
    lea rcx,[rip+ti_coef+JOINT_BYTES]
    lea rdx,[rip+ti_coef]
    mov r8d,JOINT_BYTES
    call Copy
    xor esi,esi
1:  lea rax,[rip+ti_coef]
    movsd xmm0,[rax+rsi*8]
    addsd xmm0,[rip+ti_prior]
    lea rax,[rip+ti_tensor_u]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb 1b
    mov ebp,8
.tit_iter:
    lea rcx,[rip+ti_tensor_next]
    mov edx,45*8
    call Zero
    xor r12d,r12d
.tit_i:
    lea r13d,[r12+1]
.tit_j:
    lea r14d,[r13+1]
.tit_k:
    lea rax,[rip+joint_c3]
    mov ebx,[rax+r14*4]
    lea rax,[rip+joint_c2]
    add ebx,[rax+r13*4]
    add ebx,r12d
    lea rax,[rip+ti_coef+2070*8]
    movsd xmm0,[rax+rbx*8]
    lea rax,[rip+ti_tensor_u]
    movsd xmm1,[rax+r12*8]
    movsd xmm2,[rax+r13*8]
    movsd xmm3,[rax+r14*8]
    lea rdx,[rip+ti_tensor_next]
    movapd xmm4,xmm0
    mulsd xmm4,xmm2
    mulsd xmm4,xmm3
    addsd xmm4,[rdx+r12*8]
    movsd [rdx+r12*8],xmm4
    movapd xmm4,xmm0
    mulsd xmm4,xmm1
    mulsd xmm4,xmm3
    addsd xmm4,[rdx+r13*8]
    movsd [rdx+r13*8],xmm4
    mulsd xmm0,xmm1
    mulsd xmm0,xmm2
    addsd xmm0,[rdx+r14*8]
    movsd [rdx+r14*8],xmm0
    inc r14d
    cmp r14d,[rip+joint_universe]
    jb .tit_k
    inc r13d
    mov eax,[rip+joint_universe]
    dec eax
    cmp r13d,eax
    jb .tit_j
    inc r12d
    dec eax
    cmp r12d,eax
    jb .tit_i
    xorpd xmm0,xmm0
    xor esi,esi
    lea rax,[rip+ti_tensor_next]
2:  movsd xmm1,[rax+rsi*8]
    mulsd xmm1,xmm1
    addsd xmm0,xmm1
    inc esi
    cmp esi,[rip+joint_universe]
    jb 2b
    maxsd xmm0,[rip+ti_small]
    sqrtsd xmm0,xmm0
    xor esi,esi
    lea rdx,[rip+ti_tensor_u]
3:  movsd xmm1,[rax+rsi*8]
    divsd xmm1,xmm0
    movsd [rdx+rsi*8],xmm1
    inc esi
    cmp esi,[rip+joint_universe]
    jb 3b
    dec ebp
    jnz .tit_iter
    # Two passes: calculate amplitude, then materialize the triple coefficients.
    xor ebp,ebp
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],0
.tit_pass:
    xor r12d,r12d
.tit_ai:
    lea r13d,[r12+1]
.tit_aj:
    lea r14d,[r13+1]
.tit_ak:
    lea rax,[rip+joint_c3]
    mov ebx,[rax+r14*4]
    lea rax,[rip+joint_c2]
    add ebx,[rax+r13*4]
    add ebx,r12d
    lea rax,[rip+ti_tensor_u]
    movsd xmm0,[rax+r12*8]
    mulsd xmm0,[rax+r13*8]
    mulsd xmm0,[rax+r14*8]
    test ebp,ebp
    jnz .tit_store
    movapd xmm1,xmm0
    mulsd xmm1,xmm1
    addsd xmm1,[rsp+40]
    movsd [rsp+40],xmm1
    lea rax,[rip+ti_coef+2070*8]
    mulsd xmm0,[rax+rbx*8]
    addsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
    jmp .tit_anext
.tit_store:
    mulsd xmm0,[rsp+48]
    minsd xmm0,[rip+joint_caps+16]
    movsd xmm1,[rip+joint_caps+16]
    xorpd xmm1,[rip+sign_bits]
    maxsd xmm0,xmm1
    lea rax,[rip+ti_coef+JOINT_BYTES+2070*8]
    movsd [rax+rbx*8],xmm0
.tit_anext:
    inc r14d
    cmp r14d,[rip+joint_universe]
    jb .tit_ak
    inc r13d
    mov eax,[rip+joint_universe]
    dec eax
    cmp r13d,eax
    jb .tit_aj
    inc r12d
    dec eax
    cmp r12d,eax
    jb .tit_ai
    test ebp,ebp
    jnz .tit_out
    movsd xmm0,[rsp+32]
    movsd xmm1,[rsp+40]
    maxsd xmm1,[rip+ti_small]
    divsd xmm0,xmm1
    movsd [rsp+48],xmm0
    inc ebp
    jmp .tit_pass
.tit_out: END 64

# Exact beta-Bernoulli changepoint posterior for the parity of the number sum.
# Each split (including no change at zero) has equal prior probability. Marginal
# likelihood B(k+1,n-k+1) integrates the segment's Bernoulli parameter.
FN TiChangepoint,64
    lea rcx,[rip+ti_coef+2*JOINT_BYTES]
    lea rdx,[rip+ti_coef]
    mov r8d,JOINT_BYTES
    call Copy
    mov qword ptr [rip+ti_logfact],0
    mov dword ptr [rip+ti_cp_prefix],0
    mov esi,1
1:  cvtsi2sd xmm0,esi
    call Log
    lea rax,[rip+ti_logfact]
    addsd xmm0,[rax+rsi*8-8]
    movsd [rax+rsi*8],xmm0
    inc esi
    mov eax,[rip+ti_n]
    inc eax
    cmp esi,eax
    jbe 1b
    xor esi,esi
2:  imul eax,esi,48
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    # Bits at zero-based even positions correspond to odd ball numbers.
    mov rax,0x155555555555
    and rcx,rax
    call PopCount64
    and eax,1
    lea rdx,[rip+ti_cp_prefix]
    add eax,[rdx+rsi*4]
    mov [rdx+rsi*4+4],eax
    inc esi
    cmp esi,[rip+ti_n]
    jb 2b
    mov qword ptr [rsp+32],0
    xor esi,esi
    # Stabilize log weights by their maximum before exponentiating.
    movsd xmm0,[rip+research_large]
    xorpd xmm0,[rip+sign_bits]
    movsd [rsp+40],xmm0
3:  lea rbx,[rip+ti_cp_prefix]
    mov ecx,esi
    mov edx,[rbx+rsi*4]
    call TiBetaLog
    movsd [rsp+48],xmm0
    mov ecx,[rip+ti_n]
    mov edx,[rbx+rcx*4]
    sub edx,[rbx+rsi*4]
    sub ecx,esi
    call TiBetaLog
    addsd xmm0,[rsp+48]
    lea rax,[rip+ti_cp_prob]
    movsd [rax+rsi*8],xmm0
    movsd xmm1,[rsp+40]
    maxsd xmm1,xmm0
    movsd [rsp+40],xmm1
    inc esi
    cmp esi,[rip+ti_n]
    jb 3b
    xor esi,esi
4:  lea rbx,[rip+ti_cp_prob]
    movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rsp+40]
    call Exp
    movsd [rbx+rsi*8],xmm0
    addsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
    inc esi
    cmp esi,[rip+ti_n]
    jb 4b
    xor esi,esi
5:  movsd xmm0,[rbx+rsi*8]
    divsd xmm0,[rsp+32]
    movsd [rbx+rsi*8],xmm0
    inc esi
    cmp esi,[rip+ti_n]
    jb 5b
    lea rcx,[rip+ti_counts]
    mov edx,45*8
    call Zero
    # Averaging suffix rates over all possible change locations.
    xor esi,esi
    mov qword ptr [rsp+56],0
6:  mov eax,[rip+ti_n]
    sub eax,esi
    cvtsi2sd xmm1,eax
    movsd xmm0,[rbx+rsi*8]
    divsd xmm0,xmm1
    addsd xmm0,[rsp+56]
    movsd [rsp+56],xmm0
    imul eax,esi,48
    lea rdx,[rip+history]
    mov rax,[rdx+rax+40]
    lea rdx,[rip+ti_counts]
7:  bsf rcx,rax
    btr rax,rcx
    movsd xmm1,[rdx+rcx*8]
    addsd xmm1,xmm0
    movsd [rdx+rcx*8],xmm1
    test rax,rax
    jnz 7b
    inc esi
    cmp esi,[rip+ti_n]
    jb 6b
    mov ecx,2
    lea rdx,[rip+ti_counts]
    call TiRatesToCoef
    END 64
FN TiBetaLog
    lea rax,[rip+ti_logfact]
    movsd xmm0,[rax+rdx*8]
    mov r8d,ecx
    sub r8d,edx
    addsd xmm0,[rax+r8*8]
    subsd xmm0,[rax+rcx*8+8]
    END
# ecx=family; rdx=45 fitted marginal rates. Bounded residual coefficients.
FN TiRatesToCoef
    imul ecx,JOINT_BYTES
    lea rax,[rip+ti_coef]
    add rax,rcx
    cvtsi2sd xmm3,dword ptr [rip+joint_universe]
    mov ecx,6
    cvtsi2sd xmm2,ecx
    divsd xmm2,xmm3
    xor ecx,ecx
1:  movsd xmm0,[rdx+rcx*8]
    subsd xmm0,xmm2
    mulsd xmm0,[rip+half]
    minsd xmm0,[rip+ti_cap]
    maxsd xmm0,[rip+ti_negcap]
    movsd [rax+rcx*8],xmm0
    inc ecx
    cmp ecx,[rip+joint_universe]
    jb 1b
    END
# Marginal linear Gaussian working-state filter, Q=1e-4, R=p(1-p).
# Final combination probabilities still use exact fixed-cardinality normalization.
FN TiDynamic
    lea rcx,[rip+ti_coef+3*JOINT_BYTES]
    lea rdx,[rip+ti_coef]
    mov r8d,JOINT_BYTES
    call Copy
    mov eax,6
    cvtsi2sd xmm0,eax
    cvtsi2sd xmm1,dword ptr [rip+joint_universe]
    divsd xmm0,xmm1
    movsd xmm4,[rip+one]
    subsd xmm4,xmm0
    mulsd xmm4,xmm0
    lea r8,[rip+ti_state]
    lea r9,[rip+ti_state_var]
    xor ecx,ecx
1:  movsd [r8+rcx*8],xmm0
    movsd [r9+rcx*8],xmm4
    inc ecx
    cmp ecx,[rip+joint_universe]
    jb 1b
    xor esi,esi
2:  imul eax,esi,48
    lea rdx,[rip+history]
    mov rbx,[rdx+rax+40]
    xor ecx,ecx
3:  movsd xmm1,[r9+rcx*8]
    addsd xmm1,[rip+ti_q]
    movapd xmm2,xmm1
    addsd xmm2,xmm4
    divsd xmm1,xmm2
    xor eax,eax
    bt rbx,rcx
    setc al
    cvtsi2sd xmm0,eax
    subsd xmm0,[r8+rcx*8]
    mulsd xmm0,xmm1
    addsd xmm0,[r8+rcx*8]
    movsd [r8+rcx*8],xmm0
    mulsd xmm1,xmm4
    movsd [r9+rcx*8],xmm1
    inc ecx
    cmp ecx,[rip+joint_universe]
    jb 3b
    inc esi
    cmp esi,[rip+ti_n]
    jb 2b
    mov ecx,3
    lea rdx,[rip+ti_state]
    call TiRatesToCoef
    END

# Fit all four bases and all finite posterior support points using prefix [0,n).
FN TiFit,64
    mov [rip+ti_n],ecx
    xor edx,edx
    call JointFit
    test eax,eax
    jz .tif_fail
    lea rcx,[rip+ti_coef]
    lea rdx,[rip+joint_coef]
    mov r8d,JOINT_BYTES
    call Copy
    call TiTensor
    call TiChangepoint
    call TiDynamic
    mov ecx,[rip+joint_universe]
    mov edx,6
    call Choose
    mov [rip+ti_space],rax
    xor r12d,r12d
.tif_family:
    cmp dword ptr [rip+cancel_flag],0
    jne .tif_fail
    lea rcx,[rip+ti_z]
    mov edx,TI_GRID*8
    call Zero
    mov qword ptr [rsp+32],0
    xor esi,esi
.tif_data:
    imul eax,esi,48
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    mov edx,r12d
    call TiEnergy
    addsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
    inc esi
    cmp esi,[rip+ti_n]
    jb .tif_data
    mov ebx,63
    mov r15,1
    mov ecx,[rip+joint_universe]
    shl r15,cl
    xor r14d,r14d
    mov qword ptr [rip+ti_enum_count],0
    call TiBounds
.tif_enum:
    cmp dword ptr [rip+cancel_flag],0
    jne .tif_fail
    mov rcx,rbx
    mov edx,r12d
    call TiEnergy
    movsd [rsp+40],xmm0
    xor esi,esi
.tif_grid:
    lea rax,[rip+ti_scales]
    movsd xmm0,[rax+rsi*8]
    mulsd xmm0,[rsp+40]
    call Exp
    lea rax,[rip+ti_z]
    addsd xmm0,[rax+rsi*8]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb .tif_grid
    inc r14
    test r14d,0xfffff
    jnz .tif_successor
    mov [rip+ti_enum_count],r14
    call TiBounds
.tif_successor:
    # Gosper successor enumerates each six-subset exactly once.
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
    jb .tif_enum
    mov [rip+ti_space],r14
    cvtsi2sd xmm0,r14
    call Log
    movsd [rip+ti_logspace],xmm0
    mov [rip+ti_enum_count],r14
    call TiBounds
    xor esi,esi
    movsd xmm0,[rip+research_large]
    xorpd xmm0,[rip+sign_bits]
    movsd [rsp+48],xmm0
.tif_posterior:
    lea rax,[rip+ti_z]
    movsd xmm0,[rax+rsi*8]
    call Log
    imul ebx,r12d,TI_GRID
    add ebx,esi
    lea rax,[rip+ti_logz]
    movsd [rax+rbx*8],xmm0
    cvtsi2sd xmm1,dword ptr [rip+ti_n]
    mulsd xmm0,xmm1
    lea rax,[rip+ti_scales]
    movsd xmm2,[rax+rsi*8]
    movapd xmm1,xmm2
    mulsd xmm1,[rsp+32]
    subsd xmm1,xmm0
    # Discrete Gaussian prior on scale: exp(-scale^2/2).
    mulsd xmm2,xmm2
    mulsd xmm2,[rip+half]
    subsd xmm1,xmm2
    lea rax,[rip+ti_post]
    movsd [rax+rbx*8],xmm1
    movsd xmm0,[rsp+48]
    maxsd xmm0,xmm1
    movsd [rsp+48],xmm0
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb .tif_posterior
    xor esi,esi
    mov qword ptr [rsp+56],0
.tif_exp:
    imul ebx,r12d,TI_GRID
    add ebx,esi
    lea rdi,[rip+ti_post]
    movsd xmm0,[rdi+rbx*8]
    subsd xmm0,[rsp+48]
    call Exp
    movsd [rdi+rbx*8],xmm0
    addsd xmm0,[rsp+56]
    movsd [rsp+56],xmm0
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb .tif_exp
    xor esi,esi
.tif_norm:
    imul ebx,r12d,TI_GRID
    add ebx,esi
    movsd xmm0,[rdi+rbx*8]
    divsd xmm0,[rsp+56]
    movsd [rdi+rbx*8],xmm0
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb .tif_norm
    inc r12d
    cmp r12d,TI_FAMILIES
    jb .tif_family
    mov eax,1
    jmp .tif_out
.tif_fail: xor eax,eax
.tif_out: END 64

# Enclose the unexplored partition's mass using |E|<=.8 (scale=1).
# Repeated exact enumeration removes terms from the remainder. The final
# interval is expanded for floating error; it is not a certified interval exp.
FN TiBounds,16
    mov rax,[rip+ti_space]
    sub rax,[rip+ti_enum_count]
    cvtsi2sd xmm0,rax
    movsd [rsp+32],xmm0
    movsd xmm0,[rip+joint_energy_bound]
    xorpd xmm0,[rip+sign_bits]
    call Exp
    mulsd xmm0,[rsp+32]
    addsd xmm0,[rip+ti_z+16]
    movsd xmm1,[rip+one]
    subsd xmm1,[rip+ti_margin]
    mulsd xmm0,xmm1
    movsd [rip+ti_z_lower],xmm0
    movsd xmm0,[rip+joint_energy_bound]
    call Exp
    mulsd xmm0,[rsp+32]
    addsd xmm0,[rip+ti_z+16]
    movsd xmm1,[rip+one]
    addsd xmm1,[rip+ti_margin]
    mulsd xmm0,xmm1
    movsd [rip+ti_z_upper],xmm0
    END 16

# Family posterior predictive, normalized on full six-subset support.
FN TiFamilyProbability,32
    mov ebp,edx
    call TiEnergy
    movsd [rsp+32],xmm0
    mov qword ptr [rsp+40],0
    xor esi,esi
1:  lea rax,[rip+ti_scales]
    movsd xmm0,[rax+rsi*8]
    mulsd xmm0,[rsp+32]
    imul ebx,ebp,TI_GRID
    add ebx,esi
    lea rax,[rip+ti_logz]
    subsd xmm0,[rax+rbx*8]
    call Exp
    lea rax,[rip+ti_post]
    mulsd xmm0,[rax+rbx*8]
    addsd xmm0,[rsp+40]
    movsd [rsp+40],xmm0
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb 1b
    END 32
FN TiPredict
    mov r12,rcx
    xor ebp,ebp
1:  mov rcx,r12
    mov edx,ebp
    call TiFamilyProbability
    lea rax,[rip+ti_pred]
    movsd [rax+rbp*8],xmm0
    inc ebp
    cmp ebp,TI_FAMILIES
    jb 1b
    movsd xmm0,[rip+one]
    cvtsi2sd xmm1,qword ptr [rip+ti_space]
    divsd xmm0,xmm1
    movsd [rip+ti_pred+32],xmm0
    END

# Exponentiated-gradient simplex optimization. First ordinary chronological
# stacking, then entropic/KL-penalized worst-case weighting of validation rows.
FN TiStack,64
    lea rax,[rip+ti_weights]
    movsd xmm0,[rip+ti_prior]
    xor ecx,ecx
1:  movsd [rax+rcx*8],xmm0
    inc ecx
    cmp ecx,5
    jb 1b
    cmp dword ptr [rip+ti_folds],2
    jb .tis_out
    xor r12d,r12d
.tis_iter:
    lea rcx,[rip+ti_gradient]
    mov edx,5*8
    call Zero
    mov qword ptr [rsp+32],0
    xor esi,esi
.tis_loss:
    imul ebx,esi,40
    lea rdi,[rip+ti_rows]
    add rdi,rbx
    lea rax,[rip+ti_weights]
    xorpd xmm0,xmm0
    xor ecx,ecx
2:  movsd xmm1,[rdi+rcx*8]
    mulsd xmm1,[rax+rcx*8]
    addsd xmm0,xmm1
    inc ecx
    cmp ecx,5
    jb 2b
    movsd [rsp+40],xmm0
    movsd xmm1,[rip+one]
    cmp r12d,40
    jb 3f
    # exp((loss-log(space))/tau), tau=1; stable probability floor from uniform.
    mulsd xmm0,[rip+ti_space_double]
    divsd xmm1,xmm0
3:  lea rax,[rip+ti_losses]
    movsd [rax+rsi*8],xmm1
    addsd xmm1,[rsp+32]
    movsd [rsp+32],xmm1
    inc esi
    mov eax,[rip+ti_folds]
    dec eax
    cmp esi,eax
    jb .tis_loss
    xor esi,esi
.tis_grad:
    imul ebx,esi,40
    lea rdi,[rip+ti_rows]
    add rdi,rbx
    lea rax,[rip+ti_weights]
    xorpd xmm0,xmm0
    xor ecx,ecx
4:  movsd xmm1,[rdi+rcx*8]
    mulsd xmm1,[rax+rcx*8]
    addsd xmm0,xmm1
    inc ecx
    cmp ecx,5
    jb 4b
    lea rax,[rip+ti_losses]
    movsd xmm2,[rax+rsi*8]
    divsd xmm2,[rsp+32]
    divsd xmm2,xmm0
    lea rax,[rip+ti_gradient]
    xor ecx,ecx
5:  movsd xmm1,[rdi+rcx*8]
    mulsd xmm1,xmm2
    addsd xmm1,[rax+rcx*8]
    movsd [rax+rcx*8],xmm1
    inc ecx
    cmp ecx,5
    jb 5b
    inc esi
    mov eax,[rip+ti_folds]
    dec eax
    cmp esi,eax
    jb .tis_grad
    mov qword ptr [rsp+48],0
    xor esi,esi
6:  lea rax,[rip+ti_gradient]
    movsd xmm0,[rax+rsi*8]
    mulsd xmm0,[rip+ti_rate]
    call Exp
    lea rax,[rip+ti_weights]
    mulsd xmm0,[rax+rsi*8]
    maxsd xmm0,[rip+ti_small]
    movsd [rax+rsi*8],xmm0
    addsd xmm0,[rsp+48]
    movsd [rsp+48],xmm0
    inc esi
    cmp esi,5
    jb 6b
    xor esi,esi
7:  movsd xmm0,[rax+rsi*8]
    divsd xmm0,[rsp+48]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,5
    jb 7b
    inc r12d
    cmp r12d,80
    jb .tis_iter
.tis_out: END 64

.section .bss
ti_space_double: .skip 8
.section .text
# Last chronological fold is untouched by weight fitting (outer confirmation).
# On n=60 all ten computations still run, but there is no out-of-sample evidence.
FN TiValidate
    mov eax,[rip+ti_real_n]
    sub eax,60
    cmp eax,[rip+ti_fold_limit]
    jbe 1f
    mov eax,[rip+ti_fold_limit]
1:  mov [rip+ti_folds],eax
    mov qword ptr [rip+ti_gain],0
    xor esi,esi
.tiv_fold:
    cmp esi,[rip+ti_folds]
    jae .tiv_stack
    mov ecx,[rip+ti_real_n]
    sub ecx,[rip+ti_folds]
    add ecx,esi
    mov ebp,ecx
    call TiFit
    test eax,eax
    jz .tiv_fail
    cvtsi2sd xmm0,qword ptr [rip+ti_space]
    movsd [rip+ti_space_double],xmm0
    imul eax,ebp,48
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    call TiPredict
    imul eax,esi,40
    lea rcx,[rip+ti_rows]
    add rcx,rax
    lea rdx,[rip+ti_pred]
    mov r8d,40
    call Copy
    inc esi
    jmp .tiv_fold
.tiv_stack:
    call TiStack
    cmp dword ptr [rip+ti_folds],0
    je .tiv_success
    mov eax,[rip+ti_folds]
    dec eax
    imul eax,40
    lea rdx,[rip+ti_rows]
    add rdx,rax
    lea rax,[rip+ti_weights]
    xor ecx,ecx
    xorpd xmm0,xmm0
2:  movsd xmm1,[rdx+rcx*8]
    mulsd xmm1,[rax+rcx*8]
    addsd xmm0,xmm1
    inc ecx
    cmp ecx,5
    jb 2b
    call Log
    addsd xmm0,[rip+ti_logspace]
    movsd [rip+ti_gain],xmm0
.tiv_success: mov eax,1; jmp .tiv_out
.tiv_fail: xor eax,eax
.tiv_out: END

# Actual AIS: uniform start; each intermediate independent-MH kernel leaves
# exp(beta*E) invariant. Compare mean importance weight against exact Z/N.
FN TiAIS,64
    mov qword ptr [rip+ti_ais_sum],0
    xor r12d,r12d
.tia_run:
    call TiUniform
    test rax,rax
    jz .tia_fail
    mov rbx,rax
    mov rcx,rax
    mov edx,3
    call TiEnergy
    movsd [rsp+32],xmm0
    mov qword ptr [rsp+40],0
    mov esi,1
.tia_stage:
    cmp dword ptr [rip+cancel_flag],0
    jne .tia_fail
    cvtsi2sd xmm1,dword ptr [rip+ti_ais_steps]
    movsd xmm0,[rsp+32]
    divsd xmm0,xmm1
    addsd xmm0,[rsp+40]
    movsd [rsp+40],xmm0
    call TiUniform
    test rax,rax
    jz .tia_fail
    mov rdi,rax
    mov rcx,rax
    mov edx,3
    call TiEnergy
    movsd [rsp+48],xmm0
    subsd xmm0,[rsp+32]
    cvtsi2sd xmm1,esi
    mulsd xmm0,xmm1
    cvtsi2sd xmm1,dword ptr [rip+ti_ais_steps]
    divsd xmm0,xmm1
    minsd xmm0,[rip+zero]
    call Exp
    movq rbp,xmm0
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .tia_fail
    movq xmm1,rbp
    comisd xmm0,xmm1
    jae 1f
    mov rbx,rdi
    movsd xmm0,[rsp+48]
    movsd [rsp+32],xmm0
1:  inc esi
    cmp esi,[rip+ti_ais_steps]
    jbe .tia_stage
    movsd xmm0,[rsp+40]
    call Exp
    addsd xmm0,[rip+ti_ais_sum]
    movsd [rip+ti_ais_sum],xmm0
    inc r12d
    cmp r12d,[rip+ti_ais_runs]
    jb .tia_run
    movsd xmm0,[rip+ti_logz+(3*TI_GRID+2)*8]
    subsd xmm0,[rip+ti_logspace]
    call Exp
    movsd xmm1,[rip+ti_ais_sum]
    cvtsi2sd xmm2,dword ptr [rip+ti_ais_runs]
    divsd xmm1,xmm2
    divsd xmm1,xmm0
    subsd xmm1,[rip+one]
    andpd xmm1,[rip+abs_bits]
    movsd [rip+ti_ais_error],xmm1
    mov eax,1
    jmp .tia_out
.tia_fail: xor eax,eax
.tia_out: END 64

# Transactional orchestration: history masks and legacy joint state restored
# on success, cancellation and RNG failure. Null histories refit every family,
# posterior grid, chronological stack and robust objective from scratch.
FN IntegratedAnalyze
    mov ebp,ecx
    lea rcx,[rip+ti_begin]
    mov edx,TI_BYTES
    call Zero
    mov [rip+ti_profile],ebp
    mov eax,[rip+history_count]
    cmp eax,60
    jb .tin_early_fail
    mov [rip+ti_real_n],eax
    mov dword ptr [rip+ti_grid_count],3
    mov dword ptr [rip+ti_fold_limit],2
    mov dword ptr [rip+ti_null_limit],2
    mov dword ptr [rip+ti_ais_runs],8
    mov dword ptr [rip+ti_ais_steps],8
    test ebp,ebp
    jz 1f
    mov dword ptr [rip+ti_grid_count],5
    mov dword ptr [rip+ti_fold_limit],8
    mov dword ptr [rip+ti_null_limit],8
    mov dword ptr [rip+ti_ais_runs],64
    mov dword ptr [rip+ti_ais_steps],32
1:  lea rcx,[rip+ti_saved_joint]
    lea rdx,[rip+joint_coef]
    mov r8d,JOINT_BYTES
    call Copy
    lea rcx,[rip+ti_saved_moments]
    lea rdx,[rip+joint_moment]
    mov r8d,JOINT_BYTES
    call Copy
    mov rax,[rip+joint_logz]
    mov [rip+ti_saved_logz],rax
    lea rcx,[rip+ti_saved_joint_state]
    lea rdx,[rip+joint_coef]
    mov r8d,TI_JOINT_STATE_BYTES
    call Copy
    mov eax,[rip+joint_order]
    mov [rip+ti_saved_order],eax
    mov rax,[rip+joint_shrink]
    mov [rip+ti_saved_shrink],rax
    mov dword ptr [rip+joint_order],3
    movsd xmm0,[rip+one]
    movsd [rip+joint_shrink],xmm0
    xor esi,esi
2:  imul eax,esi,48
    lea rdx,[rip+history]
    mov rax,[rdx+rax+40]
    lea rdx,[rip+ti_history]
    mov [rdx+rsi*8],rax
    inc esi
    cmp esi,[rip+ti_real_n]
    jb 2b
    mov dword ptr [rip+analysis_stage],14
    mov dword ptr [rip+progress],0
    mov dword ptr [rip+rng_mode],0
    mov dword ptr [rip+rng_error],0
    call TiValidate
    test eax,eax
    jz .tin_fail
    movsd xmm0,[rip+ti_gain]
    movsd [rip+ti_real_gain],xmm0
    xor r12d,r12d
    xor r13d,r13d
.tin_null:
    mov dword ptr [rip+analysis_stage],15
    xor esi,esi
3:  call TiUniform
    test rax,rax
    jz .tin_fail
    imul ecx,esi,48
    lea rdx,[rip+history]
    mov [rdx+rcx+40],rax
    inc esi
    cmp esi,[rip+ti_real_n]
    jb 3b
    call TiValidate
    test eax,eax
    jz .tin_fail
    # With no folds, still perform a complete fit instead of a nominal null loop.
    cmp dword ptr [rip+ti_folds],0
    jne 4f
    mov ecx,[rip+ti_real_n]
    call TiFit
    test eax,eax
    jz .tin_fail
4:  movsd xmm0,[rip+ti_gain]
    comisd xmm0,[rip+ti_real_gain]
    jb 5f
    inc r13d
5:  inc r12d
    mov [rip+ti_null_done],r12d
    imul eax,r12d,80
    xor edx,edx
    div dword ptr [rip+ti_null_limit]
    mov [rip+progress],eax
    cmp r12d,[rip+ti_null_limit]
    jb .tin_null
    inc r13d
    inc r12d
    cvtsi2sd xmm0,r13d
    cvtsi2sd xmm1,r12d
    divsd xmm0,xmm1
    movsd [rip+ti_null_p],xmm0
    call TiRestoreHistory
    # Refit original chronological weights after synthetic worlds overwrite them.
    mov dword ptr [rip+analysis_stage],16
    call TiValidate
    test eax,eax
    jz .tin_fail
    mov ecx,[rip+ti_real_n]
    call TiFit
    test eax,eax
    jz .tin_fail
    call TiAIS
    test eax,eax
    jz .tin_fail
    movsd xmm0,[rip+half]
    cmp dword ptr [rip+ti_profile],0
    jne 6f
    # Normal mode cannot revive evidence closed by the original gate.
    movsd xmm0,[rip+one]
    subsd xmm0,[rip+rho]
    minsd xmm0,[rip+half]
    movsd xmm1,[rip+one]
    subsd xmm1,[rip+ti_null_p]
    mulsd xmm0,xmm1
    cmp dword ptr [rip+ti_folds],2
    jb .tin_no_support
    movsd xmm1,[rip+ti_gain]
    comisd xmm1,[rip+zero]
    ja 6f
.tin_no_support: xorpd xmm0,xmm0
6:  movsd [rip+ti_alpha],xmm0
    mov dword ptr [rip+ti_completed],1023
    mov dword ptr [rip+ti_ready],1
    mov dword ptr [rip+progress],100
    mov ebp,1
    jmp .tin_restore
.tin_fail:
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+field_ready],0
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+advanced_ready],0
    xor ebp,ebp
.tin_restore:
    call TiRestoreHistory
    lea rcx,[rip+joint_coef]
    lea rdx,[rip+ti_saved_joint]
    mov r8d,JOINT_BYTES
    call Copy
    lea rcx,[rip+joint_moment]
    lea rdx,[rip+ti_saved_moments]
    mov r8d,JOINT_BYTES
    call Copy
    mov rax,[rip+ti_saved_logz]
    mov [rip+joint_logz],rax
    lea rcx,[rip+joint_coef]
    lea rdx,[rip+ti_saved_joint_state]
    mov r8d,TI_JOINT_STATE_BYTES
    call Copy
    mov eax,[rip+ti_saved_order]
    mov [rip+joint_order],eax
    mov rax,[rip+ti_saved_shrink]
    mov [rip+joint_shrink],rax
    mov eax,ebp
    jmp .tin_out
.tin_early_fail: xor eax,eax
.tin_out: END
FN TiRestoreHistory
    xor ecx,ecx
    lea rdx,[rip+ti_history]
    lea r8,[rip+history]
1:  mov rax,[rdx+rcx*8]
    imul r9d,ecx,48
    mov [r8+r9+40],rax
    inc ecx
    cmp ecx,[rip+ti_real_n]
    jb 1b
    END

# Mixture draw: select family and finite posterior scale, then exact rejection
# with global energy bound 2*.8=1.6. Exceeding the cap fails, never biases fallback.
FN IntegratedSample
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .tip_fail
    comisd xmm0,[rip+ti_alpha]
    jb .tip_integrated
    cmp dword ptr [rip+ti_profile],0
    jne .tip_uniform
    call SampleMask
    jmp .tip_out
.tip_uniform:
    call TiUniform
    jmp .tip_out
.tip_integrated:
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .tip_fail
    xor ebp,ebp
    lea rax,[rip+ti_weights]
1:  subsd xmm0,[rax+rbp*8]
    comisd xmm0,[rip+zero]
    jb 2f
    inc ebp
    cmp ebp,4
    jb 1b
2:  cmp ebp,4
    je .tip_uniform
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .tip_fail
    xor esi,esi
    imul eax,ebp,TI_GRID*8
    lea rdx,[rip+ti_post]
    add rdx,rax
3:  subsd xmm0,[rdx+rsi*8]
    comisd xmm0,[rip+zero]
    jb 4f
    inc esi
    mov eax,[rip+ti_grid_count]
    dec eax
    cmp esi,eax
    jb 3b
4:  mov r14d,8192
.tip_try:
    call TiUniform
    test rax,rax
    jz .tip_fail
    mov rbx,rax
    mov rcx,rax
    mov edx,ebp
    call TiEnergy
    subsd xmm0,[rip+joint_energy_bound]
    lea rax,[rip+ti_scales]
    mulsd xmm0,[rax+rsi*8]
    call Exp
    movq r12,xmm0
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .tip_fail
    movq xmm1,r12
    comisd xmm0,xmm1
    jb .tip_accept
    dec r14d
    jnz .tip_try
.tip_fail: xor eax,eax; jmp .tip_out
.tip_accept: mov rax,rbx
.tip_out: END

FN IntegratedReport
    cmp dword ptr [rip+ti_ready],1
    jne .tir_out
    lea rcx,[rip+ti_report_title]
    call Text
    mov ecx,[rip+ti_profile]
    call Number
    call Comma
    mov ecx,[rip+ti_completed]
    call Number
    call Comma
    mov ecx,[rip+ti_folds]
    call Number
    call Comma
    mov ecx,[rip+ti_null_done]
    call Number
    call Comma
    movsd xmm0,[rip+ti_null_p]
    call Decimal
    call Comma
    movsd xmm0,[rip+ti_gain]
    call Decimal
    call Comma
    movsd xmm0,[rip+ti_ais_error]
    call Decimal
    call Comma
    movsd xmm0,[rip+ti_z_lower]
    call Decimal
    call Comma
    movsd xmm0,[rip+ti_z_upper]
    call Decimal
    lea rcx,[rip+ti_report_weights]
    call Text
    xor ebp,ebp
.tir_family:
    mov ecx,ebp
    call Number
    call Comma
    lea rax,[rip+ti_weights]
    movsd xmm0,[rax+rbp*8]
    call Decimal
    cmp ebp,4
    je .tir_family_end
    xor esi,esi
.tir_scale:
    call Comma
    imul eax,ebp,TI_GRID
    add eax,esi
    lea rdx,[rip+ti_post]
    movsd xmm0,[rdx+rax*8]
    call Decimal
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb .tir_scale
.tir_family_end:
    call Newline
    inc ebp
    cmp ebp,5
    jb .tir_family
    lea rcx,[rip+ti_report_limits]
    call Text
.tir_out: END

# Actual single-game law, including the legacy field policy in normal mode.
FN IntegratedProbability,32
    mov r12,rcx
    call TiPredict
    lea rax,[rip+ti_pred]
    lea rdx,[rip+ti_weights]
    xorpd xmm0,xmm0
    xor ecx,ecx
1:  movsd xmm1,[rax+rcx*8]
    mulsd xmm1,[rdx+rcx*8]
    addsd xmm0,xmm1
    inc ecx
    cmp ecx,5
    jb 1b
    mulsd xmm0,[rip+ti_alpha]
    movsd [rsp+32],xmm0
    movsd xmm0,[rip+one]
    cvtsi2sd xmm1,qword ptr [rip+ti_space]
    divsd xmm0,xmm1
    cmp dword ptr [rip+ti_profile],0
    jne 4f
    cmp dword ptr [rip+field_ready],1
    jne 4f
    mulsd xmm0,[rip+rho]
    xor ecx,ecx
    lea rax,[rip+final_masks]
2:  cmp ecx,[rip+field_count]
    jae 4f
    cmp r12,[rax+rcx*8]
    je 3f
    inc ecx
    jmp 2b
3:  lea rax,[rip+final_prob]
    movsd xmm1,[rip+one]
    subsd xmm1,[rip+rho]
    mulsd xmm1,[rax+rcx*8]
    addsd xmm0,xmm1
4:  movsd xmm1,[rip+one]
    subsd xmm1,[rip+ti_alpha]
    mulsd xmm0,xmm1
    addsd xmm0,[rsp+32]
    END 32
