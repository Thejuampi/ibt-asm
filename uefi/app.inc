efi_main:
    EFI_ENTER
    mov rax, [rdx+ST_BOOT]
    mov [bootServices], rax
    mov rax, [rdx+ST_RUNTIME]
    mov [runtimeServices], rax
    mov rax, [rdx+ST_CONIN]
    mov [conIn], rax
    mov rax, [rdx+ST_CONOUT]
    mov [conOut], rax
    lea rcx, [rsp+576]
    call cpu_initialize
    xor ecx, ecx
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    mov rax, [bootServices]
    call [rax+BS_WATCHDOG]       ; disable the boot manager's five-minute reset
    call lp_isa_probe
    mov eax, [lpIsa]
    mov [commonIsa], eax
    call processors_init
    call select_isa
    call clock_init
    call timer_init
    call cpu_brand_read
    call memory_update
    call settings_defaults
%if UEFI_QA
    call qa_main
%else
    cmp dword [maxStressMB], 0
    jz .memory_error
    call ui_init
    call configuration_loop
    jmp .cleanup
.memory_error:
    lea rcx, [szMemoryError]
    call puts
    call read_key
%endif
.cleanup:
    mov rcx, [stopEvent]
    test rcx, rcx
    jz .returned
    mov rax, [bootServices]
    call [rax+BS_CLOSE_EVENT]
.returned:
    mov qword [rsp+608], 0
    EFI_RETURN

cpu_brand_read:
    push rbx
    push rsi
    push rdi
    mov eax, 80000000h
    cpuid
    cmp eax, 80000004h
    jb .generic
    lea rdi, [cpuBrand]
    mov esi, 80000002h
.leaf:
    mov eax, esi
    cpuid
    mov [rdi], eax
    mov [rdi+4], ebx
    mov [rdi+8], ecx
    mov [rdi+12], edx
    add rdi, 16
    inc esi
    cmp esi, 80000005h
    jb .leaf
    jmp .out
.generic:
    mov dword [cpuBrand], 'x86-'
    mov word [cpuBrand+4], '64'
.out:
    pop rdi
    pop rsi
    pop rbx
    ret

puts:
    push rsi
    push rdi
    FRAME 32
    mov rsi, rcx
    lea rdi, [wideText]
    mov ecx, 255
.convert:
    movzx eax, byte [rsi]
    inc rsi
    test eax, eax
    jz .terminated
%if UEFI_QA
    out 0e9h, al               ; debugcon exists only in the emulator build
%endif
    stosw
    loop .convert
.terminated:
    mov word [rdi], 0
    mov rcx, [conOut]
    test rcx, rcx
    jz .out
    lea rdx, [wideText]
    call [rcx+OUT_STRING]
.out:
    leave
    pop rdi
    pop rsi
    ret

print_number:
    push rdi
    FRAME 32
    lea rdi, [asciiNumber]
    call lp_itoa
    mov byte [rdi], 0
    lea rcx, [asciiNumber]
    call puts
    leave
    pop rdi
    ret

read_key:
    FRAME 32
    mov rcx, [conIn]
    test rcx, rcx
    jz .none
.again:
    mov dword [uiAwaitKey], 1
    mov rcx, [conIn]
    lea rdx, [inputKey]
    call [rcx+IN_READ]
    test rax, rax
    jz .got
    mov rcx, [conIn]
    mov rax, [rcx+IN_WAIT]
    mov [keyEvent], rax
    mov ecx, 1
    lea rdx, [keyEvent]
    lea r8, [keyIndex]
    mov rax, [bootServices]
    call [rax+BS_WAIT_EVENT]
    test rax, rax
    js .none
    jmp .again
.got:
    mov dword [uiAwaitKey], 0
    inc dword [uiKeySerial]
    movzx eax, word [inputKey+2]
    RETURN
.none:
    xor eax, eax
    RETURN

settings_defaults:
    mov eax, [nproc]
    mov [chosenThreads], eax
    mov dword [targetRuns], 10
    mov eax, 1024
    cmp eax, [maxStressMB]
    cmova eax, [maxStressMB]
    mov [stressMB], eax
    mov dword [lpIsaForce], 0
    jmp select_isa

configuration_loop:
    push rbx
    push rsi
    FRAME 64
.menu:
    mov dword [uiMode], UI_READY
    call memory_update
    cmp [stressMB], eax
    jbe .draw
    mov [stressMB], eax
.draw:
    call ui_render
.key:
    call read_key
    cmp word [inputKey], SCAN_F2
    je .advanced
    cmp word [inputKey], SCAN_F1
    je .help
    cmp al, 13
    je .start
    or al, 20h
    cmp al, 'q'
    je .out
    cmp al, 's'
    je .start
    cmp al, 'h'
    je .help
    cmp al, 'd'
    je .reset
    lea rbx, [stressMB]
    lea rcx, [uiEditSize]
    mov esi, 1
    mov edx, [maxStressMB]
    cmp al, 'm'
    je .edit
    lea rbx, [targetRuns]
    lea rcx, [uiEditRuns]
    mov esi, 2
    mov edx, 100000
    cmp al, 'r'
    je .edit
    lea rbx, [chosenThreads]
    lea rcx, [uiEditThreads]
    mov esi, 1
    mov edx, [nproc]
    cmp al, 't'
    jne .key
.edit:
    mov [rsp+32], edx
    mov [rsp+40], rcx
    mov dword [uiMode], UI_EDIT
    mov dword [uiEditError], 0
.prompt:
    mov rcx, [rsp+40]
    mov edx, [rsp+32]
    call ui_edit_prompt
    call read_number
    cmp eax, -1                 ; empty Enter / Escape keeps the current value
    je .edit_done
    cmp eax, -2
    jne .validate
    lea rdx, [targetRuns]
    cmp rbx, rdx
    je .invalid
    mov eax, [rsp+32]           ; A = maximum for Size or Threads
.validate:
    cmp eax, esi
    jb .invalid
    cmp eax, [rsp+32]
    ja .invalid
    mov [rbx], eax
.edit_done:
    xor edx, edx
    call ui_cursor_visible
    jmp .menu
.invalid:
    mov dword [uiEditError], 1
    jmp .prompt
.reset:
    call settings_defaults
    mov dword [uiResultCount], 0
    jmp .menu
.help:
    call ui_help
    jmp .draw
.advanced:
    call ui_advanced
    jmp .menu
.start:
    call session_reset
    call ui_start
    mov eax, [stressMB]
    mov ecx, eax
    shl rcx, 17
    call isqrt
    and eax, ~7
    mov [lpN], eax
    call session_prepare
    test eax, eax
    jz .verdict
    call session_run
.verdict:
    call session_cleanup
    call ui_finish
.result_key:
    call read_key
    cmp word [inputKey], SCAN_F1
    je .result_help
    cmp al, 13
    je .start
    or al, 20h
    cmp al, 'r'
    je .start
    cmp al, 'c'
    je .change
    cmp al, 'q'
    je .out
    cmp al, 'h'
    jne .result_key
.result_help:
    call ui_help
    call ui_render
    jmp .result_key
.change:
    mov dword [uiResultCount], 0
    jmp .menu
.out:
    xor edx, edx
    call ui_cursor_visible
    mov edx, UI_NORMAL
    call ui_color
    leave
    pop rsi
    pop rbx
    ret

print_isa:
    lea rcx, [szBitsSse2]
    cmp dword [lpIsaUse], 1
    jb puts
    lea rcx, [szBitsAvx]
    cmp dword [lpIsaUse], 2
    jb puts
    lea rcx, [szBitsAvx2]
    jmp puts

read_number:
    push rbx
    push rsi
    FRAME 32
    xor ebx, ebx
    xor esi, esi
.key:
    call read_key
    cmp word [inputKey], SCAN_ESC
    je .cancel
    cmp al, 27
    je .cancel
    cmp al, 13
    je .done
    cmp al, 8
    je .backspace
    or al, 20h
    cmp al, 'a'
    je .maximum
    cmp al, '0'
    jb .key
    cmp al, '9'
    ja .key
    cmp esi, 8
    jae .key
    movzx eax, al
    mov [asciiNumber], al
    mov byte [asciiNumber+1], 0
    sub eax, '0'
    imul ebx, 10
    add ebx, eax
    inc esi
    lea rcx, [asciiNumber]
    call puts
    jmp .key
.backspace:
    test esi, esi
    jz .key
    dec esi
    mov eax, ebx
    xor edx, edx
    mov ecx, 10
    div ecx
    mov ebx, eax
    lea rcx, [szBackspace]
    call puts
    jmp .key
.maximum:
    mov ebx, -2
    jmp .newline
.cancel:
    mov ebx, -1
.done:
    test esi, esi
    jnz .newline
    mov ebx, -1
.newline:
    lea rcx, [szNewline]
    call puts
    mov eax, ebx
    leave
    pop rsi
    pop rbx
    ret

session_reset:
    xor eax, eax
    mov [failed], eax
    mov [haveResid], eax
    mov [passCount], eax
    mov [resultIndex], eax
    mov [lpAllocFail], eax
    mov [lpStop], eax
    mov [lpDie], eax
    mov [lpPhase], eax
    mov [lpDone], eax
    mov [stopRequested], eax
    mov [dispatchFailed], eax
    mov [passCompleted], eax
    mov [qaCounter], eax
    mov [uiErrorKind], eax
    ret

session_prepare:
    push rbx
    push rsi
    FRAME 32
%if UEFI_QA
    lea rcx, [szPreparing]
    call puts
%endif
    mov eax, [chosenThreads]
    mov [lpNthr], eax
    call lp_alloc_n
    test eax, eax
    jz .fail
    cmp dword [lpAllocFail], 0
    jne .fail                  ; also catch the shared engine's last pack buffer
    xor ebx, ebx
    cmp dword [lpNthr], 2
    jb .allocate
    inc ebx                    ; parallel slots are AP1..APn; BSP runs firmware
.allocate:
    xor ecx, ecx
    mov edx, SCRATCH_BYTES
    call VirtualAlloc
    test rax, rax
    jz .fail
    mov qword [rax-4096+8], 1   ; defer freeing until all APs have returned
    lea rdx, [scratchPages]
    mov [rdx+rbx*8], rax
    lea rdx, [cpuTls]
    mov [rdx+rbx*8], rax
    cmp dword [lpNthr], 2
    jb .ok
    inc ebx
    cmp ebx, [lpNthr]
    jbe .allocate
.ok:
    mov eax, 1
    leave
    pop rsi
    pop rbx
    ret
.fail:
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
    xor eax, eax
    leave
    pop rsi
    pop rbx
    ret

session_run:
    push rbx
    FRAME 64
.pass:
    mov dword [lpPhase], 0
    mov dword [lpDone], 0
    mov dword [lpDie], 0
    mov dword [apReady], 0
    mov dword [apFinished], 0
    mov dword [passCompleted], 0
    xor ebx, ebx
.tls:
    lea rdx, [scratchPages]
    mov rax, [rdx+rbx*8]
    lea rdx, [cpuTls]
    mov [rdx+rbx*8], rax
    inc ebx
    cmp ebx, MAX_CPUS
    jbe .tls
    mov dword [testActive], 1
    cmp dword [lpNthr], 2
    jb .serial
    mov rcx, [mpServices]
    lea rdx, [ap_entry]
    xor r8d, r8d               ; all APs run concurrently
    xor r9d, r9d               ; blocking, valid after ReadyToBoot
    mov qword [rsp+32], 0      ; no forced termination timeout
    mov qword [rsp+40], 0
    mov qword [rsp+48], 0
    call [rcx+MP_START_ALL]
    test rax, rax
    jns .joined
    mov dword [dispatchFailed], 1
    mov dword [lpStop], 1
    jmp .joined
.serial:
    call compute_once
.joined:
    mov dword [testActive], 0
    call poll_stop
    cmp dword [passCompleted], 0
    je .out
    movsd xmm0, [lpTmpN]
    ucomisd xmm0, [lpC16]
    jp .bad
    ja .bad
    ucomisd xmm0, [lpC0]
    jb .bad
    call lp_emit               ; presentation and consistency checks on BSP only
    cmp dword [lpStop], 0
    jne .out
    mov eax, [passCount]
    cmp eax, [targetRuns]
    jb .pass
    jmp .out
.bad:
    mov dword [uiErrorKind], 2
    mov dword [failed], 1
    mov dword [lpStop], 1
    call lp_emit
.out:
    leave
    pop rbx
    ret

compute_once:
    push rbx
    FRAME 48
    cmp dword [lpStop], 0
    jne .out
    mov dword [uiStage], 1
    call core_fill
    cmp dword [lpStop], 0
    jne .out
    lea rcx, [rsp+32]
    call QueryPerformanceCounter
    mov rbx, [rsp+32]
    mov dword [uiStage], 2
    call core_factor
    cmp dword [lpStop], 0
    jne .out
    mov dword [uiStage], 3
    call core_solve
    lea rcx, [rsp+32]
    call QueryPerformanceCounter
    mov rax, [rsp+32]
    sub rax, rbx
    jnz .elapsed
    mov eax, 1
.elapsed:
    cvtsi2sd xmm0, rax
    cvtsi2sd xmm1, qword [lpFreq]
    divsd xmm0, xmm1
    movsd [lpTmpT], xmm0
    mov rax, [lpX]
    mov [lpResidX], rax
    mov dword [uiStage], 4
    call core_residual
    mov dword [passCompleted], 1
.out:
    leave
    pop rbx
    ret

ap_entry:
    EFI_ENTER
    mov qword [rsp+576], 0
    call current_slot
    cmp eax, 1
    jl .return
    cmp eax, [lpNthr]
    ja .return
    mov edi, eax
    lea rcx, [rsp+576]
    call cpu_initialize
    lock inc dword [apReady]
    cmp edi, 1
    je .coordinator
    call core_worker
    lock inc dword [apFinished]
    jmp .return
.coordinator:
    mov eax, [apReady]
    cmp eax, [lpNthr]
    jae .compute
    pause
    jmp .coordinator
.compute:
    call compute_once
    mov dword [lpDie], 1
    lock inc dword [lpPhase]
.join_workers:
    mov eax, [lpNthr]
    dec eax
    cmp eax, [apFinished]
    jbe .return
    pause
    jmp .join_workers
.return:
    mov qword [rsp+608], 0
    EFI_RETURN

session_cleanup:
    push rbx
    FRAME 32
    mov dword [testActive], 0
    call lp_free
    xor ebx, ebx
.scratch:
    lea rdx, [scratchPages]
    mov rcx, [rdx+rbx*8]
    test rcx, rcx
    jz .next
    call pages_release
    lea rdx, [scratchPages]
    mov qword [rdx+rbx*8], 0
.next:
    lea rdx, [cpuTls]
    mov qword [rdx+rbx*8], 0
    inc ebx
    cmp ebx, MAX_CPUS
    jbe .scratch
    leave
    pop rbx
    ret

post_line:
    FRAME 32
%if UEFI_QA && UEFI_QA_CASE = 3
    cmp dword [resultIndex], 1
    jne .compare
    xor qword [lpTmpN], 1      ; a hidden last-bit mismatch must also fail
.compare:
%endif
    cmp dword [haveResid], 0
    jne .match
    mov rax, [lpTmpN]
    mov [lpNorm0], rax
    mov dword [haveResid], 1
    jmp .count
.match:
    mov rax, [lpTmpN]
    cmp rax, [lpNorm0]
    je .count
    mov dword [uiErrorKind], 1
    mov dword [failed], 1
    mov dword [lpStop], 1
.count:
    inc dword [resultIndex]
    cmp dword [failed], 0
    jne .display
    inc dword [passCount]
.display:
%if UEFI_QA
    mov edx, [resultIndex]
    call print_number
    lea rcx, [szSlash]
    call puts
    mov edx, [targetRuns]
    call print_number
    lea rcx, [szSpaces]
    call puts
    lea rcx, [lineAcc]
    call puts
    lea rcx, [szSpaces]
    call puts
    lea rcx, [lineAcc+32]
    call puts
    lea rcx, [szSpaces]
    call puts
    lea rcx, [lineAcc+64]
    call puts
    lea rcx, [szNewline]
    call puts
%else
    call ui_record_result
%endif
    RETURN

print_verdict:
    lea rcx, [szAllocError]
    cmp dword [lpAllocFail], 0
    jne puts
    lea rcx, [szDispatchError]
    cmp dword [dispatchFailed], 0
    jne puts
    lea rcx, [szFail]
    cmp dword [failed], 0
    jne puts
    lea rcx, [szStopped]
    cmp dword [stopRequested], 0
    jne puts
    mov eax, [targetRuns]
    cmp eax, 2
    jb puts
    cmp eax, [passCount]
    jne puts
    lea rcx, [szPass]
    jmp puts

; Select once per numerical stage, keeping the hot kernels free of ISA checks
; added by the port. Both variants read exactly the same matrix and job state.
core_fill:
    cmp dword [lpIsaUse], 0
    je uefi_sse2_lp_fill
    jmp lp_fill
core_factor:
    cmp dword [lpIsaUse], 0
    je uefi_sse2_lp_dgetrf
    jmp lp_dgetrf
core_solve:
    cmp dword [lpIsaUse], 0
    je uefi_sse2_lp_dgetrs
    jmp lp_dgetrs
core_residual:
    cmp dword [lpIsaUse], 0
    je uefi_sse2_lp_resid
    jmp lp_resid
core_worker:
    cmp dword [lpIsaUse], 0
    je uefi_sse2_lp_worker
    jmp lp_worker
