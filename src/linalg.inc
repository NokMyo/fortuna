# Gauss-Jordan inverse with partial pivoting. Input not modified.
# rcx matrix, edx n<=45, r8 destination. Returns 0 for singular/non-finite input.
FN MatrixInverse,32
    mov rbx,rcx
    mov ebp,edx
    mov r15,r8
    lea rdi,[rip+matrix_work]
    lea r14d,[rbp*2]
    xor esi,esi
.mi_rows:
    xor r12d,r12d
1:  mov eax,esi
    imul eax,ebp
    add eax,r12d
    movsd xmm0,[rbx+rax*8]
    ucomisd xmm0,xmm0
    jp .mi_fail
    mov eax,esi
    imul eax,r14d
    add eax,r12d
    movsd [rdi+rax*8],xmm0
    add eax,ebp
    xorpd xmm0,xmm0
    cmp esi,r12d
    jne 2f
    movsd xmm0,[rip+one]
2:  movsd [rdi+rax*8],xmm0
    inc r12d
    cmp r12d,ebp
    jb 1b
    inc esi
    cmp esi,ebp
    jb .mi_rows
    xor esi,esi
.mi_pivot:
    mov r12d,esi
    mov r13d,esi
    xorpd xmm0,xmm0
1:  mov eax,r12d
    imul eax,r14d
    add eax,esi
    movsd xmm1,[rdi+rax*8]
    andpd xmm1,[rip+abs_bits]
    comisd xmm1,xmm0
    jbe 2f
    movapd xmm0,xmm1
    mov r13d,r12d
2:  inc r12d
    cmp r12d,ebp
    jb 1b
    comisd xmm0,[rip+epsilon]
    jb .mi_fail
    cmp r13d,esi
    je .mi_normalize
    mov eax,esi
    imul eax,r14d
    mov edx,r13d
    imul edx,r14d
    xor ecx,ecx
1:  movsd xmm0,[rdi+rax*8]
    movsd xmm1,[rdi+rdx*8]
    movsd [rdi+rax*8],xmm1
    movsd [rdi+rdx*8],xmm0
    inc eax
    inc edx
    inc ecx
    cmp ecx,r14d
    jb 1b
.mi_normalize:
    mov eax,esi
    imul eax,r14d
    lea edx,[rax+rsi]
    movsd xmm0,[rdi+rdx*8]
    xor ecx,ecx
1:  movsd xmm1,[rdi+rax*8]
    divsd xmm1,xmm0
    movsd [rdi+rax*8],xmm1
    inc eax
    inc ecx
    cmp ecx,r14d
    jb 1b
    xor r12d,r12d
.mi_eliminate:
    cmp r12d,esi
    je .mi_nextrow
    mov eax,r12d
    imul eax,r14d
    lea edx,[rax+rsi]
    movsd xmm0,[rdi+rdx*8]
    mov edx,esi
    imul edx,r14d
    xor ecx,ecx
1:  movsd xmm1,[rdi+rdx*8]
    mulsd xmm1,xmm0
    movsd xmm2,[rdi+rax*8]
    subsd xmm2,xmm1
    movsd [rdi+rax*8],xmm2
    inc eax
    inc edx
    inc ecx
    cmp ecx,r14d
    jb 1b
.mi_nextrow:
    inc r12d
    cmp r12d,ebp
    jb .mi_eliminate
    inc esi
    cmp esi,ebp
    jb .mi_pivot
    xor esi,esi
.mi_copy:
    xor ecx,ecx
1:  mov eax,esi
    imul eax,r14d
    add eax,ebp
    add eax,ecx
    movsd xmm0,[rdi+rax*8]
    mov eax,esi
    imul eax,ebp
    add eax,ecx
    movsd [r15+rax*8],xmm0
    inc ecx
    cmp ecx,ebp
    jb 1b
    inc esi
    cmp esi,ebp
    jb .mi_copy
    mov eax,1
    jmp .mi_out
.mi_fail: xor eax,eax
.mi_out: END 32

# Cyclic symmetric Jacobi, 24 sweeps. A is copied from null-standardized pair
# graph. U starts at identity. All accumulations are double precision.
FN SpectralLattice,48
    lea rbx,[rip+spectral_a]
    lea rdi,[rip+spectral_u]
    mov rcx,rdi
    mov edx,45*45*8
    call Zero
    xor eax,eax
1:  imul edx,eax,46
    movsd xmm0,[rip+one]
    movsd [rdi+rdx*8],xmm0
    inc eax
    cmp eax,45
    jb 1b
    mov ebp,24
.sl_sweep:
    xor esi,esi
.sl_p:
    lea r12d,[rsi+1]
.sl_q:
    imul eax,esi,45
    add eax,r12d
    movsd xmm0,[rbx+rax*8]
    movapd xmm1,xmm0
    andpd xmm1,[rip+abs_bits]
    comisd xmm1,[rip+epsilon]
    jb .sl_nextq
    imul r13d,esi,46
    imul r14d,r12d,46
    movsd xmm1,[rbx+r14*8]
    subsd xmm1,[rbx+r13*8]
    addsd xmm0,xmm0
    divsd xmm1,xmm0
    # t=sign(tau)/(abs(tau)+sqrt(1+tau^2)); tau=0 chooses +1.
    movapd xmm2,xmm1
    mulsd xmm2,xmm2
    addsd xmm2,[rip+one]
    sqrtsd xmm2,xmm2
    movapd xmm3,xmm1
    andpd xmm3,[rip+abs_bits]
    addsd xmm2,xmm3
    movsd xmm3,[rip+one]
    divsd xmm3,xmm2
    comisd xmm1,[rip+zero]
    jae 1f
    xorpd xmm3,[rip+sign_bits]
1:  movapd xmm2,xmm3
    mulsd xmm2,xmm2
    addsd xmm2,[rip+one]
    sqrtsd xmm2,xmm2
    movsd xmm4,[rip+one]
    divsd xmm4,xmm2
    movapd xmm5,xmm3
    mulsd xmm5,xmm4
    movsd [rsp+32],xmm4
    movsd [rsp+40],xmm5
    # diagonal updates and exact symmetric zero.
    movsd xmm0,[rbx+rax*8]
    mulsd xmm0,xmm3
    movsd xmm1,[rbx+r13*8]
    subsd xmm1,xmm0
    movsd [rbx+r13*8],xmm1
    addsd xmm0,[rbx+r14*8]
    movsd [rbx+r14*8],xmm0
    mov qword ptr [rbx+rax*8],0
    imul eax,r12d,45
    add eax,esi
    mov qword ptr [rbx+rax*8],0
    xor r15d,r15d
.sl_rotate:
    imul eax,r15d,45
    lea ecx,[rax+rsi]
    lea edx,[rax+r12]
    cmp r15d,esi
    je .sl_vectors
    cmp r15d,r12d
    je .sl_vectors
    movsd xmm0,[rbx+rcx*8]
    movsd xmm1,[rbx+rdx*8]
    movapd xmm2,xmm0
    mulsd xmm2,xmm4
    movapd xmm3,xmm1
    mulsd xmm3,xmm5
    subsd xmm2,xmm3
    mulsd xmm0,xmm5
    mulsd xmm1,xmm4
    addsd xmm0,xmm1
    movsd [rbx+rcx*8],xmm2
    movsd [rbx+rdx*8],xmm0
    imul eax,esi,45
    add eax,r15d
    movsd [rbx+rax*8],xmm2
    imul eax,r12d,45
    add eax,r15d
    movsd [rbx+rax*8],xmm0
.sl_vectors:
    movsd xmm0,[rdi+rcx*8]
    movsd xmm1,[rdi+rdx*8]
    movapd xmm2,xmm0
    mulsd xmm2,xmm4
    movapd xmm3,xmm1
    mulsd xmm3,xmm5
    subsd xmm2,xmm3
    mulsd xmm0,xmm5
    mulsd xmm1,xmm4
    addsd xmm0,xmm1
    movsd [rdi+rcx*8],xmm2
    movsd [rdi+rdx*8],xmm0
    inc r15d
    cmp r15d,45
    jb .sl_rotate
.sl_nextq:
    inc r12d
    cmp r12d,45
    jb .sl_q
    inc esi
    cmp esi,44
    jb .sl_p
    dec ebp
    jnz .sl_sweep
    # L_ij = sum_k U_ik [lambda/(1+.1*abs(lambda))] U_jk.
    xor esi,esi
    lea r15,[rip+spectral_l]
.sl_li:
    xor r12d,r12d
.sl_lj:
    xorpd xmm5,xmm5
    xor r13d,r13d
1:  imul eax,r13d,46
    movsd xmm0,[rbx+rax*8]
    movapd xmm1,xmm0
    andpd xmm1,[rip+abs_bits]
    mulsd xmm1,[rip+ridge]
    addsd xmm1,[rip+one]
    divsd xmm0,xmm1
    imul eax,esi,45
    add eax,r13d
    mulsd xmm0,[rdi+rax*8]
    imul eax,r12d,45
    add eax,r13d
    mulsd xmm0,[rdi+rax*8]
    addsd xmm5,xmm0
    inc r13d
    cmp r13d,45
    jb 1b
    imul eax,esi,45
    add eax,r12d
    movsd [r15+rax*8],xmm5
    inc r12d
    cmp r12d,45
    jb .sl_lj
    inc esi
    cmp esi,45
    jb .sl_li
    END 48
