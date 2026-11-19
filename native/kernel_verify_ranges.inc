; Compare the numerical routines and their adjacent helpers.
; Context extends the resolver's six qwords with the range cursor and the
; candidate and reference code limits, in RVAs.

%assign kernel_symbol_index 0
%macro KERNEL_SYMBOL 1-2
kernel_symbol_id_%1 equ kernel_symbol_index
%assign kernel_symbol_index kernel_symbol_index + 1
%endmacro
%include 'native/kernel_symbol_table.inc'
%unmacro KERNEL_SYMBOL 1-2
kernel_symbol_count equ kernel_symbol_index

; A compact table of symbol identities replaces a separate copy of the same
; comparison code for each range. Both images use the same symbol ordering.
    jmp .kernel_ranges_begin
.kernel_ranges_table:
%assign kernel_range_count 0
%macro KERNEL_COMPARE_ROUTINE 2-3
    dw kernel_symbol_id_%1, kernel_symbol_id_%2
    %if %0 = 3
        dw kernel_symbol_id_%3
    %else
        dw kernel_symbol_id_%2
    %endif
%assign kernel_range_count kernel_range_count + 1
%endmacro
%assign kernel_routine_count 0
%macro KERNEL_ROUTINE 4
    KERNEL_COMPARE_ROUTINE %1, %2
%assign kernel_routine_count kernel_routine_count + 1
%endmacro
%include 'reference/numerical/kernel_routines.inc'
%unmacro KERNEL_ROUTINE 4
%ifdef IBT_COMPACT_PIPELINE
KERNEL_COMPARE_ROUTINE dual_lp_fill_cols, dual_lp_fill_ones
KERNEL_COMPARE_ROUTINE dual_lp_fill_ones, dual_lp_fill_x
%assign kernel_routine_count kernel_routine_count + 2
%else
%macro KERNEL_ROUTINE 4
    KERNEL_COMPARE_ROUTINE %3, %4
%assign kernel_routine_count kernel_routine_count + 1
%endmacro
%include 'reference/numerical/kernel_routines.inc'
%unmacro KERNEL_ROUTINE 4
%endif
KERNEL_COMPARE_ROUTINE verify_lp_resid, verify_core_end
%assign kernel_routine_count kernel_routine_count + 1
%ifdef IBT_COMPACT_PIPELINE
%if kernel_routine_count != 29
    %error Expected 29 routines with one shared solver
%endif
%else
%if kernel_routine_count != 53
    %error The verifier must check all 53 numerical routines
%endif
%endif
KERNEL_COMPARE_ROUTINE lp_fill_x, lp_dgetrf
KERNEL_COMPARE_ROUTINE lp_laswp_tail, lp_laswp_cols
KERNEL_COMPARE_ROUTINE lp_dtrsm_left, lp_dtrsm_cols
KERNEL_COMPARE_ROUTINE lp_dgemm_neg, lp_steal
KERNEL_COMPARE_ROUTINE lp_worker, lp_dgetrs
%ifdef IBT_COMPACT_PIPELINE
KERNEL_COMPARE_ROUTINE dual_lp_fill_x, dual_lp_dgetrf, dual_core_end
KERNEL_COMPARE_ROUTINE dual_lp_fill_a, dual_lp_fill_cols
%if kernel_range_count != 36
    %error The shared solver verifier must also check seven helper ranges
%endif
%else
KERNEL_COMPARE_ROUTINE dual_lp_fill_x, dual_lp_dgetrf
KERNEL_COMPARE_ROUTINE dual_lp_laswp_tail, dual_lp_laswp_cols
KERNEL_COMPARE_ROUTINE dual_lp_dtrsm_left, dual_lp_dtrsm_cols
KERNEL_COMPARE_ROUTINE dual_lp_dgemm_neg, dual_lp_steal
%if kernel_range_count != 62
    %error The verifier must also check all nine adjacent helper ranges
%endif
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
    movzx r11d, word [r10+4]
    mov r8, [r9+24]                ; loaded-image symbol entries
    mov esi, [r8+rax*4]
    mov edi, [r8+r11*4]
    cmp esi, KERNEL_CODE_START
    jb KERNEL_INVALID
    cmp edi, esi
    jbe KERNEL_INVALID
    cmp rdi, [r9+56]
    ja KERNEL_INVALID
    add rsi, [r9]
    add rdi, [r9]
    mov ecx, [rbx+rdx*4]
    mov edx, [rbx+rax*4]
    cmp edx, KERNEL_CODE_START
    jb KERNEL_INVALID
    cmp ecx, edx
    jbe KERNEL_INVALID
    cmp rcx, [r9+64]
    ja KERNEL_INVALID
    sub rdx, [r9+16]
    sub rcx, [r9+16]
    add rdx, [r9+8]
    add rcx, [r9+8]
    lea r8, [rel kernel_resolve_symbol]
    call kcmp_compare_ranges
    test eax, eax
    jz KERNEL_DIFFERENT
    lea r9, [KERNEL_CONTEXT]
    add qword [r9+48], 6
    jmp .kernel_ranges_next
.kernel_ranges_done:
