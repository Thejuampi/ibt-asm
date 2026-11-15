_start:
    mov r12, rsp
    and rsp, -16
    cmp qword [r12], 2
    jb .gui
%ifdef ELF_NATIVE
    mov rsi, [r12+16]
    lea rdi, [elf_verify_option]
    call streq
    test eax, eax
    jz .check_gui_option
    call elf_verify_kernels
    test eax, eax
    jz .exit1
    xor edi, edi
    jmp .exit
.check_gui_option:
    mov rsi, [r12+16]
    lea rdi, [elf_gui_test_option]
    call streq
    test eax, eax
    jz .check_build_option
    call elf_load_gui
    test eax, eax
    jz .gui_test_import_fail
    call runtime_init
    call ui_open
    test eax, eax
    jz .gui_test_display_fail
    cmp qword [xftDraw], 0
    je .gui_test_fail
    cmp qword [xftFont], 0
    je .gui_test_fail
    cmp qword [xftFontBold], 0
    je .gui_test_fail
    cmp qword [xftFontLarge], 0
    je .gui_test_fail
    cmp qword [flameImage], 0
    je .gui_test_fail
    cmp qword [coffeeImage], 0
    je .gui_test_fail
    call draw_ui
    mov rdi, [xDisplay]
    call XFlush
    call ui_shutdown
    xor edi, edi
    jmp .exit
.gui_test_fail:
    call ui_shutdown
    lea rsi, [elf_gui_resources_error]
    mov edx, elf_gui_resources_error_end - elf_gui_resources_error
    call elf_report_error
    mov edi, 3
    jmp .exit
.gui_test_display_fail:
    lea rsi, [elf_gui_display_error]
    mov edx, elf_gui_display_error_end - elf_gui_display_error
    call elf_report_error
    mov edi, 2
    jmp .exit
.gui_test_import_fail:
    lea rsi, [elf_gui_import_error]
    mov edx, elf_gui_import_error_end - elf_gui_import_error
    call elf_report_error
    mov edi, 4
    jmp .exit
.check_build_option:
    mov rsi, [r12+16]
    lea rdi, [elf_build_test_option]
    call streq
    test eax, eax
    jz .regular_option
    call elf_verify_kernels
    test eax, eax
    jz .exit1
    call elf_load_gui
    test eax, eax
    jnz .build_gui_loaded
    lea rsi, [elf_gui_import_error]
    mov edx, elf_gui_import_error_end - elf_gui_import_error
    call elf_report_error
    jmp .exit1
.build_gui_loaded:
    cmp qword [r12], 3
    jb .exit1
    mov rax, [r12+8]
    mov [elf_build_source], rax
    mov rax, [r12+24]
    mov [elf_build_destination], rax
    ; NASM creates a regular output file; make it executable before testing it.
    mov eax, 90                    ; chmod
    mov rdi, [r12+8]
    mov esi, 0o755
    syscall
    test eax, eax
    js .exit1
    jmp .selftest
.regular_option:
%endif
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
%ifdef ELF_NATIVE
    call elf_load_gui
    test eax, eax
    jnz .gui_loaded
    lea rsi, [elf_gui_import_error]
    mov edx, elf_gui_import_error_end - elf_gui_import_error
    call elf_report_error
    jmp .exit1
.gui_loaded:
%endif
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
    mov dword [nproc], 4
    mov dword [stressMB], 32
    mov dword [contFixedN], 1024
    mov dword [targetRuns], 6
    mov dword [overlapRuns], 1
    mov dword [resultState], RESULT_RUNNING
    call burn_thread
    call cont_drain
    cmp dword [failed], 0
    jne .exit1
    cmp dword [passCount], 6
    jne .exit1
    cmp dword [contVerified], 6
    jne .exit1
%ifdef ELF_NATIVE
    cmp qword [elf_build_destination], 0
    je .selftest_done
    mov eax, 82                    ; rename tested image into place
    mov rdi, [elf_build_source]
    mov rsi, [elf_build_destination]
    syscall
    test eax, eax
    js .exit1
.selftest_done:
%endif
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
