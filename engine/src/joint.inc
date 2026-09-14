# Fixed-cardinality exponential family. Empirical-Bayes Gaussian working
# posterior for successive count residuals; exact finite-space normalization.
# Not an exact multinomial posterior or a proven lottery advantage.
.equ JOINT_FEATURES,16260
.equ JOINT_BYTES,JOINT_FEATURES*8
.section .data
joint_universe: .long 45
joint_order: .long 3
joint_shrink: .double 1.0
.section .rdata,"dr"
joint_caps: .double 0.05,0.02,0.01
joint_c2: .long 0,0,1,3,6,10,15,21,28,36,45,55,66,78,91,105,120,136,153,171,190,210,231,253,276,300,325,351,378,406,435,465,496,528,561,595,630,666,703,741,780,820,861,903,946
joint_c3: .long 0,0,0,1,4,10,20,35,56,84,120,165,220,286,364,455,560,680,816,969,1140,1330,1540,1771,2024,2300,2600,2925,3276,3654,4060,4495,4960,5456,5984,6545,7140,7770,8436,9139,9880,10660,11480,12341,13244
joint_half_mix: .double 0.5
.section .bss
joint_coef: .skip JOINT_BYTES
joint_moment: .skip JOINT_BYTES
joint_variance: .skip JOINT_BYTES
joint_observed: .skip JOINT_FEATURES*4
joint_active: .skip JOINT_FEATURES*4
joint_zscores: .skip JOINT_BYTES
joint_indices: .skip 41*4
joint_level_weight: .skip 3*8
joint_logz: .skip 8
joint_z: .skip 8
joint_space: .skip 8
joint_enumerated: .skip 8
joint_start: .skip 4
joint_n: .skip 4
joint_marginal_error: .skip 8
.section .text
# Decode the 41 active sufficient-statistic indices. rcx is a valid six-mask.
FN JointIndices,32
    xor edx,edx
    lea r8,[rip+joint_indices]
1:  bsf rax,rcx
    btr rcx,rax
    mov [rsp+32+rdx*4],eax
    mov [r8+rdx*4],eax
    inc edx
    cmp edx,6
    jb 1b
    imul eax,dword ptr [rsp+32],45
    add eax,[rsp+36]
    add eax,45
    mov [r8+24],eax
    imul eax,dword ptr [rsp+32],45
    add eax,[rsp+40]
    add eax,45
    mov [r8+28],eax
    imul eax,dword ptr [rsp+32],45
    add eax,[rsp+44]
    add eax,45
    mov [r8+32],eax
    imul eax,dword ptr [rsp+32],45
    add eax,[rsp+48]
    add eax,45
    mov [r8+36],eax
    imul eax,dword ptr [rsp+32],45
    add eax,[rsp+52]
    add eax,45
    mov [r8+40],eax
    imul eax,dword ptr [rsp+36],45
    add eax,[rsp+40]
    add eax,45
    mov [r8+44],eax
    imul eax,dword ptr [rsp+36],45
    add eax,[rsp+44]
    add eax,45
    mov [r8+48],eax
    imul eax,dword ptr [rsp+36],45
    add eax,[rsp+48]
    add eax,45
    mov [r8+52],eax
    imul eax,dword ptr [rsp+36],45
    add eax,[rsp+52]
    add eax,45
    mov [r8+56],eax
    imul eax,dword ptr [rsp+40],45
    add eax,[rsp+44]
    add eax,45
    mov [r8+60],eax
    imul eax,dword ptr [rsp+40],45
    add eax,[rsp+48]
    add eax,45
    mov [r8+64],eax
    imul eax,dword ptr [rsp+40],45
    add eax,[rsp+52]
    add eax,45
    mov [r8+68],eax
    imul eax,dword ptr [rsp+44],45
    add eax,[rsp+48]
    add eax,45
    mov [r8+72],eax
    imul eax,dword ptr [rsp+44],45
    add eax,[rsp+52]
    add eax,45
    mov [r8+76],eax
    imul eax,dword ptr [rsp+48],45
    add eax,[rsp+52]
    add eax,45
    mov [r8+80],eax
    lea r9,[rip+joint_c2]
    lea r10,[rip+joint_c3]
    mov eax,[rsp+40]
    mov eax,[r10+rax*4]
    mov edx,[rsp+36]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+84],eax
    mov eax,[rsp+44]
    mov eax,[r10+rax*4]
    mov edx,[rsp+36]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+88],eax
    mov eax,[rsp+48]
    mov eax,[r10+rax*4]
    mov edx,[rsp+36]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+92],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+36]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+96],eax
    mov eax,[rsp+44]
    mov eax,[r10+rax*4]
    mov edx,[rsp+40]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+100],eax
    mov eax,[rsp+48]
    mov eax,[r10+rax*4]
    mov edx,[rsp+40]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+104],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+40]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+108],eax
    mov eax,[rsp+48]
    mov eax,[r10+rax*4]
    mov edx,[rsp+44]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+112],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+44]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+116],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+48]
    add eax,[r9+rdx*4]
    add eax,[rsp+32]
    add eax,2070
    mov [r8+120],eax
    mov eax,[rsp+44]
    mov eax,[r10+rax*4]
    mov edx,[rsp+40]
    add eax,[r9+rdx*4]
    add eax,[rsp+36]
    add eax,2070
    mov [r8+124],eax
    mov eax,[rsp+48]
    mov eax,[r10+rax*4]
    mov edx,[rsp+40]
    add eax,[r9+rdx*4]
    add eax,[rsp+36]
    add eax,2070
    mov [r8+128],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+40]
    add eax,[r9+rdx*4]
    add eax,[rsp+36]
    add eax,2070
    mov [r8+132],eax
    mov eax,[rsp+48]
    mov eax,[r10+rax*4]
    mov edx,[rsp+44]
    add eax,[r9+rdx*4]
    add eax,[rsp+36]
    add eax,2070
    mov [r8+136],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+44]
    add eax,[r9+rdx*4]
    add eax,[rsp+36]
    add eax,2070
    mov [r8+140],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+48]
    add eax,[r9+rdx*4]
    add eax,[rsp+36]
    add eax,2070
    mov [r8+144],eax
    mov eax,[rsp+48]
    mov eax,[r10+rax*4]
    mov edx,[rsp+44]
    add eax,[r9+rdx*4]
    add eax,[rsp+40]
    add eax,2070
    mov [r8+148],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+44]
    add eax,[r9+rdx*4]
    add eax,[rsp+40]
    add eax,2070
    mov [r8+152],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+48]
    add eax,[r9+rdx*4]
    add eax,[rsp+40]
    add eax,2070
    mov [r8+156],eax
    mov eax,[rsp+52]
    mov eax,[r10+rax*4]
    mov edx,[rsp+48]
    add eax,[r9+rdx*4]
    add eax,[rsp+44]
    add eax,2070
    mov [r8+160],eax
    END 32
FN JointEnergy
    call JointIndices
    lea rdx,[rip+joint_coef]
    lea r8,[rip+joint_indices]
    xorpd xmm0,xmm0
    xor ecx,ecx
1:  mov eax,[r8+rcx*4]
    addsd xmm0,[rdx+rax*8]
    inc ecx
    cmp ecx,41
    jb 1b
    END
# Accumulate Z and all moments over exactly C(universe,6) masks.
FN JointEnumerate,16
    lea rcx,[rip+joint_moment]
    mov edx,JOINT_BYTES
    call Zero
    mov qword ptr [rip+joint_z],0
    mov qword ptr [rip+joint_enumerated],0
    mov ebx,63
    mov r15,1
    mov ecx,[rip+joint_universe]
    shl r15,cl
.je_loop:
    cmp dword ptr [rip+cancel_flag],0
    jne .je_fail
    mov rcx,rbx
    call JointEnergy
    call Exp
    movsd xmm1,[rip+joint_z]
    addsd xmm1,xmm0
    movsd [rip+joint_z],xmm1
    lea rdx,[rip+joint_moment]
    lea r8,[rip+joint_indices]
    xor ecx,ecx
1:  mov eax,[r8+rcx*4]
    movsd xmm1,[rdx+rax*8]
    addsd xmm1,xmm0
    movsd [rdx+rax*8],xmm1
    inc ecx
    cmp ecx,41
    jb 1b
    inc qword ptr [rip+joint_enumerated]
    mov rax,rbx
    neg rax
    and rax,rbx
    lea rdx,[rax+rbx]
    bsf rcx,rax
    xor rbx,rdx
    shr rbx,2
    shr rbx,cl
    or rbx,rdx
    cmp rbx,r15
    jb .je_loop
    movsd xmm0,[rip+joint_z]
    call Log
    movsd [rip+joint_logz],xmm0
    mov rax,[rip+joint_enumerated]
    mov [rip+joint_space],rax
    lea rdx,[rip+joint_moment]
    xor ecx,ecx
    xorpd xmm2,xmm2
1:  movsd xmm0,[rdx+rcx*8]
    divsd xmm0,[rip+joint_z]
    movsd [rdx+rcx*8],xmm0
    cmp ecx,45
    jae 2f
    addsd xmm2,xmm0
2:  inc ecx
    cmp ecx,JOINT_FEATURES
    jb 1b
    subsd xmm2,[rip+six]
    andpd xmm2,[rip+abs_bits]
    movsd [rip+joint_marginal_error],xmm2
    mov eax,1
    jmp .je_out
.je_fail: xor eax,eax
.je_out: END 16
# Fit on history[start:cutoff], no future records. Returns false on cancellation.
# Mark only valid unordered features; Gaussian working variance is conditional
# on the lower-order model. Order-specific prior variance is estimated from
# mean standardized residual squared, with conservative family-size penalty.
FN JointFit,64
    mov [rip+joint_start],edx
    mov ebp,ecx
    sub ecx,edx
    mov [rip+joint_n],ecx
    cmp ecx,1
    jl .jf_fail
    lea rcx,[rip+joint_coef]
    mov edx,JOINT_BYTES
    call Zero
    lea rcx,[rip+joint_level_weight]
    mov edx,24
    call Zero
    lea rcx,[rip+joint_observed]
    mov edx,JOINT_FEATURES*4
    call Zero
    lea rcx,[rip+joint_active]
    mov edx,JOINT_FEATURES*4
    call Zero
    # An initial uniform exact enumeration supplies finite-population moments.
    call JointEnumerate
    test eax,eax
    jz .jf_fail
    xor esi,esi
1:  lea rax,[rip+joint_moment]
    movsd xmm0,[rax+rsi*8]
    comisd xmm0,[rip+zero]
    jbe 2f
    lea rax,[rip+joint_active]
    mov dword ptr [rax+rsi*4],1
2:  inc esi
    cmp esi,JOINT_FEATURES
    jb 1b
    mov esi,[rip+joint_start]
.jf_observe:
    imul eax,esi,RECORD_SIZE
    lea rdx,[rip+history]
    mov rcx,[rdx+rax+40]
    call JointIndices
    lea rdx,[rip+joint_observed]
    lea r8,[rip+joint_indices]
    xor ecx,ecx
1:  mov eax,[r8+rcx*4]
    inc dword ptr [rdx+rax*4]
    inc ecx
    cmp ecx,41
    jb 1b
    inc esi
    cmp esi,ebp
    jb .jf_observe
    xor r12d,r12d
    xor r13d,r13d
    mov r14d,45
.jf_level:
    cmp r12d,[rip+joint_order]
    jae .jf_done
    mov esi,r13d
    xor edi,edi
    mov qword ptr [rsp+32],0
.jf_residual:
    lea rax,[rip+joint_active]
    cmp dword ptr [rax+rsi*4],0
    je .jf_resnext
    lea rax,[rip+joint_moment]
    movsd xmm1,[rax+rsi*8]
    cvtsi2sd xmm2,dword ptr [rip+joint_n]
    mulsd xmm2,xmm1
    lea rax,[rip+joint_observed]
    cvtsi2sd xmm0,dword ptr [rax+rsi*4]
    subsd xmm0,xmm2
    movsd xmm3,[rip+one]
    subsd xmm3,xmm1
    mulsd xmm2,xmm3
    addsd xmm2,[rip+one]
    # v = 1/(Np(1-p)+1), z = count residual / sqrt(Np(1-p)+1).
    movsd xmm3,[rip+one]
    divsd xmm3,xmm2
    lea rax,[rip+joint_variance]
    movsd [rax+rsi*8],xmm3
    sqrtsd xmm2,xmm2
    divsd xmm0,xmm2
    lea rax,[rip+joint_zscores]
    movsd [rax+rsi*8],xmm0
    mulsd xmm0,xmm0
    addsd xmm0,[rsp+32]
    movsd [rsp+32],xmm0
    inc edi
.jf_resnext:
    inc esi
    cmp esi,r14d
    jb .jf_residual
    cvtsi2sd xmm0,edi
    movsd [rsp+40],xmm0
    call Log
    mulsd xmm0,[rip+two]
    sqrtsd xmm0,xmm0
    movsd xmm1,[rsp+32]
    divsd xmm1,[rsp+40]
    subsd xmm1,[rip+one]
    maxsd xmm1,[rip+zero]
    # Empirical prior / noise, reduced by the prespecified shrink setting.
    movapd xmm2,xmm1
    addsd xmm2,[rip+joint_shrink]
    divsd xmm1,xmm2
    lea rax,[rip+joint_level_weight]
    movsd [rax+r12*8],xmm1
    movsd [rsp+48],xmm1
    movsd [rsp+56],xmm0
    mov esi,r13d
.jf_coeff:
    lea rax,[rip+joint_active]
    cmp dword ptr [rax+rsi*4],0
    je .jf_coefnext
    lea rax,[rip+joint_zscores]
    movsd xmm0,[rax+rsi*8]
    movapd xmm4,xmm0
    andpd xmm0,[rip+abs_bits]
    subsd xmm0,[rsp+56]
    maxsd xmm0,[rip+zero]
    lea rax,[rip+joint_variance]
    movsd xmm1,[rax+rsi*8]
    sqrtsd xmm1,xmm1
    mulsd xmm0,xmm1
    mulsd xmm0,[rsp+48]
    lea rax,[rip+joint_caps]
    minsd xmm0,[rax+r12*8]
    andpd xmm4,[rip+sign_bits]
    xorpd xmm0,xmm4
    lea rax,[rip+joint_coef]
    movsd [rax+rsi*8],xmm0
.jf_coefnext:
    inc esi
    cmp esi,r14d
    jb .jf_coeff
    call JointEnumerate
    test eax,eax
    jz .jf_fail
    inc r12d
    mov r13d,r14d
    mov r14d,2070
    cmp r12d,1
    je .jf_level
    mov r14d,JOINT_FEATURES
    cmp r12d,3
    jb .jf_level
.jf_done: mov eax,1; jmp .jf_out
.jf_fail: xor eax,eax
.jf_out: END 64
FN JointLogProbability
    call JointEnergy
    subsd xmm0,[rip+joint_logz]
    END
# Conservative full-support joint forecast: 50% uniform, 50% joint model.
FN JointForecastLog
    call JointLogProbability
    call Exp
    mulsd xmm0,[rip+half]
    cvtsi2sd xmm1,qword ptr [rip+joint_space]
    movsd xmm2,[rip+half]
    divsd xmm2,xmm1
    addsd xmm0,xmm2
    call Log
    END

.section .rdata,"dr"
joint_energy_bound: .double 0.8
.section .text
# Exact rejection sampler; coefficient caps imply |energy| <= .8.
FN JointSample
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .js_fail
    comisd xmm0,[rip+half]
    jb .js_uniform
    mov ebp,8192
.js_try:
    call GenerateUniformMask
    test rax,rax
    jz .js_fail
    mov rbx,rax
    mov rcx,rax
    call JointEnergy
    subsd xmm0,[rip+joint_energy_bound]
    call Exp
    # FN preserves integer registers but not volatile XMM registers.
    movq r12,xmm0
    call RandomUnit
    cmp dword ptr [rip+rng_error],0
    jne .js_fail
    movq xmm1,r12
    comisd xmm0,xmm1
    jb .js_accept
    dec ebp
    jnz .js_try
    jmp .js_fail
.js_accept: mov rax,rbx; jmp .js_out
.js_uniform: call GenerateUniformMask; jmp .js_out
.js_fail: xor eax,eax
.js_out: END
