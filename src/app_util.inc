
.section .text
FN Zero
    mov rdi,rcx
    mov rcx,rdx
    xor eax,eax
    rep stosb
    END
FN Copy
    mov rdi,rcx
    mov rsi,rdx
    mov rcx,r8
    rep movsb
    END

FN MaskToNumbers
    mov rbx,rcx
    mov rsi,rdx
    mov rax,rcx
    shr rax,45
    jnz 3f
    call PopCount64
    cmp eax,6
    jne 3f
    xor edi,edi
1:  bsf rax,rbx
    btr rbx,rax
    inc eax
    mov [rsi+rdi*4],eax
    inc edi
    cmp edi,6
    jb 1b
    mov eax,1
    jmp 4f
3:  xor eax,eax
4:  END

# Choose(n,k), n<=45, k<=6. Exact integer binomial.
FN PopCount64
    xor eax,eax
1:  test rcx,rcx
    jz 2f
    lea rdx,[rcx-1]
    and rcx,rdx
    inc eax
    jmp 1b
2:  END

# MaskToNumbers(mask,out) -> 1 on exactly six valid bits, otherwise 0.
