.section .bss
world_prefix_hash: .skip 32
swap_values: .skip 6*39*8
world_temp: .skip TOP_FINAL*8
world_medians: .skip WORLDS*8
base_order: .skip TOP_FINAL*4
world_cutoff: .skip 4
.section .text
# Full 234-neighbour geometry. Output [OBG,FBV,mean FSS,neighbour variance].
# Coordinates are in normalized-score units (not integer Q10).
FN GeometryOne,80
    mov rbx,rcx
    mov rbp,rdx
    call ScoreDeep
    cvtsi2sd xmm0,rax
    divsd xmm0,[rip+scale_q]
    movsd [rsp+32],xmm0
    mov qword ptr [rsp+40],0
    mov qword ptr [rsp+48],0
    mov qword ptr [rsp+56],0
    mov qword ptr [rsp+64],0
    xor r12d,r12d
    xor r13d,r13d
    mov r14,rbx
.go_remove:
    bsf rsi,r14
    btr r14,rsi
    xor edi,edi
    xor r15d,r15d
.go_add:
    bt rbx,rdi
    jc .go_next
    mov rcx,rbx
    btr rcx,rsi
    bts rcx,rdi
    call ScoreDeep
    cvtsi2sd xmm0,rax
    divsd xmm0,[rip+scale_q]
    inc r12d
    # Welford mean/M2.
    movapd xmm1,xmm0
    subsd xmm1,[rsp+40]
    cvtsi2sd xmm2,r12d
    movapd xmm3,xmm1
    divsd xmm3,xmm2
    addsd xmm3,[rsp+40]
    movsd [rsp+40],xmm3
    movapd xmm2,xmm0
    subsd xmm2,xmm3
    mulsd xmm1,xmm2
    addsd xmm1,[rsp+48]
    movsd [rsp+48],xmm1
    movsd xmm1,[rsp+32]
    subsd xmm1,xmm0
    imul eax,r13d,39
    add eax,r15d
    lea rdx,[rip+swap_values]
    movsd [rdx+rax*8],xmm1
    inc r15d
    # Stable FBV log-sum-exp with dynamic running max.
    subsd xmm0,[rsp+32]
    cmp r12d,1
    jne 1f
    movsd [rsp+56],xmm0
    movsd xmm1,[rip+one]
    movsd [rsp+64],xmm1
    jmp .go_next
1:  comisd xmm0,[rsp+56]
    jbe 2f
    movsd [rsp+72],xmm0
    movsd xmm0,[rsp+56]
    subsd xmm0,[rsp+72]
    call Exp
    mulsd xmm0,[rsp+64]
    addsd xmm0,[rip+one]
    movsd [rsp+64],xmm0
    movsd xmm0,[rsp+72]
    movsd [rsp+56],xmm0
    jmp .go_next
2:  subsd xmm0,[rsp+56]
    call Exp
    addsd xmm0,[rsp+64]
    movsd [rsp+64],xmm0
.go_next:
    inc edi
    cmp edi,45
    jb .go_add
    inc r13d
    cmp r13d,6
    jb .go_remove
    movsd xmm0,[rsp+48]
    mov eax,234
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    movsd [rbp+24],xmm0
    sqrtsd xmm0,xmm0
    movsd xmm1,[rsp+40]
    subsd xmm1,xmm0
    movsd xmm2,[rsp+32]
    subsd xmm2,[rsp+40]
    subsd xmm2,[rip+one]
    maxsd xmm2,[rip+zero]
    mulsd xmm2,[rip+half]
    subsd xmm1,xmm2
    movsd [rbp],xmm1
    movsd xmm0,[rsp+64]
    call Log
    addsd xmm0,[rsp+56]
    movsd [rbp+8],xmm0
    xor esi,esi
    mov qword ptr [rsp+80],0
.go_median:
    imul eax,esi,39*8
    lea rcx,[rip+swap_values]
    add rcx,rax
    mov edx,39
    call Median
    lea rax,[rip+swap_sensitivity]
    movsd [rax+rsi*8],xmm0
    addsd xmm0,[rsp+80]
    movsd [rsp+80],xmm0
    inc esi
    cmp esi,6
    jb .go_median
    divsd xmm0,[rip+six]
    movsd [rbp+16],xmm0
    END 80

FN BuildGeometry
    mov dword ptr [rip+analysis_stage],4
    xor esi,esi
1:  cmp esi,[rip+field_count]
    jae 2f
    cmp dword ptr [rip+cancel_flag],0
    jne 3f
    lea rax,[rip+final_masks]
    mov rcx,[rax+rsi*8]
    imul eax,esi,32
    lea rdx,[rip+geometry]
    add rdx,rax
    call GeometryOne
    inc esi
    mov eax,esi
    imul eax,eax,100
    xor edx,edx
    div dword ptr [rip+field_count]
    mov [rip+progress],eax
    jmp 1b
2:  mov eax,1
    jmp 4f
3:  xor eax,eax
4:  END

# Counterfactual worlds: 8 each ordinary bootstrap, moving blocks of length 8,
# random deletion (10%), and latest 5/10/15/20 draw deletion. These re-fit all
# base models but rank only the fixed 512 survivors; ranks are conditional.
FN BuildWorlds,32
    mov dword ptr [rip+analysis_stage],5
    mov eax,[rip+draw_count]
    mov [rip+world_cutoff],eax
    lea rcx,[rip+stats_backup]
    lea rdx,[rip+stats_begin]
    mov r8d,STATS_BYTES
    call Copy
    lea rcx,[rip+model_backup]
    lea rdx,[rip+number_models]
    mov r8d,MODEL_BYTES
    call Copy
    mov rax,[rip+rng_state]
    mov [rsp+32],rax
    mov eax,[rip+rng_mode]
    mov [rsp+40],eax
    mov dword ptr [rip+rng_mode],1
    lea rcx,[rip+history]
    imul edx,dword ptr [rip+world_cutoff],RECORD_SIZE
    lea r8,[rip+world_prefix_hash]
    call Sha256
    mov rax,[rip+world_prefix_hash]
    xor rax,20260913
    mov [rip+rng_state],rax
    xor ebp,ebp
.bw_world:
    cmp dword ptr [rip+cancel_flag],0
    jne .bw_restore
    call ResetStats
    mov r15d,ebp
    and r15d,3
    xor esi,esi
    mov r12d,[rip+world_cutoff]
    cmp r15d,3
    jne .bw_rows
    mov eax,ebp
    shr eax,2
    and eax,3
    inc eax
    imul eax,eax,5
    sub r12d,eax
    cmp r12d,30
    jge .bw_rows
    mov r12d,30
.bw_rows:
    cmp esi,r12d
    jae .bw_prepare
    cmp r15d,0
    je .bw_random
    cmp r15d,1
    je .bw_block
    cmp r15d,2
    jne .bw_sequential
    mov ecx,10
    call RandomBelow
    test eax,eax
    jz .bw_row_next
.bw_sequential:
    mov eax,esi
    jmp .bw_process
.bw_random:
    mov ecx,[rip+world_cutoff]
    call RandomBelow
    jmp .bw_process
.bw_block:
    test esi,7
    jnz 1f
    mov ecx,[rip+world_cutoff]
    call RandomBelow
    mov r13d,eax
1:  mov eax,r13d
    inc r13d
    cmp r13d,[rip+world_cutoff]
    jb .bw_process
    xor r13d,r13d
.bw_process:
    imul eax,eax,RECORD_SIZE
    lea rcx,[rip+history]
    add rcx,rax
    call ProcessRecord
.bw_row_next:
    inc esi
    jmp .bw_rows
.bw_prepare:
    call PrepareModels
    test eax,eax
    jz .bw_restore
    xor esi,esi
.bw_scores:
    lea rax,[rip+final_masks]
    mov rcx,[rax+rsi*8]
    call ScoreDeep
    cvtsi2sd xmm0,rax
    divsd xmm0,[rip+scale_q]
    mov eax,ebp
    imul eax,eax,TOP_FINAL
    add eax,esi
    lea rdx,[rip+world_scores]
    movsd [rdx+rax*8],xmm0
    inc esi
    cmp esi,[rip+field_count]
    jb .bw_scores
    # Exact conditional rank with deterministic mask tie break.
    xor esi,esi
.bw_ranks:
    mov edi,1
    xor r12d,r12d
    mov eax,ebp
    imul eax,eax,TOP_FINAL
    lea rdx,[rip+world_scores]
    lea rdx,[rdx+rax*8]
    movsd xmm0,[rdx+rsi*8]
    lea r8,[rip+final_masks]
1:  cmp r12d,esi
    je 3f
    comisd xmm0,[rdx+r12*8]
    jb 2f
    ja 3f
    mov rax,[r8+r12*8]
    cmp rax,[r8+rsi*8]
    jae 3f
2:  inc edi
3:  inc r12d
    cmp r12d,[rip+field_count]
    jb 1b
    mov eax,ebp
    imul eax,eax,TOP_FINAL
    add eax,esi
    lea rdx,[rip+world_ranks]
    mov [rdx+rax*4],edi
    inc esi
    cmp esi,[rip+field_count]
    jb .bw_ranks
    inc ebp
    mov eax,ebp
    imul eax,eax,100
    shr eax,5
    mov [rip+progress],eax
    cmp ebp,WORLDS
    jb .bw_world
.bw_restore:
    lea rcx,[rip+stats_begin]
    lea rdx,[rip+stats_backup]
    mov r8d,STATS_BYTES
    call Copy
    lea rcx,[rip+number_models]
    lea rdx,[rip+model_backup]
    mov r8d,MODEL_BYTES
    call Copy
    mov rax,[rsp+32]
    mov [rip+rng_state],rax
    mov eax,[rsp+40]
    mov [rip+rng_mode],eax
    cmp ebp,WORLDS
    jne .bw_fail
    xor esi,esi
.bw_summary:
    xor edi,edi
    xor r12d,r12d
    xor r13d,r13d
    lea r14,[rip+world_medians]
1:  imul eax,edi,TOP_FINAL
    add eax,esi
    lea rdx,[rip+world_ranks]
    mov eax,[rdx+rax*4]
    cvtsi2sd xmm0,eax
    movsd [r14+rdi*8],xmm0
    cmp eax,10
    ja 2f
    inc r12d
2:  cmp eax,100
    ja 3f
    inc r13d
3:  inc edi
    cmp edi,WORLDS
    jb 1b
    mov rcx,r14
    mov edx,WORLDS
    call Median
    movsd [rsp+48],xmm0
    imul eax,esi,32
    lea r15,[rip+world_summary]
    add r15,rax
    movsd [r15+24],xmm0
    xor edi,edi
1:  movsd xmm1,[r14+rdi*8]
    subsd xmm1,xmm0
    andpd xmm1,[rip+abs_bits]
    movsd [r14+rdi*8],xmm1
    inc edi
    cmp edi,WORLDS
    jb 1b
    mov rcx,r14
    mov edx,WORLDS
    call Median
    mov eax,100
    cvtsi2sd xmm1,eax
    divsd xmm0,xmm1
    xorpd xmm0,[rip+sign_bits]
    call Exp
    movsd [r15],xmm0
    mov eax,WORLDS
    cvtsi2sd xmm1,eax
    cvtsi2sd xmm0,r12d
    divsd xmm0,xmm1
    movsd [r15+8],xmm0
    cvtsi2sd xmm0,r13d
    divsd xmm0,xmm1
    movsd [r15+16],xmm0
    inc esi
    cmp esi,[rip+field_count]
    jb .bw_summary
    mov eax,1
    jmp .bw_out
.bw_fail: xor eax,eax
.bw_out: END 32
