; Up to three isolated instances of the same numerical engine. The pool executes
; only real matrix jobs; each coordinator solves and checks its own full system.
; No reduction is split across threads, so arithmetic order stays unchanged.
%include "ibt_dual_state.inc"
%include "ibt_third_state.inc"
%ifdef CONT_ASYNC_RESID
%include "ibt_verify_state.inc"
%endif

C_N       equ 0
C_NTHR    equ 8
C_A       equ 16
C_X       equ 24
C_RX      equ 32
C_TIME    equ 40
C_R       equ 48
C_NORM    equ 56
C_ALLOC   equ 64
C_FILL    equ 72
C_FACTOR  equ 80
C_SOLVE   equ 88
C_RESID   equ 96
C_FREE    equ 104
C_NEXT    equ 112
C_JOB     equ 120
C_GM      equ 128
C_GN      equ 136
C_RUN     equ 144
C_STEAL   equ 152
C_GB      equ 160
C_GC      equ 168
C_GLDA    equ 176
C_PACKALL equ 184
C_PAR     equ 192
C_BV      equ 200
C_DESC_BYTES equ 208
CONT_QUEUE_SIZE equ 32
%ifndef CONT_CHUNK_COLS
%define CONT_CHUNK_COLS NC
%endif
%ifndef CONT_ROW_CHUNK
%define CONT_ROW_CHUNK 12
%endif
%ifndef CONT_TILE_COLS
%define CONT_TILE_COLS 192
%endif

section .bss
alignb 64
contActive       resd 1
alignb 64
contGang         resd 1
contLaunch       resd 3
alignb 64
contGate         resd 1
contDie          resd 1
alignb 64
contReady        resd 3
alignb 64
contBusy         resd 3
contPoolCount    resd 1
contIssued       resd 1
contVerified     resd 1
contStopRequested resd 1
contAsyncActive  resd 1
contVerifier     resq 1
contHaveRef      resd 1
contQueueHead    resd 1
contQueueTail    resd 1
contQueueCount   resd 1
contFixedN       resd 1 ; test/benchmark override; production uses the MB budget
contWarmup       resd 1 ; benchmark warm-up, outside the reported wall interval
contThreads      resd 1
contContexts     resd 1
contDisplayN     resd 1
alignb 8
contRefR         resq 1
contRefNorm      resq 1
contStart        resq 1
contEnd          resq 1
contAllocated    resq 1
contSecond       resq 2
contHandles      resq 30
contQueue        resb CONT_QUEUE_SIZE*128

section .data
align 8
; Native PE builds attach a base relocation to each non-null context pointer.
%ifndef PE_NATIVE
%macro CONT_PTR 1-*
    %rep %0
        dq %1
        %rotate 1
    %endrep
%endmacro
%endif
contA:
    CONT_PTR lpN, lpNthr, lpA, lpX, lpResidX, lpTmpT, lpTmpR, lpTmpN
    CONT_PTR lp_alloc_n, lp_fill, lp_dgetrf, lp_dgetrs, lp_resid, lp_free
    CONT_PTR lpNext, lpJob, lpGM, lpGN, lp_job_run, lp_steal
    CONT_PTR lpGB, lpGC, lpGLDA, lp_pack_b_all, lp_par
    CONT_PTR lpB
contB:
    CONT_PTR dual_lpN, dual_lpNthr, dual_lpA, dual_lpX, dual_lpResidX
    CONT_PTR dual_lpTmpT, dual_lpTmpR, dual_lpTmpN
    CONT_PTR dual_lp_alloc_n, dual_lp_fill, dual_lp_dgetrf, dual_lp_dgetrs
    CONT_PTR dual_lp_resid, dual_lp_free
    CONT_PTR dual_lpNext, dual_lpJob, dual_lpGM, dual_lpGN, dual_lp_job_run, dual_lp_steal
    CONT_PTR dual_lpGB, dual_lpGC, dual_lpGLDA, dual_lp_pack_b_all, dual_lp_par
    CONT_PTR dual_lpB
%ifndef CONT_PIPELINE
contC:
    CONT_PTR third_lpN, third_lpNthr, third_lpA, third_lpX, third_lpResidX
    CONT_PTR third_lpTmpT, third_lpTmpR, third_lpTmpN
%ifdef CONT_ASYNC_RESID
%ifndef CONT_THREE_ASYNC
    CONT_PTR 0, 0, 0, 0
    CONT_PTR third_lp_resid, 0
    CONT_PTR third_lpNext, third_lpJob, third_lpGM, third_lpGN, 0, 0
    CONT_PTR third_lpGB, third_lpGC, third_lpGLDA, 0, 0
%else
    CONT_PTR third_lp_alloc_n, third_lp_fill, third_lp_dgetrf, third_lp_dgetrs
    CONT_PTR third_lp_resid, third_lp_free
    CONT_PTR third_lpNext, third_lpJob, third_lpGM, third_lpGN, third_lp_job_run, third_lp_steal
    CONT_PTR third_lpGB, third_lpGC, third_lpGLDA, third_lp_pack_b_all, third_lp_par
%endif
%else
    CONT_PTR third_lp_alloc_n, third_lp_fill, third_lp_dgetrf, third_lp_dgetrs
    CONT_PTR third_lp_resid, third_lp_free
    CONT_PTR third_lpNext, third_lpJob, third_lpGM, third_lpGN, third_lp_job_run, third_lp_steal
    CONT_PTR third_lpGB, third_lpGC, third_lpGLDA, third_lp_pack_b_all, third_lp_par
%endif
    CONT_PTR third_lpB
%endif
%ifdef CONT_ASYNC_RESID
contV:
    CONT_PTR verify_lpN, verify_lpNthr, verify_lpA, verify_lpX, verify_lpResidX
    CONT_PTR verify_lpTmpT, verify_lpTmpR, verify_lpTmpN
    CONT_PTR 0, 0, 0, 0, verify_lp_resid, 0
    CONT_PTR 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, verify_lpB
%endif

section .text
cont_lock:
    mov eax, 1
    xchg eax, [contGate]
    test eax, eax
    jz .got
.wait:
    pause
    cmp dword [contGate], 0
    jne .wait
    jmp cont_lock
.got:
    ret

cont_par_a:
    lea r10, [contA]
    xor r11d, r11d
    jmp cont_par
cont_par_b:
    lea r10, [contB]
    mov r11d, 1
    jmp cont_par
%ifndef CONT_PIPELINE
cont_par_c:
    lea r10, [contC]
    mov r11d, 2
%endif
cont_par:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    sub rsp, 20h
    mov rbx, r10
    mov esi, r11d
    call cont_lock
    mov rax, [rbx+C_NEXT]
    mov dword [rax], 0
    lea rax, [contReady]
    mov dword [rax+rsi*4], 1
%ifdef CONT_EARLY_STAGGER
    cmp qword [contStart], 0
    je .no_early_launch
    cmp esi, 1
    ja .no_early_launch
    mov rax, [rbx+C_GM]
    mov ecx, [rax]
    mov rax, [rbx+C_GN]
    cmp ecx, [rax]
    jne .no_early_launch
    mov rax, [rbx+C_N]
    mov edx, [rax]
    mov eax, edx
    shr eax, 2
    sub edx, eax
    cmp ecx, edx
    ja .no_early_launch
    lea rax, [contLaunch+4]
    mov dword [rax+rsi*4], 1
.no_early_launch:
%endif
    mov dword [contGate], 0
%ifdef CONT_PIPELINE
    mov rax, [rbx+C_JOB]
    cmp dword [rax], 5
    je .pipeline_fill_wait
%endif
%ifdef CONT_GANG
    mov rax, [rbx+C_JOB]
    cmp dword [rax], 0
    je .wait
    cmp dword [rax], 2
    je .wait
%ifdef CONT_ASYNC_RESID
    cmp dword [rax], 5
    je .wait
%endif
%endif
    ; Never delay this context's next panel by executing another context's
    ; longer chunk. Only the shared workers switch between matrix jobs.
    call [rbx+C_STEAL]
.wait:
%ifdef CONT_GANG
    call cont_lock
    mov rax, [rbx+C_JOB]
    mov rdx, [rbx+C_GN]
    cmp dword [rax], 2
    jne .gang_limit
    mov rdx, [rbx+C_GM]
.gang_limit:
    mov ecx, [rdx]
    mov rax, [rbx+C_NEXT]
    cmp [rax], ecx
    jb .gang_help
    lea rax, [contBusy]
    cmp dword [rax+rsi*4], 0
    je .done
.gang_help:
    mov dword [contGate], 0
    call cont_help
    pause
    jmp .wait
%else
    lea rax, [contBusy]
    cmp dword [rax+rsi*4], 0
    je .try_done
    pause
    jmp .wait
.try_done:
    call cont_lock
    lea rax, [contBusy]
    cmp dword [rax+rsi*4], 0
    je .done
    mov dword [contGate], 0
    jmp .wait
%endif
%ifdef CONT_PIPELINE
.pipeline_fill_wait:
    call cont_lock
    mov rax, [rbx+C_GN]
    mov ecx, [rax]
    mov rax, [rbx+C_NEXT]
    cmp [rax], ecx
    jb .pipeline_fill_help
    lea rax, [contBusy]
    cmp dword [rax+rsi*4], 0
    je .done
.pipeline_fill_help:
    mov dword [contGate], 0
    call cont_help
    pause
    jmp .pipeline_fill_wait
%endif
.done:
%ifdef CONT_GANG
    cmp [contGang], esi
    jne .clear_ready
    mov dword [contGang], -1
.clear_ready:
%endif
    lea rax, [contReady]
    mov dword [rax+rsi*4], 0
    mov dword [contGate], 0
    add rsp, 20h
    pop rsi
    pop rbx
    pop rbp
    ret

cont_worker:
    xor r10d, r10d
    jmp cont_worker_common
cont_tail_worker:
    mov r10d, 1
cont_worker_common:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    sub rsp, 28h
    mov r13d, r10d
%ifdef CONT_BIND_TEST
    test r13d, r13d
    jnz .binding_done
    call cont_bind_pool
.binding_done:
%endif
    call lp_pack_ensure
    test rax, rax
    jnz .scratch
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
.scratch:
.loop:
    test r13d, r13d
    jz .pool_worker
    cmp dword [lpStop], 0
    jne .exit
    mov eax, [contVerified]
    cmp eax, [targetRuns]
    jae .exit
.pool_worker:
    cmp dword [contDie], 0
    jne .exit
    ; An empty queue is a wait, never a source of artificial lock traffic.
    call cont_peek
    test eax, eax
    jz .idle
.available:
    call cont_help
    jmp .loop
.idle:
    pause
    jmp .loop
.exit:
    call lp_pack_drop
    call lp_vzero
    xor eax, eax
    add rsp, 28h
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

; Read-only hint; the descriptor is rechecked under the gate before claiming.
cont_peek:
%ifdef CONT_GANG
    mov r8d, [contGang]
    cmp r8d, [contContexts]
    jae .scan
    imul r9d, r8d, C_DESC_BYTES
    lea r11, [contA]
    add r9, r11
    lea r10, [contReady]
    cmp dword [r10+r8*4], 0
    je .scan
    mov rax, [r9+C_JOB]
    mov rdx, [r9+C_GN]
    cmp dword [rax], 2
    jne .gang_limit
    mov rdx, [r9+C_GM]
.gang_limit:
    mov ecx, [rdx]
    mov rax, [r9+C_NEXT]
    cmp [rax], ecx
    jb .yes
    lea r10, [contBusy]
    cmp dword [r10+r8*4], 0
    jne .none
.scan:
%endif
    xor r8d, r8d
    lea r9, [contA]
    lea r10, [contReady]
.loop:
    cmp r8d, [contContexts]
    jae .none
    cmp dword [r10+r8*4], 0
    je .next
    mov rax, [r9+C_JOB]
    mov rdx, [r9+C_GN]
    cmp dword [rax], 2
    jne .limit
    mov rdx, [r9+C_GM]
.limit:
    mov ecx, [rdx]
    mov rax, [r9+C_NEXT]
    cmp [rax], ecx
    jb .yes
.next:
    inc r8d
    add r9, C_DESC_BYTES
    jmp .loop
.yes:
    mov eax, 1
    ret
.none:
    xor eax, eax
    ret

; One original NC-column or 12-row chunk. All claimants hold contGate.
; Prefer A so B keeps real work queued for A's panel/solve/residual phases.
; A descriptor remains immutable until every busy reference is released.
cont_help:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push r12
    sub rsp, 28h
    call cont_peek
    test eax, eax
    jz .out
    call cont_lock
%ifdef CONT_ASYNC_RESID
    mov dword [rsp+20h], 2
%else
    mov dword [rsp+20h], 1
%endif
%ifdef CONT_GANG
    mov esi, [contGang]
    cmp esi, [contContexts]
    jae .pass
    imul eax, esi, C_DESC_BYTES
    lea rbx, [contA]
    add rbx, rax
    lea rax, [contReady]
    cmp dword [rax+rsi*4], 0
    je .release_gang
    mov rax, [rbx+C_JOB]
    mov rdx, [rbx+C_GN]
    cmp dword [rax], 2
    jne .gang_limit
    mov rdx, [rbx+C_GM]
.gang_limit:
    mov ecx, [rdx]
    mov rax, [rbx+C_NEXT]
    cmp [rax], ecx
    jb .active_gang
    lea rax, [contBusy]
    cmp dword [rax+rsi*4], 0
    jne .empty
.release_gang:
    mov dword [contGang], -1
    jmp .pass
.active_gang:
    mov r12d, 1
    mov dword [rsp+20h], 0
    jmp .chosen
%endif
.pass:
    xor esi, esi
    mov r12d, [contContexts]
.choose:
    imul eax, esi, C_DESC_BYTES
    lea rbx, [contA]
    add rbx, rax
.chosen:
    lea rax, [contReady]
    cmp dword [rax+rsi*4], 0
    je .next
    mov rax, [rbx+C_JOB]
    cmp dword [rsp+20h], 0
    je .job
    cmp dword [rsp+20h], 2
    jne .heavy
    cmp dword [rax], 5
    je .job
    jmp .next
.heavy:
    cmp dword [rax], 0
    je .job
    cmp dword [rax], 2
    jne .next
.job:
    mov rdx, [rbx+C_GN]
    mov r8d, NC
    cmp dword [rax], 0
    jne .not_columns
    mov r8d, CONT_CHUNK_COLS
.not_columns:
    cmp dword [rax], 2
    jne .limit
    mov rdx, [rbx+C_GM]
    mov r8d, CONT_ROW_CHUNK
.limit:
    mov r9d, [rdx]
    mov rdx, [rbx+C_NEXT]
    cmp [rdx], r9d
    jae .next
%ifdef CONT_GANG
    mov rax, [rbx+C_JOB]
    cmp dword [rax], 0
    je .set_gang
    cmp dword [rax], 2
    je .set_gang
%ifdef CONT_ASYNC_RESID
    cmp dword [rax], 5
    je .set_gang
%endif
    jmp .claim
.set_gang:
    mov [contGang], esi
.claim:
%endif
    ; Non-DGEMM coordinators also claim through the original lp_steal.
    mov eax, r8d
    lock xadd dword [rdx], eax
    cmp eax, r9d
    jae .next
    mov ecx, eax
    add eax, r8d
    cmp eax, r9d
    cmova eax, r9d
    mov edx, eax
    lea rax, [contBusy]
    lock inc dword [rax+rsi*4]
    mov dword [contGate], 0
    call [rbx+C_RUN]
    lea rax, [contBusy]
    lock dec dword [rax+rsi*4]
    jmp .out
.next:
    inc esi
    dec r12d
    jnz .choose
    cmp dword [rsp+20h], 0
    je .empty
    dec dword [rsp+20h]
    jmp .pass
.empty:
    mov dword [contGate], 0
.out:
    add rsp, 28h
    pop r12
    pop rsi
    pop rbx
    pop rbp
    ret

%ifdef CONT_GEMM_TILED
; Reuse the original packed row dispatcher on NC-column slices. Arithmetic
; kernels stay unchanged; only job publication and the B/C view are tiled.
; R10 = immutable context descriptor; packed A is already prepared.
cont_gemm_tiled:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 28h
    mov rbx, r10
    mov rax, [rbx+C_GB]
    mov r12, [rax]
    mov rax, [rbx+C_GC]
    mov r13, [rax]
    mov rax, [rbx+C_GN]
    mov r14d, [rax]
    mov rax, [rbx+C_GLDA]
    mov r15, [rax]
    shl r15, 3
    xor esi, esi
.tile:
    cmp esi, r14d
    jae .restore
    cmp dword [lpStop], 0
    jne .restore
    mov eax, esi
    imul rax, r15
    lea rdx, [r12+rax]
    mov rcx, [rbx+C_GB]
    mov [rcx], rdx
    add rax, r13
    mov rcx, [rbx+C_GC]
    mov [rcx], rax
    mov eax, r14d
    sub eax, esi
    cmp eax, CONT_TILE_COLS
    jbe .width
    mov eax, CONT_TILE_COLS
.width:
    mov rcx, [rbx+C_GN]
    mov [rcx], eax
    call [rbx+C_PACKALL]
    mov rax, [rbx+C_JOB]
    mov dword [rax], 2
    call [rbx+C_PAR]
    add esi, CONT_TILE_COLS
    jmp .tile
.restore:
    mov rax, [rbx+C_GB]
    mov [rax], r12
    mov rax, [rbx+C_GC]
    mov [rax], r13
    mov rax, [rbx+C_GN]
    mov [rax], r14d
    add rsp, 28h
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
%endif

; Both matrix sets, packed panels and per-compute-thread scratch, in bytes.
; ECX = n, EDX = total selected compute threads; all arithmetic is 64 bit.
cont_memory_bytes:
    mov r8d, ecx
    lea eax, [rcx+8]
    imul rax, r8
    shl rax, 3
    add rax, 4095
    and rax, -4096
    lea r9, [r8*8+64+4095]
    and r9, -4096
    imul r9, 3
    add rax, r9
    lea r9, [r8*4+64+4095]
    and r9, -4096
    add rax, r9
    lea r9, [r8+16]
    imul r9, NB*8
    add r9, 4095
    and r9, -4096
    lea rax, [rax+r9*2]
    mov r9d, [contContexts]
    imul rax, r9
    add rax, CONT_QUEUE_SIZE*128
    imul rdx, (((NC/4)*BPG+APS+256+4095)&~4095)
    add rax, rdx
%ifdef CONT_ASYNC_RESID
    cmp dword [contAsyncActive], 0
    je .memory_done
    lea r9, [r8*8+64+4095]
    and r9, -4096
    add rax, r9
    mov r9, r8
    shl r9, 4
    add r9, 16+63
    and r9, -64
    shl r9, 2
    add r9, 4095
    and r9, -4096
    add rax, r9
.memory_done:
%endif
    ret

; Same nominal-matrix-MB to N conversion as v3.1. Extra buffers add RAM;
; enabling overlap must never silently reduce the dimension of a preset.
cont_dimension:
    sub rsp, 28h
    mov ecx, [stressMB]
    shl rcx, 17
    call isqrt
    and eax, -8
    cmp eax, 8
    jae .out
    mov eax, 8
.out:
    add rsp, 28h
    ret

; Idle configuration/memory planning shared by the desktop frontends.
cont_configure:
    sub rsp, 28h
    call thread_count
    cmp eax, 32
    jbe .threads
    mov eax, 32
.threads:
    mov [contThreads], eax
    mov dword [contContexts], 1
    mov dword [contAsyncActive], 0
    cmp dword [overlapRuns], 0
    je .out
    cmp eax, 3
    jb .out
    mov dword [contContexts], 2
    mov dword [contAsyncActive], 1
.out:
    add rsp, 28h
    ret

cont_plan_bytes:
    sub rsp, 28h
    call cont_configure
    call cont_dimension
    mov ecx, eax
    mov edx, [contThreads]
    call cont_memory_bytes
    add rsp, 28h
    ret

; EAX = available MB budget. Return a nominal per-problem MB value that fits
; all matrix/packed/vector/thread/validation buffers (Maximum mode only).
cont_max_mb:
    push rbx
    push rsi
    sub rsp, 28h
    mov ebx, eax
    mov esi, [stressMB]
    call cont_configure
    mov [stressMB], ebx
    call cont_budget_dimension
    mov [stressMB], esi
    imul rax, rax
    shr rax, 17
    cmp eax, 8
    jae .out
    mov eax, 8
.out:
    add rsp, 28h
    pop rsi
    pop rbx
    ret

; Pick the greatest multiple of eight that fits the complete MB budget.
cont_budget_dimension:
    push rbx
    push rsi
    push rdi
    mov ebx, [stressMB]
    shl rbx, 20
    mov esi, 1
    mov edi, 262144
.search:
    cmp esi, edi
    jae .found
    lea ecx, [rsi+rdi+1]
    shr ecx, 1
    mov r11d, ecx
    shl ecx, 3
    mov edx, [contThreads]
    call cont_memory_bytes
    cmp rax, rbx
    ja .smaller
    mov esi, r11d
    jmp .search
.smaller:
    lea edi, [r11d-1]
    jmp .search
.found:
    mov ecx, esi
    shl ecx, 3
    mov edx, [contThreads]
    call cont_memory_bytes
    cmp rax, rbx
    jbe .ok
    xor esi, esi
.ok:
    mov eax, esi
    shl eax, 3
    pop rdi
    pop rsi
    pop rbx
    ret

cont_next:
    sub rsp, 28h
    call cont_lock
    mov eax, -1
    cmp dword [lpStop], 0
    jne .out
    mov edx, [contIssued]
    cmp edx, [targetRuns]
    jae .out
    mov eax, edx
    inc dword [contIssued]
.out:
    mov dword [contGate], 0
    add rsp, 28h
    ret

; ECX is not needed: descriptor is in RBX, owned exclusively by this coordinator.
cont_check:
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    mov rax, [rbx+C_NORM]
    movsd xmm0, [rax]
    ucomisd xmm0, [lpC0]
    jp .bad
    jb .bad
    ucomisd xmm0, [lpC16]
    ja .bad
    mov rax, [rbx+C_R]
    mov rdx, [rax]
    mov rcx, rdx
    shr rcx, 52
    and ecx, 7ffh
    cmp ecx, 7ffh
    je .bad
    call cont_lock
    mov rax, [rbx+C_R]
    mov rdx, [rax]
    mov rax, [rbx+C_NORM]
    mov rcx, [rax]
    cmp dword [contHaveRef], 0
    je .reference
    cmp rdx, [contRefR]
    jne .bad_locked
    cmp rcx, [contRefNorm]
    jne .bad_locked
    jmp .good
.reference:
    mov [contRefR], rdx
    mov [contRefNorm], rcx
    mov dword [contHaveRef], 1
.good:
    mov dword [contGate], 0
    xor eax, eax
    leave
    ret
.bad_locked:
    mov dword [contGate], 0
.bad:
    mov dword [failed], 1
    mov dword [lpStop], 1
    mov eax, 1
    leave
    ret

cont_publish:
    push rbp
    mov rbp, rsp
    push rsi
    push rdi
    sub rsp, 30h
.wait:
    call cont_lock
    cmp dword [contQueueCount], CONT_QUEUE_SIZE
    jb .space
    mov dword [contGate], 0
    cmp dword [lpStop], 0
    jne .out
    pause
    jmp .wait
.space:
    mov esi, [contQueueTail]
    shl esi, 7
    lea rdi, [contQueue]
    add rdi, rsi
    mov rsi, rdi
    mov rax, [rbx+C_TIME]
    movsd xmm0, [rax]
    mov ecx, 3
    call lp_ftoa
    mov byte [rdi], 0
    lea rcx, [rsp+20h]
    call QueryPerformanceCounter
    mov rax, [rsp+20h]
    mov [contEnd], rax
    inc dword [contVerified]
    mov rax, [rbx+C_TIME]
    movsd xmm2, [rax]
    mov rax, [rbx+C_N]
    mov eax, [rax]
    cvtsi2sd xmm0, rax
    movsd xmm1, xmm0
    mulsd xmm0, xmm0
    mulsd xmm0, xmm1
    mulsd xmm0, [lpTwo3]
    divsd xmm0, xmm2
    divsd xmm0, [lp1e9]
    lea rdi, [rsi+32]
    mov ecx, 4
    call lp_ftoa
    mov byte [rdi], 0
    mov rax, [rbx+C_NORM]
    movsd xmm0, [rax]
    lea rdi, [rsi+64]
    call lp_sci
    mov byte [rdi], 0
    inc dword [contQueueTail]
    and dword [contQueueTail], CONT_QUEUE_SIZE-1
    inc dword [contQueueCount]
    mov dword [contGate], 0
.out:
    add rsp, 30h
    pop rdi
    pop rsi
    pop rbp
    ret

; Called only by the platform's UI/main thread. Copy before releasing the slot.
cont_drain:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    and rsp, -16
    sub rsp, 0a0h
.next:
    call cont_lock
    cmp dword [contQueueCount], 0
    je .empty
    mov eax, [contQueueHead]
    shl eax, 7
    lea rsi, [contQueue]
    add rsi, rax
    lea rdi, [rsp+20h]
    mov ecx, 16
    rep movsq
    inc dword [contQueueHead]
    and dword [contQueueHead], CONT_QUEUE_SIZE-1
    dec dword [contQueueCount]
    mov dword [contGate], 0
    lea rcx, [rsp+20h]
    call on_result_ptr
    jmp .next
.empty:
    mov dword [contGate], 0
    lea rsp, [rbp-24]
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

cont_run:
%ifdef CONT_PIPELINE
    jmp cont_pipeline_run
%else
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    and rsp, -16
    sub rsp, 40h
    mov rbx, rcx
%ifdef CONT_BIND_TEST
    call cont_bind_coordinator
%endif
    call lp_pack_ensure
    test rax, rax
    jz .alloc_bad
.loop:
    call cont_next
    cmp eax, -1
    je .tail
    call [rbx+C_FILL]
    cmp dword [lpStop], 0
    jne .out
    lea rcx, [rsp+20h]
    call QueryPerformanceCounter
    mov r13, [rsp+20h]
    call [rbx+C_FACTOR]
    cmp dword [lpStop], 0
    jne .out
%ifdef CONT_STAGGER
    lea rax, [contA]
    cmp rbx, rax
    jne .launch_c
    mov dword [contLaunch+4], 1
.launch_c:
    lea rax, [contB]
    cmp rbx, rax
    jne .launched
    mov dword [contLaunch+8], 1
.launched:
%endif
    call [rbx+C_SOLVE]
    lea rcx, [rsp+20h]
    call QueryPerformanceCounter
    mov rax, [rsp+20h]
    sub rax, r13
    cvtsi2sd xmm0, rax
    cvtsi2sd xmm1, qword [lpFreq]
    divsd xmm0, xmm1
    mov rax, [rbx+C_TIME]
    movsd [rax], xmm0
    mov rax, [rbx+C_X]
    mov rdx, [rax]
    mov rax, [rbx+C_RX]
    mov [rax], rdx
%ifdef CONT_TEST
    call cont_inject
%endif
%ifdef CONT_ASYNC_RESID
    cmp dword [contAsyncActive], 0
    je .inline_residual
    call cont_enqueue_validation
    jmp .loop
.inline_residual:
%endif
    call [rbx+C_RESID]
%ifdef CONT_TEST
    call cont_inject_residual
%endif
    call cont_check
    test eax, eax
    jnz .out
    cmp dword [lpStop], 0
    jne .out
    call cont_publish
    jmp .loop
.tail:
    call cont_tail_worker
    jmp .out
.alloc_bad:
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
.out:
    lea rsp, [rbp-40]
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    xor eax, eax
    ret

%endif
cont_second:
%ifdef CONT_PIPELINE
    jmp cont_pipeline_prepare
%else
    push rbx
    push rsi
    sub rsp, 28h
    mov rbx, rcx
    mov rcx, -2
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .priority
    mov edx, THR_TIMECRIT
.priority:
    call SetThreadPriority
%ifdef CONT_STAGGER
    call lp_pack_ensure
    test rax, rax
    jnz .scratch
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
    jmp .drop
.scratch:
    mov rax, rbx
    lea rdx, [contA]
    sub rax, rdx
    xor edx, edx
    mov ecx, C_DESC_BYTES
    div rcx
    mov esi, eax
.launch_wait:
    cmp dword [lpStop], 0
    jne .drop
    mov eax, [contVerified]
    cmp eax, [targetRuns]
    jae .drop
    lea rax, [contLaunch]
    cmp dword [rax+rsi*4], 0
    jne .launch
    call cont_help
    pause
    jmp .launch_wait
.launch:
%endif
    mov rcx, rbx
    call cont_run
.drop:
    call lp_pack_drop
    call lp_vzero
    add rsp, 28h
    pop rsi
    pop rbx
    xor eax, eax
    ret

%endif
cont_burn_thread:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    and rsp, -16
    sub rsp, 80h
    mov dword [contQueueHead], 0
    mov dword [contQueueTail], 0
    mov dword [contQueueCount], 0
    mov dword [contDisplayN], 0
    mov dword [contVerified], 0
    mov dword [contAsyncActive], 0
%ifdef CONT_BIND_TEST
    mov dword [contBindIndex], 0
%endif
    mov qword [contVerifier], 0
    cmp dword [overlapRuns], 0
    je .single
    call thread_count
%ifdef CONT_PIPELINE
    cmp eax, 3
    jb .single
%endif
    cmp eax, 2
    jb .single
    cmp dword [targetRuns], 2
    jb .single
    cmp eax, 32
    jbe .threads
    mov eax, 32
.threads:
%ifdef CONT_ASYNC_RESID
%ifdef CONT_THREE_ASYNC
    mov ecx, 3
    cmp eax, 3
    jb .async_contexts
    lea edx, [eax-1]
    cmp ecx, edx
    cmova ecx, edx
.async_contexts:
%else
    mov ecx, 2
%endif
%else
    mov ecx, 3
%endif
    cmp ecx, eax
    cmova ecx, eax
    cmp ecx, [targetRuns]
    cmova ecx, [targetRuns]
    mov [contContexts], ecx
    mov [contThreads], eax
%ifdef CONT_ASYNC_RESID
    cmp eax, 3
    jb .async_ready
    mov dword [contAsyncActive], 1
.async_ready:
%endif
    mov qword [contStart], 0
    mov dword [contGang], -1
    mov qword [contLaunch], 0
    mov dword [contLaunch+8], 0
    mov dword [contLaunch], 1
    xor edx, edx
    mov [contDie], edx
    mov [contGate], edx
    mov [contPoolCount], edx
    mov [contIssued], edx
    mov [contVerified], edx
    mov [contHaveRef], edx
    mov [contQueueHead], edx
    mov [contQueueTail], edx
    mov [contQueueCount], edx
    mov [contReady], rdx
    mov [contReady+8], edx
    mov [contBusy], rdx
    mov [contBusy+8], edx
    mov [contSecond], rdx
    mov [contSecond+8], rdx
    mov [contEnd], rdx
    mov rcx, -2
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .priority
    mov edx, THR_TIMECRIT
.priority:
    call SetThreadPriority
    lea rcx, [lpFreq]
    call QueryPerformanceFrequency
    mov eax, [contFixedN]
    test eax, eax
    jnz .dimension
    call cont_dimension
.dimension:
    cmp eax, 8
    jb .alloc_bad
    mov [lpN], eax
    mov [dual_lpN], eax
    mov [third_lpN], eax
    mov [contDisplayN], eax
    mov ecx, eax
    mov edx, [contThreads]
    call cont_memory_bytes
    mov [contAllocated], rax
    mov eax, [contThreads]
    mov [lpNthr], eax
    mov [dual_lpNthr], eax
    mov [third_lpNthr], eax
%ifdef CONT_TEST
    cmp dword [contFailAllocation], 1
    je .alloc_bad
%endif
    call lp_alloc_n
    test eax, eax
    jz .alloc_bad
%ifdef CONT_TEST
    cmp dword [contFailAllocation], 2
    je .alloc_bad
%endif
    call dual_lp_alloc_n
    test eax, eax
    jz .alloc_bad
%ifdef CONT_ASYNC_RESID
    cmp dword [contAsyncActive], 0
    je .allocated
%ifdef CONT_TEST
    cmp dword [contFailAllocation], 3
    je .alloc_bad
%endif
    call cont_validator_alloc
    test eax, eax
    jz .alloc_bad
%ifdef CONT_THREE_ASYNC
    cmp dword [contContexts], 3
    jb .allocated
    call third_lp_alloc_n
    test eax, eax
    jz .alloc_bad
%endif
%else
    cmp dword [contContexts], 3
    jb .allocated
%ifdef CONT_TEST
    cmp dword [contFailAllocation], 3
    je .alloc_bad
%endif
    call third_lp_alloc_n
    test eax, eax
    jz .alloc_bad
%endif
.allocated:
    mov dword [contActive], 1
    xor ebx, ebx
.pool:
    mov eax, [contThreads]
    sub eax, [contContexts]
%ifdef CONT_ASYNC_RESID
    sub eax, [contAsyncActive]
%endif
    cmp ebx, eax
    jae .start
    xor ecx, ecx
    xor edx, edx
    lea r8, [cont_worker]
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    call CreateThread
    test rax, rax
    jz .alloc_bad
    lea rdx, [contHandles]
    mov [rdx+rbx*8], rax
    inc ebx
    mov [contPoolCount], ebx
    mov rcx, rax
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .worker_priority
    mov edx, THR_TIMECRIT
.worker_priority:
    call SetThreadPriority
    jmp .pool
.start:
    cmp dword [contWarmup], 0
    je .clock
    xor r12d, r12d
.warm:
    imul eax, r12d, C_DESC_BYTES
    lea rbx, [contA]
    add rbx, rax
    call cont_warm
    test eax, eax
    jnz .finish
    inc r12d
    cmp r12d, [contContexts]
    jb .warm
.clock:
%ifdef CONT_PIPELINE
    call cont_pipeline_init
    test eax, eax
    jnz .finish
%endif
    lea rcx, [contStart]
    call QueryPerformanceCounter
    mov ebx, 1
.spawn:
    cmp ebx, [contContexts]
    jae .run
    xor ecx, ecx
    xor edx, edx
    lea r8, [cont_second]
    imul r9d, ebx, C_DESC_BYTES
    lea rax, [contA]
    add r9, rax
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    call CreateThread
    test rax, rax
    jz .alloc_bad
    lea rdx, [contSecond-8]
    mov [rdx+rbx*8], rax
    inc ebx
    jmp .spawn
.run:
%ifdef CONT_ASYNC_RESID
    cmp dword [contAsyncActive], 0
    je .validator_started
    xor ecx, ecx
    xor edx, edx
    lea r8, [cont_validator]
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    call CreateThread
    test rax, rax
    jz .alloc_bad
    mov [contVerifier], rax
.validator_started:
%endif
    mov rcx, [hMain]
    mov edx, WM_PHASE
    mov r8d, 1
    xor r9d, r9d
    call PostMessageW
    lea rcx, [contA]
    call cont_run
.finish:
    xor ebx, ebx
.join_context:
    lea rax, [contSecond]
    mov rcx, [rax+rbx*8]
    test rcx, rcx
    jz .next_context
    mov edx, INFINITE
    call WaitForSingleObject
    lea rax, [contSecond]
    mov rcx, [rax+rbx*8]
    call CloseHandle
    lea rax, [contSecond]
    mov qword [rax+rbx*8], 0
.next_context:
    inc ebx
    cmp ebx, 2
    jb .join_context
%ifdef CONT_ASYNC_RESID
    mov rcx, [contVerifier]
    test rcx, rcx
    jz .stop_pool
    mov edx, INFINITE
    call WaitForSingleObject
    mov rcx, [contVerifier]
    call CloseHandle
    mov qword [contVerifier], 0
%endif
.stop_pool:
    mov dword [contDie], 1
    xor ebx, ebx
.join:
    cmp ebx, [contPoolCount]
    jae .free
    lea rax, [contHandles]
    mov rcx, [rax+rbx*8]
    mov edx, INFINITE
    call WaitForSingleObject
    lea rax, [contHandles]
    mov rcx, [rax+rbx*8]
    call CloseHandle
    inc ebx
    jmp .join
.free:
    mov dword [contActive], 0
    cmp dword [lpStop], 0
    jne .release
    mov eax, [contVerified]
    cmp eax, [targetRuns]
    je .release
    mov dword [failed], 1
.release:
    call lp_free
    call dual_lp_free
%ifdef CONT_ASYNC_RESID
    call cont_validator_free
%ifdef CONT_THREE_ASYNC
    call third_lp_free
%endif
%else
    call third_lp_free
%endif
    call lp_vzero
    cmp dword [lpTest], 0
    jne .out
    mov rcx, [hMain]
    mov edx, WM_DONE
    xor r8d, r8d
    xor r9d, r9d
    call PostMessageW
    jmp .out
.alloc_bad:
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
    jmp .finish
.single:
    mov dword [contContexts], 1
    call legacy_burn_thread
.out:
    lea rsp, [rbp-56]
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    xor eax, eax
    ret

cont_warm:
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    call [rbx+C_FILL]
    cmp dword [lpStop], 0
    jne .stopped
    call [rbx+C_FACTOR]
    cmp dword [lpStop], 0
    jne .stopped
    call [rbx+C_SOLVE]
    mov rax, [rbx+C_X]
    mov rdx, [rax]
    mov rax, [rbx+C_RX]
    mov [rax], rdx
    call [rbx+C_RESID]
    call cont_check
    leave
    ret
.stopped:
    mov eax, 1
    leave
    ret

; Build the second instance from the very same arithmetic sources. Mutable
; state and labels are renamed; ISA selection, cancellation and TLS stay shared.
%assign CONT_SAVED_SELFTEST SELFTEST
%undef SELFTEST
%define SELFTEST 0
%define IBT_SECOND_ENGINE 1
%ifdef KERNEL_REFERENCE
%include "reference/numerical/ibt_dual_symbols.inc"
%include "reference/numerical/ibt_lpk.inc"
%include "reference/numerical/bench_lib.inc"
%else
%include "ibt_dual_symbols.inc"
%include "ibt_lpk.inc"
%include "bench_lib.inc"
%endif
dual_core_end:
%ifdef KERNEL_REFERENCE
%include "reference/numerical/ibt_dual_symbols_end.inc"
%else
%include "ibt_dual_symbols_end.inc"
%endif
%ifndef CONT_PIPELINE
%define IBT_THIRD_ENGINE 1
%include "ibt_third_symbols.inc"
%ifdef CONT_ASYNC_RESID
%ifndef CONT_THREE_ASYNC
%define IBT_RESIDUAL_ENGINE 1
%endif
%endif
%include "ibt_lpk.inc"
%ifndef IBT_RESIDUAL_ENGINE
%include "bench_lib.inc"
%endif
third_core_end:
%include "ibt_third_symbols_end.inc"
%undef IBT_THIRD_ENGINE
%undef IBT_RESIDUAL_ENGINE
%endif
%ifdef CONT_ASYNC_RESID
%define IBT_RESIDUAL_ENGINE 1
%ifdef KERNEL_REFERENCE
%include "reference/numerical/ibt_verify_symbols.inc"
%include "reference/numerical/ibt_lpk.inc"
%else
%include "ibt_verify_symbols.inc"
%include "ibt_lpk.inc"
%endif
verify_core_end:
%ifdef KERNEL_REFERENCE
%include "reference/numerical/ibt_verify_symbols_end.inc"
%else
%include "ibt_verify_symbols_end.inc"
%endif
%undef IBT_RESIDUAL_ENGINE
%endif
%undef IBT_SECOND_ENGINE
%undef SELFTEST
%assign SELFTEST CONT_SAVED_SELFTEST
section .text
%ifdef CONT_ASYNC_RESID
%include "ibt_validator.inc"
%endif
%ifdef CONT_PIPELINE
%include "ibt_pipeline.inc"
%endif
