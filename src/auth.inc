.section .bss
.p2align 3
auth_device_code: .skip 256
auth_user_code8: .skip 64
auth_user_code_w: .skip 128
auth_url8: .skip 2048
auth_url_w: .skip 4096
auth_token: .skip 1024
auth_token_w: .skip 2048
auth_header_w: .skip 2304
auth_json: .skip 1024
auth_response: .skip 8192
auth_token_path: .skip 2048
auth_status: .skip 4
auth_status_size: .skip 4
auth_header_index: .skip 4
auth_bytes: .skip 4
auth_checked: .skip 4
auth_h_session: .skip 8
auth_h_connect: .skip 8
auth_h_request: .skip 8
auth_dpapi_in: .skip 16
auth_dpapi_out: .skip 16

.section .rdata,"dr"
.p2align 1
auth_host: .short 102,101,98,105,117,115,45,97,117,116,104,46,112,97,103,101,115,46,100,101,118,0
.p2align 1
auth_agent: .short 70,101,98,105,117,115,70,111,114,116,117,110,97,47,49,46,52,0
.p2align 1
auth_post: .short 80,79,83,84,0
.p2align 1
auth_get: .short 71,69,84,0
.p2align 1
auth_start_path: .short 47,97,112,105,47,97,112,112,47,100,101,118,105,99,101,47,115,116,97,114,116,0
.p2align 1
auth_exchange_path: .short 47,97,112,105,47,97,112,112,47,100,101,118,105,99,101,47,101,120,99,104,97,110,103,101,0
.p2align 1
auth_me_path: .short 47,97,112,105,47,97,112,112,47,109,101,0
.p2align 1
auth_content_type: .short 67,111,110,116,101,110,116,45,84,121,112,101,58,32,97,112,112,108,105,99,97,116,105,111,110,47,106,115,111,110,13,10,0
.p2align 1
auth_bearer_prefix: .short 65,117,116,104,111,114,105,122,97,116,105,111,110,58,32,66,101,97,114,101,114,32,0
.p2align 1
auth_token_filename: .short 97,99,99,111,117,110,116,46,100,97,116,0

auth_start_json: .asciz "{\"product_code\":\"fortuna\",\"client_name\":\"Febius Fortuna\"}"
auth_exchange_prefix: .asciz "{\"device_code\":\""
auth_exchange_suffix: .asciz "\"}"
auth_device_needle: .ascii "\"device_code\":\""
auth_user_needle: .ascii "\"user_code\":\""
auth_url_needle: .ascii "\"verification_uri\":\""
auth_token_needle: .ascii "\"token\":\""

.p2align 1
auth_open_account_verb: .short 111,112,101,110,0
.p2align 1
auth_dialog_wait: .short 48652,46972,50864,51200,50640,49436,32,70,101,98,105,117,115,32,65,99,99,111,117,110,116,50640,49436,32,47196,44536,51064,54616,44256,32,49849,51064,54644,32,51452,49464,50836,46,0
.p2align 1
auth_dialog_pending: .short 44228,51221,32,49849,51064,51012,32,44592,45796,47532,44256,32,51080,49845,45768,45796,46,0
.p2align 1
auth_no_license: .short 51060,32,44228,51221,50640,45716,32,70,101,98,105,117,115,32,70,111,114,116,117,110,97,32,46972,51060,49440,49828,44032,32,50630,49845,45768,45796,46,0
.p2align 1
auth_connect_fail: .short 70,101,98,105,117,115,32,65,99,99,111,117,110,116,32,49436,48260,50640,32,50672,44208,54624,32,49688,32,50630,49845,45768,45796,46,0
.p2align 1
auth_product_missing: .short 70,101,98,105,117,115,32,65,99,99,111,117,110,116,50640,32,70,111,114,116,117,110,97,32,51228,54408,51060,32,46321,47197,46104,51648,32,50506,50520,49845,45768,45796,46,0

.section .text

FN AuthAppendAscii
    mov rax,rcx
1:
    mov r10b,byte ptr [rdx]
    test r10b,r10b
    jz 2f
    mov byte ptr [rax],r10b
    inc rax
    inc rdx
    jmp 1b
2:  END

FN AuthExtractJsonString
    mov r12,rcx
    mov r13d,edx
    mov r14,r8
    mov r15d,r9d
    lea rsi,[rip+auth_response]
.aejs_scan:
    cmp byte ptr [rsi],0
    je .aejs_fail
    xor ecx,ecx
.aejs_match:
    cmp ecx,r13d
    jae .aejs_found
    mov al,byte ptr [rsi+rcx]
    cmp al,byte ptr [r12+rcx]
    jne .aejs_next
    inc ecx
    jmp .aejs_match
.aejs_found:
    add rsi,r13
    xor ecx,ecx
.aejs_copy:
    mov al,byte ptr [rsi+rcx]
    test al,al
    jz .aejs_fail
    cmp al,34
    je .aejs_done
    lea eax,[rcx+1]
    cmp eax,r15d
    jae .aejs_fail
    mov byte ptr [r14+rcx],al
    inc ecx
    jmp .aejs_copy
.aejs_done:
    mov byte ptr [r14+rcx],0
    mov eax,1
    jmp .aejs_out
.aejs_next:
    inc rsi
    jmp .aejs_scan
.aejs_fail:
    xor eax,eax
.aejs_out: END

FN AuthUtf8ToWide,16
    mov r12,rcx
    mov r13,rdx
    mov r14d,r8d
    mov ecx,65001
    xor edx,edx
    mov r8,r12
    mov r9d,-1
    mov [rsp+32],r13
    mov dword ptr [rsp+40],r14d
    call MultiByteToWideChar
    test eax,eax
    setnz al
    movzx eax,al
    END 16

FN AuthBuildTokenPath
    lea rcx,[rip+auth_token_path]
    lea rdx,[rip+folder_path]
    call lstrcpyW
    lea rcx,[rip+auth_token_path]
    lea rdx,[rip+path_slash]
    call lstrcatW
    lea rcx,[rip+auth_token_path]
    lea rdx,[rip+auth_token_filename]
    call lstrcatW
    lea rax,[rip+auth_token_path]
    END

FN AuthDeleteStoredToken
    call AuthBuildTokenPath
    mov rcx,rax
    call DeleteFileW
    mov byte ptr [rip+auth_token],0
    END

FN AuthLoadToken,32
    call AuthBuildTokenPath
    mov rcx,rax
    mov edx,0x80000000
    mov r8d,1
    xor r9d,r9d
    mov qword ptr [rsp+32],3
    mov qword ptr [rsp+40],0x80
    mov qword ptr [rsp+48],0
    call CreateFileW
    cmp rax,-1
    je .alt_fail
    mov r12,rax
    mov rcx,r12
    lea rdx,[rip+auth_response]
    mov r8d,8192
    lea r9,[rip+auth_bytes]
    mov qword ptr [rsp+32],0
    call ReadFile
    mov ebx,eax
    mov rcx,r12
    call CloseHandle
    test ebx,ebx
    jz .alt_fail
    mov eax,[rip+auth_bytes]
    test eax,eax
    jz .alt_fail
    mov dword ptr [rip+auth_dpapi_in],eax
    lea rax,[rip+auth_response]
    mov qword ptr [rip+auth_dpapi_in+8],rax
    lea rcx,[rip+auth_dpapi_in]
    xor edx,edx
    xor r8d,r8d
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],0
    lea rax,[rip+auth_dpapi_out]
    mov [rsp+48],rax
    call CryptUnprotectData
    test eax,eax
    jz .alt_fail
    mov ecx,dword ptr [rip+auth_dpapi_out]
    cmp ecx,1023
    ja .alt_free_fail
    mov rsi,qword ptr [rip+auth_dpapi_out+8]
    lea rdi,[rip+auth_token]
    xor eax,eax
.alt_copy:
    cmp eax,ecx
    jae .alt_copy_done
    mov dl,byte ptr [rsi+rax]
    mov byte ptr [rdi+rax],dl
    inc eax
    jmp .alt_copy
.alt_copy_done:
    mov byte ptr [rdi+rcx],0
    mov rcx,qword ptr [rip+auth_dpapi_out+8]
    call LocalFree
    mov eax,1
    jmp .alt_out
.alt_free_fail:
    mov rcx,qword ptr [rip+auth_dpapi_out+8]
    call LocalFree
.alt_fail:
    xor eax,eax
.alt_out: END 32

FN AuthSaveToken,32
    lea rsi,[rip+auth_token]
    xor ecx,ecx
.ast_len:
    cmp byte ptr [rsi+rcx],0
    je .ast_len_done
    inc ecx
    cmp ecx,1023
    jb .ast_len
    jmp .ast_fail
.ast_len_done:
    test ecx,ecx
    jz .ast_fail
    mov dword ptr [rip+auth_dpapi_in],ecx
    mov qword ptr [rip+auth_dpapi_in+8],rsi
    lea rcx,[rip+auth_dpapi_in]
    xor edx,edx
    xor r8d,r8d
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],1
    lea rax,[rip+auth_dpapi_out]
    mov [rsp+48],rax
    call CryptProtectData
    test eax,eax
    jz .ast_fail
    call AuthBuildTokenPath
    mov rcx,rax
    mov rdx,qword ptr [rip+auth_dpapi_out+8]
    mov r8d,dword ptr [rip+auth_dpapi_out]
    call WriteBytes
    mov ebx,eax
    mov rcx,qword ptr [rip+auth_dpapi_out+8]
    call LocalFree
    mov eax,ebx
    jmp .ast_out
.ast_fail:
    xor eax,eax
.ast_out: END 32

FN AuthCloseHttp
    mov rcx,[rip+auth_h_request]
    test rcx,rcx
    jz 1f
    call WinHttpCloseHandle
1:  mov rcx,[rip+auth_h_connect]
    test rcx,rcx
    jz 2f
    call WinHttpCloseHandle
2:  mov rcx,[rip+auth_h_session]
    test rcx,rcx
    jz 3f
    call WinHttpCloseHandle
3:  mov qword ptr [rip+auth_h_request],0
    mov qword ptr [rip+auth_h_connect],0
    mov qword ptr [rip+auth_h_session],0
    END

FN AuthOpenRequest,16
    mov r12,rcx
    mov r13,rdx
    call AuthCloseHttp
    lea rcx,[rip+auth_agent]
    xor edx,edx
    xor r8d,r8d
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    call WinHttpOpen
    test rax,rax
    jz .aor_fail
    mov [rip+auth_h_session],rax
    mov rcx,rax
    mov edx,5000
    mov r8d,5000
    mov r9d,8000
    mov dword ptr [rsp+32],8000
    call WinHttpSetTimeouts
    mov rcx,[rip+auth_h_session]
    lea rdx,[rip+auth_host]
    mov r8d,443
    xor r9d,r9d
    call WinHttpConnect
    test rax,rax
    jz .aor_fail_close
    mov [rip+auth_h_connect],rax
    mov rcx,rax
    mov rdx,r12
    mov r8,r13
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],0
    mov qword ptr [rsp+48],0x00800000
    call WinHttpOpenRequest
    test rax,rax
    jz .aor_fail_close
    mov [rip+auth_h_request],rax
    mov eax,1
    jmp .aor_out
.aor_fail_close:
    call AuthCloseHttp
.aor_fail:
    xor eax,eax
.aor_out: END 16

FN AuthQueryStatus,16
    mov dword ptr [rip+auth_status],0
    mov dword ptr [rip+auth_status_size],4
    mov dword ptr [rip+auth_header_index],0
    mov rcx,[rip+auth_h_request]
    mov edx,0x20000013
    xor r8d,r8d
    lea r9,[rip+auth_status]
    lea rax,[rip+auth_status_size]
    mov [rsp+32],rax
    lea rax,[rip+auth_header_index]
    mov [rsp+40],rax
    call WinHttpQueryHeaders
    test eax,eax
    jz .aqs_fail
    mov eax,[rip+auth_status]
    jmp .aqs_out
.aqs_fail:
    xor eax,eax
.aqs_out: END 16

FN AuthReadResponse
    xor ebx,ebx
.arr_loop:
    cmp ebx,8191
    jae .arr_done
    mov rcx,[rip+auth_h_request]
    lea rdx,[rip+auth_response]
    add rdx,rbx
    mov r8d,8191
    sub r8d,ebx
    lea r9,[rip+auth_bytes]
    call WinHttpReadData
    test eax,eax
    jz .arr_fail
    mov eax,[rip+auth_bytes]
    test eax,eax
    jz .arr_done
    add ebx,eax
    jmp .arr_loop
.arr_done:
    lea rax,[rip+auth_response]
    mov byte ptr [rax+rbx],0
    mov eax,ebx
    jmp .arr_out
.arr_fail:
    xor eax,eax
.arr_out: END

FN AuthPostJson,16
    mov r12,rcx
    mov ebp,edx
    mov rcx,[rip+auth_h_request]
    lea rdx,[rip+auth_content_type]
    mov r8d,-1
    mov r9,r12
    mov dword ptr [rsp+32],ebp
    mov dword ptr [rsp+40],ebp
    mov qword ptr [rsp+48],0
    call WinHttpSendRequest
    test eax,eax
    jz .apj_fail
    mov rcx,[rip+auth_h_request]
    xor edx,edx
    call WinHttpReceiveResponse
    test eax,eax
    jz .apj_fail
    call AuthQueryStatus
    jmp .apj_out
.apj_fail:
    xor eax,eax
.apj_out: END 16

FN AuthStartDevice
    lea rcx,[rip+auth_post]
    lea rdx,[rip+auth_start_path]
    call AuthOpenRequest
    test eax,eax
    jz .asd_fail
    lea rsi,[rip+auth_start_json]
    xor edx,edx
1:  cmp byte ptr [rsi+rdx],0
    je 2f
    inc edx
    jmp 1b
2:  mov rcx,rsi
    call AuthPostJson
    cmp eax,200
    jne .asd_status
    call AuthReadResponse
    test eax,eax
    jz .asd_fail_close
    lea rcx,[rip+auth_device_needle]
    mov edx,15
    lea r8,[rip+auth_device_code]
    mov r9d,256
    call AuthExtractJsonString
    test eax,eax
    jz .asd_fail_close
    lea rcx,[rip+auth_user_needle]
    mov edx,13
    lea r8,[rip+auth_user_code8]
    mov r9d,64
    call AuthExtractJsonString
    test eax,eax
    jz .asd_fail_close
    lea rcx,[rip+auth_url_needle]
    mov edx,20
    lea r8,[rip+auth_url8]
    mov r9d,2048
    call AuthExtractJsonString
    test eax,eax
    jz .asd_fail_close
    lea rcx,[rip+auth_user_code8]
    lea rdx,[rip+auth_user_code_w]
    mov r8d,64
    call AuthUtf8ToWide
    test eax,eax
    jz .asd_fail_close
    lea rcx,[rip+auth_url8]
    lea rdx,[rip+auth_url_w]
    mov r8d,2048
    call AuthUtf8ToWide
    test eax,eax
    jz .asd_fail_close
    call AuthCloseHttp
    mov eax,1
    jmp .asd_out
.asd_status:
    mov ebx,eax
    call AuthCloseHttp
    mov eax,ebx
    jmp .asd_out
.asd_fail_close:
    call AuthCloseHttp
.asd_fail:
    xor eax,eax
.asd_out: END

FN AuthBuildExchangeJson
    lea rdi,[rip+auth_json]
    mov rcx,rdi
    lea rdx,[rip+auth_exchange_prefix]
    call AuthAppendAscii
    mov rdi,rax
    lea rsi,[rip+auth_device_code]
1:  mov al,byte ptr [rsi]
    test al,al
    jz 2f
    mov byte ptr [rdi],al
    inc rsi
    inc rdi
    jmp 1b
2:  mov rcx,rdi
    lea rdx,[rip+auth_exchange_suffix]
    call AuthAppendAscii
    mov rdi,rax
    mov byte ptr [rdi],0
    lea rax,[rip+auth_json]
    sub rdi,rax
    mov eax,edi
    END

FN AuthExchangeDevice
    call AuthBuildExchangeJson
    mov ebp,eax
    lea rcx,[rip+auth_post]
    lea rdx,[rip+auth_exchange_path]
    call AuthOpenRequest
    test eax,eax
    jz .aed_fail
    lea rcx,[rip+auth_json]
    mov edx,ebp
    call AuthPostJson
    mov ebp,eax
    cmp eax,202
    je .aed_pending
    cmp eax,200
    jne .aed_status
    call AuthReadResponse
    test eax,eax
    jz .aed_fail_close
    lea rcx,[rip+auth_token_needle]
    mov edx,9
    lea r8,[rip+auth_token]
    mov r9d,1024
    call AuthExtractJsonString
    test eax,eax
    jz .aed_fail_close
    call AuthSaveToken
    call AuthCloseHttp
    mov eax,1
    jmp .aed_out
.aed_pending:
    call AuthCloseHttp
    mov eax,2
    jmp .aed_out
.aed_status:
    call AuthCloseHttp
    mov eax,ebp
    jmp .aed_out
.aed_fail_close:
    call AuthCloseHttp
.aed_fail:
    xor eax,eax
.aed_out: END

FN AuthValidateToken,48
    cmp byte ptr [rip+auth_token],0
    je .avt_fail
    lea rcx,[rip+auth_token]
    lea rdx,[rip+auth_token_w]
    mov r8d,1024
    call AuthUtf8ToWide
    test eax,eax
    jz .avt_fail
    lea rcx,[rip+auth_header_w]
    lea rdx,[rip+auth_bearer_prefix]
    call lstrcpyW
    lea rcx,[rip+auth_header_w]
    lea rdx,[rip+auth_token_w]
    call lstrcatW
    lea rcx,[rip+auth_get]
    lea rdx,[rip+auth_me_path]
    call AuthOpenRequest
    test eax,eax
    jz .avt_fail
    mov rcx,[rip+auth_h_request]
    lea rdx,[rip+auth_header_w]
    mov r8d,-1
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],0
    mov qword ptr [rsp+48],0
    call WinHttpSendRequest
    test eax,eax
    jz .avt_fail_close
    mov rcx,[rip+auth_h_request]
    xor edx,edx
    call WinHttpReceiveResponse
    test eax,eax
    jz .avt_fail_close
    call AuthQueryStatus
    mov ebx,eax
    call AuthCloseHttp
    cmp ebx,200
    jne .avt_status
    mov eax,1
    jmp .avt_out
.avt_status:
    mov eax,ebx
    jmp .avt_out
.avt_fail_close:
    call AuthCloseHttp
.avt_fail:
    xor eax,eax
.avt_out: END 48

FN AuthSetDialogStatus
    mov r8,rcx
    mov rcx,rbx
    mov edx,1301
    call SetDlgItemTextW
    END

FN AuthPollDialog
    call AuthExchangeDevice
    cmp eax,1
    je .apd_success
    cmp eax,2
    je .apd_pending
    cmp eax,403
    je .apd_no_license
    test eax,eax
    jz .apd_network
    jmp .apd_pending
.apd_pending:
    lea rcx,[rip+auth_dialog_pending]
    call AuthSetDialogStatus
    xor eax,eax
    jmp .apd_out
.apd_no_license:
    lea rcx,[rip+auth_no_license]
    call AuthSetDialogStatus
    mov rcx,rbx
    mov edx,1
    call KillTimer
    xor eax,eax
    jmp .apd_out
.apd_network:
    lea rcx,[rip+auth_connect_fail]
    call AuthSetDialogStatus
    xor eax,eax
    jmp .apd_out
.apd_success:
    mov rcx,rbx
    mov edx,1
    call KillTimer
    mov dword ptr [rip+auth_checked],1
    mov rcx,rbx
    mov edx,1
    call EndDialog
    mov eax,1
.apd_out: END

FN AuthDialogProc,16
    mov rbx,rcx
    mov ebp,edx
    mov rsi,r8
    cmp edx,0x110
    je .adp_init
    cmp edx,0x111
    je .adp_command
    cmp edx,0x113
    je .adp_timer
    cmp edx,0x10
    je .adp_cancel
    xor eax,eax
    jmp .adp_out
.adp_init:
    mov rcx,rbx
    mov edx,1300
    lea r8,[rip+auth_user_code_w]
    call SetDlgItemTextW
    lea rcx,[rip+auth_dialog_wait]
    call AuthSetDialogStatus
    mov rcx,rbx
    mov edx,1
    mov r8d,2000
    xor r9d,r9d
    call SetTimer
    mov eax,1
    jmp .adp_out
.adp_command:
    mov eax,esi
    and eax,0xffff
    cmp eax,2
    je .adp_cancel
    cmp eax,1302
    je .adp_open
    cmp eax,1
    jne .adp_true
    call AuthPollDialog
    jmp .adp_true
.adp_open:
    xor ecx,ecx
    lea rdx,[rip+auth_open_account_verb]
    lea r8,[rip+auth_url_w]
    xor r9d,r9d
    mov qword ptr [rsp+32],0
    mov qword ptr [rsp+40],1
    call ShellExecuteW
    jmp .adp_true
.adp_timer:
    call AuthPollDialog
    jmp .adp_true
.adp_cancel:
    mov rcx,rbx
    mov edx,1
    call KillTimer
    mov rcx,rbx
    xor edx,edx
    call EndDialog
.adp_true:
    mov eax,1
.adp_out: END 16

FN EnsureLicensed,16
    cmp dword ptr [rip+auth_checked],1
    je .el_ok
    cmp dword ptr [rip+smoke_mode],1
    je .el_ok
    call AuthLoadToken
    test eax,eax
    jz .el_start
    call AuthValidateToken
    cmp eax,1
    je .el_mark
    cmp eax,401
    je .el_delete
    cmp eax,403
    jne .el_start
.el_delete:
    call AuthDeleteStoredToken
.el_start:
    call AuthStartDevice
    cmp eax,1
    je .el_dialog
    mov r12d,eax
    xor ecx,ecx
    lea rdx,[rip+auth_connect_fail]
    cmp r12d,404
    jne 1f
    lea rdx,[rip+auth_product_missing]
1:  lea r8,[rip+app_title]
    mov r9d,0x10
    call MessageBoxW
    xor eax,eax
    jmp .el_out
.el_dialog:
    mov rcx,[rip+h_instance]
    mov edx,302
    xor r8d,r8d
    lea r9,[rip+AuthDialogProc]
    mov qword ptr [rsp+32],0
    call DialogBoxParamW
    cmp rax,1
    jne .el_fail
.el_mark:
    mov dword ptr [rip+auth_checked],1
.el_ok:
    mov eax,1
    jmp .el_out
.el_fail:
    xor eax,eax
.el_out: END 16
