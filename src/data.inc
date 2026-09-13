# ---------------------------------------------------------------------------
# CSV loader and parser
# Accepted rows:
#   round,n1,n2,n3,n4,n5,n6[,bonus]
# or round,date,n1,n2,n3,n4,n5,n6[,bonus]
# Header rows are ignored.
# ---------------------------------------------------------------------------
ChooseAndLoadCsv:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    lea rdi, [rip + ofn_buf]
    xor eax, eax
    mov ecx, 19
    rep stosq
    lea rdi, [rip + file_path]
    mov byte ptr [rdi], 0
    lea rax, [rip + ofn_buf]
    mov dword ptr [rax + 0], 152
    mov rcx, qword ptr [rip + h_main]
    mov qword ptr [rax + 8], rcx
    lea rcx, [rip + filter_csv]
    mov qword ptr [rax + 24], rcx
    mov dword ptr [rax + 44], 1
    lea rcx, [rip + file_path]
    mov qword ptr [rax + 48], rcx
    mov dword ptr [rax + 56], 520
    lea rcx, [rip + open_title]
    mov qword ptr [rax + 88], rcx
    mov dword ptr [rax + 96], OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER
    lea rcx, [rip + ofn_buf]
    call GetOpenFileNameA
    test eax, eax
    jz .done
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + ui_status_loading]
    call SetWindowTextA
    lea rcx, [rip + file_path]
    call LoadCsvFile
    test eax, eax
    jz .error
    lea rcx, [rip + status_buf]
    lea rdx, [rip + ui_status_loaded]
    mov r8d, dword ptr [rip + draw_count]
    call wsprintfA
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + status_buf]
    call SetWindowTextA
    mov rcx, qword ptr [rip + h_btn_recalc]
    xor edx, edx
    call EnableWindow
    mov dword ptr [rip + analysis_ready], 0
    jmp .done
.error:
    mov rcx, qword ptr [rip + h_status]
    lea rdx, [rip + ui_status_error]
    call SetWindowTextA
.done:
    leave
    ret

# rcx=path, returns eax=1 success
LoadCsvFile:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    sub rsp, 72
    mov rbx, rcx
    mov rcx, rbx
    mov edx, GENERIC_READ
    mov r8d, FILE_SHARE_READ
    xor r9d, r9d
    mov qword ptr [rsp + 32], OPEN_EXISTING
    mov qword ptr [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword ptr [rsp + 48], 0
    call CreateFileA
    cmp rax, INVALID_HANDLE_VALUE
    je .fail
    mov rsi, rax
    mov rcx, rsi
    lea rdx, [rip + file_size_q]
    call GetFileSizeEx
    test eax, eax
    jz .close_fail
    mov rax, qword ptr [rip + file_size_q]
    test rax, rax
    jz .close_fail
    cmp rax, 16777216
    ja .close_fail
    inc rax
    mov rdi, rax
    call GetProcessHeap
    mov rcx, rax
    mov edx, HEAP_ZERO_MEMORY
    mov r8, rdi
    call HeapAlloc
    test rax, rax
    jz .close_fail
    mov rbx, rax
    mov rcx, rsi
    mov rdx, rbx
    mov r8d, dword ptr [rip + file_size_q]
    lea r9, [rip + bytes_read]
    mov qword ptr [rsp + 32], 0
    call ReadFile
    test eax, eax
    jz .free_fail
    mov eax, dword ptr [rip + bytes_read]
    mov byte ptr [rbx + rax], 0
    mov rcx, rbx
    mov edx, eax
    call ParseCsv
    mov edi, eax
    call GetProcessHeap
    mov rcx, rax
    xor edx, edx
    mov r8, rbx
    call HeapFree
    mov rcx, rsi
    call CloseHandle
    mov eax, edi
    jmp .out
.free_fail:
    call GetProcessHeap
    mov rcx, rax
    xor edx, edx
    mov r8, rbx
    call HeapFree
.close_fail:
    mov rcx, rsi
    call CloseHandle
.fail:
    xor eax, eax
.out:
    add rsp, 72
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

ResetStats:
    push rdi
    xor eax, eax
    lea rdi, [rip + freq_total]
    mov ecx, 83628
    rep stosb
    mov dword ptr [rip + draw_count], 0
    mov dword ptr [rip + recent_pos], 0
    mov dword ptr [rip + final_count], 0
    mov dword ptr [rip + fast_count], 0
    mov dword ptr [rip + analysis_ready], 0
    pop rdi
    ret

# rcx=buffer rdx=len -> eax=1 if >=1 valid draw
ParseCsv:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    sub rsp, 40
    mov rbx, rcx
    mov r12, rdx
    call ResetStats
    xor rsi, rsi
.line_loop:
    cmp rsi, r12
    jae .finish
.skip_eol:
    cmp rsi, r12
    jae .finish
    mov al, byte ptr [rbx + rsi]
    cmp al, 13
    je .skip_one
    cmp al, 10
    jne .parse_line
.skip_one:
    inc rsi
    jmp .skip_eol
.parse_line:
    lea rdi, [rip + columns]
    xor eax, eax
    mov ecx, 12
    rep stosd
    xor r13d, r13d
.col_loop:
    cmp r13d, 12
    jae .skip_to_eol
    xor r8d, r8d
    xor r9d, r9d
.scan_col:
    cmp rsi, r12
    jae .store_col_end
    mov al, byte ptr [rbx + rsi]
    cmp al, ','
    je .store_col_comma
    cmp al, 13
    je .store_col_end
    cmp al, 10
    je .store_col_end
    cmp al, '0'
    jb .advance_char
    cmp al, '9'
    ja .advance_char
    test r9d, r9d
    jnz .acc_digit
    mov r9d, 1
.acc_digit:
    imul r8d, r8d, 10
    movzx eax, al
    sub eax, '0'
    add r8d, eax
.advance_char:
    inc rsi
    jmp .scan_col
.store_col_comma:
    lea rax, [rip + columns]
    mov dword ptr [rax + r13*4], r8d
    inc r13d
    inc rsi
    jmp .col_loop
.store_col_end:
    lea rax, [rip + columns]
    mov dword ptr [rax + r13*4], r8d
    inc r13d
    jmp .evaluate_line
.skip_to_eol:
    cmp rsi, r12
    jae .evaluate_line
    mov al, byte ptr [rbx + rsi]
    cmp al, 10
    je .evaluate_line
    inc rsi
    jmp .skip_to_eol
.evaluate_line:
    cmp dword ptr [rip + columns], 1
    jb .line_advance
    mov r10d, 1
    lea rcx, [rip + columns]
    call ValidateSixAt
    test eax, eax
    jnz .valid_row
    mov r10d, 2
    lea rcx, [rip + columns]
    call ValidateSixAt
    test eax, eax
    jz .line_advance
.valid_row:
    lea rax, [rip + columns]
    lea rdi, [rip + work_nums]
    xor ecx, ecx
.copy6:
    mov edx, dword ptr [rax + r10*4]
    mov dword ptr [rdi + rcx*4], edx
    inc r10d
    inc ecx
    cmp ecx, 6
    jb .copy6
    lea rcx, [rip + work_nums]
    call SortSix
    lea rcx, [rip + work_nums]
    call ValidateUniqueSix
    test eax, eax
    jz .line_advance
    lea rcx, [rip + work_nums]
    call ProcessDraw
.line_advance:
    cmp rsi, r12
    jae .finish
    mov al, byte ptr [rbx + rsi]
    cmp al, 13
    jne 1f
    inc rsi
1:  cmp rsi, r12
    jae .finish
    mov al, byte ptr [rbx + rsi]
    cmp al, 10
    jne .line_loop
    inc rsi
    jmp .line_loop
.finish:
    cmp dword ptr [rip + draw_count], 0
    setne al
    movzx eax, al
    add rsp, 40
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

ValidateSixAt:
    xor eax, eax
    xor r11d, r11d
.vloop:
    mov edx, r10d
    add edx, r11d
    mov edx, dword ptr [rcx + rdx*4]
    cmp edx, 1
    jb .bad
    cmp edx, 45
    ja .bad
    inc r11d
    cmp r11d, 6
    jb .vloop
    mov eax, 1
.bad:
    ret

SortSix:
    push rbx
    mov rbx, rcx
    mov r8d, 1
.outer_sort:
    cmp r8d, 6
    jae .sort_done
    mov eax, dword ptr [rbx + r8*4]
    mov r9d, r8d
.inner_sort:
    test r9d, r9d
    jz .place_sort
    mov edx, dword ptr [rbx + r9*4 - 4]
    cmp edx, eax
    jle .place_sort
    mov dword ptr [rbx + r9*4], edx
    dec r9d
    jmp .inner_sort
.place_sort:
    mov dword ptr [rbx + r9*4], eax
    inc r8d
    jmp .outer_sort
.sort_done:
    pop rbx
    ret

ValidateUniqueSix:
    mov eax, 1
    mov r8d, 1
.uq:
    mov edx, dword ptr [rcx + r8*4]
    cmp edx, dword ptr [rcx + r8*4 - 4]
    je .dup
    inc r8d
    cmp r8d, 6
    jb .uq
    ret
.dup:
    xor eax, eax
    ret

# rcx -> six sorted dwords
ProcessDraw:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    sub rsp, 32
    mov rbx, rcx
    mov eax, dword ptr [rip + draw_count]
    inc eax
    mov dword ptr [rip + draw_count], eax
    mov r12d, eax

    xor esi, esi
.decay_loop:
    lea rdi, [rip + freq_short]
    mov eax, dword ptr [rdi + rsi*4]
    imul rax, SHORT_DECAY_Q16
    shr rax, 16
    mov dword ptr [rdi + rsi*4], eax
    lea rdi, [rip + freq_mid]
    mov eax, dword ptr [rdi + rsi*4]
    imul rax, MID_DECAY_Q16
    shr rax, 16
    mov dword ptr [rdi + rsi*4], eax
    lea rdi, [rip + freq_long]
    mov eax, dword ptr [rdi + rsi*4]
    imul rax, LONG_DECAY_Q16
    shr rax, 16
    mov dword ptr [rdi + rsi*4], eax
    inc esi
    cmp esi, 45
    jb .decay_loop

    xor r11, r11
    xor esi, esi
    xor r8d, r8d
    xor r9d, r9d
    xor r10d, r10d
.num_update:
    mov eax, dword ptr [rbx + rsi*4]
    add r8d, eax
    test eax, 1
    jz 1f
    inc r9d
1:  cmp eax, 22
    ja 2f
    inc r10d
2:  dec eax
    lea rdi, [rip + freq_total]
    inc dword ptr [rdi + rax*4]
    lea rdi, [rip + freq_short]
    add dword ptr [rdi + rax*4], SHORT_ADD_Q16
    lea rdi, [rip + freq_mid]
    add dword ptr [rdi + rax*4], MID_ADD_Q16
    lea rdi, [rip + freq_long]
    add dword ptr [rdi + rax*4], LONG_ADD_Q16
    lea rdi, [rip + last_seen]
    mov dword ptr [rdi + rax*4], r12d
    mov ecx, eax
    mov rdx, 1
    shl rdx, cl
    or r11, rdx
    inc esi
    cmp esi, 6
    jb .num_update

    xor esi, esi
.pair_i:
    mov eax, dword ptr [rbx + rsi*4]
    dec eax
    mov edi, eax
    inc esi
    mov ecx, esi
.pair_j:
    cmp ecx, 6
    jae .pair_next_i
    mov eax, dword ptr [rbx + rcx*4]
    dec eax
    imul edx, edi, 45
    add edx, eax
    lea rax, [rip + pair_counts]
    inc dword ptr [rax + rdx*4]
    inc ecx
    jmp .pair_j
.pair_next_i:
    cmp esi, 5
    jb .pair_i

    mov eax, r8d
    cdqe
    add qword ptr [rip + sum_acc], rax
    imul rax, rax
    add qword ptr [rip + sum_sq_acc], rax
    mov eax, dword ptr [rbx + 20]
    sub eax, dword ptr [rbx]
    mov ecx, eax
    cdqe
    add qword ptr [rip + range_acc], rax
    movsxd rax, ecx
    imul rax, rax
    add qword ptr [rip + range_sq_acc], rax
    lea rax, [rip + hist_odd]
    inc dword ptr [rax + r9*4]
    lea rax, [rip + hist_low]
    inc dword ptr [rax + r10*4]
    xor esi, esi
    xor ecx, ecx
.adj_loop:
    cmp esi, 5
    jae .adj_done
    mov eax, dword ptr [rbx + rsi*4 + 4]
    sub eax, dword ptr [rbx + rsi*4]
    cmp eax, 1
    jne 3f
    inc ecx
3:  inc esi
    jmp .adj_loop
.adj_done:
    lea rax, [rip + hist_adj]
    inc dword ptr [rax + rcx*4]

    mov eax, dword ptr [rip + recent_pos]
    and eax, 63
    lea rdx, [rip + recent_masks]
    mov qword ptr [rdx + rax*8], r11
    inc dword ptr [rip + recent_pos]

    add rsp, 32
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret
