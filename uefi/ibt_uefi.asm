bits 64
default rel

; OS-free x64 UEFI frontend. Arithmetic, packing, and workers stay shared.
%define SELFTEST 0
%ifndef UEFI_QA
    %define UEFI_QA 0
%endif
%ifndef UEFI_QA_CASE
    %define UEFI_QA_CASE 0
%endif

%include "efi.inc"
%include "state.inc"
; Public COFF symbols for the build's map file and emulator inspection.
; They do not create a PE export table or runtime dependencies.
global szHeader, testActive, passCount, failed, lpAllocFail, dispatchFailed
global stopRequested, lpNorm0, stressMB, lpIsaUse
global uiMode, uiStage, uiElapsed, uiResultCount, uiResults, uiOriginX, uiOriginY

section .text
global efi_main
%include "runtime.inc"
%include "app.inc"
%include "ui.inc"
%include "qa.inc"
%define IBT_PORTABLE_ISA 1
%include "../ibt_lpk.inc"
%include "../bench_lib.inc"
; Compile the same scalar algorithm with genuine SSE2 encodings as a second
; path. The native AVX/FMA code above retains its VEX instructions throughout.
%include "portable_isa.inc"
%include "sse2_symbols.inc"
%include "../ibt_lpk.inc"
%include "../bench_lib.inc"
%include "sse2_symbols_end.inc"
