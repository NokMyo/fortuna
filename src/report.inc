.section .rdata,"dr"
r_header: .asciz "\357\273\277Febius Fortuna 1.0.0 / ORACLE reference report\r\nInternal scores and sampling weights are NOT winning probabilities.\r\nFair jackpot probability: 1 / 8145060.\r\n\r\nDataset SHA-256: "
r_config: .asciz "\r\nConfig SHA-256: "
r_field: .asciz "\r\nCandidate-field SHA-256: "
r_count: .asciz "\r\nDraws / latest / target: "
r_slash: .asciz " / "
r_newline: .asciz "\r\n"
r_comma: .asciz ","
r_validation: .asciz "\r\nFULL WALK-FORWARD / fixed descriptive challenger\r\nTraining starts at 60 draws; origins precede the reserved final 30 when N >= 120.\r\nEach fold refits all models, exhaustive scan, geometry and counterfactual worlds.\r\nFolds,mean_hits,matched_null_p,null_mean_95lo,null_mean_95hi,brier,uniform_brier,ECE\r\n"
r_confirmation: .asciz "\r\nIndependent policy confirmation (0=not eligible,1=pass,2=reject)\r\nstatus,folds,total_hits,brier,matched_null_p\r\n"
r_policy: .asciz "\r\nValidated sampling policy\r\nEvidence budget,uniform_mixture,temperature,field_entropy\r\n"
r_models: .asciz "\r\nModels (weights multiply the evidence budget; HSSM is a separate heuristic)\r\nmodel,p,q_BH,gate,relative_weight,robust_center,robust_scale\r\n"
r_modelnames:
.quad r_m0,r_m1,r_m2,r_m3,r_m4,r_m5,r_m6,r_m7,r_m8,r_m9,r_m10,r_m11
r_m0: .asciz "FNG/Bayesian number"
r_m1: .asciz "OTRK temporal resonance"
r_m2: .asciz "TPA temporal agreement"
r_m3: .asciz "Shrunk discrete hazard"
r_m4: .asciz "Null-centered pair graph"
r_m5: .asciz "Decayed pair graph"
r_m6: .asciz "FSL spectral lattice"
r_m7: .asciz "OHC hyperphase coupling"
r_m8: .asciz "OHPS Mahalanobis shell"
r_m9: .asciz "FEB five-partition entropy"
r_m10: .asciz "OIS interaction synergy"
r_m11: .asciz "HSSM heuristic crowd shadow"
r_numbers: .asciz "\r\nNumber,persistence_in_field,policy_inclusion_probability\r\n"
r_candidates: .asciz "\r\nConditional candidate field (not global jackpot ranks)\r\nmask,n1,n2,n3,n4,n5,n6,descriptive_score,energy,oracle_probability,OBG,FBV,FSS,neighbor_variance,COS,FRP10,FRP100,median_world_rank\r\n"
r_confirm_rows: .asciz "\r\nIndependent confirmation folds (same columns)\r\n"
r_folds: .asciz "\r\nHeld-out folds\r\nround,training_rows,pick_mask,actual_mask,hits,actual_fast_rank_of_8145060,brier\r\n"
r_limits: .asciz "\r\nMethod notes\r\nCounterfactual ranks are conditional on 512 survivors; 32 worlds.\r\nMonte Carlo uses 512 matched baselines; p resolution is 1/513.\r\nModel OMD tests use held-out normalized-score utility; OCSG uses leave-one-model-out additive utility.\r\nAt N >= 120, learned weights require a separate last-30 confirmation before promotion.\r\nEvidence is advisory; no predictive superiority is asserted. HSSM has no ticket-sales calibration.\r\nA local hash chain detects edits relative to a trusted anchor; it is not external timestamp proof.\r\nSee docs/IMPLEMENTATION.md for exact implemented formulas and unimplemented research extensions.\r\n"
hex_digits: .ascii "0123456789abcdef"
.section .bss
report_buffer: .skip 2097152
report_cursor: .skip 8
report_error: .skip 4
report_numbers: .skip 6*4
.section .text
FN Text
    mov rdx,[rip+report_cursor]
    lea r8,[rip+report_buffer+2097151]
1:  mov al,[rcx]
    test al,al
    jz 3f
    cmp rdx,r8
    jae 2f
    mov [rdx],al
    inc rcx
    inc rdx
    jmp 1b
2:  mov dword ptr [rip+report_error],1
3:  mov byte ptr [rdx],0
    mov [rip+report_cursor],rdx
    END
FN Number,32
    lea rdi,[rsp+63]
    mov byte ptr [rdi],0
    mov rax,rcx
    mov ebx,10
1:  xor edx,edx
    div rbx
    add dl,'0'
    dec rdi
    mov [rdi],dl
    test rax,rax
    jnz 1b
    mov rcx,rdi
    call Text
    END 32
FN SignedNumber,16
    mov rbx,rcx
    test rcx,rcx
    jns 1f
    mov word ptr [rsp+32],'-'
    lea rcx,[rsp+32]
    call Text
    neg rbx
1:  mov rcx,rbx
    call Number
    END 16
FN Decimal,48
    ucomisd xmm0,xmm0
    jp .dec_bad
    comisd xmm0,[rip+zero]
    jae 1f
    andpd xmm0,[rip+abs_bits]
    movsd [rsp+32],xmm0
    mov word ptr [rsp+40],'-'
    lea rcx,[rsp+40]
    call Text
    movsd xmm0,[rsp+32]
1:  cvttsd2si rbx,xmm0
    cvtsi2sd xmm1,rbx
    subsd xmm0,xmm1
    mov eax,1000000000
    cvtsi2sd xmm1,eax
    mulsd xmm0,xmm1
    cvttsd2si rbp,xmm0
    mov rcx,rbx
    call Number
    mov byte ptr [rsp+48],'.'
    mov byte ptr [rsp+58],0
    mov rax,rbp
    mov ebx,10
    mov esi,9
2:  xor edx,edx
    div rbx
    add dl,'0'
    mov [rsp+48+rsi],dl
    dec esi
    jnz 2b
    lea rcx,[rsp+48]
    call Text
    jmp .dec_out
.dec_bad:
    mov dword ptr [rip+report_error],1
.dec_out: END 48
FN HexHash,80
    mov rbx,rcx
    lea rdi,[rsp+32]
    lea rdx,[rip+hex_digits]
    xor esi,esi
1:  movzx eax,byte ptr [rbx+rsi]
    mov ecx,eax
    shr eax,4
    and ecx,15
    mov al,[rdx+rax]
    mov cl,[rdx+rcx]
    mov [rdi+rsi*2],al
    mov [rdi+rsi*2+1],cl
    inc esi
    cmp esi,32
    jb 1b
    mov byte ptr [rdi+64],0
    mov rcx,rdi
    call Text
    END 80
FN Comma
    lea rcx,[rip+r_comma]
    call Text
    END
FN Newline
    lea rcx,[rip+r_newline]
    call Text
    END

FN BuildReport
    lea rax,[rip+report_buffer]
    mov [rip+report_cursor],rax
    mov dword ptr [rip+report_error],0
    lea rcx,[rip+r_header]
    call Text
    lea rcx,[rip+hash_dataset]
    call HexHash
    lea rcx,[rip+r_config]
    call Text
    lea rcx,[rip+hash_config]
    call HexHash
    lea rcx,[rip+r_field]
    call Text
    lea rcx,[rip+hash_field]
    call HexHash
    lea rcx,[rip+r_count]
    call Text
    mov ecx,[rip+history_count]
    call Number
    lea rcx,[rip+r_slash]
    call Text
    mov ecx,[rip+last_round]
    call Number
    lea rcx,[rip+r_slash]
    call Text
    mov ecx,[rip+last_round]
    inc ecx
    call Number
    lea rcx,[rip+r_validation]
    call Text
    mov ecx,[rip+validation_count]
    call Number
    call Comma
    movsd xmm0,[rip+validation_mean]
    call Decimal
    call Comma
    movsd xmm0,[rip+validation_p]
    call Decimal
    call Comma
    movsd xmm0,[rip+validation_lower]
    call Decimal
    call Comma
    movsd xmm0,[rip+validation_upper]
    call Decimal
    call Comma
    movsd xmm0,[rip+validation_brier]
    call Decimal
    call Comma
    movsd xmm0,[rip+validation_baseline_brier]
    call Decimal
    call Comma
    movsd xmm0,[rip+validation_ece]
    call Decimal
    call Newline
    lea rcx,[rip+r_confirmation]
    call Text
    mov ecx,[rip+confirmation_status]
    call Number
    call Comma
    mov ecx,[rip+confirmation_count]
    call Number
    call Comma
    movsd xmm0,[rip+confirmation_hits]
    call Decimal
    call Comma
    movsd xmm0,[rip+confirmation_brier]
    call Decimal
    call Comma
    movsd xmm0,[rip+confirmation_p]
    call Decimal
    call Newline
    lea rcx,[rip+r_policy]
    call Text
    movsd xmm0,[rip+evidence]
    call Decimal
    call Comma
    movsd xmm0,[rip+rho]
    call Decimal
    call Comma
    movsd xmm0,[rip+temperature]
    call Decimal
    call Comma
    movsd xmm0,[rip+field_entropy]
    call Decimal
    call Newline
    lea rcx,[rip+r_models]
    call Text
    xor esi,esi
.br_model:
    lea rax,[rip+r_modelnames]
    mov rcx,[rax+rsi*8]
    call Text
    call Comma
    lea rax,[rip+model_pvalue]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Comma
    lea rax,[rip+model_qvalue]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Comma
    lea rax,[rip+model_gates]
    mov ecx,[rax+rsi*4]
    call Number
    call Comma
    lea rax,[rip+model_weights]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Comma
    lea rax,[rip+norm_center]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Comma
    lea rax,[rip+norm_scale]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Newline
    inc esi
    cmp esi,MODEL_COUNT
    jb .br_model
    lea rcx,[rip+r_numbers]
    call Text
    xor esi,esi
.br_number:
    lea ecx,[rsi+1]
    call Number
    call Comma
    lea rax,[rip+persistence]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Comma
    lea rax,[rip+mixture_marginals]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Newline
    inc esi
    cmp esi,45
    jb .br_number
    lea rcx,[rip+r_candidates]
    call Text
    xor esi,esi
.br_candidate:
    cmp esi,[rip+field_count]
    jae .br_folds_start
    lea rax,[rip+final_masks]
    mov rcx,[rax+rsi*8]
    mov rbx,rcx
    call Number
    mov rcx,rbx
    lea rdx,[rip+report_numbers]
    call MaskToNumbers
    xor edi,edi
1:  call Comma
    lea rax,[rip+report_numbers]
    mov ecx,[rax+rdi*4]
    call Number
    inc edi
    cmp edi,6
    jb 1b
    call Comma
    lea rax,[rip+final_scores]
    mov rcx,[rax+rsi*8]
    call SignedNumber
    call Comma
    lea rax,[rip+field_energies]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    call Comma
    lea rax,[rip+final_prob]
    movsd xmm0,[rax+rsi*8]
    call Decimal
    xor edi,edi
2:  call Comma
    imul eax,esi,4
    add eax,edi
    lea rdx,[rip+geometry]
    movsd xmm0,[rdx+rax*8]
    call Decimal
    inc edi
    cmp edi,4
    jb 2b
    xor edi,edi
3:  call Comma
    imul eax,esi,4
    add eax,edi
    lea rdx,[rip+world_summary]
    movsd xmm0,[rdx+rax*8]
    call Decimal
    inc edi
    cmp edi,4
    jb 3b
    call Newline
    inc esi
    jmp .br_candidate
.br_folds_start:
    lea rcx,[rip+r_folds]
    call Text
    xor esi,esi
    xor r14d,r14d
    mov r15d,[rip+validation_count]
.br_fold:
    cmp esi,r15d
    jae .br_next_fold_set
    imul eax,esi,64
    lea rbx,[rip+validation_rows]
    test r14d,r14d
    jz 4f
    lea rbx,[rip+confirmation_rows]
4:  add rbx,rax
    mov ecx,[rbx]
    call Number
    call Comma
    mov ecx,[rbx+4]
    call Number
    call Comma
    mov rcx,[rbx+8]
    call Number
    call Comma
    mov rcx,[rbx+16]
    call Number
    call Comma
    movsd xmm0,[rbx+24]
    call Decimal
    call Comma
    mov ecx,[rbx+32]
    call Number
    call Comma
    movsd xmm0,[rbx+40]
    call Decimal
    call Newline
    inc esi
    jmp .br_fold
.br_next_fold_set:
    test r14d,r14d
    jnz .br_finish
    mov r14d,1
    xor esi,esi
    mov r15d,[rip+confirmation_count]
    lea rcx,[rip+r_confirm_rows]
    call Text
    jmp .br_fold
.br_finish:
    lea rcx,[rip+r_limits]
    call Text
    mov rax,[rip+report_cursor]
    lea rdx,[rip+report_buffer]
    sub rax,rdx
    cmp dword ptr [rip+report_error],0
    je 1f
    xor eax,eax
1:  END
