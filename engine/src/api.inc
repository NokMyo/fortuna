.globl FortunaOracleGetAbiVersion
.globl FortunaOracleGetEngineVersion
.globl FortunaOracleInitialize
.globl FortunaOracleSetBacktestRange
.globl FortunaOracleLoadCsv
.globl FortunaOracleLoadCsvFileW
.globl FortunaOracleAnalyze
.globl FortunaOracleCancel
.globl FortunaOracleGenerate
.globl FortunaOracleGetSnapshot
.globl FortunaOracleGetProgress
.globl FortunaOracleGetReport
.globl FortunaOracleSealW
.globl FortunaOracleUseUniform
.globl FortunaOracleSelfTest
# Public ABI v1. All heavy state is private to this DLL, not exported to clients.
# One state per process. Mutating calls are serialized; concurrent calls fail busy.
.section .bss
.p2align 3
api_lock: .skip 8
api_initialized: .skip 4
.section .text
FN FortunaOracleGetAbiVersion
    mov eax,0x10000
    END
FN FortunaOracleGetEngineVersion
    mov eax,0x10600
    END
FN ApiLock
    lea rcx,[rip+api_lock]
    call TryAcquireSRWLockExclusive
    test al,al
    jz .al_busy
    cmp dword ptr [rip+api_initialized],1
    je .al_ok
    lea rcx,[rip+api_lock]
    call ReleaseSRWLockExclusive
    mov eax,-4
    jmp .al_out
.al_ok: xor eax,eax; jmp .al_out
.al_busy: mov eax,-2
.al_out: END
FN ApiFinish
    mov ebx,ecx
    lea rcx,[rip+api_lock]
    call ReleaseSRWLockExclusive
    mov eax,ebx
    END
FN FortunaOracleInitialize
    lea rcx,[rip+api_lock]
    call TryAcquireSRWLockExclusive
    test al,al
    jz .init_busy
    cmp dword ptr [rip+api_initialized],1
    je .init_ok
    lea rcx,[rip+config_version]
    xor edx,edx
1:  cmp byte ptr [rcx+rdx],0
    je 2f
    inc edx
    jmp 1b
2:  lea r8,[rip+hash_config]
    call Sha256
    call StartupCheck
    test eax,eax
    jz .init_fail
    mov dword ptr [rip+api_initialized],1
.init_ok: xor ecx,ecx; jmp .init_finish
.init_fail: mov ecx,-5
.init_finish: call ApiFinish; jmp .init_out
.init_busy: mov eax,-2
.init_out: END
FN FortunaOracleSetBacktestRange
    mov ebx,ecx
    mov esi,edx
    call ApiLock
    test eax,eax
    jnz .sbr_out
    test ebx,ebx
    jnz .sbr_custom
    test esi,esi
    jnz .sbr_arg_locked
    jmp .sbr_valid
.sbr_custom:
    test esi,esi
    jz .sbr_arg_locked
    mov eax,[rip+history_count]
    cmp eax,120
    jb .sbr_arg_locked
    mov r8d,[rip+history]
    lea eax,[r8+60]
    cmp ebx,eax
    jb .sbr_arg_locked
    cmp esi,[rip+last_round]
    ja .sbr_arg_locked
    cmp esi,ebx
    jb .sbr_arg_locked
    mov eax,esi
    sub eax,ebx
    inc eax
    cmp eax,60
    jb .sbr_arg_locked
.sbr_valid:
    cmp ebx,[rip+backtest_start_round]
    jne .sbr_change
    cmp esi,[rip+backtest_end_round]
    je .sbr_success
.sbr_change:
    mov [rip+backtest_start_round],ebx
    mov [rip+backtest_end_round],esi
    mov dword ptr [rip+model_ready],0
    mov dword ptr [rip+field_ready],0
    mov dword ptr [rip+field_count],0
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+advanced_ready],0
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+forecast_valid],0
    mov dword ptr [rip+validation_complete],0
    mov dword ptr [rip+validation_count],0
    mov dword ptr [rip+session_count],0
    mov qword ptr [rip+selected_mask],0
    mov dword ptr [rip+selected_index],0
    mov qword ptr [rip+evidence],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
.sbr_success:
    xor ecx,ecx
    call ApiFinish
    jmp .sbr_out
.sbr_arg_locked:
    mov ecx,-1
    call ApiFinish
.sbr_out: END

FN FortunaOracleLoadCsv
    test rcx,rcx
    jz .lc_arg
    test rdx,rdx
    jz .lc_arg
    cmp rdx,16777216
    ja .lc_arg
    mov rbx,rcx
    mov rsi,rdx
    call ApiLock
    test eax,eax
    jnz .lc_out
    mov rcx,rbx
    mov rdx,rsi
    call ParseCsv
    xor ecx,ecx
    test eax,eax
    jnz 1f
    mov ecx,-3
1:  call ApiFinish
    jmp .lc_out
.lc_arg: mov eax,-1
.lc_out: END
FN FortunaOracleLoadCsvFileW
    test rcx,rcx
    jz .lfw_arg
    mov rbx,rcx
    call ApiLock
    test eax,eax
    jnz .lfw_out
    mov rcx,rbx
    call LoadCsvFile
    xor ecx,ecx
    test eax,eax
    jnz 1f
    mov ecx,-3
1:  call ApiFinish
    jmp .lfw_out
.lfw_arg: mov eax,-1
.lfw_out: END
FN FortunaOracleAnalyze
    call ApiLock
    test eax,eax
    jnz .ana_out
    mov dword ptr [rip+cancel_flag],0
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+forecast_valid],0
    call AnalyzeOracle
    xor ecx,ecx
    test eax,eax
    jnz 1f
    mov ecx,-3
    cmp dword ptr [rip+cancel_flag],1
    jne 1f
    mov ecx,-6
1:  call ApiFinish
.ana_out: END
FN FortunaOracleCancel
    mov eax,1
    xchg [rip+cancel_flag],eax
    xor eax,eax
    END
FN FortunaOracleGenerate
    cmp ecx,1
    jb .gen_arg
    cmp ecx,10
    ja .gen_arg
    test rdx,rdx
    jz .gen_arg
    cmp r8d,ecx
    jb .gen_arg
    mov ebx,ecx
    mov rsi,rdx
    call ApiLock
    test eax,eax
    jnz .gen_out
    mov ecx,ebx
    call GenerateTickets
    test eax,eax
    jz .gen_fail
    mov rcx,rsi
    lea rdx,[rip+session_masks]
    lea r8d,[rbx*8]
    call Copy
    xor ecx,ecx
    jmp .gen_finish
.gen_fail: mov ecx,-5
.gen_finish: call ApiFinish; jmp .gen_out
.gen_arg: mov eax,-1
.gen_out: END
FN FortunaOracleGetSnapshot
    test rcx,rcx
    jz .snap_arg
    cmp edx,272
    jb .snap_arg
    mov rbx,rcx
    call ApiLock
    test eax,eax
    jnz .snap_out
    mov rcx,rbx
    mov edx,272
    call Zero
    mov dword ptr [rbx],272
    mov dword ptr [rbx+4],0x10000
    mov dword ptr [rbx+8],0x10600
    mov eax,[rip+model_ready]
    shl eax,1
    or eax,[rip+field_ready]
    mov [rbx+12],eax
    mov eax,[rip+history_count]
    mov [rbx+16],eax
    mov eax,[rip+last_round]
    mov [rbx+20],eax
    mov eax,[rip+parse_error]
    mov [rbx+24],eax
    mov eax,[rip+parse_line]
    mov [rbx+28],eax
    mov eax,[rip+field_count]
    mov [rbx+32],eax
    mov eax,[rip+validation_count]
    mov [rbx+36],eax
    mov eax,[rip+selected_index]
    mov [rbx+40],eax
    mov eax,[rip+session_count]
    mov [rbx+44],eax
    mov rax,[rip+selected_mask]
    mov [rbx+48],rax
    lea rcx,[rbx+56]
    lea rdx,[rip+selected_nums]
    mov r8d,24
    call Copy
    lea rcx,[rbx+80]
    lea rdx,[rip+session_masks]
    mov r8d,80
    call Copy
    lea rcx,[rbx+160]
    lea rdx,[rip+rho]
    mov r8d,8
    call Copy
    lea rcx,[rbx+168]
    lea rdx,[rip+evidence]
    mov r8d,8
    call Copy
    lea rcx,[rbx+176]
    lea rdx,[rip+hash_dataset]
    mov r8d,32
    call Copy
    lea rcx,[rbx+208]
    lea rdx,[rip+hash_field]
    mov r8d,32
    call Copy
    lea rcx,[rbx+240]
    lea rdx,[rip+hash_config]
    mov r8d,32
    call Copy
    xor ecx,ecx
    call ApiFinish
    jmp .snap_out
.snap_arg: mov eax,-1
.snap_out: END
# Approximate independently sampled counters; callable while analysis owns lock.
FN FortunaOracleGetProgress
    test rcx,rcx
    jz .prog_arg
    cmp edx,16
    jb .prog_arg
    mov eax,[rip+analysis_stage]
    mov [rcx],eax
    mov eax,[rip+progress]
    mov [rcx+4],eax
    cmp dword ptr [rip+research_active],1
    jne 1f
    mov eax,[rip+research_ablation_count]
    cmp eax,13
    jae 1f
    mov dword ptr [rcx],8
    imul eax,eax,100
    xor edx,edx
    mov r8d,13
    div r8d
    mov [rcx+4],eax
1:
    mov eax,[rip+validation_count]
    mov [rcx+8],eax
    mov eax,[rip+history_count]
    mov [rcx+12],eax
    xor eax,eax
    jmp .prog_out
.prog_arg: mov eax,-1
.prog_out: END
# Return required bytes including terminating NUL. Null/zero is a size query.
# An undersized buffer is not written. No internal memory pointer escapes.
FN FortunaOracleGetReport
    test rcx,rcx
    jnz 1f
    test edx,edx
    jnz .rep_arg
1:  mov rbx,rcx
    mov esi,edx
    call ApiLock
    test eax,eax
    jnz .rep_out
    cmp dword ptr [rip+field_ready],1
    jne .rep_state
    call BuildReport
    test eax,eax
    jz .rep_state
    lea edi,[rax+1]
    test rbx,rbx
    jz .rep_size
    cmp esi,edi
    jb .rep_size
    mov rcx,rbx
    lea rdx,[rip+report_buffer]
    mov r8d,edi
    call Copy
.rep_size: mov ecx,edi; jmp .rep_finish
.rep_state: mov ecx,-4
.rep_finish: call ApiFinish; jmp .rep_out
.rep_arg: mov eax,-1
.rep_out: END
FN FortunaOracleSealW
    test rcx,rcx
    jz .seal_api_arg
    mov rbx,rcx
    xor esi,esi
1:  cmp word ptr [rbx+rsi*2],0
    je 2f
    inc esi
    cmp esi,1023
    jb 1b
    jmp .seal_api_arg
2:  test esi,esi
    jz .seal_api_arg
    call ApiLock
    test eax,eax
    jnz .seal_api_out
    lea rcx,[rip+ledger_path]
    mov rdx,rbx
    lea r8d,[rsi*2+2]
    call Copy
    call SealPick
    mov ecx,-3
    test eax,eax
    jz 3f
    lea ecx,[rax-1]
3:  call ApiFinish
    jmp .seal_api_out
.seal_api_arg: mov eax,-1
.seal_api_out: END
FN FortunaOracleUseUniform
    call ApiLock
    test eax,eax
    jnz .uniform_out
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+field_ready],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
    xor ecx,ecx
    call ApiFinish
.uniform_out: END
# Diagnostic is destructive to the process-local engine state. No user files.
FN FortunaOracleSelfTest
    call ApiLock
    test eax,eax
    jnz .test_out
    mov dword ptr [rip+cancel_flag],0
    call SelfTest
    mov ecx,eax
    call ApiFinish
.test_out: END

.globl FortunaOracleAnalyzeResearch
.globl FortunaOracleGetJointProbability
.globl FortunaOracleForecastLedgerW
.globl FortunaOracleGetResearchSnapshot
# flags bit0 requests exhaustive deep scoring (very expensive); other bits invalid.
FN FortunaOracleAnalyzeResearch
    cmp ecx,1
    ja .arapi_arg
    mov ebx,ecx
    call ApiLock
    test eax,eax
    jnz .arapi_out
    mov [rip+research_full_scan],ebx
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+cancel_flag],0
    mov dword ptr [rip+forecast_valid],0
    call AnalyzeResearch
    xor ecx,ecx
    test eax,eax
    jnz 1f
    mov ecx,-3
    cmp dword ptr [rip+cancel_flag],1
    jne 1f
    mov ecx,-6
1:  call ApiFinish
    jmp .arapi_out
.arapi_arg: mov eax,-1
.arapi_out: END
# out receives [joint model probability, 50%-uniform forecast probability].
FN FortunaOracleGetJointProbability
    test rdx,rdx
    jz .jp_arg
    mov rbx,rcx
    mov rsi,rdx
    mov rax,rcx
    shr rax,45
    jnz .jp_arg
    call PopCount64
    cmp eax,6
    jne .jp_arg
    call ApiLock
    test eax,eax
    jnz .jp_out
    cmp dword ptr [rip+research_ready],1
    jne .jp_state
    mov rcx,rbx
    call JointLogProbability
    call Exp
    movsd [rsi],xmm0
    mov rcx,rbx
    call JointForecastLog
    call Exp
    movsd [rsi+8],xmm0
    xor ecx,ecx
    jmp .jp_finish
.jp_state: mov ecx,-4
.jp_finish: call ApiFinish; jmp .jp_out
.jp_arg: mov eax,-1
.jp_out: END
FN FortunaOracleForecastLedgerW
    test rcx,rcx
    jz .fla_arg
    cmp edx,1
    ja .fla_arg
    mov rbx,rcx
    mov esi,edx
    call ApiLock
    test eax,eax
    jnz .fla_out
    mov rcx,rbx
    mov edx,esi
    call ForecastLedger
    mov ecx,-3
    test eax,eax
    jz 1f
    lea ecx,[rax-1]
1:  call ApiFinish
    jmp .fla_out
.fla_arg: mov eax,-1
.fla_out: END
FN FortunaOracleGetResearchSnapshot
    test rcx,rcx
    jz .rs_arg
    cmp edx,128
    jb .rs_arg
    mov rbx,rcx
    call ApiLock
    test eax,eax
    jnz .rs_out
    mov rcx,rbx
    mov edx,128
    call Zero
    mov dword ptr [rbx],128
    mov eax,[rip+research_ready]
    mov [rbx+4],eax
    mov eax,[rip+research_ablation_count]
    mov [rbx+8],eax
    mov eax,[rip+research_outer_count]
    mov [rbx+12],eax
    mov eax,[rip+research_world_count]
    mov [rbx+16],eax
    mov eax,[rip+research_mc_converged]
    mov [rbx+20],eax
    mov eax,[rip+forecast_evaluated]
    mov [rbx+24],eax
    mov eax,[rip+forecast_pending]
    mov [rbx+28],eax
    mov rax,[rip+joint_logz]
    mov [rbx+32],rax
    mov rax,[rip+research_log_e]
    mov [rbx+40],rax
    mov rax,[rip+forecast_log_e]
    mov [rbx+48],rax
    mov rax,[rip+forecast_max_log_e]
    mov [rbx+56],rax
    mov rax,[rip+research_joint_loss]
    mov [rbx+64],rax
    mov rax,[rip+research_mc_error]
    mov [rbx+72],rax
    mov rax,[rip+research_data_variance]
    mov [rbx+80],rax
    mov rax,[rip+research_disagreement]
    mov [rbx+88],rax
    mov rax,[rip+research_aleatoric]
    mov [rbx+96],rax
    mov eax,[rip+audit_samples]
    mov [rbx+104],eax
    mov eax,[rip+audit_misses_before]
    mov [rbx+108],eax
    mov eax,[rip+audit_misses_after]
    mov [rbx+112],eax
    mov eax,[rip+audit_capacity]
    mov [rbx+116],eax
    mov eax,[rip+forecast_valid]
    mov [rbx+120],eax
    xor ecx,ecx
    call ApiFinish
    jmp .rs_out
.rs_arg: mov eax,-1
.rs_out: END

.globl FortunaOracleGenerateResearch
# Research-only sampling, never represented as a validated production policy.
FN FortunaOracleGenerateResearch
    cmp ecx,1
    jb .gr_arg
    cmp ecx,10
    ja .gr_arg
    test rdx,rdx
    jz .gr_arg
    cmp r8d,ecx
    jb .gr_arg
    mov ebp,ecx
    mov rbx,rdx
    call ApiLock
    test eax,eax
    jnz .gr_out
    cmp dword ptr [rip+research_ready],1
    jne .gr_state
    mov dword ptr [rip+rng_mode],0
    mov dword ptr [rip+rng_error],0
    mov dword ptr [rip+session_count],0
    xor esi,esi
.gr_sample:
    call JointSample
    test rax,rax
    jz .gr_random
    lea rdx,[rip+session_masks]
    mov [rdx+rsi*8],rax
    inc esi
    cmp esi,ebp
    jb .gr_sample
    # Independent research draws may repeat; no post-hoc diversification.
    mov [rip+session_count],ebp
    mov rcx,[rip+session_masks]
    call SelectMask
    mov rcx,rbx
    lea rdx,[rip+session_masks]
    lea r8d,[rbp*8]
    call Copy
    xor ecx,ecx
    jmp .gr_finish
.gr_random: mov ecx,-5; jmp .gr_finish
.gr_state: mov ecx,-4
.gr_finish: call ApiFinish; jmp .gr_out
.gr_arg: mov eax,-1
.gr_out: END

.globl FortunaOracleGenerateUniform
# Independent uniform draws preserve all cached analysis and research state.
FN FortunaOracleGenerateUniform
    cmp ecx,1
    jb .gu_arg
    cmp ecx,10
    ja .gu_arg
    test rdx,rdx
    jz .gu_arg
    cmp r8d,ecx
    jb .gu_arg
    mov ebp,ecx
    mov rbx,rdx
    call ApiLock
    test eax,eax
    jnz .gu_out
    mov dword ptr [rip+rng_mode],0
    mov dword ptr [rip+rng_error],0
    mov dword ptr [rip+session_count],0
    xor esi,esi
.gu_sample:
    call GenerateUniformMask
    test rax,rax
    jz .gu_rng
    lea rdx,[rip+session_masks]
    mov [rdx+rsi*8],rax
    inc esi
    cmp esi,ebp
    jb .gu_sample
    mov rax,[rip+session_masks]
    mov [rip+selected_mask],rax
    mov rcx,rax
    lea rdx,[rip+selected_nums]
    call MaskToNumbers
    mov dword ptr [rip+selected_index],0
    mov qword ptr [rip+selected_score],0
    mov [rip+session_count],ebp
    mov rcx,rbx
    lea rdx,[rip+session_masks]
    lea r8d,[rbp*8]
    call Copy
    xor ecx,ecx
    jmp .gu_finish
.gu_rng: mov ecx,-5
.gu_finish: call ApiFinish; jmp .gu_out
.gu_arg: mov eax,-1
.gu_out: END
