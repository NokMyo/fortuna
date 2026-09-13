# ---------------------------------------------------------------------------
# Entry and Win32 UI
# ---------------------------------------------------------------------------
WinMainCRTStartup:
    sub rsp, 40
    xor rcx, rcx
    call GetModuleHandleA
    mov qword ptr [rip + h_instance], rax

    # WNDCLASSEXA (80 bytes)
    lea rdi, [rip + wc_buf]
    xor eax, eax
    mov ecx, 10
    rep stosq
    lea rdi, [rip + wc_buf]
    mov dword ptr [rdi + 0], 80
    mov dword ptr [rdi + 4], 0x0003
    lea rax, [rip + WndProc]
    mov qword ptr [rdi + 8], rax
    mov rax, qword ptr [rip + h_instance]
    mov qword ptr [rdi + 24], rax
    xor rcx, rcx
    mov edx, IDC_ARROW
    call LoadCursorA
    mov qword ptr [rdi + 40], rax
    mov qword ptr [rdi + 48], COLOR_WINDOW_BRUSH
    lea rax, [rip + class_name]
    mov qword ptr [rdi + 64], rax
    lea rcx, [rip + wc_buf]
    call RegisterClassExA
    test ax, ax
    jz .fatal

    sub rsp, 96
    mov qword ptr [rsp + 32], -2147483648
    mov qword ptr [rsp + 40], -2147483648
    mov qword ptr [rsp + 48], 800
    mov qword ptr [rsp + 56], 585
    mov qword ptr [rsp + 64], 0
    mov qword ptr [rsp + 72], 0
    mov rax, qword ptr [rip + h_instance]
    mov qword ptr [rsp + 80], rax
    mov qword ptr [rsp + 88], 0
    xor ecx, ecx
    lea rdx, [rip + class_name]
    lea r8, [rip + app_title]
    mov r9d, WS_OVERLAPPEDWINDOW
    call CreateWindowExA
    add rsp, 96
    test rax, rax
    jz .fatal
    mov qword ptr [rip + h_main], rax
    mov rcx, rax
    mov edx, SW_SHOWDEFAULT
    call ShowWindow
    mov rcx, qword ptr [rip + h_main]
    call UpdateWindow
.msg_loop:
    lea rcx, [rip + msg_buf]
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call GetMessageA
    test eax, eax
    jle .exit
    lea rcx, [rip + msg_buf]
    call TranslateMessage
    lea rcx, [rip + msg_buf]
    call DispatchMessageA
    jmp .msg_loop
.exit:
    xor ecx, ecx
    call ExitProcess
.fatal:
    mov ecx, 1
    call ExitProcess

MakeFont:
    push rbp
    mov rbp, rsp
    sub rsp, 112
    mov r10d, ecx
    mov r11d, edx
    mov ecx, r10d
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    mov qword ptr [rsp + 32], r11
    mov qword ptr [rsp + 40], 0
    mov qword ptr [rsp + 48], 0
    mov qword ptr [rsp + 56], 0
    mov qword ptr [rsp + 64], 1
    mov qword ptr [rsp + 72], 0
    mov qword ptr [rsp + 80], 0
    mov qword ptr [rsp + 88], 5
    mov qword ptr [rsp + 96], 0x20
    lea rax, [rip + font_face]
    mov qword ptr [rsp + 104], rax
    call CreateFontA
    leave
    ret

CreateChild:
    push rbp
    mov rbp, rsp
    sub rsp, 96
    mov r10, rcx
    mov r11, rdx
    mov eax, r8d
    or eax, WS_CHILD | WS_VISIBLE
    mov r8d, eax
    mov rax, r9
    movsxd rcx, dword ptr [rbp + 48]
    mov qword ptr [rsp + 32], rcx
    movsxd rcx, dword ptr [rbp + 56]
    mov qword ptr [rsp + 40], rcx
    movsxd rcx, dword ptr [rbp + 64]
    mov qword ptr [rsp + 48], rcx
    movsxd rcx, dword ptr [rbp + 72]
    mov qword ptr [rsp + 56], rcx
    mov qword ptr [rsp + 64], rax
    movsxd rcx, dword ptr [rbp + 80]
    mov qword ptr [rsp + 72], rcx
    mov rcx, qword ptr [rip + h_instance]
    mov qword ptr [rsp + 80], rcx
    mov qword ptr [rsp + 88], 0
    xor ecx, ecx
    mov rdx, r10
    mov r9d, r8d
    mov r8, r11
    call CreateWindowExA
    leave
    ret

WndProc:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 40
    mov rbx, rcx
    cmp edx, WM_CREATE
    je .create
    cmp edx, WM_COMMAND
    je .command
    cmp edx, WM_APP_DONE
    je .analysis_done
    cmp edx, WM_DESTROY
    je .destroy
    mov rcx, rbx
    call DefWindowProcA
    jmp .return
.create:
    mov qword ptr [rip + h_main], rbx
    mov ecx, 28
    mov edx, 700
    call MakeFont
    mov qword ptr [rip + h_font_title], rax
    mov ecx, 42
    mov edx, 700
    call MakeFont
    mov qword ptr [rip + h_font_result], rax
    mov ecx, 18
    mov edx, 400
    call MakeFont
    mov qword ptr [rip + h_font_ui], rax

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 16
    mov dword ptr [rsp + 48], 735
    mov dword ptr [rsp + 56], 36
    mov dword ptr [rsp + 64], 0
    lea rcx, [rip + static_class]
    lea rdx, [rip + ui_title]
    xor r8d, r8d
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov rcx, rax
    mov edx, WM_SETFONT
    mov r8, qword ptr [rip + h_font_title]
    mov r9d, 1
    call SendMessageA

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 55
    mov dword ptr [rsp + 48], 735
    mov dword ptr [rsp + 56], 28
    mov dword ptr [rsp + 64], 0
    lea rcx, [rip + static_class]
    lea rdx, [rip + ui_subtitle]
    xor r8d, r8d
    mov r9, rbx
    call CreateChild
    add rsp, 80

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 94
    mov dword ptr [rsp + 48], 735
    mov dword ptr [rsp + 56], 82
    mov dword ptr [rsp + 64], 0
    lea rcx, [rip + static_class]
    lea rdx, [rip + ui_result_empty]
    mov r8d, WS_BORDER | SS_CENTER
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov qword ptr [rip + h_result], rax
    mov rcx, rax
    mov edx, WM_SETFONT
    mov r8, qword ptr [rip + h_font_result]
    mov r9d, 1
    call SendMessageA

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 188
    mov dword ptr [rsp + 48], 735
    mov dword ptr [rsp + 56], 28
    mov dword ptr [rsp + 64], 0
    lea rcx, [rip + static_class]
    lea rdx, [rip + ui_status_ready]
    xor r8d, r8d
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov qword ptr [rip + h_status], rax

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 232
    mov dword ptr [rsp + 48], 170
    mov dword ptr [rsp + 56], 38
    mov dword ptr [rsp + 64], ID_LOAD
    lea rcx, [rip + button_class]
    lea rdx, [rip + btn_load]
    mov r8d, BS_PUSHBUTTON
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov qword ptr [rip + h_btn_load], rax

    sub rsp, 80
    mov dword ptr [rsp + 32], 207
    mov dword ptr [rsp + 40], 232
    mov dword ptr [rsp + 48], 170
    mov dword ptr [rsp + 56], 38
    mov dword ptr [rsp + 64], ID_ANALYZE
    lea rcx, [rip + button_class]
    lea rdx, [rip + btn_analyze]
    mov r8d, BS_PUSHBUTTON
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov qword ptr [rip + h_btn_analyze], rax

    sub rsp, 80
    mov dword ptr [rsp + 32], 390
    mov dword ptr [rsp + 40], 232
    mov dword ptr [rsp + 48], 170
    mov dword ptr [rsp + 56], 38
    mov dword ptr [rsp + 64], ID_RECALC
    lea rcx, [rip + button_class]
    lea rdx, [rip + btn_recalc]
    mov r8d, BS_PUSHBUTTON
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov qword ptr [rip + h_btn_recalc], rax
    mov rcx, rax
    xor edx, edx
    call EnableWindow

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 287
    mov dword ptr [rsp + 48], 735
    mov dword ptr [rsp + 56], 180
    mov dword ptr [rsp + 64], 0
    lea rcx, [rip + edit_class]
    lea rdx, [rip + metrics_empty]
    mov r8d, WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL
    mov r9, rbx
    call CreateChild
    add rsp, 80
    mov qword ptr [rip + h_metrics], rax
    mov rcx, rax
    mov edx, WM_SETFONT
    mov r8, qword ptr [rip + h_font_ui]
    mov r9d, 1
    call SendMessageA

    sub rsp, 80
    mov dword ptr [rsp + 32], 24
    mov dword ptr [rsp + 40], 480
    mov dword ptr [rsp + 48], 735
    mov dword ptr [rsp + 56], 48
    mov dword ptr [rsp + 64], 0
    lea rcx, [rip + static_class]
    lea rdx, [rip + disclaimer]
    xor r8d, r8d
    mov r9, rbx
    call CreateChild
    add rsp, 80
    xor eax, eax
    jmp .return
.command:
    mov eax, r8d
    and eax, 0xFFFF
    cmp eax, ID_LOAD
    je .do_load
    cmp eax, ID_ANALYZE
    je .do_analyze
    cmp eax, ID_RECALC
    je .do_recalc
    xor eax, eax
    jmp .return
.do_load:
    cmp dword ptr [rip + analysis_busy], 0
    jne .busy_msg
    call ChooseAndLoadCsv
    xor eax, eax
    jmp .return
.do_analyze:
    cmp dword ptr [rip + analysis_busy], 0
    jne .busy_msg
    cmp dword ptr [rip + draw_count], 30
    jb .need_data
    mov dword ptr [rip + analysis_busy], 1
    mov dword ptr [rip + analysis_ready], 0
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + ui_status_analyzing]
    call SetWindowTextA
    mov rcx, qword ptr [rip + h_btn_load]
    xor edx, edx
    call EnableWindow
    mov rcx, qword ptr [rip + h_btn_analyze]
    xor edx, edx
    call EnableWindow
    sub rsp, 16
    xor ecx, ecx
    xor edx, edx
    lea r8, [rip + AnalysisThread]
    xor r9d, r9d
    mov qword ptr [rsp + 32], 0
    mov qword ptr [rsp + 40], 0
    call CreateThread
    add rsp, 16
    test rax, rax
    jnz .thread_ok
    mov dword ptr [rip + analysis_busy], 0
    jmp .need_data
.thread_ok:
    mov rcx, rax
    call CloseHandle
    xor eax, eax
    jmp .return
.do_recalc:
    cmp dword ptr [rip + analysis_ready], 1
    jne .need_data
    call SampleCandidate
    call UpdateResultUI
    xor eax, eax
    jmp .return
.busy_msg:
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + ui_status_busy]
    call SetWindowTextA
    xor eax, eax
    jmp .return
.need_data:
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + ui_status_needdata]
    call SetWindowTextA
    xor eax, eax
    jmp .return
.analysis_done:
    mov dword ptr [rip + analysis_busy], 0
    mov dword ptr [rip + analysis_ready], 1
    mov rcx, qword ptr [rip + h_btn_load]
    mov edx, 1
    call EnableWindow
    mov rcx, qword ptr [rip + h_btn_analyze]
    mov edx, 1
    call EnableWindow
    mov rcx, qword ptr [rip + h_btn_recalc]
    mov edx, 1
    call EnableWindow
    call UpdateResultUI
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + ui_status_done]
    call SetWindowTextA
    xor eax, eax
    jmp .return
.destroy:
    mov rcx, qword ptr [rip + h_font_title]
    test rcx, rcx
    jz 1f
    call DeleteObject
1:  mov rcx, qword ptr [rip + h_font_result]
    test rcx, rcx
    jz 2f
    call DeleteObject
2:  mov rcx, qword ptr [rip + h_font_ui]
    test rcx, rcx
    jz 3f
    call DeleteObject
3:  xor ecx, ecx
    call PostQuitMessage
    xor eax, eax
.return:
    add rsp, 40
    pop rbx
    leave
    ret
