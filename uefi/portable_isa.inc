; The shared scalar algorithm used VEX encodings even in its SSE2 branch.
; Lower only its XMM scalar instructions to legacy SSE2 here. AVX/AVX2 YMM
; kernels remain native, with no per-operation dispatch or separate algorithm.
; Reject operand aliasing that cannot be lowered without a temporary register.
%macro SCALAR_BINARY 4
    %ifnidni %2, %3
        %ifidni %2, %4
            %error "SSE2 lowering needs an explicit temporary for this alias"
        %endif
        movapd %2, %3
    %endif
    %1 %2, %4
%endmacro
%macro vmovsd 2
    movsd %1, %2
%endmacro
%macro vmovapd 2
    %defstr %%operand %1
    %substr %%head %%operand 1,3
    %ifidni %%head, 'xmm'
        movapd %1, %2
    %else
        vmovapd %1, %2
    %endif
%endmacro
%macro vcvtsi2sd 3
    %ifnidni %1, %2
        movapd %1, %2
    %endif
    cvtsi2sd %1, %3
%endmacro
%macro vucomisd 2
    ucomisd %1, %2
%endmacro
%macro vxorpd 3
    SCALAR_BINARY xorpd, %1, %2, %3
%endmacro
%macro vandpd 3
    SCALAR_BINARY andpd, %1, %2, %3
%endmacro
%macro vaddsd 3
    SCALAR_BINARY addsd, %1, %2, %3
%endmacro
%macro vsubsd 3
    SCALAR_BINARY subsd, %1, %2, %3
%endmacro
%macro vmulsd 3
    SCALAR_BINARY mulsd, %1, %2, %3
%endmacro
%macro vdivsd 3
    SCALAR_BINARY divsd, %1, %2, %3
%endmacro
%macro vmaxsd 3
    SCALAR_BINARY maxsd, %1, %2, %3
%endmacro

; Four independent columns, two rows per iteration. No AVX or FMA needed.
; Keep the same multiply-then-subtract semantics as the scalar daxpy routine.
section .text
uefi_daxpy4_sse2:
    test dword [lpIsa], 2
    jz .legacy
    vzeroupper                 ; AVX-only callers enter a clean legacy SSE path
.legacy:
    test ecx, ecx
    jle .out
    movapd xmm12, xmm0
    movapd xmm13, xmm1
    movapd xmm14, xmm2
    movapd xmm15, xmm3
    unpcklpd xmm12, xmm12
    unpcklpd xmm13, xmm13
    unpcklpd xmm14, xmm14
    unpcklpd xmm15, xmm15
.pair:
    cmp ecx, 2
    jb .tail
    movupd xmm0, [rdx]
    movapd xmm1, xmm0
    mulpd xmm1, xmm12
    movupd xmm2, [r8]
    subpd xmm2, xmm1
    movupd [r8], xmm2
    movapd xmm1, xmm0
    mulpd xmm1, xmm13
    movupd xmm2, [r9]
    subpd xmm2, xmm1
    movupd [r9], xmm2
    movapd xmm1, xmm0
    mulpd xmm1, xmm14
    movupd xmm2, [r10]
    subpd xmm2, xmm1
    movupd [r10], xmm2
    mulpd xmm0, xmm15
    movupd xmm2, [r11]
    subpd xmm2, xmm0
    movupd [r11], xmm2
    add rdx, 16
    add r8, 16
    add r9, 16
    add r10, 16
    add r11, 16
    sub ecx, 2
    jmp .pair
.tail:
    test ecx, ecx
    jz .out
    movsd xmm0, [rdx]
    movapd xmm1, xmm0
    mulsd xmm1, xmm12
    movsd xmm2, [r8]
    subsd xmm2, xmm1
    movsd [r8], xmm2
    movapd xmm1, xmm0
    mulsd xmm1, xmm13
    movsd xmm2, [r9]
    subsd xmm2, xmm1
    movsd [r9], xmm2
    movapd xmm1, xmm0
    mulsd xmm1, xmm14
    movsd xmm2, [r10]
    subsd xmm2, xmm1
    movsd [r10], xmm2
    mulsd xmm0, xmm15
    movsd xmm2, [r11]
    subsd xmm2, xmm0
    movsd [r11], xmm2
.out:
    ret
