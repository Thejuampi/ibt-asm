; Compare the 53 named routines, using the reference image's symbol identities.
; Define KERNEL_RVA_BIAS, KERNEL_FILE_SIZE, KERNEL_CONTEXT and KERNEL_FAILURE.
; R12 points to the reference image; the context contains its symbol table.

%assign kernel_symbol_index 0
%macro KERNEL_SYMBOL 1
kernel_symbol_id_%1 equ kernel_symbol_index
%assign kernel_symbol_index kernel_symbol_index + 1
%endmacro
%include 'native/kernel_symbol_table.inc'
%unmacro KERNEL_SYMBOL 1

    lea rax, [KERNEL_CONTEXT]
    mov rbx, [rax+32]               ; reference symbol entries

%macro KERNEL_COMPARE_ROUTINE 2
    mov eax, [rbx + kernel_symbol_id_%1 * 4]
    mov ecx, [rbx + kernel_symbol_id_%2 * 4]
    cmp rcx, rax
    jbe KERNEL_FAILURE
    sub rcx, rax                    ; reference routine length
    sub rax, KERNEL_RVA_BIAS        ; reference RVA to file offset
    jc KERNEL_FAILURE
    lea rdx, [rax + rcx]
    cmp rdx, KERNEL_FILE_SIZE
    ja KERNEL_FAILURE
    lea rdx, [r12 + rax]
    add rcx, rdx                    ; reference end
    lea rsi, [rel %1]
    lea rdi, [rel %2]
    lea r8, [rel kernel_resolve_symbol]
    lea r9, [KERNEL_CONTEXT]
    call kcmp_compare_ranges
    test eax, eax
    jz KERNEL_FAILURE
%endmacro

%assign kernel_routine_count 0
%macro KERNEL_ROUTINE 4
    KERNEL_COMPARE_ROUTINE %1, %2
%assign kernel_routine_count kernel_routine_count + 1
%endmacro
%include 'reference/numerical/kernel_routines.inc'
%unmacro KERNEL_ROUTINE 4
%macro KERNEL_ROUTINE 4
    KERNEL_COMPARE_ROUTINE %3, %4
%assign kernel_routine_count kernel_routine_count + 1
%endmacro
%include 'reference/numerical/kernel_routines.inc'
%unmacro KERNEL_ROUTINE 4
KERNEL_COMPARE_ROUTINE verify_lp_resid, verify_core_end
%assign kernel_routine_count kernel_routine_count + 1
%if kernel_routine_count != 53
    %error The verifier must check all 53 numerical routines
%endif

; Preserve the coverage of adjacent helpers that belonged to the original
; checked spans but are not themselves among the 53 named routines.
KERNEL_COMPARE_ROUTINE lp_fill_x, lp_dgetrf
KERNEL_COMPARE_ROUTINE lp_laswp_tail, lp_laswp_cols
KERNEL_COMPARE_ROUTINE lp_dtrsm_left, lp_dtrsm_cols
KERNEL_COMPARE_ROUTINE lp_dgemm_neg, lp_steal
KERNEL_COMPARE_ROUTINE lp_worker, lp_dgetrs
KERNEL_COMPARE_ROUTINE dual_lp_fill_x, dual_lp_dgetrf
KERNEL_COMPARE_ROUTINE dual_lp_laswp_tail, dual_lp_laswp_cols
KERNEL_COMPARE_ROUTINE dual_lp_dtrsm_left, dual_lp_dtrsm_cols
KERNEL_COMPARE_ROUTINE dual_lp_dgemm_neg, dual_lp_steal
%unmacro KERNEL_COMPARE_ROUTINE 2
