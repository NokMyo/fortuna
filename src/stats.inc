# Eight-dimensional structural coordinates and five exact entropy partitions.
# ExtractFeatures(mask) uses private analysis scratch, not callable concurrently.
FN ExtractFeatures
    mov rbx,rcx
    lea rdx,[rip+extract_buf]
    call MaskToNumbers
    lea rcx,[rip+feature_buf]
    mov edx,FEATURE_COUNT*8
    call Zero
    lea rcx,[rip+feature_buckets]
    mov edx,50*4
    call Zero
    xor esi,esi
    xor r12d,r12d
    xor r13d,r13d
    xor r14d,r14d
    xor r15d,r15d
    lea rdi,[rip+extract_buf]
.ef_num:
    mov eax,[rdi+rsi*4]
    add r12d,eax
    test eax,1
    jz 1f
    inc r13d
1:  cmp eax,22
    ja 2f
    inc r14d
2:  test esi,esi
    jz 3f
    mov ecx,eax
    sub ecx,[rdi+rsi*4-4]
    cmp ecx,1
    jne 3f
    inc r15d
3:  inc esi
    cmp esi,6
    jb .ef_num
    lea rbx,[rip+feature_buf]
    cvtsi2sd xmm0,r12d
    movsd [rbx],xmm0
    mov eax,[rdi+20]
    sub eax,[rdi]
    cvtsi2sd xmm0,eax
    movsd [rbx+8],xmm0
    cvtsi2sd xmm0,r13d
    movsd [rbx+16],xmm0
    cvtsi2sd xmm0,r14d
    movsd [rbx+24],xmm0
    cvtsi2sd xmm0,r15d
    movsd [rbx+32],xmm0
    xor esi,esi
    xor ebp,ebp
.ef_partition:
    xor r12d,r12d
    mov r13d,2
    cmp esi,0
    je 1f
    mov r13d,3
    cmp esi,1
    je 1f
    mov r13d,5
    cmp esi,2
    je 1f
    mov r13d,9
    cmp esi,3
    je 1f
    mov r13d,10
1:  imul eax,esi,40
    lea r14,[rip+feature_buckets]
    add r14,rax
.ef_buckets:
    mov eax,[rdi+r12*4]
    cmp esi,4
    je 2f
    dec eax
    imul eax,r13d
    xor edx,edx
    mov ecx,45
    div ecx
    jmp 3f
2:  xor edx,edx
    mov ecx,10
    div ecx
    mov eax,edx
3:  inc dword ptr [r14+rax*4]
    inc r12d
    cmp r12d,6
    jb .ef_buckets
    xorpd xmm0,xmm0
    xor r12d,r12d
    lea r15,[rip+entropy_lut]
.ef_entropy:
    mov eax,[r14+r12*4]
    addsd xmm0,[r15+rax*8]
    cmp esi,4
    jne 4f
    mov ecx,eax
    dec ecx
    imul ecx,eax
    shr ecx,1
    add ebp,ecx
4:  inc r12d
    cmp r12d,r13d
    jb .ef_entropy
    lea rax,[rip+entropy_buf]
    movsd [rax+rsi*8],xmm0
    inc esi
    cmp esi,5
    jb .ef_partition
    cvtsi2sd xmm0,ebp
    movsd [rbx+48],xmm0
    movsd xmm0,[rip+entropy_buf+16]
    movsd [rbx+56],xmm0
    xor esi,esi
    xor ebp,ebp
    mov rdx,[rip+prime_bits]
1:  mov eax,[rdi+rsi*4]
    dec eax
    bt rdx,rax
    adc ebp,0
    inc esi
    cmp esi,6
    jb 1b
    cvtsi2sd xmm0,ebp
    movsd [rbx+40],xmm0
    END

FN ResetStats
    lea rcx,[rip+stats_begin]
    mov edx,stats_end-stats_begin
    call Zero
    END
FN RebuildStats
    mov ebp,ecx
    cmp ecx,[rip+history_count]
    ja 3f
    call ResetStats
    xor esi,esi
    lea rdi,[rip+history]
1:  cmp esi,ebp
    jae 2f
    mov rcx,rdi
    call ProcessRecord
    add rdi,RECORD_SIZE
    inc esi
    jmp 1b
2:  mov eax,1
    jmp 4f
3:  xor eax,eax
4:  END

FN ProcessRecord,32
    mov rbx,rcx
    mov rbp,[rbx+40]
    inc dword ptr [rip+draw_count]
    mov r15d,[rip+draw_count]
    xor esi,esi
.pr_ewma:
    lea rax,[rip+decays]
    movsd xmm0,[rax+rsi*8]
    lea rax,[rip+ewma_mass]
    movsd xmm1,[rax+rsi*8]
    mulsd xmm1,xmm0
    addsd xmm1,[rip+one]
    movsd [rax+rsi*8],xmm1
    imul eax,esi,45*8
    lea rdi,[rip+freq_ewma]
    add rdi,rax
    xor ecx,ecx
1:  movsd xmm1,[rdi+rcx*8]
    mulsd xmm1,xmm0
    bt rbp,rcx
    jnc 2f
    addsd xmm1,[rip+one]
2:  movsd [rdi+rcx*8],xmm1
    inc ecx
    cmp ecx,45
    jb 1b
    inc esi
    cmp esi,7
    jb .pr_ewma
    xor esi,esi
.pr_hazard:
    lea rdi,[rip+last_seen]
    mov eax,[rdi+rsi*4]
    test eax,eax
    jz .pr_seen
    mov ecx,r15d
    sub ecx,eax
    dec ecx
    cmp ecx,63
    jbe 1f
    mov ecx,63
1:  mov eax,esi
    shl eax,6
    add eax,ecx
    lea rdx,[rip+hazard_risk]
    inc dword ptr [rdx+rax*4]
    bt rbp,rsi
    jnc .pr_seen
    lea rdx,[rip+hazard_events]
    inc dword ptr [rdx+rax*4]
.pr_seen:
    bt rbp,rsi
    jnc 2f
    mov [rdi+rsi*4],r15d
    lea rax,[rip+freq_total]
    inc dword ptr [rax+rsi*4]
2:  inc esi
    cmp esi,45
    jb .pr_hazard
    lea rdi,[rip+pair_decay]
    xor ecx,ecx
    movsd xmm0,[rip+decays+32]
1:  movsd xmm1,[rdi+rcx*8]
    mulsd xmm1,xmm0
    movsd [rdi+rcx*8],xmm1
    inc ecx
    cmp ecx,45*45
    jb 1b
    xor esi,esi
.pr_pairs_i:
    mov r12d,[rbx+rsi*4+8]
    dec r12d
    lea edi,[rsi+1]
.pr_pairs_j:
    mov r13d,[rbx+rdi*4+8]
    dec r13d
    imul eax,r12d,45
    add eax,r13d
    lea rdx,[rip+pair_counts]
    inc dword ptr [rdx+rax*4]
    lea rdx,[rip+pair_decay]
    movsd xmm0,[rdx+rax*8]
    addsd xmm0,[rip+one]
    movsd [rdx+rax*8],xmm0
    imul ecx,r13d,45
    add ecx,r12d
    movsd [rdx+rcx*8],xmm0
    lea rdx,[rip+pair_counts]
    mov eax,[rdx+rax*4]
    mov [rdx+rcx*4],eax
    lea r14d,[rdi+1]
.pr_triple:
    cmp r14d,6
    jae .pr_next_pair
    mov r8d,[rbx+r14*4+8]
    dec r8d
    mov ecx,r12d
    mov edx,r13d
    call TripleIndex
    lea rdx,[rip+triple_counts]
    inc dword ptr [rdx+rax*4]
    inc r14d
    jmp .pr_triple
.pr_next_pair:
    inc edi
    cmp edi,6
    jb .pr_pairs_j
    inc esi
    cmp esi,5
    jb .pr_pairs_i
    mov rcx,rbp
    call ExtractFeatures
    cvtsi2sd xmm5,r15d
    lea rbx,[rip+feature_buf]
    lea rdi,[rip+feature_mean]
    lea r12,[rip+feature_delta]
    xor esi,esi
1:  movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rdi+rsi*8]
    movsd [r12+rsi*8],xmm0
    divsd xmm0,xmm5
    addsd xmm0,[rdi+rsi*8]
    movsd [rdi+rsi*8],xmm0
    inc esi
    cmp esi,FEATURE_COUNT
    jb 1b
    xor esi,esi
    lea r13,[rip+feature_m2]
.pr_cov_i:
    xor ecx,ecx
1:  movsd xmm0,[rbx+rcx*8]
    subsd xmm0,[rdi+rcx*8]
    mulsd xmm0,[r12+rsi*8]
    mov eax,esi
    imul eax,eax,FEATURE_COUNT
    add eax,ecx
    addsd xmm0,[r13+rax*8]
    movsd [r13+rax*8],xmm0
    inc ecx
    cmp ecx,FEATURE_COUNT
    jb 1b
    inc esi
    cmp esi,FEATURE_COUNT
    jb .pr_cov_i
    lea rbx,[rip+entropy_buf]
    lea rdi,[rip+entropy_mean]
    lea r12,[rip+entropy_m2]
    xor esi,esi
1:  movsd xmm0,[rbx+rsi*8]
    subsd xmm0,[rdi+rsi*8]
    movapd xmm1,xmm0
    divsd xmm1,xmm5
    addsd xmm1,[rdi+rsi*8]
    movsd [rdi+rsi*8],xmm1
    movsd xmm2,[rbx+rsi*8]
    subsd xmm2,xmm1
    mulsd xmm0,xmm2
    addsd xmm0,[r12+rsi*8]
    movsd [r12+rsi*8],xmm0
    inc esi
    cmp esi,5
    jb 1b
    END 32
