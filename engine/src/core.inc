# Microsoft x64 ABI throughout. All nonvolatile GPRs are preserved. SSE2 only.
# Frame local storage starts at rsp+32; n must be a multiple of 16.
.macro FN name, n=0
.ifdef WINDOWS
    .seh_proc \name
.endif
\name:
    push rbp
.ifdef WINDOWS
    .seh_pushreg rbp
.endif
    push rbx
.ifdef WINDOWS
    .seh_pushreg rbx
.endif
    push rsi
.ifdef WINDOWS
    .seh_pushreg rsi
.endif
    push rdi
.ifdef WINDOWS
    .seh_pushreg rdi
.endif
    push r12
.ifdef WINDOWS
    .seh_pushreg r12
.endif
    push r13
.ifdef WINDOWS
    .seh_pushreg r13
.endif
    push r14
.ifdef WINDOWS
    .seh_pushreg r14
.endif
    push r15
.ifdef WINDOWS
    .seh_pushreg r15
.endif
    sub rsp, 40+\n
.ifdef WINDOWS
    .seh_stackalloc 40+\n
    .seh_endprologue
.endif
.endm
.macro END n=0
    add rsp, 40+\n
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
.ifdef WINDOWS
    .seh_endproc
.endif
.endm
.equ MAX_DRAWS, 8192
.equ RECORD_SIZE, 48
.equ TOP_FAST, 4096
.equ MAX_FAST,8192
.equ TOP_FINAL, 512
.equ MODEL_COUNT, 12
.equ FEATURE_COUNT, 8
.equ WORLDS, 32
.equ NULL_COUNT, 512
.equ TOTAL_COMBINATIONS, 8145060

.section .rdata,"dr"
.p2align 4
one: .double 1.0
zero: .double 0.0
two: .double 2.0
half: .double 0.5
six: .double 6.0
fortyfive: .double 45.0
p_one: .double 0.13333333333333333333
p_pair: .double 0.01515151515151515152
p_triple: .double 0.00140944326990838619
prior: .double 60.0
prior_pair: .double 120.0
prior_triple: .double 240.0
epsilon: .double 0.000000001
ridge: .double 0.10
scale_q: .double 1024.0
mad_factor: .double 1.4826
zlimit: .double 4.0
neg_zlimit: .double -4.0
ln2: .double 0.69314718055994530942
inv_ln2: .double 1.44269504088896340736
exp_min: .double -700.0
exp_max: .double 700.0
.p2align 4
abs_bits: .quad 0x7fffffffffffffff,0x7fffffffffffffff
.p2align 4
sign_bits: .quad 0x8000000000000000,0
# exp(-1/tau), tau=4,8,16,32,64,128,256 (infinity is lifetime).
decays: .double 0.7788007830714049,0.8824969025845955,0.9394130628134758,0.9692332344763441,0.9844964370054085,0.9922179382602435,0.9961013694701175
entropy_lut: .double 0,0.2986265782046758,0.3662040962227032,0.3465735902799727,0.2703100720721096,0.1519346306616288,0
prime_bits: .quad 0x51050451456

.section .data
.p2align 4
rng_state: .quad 1
rng_mode: .long 0
rng_error: .long 0
cancel_flag: .long 0
progress: .long 0
analysis_stage: .long 0
history_count: .long 0
last_round: .long 0
# Full loaded source dataset and currently activated analysis range.
source_history_count: .long 0
source_first_round: .long 0
source_last_round: .long 0
backtest_start_round: .long 0
backtest_end_round: .long 0
# Causal validation targets inside the active analysis range. The first
# 60 active rows are training-only; the rest may become validation targets.
backtest_start_index: .long 60
backtest_end_index: .long 0
parse_error: .long 0
parse_line: .long 0
model_ready: .long 0
field_ready: .long 0
field_count: .long 0
scan_count: .long 0
rho: .double 1.0
evidence: .double 0.0
temperature: .double 1.0
elapsed_ms: .quad 0
config_version: .asciz "OFA-research-1.1;JOINT=EB-residual-45C6;caps=.05/.02/.01;shrink=.5/1/2;inner=4;outer_max=30;worlds=8..32;MC_SE=.0005;audit=512/65536;expanded=8192;forecast=FJP1/half-uniform;K=4096/512;models=12;worlds=32;null=512;seed=20260913;gate=0.05;min_train=60;min_test=30;confirmation=30;min_promote=120;analysis=history-range-v2"

.section .bss
.p2align 4
history: .skip MAX_DRAWS*RECORD_SIZE
# Immutable source copy for the current loaded file. Range changes rebuild the
# active history from here, so selecting 1..432 truly removes 433+ from fitting.
source_history: .skip MAX_DRAWS*RECORD_SIZE
staging: .skip MAX_DRAWS*RECORD_SIZE
staging_count: .skip 4
csv_columns: .skip 10*8
csv_colflags: .skip 10*4
csv_numbers: .skip 6*4
hash_dataset: .skip 32
hash_config: .skip 32
hash_field: .skip 32
selected_mask: .skip 8
selected_nums: .skip 6*4
selected_score: .skip 8
selected_index: .skip 4
session_count: .skip 4
session_masks: .skip 10*8
session_indices: .skip 10*4

stats_begin:
draw_count: .skip 4
freq_total: .skip 45*4
last_seen: .skip 45*4
hazard_risk: .skip 45*64*4
hazard_events: .skip 45*64*4
pair_counts: .skip 45*45*4
pair_decay: .skip 45*45*8
triple_counts: .skip 14190*4
freq_ewma: .skip 7*45*8
ewma_mass: .skip 7*8
feature_mean: .skip FEATURE_COUNT*8
feature_m2: .skip FEATURE_COUNT*FEATURE_COUNT*8
entropy_mean: .skip 5*8
entropy_m2: .skip 5*8
stats_end:
.equ STATS_BYTES,stats_end-stats_begin

feature_buf: .skip FEATURE_COUNT*8
feature_delta: .skip FEATURE_COUNT*8
entropy_buf: .skip 5*8
extract_buf: .skip 6*4
feature_buckets: .skip 50*4
number_models: .skip 4*45*8
pair_models: .skip 3*45*45*8
triple_scores: .skip 14190*8
spectral_a: .skip 45*45*8
spectral_u: .skip 45*45*8
spectral_l: .skip 45*45*8
cov_inverse: .skip FEATURE_COUNT*FEATURE_COUNT*8
matrix_work: .skip 45*90*8
number_fast: .skip 45*8
pair_fast: .skip 45*45*8
raw_models: .skip MODEL_COUNT*8
.equ MODEL_BYTES,raw_models-number_models
norm_center: .skip MODEL_COUNT*8
norm_scale: .skip MODEL_COUNT*8
model_weights: .skip MODEL_COUNT*8
model_quality: .skip MODEL_COUNT*8
model_pvalue: .skip MODEL_COUNT*8
model_qvalue: .skip MODEL_COUNT*8
model_gates: .skip MODEL_COUNT*4
model_corr: .skip MODEL_COUNT*MODEL_COUNT*8
null_masks: .skip NULL_COUNT*8
null_models: .skip NULL_COUNT*MODEL_COUNT*8
sort_work: .skip 8192*8
fast_count: .skip 4
fast_masks: .skip MAX_FAST*8
fast_scores: .skip MAX_FAST*8
final_count: .skip 4
final_masks: .skip TOP_FINAL*8
final_scores: .skip TOP_FINAL*8
final_raw: .skip TOP_FINAL*MODEL_COUNT*8
final_prob: .skip TOP_FINAL*8
final_cdf: .skip TOP_FINAL*8
persistence: .skip 45*8
geometry: .skip TOP_FINAL*4*8
swap_sensitivity: .skip 6*8
world_scores: .skip WORLDS*TOP_FINAL*8
world_ranks: .skip WORLDS*TOP_FINAL*4
world_summary: .skip TOP_FINAL*4*8
stats_backup: .skip stats_end-stats_begin
model_backup: .skip 300000
validation_begin:
validation_count: .skip 4
validation_cutoff: .skip 4
validation_hits: .skip 7*4
validation_sum: .skip 8
validation_sumsq: .skip 8
validation_brier: .skip 8
validation_baseline_brier: .skip 8
validation_p: .skip 8
validation_mean: .skip 8
validation_lower: .skip 8
validation_upper: .skip 8
validation_models: .skip MODEL_COUNT*8
validation_models_sq: .skip MODEL_COUNT*8
validation_ablation: .skip MODEL_COUNT*8
validation_bins: .skip 10*3*8
validation_rows: .skip MAX_DRAWS*64
validation_end:
.equ VALIDATION_BYTES,validation_end-validation_begin

.section .text
# Zero(ptr,bytes), Copy(dst,src,bytes); safe for zero length, non-overlap Copy.
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
FN Choose
    mov eax,1
    test edx,edx
    jz 3f
    cmp edx,ecx
    ja 2f
    mov r8d,edx
    mov r9d,1
    sub ecx,edx
1:  lea r10d,[rcx+r9]
    imul rax,r10
    xor edx,edx
    div r9
    inc r9d
    cmp r9d,r8d
    jbe 1b
    jmp 3f
2:  xor eax,eax
3:  END

# TripleIndex(a,b,c), zero-based sorted indices. C(c,3)+C(b,2)+a.
FN TripleIndex
    mov eax,r8d
    lea r9d,[rax-1]
    imul eax,r9d
    sub r9d,1
    imul eax,r9d
    mov r10d,edx
    xor edx,edx
    mov r9d,6
    div r9d
    lea edx,[r10-1]
    imul edx,r10d
    shr edx,1
    add eax,edx
    add eax,ecx
    END

# Natural log for finite x>0; x87 elementary instruction, result SSE2.
FN Log,16
    movsd [rsp+32],xmm0
    fldln2
    fld qword ptr [rsp+32]
    fyl2x
    fstp qword ptr [rsp+32]
    movsd xmm0,[rsp+32]
    END 16
# exp(x), clamped to [-700,700], explicit x87 stack balance.
FN Exp,16
    maxsd xmm0,[rip+exp_min]
    minsd xmm0,[rip+exp_max]
    mulsd xmm0,[rip+inv_ln2]
    movsd [rsp+32],xmm0
    fld qword ptr [rsp+32]
    fld st(0)
    frndint
    fxch st(1)
    fsub st(0),st(1)
    f2xm1
    fld1
    faddp st(1),st(0)
    fscale
    fstp st(1)
    fstp qword ptr [rsp+32]
    movsd xmm0,[rsp+32]
    END 16

# Stable insertion sort of finite doubles; bounded n <=8192, diagnostics path.
FN SortDoubles
    mov r8d,1
1:  cmp r8d,edx
    jae 4f
    movsd xmm0,[rcx+r8*8]
    mov r9d,r8d
2:  test r9d,r9d
    jz 3f
    movsd xmm1,[rcx+r9*8-8]
    comisd xmm1,xmm0
    jbe 3f
    movsd [rcx+r9*8],xmm1
    dec r9d
    jmp 2b
3:  movsd [rcx+r9*8],xmm0
    inc r8d
    jmp 1b
4:  END
# Median of an array, copy into private global scratch (single analysis worker).
FN Median
    mov ebx,edx
    mov rsi,rcx
    lea rcx,[rip+sort_work]
    mov rdx,rsi
    lea r8,[rbx*8]
    call Copy
    lea rcx,[rip+sort_work]
    mov edx,ebx
    call SortDoubles
    mov eax,ebx
    shr eax,1
    lea rdx,[rip+sort_work]
    movsd xmm0,[rdx+rax*8]
    test ebx,1
    jnz 1f
    addsd xmm0,[rdx+rax*8-8]
    mulsd xmm0,[rip+half]
1:  END

# Reproducible SplitMix64 stream for research/official mode only. Session uses OS.
FN Random64,16
    cmp dword ptr [rip+rng_mode],0
    je 2f
    mov rax,0x9e3779b97f4a7c15
    add [rip+rng_state],rax
    mov rax,[rip+rng_state]
    mov rdx,rax
    shr rdx,30
    xor rax,rdx
    mov rdx,0xbf58476d1ce4e5b9
    imul rax,rdx
    mov rdx,rax
    shr rdx,27
    xor rax,rdx
    mov rdx,0x94d049bb133111eb
    imul rax,rdx
    mov rdx,rax
    shr rdx,31
    xor rax,rdx
    jmp 4f
2:  xor ecx,ecx
    lea rdx,[rsp+32]
    mov r8d,8
    mov r9d,2
    call BCryptGenRandom
    test eax,eax
    jz 3f
    mov dword ptr [rip+rng_error],1
    xor eax,eax
    jmp 4f
3:  mov rax,[rsp+32]
4:  END 16

# Unbiased bounded uint64 via threshold rejection; failure is explicit in rng_error.
FN RandomBelow
    mov rbx,rcx
    test rbx,rbx
    jz 3f
    mov rax,rbx
    neg rax
    xor edx,edx
    div rbx
    mov rsi,rdx
1:  call Random64
    cmp dword ptr [rip+rng_error],0
    jne 3f
    cmp rax,rsi
    jb 1b
    xor edx,edx
    div rbx
    mov rax,rdx
    jmp 4f
3:  mov dword ptr [rip+rng_error],1
    xor eax,eax
4:  END
FN GenerateUniformMask
    xor ebx,ebx
    xor esi,esi
1:  mov ecx,45
    call RandomBelow
    cmp dword ptr [rip+rng_error],0
    jne 3f
    bt rbx,rax
    jc 1b
    bts rbx,rax
    inc esi
    cmp esi,6
    jb 1b
    mov rax,rbx
    jmp 4f
3:  xor eax,eax
4:  END
FN RandomUnit
    call Random64
    shr rax,11
    cvtsi2sd xmm0,rax
    mov rax,0x4340000000000000
    movq xmm1,rax
    divsd xmm0,xmm1
    END
