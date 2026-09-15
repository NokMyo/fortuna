# Depth validation 1.0. Microsoft x64 ABI, SSE2, no foreign runtime.
# Working marginal models are not a claim of physical lottery predictability.
.section .rdata,"dr"
depth_horizons: .double 16,64,256,0
depth_prior: .double 60
depth_quarter: .double 0.25
depth_five: .double 5
depth_tolerance: .double 0.0000000001
depth_norm_margin: .double 1.000001
depth_threshold: .double 0.05
depth_title: .asciz "\r\nDEPTH VALIDATION 1.0\r\nfolds,bootstrap_replicates,graph_iterations,graph_residual,anytime_adjusted_p,online_log_e,bootstrap_mean_low,bootstrap_mean_high,loss_CUSUM,outer_ablation_max_change\r\n"
depth_moment_title: .asciz "\r\nfamily,scale_index,energy_mean,Fisher_energy_variance\r\n"
depth_limits: .asciz "Bootstrap is an approximate stationary-block percentile interval, available only with >=8 causal rows; it does not refit models. E-processes compare prefix predictions with independent uniform draws within this run/window, not persistent prospective evidence. Discounted marginal likelihood is a working composite score. Graph smoothing solves (I+L)x=b. Numerical checks are not certified interval arithmetic.\r\n"
.section .bss
.p2align 4
depth_result_begin:
depth_boot_done: .skip 4
depth_graph_iterations: .skip 4
depth_graph_residual: .skip 8
depth_anytime_p: .skip 8
depth_online_loge: .skip 8
depth_boot_low: .skip 8
depth_boot_high: .skip 8
depth_cusum: .skip 8
depth_ablation: .skip 8
depth_result_end:
.equ DEPTH_RESULT_BYTES,depth_result_end-depth_result_begin
depth_decay: .skip 4*8
depth_mass: .skip 4*8
depth_counts: .skip 4*45*8
depth_scores: .skip 4*8
depth_scale_weights: .skip 4*8
depth_raw_rates: .skip 45*8
depth_rates: .skip 45*8
depth_graph: .skip 45*45*8
depth_diag: .skip 45*8
depth_x: .skip 45*8
depth_next: .skip 45*8
depth_p: .skip 8
depth_z_comp: .skip 5*8
depth_m1: .skip 5*8
depth_m2: .skip 5*8
depth_m1_comp: .skip 5*8
depth_m2_comp: .skip 5*8
depth_means: .skip 20*8
depth_variances: .skip 20*8
depth_online_weights: .skip 5*8
depth_predict_weights: .skip 16*5*8
depth_loge: .skip 5*8
depth_gains: .skip 16*8
depth_bootstrap: .skip 4096*8
.section .text

# Four discounted beta working filters. Score each row BEFORE incorporating it.
# Average log marginal likelihood over N balls tempers within-draw dependence.
FN DepthDynamic,64
    lea rcx,[rip+depth_counts]
    mov edx,4*45*8
    call Zero
    lea rcx,[rip+depth_mass]
    mov edx,4*8
    call Zero
    lea rcx,[rip+depth_scores]
    mov edx,4*8
    call Zero
    movsd xmm0,[rip+six]
    cvtsi2sd xmm1,dword ptr [rip+joint_universe]
    divsd xmm0,xmm1
    movsd [rip+depth_p],xmm0
    xor esi,esi
1:  movsd xmm0,[rip+one]
    cmp esi,3
    je 2f
    movsd xmm0,[rip+ln2]
    xorpd xmm0,[rip+sign_bits]
    lea rax,[rip+depth_horizons]
    divsd xmm0,[rax+rsi*8]
    call Exp
2:  lea rax,[rip+depth_decay]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,4
    jb 1b
    xor ebp,ebp
.dd_row:
    cmp dword ptr [rip+cancel_flag],0
    jne .dd_fail
    imul eax,ebp,48
    lea rdx,[rip+history]
    mov r15,[rdx+rax+40]
    xor r12d,r12d
.dd_scale:
    imul eax,r12d,45*8
    lea r14,[rip+depth_counts]
    add r14,rax
    lea rax,[rip+depth_mass]
    movsd xmm0,[rax+r12*8]
    addsd xmm0,[rip+depth_prior]
    movsd [rsp+32],xmm0
    mov qword ptr [rsp+40],0
    xor esi,esi
.dd_ball:
    movsd xmm0,[rip+depth_p]
    mulsd xmm0,[rip+depth_prior]
    addsd xmm0,[r14+rsi*8]
    divsd xmm0,[rsp+32]
    bt r15,rsi
    jc 3f
    movsd xmm1,[rip+one]
    subsd xmm1,xmm0
    movapd xmm0,xmm1
3:  call Log
    addsd xmm0,[rsp+40]
    movsd [rsp+40],xmm0
    # Discount old excess counts, then add the current observation.
    lea rax,[rip+depth_decay]
    movsd xmm0,[r14+rsi*8]
    mulsd xmm0,[rax+r12*8]
    bt r15,rsi
    jnc 4f
    addsd xmm0,[rip+one]
4:  movsd [r14+rsi*8],xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb .dd_ball
    movsd xmm0,[rsp+40]
    cvtsi2sd xmm1,dword ptr [rip+joint_universe]
    divsd xmm0,xmm1
    lea rax,[rip+depth_scores]
    addsd xmm0,[rax+r12*8]
    movsd [rax+r12*8],xmm0
    lea rax,[rip+depth_mass]
    lea rdx,[rip+depth_decay]
    movsd xmm0,[rax+r12*8]
    mulsd xmm0,[rdx+r12*8]
    addsd xmm0,[rip+one]
    movsd [rax+r12*8],xmm0
    inc r12d
    cmp r12d,4
    jb .dd_scale
    inc ebp
    cmp ebp,[rip+ti_n]
    jb .dd_row
    lea rax,[rip+depth_scores]
    movsd xmm0,[rax]
    xor esi,esi
5:  maxsd xmm0,[rax+rsi*8]
    inc esi
    cmp esi,4
    jb 5b
    movsd [rsp+32],xmm0
    mov qword ptr [rsp+40],0
    xor esi,esi
6:  lea rax,[rip+depth_scores]
    movsd xmm0,[rax+rsi*8]
    subsd xmm0,[rsp+32]
    call Exp
    lea rax,[rip+depth_scale_weights]
    movsd [rax+rsi*8],xmm0
    addsd xmm0,[rsp+40]
    movsd [rsp+40],xmm0
    inc esi
    cmp esi,4
    jb 6b
    xor esi,esi
7:  movsd xmm0,[rax+rsi*8]
    divsd xmm0,[rsp+40]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,4
    jb 7b
    xor esi,esi
.dd_rates:
    xorpd xmm3,xmm3
    xor edi,edi
8:  imul eax,edi,45
    add eax,esi
    lea rdx,[rip+depth_counts]
    movsd xmm0,[rip+depth_p]
    mulsd xmm0,[rip+depth_prior]
    addsd xmm0,[rdx+rax*8]
    lea rax,[rip+depth_mass]
    movsd xmm1,[rax+rdi*8]
    addsd xmm1,[rip+depth_prior]
    divsd xmm0,xmm1
    lea rax,[rip+depth_scale_weights]
    mulsd xmm0,[rax+rdi*8]
    addsd xmm3,xmm0
    inc edi
    cmp edi,4
    jb 8b
    lea rax,[rip+ti_state]
    addsd xmm3,[rax+rsi*8]
    mulsd xmm3,[rip+half]
    lea rax,[rip+depth_raw_rates]
    movsd [rax+rsi*8],xmm3
    inc esi
    cmp esi,[rip+joint_universe]
    jb .dd_rates
    call DepthGraph
    jmp .dd_out
.dd_fail: xor eax,eax
.dd_out: END 64

# Symmetric nonnegative graph from |pair coefficients|. Jacobi solves the
# strictly diagonally dominant SPD system (I+L)x=(rate-p). No sign edge claim.
FN DepthGraph,32
    xor esi,esi
.dg_i:
    movsd xmm3,[rip+one]
    xor edi,edi
.dg_j:
    xorpd xmm0,xmm0
    cmp esi,edi
    je 1f
    mov eax,esi
    mov edx,edi
    cmp eax,edx
    jb 2f
    xchg eax,edx
2:  imul eax,45
    add eax,edx
    lea rdx,[rip+ti_coef+45*8]
    movsd xmm0,[rdx+rax*8]
    andpd xmm0,[rip+abs_bits]
1:  imul eax,esi,45
    add eax,edi
    lea rdx,[rip+depth_graph]
    movsd [rdx+rax*8],xmm0
    addsd xmm3,xmm0
    inc edi
    cmp edi,[rip+joint_universe]
    jb .dg_j
    lea rax,[rip+depth_diag]
    movsd [rax+rsi*8],xmm3
    lea rax,[rip+depth_raw_rates]
    movsd xmm0,[rax+rsi*8]
    subsd xmm0,[rip+depth_p]
    lea rax,[rip+depth_x]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb .dg_i
    mov ebp,32
    cmp dword ptr [rip+ti_profile],0
    je 3f
    mov ebp,96
3:  mov [rip+depth_graph_iterations],ebp
.dg_iter:
    cmp dword ptr [rip+cancel_flag],0
    jne .dg_fail
    xor esi,esi
.dg_row:
    lea rax,[rip+depth_raw_rates]
    movsd xmm0,[rax+rsi*8]
    subsd xmm0,[rip+depth_p]
    imul eax,esi,45*8
    lea rdx,[rip+depth_graph]
    add rdx,rax
    lea rax,[rip+depth_x]
    xor edi,edi
4:  movsd xmm1,[rdx+rdi*8]
    mulsd xmm1,[rax+rdi*8]
    addsd xmm0,xmm1
    inc edi
    cmp edi,[rip+joint_universe]
    jb 4b
    lea rax,[rip+depth_diag]
    divsd xmm0,[rax+rsi*8]
    lea rax,[rip+depth_next]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb .dg_row
    lea rcx,[rip+depth_x]
    lea rdx,[rip+depth_next]
    mov r8d,45*8
    call Copy
    dec ebp
    jnz .dg_iter
    mov qword ptr [rip+depth_graph_residual],0
    xor esi,esi
.dg_check:
    lea rax,[rip+depth_diag]
    movsd xmm0,[rax+rsi*8]
    lea rax,[rip+depth_x]
    mulsd xmm0,[rax+rsi*8]
    movsd xmm2,[rax+rsi*8]
    addsd xmm2,[rip+depth_p]
    lea rdx,[rip+depth_rates]
    movsd [rdx+rsi*8],xmm2
    imul edx,esi,45*8
    lea r8,[rip+depth_graph]
    add r8,rdx
    xor edi,edi
5:  movsd xmm1,[r8+rdi*8]
    mulsd xmm1,[rax+rdi*8]
    subsd xmm0,xmm1
    inc edi
    cmp edi,[rip+joint_universe]
    jb 5b
    lea rax,[rip+depth_raw_rates]
    subsd xmm0,[rax+rsi*8]
    addsd xmm0,[rip+depth_p]
    andpd xmm0,[rip+abs_bits]
    ucomisd xmm0,xmm0
    jp .dg_fail
    maxsd xmm0,[rip+depth_graph_residual]
    movsd [rip+depth_graph_residual],xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb .dg_check
    comisd xmm0,[rip+depth_tolerance]
    ja .dg_fail
    mov eax,1
    jmp .dg_out
.dg_fail: xor eax,eax
.dg_out: END 32

FN DepthMomentsReset
    lea rcx,[rip+depth_z_comp]
    mov edx,5*5*8
    call Zero
    END

# ecx=grid, xmm0=exp(aE), xmm1=E. Compensated sums for Z,Z',Z''.
.macro DepthKahan sum, correction
    lea rax,[rip+\sum]
    lea rdx,[rip+\correction]
    subsd xmm0,[rdx+rcx*8]
    movsd xmm2,[rax+rcx*8]
    movapd xmm3,xmm2
    addsd xmm3,xmm0
    movapd xmm4,xmm3
    subsd xmm4,xmm2
    subsd xmm4,xmm0
    movsd [rdx+rcx*8],xmm4
    movsd [rax+rcx*8],xmm3
.endm
FN DepthAccumulate
    movapd xmm5,xmm0
    DepthKahan ti_z,depth_z_comp
    movapd xmm0,xmm5
    mulsd xmm0,xmm1
    movapd xmm5,xmm0
    DepthKahan depth_m1,depth_m1_comp
    movapd xmm0,xmm5
    mulsd xmm0,xmm1
    DepthKahan depth_m2,depth_m2_comp
    END

# Save d logZ/da and d2 logZ/da2; reject nonfinite/negative curvature.
FN DepthMomentsSave
    imul ebx,ecx,5
    xor esi,esi
1:  lea rax,[rip+ti_z]
    movsd xmm2,[rax+rsi*8]
    ucomisd xmm2,xmm2
    jp 3f
    comisd xmm2,[rip+zero]
    jbe 3f
    lea rax,[rip+depth_m1]
    movsd xmm0,[rax+rsi*8]
    divsd xmm0,xmm2
    lea eax,[rbx+rsi]
    lea rdx,[rip+depth_means]
    movsd [rdx+rax*8],xmm0
    mulsd xmm0,xmm0
    lea rdx,[rip+depth_m2]
    movsd xmm1,[rdx+rsi*8]
    divsd xmm1,xmm2
    subsd xmm1,xmm0
    ucomisd xmm1,xmm1
    jp 3f
    movsd xmm0,[rip+depth_tolerance]
    xorpd xmm0,[rip+sign_bits]
    comisd xmm1,xmm0
    jb 3f
    maxsd xmm1,[rip+zero]
    comisd xmm1,[rip+one]
    ja 3f
    lea rdx,[rip+depth_variances]
    movsd [rdx+rax*8],xmm1
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb 1b
    mov eax,1
    jmp 4f
3:  xor eax,eax
4:  END

# Audit finite logZ against global bounds and posterior mass against one.
FN DepthAuditFamily
    imul ebx,ecx,5
    xorpd xmm5,xmm5
    xor esi,esi
1:  lea eax,[rbx+rsi]
    lea rdx,[rip+ti_logz]
    movsd xmm0,[rdx+rax*8]
    subsd xmm0,[rip+ti_logspace]
    andpd xmm0,[rip+abs_bits]
    ucomisd xmm0,xmm0
    jp 3f
    lea rdx,[rip+ti_scales]
    movsd xmm1,[rdx+rsi*8]
    mulsd xmm1,[rip+joint_energy_bound]
    addsd xmm1,[rip+depth_tolerance]
    comisd xmm0,xmm1
    ja 3f
    lea rdx,[rip+ti_post]
    movsd xmm0,[rdx+rax*8]
    ucomisd xmm0,xmm0
    jp 3f
    comisd xmm0,[rip+zero]
    jb 3f
    comisd xmm0,[rip+one]
    ja 3f
    addsd xmm5,xmm0
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb 1b
    subsd xmm5,[rip+one]
    andpd xmm5,[rip+abs_bits]
    comisd xmm5,[rip+depth_tolerance]
    ja 3f
    mov eax,1
    jmp 4f
3:  xor eax,eax
4:  END

# Online mixture weights are written BEFORE observing each validation row.
# Log likelihood ratios are protected by a declared 1e-6 normalization margin.
FN DepthSequential,64
    lea rcx,[rip+depth_loge]
    mov edx,5*8
    call Zero
    lea rcx,[rip+depth_gains]
    mov edx,16*8
    call Zero
    mov qword ptr [rip+depth_online_loge],0
    mov qword ptr [rip+depth_cusum],0
    mov qword ptr [rip+depth_ablation],0
    movsd xmm0,[rip+one]
    movsd [rip+depth_anytime_p],xmm0
    mov qword ptr [rsp+32],0 # largest log-e over five tests AND time, incl t=0
    mov qword ptr [rsp+40],0 # current loss CUSUM
    movsd xmm0,[rip+ti_prior]
    lea rax,[rip+depth_online_weights]
    xor ecx,ecx
1:  movsd [rax+rcx*8],xmm0
    inc ecx
    cmp ecx,5
    jb 1b
    xor esi,esi
.ds_row:
    cmp esi,[rip+ti_folds]
    jae .ds_finish
    imul ebx,esi,40
    lea rcx,[rip+depth_predict_weights]
    add rcx,rbx
    lea rdx,[rip+depth_online_weights]
    mov r8d,40
    call Copy
    lea r14,[rip+ti_rows]
    add r14,rbx
    mov qword ptr [rsp+48],0 # predictive probability
    xor edi,edi
.ds_family:
    movsd xmm0,[r14+rdi*8]
    ucomisd xmm0,xmm0
    jp .ds_fail
    comisd xmm0,[rip+zero]
    jbe .ds_fail
    comisd xmm0,[rip+one]
    ja .ds_fail
    lea rax,[rip+depth_online_weights]
    mulsd xmm0,[rax+rdi*8]
    addsd xmm0,[rsp+48]
    movsd [rsp+48],xmm0
    cmp edi,4
    je 2f
    movsd xmm0,[r14+rdi*8]
    mulsd xmm0,[rip+ti_space_double]
    divsd xmm0,[rip+depth_norm_margin]
    call Log
    lea rax,[rip+depth_loge]
    addsd xmm0,[rax+rdi*8]
    movsd [rax+rdi*8],xmm0
    maxsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
2:  inc edi
    cmp edi,5
    jb .ds_family
    movsd xmm0,[rsp+48]
    mulsd xmm0,[rip+ti_space_double]
    addsd xmm0,[rip+one]
    mulsd xmm0,[rip+half]
    divsd xmm0,[rip+depth_norm_margin]
    call Log
    lea rax,[rip+depth_gains]
    movsd [rax+rsi*8],xmm0
    movsd xmm1,[rsp+40]
    subsd xmm1,xmm0
    maxsd xmm1,[rip+zero]
    movsd [rsp+40],xmm1
    maxsd xmm1,[rip+depth_cusum]
    movsd [rip+depth_cusum],xmm1
    addsd xmm0,[rip+depth_loge+32]
    movsd [rip+depth_loge+32],xmm0
    maxsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
    # Bayesian expert update only after scoring this row, with 2% fixed share.
    xor edi,edi
3:  lea rax,[rip+depth_online_weights]
    movsd xmm0,[rax+rdi*8]
    mulsd xmm0,[r14+rdi*8]
    divsd xmm0,[rsp+48]
    movsd xmm1,[rip+one]
    subsd xmm1,[rip+ti_rate]
    mulsd xmm0,xmm1
    movsd xmm1,[rip+ti_rate]
    mulsd xmm1,[rip+ti_prior]
    addsd xmm0,xmm1
    movsd [rax+rdi*8],xmm0
    inc edi
    cmp edi,5
    jb 3b
    inc esi
    jmp .ds_row
.ds_finish:
    movsd xmm0,[rip+depth_loge+32]
    movsd [rip+depth_online_loge],xmm0
    movsd xmm0,[rsp+32]
    xorpd xmm0,[rip+sign_bits]
    call Exp
    mulsd xmm0,[rip+depth_five]
    minsd xmm0,[rip+one]
    movsd [rip+depth_anytime_p],xmm0
    # Ablation is evaluated only on the last untouched outer row.
    cmp dword ptr [rip+ti_folds],0
    je .ds_success
    mov eax,[rip+ti_folds]
    dec eax
    imul eax,40
    lea r14,[rip+ti_rows]
    add r14,rax
    lea r15,[rip+ti_weights]
    xorpd xmm0,xmm0
    xor esi,esi
4:  movsd xmm1,[r14+rsi*8]
    mulsd xmm1,[r15+rsi*8]
    addsd xmm0,xmm1
    inc esi
    cmp esi,5
    jb 4b
    movsd [rsp+48],xmm0
    xor esi,esi
5:  movsd xmm0,[r14+rsi*8]
    mulsd xmm0,[r15+rsi*8]
    movsd xmm1,[rsp+48]
    subsd xmm1,xmm0
    movsd xmm0,[rip+one]
    subsd xmm0,[r15+rsi*8]
    divsd xmm1,xmm0
    divsd xmm1,[rsp+48]
    movapd xmm0,xmm1
    call Log
    andpd xmm0,[rip+abs_bits]
    maxsd xmm0,[rip+depth_ablation]
    movsd [rip+depth_ablation],xmm0
    inc esi
    cmp esi,4
    jb 5b
.ds_success: mov eax,1; jmp .ds_out
.ds_fail: xor eax,eax
.ds_out: END 64

# Stationary circular block bootstrap of CAUSAL log gains, not hindsight-fitted
# scores. Geometric block length mean 2 (normal) or 4 (deep); >=8 rows required.
FN DepthBootstrap,64
    mov dword ptr [rip+depth_boot_done],0
    mov qword ptr [rip+depth_boot_low],0
    mov qword ptr [rip+depth_boot_high],0
    cmp dword ptr [rip+ti_folds],8
    jb .db_success
    mov r12d,512
    mov r13d,2
    cmp dword ptr [rip+ti_profile],0
    je 1f
    mov r12d,4096
    mov r13d,4
1:  xor ebp,ebp
.db_rep:
    cmp dword ptr [rip+cancel_flag],0
    jne .db_fail
    mov ecx,[rip+ti_folds]
    call RandomBelow
    mov edi,eax
    mov qword ptr [rsp+32],0
    xor esi,esi
.db_row:
    cmp dword ptr [rip+rng_error],0
    jne .db_fail
    lea rax,[rip+depth_gains]
    movsd xmm0,[rax+rdi*8]
    addsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
    inc esi
    cmp esi,[rip+ti_folds]
    jae .db_store
    mov ecx,r13d
    call RandomBelow
    test eax,eax
    jnz 2f
    mov ecx,[rip+ti_folds]
    call RandomBelow
    mov edi,eax
    jmp .db_row
2:  inc edi
    cmp edi,[rip+ti_folds]
    jb .db_row
    xor edi,edi
    jmp .db_row
.db_store:
    movsd xmm0,[rsp+32]
    cvtsi2sd xmm1,dword ptr [rip+ti_folds]
    divsd xmm0,xmm1
    lea rax,[rip+depth_bootstrap]
    movsd [rax+rbp*8],xmm0
    inc ebp
    mov [rip+depth_boot_done],ebp
    cmp ebp,r12d
    jb .db_rep
    lea rcx,[rip+depth_bootstrap]
    mov edx,r12d
    call SortDoubles
    mov eax,r12d
    xor edx,edx
    mov ecx,40
    div ecx
    lea rdx,[rip+depth_bootstrap]
    movsd xmm0,[rdx+rax*8]
    movsd [rip+depth_boot_low],xmm0
    mov ecx,r12d
    dec ecx
    sub ecx,eax
    movsd xmm0,[rdx+rcx*8]
    movsd [rip+depth_boot_high],xmm0
.db_success: mov eax,1; jmp .db_out
.db_fail: xor eax,eax
.db_out: END 64

FN DepthFinalize
    call DepthSequential
    test eax,eax
    jz 1f
    call DepthBootstrap
1:  END

FN DepthReport
    lea rcx,[rip+depth_title]
    call Text
    mov ecx,[rip+ti_folds]
    call Number
    call Comma
    mov ecx,[rip+depth_boot_done]
    call Number
    call Comma
    mov ecx,[rip+depth_graph_iterations]
    call Number
    xor esi,esi
1:  call Comma
    lea rax,[rip+depth_graph_residual]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    inc esi
    cmp esi,7
    jb 1b
    lea rcx,[rip+depth_moment_title]
    call Text
    xor ebp,ebp
2:  xor esi,esi
3:  mov ecx,ebp
    call Number
    call Comma
    mov ecx,esi
    call Number
    call Comma
    imul ebx,ebp,5
    add ebx,esi
    lea rax,[rip+depth_means]
    movsd xmm0,[rax+rbx*8]
    call Decimal
    call Comma
    lea rax,[rip+depth_variances]
    movsd xmm0,[rax+rbx*8]
    call Decimal
    call Newline
    inc esi
    cmp esi,[rip+ti_grid_count]
    jb 3b
    inc ebp
    cmp ebp,4
    jb 2b
    lea rcx,[rip+depth_limits]
    call Text
    END
