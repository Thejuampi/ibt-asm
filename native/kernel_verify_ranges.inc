; Compare the 53 named routines and the previously covered adjacent helpers.
; Define KERNEL_RVA_BIAS, KERNEL_FILE_SIZE, KERNEL_CONTEXT and KERNEL_FAILURE.
; R12 points to the reference image. The context has room for a seventh qword,
; used here to retain the table cursor across calls to the comparator.

%assign kernel_symbol_index 0
%macro KERNEL_SYMBOL 1
kernel_symbol_id_%1 equ kernel_symbol_index
%assign kernel_symbol_index kernel_symbol_index + 1
%endmacro
%include 'native/kernel_symbol_table.inc'
%unmacro KERNEL_SYMBOL 1

; A compact table of symbol identities replaces a separate copy of the same
; comparison code for each range. Both images use the same symbol ordering.
    jmp .kernel_ranges_begin
.kernel_ranges_table:
%assign kernel_range_count 0
%macro KERNEL_COMPARE_ROUTINE 2
    dw kernel_symbol_id_%1, kernel_symbol_id_%2
%assign kernel_range_count kernel_range_count + 1
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
KERNEL_COMPARE_ROUTINE lp_fill_x, lp_dgetrf
KERNEL_COMPARE_ROUTINE lp_laswp_tail, lp_laswp_cols
KERNEL_COMPARE_ROUTINE lp_dtrsm_left, lp_dtrsm_cols
KERNEL_COMPARE_ROUTINE lp_dgemm_neg, lp_steal
KERNEL_COMPARE_ROUTINE lp_worker, lp_dgetrs
KERNEL_COMPARE_ROUTINE dual_lp_fill_x, dual_lp_dgetrf
KERNEL_COMPARE_ROUTINE dual_lp_laswp_tail, dual_lp_laswp_cols
KERNEL_COMPARE_ROUTINE dual_lp_dtrsm_left, dual_lp_dtrsm_cols
KERNEL_COMPARE_ROUTINE dual_lp_dgemm_neg, dual_lp_steal
%if kernel_range_count != 62
    %error The verifier must also check all nine adjacent helper ranges
%endif
%unmacro KERNEL_COMPARE_ROUTINE 2
.kernel_ranges_end:

.kernel_ranges_begin:
    lea r9, [KERNEL_CONTEXT]
    mov rbx, [r9+32]               ; reference symbol entries
    lea rax, [rel .kernel_ranges_table]
    mov [r9+48], rax               ; cursor survives comparator calls
.kernel_ranges_next:
    lea r9, [KERNEL_CONTEXT]
    mov r10, [r9+48]
    lea rax, [rel .kernel_ranges_end]
    cmp r10, rax
    je .kernel_ranges_done
    movzx eax, word [r10]
    movzx edx, word [r10+2]
    mov r8, [r9+24]                ; loaded-image symbol entries
    mov esi, [r8+rax*4]
    mov edi, [r8+rdx*4]
    add rsi, [r9]
    add rdi, [r9]
    mov eax, [rbx+rax*4]
    mov ecx, [rbx+rdx*4]
    cmp rcx, rax
    jbe KERNEL_FAILURE
    sub rcx, rax                   ; reference routine length
    sub rax, KERNEL_RVA_BIAS       ; reference RVA to file offset
    jc KERNEL_FAILURE
    lea rdx, [rax+rcx]
    cmp rdx, KERNEL_FILE_SIZE
    ja KERNEL_FAILURE
    lea rdx, [r12+rax]
    add rcx, rdx                   ; reference end
    lea r8, [rel kernel_resolve_symbol]
    call kcmp_compare_ranges
    test eax, eax
    jz KERNEL_FAILURE
    lea r9, [KERNEL_CONTEXT]
    add qword [r9+48], 4
    jmp .kernel_ranges_next
.kernel_ranges_done:
