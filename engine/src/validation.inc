.section .rdata,"dr"
alpha: .double 0.05
expected_hits: .double 0.8
ci_z: .double 1.96
.section .bss
null_totals: .skip NULL_COUNT*8
null_model_totals: .skip NULL_COUNT*MODEL_COUNT*8
validation_truth: .skip 8
validation_pick: .skip 8
validation_challenger_hit: .skip 8
validation_ece: .skip 8
rank_truth_mask: .skip 8
rank_truth_score: .skip 8
rank_truth_position: .skip 4
q_sort: .skip MODEL_COUNT*4
validation_complete: .skip 4
validation_stop: .skip 4
confirmation_begin:
confirmation_status: .skip 4
confirmation_count: .skip 4
confirmation_hits: .skip 8
confirmation_brier: .skip 8
confirmation_p: .skip 8
confirmation_rows: .skip 30*64
confirmation_end:
.equ CONFIRMATION_BYTES,confirmation_end-confirmation_begin
confirmation_null: .skip NULL_COUNT*8
.section .text
# Resolve the configured inclusive target-round window to zero-based history
# indices. Automatic mode preserves the legacy policy: every target after the
# first 60 training rows, with the last 30 reserved for confirmation only when
# at least 120 rows exist. A custom window always contains at least 60 targets,
# so 30 learning folds and 30 independent confirmation folds remain available.
FN ResolveBacktestRange
    mov eax,[rip+history_count]
    cmp eax,60
    jb .rbr_fail
    mov dword ptr [rip+backtest_start_index],60
    mov [rip+backtest_end_index],eax
    mov dword ptr [rip+validation_stop],eax
    cmp dword ptr [rip+backtest_start_round],0
    jne .rbr_custom
    cmp dword ptr [rip+backtest_end_round],0
    jne .rbr_fail
    cmp eax,120
    jb .rbr_ok
    sub eax,30
    mov [rip+validation_stop],eax
    jmp .rbr_ok
.rbr_custom:
    cmp dword ptr [rip+backtest_end_round],0
    je .rbr_fail
    mov r8d,[rip+history]
    mov eax,[rip+backtest_start_round]
    sub eax,r8d
    jc .rbr_fail
    cmp eax,60
    jb .rbr_fail
    mov ebx,eax
    mov eax,[rip+backtest_end_round]
    sub eax,r8d
    jc .rbr_fail
    inc eax
    cmp eax,[rip+history_count]
    ja .rbr_fail
    cmp eax,ebx
    jbe .rbr_fail
    mov ecx,eax
    sub ecx,ebx
    cmp ecx,60
    jb .rbr_fail
    mov [rip+backtest_start_index],ebx
    mov [rip+backtest_end_index],eax
    sub eax,30
    mov [rip+validation_stop],eax
.rbr_ok:
    mov eax,1
    jmp .rbr_out
.rbr_fail:
    xor eax,eax
.rbr_out: END

# BH adjusted q-values: stable index sort, reverse cumulative minimum.
FN BHCorrection
    mov rbx,rcx
    mov ebp,edx
    mov rdi,r8
    lea r12,[rip+q_sort]
    xor esi,esi
1:  mov [r12+rsi*4],esi
    inc esi
    cmp esi,ebp
    jb 1b
    mov esi,1
.bh_sort:
    mov eax,[r12+rsi*4]
    movsd xmm0,[rbx+rax*8]
    mov edx,esi
1:  test edx,edx
    jz 2f
    mov ecx,[r12+rdx*4-4]
    comisd xmm0,[rbx+rcx*8]
    jae 2f
    mov [r12+rdx*4],ecx
    dec edx
    jmp 1b
2:  mov [r12+rdx*4],eax
    inc esi
    cmp esi,ebp
    jb .bh_sort
    mov esi,ebp
    movsd xmm5,[rip+one]
1:  mov eax,[r12+rsi*4-4]
    movsd xmm0,[rbx+rax*8]
    cvtsi2sd xmm1,ebp
    mulsd xmm0,xmm1
    cvtsi2sd xmm1,esi
    divsd xmm0,xmm1
    minsd xmm5,xmm0
    movsd [rdi+rax*8],xmm5
    dec esi
    jnz 1b
    END

# Full rolling-origin challenger validation: every cutoff from 60 to N-1.
# Every fold rebuilds normalization, spectral/covariance/triple models, the
# exhaustive 8,145,060 scan, geometry, 32 worlds and thermal field using prefix
# data only. Champion remains uniform until independent gates below pass.
FN ValidateHistory,32
    mov qword ptr [rip+research_validation_logsum],0
    mov qword ptr [rip+research_confirmation_logsum],0
    mov dword ptr [rip+validation_complete],0
    call ResolveBacktestRange
    test eax,eax
    jz .vh_cancel
    lea rcx,[rip+confirmation_begin]
    mov edx,offset CONFIRMATION_BYTES
    call Zero
    lea rcx,[rip+validation_begin]
    mov edx,offset VALIDATION_BYTES
    call Zero
    lea rcx,[rip+null_totals]
    mov edx,NULL_COUNT*8
    call Zero
    lea rcx,[rip+null_model_totals]
    mov edx,NULL_COUNT*MODEL_COUNT*8
    call Zero
    lea rcx,[rip+model_weights]
    mov edx,MODEL_COUNT*8
    call Zero
    mov dword ptr [rip+rng_mode],1
    mov qword ptr [rip+rng_state],73518492
    mov dword ptr [rip+rng_error],0
    mov eax,[rip+backtest_start_index]
    mov [rip+validation_cutoff],eax
.vh_fold:
    mov ebp,[rip+validation_cutoff]
    cmp ebp,[rip+validation_stop]
    jae .vh_done
    cmp dword ptr [rip+cancel_flag],0
    jne .vh_cancel
    mov ecx,ebp
    call RebuildStats
    call PrepareModels
    test eax,eax
    jz .vh_cancel
    call BuildNormalization
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov rax,[rdx+rax+40]
    mov [rip+validation_truth],rax
    mov [rip+rank_truth_mask],rax
    call ScanField
    test eax,eax
    jz .vh_cancel
    call BuildGeometry
    test eax,eax
    jz .vh_cancel
    call BuildWorlds
    test eax,eax
    jz .vh_cancel
    movsd xmm0,[rip+one]
    movsd [rip+evidence],xmm0
    mov qword ptr [rip+rho],0
    call BuildThermal
    call ResearchLogFold
    call SampleMask
    mov [rip+validation_pick],rax
    mov rcx,rax
    and rcx,[rip+validation_truth]
    call PopCount64
    lea rdx,[rip+validation_hits]
    inc dword ptr [rdx+rax*4]
    cvtsi2sd xmm0,eax
    movsd [rip+validation_challenger_hit],xmm0
    addsd xmm0,[rip+validation_sum]
    movsd [rip+validation_sum],xmm0
    mov eax,[rip+validation_count]
    imul eax,eax,64
    lea rbx,[rip+validation_rows]
    add rbx,rax
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov ecx,[rdx+rax]
    mov [rbx],ecx
    mov [rbx+4],ebp
    mov rax,[rip+validation_pick]
    mov [rbx+8],rax
    mov rax,[rip+validation_truth]
    mov [rbx+16],rax
    movsd xmm0,[rip+validation_challenger_hit]
    movsd [rbx+24],xmm0
    mov eax,[rip+rank_truth_position]
    mov [rbx+32],eax
    # Proper marginal Brier loss and a 10-bin inclusion reliability curve.
    xor esi,esi
    lea rdi,[rip+challenger_marginals]
    xorpd xmm4,xmm4
    xorpd xmm5,xmm5
.vh_brier:
    xorpd xmm0,xmm0
    mov rax,[rip+validation_truth]
    bt rax,rsi
    jnc 1f
    movsd xmm0,[rip+one]
1:  movsd xmm1,[rdi+rsi*8]
    movapd xmm2,xmm1
    subsd xmm2,xmm0
    mulsd xmm2,xmm2
    addsd xmm4,xmm2
    movsd xmm2,[rip+p_one]
    subsd xmm2,xmm0
    mulsd xmm2,xmm2
    addsd xmm5,xmm2
    mov eax,10
    cvtsi2sd xmm2,eax
    mulsd xmm2,xmm1
    cvttsd2si eax,xmm2
    cmp eax,9
    jbe 2f
    mov eax,9
2:  imul eax,eax,24
    lea rdx,[rip+validation_bins]
    movsd xmm2,[rdx+rax]
    addsd xmm2,[rip+one]
    movsd [rdx+rax],xmm2
    addsd xmm0,[rdx+rax+8]
    movsd [rdx+rax+8],xmm0
    addsd xmm1,[rdx+rax+16]
    movsd [rdx+rax+16],xmm1
    inc esi
    cmp esi,45
    jb .vh_brier
    divsd xmm4,[rip+fortyfive]
    divsd xmm5,[rip+fortyfive]
    movsd [rbx+40],xmm4
    addsd xmm4,[rip+validation_brier]
    movsd [rip+validation_brier],xmm4
    addsd xmm5,[rip+validation_baseline_brier]
    movsd [rip+validation_baseline_brier],xmm5
    mov rcx,[rip+validation_truth]
    call ComputeNormalized
    lea rbx,[rip+norm_z]
    lea rdi,[rip+model_null_mean]
    lea r12,[rip+validation_models]
    xor esi,esi
    xorpd xmm5,xmm5
1:  movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rdi+rsi*8]
    addsd xmm5,xmm0
    movsd [rsp+32],xmm0
    addsd xmm0,[r12+rsi*8]
    movsd [r12+rsi*8],xmm0
    inc esi
    cmp esi,MODEL_COUNT-1
    jb 1b
    mov eax,MODEL_COUNT-1
    cvtsi2sd xmm1,eax
    divsd xmm5,xmm1
    lea r12,[rip+validation_ablation]
    xor esi,esi
1:  movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rdi+rsi*8]
    subsd xmm0,xmm5
    addsd xmm0,[r12+rsi*8]
    movsd [r12+rsi*8],xmm0
    inc esi
    cmp esi,MODEL_COUNT-1
    jb 1b
    # 512 matched uniform baselines and independent model-null resamples.
    xor esi,esi
.vh_null:
    call GenerateUniformMask
    mov rcx,rax
    and rcx,[rip+validation_pick]
    call PopCount64
    cvtsi2sd xmm0,eax
    lea rdx,[rip+null_totals]
    addsd xmm0,[rdx+rsi*8]
    movsd [rdx+rsi*8],xmm0
    mov ecx,NULL_COUNT
    call RandomBelow
    imul eax,eax,MODEL_COUNT*8
    lea rbx,[rip+null_models]
    add rbx,rax
    imul eax,esi,MODEL_COUNT*8
    lea rdi,[rip+null_model_totals]
    add rdi,rax
    lea r12,[rip+model_null_mean]
    xor ecx,ecx
1:  movsd xmm0,[rbx+rcx*8]
    subsd xmm0,[r12+rcx*8]
    addsd xmm0,[rdi+rcx*8]
    movsd [rdi+rcx*8],xmm0
    inc ecx
    cmp ecx,MODEL_COUNT-1
    jb 1b
    inc esi
    cmp esi,NULL_COUNT
    jb .vh_null
    inc dword ptr [rip+validation_count]
    inc dword ptr [rip+validation_cutoff]
    mov dword ptr [rip+analysis_stage],6
    jmp .vh_fold
.vh_done:
    mov dword ptr [rip+validation_complete],1
    call ValidationGates
    mov qword ptr [rip+rank_truth_mask],0
    mov eax,1
    jmp .vh_out
.vh_cancel:
    mov dword ptr [rip+field_ready],0
    mov qword ptr [rip+rank_truth_mask],0
    xor eax,eax
.vh_out: END 32

FN ValidationGates,32
    mov qword ptr [rip+evidence],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
    lea rcx,[rip+model_weights]
    mov edx,MODEL_COUNT*8
    call Zero
    lea rcx,[rip+model_gates]
    mov edx,MODEL_COUNT*4
    call Zero
    mov ebp,[rip+validation_count]
    test ebp,ebp
    jz .vg_out
    cvtsi2sd xmm5,ebp
    movsd [rsp+32],xmm5
    movsd xmm0,[rip+validation_sum]
    divsd xmm0,xmm5
    movsd [rip+validation_mean],xmm0
    xor esi,esi
    mov edi,1
    lea rbx,[rip+null_totals]
1:  movsd xmm0,[rbx+rsi*8]
    comisd xmm0,[rip+validation_sum]
    jb 2f
    inc edi
2:  inc esi
    cmp esi,NULL_COUNT
    jb 1b
    cvtsi2sd xmm0,edi
    mov eax,NULL_COUNT+1
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    movsd [rip+validation_p],xmm0
    # Null percentile interval for average hit (not a predictive confidence).
    mov rcx,rbx
    mov edx,NULL_COUNT
    call SortDoubles
    movsd xmm0,[rbx+12*8]
    divsd xmm0,[rsp+32]
    movsd [rip+validation_lower],xmm0
    movsd xmm0,[rbx+499*8]
    divsd xmm0,[rsp+32]
    movsd [rip+validation_upper],xmm0
    movsd xmm0,[rip+validation_brier]
    divsd xmm0,[rsp+32]
    movsd [rip+validation_brier],xmm0
    movsd xmm0,[rip+validation_baseline_brier]
    divsd xmm0,[rsp+32]
    movsd [rip+validation_baseline_brier],xmm0
    xor esi,esi
    lea r12,[rip+validation_models]
.vg_model:
    xor edi,edi
    mov r13d,1
1:  imul eax,edi,MODEL_COUNT
    add eax,esi
    lea rdx,[rip+null_model_totals]
    movsd xmm0,[rdx+rax*8]
    comisd xmm0,[r12+rsi*8]
    jb 2f
    inc r13d
2:  inc edi
    cmp edi,NULL_COUNT
    jb 1b
    cvtsi2sd xmm0,r13d
    mov eax,NULL_COUNT+1
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    lea rdx,[rip+model_pvalue]
    movsd [rdx+rsi*8],xmm0
    inc esi
    cmp esi,MODEL_COUNT-1
    jb .vg_model
    lea rcx,[rip+model_pvalue]
    mov edx,MODEL_COUNT-1
    lea r8,[rip+model_qvalue]
    call BHCorrection
    # Expected calibration error on held-out number-inclusion probabilities.
    xorpd xmm5,xmm5
    lea rbx,[rip+validation_bins]
    xor esi,esi
1:  imul eax,esi,24
    movsd xmm0,[rbx+rax+8]
    subsd xmm0,[rbx+rax+16]
    andpd xmm0,[rip+abs_bits]
    addsd xmm5,xmm0
    inc esi
    cmp esi,10
    jb 1b
    divsd xmm5,[rsp+32]
    divsd xmm5,[rip+fortyfive]
    movsd [rip+validation_ece],xmm5
    cmp dword ptr [rip+history_count],120
    jb .vg_out
    cmp ebp,30
    jb .vg_out
    movsd xmm0,[rip+validation_p]
    comisd xmm0,[rip+alpha]
    jae .vg_out
    movsd xmm0,[rip+validation_brier]
    comisd xmm0,[rip+validation_baseline_brier]
    jae .vg_out
    # OCSG + MOM redundancy correction and FECL bounded total weight.
    xor esi,esi
    lea rbx,[rip+model_qvalue]
    lea rdi,[rip+model_weights]
    xorpd xmm5,xmm5
.vg_gate:
    cmp esi,[rip+research_excluded]
    je .vg_next
    movsd xmm0,[rbx+rsi*8]
    comisd xmm0,[rip+alpha]
    jae .vg_next
    lea rdx,[rip+validation_models]
    movsd xmm1,[rdx+rsi*8]
    comisd xmm1,[rip+zero]
    jbe .vg_next
    lea rdx,[rip+validation_ablation]
    movsd xmm2,[rdx+rsi*8]
    comisd xmm2,[rip+zero]
    jbe .vg_next
    movsd xmm2,[rip+one]
    subsd xmm2,xmm0
    mulsd xmm1,xmm2
    divsd xmm1,[rsp+32]
    imul eax,esi,MODEL_COUNT
    lea rdx,[rip+model_corr]
    lea rdx,[rdx+rax*8]
    xor ecx,ecx
    movsd xmm2,[rip+one]
1:  cmp ecx,esi
    je 2f
    movsd xmm0,[rdx+rcx*8]
    andpd xmm0,[rip+abs_bits]
    addsd xmm2,xmm0
2:  inc ecx
    cmp ecx,MODEL_COUNT-1
    jb 1b
    divsd xmm1,xmm2
    movsd [rdi+rsi*8],xmm1
    addsd xmm5,xmm1
    lea rdx,[rip+model_gates]
    mov dword ptr [rdx+rsi*4],1
.vg_next:
    inc esi
    cmp esi,MODEL_COUNT-1
    jb .vg_gate
    comisd xmm5,[rip+epsilon]
    jbe .vg_out
    movsd xmm0,[rip+validation_mean]
    subsd xmm0,[rip+expected_hits]
    maxsd xmm0,[rip+zero]
    minsd xmm0,[rip+one]
    movsd xmm1,[rip+one]
    subsd xmm1,[rip+validation_p]
    mulsd xmm0,xmm1
    movsd [rip+evidence],xmm0
    movapd xmm1,xmm0
    mulsd xmm1,[rip+half]
    movsd xmm2,[rip+one]
    subsd xmm2,xmm1
    movsd [rip+rho],xmm2
    xor esi,esi
1:  movsd xmm1,[rdi+rsi*8]
    divsd xmm1,xmm5
    movsd [rdi+rsi*8],xmm1
    inc esi
    cmp esi,MODEL_COUNT-1
    jb 1b
.vg_out: END 32

FN AnalyzeOracle
    mov dword ptr [rip+field_ready],0
    mov dword ptr [rip+model_ready],0
    mov dword ptr [rip+analysis_stage],1
    cmp dword ptr [rip+history_count],60
    jb .ao_fail
    call ValidateHistory
    test eax,eax
    jz .ao_fail
    call ConfirmPolicy
    test eax,eax
    jz .ao_fail
    mov ecx,[rip+history_count]
    call RebuildStats
    call PrepareModels
    test eax,eax
    jz .ao_fail
    call BuildNormalization
    call ScanField
    test eax,eax
    jz .ao_fail
    call BuildGeometry
    test eax,eax
    jz .ao_fail
    call BuildWorlds
    test eax,eax
    jz .ao_fail
    call BuildThermal
    mov dword ptr [rip+rng_mode],0
    mov dword ptr [rip+analysis_stage],7
    mov dword ptr [rip+progress],100
    mov eax,1
    jmp .ao_out
.ao_fail:
    mov dword ptr [rip+field_ready],0
    mov dword ptr [rip+rng_mode],0
    mov qword ptr [rip+evidence],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
    xor eax,eax
.ao_out: END

# Fixed learned policy is fitted only on origins before the reserved last 30.
# Refit features on each prefix, never weights on confirmation outcomes.
# No promotion without both a matched-null hit test and improved Brier loss.
FN ConfirmPolicy,32
    movsd xmm0,[rip+evidence]
    comisd xmm0,[rip+zero]
    jbe .cp_skip
    lea rcx,[rip+confirmation_null]
    mov edx,NULL_COUNT*8
    call Zero
    mov ebp,[rip+validation_stop]
    mov qword ptr [rip+rng_state],420260913
.cp_fold:
    cmp ebp,[rip+backtest_end_index]
    jae .cp_gate
    cmp dword ptr [rip+cancel_flag],0
    jne .cp_cancel
    mov ecx,ebp
    call RebuildStats
    call PrepareModels
    test eax,eax
    jz .cp_cancel
    call BuildNormalization
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov rax,[rdx+rax+40]
    mov [rip+validation_truth],rax
    mov [rip+rank_truth_mask],rax
    call ScanField
    test eax,eax
    jz .cp_cancel
    call BuildGeometry
    test eax,eax
    jz .cp_cancel
    call BuildWorlds
    test eax,eax
    jz .cp_cancel
    call BuildThermal
    call ResearchLogConfirmation
    call SampleMask
    mov [rip+validation_pick],rax
    mov rcx,rax
    and rcx,[rip+validation_truth]
    call PopCount64
    cvtsi2sd xmm0,eax
    movsd [rsp+32],xmm0
    addsd xmm0,[rip+confirmation_hits]
    movsd [rip+confirmation_hits],xmm0
    mov eax,[rip+confirmation_count]
    shl eax,6
    lea rbx,[rip+confirmation_rows]
    add rbx,rax
    imul eax,ebp,RECORD_SIZE
    lea rdx,[rip+history]
    mov eax,[rdx+rax]
    mov [rbx],eax
    mov [rbx+4],ebp
    mov rax,[rip+validation_pick]
    mov [rbx+8],rax
    mov rax,[rip+validation_truth]
    mov [rbx+16],rax
    movsd xmm0,[rsp+32]
    movsd [rbx+24],xmm0
    mov eax,[rip+rank_truth_position]
    mov [rbx+32],eax
    xor esi,esi
    xorpd xmm5,xmm5
    lea rdi,[rip+mixture_marginals]
1:  movsd xmm0,[rdi+rsi*8]
    mov rax,[rip+validation_truth]
    bt rax,rsi
    jnc 2f
    subsd xmm0,[rip+one]
2:  mulsd xmm0,xmm0
    addsd xmm5,xmm0
    inc esi
    cmp esi,45
    jb 1b
    divsd xmm5,[rip+fortyfive]
    movsd [rbx+40],xmm5
    addsd xmm5,[rip+confirmation_brier]
    movsd [rip+confirmation_brier],xmm5
    xor esi,esi
.cp_null:
    call GenerateUniformMask
    mov rcx,rax
    and rcx,[rip+validation_truth]
    call PopCount64
    cvtsi2sd xmm0,eax
    lea rdx,[rip+confirmation_null]
    addsd xmm0,[rdx+rsi*8]
    movsd [rdx+rsi*8],xmm0
    inc esi
    cmp esi,NULL_COUNT
    jb .cp_null
    inc dword ptr [rip+confirmation_count]
    inc ebp
    jmp .cp_fold
.cp_gate:
    xor esi,esi
    mov edi,1
    lea rbx,[rip+confirmation_null]
1:  movsd xmm0,[rbx+rsi*8]
    comisd xmm0,[rip+confirmation_hits]
    jb 2f
    inc edi
2:  inc esi
    cmp esi,NULL_COUNT
    jb 1b
    cvtsi2sd xmm0,edi
    mov eax,NULL_COUNT+1
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    movsd [rip+confirmation_p],xmm0
    cvtsi2sd xmm1,dword ptr [rip+confirmation_count]
    movsd xmm2,[rip+confirmation_brier]
    divsd xmm2,xmm1
    movsd [rip+confirmation_brier],xmm2
    mov dword ptr [rip+confirmation_status],2
    cmp dword ptr [rip+confirmation_count],30
    jne .cp_reject
    comisd xmm0,[rip+alpha]
    jae .cp_reject
    comisd xmm2,[rip+validation_baseline_brier]
    jae .cp_reject
    mov dword ptr [rip+confirmation_status],1
    jmp .cp_skip
.cp_reject:
    mov qword ptr [rip+evidence],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
    lea rcx,[rip+model_weights]
    mov edx,MODEL_COUNT*8
    call Zero
    lea rcx,[rip+model_gates]
    mov edx,MODEL_COUNT*4
    call Zero
.cp_skip:
    mov qword ptr [rip+rank_truth_mask],0
    mov eax,1
    jmp .cp_out
.cp_cancel:
    xor eax,eax
.cp_out: END 32
