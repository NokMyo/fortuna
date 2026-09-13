.intel_syntax noprefix

# Febius Fortuna — x86-64 Windows native application.
# The executable is assembled from assembly-only modules; no C/C++ runtime.
.include "src/platform.inc"
.include "src/state.inc"

.section .text
.globl WinMainCRTStartup
.include "src/ui.inc"
.include "src/data.inc"
.include "src/oracle.inc"
.include "src/sample.inc"
