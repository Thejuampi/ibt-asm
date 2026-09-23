_start:
    mov r12, rsp
    and rsp, -16
    cmp qword [r12], 2
    jb .gui
    mov rsi, [r12+16]
    lea rdi, [szSelftest]
    call streq
    test eax, eax
    jnz .selftest
    mov rsi, [r12+16]
    lea rdi, [szSmoketest]
    call streq
    test eax, eax
    jz .gui
    mov dword [uiSmoke], 1
.gui:
    call runtime_init
    cmp dword [uiSmoke], 0
    je .open
    mov dword [stressChoice], 4
    mov dword [customMB], 1
    mov dword [timesValue], 2
    mov dword [threadChoice], 2
    mov dword [logEnabled], 1
.open:
    call ui_open
    test eax, eax
    jz .exit1
    cmp dword [uiSmoke], 0
    je .events
    call ui_start
.events:
    call event_loop
    call ui_shutdown
    xor edi, edi
    jmp .exit
.selftest:
    call runtime_init
    mov dword [nproc], 2
    mov dword [stressMB], 1
    mov dword [targetRuns], 1
    mov dword [resultState], RESULT_RUNNING
    call burn_thread
    cmp dword [failed], 0
    jne .exit1
    cmp dword [passCount], 1
    jne .exit1
    xor edi, edi
    jmp .exit
.exit1:
    mov edi, 1
.exit:
    mov eax, 60
    syscall

runtime_init:
    push rbp
    mov rbp, rsp
    call lp_isa_probe
    call TlsAlloc
    mov [lpTlsIdx], eax
    mov edi, 84                    ; _SC_NPROCESSORS_ONLN
    call sysconf
    test eax, eax
    jg .cpuok
    mov eax, 1
.cpuok:
    cmp eax, 32
    jbe .cpucap
    mov eax, 32
.cpucap:
    mov [nproc], eax
    mov ecx, eax
    inc ecx
    shr ecx, 1
    mov [lpPcores], ecx
    mov dword [timesValue], 10
    mov dword [customMB], 1024
    mov dword [logFd], -1
    call memory_update
    pop rbp
    ret

memory_update:
    push rbp
    mov rbp, rsp
    sub rsp, 128
    lea rdi, [rsp]
    call sysinfo
    test eax, eax
    jnz .fallback
    mov rax, [rsp+40]              ; freeram
    mov ecx, [rsp+104]             ; mem_unit
    imul rax, rcx
    shr rax, 20
    test eax, eax
    jnz .store
.fallback:
    mov eax, 1024
.store:
    mov [freeMB], eax
    mov [availMB], eax
    leave
    ret
