%if UEFI_QA
; This code is excluded from the distributable. Exercise the actual EFI image,
; firmware dispatcher, shared kernels, cleanup, and verdicts under OVMF/QEMU.
qa_main:
    push rbx
    push rsi
    push rdi
    FRAME 48
    lea rcx, [szHeader]
    call puts
    lea rcx, [szNewline]
    call puts
    lea rcx, [szCpu]
    call puts
    lea rcx, [cpuBrand]
    call puts
    lea rcx, [szNewline]
    call puts
    call print_isa
    lea rcx, [szNewline]
    call puts
%if UEFI_QA_CASE
    mov dword [chosenThreads], 1
    mov dword [lpN], 400
    mov dword [targetRuns], 2
    call session_reset
%if UEFI_QA_CASE = 1
    mov dword [allocCountdown], 1
%elif UEFI_QA_CASE = 2
    mov dword [allocCountdown], 7
%endif
    call session_prepare
    test eax, eax
    jz .injected_done
    call session_run
.injected_done:
    call session_cleanup
    call print_verdict
    cmp qword [allocatedPages], 0
    jne .fail
%if UEFI_QA_CASE = 1 || UEFI_QA_CASE = 2
    cmp dword [lpAllocFail], 1
    jne .fail
    cmp dword [passCount], 0
    jne .fail
    mov dword [lpN], 68
    call qa_case               ; a failed allocation must allow a clean retry
    test eax, eax
    jnz .fail
%elif UEFI_QA_CASE = 3
    cmp dword [failed], 1
    jne .fail
    cmp dword [passCount], 1
    jne .fail
%elif UEFI_QA_CASE = 4
    cmp dword [stopRequested], 1
    jne .fail
    cmp dword [passCount], 0
    jne .fail
%endif
    jmp .ok
%else
    mov esi, 1                 ; cap each available execution path in turn
.isa:
    mov [lpIsaForce], esi
    call select_isa
    mov eax, [lpIsaUse]
    inc eax
    cmp eax, esi
    jne .ok
    call print_isa
    lea rcx, [szNewline]
    call puts
    lea rdi, [qaSizes]
.size:
    mov eax, [rdi]
    test eax, eax
    jz .parallel
    mov [lpN], eax
    mov dword [chosenThreads], 1
    call qa_case
    test eax, eax
    jnz .fail
    add rdi, 4
    jmp .size
.parallel:
    cmp dword [nproc], 2
    jb .next_isa
    mov dword [lpN], 400
    mov eax, [nproc]
    mov [chosenThreads], eax
    call qa_case
    test eax, eax
    jnz .fail
    mov dword [lpN], 768
    call qa_case
    test eax, eax
    jnz .fail
.next_isa:
    inc esi
    cmp esi, 3
    jbe .isa
%endif
.ok:
    lea rcx, [szQaTotal]
    call puts
    mov edx, [qaCases]
    call print_number
    lea rcx, [szNewline]
    call puts
    lea rcx, [szQaOk]
    call puts
    mov eax, 16
    jmp .exit
.fail:
    lea rcx, [szQaFail]
    call puts
    mov eax, 17
.exit:
    mov dx, 0f4h
    out dx, eax                ; isa-debug-exit: success exit status is 33
    cli
.halt:
    hlt
    jmp .halt

qa_case:
    push rbx
    FRAME 48
    lea rcx, [szQaCase]
    call puts
    mov edx, [lpN]
    call print_number
    lea rcx, [szQaWorkers]
    call puts
    mov edx, [chosenThreads]
    call print_number
    lea rcx, [szNewline]
    call puts
    mov ebx, [lpN]
    call session_reset
    mov [lpN], ebx
    mov dword [targetRuns], 2
    call session_prepare
    test eax, eax
    jz .failed
    call session_run
    call session_cleanup
    cmp qword [allocatedPages], 0
    jne .failed
    cmp dword [failed], 0
    jne .failed
    cmp dword [lpAllocFail], 0
    jne .failed
    cmp dword [dispatchFailed], 0
    jne .failed
    cmp dword [stopRequested], 0
    jne .failed
    cmp dword [passCount], 2
    jne .failed
    inc dword [qaCases]
    xor eax, eax
    jmp .out
.failed:
    call session_cleanup
    call print_verdict
    mov eax, 1
.out:
    leave
    pop rbx
    ret
%endif
