; One LU at a time, two matrix buffers, original full asynchronous residual.
; The background producer claims only an empty matrix. Dense jobs take
; priority over refill chunks; every prepared matrix is for a requested solve.
struc CONT_MATRIX
    .a resq 1
    .x resq 1
    .b resq 1
endstruc

section .bss
alignb 64
contMatrices resb 2*CONT_MATRIX_size
contFrontIndex resd 1
contBufferReady resd 2
contPipelinePhase resd 1 ; 0 waiting, 1 factor, 2 solve, 3 enqueue, 4 final checks
alignb 8
contComputeEnd resq 1

section .text
cont_pipeline_capture:
    mov rax, [lpA]
    mov [contMatrices+CONT_MATRIX.a], rax
    mov rax, [lpX]
    mov [contMatrices+CONT_MATRIX.x], rax
    mov rax, [lpB]
    mov [contMatrices+CONT_MATRIX.b], rax
    mov rax, [dual_lpA]
    mov [contMatrices+CONT_MATRIX_size+CONT_MATRIX.a], rax
    mov rax, [dual_lpX]
    mov [contMatrices+CONT_MATRIX_size+CONT_MATRIX.x], rax
    mov rax, [dual_lpB]
    mov [contMatrices+CONT_MATRIX_size+CONT_MATRIX.b], rax
    ret

; ECX is the owned slot and RBX is the idle role's descriptor.
cont_bind_matrix:
    lea rdx, [contMatrices]
    imul ecx, ecx, CONT_MATRIX_size
    add rdx, rcx
    mov rax, [rdx+CONT_MATRIX.a]
    mov rcx, [rbx+C_A]
    mov [rcx], rax
    mov rax, [rdx+CONT_MATRIX.x]
    mov rcx, [rbx+C_X]
    mov [rcx], rax
    mov rax, [rdx+CONT_MATRIX.b]
    mov rcx, [rbx+C_BV]
    mov [rcx], rax
    ret

; lp_free releases B through the currently bound pointer.
cont_pipeline_restore:
    cmp qword [contMatrices+CONT_MATRIX.a], 0
    je .out
    push rbx
    sub rsp, 20h
    lea rbx, [contA]
    xor ecx, ecx
    call cont_bind_matrix
    lea rbx, [contB]
    mov ecx, 1
    call cont_bind_matrix
    mov qword [contMatrices+CONT_MATRIX.a], 0
    add rsp, 20h
    pop rbx
.out:
    ret

cont_pipeline_init:
    push rbx
    push rsi
    sub rsp, 28h
    mov dword [contFrontIndex], 0
    mov qword [contBufferReady], 0
    mov dword [contPipelinePhase], 0
    mov qword [contComputeEnd], 0
    lea rbx, [contB]
    xor esi, esi
.fill:
    mov ecx, esi
    call cont_bind_matrix
    call [rbx+C_FILL]
    cmp dword [lpStop], 0
    jne .stopped
    lea rax, [contBufferReady]
    mov dword [rax+rsi*4], 1
    inc esi
    cmp esi, 2
    jb .fill
    xor eax, eax
    jmp .out
.stopped:
    mov eax, 1
.out:
    add rsp, 28h
    pop rsi
    pop rbx
    ret

cont_pipeline_run:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    and rsp, -16
    sub rsp, 40h
    call lp_pack_ensure
    test rax, rax
    jz .allocation_failed
    lea rbx, [contA]
    xor esi, esi
.loop:
    cmp dword [lpStop], 0
    jne .out
    mov eax, [contIssued]
    cmp eax, [targetRuns]
    jae .tail
.ready:
    cmp dword [lpStop], 0
    jne .out
    call cont_lock
    lea rax, [contBufferReady]
    cmp dword [rax+rsi*4], 1
    je .take
    mov dword [contGate], 0
    call cont_help
    pause
    jmp .ready
.take:
    mov dword [rax+rsi*4], 3 ; exclusively owned by the LU coordinator
    mov dword [contGate], 0
    mov ecx, esi
    call cont_bind_matrix
    call cont_next
    cmp eax, -1
    je .tail
    lea rcx, [rsp+20h]
    call QueryPerformanceCounter
    mov r13, [rsp+20h]
    mov dword [contPipelinePhase], 1
    call [rbx+C_FACTOR]
    cmp dword [lpStop], 0
    jne .out
    mov dword [contPipelinePhase], 2
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
    mov dword [contPipelinePhase], 3
    call cont_enqueue_validation
    cmp dword [lpStop], 0
    jne .out
    mov eax, [contIssued]
    cmp eax, [targetRuns]
    jne .swap
    lea rcx, [contComputeEnd]
    call QueryPerformanceCounter
.swap:
    call cont_lock
    lea rax, [contBufferReady]
    mov dword [rax+rsi*4], 0
    xor esi, 1
    mov [contFrontIndex], esi
    mov dword [contGate], 0
    mov dword [contPipelinePhase], 0
    jmp .loop
.tail:
    mov dword [contPipelinePhase], 4
    call cont_tail_worker
    jmp .out
.allocation_failed:
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

cont_pipeline_prepare:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    sub rsp, 28h
    mov rcx, -2
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .priority
    mov edx, THR_TIMECRIT
.priority:
    call SetThreadPriority
    call lp_pack_ensure
    test rax, rax
    jz .allocation_failed
    lea rbx, [contB]
.loop:
    cmp dword [lpStop], 0
    jne .out
    mov eax, [contVerified]
    cmp eax, [targetRuns]
    jae .out
    mov eax, [contIssued]
    cmp eax, [targetRuns]
    jae .help
    call cont_lock
    mov esi, [contFrontIndex]
    lea rax, [contBufferReady]
    cmp dword [rax+rsi*4], 0
    je .claim_empty
    xor esi, 1
    cmp dword [rax+rsi*4], 0
    jne .unlock_help
.claim_empty:
    mov dword [rax+rsi*4], 2 ; exclusively owned by the producer
    mov dword [contGate], 0
    mov ecx, esi
    call cont_bind_matrix
    call [rbx+C_FILL]
    call cont_lock
    lea rax, [contBufferReady]
    mov dword [rax+rsi*4], 1
    mov dword [contGate], 0
    jmp .loop
.unlock_help:
    mov dword [contGate], 0
.help:
    call cont_help
    pause
    jmp .loop
.allocation_failed:
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
.out:
    call lp_pack_drop
    call lp_vzero
    add rsp, 28h
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    xor eax, eax
    ret
