.section .text
FN RefreshEngineState
    lea rcx,[rip+api_snapshot]
    mov edx,272
    call FortunaOracleGetSnapshot
    test eax,eax
    jnz .refresh_out
    lea rbx,[rip+api_snapshot]
    mov eax,[rbx+16]
    mov [rip+history_count],eax
    mov eax,[rbx+20]
    mov [rip+last_round],eax
    mov eax,[rbx+24]
    mov [rip+parse_error],eax
    mov eax,[rbx+28]
    mov [rip+parse_line],eax
    mov eax,[rbx+32]
    mov [rip+field_count],eax
    mov eax,[rbx+36]
    mov [rip+validation_count],eax
    mov eax,[rbx+40]
    mov [rip+selected_index],eax
    mov eax,[rbx+44]
    mov [rip+session_count],eax
    mov eax,[rbx+12]
    mov edx,eax
    and eax,1
    shr edx,1
    and edx,1
    mov [rip+field_ready],eax
    mov [rip+model_ready],edx
    mov rax,[rbx+48]
    mov [rip+selected_mask],rax
    mov rax,[rbx+160]
    mov [rip+rho],rax
    lea rcx,[rip+selected_nums]
    lea rdx,[rbx+56]
    mov r8d,24
    call Copy
    lea rcx,[rip+session_masks]
    lea rdx,[rbx+80]
    mov r8d,80
    call Copy
.refresh_out: END
FN ReadEngineProgress
    cmp dword ptr [rip+cancel_flag],1
    jne 1f
    call FortunaOracleCancel
1:
    lea rcx,[rip+api_progress]
    mov edx,16
    call FortunaOracleGetProgress
    mov eax,[rip+api_progress]
    mov [rip+analysis_stage],eax
    mov eax,[rip+api_progress+4]
    mov [rip+progress],eax
    mov eax,[rip+api_progress+8]
    mov [rip+validation_count],eax
    END

# Production callers must never inherit the historical smoke-mode auth bypass.
FN StrictLicenseCheck
    mov ebx,[rip+smoke_mode]
    mov dword ptr [rip+smoke_mode],0
    call EnsureLicensed
    mov [rip+smoke_mode],ebx
    END

FN StartupCheck
    call FortunaOracleGetAbiVersion
    cmp eax,0x10000
    jne .startup_fail
    call StrictLicenseCheck
    test eax,eax
    jz .startup_fail
    mov eax,1
    jmp .startup_out
.startup_fail:
    xor eax,eax
.startup_out:
    END
FN SelfTest
    call FortunaOracleSelfTest
    END
FN LoadCsvFile
    mov r12,rcx
    call StrictLicenseCheck
    test eax,eax
    jz .load_auth_fail
    mov rcx,r12
    call FortunaOracleLoadCsvFileW
    mov ebx,eax
    call RefreshEngineState
    xor eax,eax
    test ebx,ebx
    sete al
    jmp .load_out
.load_auth_fail:
    xor eax,eax
.load_out: END
FN AnalyzeOracle
    mov ecx,[rip+app_research_mode]
    test ecx,ecx
    jz 1f
    dec ecx
    call FortunaOracleAnalyzeResearch
    jmp 2f
1:  call FortunaOracleAnalyze
2:
    mov ebx,eax
    call RefreshEngineState
    xor eax,eax
    test ebx,ebx
    sete al
    END
FN GenerateTickets
    lea rdx,[rip+session_masks]
    mov r8d,10
    call FortunaOracleGenerate
    mov ebx,eax
    call RefreshEngineState
    xor eax,eax
    test ebx,ebx
    sete al
    END
FN SampleCandidate
    mov ecx,1
    call GenerateTickets
    END
FN BuildReport
    lea rcx,[rip+report_buffer]
    mov edx,2097152
    call FortunaOracleGetReport
    test eax,eax
    jle 1f
    dec eax
    jmp 2f
1:  xor eax,eax
2:  END
FN SealPick
    lea rcx,[rip+ledger_path]
    call FortunaOracleSealW
    mov ebx,eax
    call RefreshEngineState
    xor eax,eax
    test ebx,ebx
    js 1f
    lea eax,[rbx+1]
1:  END

# ecx=count, edx=0 uniform / 1 standard / 2 deep. Commit label only on success.
FN GenerateDraw
    mov esi,edx
    lea rdx,[rip+session_masks]
    mov r8d,10
    test esi,esi
    jz .draw_uniform
    cmp esi,2
    je .draw_deep
    call FortunaOracleGenerate
    jmp .draw_result
.draw_uniform:
    call FortunaOracleGenerateUniform
    jmp .draw_result
.draw_deep:
    call FortunaOracleGenerateResearch
.draw_result:
    mov ebx,eax
    call RefreshEngineState
    test ebx,ebx
    jnz .draw_fail
    mov [rip+app_draw_kind],esi
    mov eax,1
    jmp .draw_out
.draw_fail: xor eax,eax
.draw_out: END
