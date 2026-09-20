.section .bss
.p2align 3
auth_username_w: .skip 128
auth_password_w: .skip 2050
auth_username8: .skip 128
auth_password8: .skip 4097
auth_token: .skip 1024
auth_token_w: .skip 2048
auth_header_w: .skip 2304
auth_json: .skip 16384
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
auth_agent: .short 70,101,98,105,117,115,70,111,114,116,117,110,97,47,49,46,57,46,48,0
.p2align 1
auth_post: .short 80,79,83,84,0
.p2align 1
auth_get: .short 71,69,84,0
.p2align 1
auth_login_path: .short 47,97,112,105,47,97,112,112,47,108,111,103,105,110,0
.p2align 1
auth_me_path: .short 47,97,112,105,47,97,112,112,47,109,101,0
.p2align 1
auth_content_type: .short 67,111,110,116,101,110,116,45,84,121,112,101,58,32,97,112,112,108,105,99,97,116,105,111,110,47,106,115,111,110,13,10,0
.p2align 1
auth_bearer_prefix: .short 65,117,116,104,111,114,105,122,97,116,105,111,110,58,32,66,101,97,114,101,114,32,0
.p2align 1
auth_token_filename: .short 97,99,99,111,117,110,116,46,100,97,116,0

auth_login_prefix: .asciz "{\"username\":\""
auth_login_middle: .asciz "\",\"password\":\""
auth_login_suffix: .asciz "\",\"product_code\":\"fortuna\",\"client_name\":\"Febius Fortuna\"}"
auth_token_needle: .ascii "\"token\":\""

.p2align 1
auth_login_ready: .short 50976,54952,54620,32,44050,51012,32,51077,47141,54616,49901,49884,50724,46,0
.p2align 1
auth_login_working: .short 47196,44536,51064,32,48143,32,46972,51060,49440,49828,47484,32,54869,51064,54616,44256,32,51080,49845,45768,45796,46,46,46,0
.p2align 1
auth_bad_credentials: .short 50500,51060,46356,32,46608,45716,32,48708,48128,48264,54840,47484,32,54869,51064,54644,32,51452,49464,50836,46,0
.p2align 1
auth_no_license: .short 51060,32,44228,51221,50640,45716,32,70,101,98,105,117,115,32,70,111,114,116,117,110,97,32,49324,50857,32,44428,54620,51060,32,50630,49845,45768,45796,46,0
.p2align 1
auth_rate_limit: .short 47196,44536,51064,32,49884,46020,44032,32,47566,49845,45768,45796,46,32,51104,49884,32,54980,32,45796,49884,32,49884,46020,54644,32,51452,49464,50836,46,0
.p2align 1
auth_connect_fail: .short 70,101,98,105,117,115,32,65,99,99,111,117,110,116,32,49436,48260,50640,32,50672,44208,54624,32,49688,32,50630,49845,45768,45796,46,0
.p2align 1
auth_product_missing: .short 70,101,98,105,117,115,32,65,99,99,111,117,110,116,50640,32,70,111,114,116,117,110,97,32,51228,54408,51060,32,46321,47197,46104,50612,32,51080,51648,32,50506,49845,45768,45796,46,0
.p2align 1
auth_input_invalid: .short 50976,54952,54620,32,44050,51012,32,51077,47141,54616,49901,49884,50724,46,0
.p2align 1
auth_login_failed: .short 47196,44536,51064,32,50836,52397,51012,32,52376,47532,54616,51648,32,47803,54664,49845,45768,45796,46,0

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

# Copy UTF-8 bytes into JSON, escaping JSON syntax/control characters.
# rcx=source, rdx=destination, r8=end-exclusive. rax=new destination or zero.
FN AuthEscapeJson
    mov r12,rcx
    mov r13,rdx
    mov r14,r8
.aej_next:
    mov al,byte ptr [r12]
    test al,al
    jz .aej_done
    inc r12
    cmp al,34
    je .aej_quote
    cmp al,92
    je .aej_backslash
    cmp al,8
    je .aej_b
    cmp al,9
    je .aej_t
    cmp al,10
    je .aej_n
    cmp al,12
    je .aej_f
    cmp al,13
    je .aej_r
    cmp al,32
    jb .aej_fail
    cmp r13,r14
    jae .aej_fail
    mov byte ptr [r13],al
    inc r13
    jmp .aej_next
.aej_quote:
    mov dl,34
    jmp .aej_escape
.aej_backslash:
    mov dl,92
    jmp .aej_escape
.aej_b:
    mov dl,98
    jmp .aej_escape
.aej_t:
    mov dl,116
    jmp .aej_escape
.aej_n:
    mov dl,110
    jmp .aej_escape
.aej_f:
    mov dl,102
    jmp .aej_escape
.aej_r:
    mov dl,114
.aej_escape:
    lea rax,[r13+2]
    cmp rax,r14
    ja .aej_fail
    mov byte ptr [r13],92
    mov byte ptr [r13+1],dl
    add r13,2
    jmp .aej_next
.aej_done:
    mov rax,r13
    jmp .aej_out
.aej_fail:
    xor eax,eax
.aej_out: END

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

FN AuthWideToUtf8,32
    mov r12,rcx
    mov r13,rdx
    mov r14d,r8d
    mov ecx,65001
    xor edx,edx
    mov r8,r12
    mov r9d,-1
    mov [rsp+32],r13
    mov dword ptr [rsp+40],r14d
    mov qword ptr [rsp+48],0
    mov qword ptr [rsp+56],0
    call WideCharToMultiByte
    test eax,eax
    setnz al
    movzx eax,al
    END 32

FN AuthClearSensitive
    lea rcx,[rip+auth_password_w]
    mov edx,2050
    call Zero
    lea rcx,[rip+auth_password8]
    mov edx,4097
    call Zero
    lea rcx,[rip+auth_json]
    mov edx,16384
    call Zero
    END

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

FN AuthBuildLoginJson
    lea rdi,[rip+auth_json]
    mov rcx,rdi
    lea rdx,[rip+auth_login_prefix]
    call AuthAppendAscii
    mov rdi,rax
    lea rcx,[rip+auth_username8]
    mov rdx,rdi
    lea r8,[rip+auth_json+16000]
    call AuthEscapeJson
    test rax,rax
    jz .ablj_fail
    mov rdi,rax
    mov rcx,rdi
    lea rdx,[rip+auth_login_middle]
    call AuthAppendAscii
    mov rdi,rax
    lea rcx,[rip+auth_password8]
    mov rdx,rdi
    lea r8,[rip+auth_json+16000]
    call AuthEscapeJson
    test rax,rax
    jz .ablj_fail
    mov rdi,rax
    mov rcx,rdi
    lea rdx,[rip+auth_login_suffix]
    call AuthAppendAscii
    mov rdi,rax
    mov byte ptr [rdi],0
    lea rax,[rip+auth_json]
    sub rdi,rax
    mov eax,edi
    jmp .ablj_out
.ablj_fail:
    xor eax,eax
.ablj_out: END

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

# rcx=dialog HWND, rdx=wide status string.
FN AuthSetDialogStatus
    mov r8,rdx
    mov edx,1302
    call SetDlgItemTextW
    END

FN AuthClearPasswordControl
    mov r12,rcx
    mov rcx,r12
    mov edx,1301
    lea r8,[rip+empty]
    call SetDlgItemTextW
    call AuthClearSensitive
    END

# rcx=dialog HWND. Returns 1 on successful authenticated+licensed login.
FN AuthLoginFromDialog,16
    mov r12,rcx
    mov rcx,r12
    mov edx,1300
    lea r8,[rip+auth_username_w]
    mov r9d,64
    call GetDlgItemTextW
    cmp eax,3
    jb .alfd_input
    mov rcx,r12
    mov edx,1301
    lea r8,[rip+auth_password_w]
    mov r9d,1025
    call GetDlgItemTextW
    cmp eax,8
    jb .alfd_input

    lea rcx,[rip+auth_username_w]
    lea rdx,[rip+auth_username8]
    mov r8d,128
    call AuthWideToUtf8
    test eax,eax
    jz .alfd_fail
    lea rcx,[rip+auth_password_w]
    lea rdx,[rip+auth_password8]
    mov r8d,4097
    call AuthWideToUtf8
    test eax,eax
    jz .alfd_fail
    call AuthBuildLoginJson
    test eax,eax
    jz .alfd_fail
    mov ebp,eax

    mov rcx,r12
    lea rdx,[rip+auth_login_working]
    call AuthSetDialogStatus

    lea rcx,[rip+auth_post]
    lea rdx,[rip+auth_login_path]
    call AuthOpenRequest
    test eax,eax
    jz .alfd_network

    lea rcx,[rip+auth_json]
    mov edx,ebp
    call AuthPostJson
    mov ebp,eax
    test eax,eax
    jz .alfd_network_close
    call AuthReadResponse
    mov r13d,eax
    call AuthCloseHttp
    cmp ebp,200
    je .alfd_success_response
    cmp ebp,401
    je .alfd_bad_credentials
    cmp ebp,403
    je .alfd_no_license
    cmp ebp,404
    je .alfd_product_missing
    cmp ebp,429
    je .alfd_rate
    jmp .alfd_server

.alfd_success_response:
    test r13d,r13d
    jz .alfd_fail
    lea rcx,[rip+auth_token_needle]
    mov edx,9
    lea r8,[rip+auth_token]
    mov r9d,1024
    call AuthExtractJsonString
    test eax,eax
    jz .alfd_fail
    call AuthSaveToken
    test eax,eax
    jz .alfd_fail
    mov dword ptr [rip+auth_checked],1
    mov rcx,r12
    call AuthClearPasswordControl
    mov eax,1
    jmp .alfd_out

.alfd_input:
    mov rcx,r12
    lea rdx,[rip+auth_input_invalid]
    call AuthSetDialogStatus
    mov rcx,r12
    call AuthClearPasswordControl
    xor eax,eax
    jmp .alfd_out
.alfd_bad_credentials:
    mov rcx,r12
    lea rdx,[rip+auth_bad_credentials]
    call AuthSetDialogStatus
    jmp .alfd_retry
.alfd_no_license:
    mov rcx,r12
    lea rdx,[rip+auth_no_license]
    call AuthSetDialogStatus
    jmp .alfd_retry
.alfd_product_missing:
    mov rcx,r12
    lea rdx,[rip+auth_product_missing]
    call AuthSetDialogStatus
    jmp .alfd_retry
.alfd_rate:
    mov rcx,r12
    lea rdx,[rip+auth_rate_limit]
    call AuthSetDialogStatus
    jmp .alfd_retry
.alfd_server:
    mov rcx,r12
    lea rdx,[rip+auth_login_failed]
    call AuthSetDialogStatus
    jmp .alfd_retry
.alfd_network_close:
    call AuthCloseHttp
.alfd_network:
    mov rcx,r12
    lea rdx,[rip+auth_connect_fail]
    call AuthSetDialogStatus
    jmp .alfd_retry
.alfd_fail:
    mov rcx,r12
    lea rdx,[rip+auth_login_failed]
    call AuthSetDialogStatus
.alfd_retry:
    mov rcx,r12
    call AuthClearPasswordControl
    xor eax,eax
.alfd_out: END 16

FN AuthDialogProc,16
    mov rbx,rcx
    mov ebp,edx
    mov rsi,r8
    cmp edx,0x110
    je .adp_init
    cmp edx,0x111
    je .adp_command
    cmp edx,0x10
    je .adp_cancel
    xor eax,eax
    jmp .adp_out
.adp_init:
    mov rcx,rbx
    mov edx,1300
    call GetDlgItem
    mov rcx,rax
    mov edx,0x00c5
    mov r8d,24
    xor r9d,r9d
    call SendMessageW
    mov rcx,rbx
    mov edx,1301
    call GetDlgItem
    mov rcx,rax
    mov edx,0x00c5
    mov r8d,1024
    xor r9d,r9d
    call SendMessageW
    mov rcx,rbx
    lea rdx,[rip+empty]
    call AuthSetDialogStatus
    mov eax,1
    jmp .adp_out
.adp_command:
    mov eax,esi
    and eax,0xffff
    cmp eax,2
    je .adp_cancel
    cmp eax,1
    jne .adp_true
    mov rcx,rbx
    call AuthLoginFromDialog
    test eax,eax
    jz .adp_true
    mov rcx,rbx
    mov edx,1
    call EndDialog
    jmp .adp_true
.adp_cancel:
    mov rcx,rbx
    call AuthClearPasswordControl
    mov rcx,rbx
    xor edx,edx
    call EndDialog
.adp_true:
    mov eax,1
.adp_out: END 16

FN EnsureLicensed,16
    cmp dword ptr [rip+auth_checked],1
    je .el_ok
    call AuthLoadToken
    test eax,eax
    jz .el_dialog
    call AuthValidateToken
    cmp eax,1
    je .el_mark
    cmp eax,401
    je .el_delete
    cmp eax,403
    jne .el_dialog
.el_delete:
    call AuthDeleteStoredToken
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
