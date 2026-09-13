# ---------------------------------------------------------------------------
# Sampling and presentation
# ---------------------------------------------------------------------------
SampleCandidate:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    sub rsp, 48
    cmp dword ptr [rip + final_count], 0
    je .sample_out

    # Adaptive skepticism gate: some recalculations use a full uniform 6/45 draw.
    lea rcx, [rsp + 32]
    mov edx, 4
    call SystemFunction036
    test eax, eax
    jz .oracle_sample
    mov eax, dword ptr [rsp + 32]
    xor edx, edx
    mov ecx, 100
    div ecx
    cmp edx, dword ptr [rip + skepticism_pct]
    jae .oracle_sample
    call GenerateUniformMask
    mov qword ptr [rip + selected_mask], rax
    mov rcx, rax
    call ScoreDeep
    mov qword ptr [rip + selected_score], rax
    jmp .extract_selected

.oracle_sample:
    # Weight_i = max(score_i - heap_min + 1, 1).
    lea rax, [rip + final_scores]
    mov rbx, qword ptr [rax]
    xor rsi, rsi
    xor ecx, ecx
.sum_weights:
    cmp ecx, dword ptr [rip + final_count]
    jae .have_total
    mov rdx, qword ptr [rax + rcx*8]
    sub rdx, rbx
    inc rdx
    cmp rdx, 1
    jge 1f
    mov edx, 1
1:  add rsi, rdx
    inc ecx
    jmp .sum_weights
.have_total:
    lea rcx, [rsp + 32]
    mov edx, 8
    call SystemFunction036
    test eax, eax
    jz .fallback_first
    mov rax, qword ptr [rsp + 32]
    xor edx, edx
    div rsi
    mov r10, rdx
    xor r11, r11
    xor ecx, ecx
    lea r8, [rip + final_scores]
    lea r9, [rip + final_masks]
.pick_loop:
    mov rdx, qword ptr [r8 + rcx*8]
    sub rdx, rbx
    inc rdx
    cmp rdx, 1
    jge 2f
    mov edx, 1
2:  add r11, rdx
    cmp r10, r11
    jb .picked
    inc ecx
    cmp ecx, dword ptr [rip + final_count]
    jb .pick_loop
    xor ecx, ecx
.picked:
    mov rax, qword ptr [r9 + rcx*8]
    mov qword ptr [rip + selected_mask], rax
    mov rax, qword ptr [r8 + rcx*8]
    mov qword ptr [rip + selected_score], rax
    jmp .extract_selected
.fallback_first:
    lea rax, [rip + final_masks]
    mov rdx, qword ptr [rax]
    mov qword ptr [rip + selected_mask], rdx
    lea rax, [rip + final_scores]
    mov rdx, qword ptr [rax]
    mov qword ptr [rip + selected_score], rdx

.extract_selected:
    mov rax, qword ptr [rip + selected_mask]
    lea r8, [rip + selected_nums]
    xor ecx, ecx
.sel_bits:
    bsf rdx, rax
    btr rax, rdx
    inc edx
    mov dword ptr [r8 + rcx*4], edx
    inc ecx
    cmp ecx, 6
    jb .sel_bits
.sample_out:
    add rsp, 48
    pop rsi
    pop rbx
    leave
    ret

# Exact uniform 6/45 generation: OS cryptographic RNG, modulo-bias rejection,
# then duplicate rejection. Returns a 45-bit mask in rax.
GenerateUniformMask:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    sub rsp, 48
    xor rbx, rbx
    xor esi, esi
.gu_loop:
    lea rcx, [rsp + 32]
    mov edx, 4
    call SystemFunction036
    test eax, eax
    jz .gu_fallback
    mov eax, dword ptr [rsp + 32]
    cmp eax, 0xFFFFFFE1
    jae .gu_loop
    xor edx, edx
    mov ecx, 45
    div ecx
    mov ecx, edx
    bt rbx, rcx
    jc .gu_loop
    bts rbx, rcx
    inc esi
    cmp esi, 6
    jb .gu_loop
    mov rax, rbx
    jmp .gu_out
.gu_fallback:
    # Deterministic fallback only if the OS RNG itself fails.
    mov rax, 0x0000040201008081
.gu_out:
    add rsp, 48
    pop rsi
    pop rbx
    leave
    ret

UpdateResultUI:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    sub rsp, 216
    lea rbx, [rip + selected_nums]
    lea rcx, [rip + result_buf]
    lea rdx, [rip + fmt_result]
    mov r8d, dword ptr [rbx]
    mov r9d, dword ptr [rbx + 4]
    mov eax, dword ptr [rbx + 8]
    mov qword ptr [rsp + 32], rax
    mov eax, dword ptr [rbx + 12]
    mov qword ptr [rsp + 40], rax
    mov eax, dword ptr [rbx + 16]
    mov qword ptr [rsp + 48], rax
    mov eax, dword ptr [rbx + 20]
    mov qword ptr [rsp + 56], rax
    call wsprintfA
    mov rcx, qword ptr [rip + h_result]
    lea rdx, [rip + result_buf]
    call SetWindowTextA

    # Find top five persistence numbers.
    lea rdi, [rsp + 144]
    xor eax, eax
    mov ecx, 10
    rep stosd
    xor esi, esi
.persist_scan:
    cmp esi, 45
    jae .persist_done
    lea rax, [rip + persistence]
    mov edx, dword ptr [rax + rsi*4]
    xor ecx, ecx
.insert_top5:
    cmp ecx, 5
    jae .persist_next
    mov eax, dword ptr [rsp + 144 + rcx*8 + 4]
    cmp edx, eax
    jle .insert_advance
    mov r8d, 4
.shift_top:
    cmp r8d, ecx
    jle .write_top
    mov eax, dword ptr [rsp + 144 + r8*8 - 8]
    mov dword ptr [rsp + 144 + r8*8], eax
    mov eax, dword ptr [rsp + 144 + r8*8 - 4]
    mov dword ptr [rsp + 144 + r8*8 + 4], eax
    dec r8d
    jmp .shift_top
.write_top:
    lea eax, [rsi + 1]
    mov dword ptr [rsp + 144 + rcx*8], eax
    mov dword ptr [rsp + 144 + rcx*8 + 4], edx
    jmp .persist_next
.insert_advance:
    inc ecx
    jmp .insert_top5
.persist_next:
    inc esi
    jmp .persist_scan
.persist_done:
    # ORACLE Index is an internal 0..1000 relative score, not a probability.
    mov rax, qword ptr [rip + selected_score]
    lea rdx, [rip + final_scores]
    sub rax, qword ptr [rdx]
    sar rax, 5
    add eax, 500
    cmp eax, 0
    jge 1f
    xor eax, eax
1:  cmp eax, 1000
    jle 2f
    mov eax, 1000
2:  mov ebx, eax

    lea rcx, [rip + metrics_buf]
    lea rdx, [rip + fmt_metrics]
    mov r8d, ebx
    mov r9d, dword ptr [rip + draw_count]
    mov eax, dword ptr [rip + skepticism_pct]
    mov qword ptr [rsp + 32], rax
    mov rax, qword ptr [rip + selected_score]
    mov qword ptr [rsp + 40], rax
    # top5 pairs: number,persistence percent
    xor esi, esi
    lea rdi, [rsp + 48]
.metric_top_loop:
    cmp esi, 5
    jae .metric_call
    mov eax, dword ptr [rsp + 144 + rsi*8]
    mov r8, rsi
    shl r8, 4
    mov qword ptr [rdi + r8], rax
    mov eax, dword ptr [rsp + 144 + rsi*8 + 4]
    imul eax, eax, 100
    xor edx, edx
    div dword ptr [rip + final_count]
    mov qword ptr [rdi + r8 + 8], rax
    inc esi
    jmp .metric_top_loop
.metric_call:
    call wsprintfA
    mov rcx, qword ptr [rip + h_metrics]
    lea rdx, [rip + metrics_buf]
    call SetWindowTextA
    add rsp, 216
    pop rdi
    pop rsi
    pop rbx
    leave
    ret
