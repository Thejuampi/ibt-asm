; Assemble the Windows application directly as a PE32+ image with NASM.
bits 64
default rel
%define PE_BASE 0x140000000

section .header start=0 vstart=PE_BASE
    db 'MZ'
    times 0x3c - ($ - $$) db 0
    dd pe_header - PE_BASE
pe_header:
    db 'PE', 0, 0
    dw 0x8664, 4
    dd 0, 0, 0
    dw 0xf0, 0x0022
    dw 0x20b
    db 0, 0
    dd pe_text_raw_size, pe_data_raw_size, pe_bss_end - pe_bss_start
    dd start - PE_BASE, pe_text_start - PE_BASE
    dq PE_BASE
    dd 0x1000, 0x200
    dw 6, 0, 0, 0, 6, 0
    dd 0
    dd pe_reloc_start - PE_BASE + ((pe_reloc_end - pe_reloc_start + 0xfff) & ~0xfff)
    dd 0x400, 0
    dw 2, 0x0160
    dq 0x100000, 0x1000, 0x100000, 0x1000
    dd 0, 16
    dd 0, 0
    dd pe_import_descriptors - PE_BASE, pe_import_descriptors_end - pe_import_descriptors
    dd pe_rsrc_start - PE_BASE, pe_rsrc_end - pe_rsrc_start
    dd 0, 0
    dd 0, 0
    dd pe_reloc_start - PE_BASE, pe_reloc_end - pe_reloc_start
    times 10 dq 0

    db '.text', 0, 0, 0
    dd pe_text_raw_size, pe_text_start - PE_BASE
    dd pe_text_raw_size, 0x400
    dd 0, 0
    dw 0, 0
    dd 0x60000020
    db '.data', 0, 0, 0
    dd pe_data_virtual_size, pe_data_start - PE_BASE
    dd pe_data_raw_size, 0x400 + pe_text_raw_size
    dd 0, 0
    dw 0, 0
    dd 0xc0000040
    db '.rsrc', 0, 0, 0
    dd pe_rsrc_end - pe_rsrc_start, pe_rsrc_start - PE_BASE
    dd pe_rsrc_raw_size, 0x400 + pe_text_raw_size + pe_data_raw_size
    dd 0, 0
    dw 0, 0
    dd 0x40000040
    db '.reloc', 0, 0
    dd pe_reloc_end - pe_reloc_start, pe_reloc_start - PE_BASE
    dd pe_reloc_raw_size, 0x400 + pe_text_raw_size + pe_data_raw_size + pe_rsrc_raw_size
    dd 0, 0
    dw 0, 0
    dd 0x42000040
    ; Stable offsets let the verifier find the original routines in this image.
    times 0x300 - ($ - $$) db 0
    db 'KRN1'
    dd 4
    dd lp_fill_cols - pe_text_start, lp_emit - lp_fill_cols
    dd lp_pack_a - pe_text_start, lp_core_end - lp_pack_a
    dd dual_lp_fill_cols - pe_text_start, dual_core_end - dual_lp_fill_cols
    dd verify_lp_resid - pe_text_start, verify_core_end - verify_lp_resid
%assign kernel_count 0
%macro KERNEL_LPK 1
    dd %1 - lp_fill_cols
%assign kernel_count kernel_count + 1
%endmacro
%macro KERNEL_BENCH 1
    dd %1 - lp_pack_a
%assign kernel_count kernel_count + 1
%endmacro
%include 'reference/numerical/kernel_table.inc'
%unmacro KERNEL_LPK 1
%unmacro KERNEL_BENCH 1
%if kernel_count != 26
    %error The numerical reference must name all 26 primary routines
%endif
%macro KERNEL_LPK 1
    dd dual_%1 - dual_lp_fill_cols
%endmacro
%macro KERNEL_BENCH 1
    dd dual_%1 - dual_lp_fill_cols
%endmacro
%include 'reference/numerical/kernel_table.inc'
%unmacro KERNEL_LPK 1
%unmacro KERNEL_BENCH 1
    dd verify_lp_resid - verify_lp_resid
    dd 0x400 + ((pe_text_primary_end - pe_text_start + 15) & ~15) + (pe_kernel_symbols - pe_v3text_start)
    times 0x400 - ($ - $$) db 0

section .text start=0x400 vstart=PE_BASE+0x1000 align=16
pe_text_start:
section .text$v3 follows=.text vfollows=.text align=16 valign=16
pe_v3text_start:
section .data follows=.text$v3 vfollows=.text$v3 align=512 valign=4096
pe_data_start:
section .v3str follows=.data vfollows=.data align=16 valign=16
pe_v3str_start:
section .idata follows=.v3str vfollows=.v3str align=8 valign=8
pe_idata_start:
; Uninitialized state occupies memory, not bytes in the executable.
section .pe_bss nobits follows=.idata align=64
pe_bss_start:
%ifdef KERNEL_EQUIVALENT_ADDRESS
    resb 64
%endif
section .rsrc follows=.idata vfollows=.pe_bss align=512 valign=4096
section .reloc follows=.rsrc vfollows=.rsrc align=512 valign=4096
pe_reloc_start:

; Binary output has no COFF imports: the stubs and IAT are emitted below.
%macro extern 1
%endmacro
%macro section 1-*
    %ifidni %1, .bss
        [section .pe_bss]
    %else
        [section %1]
    %endif
%endmacro
%define PE_NATIVE 1
%macro PE_CONTEXT_POINTER 1
%%location:
    dq %1
    %ifnum %1
        %if %1 != 0
            %error Numeric context pointers must be null
        %endif
    %else
        [section .reloc]
        dd pe_data_start - PE_BASE + ((%%location - pe_data_start) & ~0xfff), 12
        dw 0xa000 | ((%%location - pe_data_start) & 0xfff), 0
        [section .data]
    %endif
%endmacro
%macro CONT_PTR 1-*
    %rep %0
        PE_CONTEXT_POINTER %1
        %rotate 1
    %endrep
%endmacro
%include 'ibt.asm'

section .text
; Preserve the numerical entry-point alignment checked by the former listing test.
%macro PE_REQUIRE_ALIGNED 1
    times (((((%1 - pe_text_start) & 31) = 0) - 1)) db 0
%endmacro
PE_REQUIRE_ALIGNED lp_k12x4
PE_REQUIRE_ALIGNED lp_k8x4
PE_REQUIRE_ALIGNED lp_k8x4_avx
PE_REQUIRE_ALIGNED lp_krest
PE_REQUIRE_ALIGNED lp_pack_a
PE_REQUIRE_ALIGNED lp_pack_b
PE_REQUIRE_ALIGNED lp_dgemm_cols
PE_REQUIRE_ALIGNED lp_dgemm_ir
PE_REQUIRE_ALIGNED lp_pack_b_all
PE_REQUIRE_ALIGNED lp_dgetrf
PE_REQUIRE_ALIGNED lp_dgetf2
PE_REQUIRE_ALIGNED lp_panel
PE_REQUIRE_ALIGNED lp_dgetrs
PE_REQUIRE_ALIGNED lp_dtrsm_cols
PE_REQUIRE_ALIGNED lp_scale_col
PE_REQUIRE_ALIGNED lp_daxpy_neg
PE_REQUIRE_ALIGNED lp_daxpy4_neg
PE_REQUIRE_ALIGNED lp_pivot
pe_text_primary_end:
section .data
pe_data_primary_end:
section .v3str
pe_expected_time: dw __utf16le__('2.000'), 0
pe_expected_time_label: dw __utf16le__('2.000 s'), 0
pe_expected_speed: dw __utf16le__('0.3579'), 0
pe_expected_residual: dw __utf16le__('1.250000e+00'), 0
pe_expected_changed_residual: dw __utf16le__('1.500000e+00'), 0
pe_expected_short_time: dw __utf16le__('0.125'), 0
pe_expected_short_speed: dw __utf16le__('0.7158'), 0
pe_expected_small_residual: dw __utf16le__('1.410597e-03'), 0
pe_small_residual: dq 0.001410597
pe_kernel_reference_path: dw __utf16le__('reference/numerical/windows.bin'), 0
pe_expected_selftest: db '1024 1032 4 2.000 0.3579 2.500000e+00 1.250000e+00', 0
pe_v3str_end:
section .text$v3
%macro PE_DLL 2
%endmacro
%macro PE_END_DLL 0
%endmacro
%macro PE_API 1
%1: jmp [rel pe_iat_%1]
%endmacro
%macro PE_ORD 2
    PE_API %1
%endmacro
%include 'native/pe_imports.inc'
%unmacro PE_DLL 2
%unmacro PE_END_DLL 0
%unmacro PE_API 1
%unmacro PE_ORD 2

; A self-test checks the resources embedded in this exact PE image.
pe_check_resources:
    sub rsp, 0x38
%ifdef PE_VERIFY_ASLR
    mov rax, PE_BASE
    cmp [hInst], rax
    je .fail
%endif
    mov rcx, [hInst]
    mov edx, 200
    mov r8d, 2
    call FindResourceW
    test rax, rax
    jz .fail
    mov rcx, [hInst]
    mov edx, 201
    mov r8d, 2
    call FindResourceW
    test rax, rax
    jz .fail
    mov rcx, [hInst]
    mov edx, 1
    mov r8d, 14
    call FindResourceW
    test rax, rax
    jz .fail
    mov rcx, [hInst]
    mov edx, 1
    mov r8d, 24
    call FindResourceW
    test rax, rax
    jz .fail
    mov rcx, [hInst]
    mov edx, 200
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp + 0x20], 0
    mov qword [rsp + 0x28], 0
    call LoadImageW
    test rax, rax
    jz .fail
    mov rcx, rax
    call DeleteObject
    mov rcx, [hInst]
    mov edx, 1
    mov r8d, 1
    xor r9d, r9d
    mov qword [rsp + 0x20], 0
    mov qword [rsp + 0x28], 0
    call LoadImageW
    test rax, rax
    jz .fail
    mov rcx, rax
    call DestroyIcon
    mov eax, 1
    add rsp, 0x38
    ret
.fail:
    xor eax, eax
    add rsp, 0x38
    ret

; Exercise the real window-message/result path with known numerical values.
pe_check_result_fields:
    sub rsp, 0x28
    mov dword [lpN], 1024
    mov dword [lpLDA], 1032
    mov rax, 0x4000000000000000    ; 2.0 seconds
    mov [lpTmpT], rax
    mov rax, 0x4004000000000000    ; 2.5 raw residual
    mov [lpTmpR], rax
    mov rax, 0x3ff4000000000000    ; 1.25 normalized residual
    mov [lpTmpN], rax
    mov dword [lpTest], 1
    call lp_emit
    lea rax, [lineAcc]
    lea rdx, [pe_expected_selftest]
.check_selftest:
    mov cl, [rax]
    cmp cl, [rdx]
    jne .fail
    test cl, cl
    jz .check_display
    inc rax
    inc rdx
    jmp .check_selftest
.check_display:
    mov dword [lpTest], 0
    mov dword [running], 1
    mov dword [resultState], RESULT_RUNNING
    mov dword [targetRuns], 10
    call lp_emit
    cmp dword [resultIndex], 1
    jne .fail
    cmp dword [passCount], 1
    jne .fail
    lea rcx, [resultTime]
    lea rdx, [pe_expected_time]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    lea rcx, [resultSpeed]
    lea rdx, [pe_expected_speed]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    lea rcx, [resultResid]
    lea rdx, [pe_expected_residual]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    call pe_check_metric_paint
    test eax, eax
    jz .fail
    lea rcx, [tmpBuf]
    lea rdx, [pe_expected_time_label]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    call lp_emit
    cmp dword [resultIndex], 2
    jne .fail
    cmp dword [passCount], 2
    jne .fail
    cmp dword [failed], 0
    jne .fail
    mov rax, 0x3ff8000000000000    ; 1.5 must not match the first residual
    mov [lpTmpN], rax
    call lp_emit
    cmp dword [resultIndex], 3
    jne .fail
    cmp dword [passCount], 2
    jne .fail
    cmp dword [failed], 1
    jne .fail
    lea rcx, [resultResid]
    lea rdx, [pe_expected_changed_residual]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    ; A small scientific-notation residual mirrors the reported display failure.
    mov dword [resultIndex], 0
    mov dword [passCount], 0
    mov dword [haveResid], 0
    mov dword [failed], 0
    mov dword [lpStop], 0
    mov dword [lpN], 512
    mov rax, 0x3fc0000000000000    ; 0.125 seconds
    mov [lpTmpT], rax
    movsd xmm0, [pe_small_residual]
    movsd [lpTmpN], xmm0
    call lp_emit
    lea rcx, [resultTime]
    lea rdx, [pe_expected_short_time]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    lea rcx, [resultSpeed]
    lea rdx, [pe_expected_short_speed]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    lea rcx, [resultResid]
    lea rdx, [pe_expected_small_residual]
    call lstrcmpiW
    test eax, eax
    jnz .fail
    call pe_check_metric_paint
    test eax, eax
    jz .fail
    mov dword [running], 0
    mov eax, 1
    add rsp, 0x28
    ret
.fail:
    mov dword [running], 0
    xor eax, eax
    add rsp, 0x28
    ret

; Draw the same result card used by the application, on its actual window DC.
pe_check_metric_paint:
    sub rsp, 0x78
    mov rcx, [hResultCard]
    call GetDC
    test rax, rax
    jz .fail
    mov [rsp + 0x20], rax
    pxor xmm0, xmm0
    movdqu [rsp + 0x30], xmm0
    movdqu [rsp + 0x40], xmm0
    movdqu [rsp + 0x50], xmm0
    movdqu [rsp + 0x60], xmm0
    mov [rsp + 0x50], rax        ; DRAWITEMSTRUCT.hDC
    mov rcx, [hResultCard]
    lea rdx, [rsp + 0x58]        ; DRAWITEMSTRUCT.rcItem
    call GetClientRect
    test eax, eax
    jz .release_fail
    lea rcx, [rsp + 0x30]
    call result_draw
    mov [rsp + 0x28], eax
    mov rcx, [hResultCard]
    mov rdx, [rsp + 0x20]
    call ReleaseDC
    cmp dword [rsp + 0x28], 1
    jne .fail
    mov eax, 1
    add rsp, 0x78
    ret
.release_fail:
    mov rcx, [hResultCard]
    mov rdx, [rsp + 0x20]
    call ReleaseDC
.fail:
    xor eax, eax
    add rsp, 0x78
    ret
; Compare the loaded numerical code to a reference assembled from frozen source.
; Each of the 53 routines is compared instruction by instruction.
pe_verify_kernels:
    sub rsp, 0x28
    lea rcx, [rel pe_kernel_reference_path]
    call pe_verify_reference
    add rsp, 0x28
    ret

pe_verify_reference:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 0x98
    xor r15d, r15d
    mov edx, 0x80000000
    mov r8d, 1
    xor r9d, r9d
    mov qword [rsp+0x20], 3
    mov qword [rsp+0x28], 0x80
    mov qword [rsp+0x30], 0
    call CreateFileW
    cmp rax, -1
    je .done
    mov rbx, rax
    xor ecx, ecx
    mov edx, 0x100000
    mov r8d, 0x3000
    mov r9d, 4
    call VirtualAlloc
    test rax, rax
    jz .close_only
    mov r12, rax
    mov rcx, rbx
    mov rdx, r12
    mov r8d, 0x100000
    lea r9, [rsp+0x40]
    mov qword [rsp+0x20], 0
    call ReadFile
    mov r13d, eax
    mov r14d, [rsp+0x40]
    mov rcx, rbx
    call CloseHandle
    test r13d, r13d
    jz .free
    cmp r14d, 0x400
    jb .free
    cmp dword [r12+0x300], 0x314e524b ; KRN1
    jne .free
    cmp dword [r12+0x304], 4
    jne .free
    mov eax, [r12+0x3fc]
    lea rdx, [rax+8]
    cmp rdx, r14
    ja .free
    lea rdx, [r12+rax]
    cmp dword [rdx], 0x3159534b ; KSY1
    jne .free
    mov ecx, [rdx+4]
    cmp ecx, (pe_kernel_symbol_end - pe_kernel_symbol_entries) / 4
    jne .free
    lea rax, [rax+rcx*4+8]
    cmp rax, r14
    ja .free
    mov rax, [hInst]
    mov [rsp+0x58], rax
    mov [rsp+0x60], r12
    mov qword [rsp+0x68], 0xc00
    lea rax, [rel pe_kernel_symbol_entries]
    mov [rsp+0x70], rax
    add rdx, 8
    mov [rsp+0x78], rdx
    mov [rsp+0x80], rcx
%define KERNEL_RVA_BIAS 0xc00
%define KERNEL_FILE_SIZE r14
%define KERNEL_CONTEXT rsp+0x58
%define KERNEL_FAILURE .free
%include 'native/kernel_verify_ranges.inc'
%undef KERNEL_RVA_BIAS
%undef KERNEL_FILE_SIZE
%undef KERNEL_CONTEXT
%undef KERNEL_FAILURE
    mov r15d, 1
.free:
    mov rcx, r12
    xor edx, edx
    mov r8d, 0x8000
    call VirtualFree
    jmp .done
.close_only:
    mov rcx, rbx
    call CloseHandle
.done:
    mov eax, r15d
    add rsp, 0x98
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
%include 'native/kernel_symbol_resolver.inc'
%include 'native/kernel_semantic_compare.inc'
%include 'native/kernel_decode.inc'
align 4, db 0
pe_kernel_symbols:
    db 'KSY1'
    dd (pe_kernel_symbol_end - pe_kernel_symbol_entries) / 4
pe_kernel_symbol_entries:
%macro KERNEL_SYMBOL 1
    dd %1 - PE_BASE
%endmacro
%include 'native/kernel_symbol_table.inc'
%unmacro KERNEL_SYMBOL 1
pe_kernel_symbol_end:
pe_text_end:

section .idata
pe_import_descriptors:
%macro PE_DLL 2
    dd 0, 0, 0, pe_dll_%1 - PE_BASE, pe_iat_%1 - PE_BASE
%endmacro
%macro PE_END_DLL 0
%endmacro
%macro PE_API 1
%endmacro
%macro PE_ORD 2
%endmacro
%include 'native/pe_imports.inc'
    times 5 dd 0
pe_import_descriptors_end:
%unmacro PE_DLL 2
%unmacro PE_END_DLL 0
%unmacro PE_API 1
%unmacro PE_ORD 2

%macro PE_DLL 2
align 8, db 0
pe_iat_%1:
%endmacro
%macro PE_END_DLL 0
    dq 0
%endmacro
%macro PE_API 1
pe_iat_%1: dq pe_name_%1 - PE_BASE
%endmacro
%macro PE_ORD 2
pe_iat_%1: dq 0x8000000000000000 | %2
%endmacro
%include 'native/pe_imports.inc'
%unmacro PE_DLL 2
%unmacro PE_END_DLL 0
%unmacro PE_API 1
%unmacro PE_ORD 2

%macro PE_DLL 2
pe_dll_%1: db %2, 0
%endmacro
%macro PE_END_DLL 0
%endmacro
%macro PE_API 1
align 2, db 0
pe_name_%1:
    dw 0
    %defstr %%name %1
    db %%name, 0
%endmacro
%macro PE_ORD 2
%endmacro
%include 'native/pe_imports.inc'
%unmacro PE_DLL 2
%unmacro PE_END_DLL 0
%unmacro PE_API 1
%unmacro PE_ORD 2
pe_data_initialized_end:

section .pe_bss
pe_gui_test: resd 1
pe_bss_end:

%include 'native/pe_resources.inc'

section .reloc
pe_reloc_end:

pe_text_raw_size equ ((((pe_text_primary_end - pe_text_start + 15) & ~15) + pe_text_end - pe_v3text_start + 0x1ff) & ~0x1ff)
pe_data_init_size equ (((((pe_data_primary_end - pe_data_start + 15) & ~15) + pe_v3str_end - pe_v3str_start + 7) & ~7) + pe_data_initialized_end - pe_idata_start)
pe_data_virtual_size equ ((pe_data_init_size + 63) & ~63) + pe_bss_end - pe_bss_start
pe_data_raw_size equ (pe_data_init_size + 0x1ff) & ~0x1ff
pe_rsrc_raw_size equ (pe_rsrc_end - pe_rsrc_start + 0x1ff) & ~0x1ff
pe_reloc_raw_size equ (pe_reloc_end - pe_reloc_start + 0x1ff) & ~0x1ff
times pe_reloc_raw_size - (pe_reloc_end - pe_reloc_start) db 0
