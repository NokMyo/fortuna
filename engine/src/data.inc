# Activate an inclusive source-history range as the dataset seen by every
# analysis component. rcx=first round, rdx=last round. Requires >=60 rows.
# This is intentionally stronger than selecting validation folds: final fitting,
# reports, target round and deep analysis all see only the activated slice.
FN ActivateHistoryRange
    mov ebx,ecx
    mov esi,edx
    mov eax,[rip+source_history_count]
    cmp eax,1
    jb .ahr_fail
    cmp ebx,[rip+source_first_round]
    jb .ahr_fail
    cmp esi,[rip+source_last_round]
    ja .ahr_fail
    cmp esi,ebx
    jb .ahr_fail
    mov ebp,esi
    sub ebp,ebx
    inc ebp
    cmp ebp,1
    jb .ahr_fail
    mov eax,ebx
    sub eax,[rip+source_first_round]
    imul eax,eax,RECORD_SIZE
    lea rdx,[rip+source_history]
    add rdx,rax
    lea rcx,[rip+history]
    mov r8d,ebp
    imul r8d,r8d,RECORD_SIZE
    call Copy
    mov [rip+history_count],ebp
    mov [rip+last_round],esi
    mov [rip+backtest_start_round],ebx
    mov [rip+backtest_end_round],esi
    mov dword ptr [rip+backtest_start_index],60
    mov [rip+backtest_end_index],ebp
    mov dword ptr [rip+model_ready],0
    mov dword ptr [rip+research_ready],0
    mov dword ptr [rip+advanced_ready],0
    mov dword ptr [rip+ti_ready],0
    mov dword ptr [rip+forecast_valid],0
    mov dword ptr [rip+field_ready],0
    mov dword ptr [rip+field_count],0
    mov dword ptr [rip+session_count],0
    mov qword ptr [rip+selected_mask],0
    mov dword ptr [rip+selected_index],0
    mov dword ptr [rip+validation_complete],0
    mov dword ptr [rip+validation_count],0
    mov qword ptr [rip+evidence],0
    movsd xmm0,[rip+one]
    movsd [rip+rho],xmm0
    lea rcx,[rip+history]
    mov edx,[rip+history_count]
    imul edx,edx,RECORD_SIZE
    lea r8,[rip+hash_dataset]
    call Sha256
    mov ecx,[rip+history_count]
    call RebuildStats
    mov eax,1
    jmp .ahr_out
.ahr_fail:
    xor eax,eax
.ahr_out: END

# Strict transactional CSV. No silently skipped malformed rows.
# record: round:u32,date:YYYYMMDD,u32 numbers[6],bonus:u32,reserved:u32,mask:u64.
# Returns 1; on failure history and active field are untouched. parse_error and
# parse_line describe rejected input. Explicit contiguous round history required.
FN ParseCsv,96
    mov rbx,rcx
    mov r12,rdx
    xor esi,esi
    mov dword ptr [rip+parse_error],0
    mov dword ptr [rip+parse_line],0
    mov dword ptr [rip+staging_count],0
    cmp r12,3
    jb .csv_line
    cmp byte ptr [rbx],0xef
    jne .csv_line
    cmp byte ptr [rbx+1],0xbb
    jne .csv_line
    cmp byte ptr [rbx+2],0xbf
    jne .csv_line
    mov esi,3
.csv_line:
    cmp rsi,r12
    jae .csv_finish
    inc dword ptr [rip+parse_line]
    # Empty lines are permitted; a header is permitted only on physical line 1.
    mov al,[rbx+rsi]
    cmp al,13
    je .csv_eol
    cmp al,10
    je .csv_eol
    cmp dword ptr [rip+parse_line],1
    jne .csv_fields_start
    cmp al,'r'
    je .csv_header
    cmp al,'R'
    je .csv_header
    jmp .csv_fields_start
.csv_header:
    # Header must begin with literal round (case of R accepted).
    lea rax,[rsi+5]
    cmp rax,r12
    jae .csv_bad_format
    cmp byte ptr [rbx+rsi+1],'o'
    jne .csv_bad_format
    cmp byte ptr [rbx+rsi+2],'u'
    jne .csv_bad_format
    cmp byte ptr [rbx+rsi+3],'n'
    jne .csv_bad_format
    cmp byte ptr [rbx+rsi+4],'d'
    jne .csv_bad_format
    cmp byte ptr [rbx+rsi+5],','
    jne .csv_bad_format
1:  cmp rsi,r12
    jae .csv_finish
    mov al,[rbx+rsi]
    cmp al,10
    je .csv_eol
    inc rsi
    jmp 1b
.csv_fields_start:
    xor r13d,r13d
    lea rcx,[rip+csv_columns]
    mov edx,80
    call Zero
    lea rcx,[rip+csv_colflags]
    mov edx,40
    call Zero
.csv_field:
    cmp r13d,9
    jae .csv_bad_format
    xor r14d,r14d
    xor r15d,r15d
    xor ebp,ebp
    mov dword ptr [rsp+32],0
    mov dword ptr [rsp+36],0
.csv_leading:
    cmp rsi,r12
    jae .csv_bad_format
    mov al,[rbx+rsi]
    cmp al,' '
    je 1f
    cmp al,9
    jne 2f
1:  inc rsi
    jmp .csv_leading
2:  cmp al,'"'
    jne .csv_digit
    mov ebp,1
    inc rsi
.csv_digit:
    cmp rsi,r12
    jae .csv_end_field
    movzx eax,byte ptr [rbx+rsi]
    cmp eax,'0'
    jb .csv_other
    cmp eax,'9'
    ja .csv_other
    cmp r15d,8
    jae .csv_bad_format
    imul r14,r14,10
    sub eax,'0'
    add r14,rax
    inc r15d
    inc rsi
    jmp .csv_digit
.csv_other:
    cmp al,'-'
    jne .csv_end_field
    cmp r13d,1
    jne .csv_bad_format
    cmp r15d,4
    je 1f
    cmp r15d,6
    jne .csv_bad_format
    cmp dword ptr [rsp+32],1
    jne .csv_bad_format
    jmp 2f
1:  cmp dword ptr [rsp+32],0
    jne .csv_bad_format
2:  inc dword ptr [rsp+32]
    inc rsi
    jmp .csv_digit
.csv_end_field:
    test r15d,r15d
    jz .csv_bad_format
    test ebp,ebp
    jz .csv_trailing
    cmp rsi,r12
    jae .csv_bad_format
    cmp byte ptr [rbx+rsi],'"'
    jne .csv_bad_format
    inc rsi
.csv_trailing:
    cmp rsi,r12
    jae .csv_store
    mov al,[rbx+rsi]
    cmp al,' '
    je 1f
    cmp al,9
    jne .csv_store
1:  inc rsi
    jmp .csv_trailing
.csv_store:
    cmp dword ptr [rsp+32],0
    je 1f
    cmp dword ptr [rsp+32],2
    jne .csv_bad_format
    cmp r15d,8
    jne .csv_bad_format
1:  lea rax,[rip+csv_columns]
    mov [rax+r13*8],r14
    lea rax,[rip+csv_colflags]
    mov edx,[rsp+32]
    mov [rax+r13*4],edx
    inc r13d
    cmp rsi,r12
    jae .csv_evaluate
    mov al,[rbx+rsi]
    cmp al,13
    je .csv_evaluate
    cmp al,10
    je .csv_evaluate
    cmp al,','
    jne .csv_bad_format
    inc rsi
    jmp .csv_field
.csv_evaluate:
    cmp r13d,7
    jb .csv_bad_format
    lea rdi,[rip+csv_columns]
    mov rax,[rdi]
    cmp eax,1
    jb .csv_bad_format
    cmp eax,1000000
    ja .csv_bad_format
    mov edx,[rip+staging_count]
    cmp edx,MAX_DRAWS
    jae .csv_too_large
    imul edx,edx,RECORD_SIZE
    lea rbp,[rip+staging]
    add rbp,rdx
    mov [rbp],eax
    mov dword ptr [rbp+4],0
    mov dword ptr [rbp+32],0
    mov dword ptr [rbp+36],0
    mov r14d,1
    cmp qword ptr [rdi+8],10000000
    jb .csv_no_date
    mov ecx,[rdi+8]
    call ValidateDate
    test eax,eax
    jz .csv_bad_date
    mov eax,[rdi+8]
    mov [rbp+4],eax
    inc r14d
.csv_no_date:
    lea eax,[r14+6]
    cmp r13d,eax
    jb .csv_bad_format
    inc eax
    cmp r13d,eax
    ja .csv_bad_format
    xor r15d,r15d
    xor r11d,r11d
.csv_six:
    mov eax,[rdi+r14*8]
    cmp eax,1
    jb .csv_bad_number
    cmp eax,45
    ja .csv_bad_number
    lea ecx,[rax-1]
    bt r11,rcx
    jc .csv_bad_number
    bts r11,rcx
    inc r14d
    inc r15d
    cmp r15d,6
    jb .csv_six
    mov [rbp+40],r11
    cmp r13d,r14d
    je .csv_bonus_done
    mov eax,[rdi+r14*8]
    cmp eax,1
    jb .csv_bad_number
    cmp eax,45
    ja .csv_bad_number
    lea ecx,[rax-1]
    bt r11,rcx
    jc .csv_bad_number
    mov [rbp+32],eax
.csv_bonus_done:
    mov rcx,r11
    lea rdx,[rbp+8]
    call MaskToNumbers
    inc dword ptr [rip+staging_count]
.csv_eol:
    cmp rsi,r12
    jae .csv_finish
    cmp byte ptr [rbx+rsi],13
    jne 1f
    inc rsi
1:  cmp rsi,r12
    jae .csv_finish
    cmp byte ptr [rbx+rsi],10
    jne .csv_line
    inc rsi
    jmp .csv_line
.csv_finish:
    cmp dword ptr [rip+staging_count],1
    jb .csv_bad_format
    # Canonical insertion sort: row order in files never changes the analysis.
    mov esi,1
    lea rbx,[rip+staging]
.csv_sort:
    cmp esi,[rip+staging_count]
    jae .csv_order_check
    imul eax,esi,RECORD_SIZE
    lea rdx,[rbx+rax]
    lea rcx,[rsp+48]
    mov r8d,RECORD_SIZE
    call Copy
    mov edi,esi
1:  test edi,edi
    jz 2f
    imul eax,edi,RECORD_SIZE
    mov ecx,[rsp+48]
    cmp [rbx+rax-RECORD_SIZE],ecx
    jbe 2f
    lea rcx,[rbx+rax]
    lea rdx,[rcx-RECORD_SIZE]
    mov r8d,RECORD_SIZE
    call Copy
    dec edi
    jmp 1b
2:  imul eax,edi,RECORD_SIZE
    lea rcx,[rbx+rax]
    lea rdx,[rsp+48]
    mov r8d,RECORD_SIZE
    call Copy
    inc esi
    jmp .csv_sort
.csv_order_check:
    mov esi,1
1:  cmp esi,[rip+staging_count]
    jae .csv_commit
    imul eax,esi,RECORD_SIZE
    mov ecx,[rbx+rax-RECORD_SIZE]
    inc ecx
    cmp [rbx+rax],ecx
    jne .csv_bad_order
    mov ecx,[rbx+rax-RECORD_SIZE+4]
    mov edx,[rbx+rax+4]
    test ecx,ecx
    jz 2f
    test edx,edx
    jz .csv_bad_date
    cmp edx,ecx
    jbe .csv_bad_date
    jmp 3f
2:  test edx,edx
    jnz .csv_bad_date
3:  inc esi
    jmp 1b
.csv_commit:
    # Preserve the complete imported dataset, then activate its full range.
    lea rcx,[rip+source_history]
    lea rdx,[rip+staging]
    mov r8d,[rip+staging_count]
    imul r8d,r8d,RECORD_SIZE
    call Copy
    mov eax,[rip+staging_count]
    mov [rip+source_history_count],eax
    lea rdx,[rip+source_history]
    mov ecx,[rdx]
    mov [rip+source_first_round],ecx
    dec eax
    imul eax,eax,RECORD_SIZE
    mov edx,[rdx+rax]
    mov [rip+source_last_round],edx
    call ActivateHistoryRange
    test eax,eax
    jz .csv_bad_format
    mov eax,1
    jmp .csv_out
.csv_bad_format: mov eax,1; jmp .csv_fail
.csv_bad_number: mov eax,2; jmp .csv_fail
.csv_bad_order: mov eax,3; jmp .csv_fail
.csv_bad_date: mov eax,4; jmp .csv_fail
.csv_too_large: mov eax,5
.csv_fail:
    mov [rip+parse_error],eax
    xor eax,eax
.csv_out:
    END 96

# YYYYMMDD, Gregorian leap-year validation, 2002..9999.
FN ValidateDate
    mov eax,ecx
    xor edx,edx
    mov r8d,100
    div r8d
    mov r9d,edx
    xor edx,edx
    div r8d
    mov r10d,edx
    cmp eax,2002
    jb .date_bad
    cmp eax,9999
    ja .date_bad
    cmp r10d,1
    jb .date_bad
    cmp r10d,12
    ja .date_bad
    mov r11d,31
    cmp r10d,2
    je .date_feb
    cmp r10d,4
    je .date_30
    cmp r10d,6
    je .date_30
    cmp r10d,9
    je .date_30
    cmp r10d,11
    jne .date_day
.date_30: mov r11d,30; jmp .date_day
.date_feb:
    mov r11d,28
    test eax,3
    jnz .date_day
    mov ecx,eax
    xor edx,edx
    div r8d
    test edx,edx
    jnz .date_leap
    mov eax,ecx
    mov r8d,400
    xor edx,edx
    div r8d
    test edx,edx
    jnz .date_day
.date_leap: inc r11d
.date_day:
    cmp r9d,1
    jb .date_bad
    cmp r9d,r11d
    ja .date_bad
    mov eax,1
    jmp .date_out
.date_bad: xor eax,eax
.date_out: END
