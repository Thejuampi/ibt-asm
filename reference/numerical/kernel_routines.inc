; Primary start/end and corresponding second-instance start/end.
; The list is the same 26-routine set as the historic semantic baseline.
KERNEL_ROUTINE lp_fill_cols, lp_fill_ones, dual_lp_fill_cols, dual_lp_fill_ones
KERNEL_ROUTINE lp_fill_ones, lp_fill_x, dual_lp_fill_ones, dual_lp_fill_x
KERNEL_ROUTINE lp_dgetrf, lp_scale_col, dual_lp_dgetrf, dual_lp_scale_col
KERNEL_ROUTINE lp_scale_col, lp_daxpy_neg, dual_lp_scale_col, dual_lp_daxpy_neg
KERNEL_ROUTINE lp_daxpy_neg, lp_daxpy4_neg, dual_lp_daxpy_neg, dual_lp_daxpy4_neg
KERNEL_ROUTINE lp_daxpy4_neg, lp_panel, dual_lp_daxpy4_neg, dual_lp_panel
KERNEL_ROUTINE lp_panel, lp_laswp_tail, dual_lp_panel, dual_lp_laswp_tail
KERNEL_ROUTINE lp_laswp_cols, lp_dgetf2, dual_lp_laswp_cols, dual_lp_dgetf2
KERNEL_ROUTINE lp_dgetf2, lp_pivot, dual_lp_dgetf2, dual_lp_pivot
KERNEL_ROUTINE lp_pivot, lp_swap_rows, dual_lp_pivot, dual_lp_swap_rows
KERNEL_ROUTINE lp_swap_rows, lp_dtrsm_left, dual_lp_swap_rows, dual_lp_dtrsm_left
KERNEL_ROUTINE lp_dtrsm_cols, lp_dgemm_update, dual_lp_dtrsm_cols, dual_lp_dgemm_update
KERNEL_ROUTINE lp_dgemm_update, lp_dgemm_neg, dual_lp_dgemm_update, dual_lp_dgemm_neg
KERNEL_ROUTINE lp_steal, lp_job_run, dual_lp_steal, dual_lp_job_run
KERNEL_ROUTINE lp_job_run, lp_worker, dual_lp_job_run, dual_lp_dgetrs
KERNEL_ROUTINE lp_dgetrs, lp_resid, dual_lp_dgetrs, dual_lp_resid
KERNEL_ROUTINE lp_resid, lp_emit, dual_lp_resid, dual_lp_pack_a
KERNEL_ROUTINE lp_pack_a, lp_pack_b, dual_lp_pack_a, dual_lp_pack_b
KERNEL_ROUTINE lp_pack_b, lp_pack_b_all, dual_lp_pack_b, dual_lp_pack_b_all
KERNEL_ROUTINE lp_pack_b_all, lp_dgemm_ir, dual_lp_pack_b_all, dual_lp_dgemm_ir
KERNEL_ROUTINE lp_dgemm_ir, lp_dgemm_cols, dual_lp_dgemm_ir, dual_lp_dgemm_cols
KERNEL_ROUTINE lp_dgemm_cols, lp_k12x4, dual_lp_dgemm_cols, dual_lp_k12x4
KERNEL_ROUTINE lp_k12x4, lp_k8x4, dual_lp_k12x4, dual_lp_k8x4
KERNEL_ROUTINE lp_k8x4, lp_k8x4_avx, dual_lp_k8x4, dual_lp_k8x4_avx
KERNEL_ROUTINE lp_k8x4_avx, lp_krest, dual_lp_k8x4_avx, dual_lp_krest
KERNEL_ROUTINE lp_krest, lp_core_end, dual_lp_krest, dual_core_end
