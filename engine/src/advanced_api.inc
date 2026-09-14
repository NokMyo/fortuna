# Additive ABI-v1-compatible advanced entry points. Existing exports remain
# untouched; Fortuna uses these for the strengthened normal/deep paths.
.section .text
.globl FortunaOracleGetAdvancedVersion
FN FortunaOracleGetAdvancedVersion
    mov eax,0x10300
    END

.globl FortunaOracleAnalyzeAdvanced
FN FortunaOracleAnalyzeAdvanced
    call ApiLock
    test eax,eax
    jnz .aaa_out
    mov dword ptr [rip+cancel_flag],0
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+forecast_valid],0
    call AnalyzeOracle
    test eax,eax
    jz .aaa_fail
    call AdvancedFinalizeNormalV2
    test eax,eax
    jz .aaa_fail
    xor ecx,ecx
    call IntegratedAnalyze
    test eax,eax
    jz .aaa_fail
    xor ecx,ecx
    jmp .aaa_finish
.aaa_fail:
    mov ecx,-3
    cmp dword ptr [rip+cancel_flag],1
    jne .aaa_finish
    mov ecx,-6
.aaa_finish:
    call ApiFinish
.aaa_out: END

.globl FortunaOracleAnalyzeResearchAdvanced
FN FortunaOracleAnalyzeResearchAdvanced
    cmp ecx,1
    ja .aara_arg
    mov ebx,ecx
    call ApiLock
    test eax,eax
    jnz .aara_out
    mov [rip+research_full_scan],ebx
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+cancel_flag],0
    mov dword ptr [rip+forecast_valid],0
    call AnalyzeResearch
    test eax,eax
    jz .aara_fail
    call AdvancedFinalizeDeepV2
    test eax,eax
    jz .aara_fail
    mov ecx,1
    call IntegratedAnalyze
    test eax,eax
    jz .aara_fail
    xor ecx,ecx
    jmp .aara_finish
.aara_fail:
    mov ecx,-3
    cmp dword ptr [rip+cancel_flag],1
    jne .aara_finish
    mov ecx,-6
.aara_finish:
    call ApiFinish
    jmp .aara_out
.aara_arg: mov eax,-1
.aara_out: END

.globl FortunaOracleGenerateResearchAdvanced
FN FortunaOracleGenerateResearchAdvanced
    cmp ecx,1
    jb .agra_arg
    cmp ecx,10
    ja .agra_arg
    test rdx,rdx
    jz .agra_arg
    cmp r8d,ecx
    jb .agra_arg
    mov ebp,ecx
    mov rbx,rdx
    call ApiLock
    test eax,eax
    jnz .agra_out
    cmp dword ptr [rip+research_ready],1
    jne .agra_state
    cmp dword ptr [rip+advanced_ready],1
    jne .agra_state
    mov dword ptr [rip+rng_mode],0
    mov dword ptr [rip+rng_error],0
    mov dword ptr [rip+session_count],0
    xor esi,esi
.agra_sample:
    call AdvancedJointSampleV2
    test rax,rax
    jz .agra_random
    lea rdx,[rip+session_masks]
    mov [rdx+rsi*8],rax
    inc esi
    cmp esi,ebp
    jb .agra_sample
    mov [rip+session_count],ebp
    mov rcx,[rip+session_masks]
    call SelectMask
    mov rcx,rbx
    lea rdx,[rip+session_masks]
    lea r8d,[rbp*8]
    call Copy
    xor ecx,ecx
    jmp .agra_finish
.agra_random: mov ecx,-5; jmp .agra_finish
.agra_state: mov ecx,-4
.agra_finish: call ApiFinish; jmp .agra_out
.agra_arg: mov eax,-1
.agra_out: END
