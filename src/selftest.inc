.section .rdata,"dr"
sha_test_input: .ascii "abc"
sha_test_expected: .byte 0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad
bad_csv: .ascii "1,1,2,3,4,5,5\n"
bad_csv_end:
.section .text
FN SelfTest,64
    mov ebp,10
    lea rcx,[rip+sha_test_input]
    mov edx,3
    lea r8,[rsp+32]
    call Sha256
    lea rbx,[rip+sha_test_expected]
    xor esi,esi
1:  mov al,[rbx+rsi]
    cmp al,[rsp+32+rsi]
    jne .st_fail
    inc esi
    cmp esi,32
    jb 1b
    inc ebp
    mov ecx,45
    mov edx,6
    call Choose
    cmp rax,TOTAL_COMBINATIONS
    jne .st_fail
    inc ebp
    mov dword ptr [rip+rng_mode],1
    mov qword ptr [rip+rng_state],86421
    mov esi,10000
1:  call GenerateUniformMask
    mov rbx,rax
    shr rax,45
    jnz .st_fail
    mov rcx,rbx
    call PopCount64
    cmp eax,6
    jne .st_fail
    dec esi
    jnz 1b
    inc ebp
    lea rcx,[rip+selftest_csv]
    mov edx,selftest_csv_end-selftest_csv
    call ParseCsv
    cmp eax,1
    jne .st_fail
    cmp dword ptr [rip+draw_count],60
    jne .st_fail
    inc ebp
    lea rcx,[rip+bad_csv]
    mov edx,bad_csv_end-bad_csv
    call ParseCsv
    test eax,eax
    jnz .st_fail
    cmp dword ptr [rip+draw_count],60
    jne .st_fail
    inc ebp
    call AnalyzeOracle
    test eax,eax
    jz .st_fail
    cmp dword ptr [rip+scan_count],TOTAL_COMBINATIONS
    jne .st_fail
    cmp dword ptr [rip+field_count],512
    jne .st_fail
    inc ebp
    mov ecx,10
    call GenerateTickets
    test eax,eax
    jz .st_fail
    inc ebp
    call BuildReport
    test eax,eax
    jz .st_fail
    xor eax,eax
    jmp .st_out
.st_fail: mov eax,ebp
.st_out: END 64

.section .rdata,"dr"
selftest_csv:
    .ascii "round,n1,n2,n3,n4,n5,n6\n"
    .ascii "1,7,13,15,24,25,40\n"
    .ascii "2,4,14,22,24,26,27\n"
    .ascii "3,4,9,11,32,35,41\n"
    .ascii "4,8,10,17,26,28,32\n"
    .ascii "5,6,14,20,24,34,43\n"
    .ascii "6,14,30,33,34,38,40\n"
    .ascii "7,3,19,23,30,31,32\n"
    .ascii "8,1,3,12,17,22,34\n"
    .ascii "9,4,10,22,29,32,39\n"
    .ascii "10,1,6,9,15,23,37\n"
    .ascii "11,4,10,20,21,26,33\n"
    .ascii "12,5,17,19,34,37,38\n"
    .ascii "13,2,7,14,36,41,44\n"
    .ascii "14,2,27,30,37,39,40\n"
    .ascii "15,2,6,8,9,26,41\n"
    .ascii "16,2,8,16,25,29,32\n"
    .ascii "17,1,3,7,18,20,44\n"
    .ascii "18,8,13,16,33,34,38\n"
    .ascii "19,1,4,7,12,29,40\n"
    .ascii "20,4,13,25,33,35,38\n"
    .ascii "21,1,10,19,29,36,38\n"
    .ascii "22,24,33,36,40,41,43\n"
    .ascii "23,8,9,14,28,35,43\n"
    .ascii "24,11,29,30,32,35,39\n"
    .ascii "25,2,11,18,21,28,38\n"
    .ascii "26,10,24,26,35,36,41\n"
    .ascii "27,2,9,12,24,28,45\n"
    .ascii "28,5,11,24,27,36,40\n"
    .ascii "29,2,10,17,19,32,36\n"
    .ascii "30,15,19,24,33,37,45\n"
    .ascii "31,3,14,15,24,25,30\n"
    .ascii "32,11,15,24,28,33,39\n"
    .ascii "33,12,30,35,38,39,43\n"
    .ascii "34,2,10,18,20,29,37\n"
    .ascii "35,5,21,25,30,32,33\n"
    .ascii "36,3,13,19,28,30,33\n"
    .ascii "37,2,9,18,19,30,35\n"
    .ascii "38,19,29,38,40,42,43\n"
    .ascii "39,12,17,26,31,35,40\n"
    .ascii "40,3,18,27,30,34,35\n"
    .ascii "41,12,13,23,26,32,42\n"
    .ascii "42,5,7,16,20,22,37\n"
    .ascii "43,14,15,18,26,34,38\n"
    .ascii "44,6,7,12,19,21,28\n"
    .ascii "45,6,13,17,27,32,41\n"
    .ascii "46,6,8,17,23,32,34\n"
    .ascii "47,4,9,24,26,32,35\n"
    .ascii "48,11,14,18,29,35,45\n"
    .ascii "49,2,6,11,30,32,33\n"
    .ascii "50,3,18,31,34,35,36\n"
    .ascii "51,3,7,8,27,32,45\n"
    .ascii "52,1,2,18,24,37,40\n"
    .ascii "53,14,16,21,23,26,35\n"
    .ascii "54,1,6,19,26,29,40\n"
    .ascii "55,4,5,7,9,33,39\n"
    .ascii "56,3,9,12,24,42,43\n"
    .ascii "57,7,9,13,16,19,24\n"
    .ascii "58,7,21,28,32,34,37\n"
    .ascii "59,8,13,22,32,35,42\n"
    .ascii "60,7,9,11,16,19,27\n"
selftest_csv_end:

.section .text
# Small non-destructive startup gate; the full fixture test is CLI/CI-only.
FN StartupCheck,32
    lea rcx,[rip+sha_test_input]
    mov edx,3
    lea r8,[rsp+32]
    call Sha256
    lea rbx,[rip+sha_test_expected]
    xor esi,esi
1:  mov al,[rbx+rsi]
    cmp al,[rsp+32+rsi]
    jne .sc_fail
    inc esi
    cmp esi,32
    jb 1b
    mov ecx,45
    mov edx,6
    call Choose
    cmp rax,TOTAL_COMBINATIONS
    jne .sc_fail
    call GenerateUniformMask
    test rax,rax
    jz .sc_fail
    mov eax,1
    jmp .sc_out
.sc_fail: xor eax,eax
.sc_out: END 32
