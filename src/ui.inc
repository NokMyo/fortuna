FN WinMainCRTStartup,32
    xor ecx,ecx
    call GetModuleHandleW
    mov [rip+h_instance],rax
    call FortunaOracleGetAbiVersion
    cmp eax,0x10000
    jne .entry_engine_error
    call FortunaOracleInitialize
    test eax,eax
    jnz .entry_engine_error
    call InitStorage
    call GetCommandLineW
    mov rcx,rax
    lea rdx,[rip+command_argc]
    call CommandLineToArgvW
    mov rbx,rax
    test rax,rax
    jz .entry_gui
    cmp dword ptr [rip+command_argc],2
    jb .entry_free
    mov rcx,[rbx+8]
    lea rdx,[rip+cmd_selftest]
    call lstrcmpW
    test eax,eax
    jnz 1f
    call SelfTest
    test eax,eax
    jnz .entry_selftest_exit
    call LoadEmbeddedHistory
    test eax,eax
    jnz .entry_selftest_ok
    mov eax,20
    jmp .entry_selftest_exit
.entry_selftest_ok:
    xor eax,eax
.entry_selftest_exit:
    mov ecx,eax
    call ExitProcess
1:  mov rcx,[rbx+8]
    lea rdx,[rip+cmd_smoke]
    call lstrcmpW
    test eax,eax
    jnz 2f
    mov dword ptr [rip+smoke_mode],1
    jmp .entry_free
2:  mov rcx,[rbx+8]
    lea rdx,[rip+cmd_open]
    call lstrcmpW
    test eax,eax
    jnz .entry_check_analyze
    cmp dword ptr [rip+command_argc],3
    jne .entry_bad_args
    mov rcx,[rbx+16]
    call LoadCsvFile
    test eax,eax
    jz .entry_error
    jmp .entry_free
.entry_check_analyze:
    mov rcx,[rbx+8]
    lea rdx,[rip+cmd_analyze]
    call lstrcmpW
    test eax,eax
    jz .entry_analysis_args
    mov rcx,[rbx+8]
    lea rdx,[rip+cmd_research]
    call lstrcmpW
    mov dword ptr [rip+app_research_mode],1
    test eax,eax
    jz .entry_analysis_args
    mov rcx,[rbx+8]
    lea rdx,[rip+cmd_research_full]
    call lstrcmpW
    test eax,eax
    jnz .entry_bad_args
    mov dword ptr [rip+app_research_mode],2
.entry_analysis_args:
    cmp dword ptr [rip+command_argc],4
    jne .entry_bad_args
    mov rcx,[rbx+16]
    call LoadCsvFile
    test eax,eax
    jz .entry_error
    call AnalyzeOracle
    test eax,eax
    jz .entry_error
    call BuildReport
    test eax,eax
    jz .entry_error
    mov r8d,eax
    mov rcx,[rbx+24]
    lea rdx,[rip+report_buffer]
    call WriteBytes
    test eax,eax
    jz .entry_error
    xor ecx,ecx
    call ExitProcess
.entry_engine_error:
    xor ecx,ecx
    lea rdx,[rip+text_startup_fail]
    lea r8,[rip+app_title]
    mov r9d,0x10
    call MessageBoxW
    mov ecx,3
    call ExitProcess
.entry_bad_args:
    mov ecx,2
    call ExitProcess
.entry_error:
    mov ecx,1
    call ExitProcess
.entry_free:
    mov rcx,rbx
    call LocalFree
.entry_gui:
    call StartupCheck
    test eax,eax
    jnz 3f
    xor ecx,ecx
    lea rdx,[rip+text_startup_fail]
    lea r8,[rip+app_title]
    mov r9d,0x10
    call MessageBoxW
    mov ecx,3
    call ExitProcess
3:
    mov dword ptr [rsp+32],8
    mov dword ptr [rsp+36],0x20
    lea rcx,[rsp+32]
    call InitCommonControlsEx
    mov rcx,[rip+h_instance]
    mov edx,101
    xor r8d,r8d
    lea r9,[rip+DialogProc]
    mov qword ptr [rsp+32],0
    call DialogBoxParamW
    xor ecx,ecx
    cmp rax,-1
    jne 1f
    mov ecx,1
1:  call ExitProcess
    END 32

FN MakeFont,80
    mov ebx,ecx
    mov ebp,edx
    mov rcx,[rip+h_main]
    call GetDpiForWindow
    test eax,eax
    jnz 1f
    mov eax,96
1:  imul eax,ebx
    cdq
    mov ecx,96
    idiv ecx
    neg eax
    mov ecx,eax
    xor edx,edx
    xor r8d,r8d
    xor r9d,r9d
    mov [rsp+32],rbp
    mov qword ptr [rsp+40],0
    mov qword ptr [rsp+48],0
    mov qword ptr [rsp+56],0
    mov qword ptr [rsp+64],1
    mov qword ptr [rsp+72],0
    mov qword ptr [rsp+80],0
    mov qword ptr [rsp+88],5
    mov qword ptr [rsp+96],0
    lea rax,[rip+font_name]
    mov [rsp+104],rax
    call CreateFontW
    END 80
FN SetControlFont
    mov rbx,rdx
    mov edx,ecx
    mov rcx,[rip+h_main]
    call GetDlgItem
    mov rcx,rax
    mov edx,0x30
    mov r8,rbx
    mov r9d,1
    call SendMessageW
    END
FN SetBusyControls
    mov ebp,ecx
    mov esi,1001
1:  mov rcx,[rip+h_main]
    mov edx,esi
    call GetDlgItem
    mov rcx,rax
    mov edx,1
    sub edx,ebp
    cmp esi,1004
    jne 2f
    mov edx,ebp
2:  call EnableWindow
    inc esi
    cmp esi,1023
    jbe 1b
    mov esi,1120
3:  mov rcx,[rip+h_main]
    mov edx,esi
    call GetDlgItem
    mov rcx,rax
    mov edx,1
    sub edx,ebp
    call EnableWindow
    inc esi
    cmp esi,1122
    jbe 3b
    mov esi,ID_BACKTEST_START
4:  mov rcx,[rip+h_main]
    mov edx,esi
    call GetDlgItem
    mov rcx,rax
    mov edx,1
    sub edx,ebp
    call EnableWindow
    inc esi
    cmp esi,ID_BACKTEST_END
    jbe 4b
    END

FN DialogProc,128
    mov rbx,rcx
    mov ebp,edx
    mov rsi,r8
    cmp edx,0x110
    je .dp_init
    cmp edx,0x111
    je .dp_command
    cmp edx,0x113
    je .dp_timer
    cmp edx,0x8001
    je .dp_done
    cmp edx,0x10
    je .dp_close
    xor eax,eax
    jmp .dp_out
.dp_init:
    mov [rip+h_main],rbx
    mov ecx,32
    mov edx,700
    call MakeFont
    mov [rip+h_title_font],rax
    mov rdx,rax
    mov ecx,1100
    call SetControlFont
    mov ecx,29
    mov edx,700
    call MakeFont
    mov [rip+h_result_font],rax
    mov rdx,rax
    mov ecx,1101
    call SetControlFont
    xor ecx,ecx
    call SetBusyControls
    mov rcx,rbx
    mov edx,1106
    call GetDlgItem
    mov rcx,rax
    mov edx,0x406
    xor r8d,r8d
    mov r9d,100
    call SendMessageW
    mov rcx,rbx
    mov edx,1
    mov r8d,150
    xor r9d,r9d
    call SetTimer
    mov rcx,rbx
    mov edx,1120
    mov r8d,1122
    mov r9d,1120
    call CheckRadioButton
    cmp dword ptr [rip+history_count],0
    je .dp_init_smoke
    call UpdateDataUI
.dp_init_smoke:
    cmp dword ptr [rip+smoke_mode],1
    je .dp_smoke_run
    call BenchmarkPerformance
    call UpdateEstimatedTimes
    jmp .dp_true
.dp_smoke_run:
    cmp dword ptr [rip+smoke_mode],1
    jne .dp_true
    mov ecx,5
    xor edx,edx
    call GenerateDraw
    call UpdateResultUI
    jmp .dp_true
.dp_command:
    mov eax,esi
    and eax,0xffff
    cmp eax,2
    je .dp_close
    cmp eax,ID_EXIT
    je .dp_close
    cmp eax,ID_CANCEL
    je .dp_cancel
    cmp eax,1018
    je .dp_engine_info
    cmp eax,ID_ABOUT
    je .dp_about
    cmp eax,ID_RELEASES
    je .dp_releases
    cmp eax,ID_MANUAL
    je .dp_manual
    cmp dword ptr [rip+analysis_busy],0
    jne .dp_busy
    cmp eax,1120
    jb .dp_not_count
    cmp eax,1122
    jbe .dp_count
.dp_not_count:
    cmp eax,ID_LOAD
    je .dp_load
    cmp eax,1023
    je .dp_research
    cmp eax,1019
    je .dp_research
    cmp eax,1020
    je .dp_research_full
    cmp eax,1021
    je .dp_forecast_save
    cmp eax,1022
    je .dp_forecast_audit
    cmp eax,ID_ANALYZE
    je .dp_analyze
    cmp eax,ID_GENERATE
    je .dp_generate
    cmp eax,ID_FIVE
    je .dp_five
    cmp eax,ID_TEN
    je .dp_ten
    cmp eax,ID_COPY
    je .dp_copy
    cmp eax,ID_SAVE
    je .dp_save
    cmp eax,ID_REPORT
    je .dp_report
    cmp eax,ID_DETAILS
    je .dp_details
    cmp eax,ID_FOLDER
    je .dp_folder
    cmp eax,ID_UNIFORM
    je .dp_uniform
    cmp eax,ID_SEAL
    je .dp_seal
    jmp .dp_true
.dp_busy:
    lea rcx,[rip+text_busy]
    call Notify
    jmp .dp_true
.dp_load:
    xor ecx,ecx
    call DialogFile
    test eax,eax
    jz .dp_true
    lea rcx,[rip+file_path]
    call LoadCsvFile
    test eax,eax
    jz .dp_load_error
    mov dword ptr [rip+app_model_kind],0
    mov dword ptr [rip+app_draw_kind],0
    call UpdateDataUI
    call UpdateResultUI
    lea rcx,[rip+text_loaded]
    call Notify
    jmp .dp_true
.dp_load_error:
    lea rcx,[rip+ui_status]
    lea rdx,[rip+fmt_error]
    mov r8d,[rip+parse_error]
    mov r9d,[rip+parse_line]
    call wsprintfW
    lea rcx,[rip+ui_status]
    call Alert
    jmp .dp_true
.dp_analyze:
    mov dword ptr [rip+app_request_kind],1
    mov dword ptr [rip+app_research_mode],0
    jmp .dp_analyze_start
.dp_research:
    mov dword ptr [rip+app_request_kind],2
    mov dword ptr [rip+app_research_mode],1
    jmp .dp_analyze_start
.dp_research_full:
    mov dword ptr [rip+app_request_kind],2
    mov dword ptr [rip+app_research_mode],2
.dp_analyze_start:
    cmp dword ptr [rip+history_count],60
    jb .dp_need
    call ApplyBacktestRangeFromUI
    test eax,eax
    jz .dp_backtest_invalid
    mov eax,[rip+app_research_mode]
    inc eax
    cmp eax,[rip+app_model_kind]
    jne .dp_start_worker
    cmp dword ptr [rip+field_ready],1
    jne .dp_start_worker
    mov ecx,[rip+app_ticket_count]
    mov edx,[rip+app_request_kind]
    call GenerateDraw
    test eax,eax
    jz .dp_rng
    jmp .dp_update
.dp_start_worker:
    mov dword ptr [rip+app_model_kind],0
    mov dword ptr [rip+analysis_busy],1
    mov dword ptr [rip+cancel_flag],0
    mov dword ptr [rip+field_ready],0
    mov ecx,1
    call SetBusyControls
    xor ecx,ecx
    xor edx,edx
    lea r8,[rip+AnalysisThread]
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],0
    call CreateThread
    test rax,rax
    jz .dp_thread_error
    mov [rip+h_worker],rax
    jmp .dp_true
.dp_thread_error:
    mov dword ptr [rip+analysis_busy],0
    xor ecx,ecx
    call SetBusyControls
    lea rcx,[rip+text_thread]
    call Alert
    jmp .dp_true
.dp_need:
    lea rcx,[rip+text_need]
    call Alert
    jmp .dp_true
.dp_backtest_invalid:
    lea rcx,[rip+text_backtest_invalid]
    call Alert
    jmp .dp_true
.dp_count:
    mov r9d,eax
    sub eax,1120
    mov ecx,1
    test eax,eax
    jz 1f
    imul ecx,eax,5
1:  mov [rip+app_ticket_count],ecx
    mov rcx,rbx
    mov edx,1120
    mov r8d,1122
    call CheckRadioButton
    jmp .dp_true
.dp_five: mov eax,1121; jmp .dp_count
.dp_ten: mov eax,1122; jmp .dp_count
.dp_generate:
    mov ecx,[rip+app_ticket_count]
    xor edx,edx
    call GenerateDraw
    test eax,eax
    jz .dp_rng
.dp_update:
    call UpdateDrawProgress
    call UpdateResultUI
    lea rcx,[rip+text_generated]
    call Notify
    jmp .dp_true
.dp_rng:
    lea rcx,[rip+text_rng]
    call Alert
    jmp .dp_true
.dp_cancel:
    call FortunaOracleCancel
    mov dword ptr [rip+cancel_flag],1
    jmp .dp_true
.dp_copy:
    call CopyTickets
    test eax,eax
    jz .dp_true
    lea rcx,[rip+text_copied]
    call Notify
    jmp .dp_true
.dp_save:
    cmp dword ptr [rip+session_count],0
    je .dp_true
    mov ecx,1
    call DialogFile
    test eax,eax
    jz .dp_true
    # UTF-16LE BOM + text; every Windows text editor reads the result correctly.
    mov word ptr [rip+ui_status],0xfeff
    lea rcx,[rip+ui_status+2]
    lea rdx,[rip+ui_tickets]
    call lstrcpyW
    lea rcx,[rip+ui_status]
    call lstrlenW
    lea r8d,[rax*2]
    lea rcx,[rip+file_path]
    lea rdx,[rip+ui_status]
    call WriteBytes
    jmp .dp_saved
.dp_report:
    cmp dword ptr [rip+field_ready],1
    jne .dp_report_need
    mov ecx,2
    call DialogFile
    test eax,eax
    jz .dp_true
    call BuildReport
    test eax,eax
    jz .dp_save_error
    mov r8d,eax
    lea rcx,[rip+file_path]
    lea rdx,[rip+report_buffer]
    call WriteBytes
.dp_saved:
    test eax,eax
    jz .dp_save_error
    lea rcx,[rip+text_saved]
    call Notify
    jmp .dp_true
.dp_save_error:
    lea rcx,[rip+text_savefail]
    call Alert
    jmp .dp_true
.dp_details:
    cmp dword ptr [rip+field_ready],1
    jne .dp_report_need
    call BuildReport
    test eax,eax
    jz .dp_save_error
    mov r8d,eax
    lea rcx,[rip+report_path]
    lea rdx,[rip+report_buffer]
    call WriteBytes
    test eax,eax
    jz .dp_save_error
    lea rcx,[rip+report_path]
    call OpenPath
    jmp .dp_true
.dp_report_need:
    lea rcx,[rip+text_report_need]
    call Alert
    jmp .dp_true
.dp_forecast_save:
    mov edx,1
    jmp .dp_forecast_run
.dp_forecast_audit:
    xor edx,edx
.dp_forecast_run:
    mov ebp,edx
    cmp word ptr [rip+forecast_path],0
    je .dp_forecast_error
    lea rcx,[rip+forecast_path]
    call FortunaOracleForecastLedgerW
    test eax,eax
    js .dp_forecast_error
    lea rcx,[rip+text_forecast_saved]
    test ebp,ebp
    jnz 1f
    lea rcx,[rip+text_forecast_audited]
1:  call Notify
    jmp .dp_true
.dp_forecast_error:
    lea rcx,[rip+text_forecast_error]
    call Alert
    jmp .dp_true
.dp_folder:
    lea rcx,[rip+folder_path]
    call OpenPath
    jmp .dp_true
.dp_uniform:
    jmp .dp_generate
.dp_seal:
    call SealPick
    test eax,eax
    jz .dp_seal_error
    mov esi,eax
    mov dword ptr [rip+app_draw_kind],3
    mov rax,[rip+selected_mask]
    mov [rip+session_masks],rax
    mov dword ptr [rip+session_count],1
    call UpdateResultUI
    lea rcx,[rip+text_sealed]
    cmp esi,2
    jne 1f
    lea rcx,[rip+text_seal_existing]
1:  call Alert
    jmp .dp_true
.dp_seal_error:
    lea rcx,[rip+text_seal_fail]
    call Alert
    jmp .dp_true
.dp_engine_info:
    mov edx,301
    jmp .dp_show_about
.dp_about:
    mov edx,300
.dp_show_about:
    mov rcx,[rip+h_instance]
    mov r8,rbx
    lea r9,[rip+AboutProc]
    mov qword ptr [rsp+32],0
    call DialogBoxParamW
    jmp .dp_true
.dp_releases:
    lea rcx,[rip+releases_url]
    call OpenPath
    jmp .dp_true
.dp_manual:
    lea rcx,[rip+manual_url]
    call OpenPath
    jmp .dp_true
.dp_timer:
    cmp dword ptr [rip+smoke_mode],1
    jne 1f
    inc dword ptr [rip+smoke_ticks]
    cmp dword ptr [rip+smoke_ticks],20
    jae .dp_close
1:  cmp dword ptr [rip+analysis_busy],1
    jne .dp_timer_idle
    call ReadEngineProgress
    mov eax,[rip+last_round]
    sub eax,[rip+history_count]
    inc eax
    mov [rsp+32],rax
    mov eax,[rip+last_round]
    mov [rsp+40],rax
    lea rcx,[rip+ui_status]
    lea rdx,[rip+fmt_busy]
    mov r8d,[rip+analysis_stage]
    mov r9d,[rip+progress]
    call wsprintfW
    lea rcx,[rip+ui_status]
    call Notify
    mov rcx,rbx
    mov edx,1106
    call GetDlgItem
    mov rcx,rax
    mov edx,0x402
    mov r8d,[rip+progress]
    xor r9d,r9d
    call SendMessageW
    jmp .dp_true
.dp_timer_idle:
    call UpdateEstimatedTimes
    jmp .dp_true
.dp_done:
    mov rcx,[rip+h_worker]
    mov edx,-1
    call WaitForSingleObject
    mov rcx,[rip+h_worker]
    call CloseHandle
    mov qword ptr [rip+h_worker],0
    mov dword ptr [rip+analysis_busy],0
    xor ecx,ecx
    call SetBusyControls
    cmp dword ptr [rip+close_after],1
    je .dp_close
    cmp dword ptr [rip+worker_result],1
    jne .dp_canceled
    call UpdateDrawProgress
    call UpdateResultUI
    lea rcx,[rip+text_done]
    call Notify
    jmp .dp_true
.dp_canceled:
    call UpdateResultUI
    lea rcx,[rip+text_cancel]
    cmp dword ptr [rip+cancel_flag],1
    je 1f
    lea rcx,[rip+text_analysis_fail]
1:
    call Notify
    jmp .dp_true
.dp_close:
    cmp dword ptr [rip+analysis_busy],1
    jne .dp_destroy
    call FortunaOracleCancel
    mov dword ptr [rip+cancel_flag],1
    mov dword ptr [rip+close_after],1
    jmp .dp_true
.dp_destroy:
    mov rcx,rbx
    mov edx,1
    call KillTimer
    mov rcx,[rip+h_title_font]
    call DeleteObject
    mov rcx,[rip+h_result_font]
    call DeleteObject
    mov rcx,rbx
    xor edx,edx
    call EndDialog
.dp_true: mov eax,1
.dp_out: END 128

FN AboutProc,16
    cmp edx,0x110
    je .about_init
    cmp edx,0x111
    je 1f
    cmp edx,0x10
    jne 2f
1:  mov edx,1
    call EndDialog
    mov eax,1
    jmp 3f
2:  xor eax,eax
    jmp 3f
.about_init:
    call EngineAboutInit
3:  END 16

FN EngineAboutInit,16
    mov rbx,rcx
    call FortunaOracleGetEngineVersion
    mov r8d,eax
    shr r8d,16
    mov r9d,eax
    shr r9d,8
    and r9d,255
    and eax,255
    mov [rsp+32],rax
    lea rcx,[rip+engine_about_text]
    lea rdx,[rip+fmt_engine_version]
    call wsprintfW
    mov rcx,rbx
    mov edx,1201
    lea r8,[rip+engine_about_text]
    call SetDlgItemTextW
    mov eax,1
    END 16
FN AnalysisThread
    call GetTickCount64
    mov rbx,rax
    call AnalyzeOracle
    mov ebp,eax
    call GetTickCount64
    sub rax,rbx
    mov [rip+elapsed_ms],rax
    test ebp,ebp
    jz 1f
    mov eax,[rip+app_research_mode]
    inc eax
    mov [rip+app_model_kind],eax
    mov ecx,[rip+app_ticket_count]
    mov edx,[rip+app_request_kind]
    call GenerateDraw
    mov ebp,eax
1:  mov [rip+worker_result],ebp
    mov rcx,[rip+h_main]
    mov edx,0x8001
    xor r8d,r8d
    xor r9d,r9d
    call PostMessageW
    xor eax,eax
    END

FN UpdateDataUI,16
    # Loaded files are contiguous, so the available first round is derived from
    # the full loaded count before any user range is activated.
    mov eax,[rip+last_round]
    sub eax,[rip+history_count]
    inc eax
    mov [rip+available_first_round],eax
    mov ecx,[rip+last_round]
    mov [rip+available_last_round],ecx
    lea rcx,[rip+ui_status]
    lea rdx,[rip+fmt_available_range]
    mov r8d,[rip+available_first_round]
    mov r9d,[rip+available_last_round]
    call wsprintfW
    mov rcx,[rip+h_main]
    mov edx,1105
    lea r8,[rip+ui_status]
    call SetDlgItemTextW
    mov r8d,[rip+available_first_round]
    mov rcx,[rip+h_main]
    mov edx,ID_BACKTEST_START
    xor r9d,r9d
    call SetDlgItemInt
    mov r8d,[rip+available_last_round]
    mov rcx,[rip+h_main]
    mov edx,ID_BACKTEST_END
    xor r9d,r9d
    call SetDlgItemInt
    mov rcx,[rip+h_main]
    mov edx,1107
    lea r8,[rip+empty]
    call SetDlgItemTextW
    mov dword ptr [rip+estimate_ready],0
    call UpdateEstimatedTimes
    END 16

FN UpdateDrawProgress
    xor ebx,ebx
    cmp dword ptr [rip+app_draw_kind],0
    je 1f
    mov ebx,100
1:  mov rcx,[rip+h_main]
    mov edx,1106
    call GetDlgItem
    mov rcx,rax
    mov edx,0x402
    mov r8d,ebx
    xor r9d,r9d
    call SendMessageW
    END
