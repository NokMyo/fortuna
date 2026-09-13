# ---------------------------------------------------------------------------
# ORACLE engine
# ---------------------------------------------------------------------------
AnalysisThread:
    sub rsp, 40
    call AnalyzeOracle
    call SampleCandidate
    mov rcx, qword ptr [rip + h_main]
    mov edx, WM_APP_DONE
    xor r8d, r8d
    xor r9d, r9d
    call PostMessageA
    xor eax, eax
    add rsp, 40
    ret

AnalyzeOracle:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 40
    mov dword ptr [rip + fast_count], 0
    mov dword ptr [rip + final_count], 0

    # Stage A: exhaustive C(45,6) scan.
    # Loop indices stay in non-volatile registers across scoring calls.
    mov esi, 1
.a_loop:
    cmp esi, 40
    ja .stage_b
    lea edi, [rsi + 1]
.b_loop:
    cmp edi, 41
    ja .next_a
    lea r12d, [rdi + 1]
.c_loop:
    cmp r12d, 42
    ja .next_b
    lea r13d, [r12 + 1]
.d_loop:
    cmp r13d, 43
    ja .next_c
    lea r14d, [r13 + 1]
.e_loop:
    cmp r14d, 44
    ja .next_d
    lea r15d, [r14 + 1]
.f_loop:
    cmp r15d, 45
    ja .next_e
    xor rbx, rbx
    mov ecx, esi
    dec ecx
    bts rbx, rcx
    mov ecx, edi
    dec ecx
    bts rbx, rcx
    mov ecx, r12d
    dec ecx
    bts rbx, rcx
    mov ecx, r13d
    dec ecx
    bts rbx, rcx
    mov ecx, r14d
    dec ecx
    bts rbx, rcx
    mov ecx, r15d
    dec ecx
    bts rbx, rcx
    mov rcx, rbx
    call ScoreFast
    mov rcx, rbx
    mov rdx, rax
    call FastHeapInsert
    inc r15d
    jmp .f_loop
.next_e:
    inc r14d
    jmp .e_loop
.next_d:
    inc r13d
    jmp .d_loop
.next_c:
    inc r12d
    jmp .c_loop
.next_b:
    inc edi
    jmp .b_loop
.next_a:
    inc esi
    jmp .a_loop

.stage_b:
    # Deep-score the 4096 survivors, retain 512.
    xor esi, esi
.deep_loop:
    cmp esi, dword ptr [rip + fast_count]
    jae .deep_done
    lea rax, [rip + fast_masks]
    mov rbx, qword ptr [rax + rsi*8]
    mov rcx, rbx
    call ScoreDeep
    mov rcx, rbx
    mov rdx, rax
    call FinalHeapInsert
    inc esi
    jmp .deep_loop
.deep_done:
    call BuildPersistence
    call ComputeSkepticism
    add rsp, 40
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

# ScoreFast(mask) -> signed rax
# Components: frequency residual, multi-scale EWMA, gap, pair residual,
# empirical structural density and dispersion.
ScoreFast:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    sub rsp, 72
    mov rbx, rcx
    lea rdi, [rsp]
    xor esi, esi
    xor r12, r12
    xor r13d, r13d
    xor r8d, r8d
    xor r9d, r9d
.extract:
    bsf rcx, rbx
    mov dword ptr [rdi + rsi*4], ecx
    btr rbx, rcx
    lea rax, [rip + freq_total]
    mov eax, dword ptr [rax + rcx*4]
    imul eax, eax, 45
    mov edx, dword ptr [rip + draw_count]
    imul edx, edx, 6
    sub eax, edx
    movsxd rax, eax
    imul rax, rax, 12
    add r12, rax
    lea rax, [rip + freq_short]
    mov eax, dword ptr [rax + rcx*4]
    sub eax, EXPECTED_Q16
    movsxd rax, eax
    imul rax, rax, 3
    add r12, rax
    lea rax, [rip + freq_mid]
    mov eax, dword ptr [rax + rcx*4]
    sub eax, EXPECTED_Q16
    movsxd rax, eax
    add r12, rax
    lea rax, [rip + freq_long]
    mov eax, dword ptr [rax + rcx*4]
    sub eax, EXPECTED_Q16
    cdqe
    sar rax, 1
    add r12, rax
    lea rax, [rip + last_seen]
    mov eax, dword ptr [rax + rcx*4]
    mov edx, dword ptr [rip + draw_count]
    sub edx, eax
    cmp edx, 24
    jbe 1f
    mov edx, 24
1:  sub edx, 6
    imul edx, edx, 30
    movsxd rax, edx
    add r12, rax
    lea eax, [rcx + 1]
    add r13d, eax
    test eax, 1
    jz 2f
    inc r8d
2:  cmp eax, 22
    ja 3f
    inc r9d
3:  inc esi
    cmp esi, 6
    jb .extract

    # Pair residual: observed*66 - draw_count, because P(pair)=1/66.
    xor r10, r10
    xor esi, esi
.p_i:
    mov eax, dword ptr [rdi + rsi*4]
    mov r11d, esi
    inc r11d
.p_j:
    cmp r11d, 6
    jae .p_next
    mov edx, dword ptr [rdi + r11*4]
    imul ecx, eax, 45
    add ecx, edx
    lea rdx, [rip + pair_counts]
    mov edx, dword ptr [rdx + rcx*4]
    imul edx, edx, 66
    sub edx, dword ptr [rip + draw_count]
    movsxd rdx, edx
    imul rdx, rdx, 9
    add r10, rdx
    inc r11d
    jmp .p_j
.p_next:
    inc esi
    cmp esi, 5
    jb .p_i

    # Empirical structure density from odd/low histograms.
    xor rbx, rbx
    lea rax, [rip + hist_odd]
    mov eax, dword ptr [rax + r8*4]
    imul eax, eax, 8000
    xor edx, edx
    div dword ptr [rip + draw_count]
    mov ebx, eax
    lea rax, [rip + hist_low]
    mov eax, dword ptr [rax + r9*4]
    imul eax, eax, 6000
    xor edx, edx
    div dword ptr [rip + draw_count]
    add ebx, eax

    # Sum distance from historical center.
    mov rax, qword ptr [rip + sum_acc]
    xor edx, edx
    mov ecx, dword ptr [rip + draw_count]
    div rcx
    mov ecx, r13d
    sub ecx, eax
    movsxd rax, ecx
    imul rax, rax
    imul rax, rax, 12
    sub rbx, rax

    # Range distance from historical center.
    mov eax, dword ptr [rdi + 20]
    sub eax, dword ptr [rdi]
    mov ecx, eax
    mov rax, qword ptr [rip + range_acc]
    xor edx, edx
    mov r11d, dword ptr [rip + draw_count]
    div r11
    sub ecx, eax
    movsxd rax, ecx
    imul rax, rax
    imul rax, rax, 15
    sub rbx, rax

    # Dispersion/entropy proxy: penalize decade-bucket collisions.
    lea r11, [rsp + 32]
    mov qword ptr [r11], 0
    mov qword ptr [r11 + 8], 0
    mov dword ptr [r11 + 16], 0
    xor esi, esi
.bucket_loop:
    mov ecx, dword ptr [rdi + rsi*4]
    lea ecx, [rcx + 1]
    cmp ecx, 10
    jb .bucket0
    cmp ecx, 20
    jb .bucket1
    cmp ecx, 30
    jb .bucket2
    cmp ecx, 40
    jb .bucket3
    mov ecx, 4
    jmp .bucket_add
.bucket0: xor ecx, ecx; jmp .bucket_add
.bucket1: mov ecx, 1; jmp .bucket_add
.bucket2: mov ecx, 2; jmp .bucket_add
.bucket3: mov ecx, 3
.bucket_add:
    inc dword ptr [r11 + rcx*4]
    inc esi
    cmp esi, 6
    jb .bucket_loop
    xor ecx, ecx
    xor edx, edx
.ent_loop:
    mov eax, dword ptr [r11 + rcx*4]
    imul eax, eax
    add edx, eax
    inc ecx
    cmp ecx, 5
    jb .ent_loop
    mov eax, 36
    sub eax, edx
    imul eax, eax, 120
    movsxd rax, eax
    add rbx, rax

    mov rax, r12
    add rax, r10
    add rax, rbx
    add rsp, 72
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

# Deep score adds anti-overfit, crowd avoidance, recent-overlap and robustness.
ScoreDeep:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    sub rsp, 64
    mov rbx, rcx
    call ScoreFast
    mov r12, rax
    lea rdi, [rsp + 32]
    mov rax, rbx
    xor esi, esi
.extract_deep:
    bsf rcx, rax
    mov dword ptr [rdi + rsi*4], ecx
    btr rax, rcx
    inc esi
    cmp esi, 6
    jb .extract_deep

    xor r8d, r8d
    xor r9d, r9d
    xor esi, esi
.crowd_loop:
    mov eax, dword ptr [rdi + rsi*4]
    lea eax, [rax + 1]
    cmp eax, 31
    jbe 1f
    inc r8d
1:  xor edx, edx
    mov ecx, 5
    div ecx
    test edx, edx
    jnz 2f
    inc r9d
2:  inc esi
    cmp esi, 6
    jb .crowd_loop
    imul eax, r8d, 350
    add r12, rax
    cmp r9d, 2
    jbe 3f
    sub r9d, 2
    imul eax, r9d, 450
    sub r12, rax
3:
    # Same-last-digit pair penalty.
    xor r10d, r10d
    xor esi, esi
.end_i:
    mov eax, dword ptr [rdi + rsi*4]
    inc eax
    xor edx, edx
    mov ecx, 10
    div ecx
    mov r11d, edx
    mov ecx, esi
    inc ecx
.end_j:
    cmp ecx, 6
    jae .end_next
    mov eax, dword ptr [rdi + rcx*4]
    inc eax
    xor edx, edx
    push rcx
    mov ecx, 10
    div ecx
    pop rcx
    cmp edx, r11d
    jne 4f
    inc r10d
4:  inc ecx
    jmp .end_j
.end_next:
    inc esi
    cmp esi, 5
    jb .end_i
    imul eax, r10d, 220
    sub r12, rax

    # Arithmetic-pattern penalty for equal consecutive gaps.
    xor r10d, r10d
    xor esi, esi
.gap_eq:
    cmp esi, 4
    jae .gap_done
    mov eax, dword ptr [rdi + rsi*4 + 4]
    sub eax, dword ptr [rdi + rsi*4]
    mov edx, dword ptr [rdi + rsi*4 + 8]
    sub edx, dword ptr [rdi + rsi*4 + 4]
    cmp eax, edx
    jne 5f
    inc r10d
5:  inc esi
    jmp .gap_eq
.gap_done:
    imul eax, r10d, 180
    sub r12, rax

    # Penalize >=4-number overlap with any of the latest 64 draws.
    xor esi, esi
.recent_loop:
    cmp esi, 64
    jae .recent_done
    lea rax, [rip + recent_masks]
    mov rcx, qword ptr [rax + rsi*8]
    test rcx, rcx
    jz .recent_next
    and rcx, rbx
    call PopCount64
    cmp eax, 4
    jb .recent_next
    sub eax, 3
    imul eax, eax, 900
    sub r12, rax
.recent_next:
    inc esi
    jmp .recent_loop
.recent_done:
    # Soft clamp reduces single-feature domination.
    mov rax, r12
    mov rcx, rax
    sar rcx, 13
    mov rdx, rcx
    imul rdx, rdx
    cmp rdx, 20000
    jbe 6f
    sub rdx, 20000
    sar rdx, 2
    sub rax, rdx
6:  add rsp, 64
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

# Software popcount avoids requiring a specific x86-64 extension.
PopCount64:
    mov rax, rcx
    mov rdx, rax
    shr rdx, 1
    mov rcx, 0x5555555555555555
    and rdx, rcx
    sub rax, rdx
    mov rdx, rax
    mov rcx, 0x3333333333333333
    and rax, rcx
    shr rdx, 2
    and rdx, rcx
    add rax, rdx
    mov rdx, rax
    shr rdx, 4
    add rax, rdx
    mov rcx, 0x0F0F0F0F0F0F0F0F
    and rax, rcx
    mov rcx, 0x0101010101010101
    imul rax, rcx
    shr rax, 56
    ret

FastHeapInsert:
    lea r8, [rip + fast_masks]
    lea r9, [rip + fast_scores]
    lea r10, [rip + fast_count]
    mov r11d, TOP_FAST
    jmp HeapInsertGeneric
FinalHeapInsert:
    lea r8, [rip + final_masks]
    lea r9, [rip + final_scores]
    lea r10, [rip + final_count]
    mov r11d, TOP_FINAL
    jmp HeapInsertGeneric

# Fixed-capacity signed min-heap. rcx mask, rdx score.
HeapInsertGeneric:
    push rbx
    push rsi
    mov eax, dword ptr [r10]
    cmp eax, r11d
    jb .append
    cmp rdx, qword ptr [r9]
    jle .heap_out
    mov qword ptr [r8], rcx
    mov qword ptr [r9], rdx
    xor eax, eax
.sift_down:
    lea ebx, [rax*2 + 1]
    cmp ebx, r11d
    jae .heap_out
    lea esi, [rbx + 1]
    cmp esi, r11d
    jae .left_chosen
    mov rdx, qword ptr [r9 + rbx*8]
    cmp rdx, qword ptr [r9 + rsi*8]
    jle .left_chosen
    mov ebx, esi
.left_chosen:
    mov rdx, qword ptr [r9 + rax*8]
    cmp rdx, qword ptr [r9 + rbx*8]
    jle .heap_out
    mov rcx, qword ptr [r8 + rax*8]
    mov rsi, qword ptr [r8 + rbx*8]
    mov qword ptr [r8 + rax*8], rsi
    mov qword ptr [r8 + rbx*8], rcx
    mov rcx, qword ptr [r9 + rax*8]
    mov rsi, qword ptr [r9 + rbx*8]
    mov qword ptr [r9 + rax*8], rsi
    mov qword ptr [r9 + rbx*8], rcx
    mov eax, ebx
    jmp .sift_down
.append:
    mov ebx, eax
    mov qword ptr [r8 + rbx*8], rcx
    mov qword ptr [r9 + rbx*8], rdx
    inc dword ptr [r10]
.sift_up:
    test ebx, ebx
    jz .heap_out
    lea eax, [rbx - 1]
    shr eax, 1
    mov rdx, qword ptr [r9 + rbx*8]
    cmp rdx, qword ptr [r9 + rax*8]
    jge .heap_out
    mov rcx, qword ptr [r8 + rbx*8]
    mov rsi, qword ptr [r8 + rax*8]
    mov qword ptr [r8 + rbx*8], rsi
    mov qword ptr [r8 + rax*8], rcx
    mov rcx, qword ptr [r9 + rbx*8]
    mov rsi, qword ptr [r9 + rax*8]
    mov qword ptr [r9 + rbx*8], rsi
    mov qword ptr [r9 + rax*8], rcx
    mov ebx, eax
    jmp .sift_up
.heap_out:
    pop rsi
    pop rbx
    ret

BuildPersistence:
    push rbx
    push rsi
    push rdi
    lea rdi, [rip + persistence]
    xor eax, eax
    mov ecx, 45
    rep stosd
    xor esi, esi
.bp_combo:
    cmp esi, dword ptr [rip + final_count]
    jae .bp_done
    lea rax, [rip + final_masks]
    mov rbx, qword ptr [rax + rsi*8]
.bp_bits:
    test rbx, rbx
    jz .bp_next
    bsf rcx, rbx
    btr rbx, rcx
    lea rax, [rip + persistence]
    inc dword ptr [rax + rcx*4]
    jmp .bp_bits
.bp_next:
    inc esi
    jmp .bp_combo
.bp_done:
    pop rdi
    pop rsi
    pop rbx
    ret

# If aggregate frequency deviations are weak, increase uniform regularization;
# stronger residual dispersion lowers it. Bounded to 12..55 percent.
ComputeSkepticism:
    push rbx
    xor rbx, rbx
    xor ecx, ecx
.cs_loop:
    lea rax, [rip + freq_total]
    mov eax, dword ptr [rax + rcx*4]
    imul eax, eax, 45
    mov edx, dword ptr [rip + draw_count]
    imul edx, edx, 6
    sub eax, edx
    cdq
    xor eax, edx
    sub eax, edx
    add rbx, rax
    inc ecx
    cmp ecx, 45
    jb .cs_loop
    mov eax, dword ptr [rip + draw_count]
    imul eax, eax, 8
    test eax, eax
    jz .cs_default
    mov rax, rbx
    xor edx, edx
    mov ecx, dword ptr [rip + draw_count]
    div rcx
    shr eax, 1
    mov edx, 55
    sub edx, eax
    cmp edx, 12
    jge 1f
    mov edx, 12
1:  cmp edx, 55
    jle 2f
    mov edx, 55
2:  mov dword ptr [rip + skepticism_pct], edx
    pop rbx
    ret
.cs_default:
    mov dword ptr [rip + skepticism_pct], 25
    pop rbx
    ret
