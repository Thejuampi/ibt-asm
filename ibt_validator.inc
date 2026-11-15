; The original full residual needs x, b, n and private workspace, not LU.
; Queue slots remain owned by the verifier until their checks are complete.
section .bss
alignb 64
contPendingHead resd 1
contPendingTail resd 1
contPendingCount resd 1
alignb 8
contPendingStride resq 1
contPendingBuffer resq 1
contValidationWork resq 1

section .text
cont_validator_alloc:
    push rbx
    sub rsp, 30h
    mov dword [contPendingHead], 0
    mov dword [contPendingTail], 0
    mov dword [contPendingCount], 0
    mov eax, [lpN]
    mov [verify_lpN], eax
    add eax, 8
    mov [verify_lpLDA], eax
    mov eax, [lpN]
    shl rax, 4
    add rax, 16+63
    and rax, -64
    mov [contPendingStride], rax
    lea rdx, [rax*4]
    xor ecx, ecx
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [contPendingBuffer], rax
    test rax, rax
    jz .out
    mov eax, [lpN]
    lea rdx, [rax*8+64]
    xor ecx, ecx
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [contValidationWork], rax
    mov [verify_lpWrk], rax
    test rax, rax
    setnz al
    movzx eax, al
.out:
    add rsp, 30h
    pop rbx
    ret

cont_validator_free:
    sub rsp, 28h
    mov rcx, [contPendingBuffer]
    test rcx, rcx
    jz .work
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
.work:
    mov rcx, [contValidationWork]
    test rcx, rcx
    jz .clear
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
.clear:
    mov qword [contPendingBuffer], 0
    mov qword [contValidationWork], 0
    mov qword [verify_lpWrk], 0
    mov qword [verify_lpB], 0
    mov qword [verify_lpResidX], 0
    mov dword [contPendingCount], 0
    add rsp, 28h
    ret

cont_enqueue_validation:
    push rbp
    mov rbp, rsp
    push rsi
    push rdi
    push r12
    sub rsp, 28h
.wait:
    cmp dword [lpStop], 0
    jne .out
    call cont_lock
    cmp dword [contPendingCount], 4
    jb .copy
    mov dword [contGate], 0
    call cont_help
    pause
    jmp .wait
.copy:
    mov eax, [contPendingTail]
    imul rax, [contPendingStride]
    add rax, [contPendingBuffer]
    mov r12, rax
    mov rdx, [rbx+C_TIME]
    mov rdx, [rdx]
    mov [rax], rdx
    lea rdi, [rax+16]
    mov rax, [rbx+C_X]
    mov rsi, [rax]
    mov rax, [rbx+C_N]
    mov ecx, [rax]
    rep movsq
    mov rax, [rbx+C_BV]
    mov rsi, [rax]
    mov rax, [rbx+C_N]
    mov ecx, [rax]
    rep movsq
    inc dword [contPendingTail]
    and dword [contPendingTail], 3
    inc dword [contPendingCount]
    mov dword [contGate], 0
.out:
    add rsp, 28h
    pop r12
    pop rdi
    pop rsi
    pop rbp
    ret

cont_validator:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    sub rsp, 30h
%ifdef CONT_BIND_TEST
    call cont_bind_coordinator
%endif
    mov rcx, -2
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .priority
    mov edx, THR_TIMECRIT
.priority:
    call SetThreadPriority
    call lp_pack_ensure
    test rax, rax
    jnz .scratch
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
.scratch:
    lea rbx, [contV]
.loop:
    cmp dword [lpStop], 0
    jne .out
    mov eax, [contVerified]
    cmp eax, [targetRuns]
    jae .out
    call cont_lock
    cmp dword [contPendingCount], 0
    je .empty
    mov eax, [contPendingHead]
    imul rax, [contPendingStride]
    add rax, [contPendingBuffer]
    mov r12, rax
    mov rdx, [rax]
    mov [verify_lpTmpT], rdx
    lea rdx, [rax+16]
    mov [verify_lpResidX], rdx
    mov eax, [lpN]
    lea rdx, [rdx+rax*8]
    mov [verify_lpB], rdx
    mov dword [contGate], 0
    call verify_lp_resid
%ifdef CONT_TEST
    call cont_inject_residual
%endif
    call cont_check
    test eax, eax
    jnz .consume
    cmp dword [lpStop], 0
    jne .consume
    call cont_publish
.consume:
    call cont_lock
    inc dword [contPendingHead]
    and dword [contPendingHead], 3
    dec dword [contPendingCount]
    mov dword [contGate], 0
    jmp .loop
.empty:
    mov dword [contGate], 0
    call cont_help
    pause
    jmp .loop
.out:
    call lp_pack_drop
    call lp_vzero
    xor eax, eax
    add rsp, 30h
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

