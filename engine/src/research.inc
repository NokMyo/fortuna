# Research orchestration. Expensive, explicitly requested; all state remains
# engine-private and is protected by the public API lock.
.section .data
research_excluded: .long -1
scan_capacity: .long TOP_FAST
research_outer_limit: .long 30
research_world_limit: .long 32
.section .rdata,"dr"
research_settings: .double 0.5,1.0,2.0
research_precision: .double 0.0005
research_large: .double 1.0e100
.section .bss
forecast_valid: .skip 4
forecast_evaluated: .skip 4
forecast_pending: .skip 4
forecast_log_e: .skip 8
forecast_max_log_e: .skip 8
research_active: .skip 4
research_full_scan: .skip 4
audit_running: .skip 4
research_validation_logsum: .skip 8
research_confirmation_logsum: .skip 8
research_begin:
research_ready: .skip 4
research_ablation_count: .skip 4
research_ablations: .skip 13*64
research_outer_count: .skip 4
research_outer_rows: .skip 32*48
research_log_e: .skip 8
research_max_log_e: .skip 8
research_joint_loss: .skip 8
research_uniform_loss: .skip 8
research_chosen: .skip 4
research_world_count: .skip 4
research_mc_error: .skip 8
research_data_variance: .skip 8
research_disagreement: .skip 8
research_aleatoric: .skip 8
research_mc_converged: .skip 4
audit_samples: .skip 4
audit_misses_before: .skip 4
audit_misses_after: .skip 4
audit_capacity: .skip 4
research_end:
.equ RESEARCH_BYTES,research_end-research_begin
research_saved_coef: .skip JOINT_BYTES
research_saved_logz: .skip 8
research_saved_weights: .skip 24
research_saved_moments: .skip JOINT_BYTES
research_world_mean: .skip 45*8
research_world_m2: .skip 45*8
research_world_model_mean: .skip 3*45*8
research_world_model_count: .skip 3*4
audit_seen: .skip 512*8
.section .text
# Full-support diagnostic forecast of the candidate field. Training challenger
# has rho=0; for logarithmic evaluation it is mixed with 50% uniform explicitly.
FN ResearchPolicyLog
    mov rbx,rcx
    movsd xmm4,[rip+rho]
    maxsd xmm4,[rip+half]
    mov eax,TOTAL_COMBINATIONS
    cvtsi2sd xmm1,eax
    movapd xmm0,xmm4
    divsd xmm0,xmm1
    lea rdx,[rip+final_masks]
    lea r8,[rip+final_prob]
    xor ecx,ecx
1:  cmp ecx,[rip+field_count]
    jae 3f
    cmp rbx,[rdx+rcx*8]
    je 2f
    inc ecx
    jmp 1b
2:  movsd xmm1,[rip+one]
    subsd xmm1,xmm4
    mulsd xmm1,[r8+rcx*8]
    addsd xmm0,xmm1
3:  call Log
    END
FN ResearchLogFold
    mov rcx,[rip+validation_truth]
    call ResearchPolicyLog
    movsd xmm1,[rip+research_validation_logsum]
    subsd xmm1,xmm0
    movsd [rip+research_validation_logsum],xmm1
    END
FN ResearchLogConfirmation
    mov rcx,[rip+validation_truth]
    call ResearchPolicyLog
    movsd xmm1,[rip+research_confirmation_logsum]
    subsd xmm1,xmm0
    movsd [rip+research_confirmation_logsum],xmm1
    END
# Audit uniformly sampled, distinct rejected masks. Expand to 8192 fast
# survivors and rebuild the deep heap if the audit detects a missed survivor.
FN SearchAudit,32
    mov dword ptr [rip+audit_running],1
    mov dword ptr [rip+audit_samples],0
    mov dword ptr [rip+audit_misses_before],0
    mov dword ptr [rip+audit_misses_after],0
    mov dword ptr [rip+audit_capacity],TOP_FAST
    mov rax,[rip+rng_state]
    mov [rsp+32],rax
    mov eax,[rip+rng_mode]
    mov [rsp+40],eax
    mov dword ptr [rip+rng_mode],1
    # Prefix-only deterministic diagnostic sampling, independent of truth.
    lea rcx,[rip+history]
    imul edx,dword ptr [rip+draw_count],RECORD_SIZE
    lea r8,[rip+world_prefix_hash]
    call Sha256
    mov rax,[rip+world_prefix_hash]
    xor rax,0x465341
    mov [rip+rng_state],rax
    xor ebp,ebp
    cmp dword ptr [rip+research_full_scan],1
    je .sa_done
.sa_pass:
    xor esi,esi
    xor r12d,r12d
    xor r14d,r14d
    mov r13,[rip+final_scores]
.sa_sample:
    inc r14d
    cmp r14d,65536
    ja .sa_pass_end
    cmp dword ptr [rip+cancel_flag],0
    jne .sa_fail
    call GenerateUniformMask
    mov rbx,rax
    xor ecx,ecx
    lea rdx,[rip+audit_seen]
1:  cmp ecx,esi
    jae 2f
    cmp rbx,[rdx+rcx*8]
    je .sa_sample
    inc ecx
    jmp 1b
2:  mov rcx,rbx
    call ScoreFast
    cmp rax,[rip+fast_scores]
    jg .sa_sample
    jl 4f
    xor ecx,ecx
    lea rdx,[rip+fast_masks]
5:  cmp ecx,[rip+fast_count]
    jae 4f
    cmp rbx,[rdx+rcx*8]
    je .sa_sample
    inc ecx
    jmp 5b
4:
    lea rdx,[rip+audit_seen]
    mov [rdx+rsi*8],rbx
    mov rcx,rbx
    call ScoreDeep
    cmp rax,r13
    jle 3f
    inc r12d
3:  inc esi
    inc dword ptr [rip+audit_samples]
    cmp esi,512
    jb .sa_sample
.sa_pass_end:
    test ebp,ebp
    jnz .sa_after
    mov [rip+audit_misses_before],r12d
    test r12d,r12d
    jz .sa_done
    mov dword ptr [rip+scan_capacity],MAX_FAST
    mov dword ptr [rip+audit_capacity],MAX_FAST
    call ScanField
    test eax,eax
    jz .sa_fail
    inc ebp
    jmp .sa_pass
.sa_after:
    mov [rip+audit_misses_after],r12d
.sa_done: mov ebx,1; jmp .sa_restore
.sa_fail: xor ebx,ebx
.sa_restore:
    mov dword ptr [rip+scan_capacity],TOP_FAST
    mov dword ptr [rip+audit_running],0
    mov rax,[rsp+32]
    mov [rip+rng_state],rax
    mov eax,[rsp+40]
    mov [rip+rng_mode],eax
    mov eax,ebx
    END 32
# Nested chronological selection. Inner block is four previously observed draws;
# outer outcomes never choose the setting used to forecast themselves.
FN JointSelectSetting,32
    cmp ecx,64
    jae .jss_nested
    mov dword ptr [rip+research_chosen],1
    mov dword ptr [rip+joint_order],3
    movsd xmm0,[rip+one]
    movsd [rip+joint_shrink],xmm0
    xor edx,edx
    call JointFit
    jmp .jss_out
.jss_nested:
    mov ebp,ecx
    movsd xmm0,[rip+research_large]
    movsd [rsp+32],xmm0
    xor r12d,r12d
    mov dword ptr [rip+research_chosen],0
.jss_setting:
    lea rax,[rip+research_settings]
    movsd xmm0,[rax+r12*8]
    movsd [rip+joint_shrink],xmm0
    mov dword ptr [rip+joint_order],3
    lea ecx,[rbp-4]
    xor edx,edx
    call JointFit
    test eax,eax
    jz .jss_fail
    mov qword ptr [rsp+40],0
    lea esi,[rbp-4]
1:  imul eax,esi,RECORD_SIZE
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    call JointForecastLog
    movsd xmm1,[rsp+40]
    subsd xmm1,xmm0
    movsd [rsp+40],xmm1
    inc esi
    cmp esi,ebp
    jb 1b
    comisd xmm1,[rsp+32]
    jae 2f
    movsd [rsp+32],xmm1
    mov [rip+research_chosen],r12d
2:  inc r12d
    cmp r12d,3
    jb .jss_setting
    mov eax,[rip+research_chosen]
    lea rdx,[rip+research_settings]
    movsd xmm0,[rdx+rax*8]
    movsd [rip+joint_shrink],xmm0
    mov ecx,ebp
    xor edx,edx
    call JointFit
    jmp .jss_out
.jss_fail: xor eax,eax
.jss_out: END 32
FN JointNestedValidation,32
    mov dword ptr [rip+analysis_stage],9
    mov dword ptr [rip+progress],0
    mov dword ptr [rip+research_outer_count],0
    mov qword ptr [rip+research_log_e],0
    mov qword ptr [rip+research_max_log_e],0
    mov qword ptr [rip+research_joint_loss],0
    mov qword ptr [rip+research_uniform_loss],0
    mov ebp,[rip+backtest_end_index]
    sub ebp,[rip+research_outer_limit]
    cmp ebp,[rip+backtest_start_index]
    jae .jnv_min_train
    mov ebp,[rip+backtest_start_index]
.jnv_min_train:
    cmp ebp,64
    jge .jnv_loop
    mov ebp,64
.jnv_loop:
    mov eax,[rip+backtest_end_index]
    sub eax,ebp
    test eax,eax
    jns 2f
    xor eax,eax
2:  mov ecx,eax
    test ecx,ecx
    jnz 3f
    mov ecx,1
3:
    mov eax,[rip+research_outer_count]
    imul eax,eax,100
    xor edx,edx
    div ecx
    mov [rip+progress],eax
    cmp ebp,[rip+backtest_end_index]
    jae .jnv_final
    mov dword ptr [rip+analysis_stage],9
    mov ecx,ebp
    call JointSelectSetting
    test eax,eax
    jz .jnv_fail
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    call JointForecastLog
    movsd [rsp+32],xmm0
    movsd xmm1,[rip+research_joint_loss]
    subsd xmm1,xmm0
    movsd [rip+research_joint_loss],xmm1
    cvtsi2sd xmm0,qword ptr [rip+joint_space]
    call Log
    movsd [rsp+40],xmm0
    movsd xmm1,[rip+research_uniform_loss]
    addsd xmm1,xmm0
    movsd [rip+research_uniform_loss],xmm1
    addsd xmm0,[rsp+32]
    addsd xmm0,[rip+research_log_e]
    movsd [rip+research_log_e],xmm0
    movsd xmm1,[rip+research_max_log_e]
    maxsd xmm1,xmm0
    movsd [rip+research_max_log_e],xmm1
    mov eax,[rip+research_outer_count]
    imul eax,eax,48
    lea rbx,[rip+research_outer_rows]
    add rbx,rax
    mov [rbx],ebp
    mov eax,[rip+research_chosen]
    mov [rbx+4],eax
    movsd xmm0,[rsp+32]
    xorpd xmm0,[rip+sign_bits]
    movsd [rbx+8],xmm0
    movsd xmm0,[rsp+40]
    movsd [rbx+16],xmm0
    movsd xmm0,[rip+research_log_e]
    movsd [rbx+24],xmm0
    # Proper number-inclusion Brier loss for this exact full-support forecast.
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov r12,[rdx+rax+40]
    xor esi,esi
    xorpd xmm4,xmm4
    lea rdx,[rip+joint_moment]
1:  movsd xmm0,[rdx+rsi*8]
    movsd xmm1,[rip+six]
    cvtsi2sd xmm2,dword ptr [rip+joint_universe]
    divsd xmm1,xmm2
    addsd xmm0,xmm1
    mulsd xmm0,[rip+half]
    bt r12,rsi
    jnc 2f
    subsd xmm0,[rip+one]
2:  mulsd xmm0,xmm0
    addsd xmm4,xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb 1b
    divsd xmm4,xmm2
    movsd [rbx+32],xmm4
    inc dword ptr [rip+research_outer_count]
    inc ebp
    jmp .jnv_loop
.jnv_final:
    mov ecx,[rip+history_count]
    call JointSelectSetting
    jmp .jnv_out
.jnv_fail: xor eax,eax
.jnv_out: END 32
# Monte Carlo uncertainty in exact marginals across random window/shrink/order
# perturbations. Stop on estimated SE, not a claimed confidence guarantee.
FN JointUncertainty,48
    lea rcx,[rip+research_saved_coef]
    lea rdx,[rip+joint_coef]
    mov r8d,JOINT_BYTES
    call Copy
    lea rcx,[rip+research_saved_moments]
    lea rdx,[rip+joint_moment]
    mov r8d,JOINT_BYTES
    call Copy
    lea rcx,[rip+research_saved_weights]
    lea rdx,[rip+joint_level_weight]
    mov r8d,24
    call Copy
    mov rax,[rip+joint_logz]
    mov [rip+research_saved_logz],rax
    mov rax,[rip+joint_shrink]
    mov [rsp+32],rax
    mov rax,[rip+rng_state]
    mov [rsp+40],rax
    mov eax,[rip+rng_mode]
    mov [rsp+48],eax
    mov dword ptr [rip+rng_mode],1
    mov rax,[rip+hash_dataset]
    xor rax,0x4a554e43
    mov [rip+rng_state],rax
    lea rcx,[rip+research_world_mean]
    mov edx,45*8
    call Zero
    lea rcx,[rip+research_world_m2]
    mov edx,45*8
    call Zero
    lea rcx,[rip+research_world_model_mean]
    mov edx,3*45*8
    call Zero
    lea rcx,[rip+research_world_model_count]
    mov edx,12
    call Zero
    mov dword ptr [rip+research_world_count],0
    mov dword ptr [rip+research_mc_converged],0
    xor ebp,ebp
    mov dword ptr [rip+progress],0
.ju_world:
    mov dword ptr [rip+analysis_stage],10
    cmp ebp,6
    jae 1f
    mov eax,ebp
    xor edx,edx
    mov ecx,3
    div ecx
    mov eax,edx
    jmp 2f
1:  mov ecx,3
    call RandomBelow
2:  inc eax
    mov [rip+joint_order],eax
    mov r12d,eax
    dec r12d
    mov ecx,3
    call RandomBelow
    lea rdx,[rip+research_settings]
    movsd xmm0,[rdx+rax*8]
    movsd [rip+joint_shrink],xmm0
    mov ecx,3
    call RandomBelow
    imul eax,dword ptr [rip+history_count]
    xor edx,edx
    mov ecx,10
    div ecx
    mov edx,eax
    mov ecx,[rip+history_count]
    call JointFit
    test eax,eax
    jz .ju_fail
    inc ebp
    mov [rip+research_world_count],ebp
    imul eax,ebp,100
    xor edx,edx
    div dword ptr [rip+research_world_limit]
    mov [rip+progress],eax
    lea rax,[rip+research_world_model_count]
    inc dword ptr [rax+r12*4]
    mov edi,[rax+r12*4]
    xor esi,esi
    lea rbx,[rip+joint_moment]
    lea r13,[rip+research_world_mean]
    lea r14,[rip+research_world_m2]
    imul eax,r12d,45*8
    lea r15,[rip+research_world_model_mean]
    add r15,rax
1:  movsd xmm0,[rbx+rsi*8]
    movapd xmm1,xmm0
    subsd xmm1,[r13+rsi*8]
    cvtsi2sd xmm2,ebp
    movapd xmm3,xmm1
    divsd xmm3,xmm2
    addsd xmm3,[r13+rsi*8]
    movsd [r13+rsi*8],xmm3
    movapd xmm2,xmm0
    subsd xmm2,xmm3
    mulsd xmm1,xmm2
    addsd xmm1,[r14+rsi*8]
    movsd [r14+rsi*8],xmm1
    subsd xmm0,[r15+rsi*8]
    cvtsi2sd xmm2,edi
    divsd xmm0,xmm2
    addsd xmm0,[r15+rsi*8]
    movsd [r15+rsi*8],xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb 1b
    cmp ebp,8
    jb .ju_world
    test ebp,3
    jnz .ju_world
    xor esi,esi
    xorpd xmm4,xmm4
    xorpd xmm5,xmm5
    lea eax,[rbp-1]
    cvtsi2sd xmm2,eax
    cvtsi2sd xmm3,ebp
2:  movsd xmm0,[r14+rsi*8]
    divsd xmm0,xmm2
    addsd xmm5,xmm0
    divsd xmm0,xmm3
    sqrtsd xmm0,xmm0
    maxsd xmm4,xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb 2b
    movsd [rip+research_mc_error],xmm4
    cvtsi2sd xmm0,dword ptr [rip+joint_universe]
    divsd xmm5,xmm0
    movsd [rip+research_data_variance],xmm5
    # Require each model family to be represented before early termination.
    lea rax,[rip+research_world_model_count]
    cmp dword ptr [rax],2
    jb .ju_cap
    cmp dword ptr [rax+4],2
    jb .ju_cap
    cmp dword ptr [rax+8],2
    jb .ju_cap
    comisd xmm4,[rip+research_precision]
    ja .ju_cap
    mov dword ptr [rip+research_mc_converged],1
    jmp .ju_finish
.ju_cap:
    cmp ebp,[rip+research_world_limit]
    jb .ju_world
.ju_finish:
    # Between-model-family variation (descriptive, not a posterior CI).
    xorpd xmm5,xmm5
    xor esi,esi
3:  lea rdx,[rip+research_world_model_mean]
    movsd xmm0,[rdx+rsi*8]
    movsd xmm1,[rdx+rsi*8+45*8]
    movsd xmm2,[rdx+rsi*8+90*8]
    movapd xmm3,xmm0
    subsd xmm3,xmm1
    mulsd xmm3,xmm3
    addsd xmm5,xmm3
    subsd xmm1,xmm2
    mulsd xmm1,xmm1
    addsd xmm5,xmm1
    subsd xmm0,xmm2
    mulsd xmm0,xmm0
    addsd xmm5,xmm0
    inc esi
    cmp esi,[rip+joint_universe]
    jb 3b
    cvtsi2sd xmm0,dword ptr [rip+joint_universe]
    mulsd xmm0,[rip+six]
    divsd xmm5,xmm0
    movsd [rip+research_disagreement],xmm5
    mov ebx,1
    jmp .ju_restore
.ju_fail: xor ebx,ebx
.ju_restore:
    lea rcx,[rip+joint_coef]
    lea rdx,[rip+research_saved_coef]
    mov r8d,JOINT_BYTES
    call Copy
    lea rcx,[rip+joint_moment]
    lea rdx,[rip+research_saved_moments]
    mov r8d,JOINT_BYTES
    call Copy
    lea rcx,[rip+joint_level_weight]
    lea rdx,[rip+research_saved_weights]
    mov r8d,24
    call Copy
    mov rax,[rip+research_saved_logz]
    mov [rip+joint_logz],rax
    mov rax,[rsp+32]
    mov [rip+joint_shrink],rax
    mov rax,[rsp+40]
    mov [rip+rng_state],rax
    mov eax,[rsp+48]
    mov [rip+rng_mode],eax
    movsd xmm0,[rip+joint_logz]
    call Exp
    movsd [rip+joint_z],xmm0
    mov dword ptr [rip+joint_order],3
    mov ecx,[rip+history_count]
    mov [rip+joint_n],ecx
    mov dword ptr [rip+joint_start],0
    xorpd xmm5,xmm5
    lea rdx,[rip+joint_moment]
    xor ecx,ecx
1:  movsd xmm0,[rdx+rcx*8]
    movsd xmm1,[rip+one]
    subsd xmm1,xmm0
    mulsd xmm0,xmm1
    addsd xmm5,xmm0
    inc ecx
    cmp ecx,[rip+joint_universe]
    jb 1b
    cvtsi2sd xmm0,ecx
    divsd xmm5,xmm0
    movsd [rip+research_aleatoric],xmm5
    mov eax,ebx
    END 48
FN AnalyzeResearch
    lea rcx,[rip+research_begin]
    mov edx,offset RESEARCH_BYTES
    call Zero
    mov dword ptr [rip+research_active],1
    mov dword ptr [rip+joint_universe],45
    xor ebp,ebp
.ar_ablation:
    mov [rip+research_excluded],ebp
    cmp ebp,MODEL_COUNT
    jne 1f
    mov dword ptr [rip+research_excluded],-1
1:  mov dword ptr [rip+analysis_stage],8
    call AnalyzeOracle
    test eax,eax
    jz .ar_fail
    imul eax,ebp,64
    lea rbx,[rip+research_ablations]
    add rbx,rax
    mov eax,[rip+research_excluded]
    mov [rbx],eax
    mov eax,[rip+validation_count]
    mov [rbx+4],eax
    movsd xmm0,[rip+validation_mean]
    movsd [rbx+8],xmm0
    movsd xmm0,[rip+validation_brier]
    movsd [rbx+16],xmm0
    movsd xmm0,[rip+research_validation_logsum]
    movsd [rbx+24],xmm0
    movsd xmm0,[rip+research_confirmation_logsum]
    movsd [rbx+32],xmm0
    movsd xmm0,[rip+rho]
    movsd [rbx+40],xmm0
    mov eax,[rip+confirmation_status]
    mov [rbx+48],eax
    mov eax,[rip+confirmation_count]
    mov [rbx+52],eax
    inc dword ptr [rip+research_ablation_count]
    inc ebp
    cmp ebp,MODEL_COUNT+1
    jb .ar_ablation
    call JointNestedValidation
    test eax,eax
    jz .ar_fail
    call JointUncertainty
    test eax,eax
    jz .ar_fail
    mov dword ptr [rip+research_ready],1
    mov dword ptr [rip+analysis_stage],7
    mov dword ptr [rip+progress],100
    mov eax,1
    jmp .ar_finish
.ar_fail:
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+field_ready],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
    xor eax,eax
.ar_finish:
    mov dword ptr [rip+research_active],0
    mov dword ptr [rip+research_excluded],-1
    mov dword ptr [rip+research_full_scan],0
    mov dword ptr [rip+rng_mode],0
    END

.section .rdata,"dr"
rr_none: .asciz "\r\nRESEARCH SUITE: not run for this dataset. Use AnalyzeResearch / --research.\r\n"
rr_head: .asciz "\r\nORACLE RESEARCH 1.1 / exact joint model + complete component ablation\r\nJoint model is a research forecast, not an established winning probability.\r\nGaussian working posterior + empirical-Bayes level shrinkage + family threshold.\r\nExact support, log Z, shrink setting, marginal sum error, level weights\r\n"
rr_ablations: .asciz "\r\nFULL PIPELINE ABLATION (excluded -1 = all; 0..11 = omitted component)\r\nEvery row reruns chronological fitting, selection, weights, sampling, and confirmation.\r\nLog loss uses at least 50% uniform support; values are sums, not means.\r\nexcluded,learning_folds,mean_hits,brier,learning_log_loss,confirmation_log_loss,rho,confirmation_status,confirmation_folds\r\n"
rr_nested: .asciz "\r\nNESTED JOINT FORECASTS / latest at most 30 origins inside the active historical range, each >=64\r\nInner four past outcomes select shrink 0.5/1/2; refit precedes outer outcome.\r\nRetrospective log-e is diagnostic: repeated development is not prospective evidence.\r\ntraining_rows,chosen_setting_index,log_loss,uniform_log_loss,cumulative_log_e,brier\r\n"
rr_uncertainty: .asciz "\r\nUNCERTAINTY / random training-window, shrink and interaction-order perturbations\r\nConditional on the chosen finite perturbation family, not total real-world uncertainty.\r\nworlds,estimated_max_marginal_MC_SE,combined_perturbation_variance,model_family_disagreement,intrinsic_bernoulli_variance,precision_target_met\r\n"
rr_search: .asciz "\r\nSEARCH AUDIT / rejected uniform masks, bounded at 65536 attempts per pass\r\nTested masks,misses_before_expansion,misses_after_expansion,fast_capacity\r\nCounts are sample diagnostics, not a guarantee of zero global omissions.\r\n"
rr_forecast: .asciz "\r\nLOCAL PROSPECTIVE FORECAST LEDGER\r\nvalid,evaluated,pending,log_e,max_log_e\r\nOnly trust values when valid=1. Each full distribution is frozen before its local-clock deadline.\r\nUnder independent uniform draws, previsible normalized forecasts give an e-process.\r\nlog(20) is the one-stream 5% crossing boundary; no adjustment for multiple chosen ledgers.\r\nLocal hashes and clocks are NOT third-party timestamp proof. Preserve a trusted external copy.\r\n"
.section .text
FN ResearchReport
    cmp dword ptr [rip+research_ready],1
    je .rr_begin
    lea rcx,[rip+rr_none]
    call Text
    jmp .rr_ledger
.rr_begin:
    lea rcx,[rip+rr_head]
    call Text
    mov rcx,[rip+joint_space]
    call Number
    call Comma
    movsd xmm0,[rip+joint_logz]
    call Decimal
    call Comma
    movsd xmm0,[rip+joint_shrink]
    call Decimal
    call Comma
    movsd xmm0,[rip+joint_marginal_error]
    call Decimal
    xor esi,esi
1:  call Comma
    lea rax,[rip+joint_level_weight]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    inc esi
    cmp esi,3
    jb 1b
    call Newline
    lea rcx,[rip+rr_ablations]
    call Text
    xor esi,esi
.rr_ablation:
    cmp esi,[rip+research_ablation_count]
    jae .rr_outer_head
    imul eax,esi,64
    lea rbx,[rip+research_ablations]
    add rbx,rax
    movsxd rcx,dword ptr [rbx]
    call SignedNumber
    call Comma
    mov ecx,[rbx+4]
    call Number
    mov edi,8
1:  call Comma
    movsd xmm0,[rbx+rdi]
    call Decimal
    add edi,8
    cmp edi,48
    jb 1b
    call Comma
    mov ecx,[rbx+48]
    call Number
    call Comma
    mov ecx,[rbx+52]
    call Number
    call Newline
    inc esi
    jmp .rr_ablation
.rr_outer_head:
    lea rcx,[rip+rr_nested]
    call Text
    xor esi,esi
.rr_outer:
    cmp esi,[rip+research_outer_count]
    jae .rr_uncertainty
    imul eax,esi,48
    lea rbx,[rip+research_outer_rows]
    add rbx,rax
    mov ecx,[rbx]
    call Number
    call Comma
    mov ecx,[rbx+4]
    call Number
    mov edi,8
1:  call Comma
    movsd xmm0,[rbx+rdi]
    call Decimal
    add edi,8
    cmp edi,40
    jb 1b
    call Newline
    inc esi
    jmp .rr_outer
.rr_uncertainty:
    lea rcx,[rip+rr_uncertainty]
    call Text
    mov ecx,[rip+research_world_count]
    call Number
    call Comma
    movsd xmm0,[rip+research_mc_error]
    call Decimal
    call Comma
    movsd xmm0,[rip+research_data_variance]
    call Decimal
    call Comma
    movsd xmm0,[rip+research_disagreement]
    call Decimal
    call Comma
    movsd xmm0,[rip+research_aleatoric]
    call Decimal
    call Comma
    mov ecx,[rip+research_mc_converged]
    call Number
    call Newline
    lea rcx,[rip+rr_search]
    call Text
    mov ecx,[rip+audit_samples]
    call Number
    call Comma
    mov ecx,[rip+audit_misses_before]
    call Number
    call Comma
    mov ecx,[rip+audit_misses_after]
    call Number
    call Comma
    mov ecx,[rip+audit_capacity]
    call Number
    call Newline
.rr_ledger:
    lea rcx,[rip+rr_forecast]
    call Text
    mov ecx,[rip+forecast_valid]
    call Number
    call Comma
    mov ecx,[rip+forecast_evaluated]
    call Number
    call Comma
    mov ecx,[rip+forecast_pending]
    call Number
    call Comma
    movsd xmm0,[rip+forecast_log_e]
    call Decimal
    call Comma
    movsd xmm0,[rip+forecast_max_log_e]
    call Decimal
    call Newline
    END
