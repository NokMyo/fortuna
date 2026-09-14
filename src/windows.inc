.equ ID_LOAD,1001
.equ ID_ANALYZE,1002
.equ ID_GENERATE,1003
.equ ID_CANCEL,1004
.equ ID_FIVE,1005
.equ ID_TEN,1006
.equ ID_COPY,1007
.equ ID_SAVE,1008
.equ ID_DETAILS,1009
.equ ID_SEAL,1010
.equ ID_REPORT,1011
.equ ID_FOLDER,1012
.equ ID_EXIT,1013
.equ ID_UNIFORM,1014
.equ ID_MANUAL,1015
.equ ID_RELEASES,1016
.equ ID_ABOUT,1017
.section .bss
h_instance: .skip 8
h_main: .skip 8
h_worker: .skip 8
h_title_font: .skip 8
h_result_font: .skip 8
analysis_busy: .skip 4
close_after: .skip 4
worker_result: .skip 4
smoke_mode: .skip 4
smoke_ticks: .skip 4
file_path: .skip 2048
folder_path: .skip 2048
report_path: .skip 2048
ledger_path: .skip 2048
ofn: .skip 152
io_size: .skip 8
io_bytes: .skip 4
ui_status: .skip 2048
ui_result: .skip 256
ui_metrics: .skip 2048
ui_tickets: .skip 4096
ui_ticket_line: .skip 256
ui_numbers: .skip 24
command_argc: .skip 4
report_length: .skip 4

.section .text
# File I/O is Unicode and bounded. A short read is treated as failure.
FN WriteBytes,32
    mov rbx,rdx
    mov edi,r8d
    mov edx,0x40000000
    xor r8d,r8d
    xor r9d,r9d
    mov qword ptr [rsp+32],2
    mov qword ptr [rsp+40],0x80
    mov qword ptr [rsp+48],0
    call CreateFileW
    cmp rax,-1
    je .wb_fail
    mov rsi,rax
    mov rcx,rax
    mov rdx,rbx
    mov r8d,edi
    lea r9,[rip+io_bytes]
    mov qword ptr [rsp+32],0
    call WriteFile
    xor ebp,ebp
    test eax,eax
    jz .wb_close
    cmp edi,[rip+io_bytes]
    jne .wb_close
    mov rcx,rsi
    call FlushFileBuffers
    mov ebp,eax
.wb_close:
    mov rcx,rsi
    call CloseHandle
    mov eax,ebp
    jmp .wb_out
.wb_fail: xor eax,eax
.wb_out: END 32

# DialogFile(kind): 0 CSV open,1 tickets txt,2 analysis txt.
FN DialogFile
    mov ebp,ecx
    lea rcx,[rip+ofn]
    mov edx,152
    call Zero
    lea rbx,[rip+ofn]
    mov dword ptr [rbx],152
    mov rax,[rip+h_main]
    mov [rbx+8],rax
    lea rax,[rip+file_path]
    mov [rbx+48],rax
    mov dword ptr [rbx+56],1024
    mov dword ptr [rbx+44],1
    mov word ptr [rax],0
    test ebp,ebp
    jnz .df_save
    lea rax,[rip+filter_csv]
    mov [rbx+24],rax
    lea rax,[rip+load_title]
    mov [rbx+88],rax
    mov dword ptr [rbx+96],0x81808
    mov rcx,rbx
    call GetOpenFileNameW
    jmp .df_out
.df_save:
    lea rcx,[rip+file_path]
    lea rdx,[rip+default_tickets]
    cmp ebp,1
    je 1f
    lea rdx,[rip+default_report]
1:  call lstrcpyW
    lea rax,[rip+filter_txt]
    mov [rbx+24],rax
    lea rax,[rip+extension_txt]
    mov [rbx+104],rax
    lea rax,[rip+save_title]
    cmp ebp,1
    je 2f
    lea rax,[rip+report_title]
2:  mov [rbx+88],rax
    mov dword ptr [rbx+96],0x8080a
    mov rcx,rbx
    call GetSaveFileNameW
.df_out: END

# Initialize durable app directory; path failures disable disk actions.
FN InitStorage
    lea rcx,[rip+appdata_env]
    lea rdx,[rip+folder_path]
    mov r8d,900
    call GetEnvironmentVariableW
    test eax,eax
    jz .is_out
    cmp eax,900
    jae .is_out
    lea rcx,[rip+folder_path]
    lea rdx,[rip+path_slash]
    call lstrcatW
    lea rcx,[rip+folder_path]
    lea rdx,[rip+folder_name]
    call lstrcatW
    xor ecx,ecx
    lea rdx,[rip+folder_path]
    xor r8d,r8d
    call SHCreateDirectoryExW
    test eax,eax
    jz 1f
    cmp eax,183
    jne .is_out
1:  lea rcx,[rip+report_path]
    lea rdx,[rip+folder_path]
    call lstrcpyW
    lea rcx,[rip+report_path]
    lea rdx,[rip+path_slash]
    call lstrcatW
    lea rcx,[rip+report_path]
    lea rdx,[rip+report_filename]
    call lstrcatW
    lea rcx,[rip+ledger_path]
    lea rdx,[rip+folder_path]
    call lstrcpyW
    lea rcx,[rip+ledger_path]
    lea rdx,[rip+path_slash]
    call lstrcatW
    lea rcx,[rip+ledger_path]
    lea rdx,[rip+ledger_filename]
    call lstrcatW
    lea rcx,[rip+forecast_path]
    lea rdx,[rip+folder_path]
    call lstrcpyW
    lea rcx,[rip+forecast_path]
    lea rdx,[rip+path_slash]
    call lstrcatW
    lea rcx,[rip+forecast_path]
    lea rdx,[rip+forecast_filename]
    call lstrcatW
.is_out: END

FN Notify
    mov r8,rcx
    mov rcx,[rip+h_main]
    mov edx,1107
    call SetDlgItemTextW
    END
FN Alert
    mov rdx,rcx
    mov rcx,[rip+h_main]
    lea r8,[rip+app_title]
    mov r9d,0x40
    call MessageBoxW
    END
FN OpenPath,16
    mov r8,rcx
    mov rcx,[rip+h_main]
    lea rdx,[rip+open_verb]
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],1
    call ShellExecuteW
    END 16

FN UpdateResultUI,96
    cmp qword ptr [rip+selected_mask],0
    je .ur_empty
    lea rbx,[rip+selected_nums]
    mov eax,[rbx+8]
    mov [rsp+32],rax
    mov eax,[rbx+12]
    mov [rsp+40],rax
    mov eax,[rbx+16]
    mov [rsp+48],rax
    mov eax,[rbx+20]
    mov [rsp+56],rax
    lea rcx,[rip+ui_result]
    lea rdx,[rip+fmt_result]
    mov r8d,[rbx]
    mov r9d,[rbx+4]
    call wsprintfW
    mov rcx,[rip+h_main]
    mov edx,1101
    lea r8,[rip+ui_result]
    call SetDlgItemTextW
    jmp .ur_list
.ur_empty:
    mov rcx,[rip+h_main]
    mov edx,1101
    lea r8,[rip+text_placeholder]
    call SetDlgItemTextW
.ur_list:
    mov word ptr [rip+ui_tickets],0
    xor esi,esi
.ur_tickets:
    cmp esi,[rip+session_count]
    jae .ur_metrics
    lea rax,[rip+session_masks]
    mov rcx,[rax+rsi*8]
    lea rdx,[rip+ui_numbers]
    call MaskToNumbers
    lea rbx,[rip+ui_numbers]
    xor edi,edi
1:  cmp edi,5
    jae 2f
    mov eax,[rbx+rdi*4+4]
    mov [rsp+32+rdi*8],rax
    inc edi
    jmp 1b
2:  lea rcx,[rip+ui_ticket_line]
    lea rdx,[rip+fmt_ticket]
    lea r8d,[rsi+1]
    mov r9d,[rbx]
    call wsprintfW
    lea rcx,[rip+ui_tickets]
    lea rdx,[rip+ui_ticket_line]
    call lstrcatW
    inc esi
    jmp .ur_tickets
.ur_metrics:
    mov rcx,[rip+h_main]
    mov edx,1102
    lea r8,[rip+ui_tickets]
    call SetDlgItemTextW
    cmp dword ptr [rip+app_draw_kind],0
    je .ur_random_metrics
    cmp dword ptr [rip+app_draw_kind],2
    je .ur_deep_metrics
    cmp dword ptr [rip+app_draw_kind],1
    je .ur_integrated_metrics
    cmp dword ptr [rip+model_ready],1
    jne .ur_no_metrics
    mov eax,[rip+field_count]
    mov [rsp+32],rax
    mov eax,[rip+validation_count]
    mov [rsp+40],rax
    mov rax,[rip+elapsed_ms]
    xor edx,edx
    mov ecx,1000
    div rcx
    mov [rsp+48],rax
    mov [rsp+56],rdx
    movsd xmm0,[rip+rho]
    mov eax,100
    cvtsi2sd xmm1,eax
    mulsd xmm0,xmm1
    cvtsd2si r9d,xmm0
    lea rcx,[rip+ui_metrics]
    lea rdx,[rip+fmt_metrics]
    mov r8d,[rip+selected_index]
    call wsprintfW
    mov rcx,[rip+h_main]
    mov edx,1104
    lea r8,[rip+ui_metrics]
    call SetDlgItemTextW
    jmp .ur_label
.ur_no_metrics:
    mov rcx,[rip+h_main]
    mov edx,1104
    lea r8,[rip+empty]
    call SetDlgItemTextW
    jmp .ur_label
.ur_random_metrics:
    lea r8,[rip+text_random_metrics]
    jmp .ur_set_metrics
.ur_deep_metrics:
    lea r8,[rip+text_ten_deep]
    jmp .ur_set_metrics
.ur_integrated_metrics:
    lea r8,[rip+text_ten_normal]
.ur_set_metrics:
    mov rcx,[rip+h_main]
    mov edx,1104
    call SetDlgItemTextW
.ur_label:
    lea r8,[rip+text_mode_sealed]
    cmp dword ptr [rip+app_draw_kind],3
    je 3f
    lea r8,[rip+text_mode_random]
    cmp dword ptr [rip+app_draw_kind],0
    je 3f
    lea r8,[rip+text_mode_deep]
    cmp dword ptr [rip+app_draw_kind],2
    je 3f
    lea r8,[rip+text_mode_oracle]
    movsd xmm0,[rip+rho]
    ucomisd xmm0,[rip+one]
    jb 3f
    lea r8,[rip+text_oracle_fallback]
3:  mov rcx,[rip+h_main]
    mov edx,1103
    call SetDlgItemTextW
    END 96

FN CopyTickets
    lea rcx,[rip+ui_tickets]
    call lstrlenW
    test eax,eax
    jz .ct_fail
    lea rdx,[rax*2+2]
    mov ecx,2
    call GlobalAlloc
    test rax,rax
    jz .ct_fail
    mov rbx,rax
    mov rcx,rax
    call GlobalLock
    test rax,rax
    jz .ct_free
    mov rcx,rax
    lea rdx,[rip+ui_tickets]
    call lstrcpyW
    mov rcx,rbx
    call GlobalUnlock
    mov rcx,[rip+h_main]
    call OpenClipboard
    test eax,eax
    jz .ct_free
    call EmptyClipboard
    test eax,eax
    jz .ct_closefail
    mov ecx,13
    mov rdx,rbx
    call SetClipboardData
    test rax,rax
    jz .ct_closefail
    call CloseClipboard
    mov eax,1
    jmp .ct_out
.ct_closefail:
    call CloseClipboard
.ct_free:
    mov rcx,rbx
    call GlobalFree
.ct_fail: xor eax,eax
.ct_out: END
