# Additive ABI v1 snapshot. Results are readable only with a completed analysis.
.section .text
.globl FortunaOracleGetDepthSnapshot
# 96 bytes: eight uint32 (size,version,ready,profile,folds,bootstrap,iterations,
# reserved), eight doubles (residual,p,log_e,boot_low,boot_high,CUSUM,ablation,0).
FN FortunaOracleGetDepthSnapshot
    test rcx,rcx
    jz .gds_arg
    cmp edx,96
    jb .gds_arg
    mov rbx,rcx
    call ApiLock
    test eax,eax
    jnz .gds_out
    cmp dword ptr [rip+ti_ready],1
    jne .gds_state
    mov rcx,rbx
    mov edx,96
    call Zero
    mov dword ptr [rbx],96
    mov dword ptr [rbx+4],0x10000
    mov dword ptr [rbx+8],1
    mov eax,[rip+ti_profile]
    mov [rbx+12],eax
    mov eax,[rip+ti_folds]
    mov [rbx+16],eax
    mov eax,[rip+depth_boot_done]
    mov [rbx+20],eax
    mov eax,[rip+depth_graph_iterations]
    mov [rbx+24],eax
    lea rcx,[rbx+32]
    lea rdx,[rip+depth_graph_residual]
    mov r8d,7*8
    call Copy
    xor ecx,ecx
    jmp .gds_finish
.gds_state: mov ecx,-4
.gds_finish: call ApiFinish; jmp .gds_out
.gds_arg: mov eax,-1
.gds_out: END
