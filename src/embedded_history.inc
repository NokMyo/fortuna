# Generated payload lives in build/embedded_history_data.inc.
# scripts/generate-embedded-history.py verifies and expands the compressed source
# before every application build. Runtime logic remains x86-64 assembly only.
.section .rdata,"dr"
.p2align 4
embedded_history_start:
.include "build/embedded_history_data.inc"
embedded_history_end:
