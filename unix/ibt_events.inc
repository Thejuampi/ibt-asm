event_loop:
    push rbp
    mov rbp, rsp
.loop:
    cmp dword [uiDone], 0
    je .paint
    call ui_finish
.paint:
    cmp dword [uiDirty], 0
    je .pending
    call draw_ui
.pending:
    mov rdi, [xDisplay]
    call XPending
    test eax, eax
    jz .idle
    mov rdi, [xDisplay]
    lea rsi, [xEvent]
    call XNextEvent
    mov eax, [xEvent]
    cmp eax, 12                    ; Expose
    je .expose
    cmp eax, 4                     ; ButtonPress
    je .button
    cmp eax, 2                     ; KeyPress
    je .key
    cmp eax, 33                    ; ClientMessage
    je .client
    jmp .check
.expose:
    mov dword [uiDirty], 1
    jmp .check
.button:
    call handle_button
    jmp .check
.key:
    call handle_key
    jmp .check
.client:
    mov rax, [xEvent+56]
    cmp rax, [xDelete]
    jne .check
    mov dword [uiQuit], 1
.check:
    cmp dword [uiQuit], 0
    je .loop
    pop rbp
    ret
.idle:
    cmp dword [running], 0
    je .sleep
    inc dword [flameTick]
    cmp dword [flameTick], 5
    jb .sleep
    mov dword [flameTick], 0
    inc dword [flameFrame]
    and dword [flameFrame], 7
    mov dword [uiDirty], 1
.sleep:
    mov edi, 20000
    call usleep
    jmp .check

handle_button:
    push rbp
    mov rbp, rsp
    mov eax, [xEvent+64]
    mov edx, [xEvent+68]
    cmp dword [uiAbout], 0
    jne .about
    mov ecx, [uiDrop]
    test ecx, ecx
    jz .normal
    mov dword [uiDrop], 0
    cmp ecx, 1
    jne .dropthreads
    cmp eax, 222
    jb .dirty
    cmp eax, 327
    jae .dirty
    cmp edx, 62
    jb .dirty
    cmp edx, 162
    jae .dirty
    mov eax, edx
    sub eax, 62
    xor edx, edx
    mov ecx, 20
    div ecx
    mov [stressChoice], eax
    jmp .dirty
.dropthreads:
    cmp eax, 222
    jb .dirty
    cmp eax, 294
    jae .dirty
    cmp edx, 100
    jb .dirty
    cmp edx, 260
    jae .dirty
    mov eax, edx
    sub eax, 100
    xor edx, edx
    mov ecx, 20
    div ecx
    mov [threadChoice], eax
    jmp .dirty
.normal:
    cmp edx, 36
    jb .out
    cmp edx, 64
    jae .row2
    cmp eax, 222
    jb .mbfield
    cmp eax, 327
    jae .mbfield
    cmp dword [running], 0
    jne .out
    mov dword [uiDrop], 1
    jmp .dirty
.mbfield:
    cmp eax, 336
    jb .start
    cmp eax, 391
    jae .start
    cmp dword [running], 0
    jne .out
    cmp dword [stressChoice], 4
    jne .out
    mov dword [uiFocus], 2
    jmp .dirty
.start:
    cmp eax, 526
    jb .out
    cmp eax, 596
    jae .out
    cmp dword [running], 0
    jne .out
    call ui_start
    jmp .dirty
.row2:
    cmp edx, 67
    jb .out
    cmp edx, 101
    jae .row3
    cmp eax, 100
    jb .threads
    cmp eax, 148
    jae .threads
    cmp dword [running], 0
    jne .out
    mov dword [uiFocus], 1
    jmp .dirty
.threads:
    cmp eax, 222
    jb .stop
    cmp eax, 294
    jae .stop
    cmp dword [running], 0
    jne .out
    mov dword [uiDrop], 2
    jmp .dirty
.stop:
    cmp eax, 526
    jb .out
    cmp eax, 596
    jae .out
    cmp dword [running], 0
    je .out
    call ui_stop
    jmp .dirty
.row3:
    cmp edx, 104
    jb .out
    cmp edx, 134
    jae .out
    cmp eax, 24
    jb .aboutbtn
    cmp eax, 218
    jae .aboutbtn
    cmp dword [running], 0
    jne .out
    xor dword [logEnabled], 1
    jmp .dirty
.aboutbtn:
    cmp eax, 462
    jb .coffee
    cmp eax, 505
    jae .coffee
    mov dword [uiAbout], 1
    jmp .dirty
.coffee:
    cmp eax, 511
    jb .out
    cmp eax, 596
    jae .out
    lea rdi, [szCoffeeCmd]
    call system
    jmp .out
.about:
    cmp eax, 270
    jb .out
    cmp eax, 350
    jae .out
    cmp edx, 294
    jb .out
    cmp edx, 328
    jae .out
    mov dword [uiAbout], 0
.dirty:
    mov dword [uiDirty], 1
.out:
    pop rbp
    ret

handle_key:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    cmp dword [uiAbout], 0
    je .input
    mov dword [uiAbout], 0
    jmp .dirty
.input:
    cmp dword [uiFocus], 0
    je .out
    lea rdi, [xEvent]
    lea rsi, [keyBuf]
    mov edx, 8
    lea rcx, [rsp]
    xor r8d, r8d
    call XLookupString
    test eax, eax
    jz .out
    movzx eax, byte [keyBuf]
    cmp al, 13
    je .blur
    cmp al, 27
    je .blur
    cmp al, 8
    je .back
    cmp al, 127
    je .back
    cmp al, '0'
    jb .out
    cmp al, '9'
    ja .out
    mov edx, [uiFocus]
    cmp edx, 1
    jne .mbdigit
    mov ecx, [timesValue]
    imul ecx, 10
    sub eax, '0'
    add ecx, eax
    cmp ecx, 999
    ja .out
    mov [timesValue], ecx
    jmp .dirty
.mbdigit:
    cmp dword [stressChoice], 4
    jne .out
    mov ecx, [customMB]
    imul ecx, 10
    sub eax, '0'
    add ecx, eax
    cmp ecx, 1048576
    ja .out
    mov [customMB], ecx
    jmp .dirty
.back:
    cmp dword [uiFocus], 1
    jne .mbback
    mov eax, [timesValue]
    xor edx, edx
    mov ecx, 10
    div ecx
    mov [timesValue], eax
    jmp .dirty
.mbback:
    mov eax, [customMB]
    xor edx, edx
    mov ecx, 10
    div ecx
    mov [customMB], eax
    jmp .dirty
.blur:
    mov dword [uiFocus], 0
.dirty:
    mov dword [uiDirty], 1
.out:
    leave
    ret

ui_start:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    cmp dword [running], 0
    jne .out
    mov eax, [timesValue]
    test eax, eax
    jnz .runs
    mov eax, 1
.runs:
    mov [targetRuns], eax
    mov eax, [stressChoice]
    cmp eax, 3
    je .maximum
    cmp eax, 4
    je .custom
    lea rdx, [stressSizes]
    mov eax, [rdx+rax*4]
    jmp .mem
.maximum:
    mov eax, [freeMB]
    sub eax, 256
    jnc .mem
    mov eax, 1
    jmp .mem
.custom:
    mov eax, [customMB]
    test eax, eax
    jnz .mem
    mov eax, 1
.mem:
    cmp eax, [freeMB]
    jbe .memok
    mov eax, [freeMB]
.memok:
    mov [stressMB], eax
    mov dword [failed], 0
    mov dword [haveResid], 0
    mov dword [passCount], 0
    mov dword [resultIndex], 0
    mov dword [lpAllocFail], 0
    mov byte [firstResid], 0
    mov byte [resultTime], 0
    mov byte [resultSpeed], 0
    mov byte [resultResid], 0
    mov dword [flameFrame], 0
    mov dword [flameTick], 0
    call log_open
    xor ecx, ecx
    xor edx, edx
    lea r8, [burn_thread]
    xor r9d, r9d
    call CreateThread
    test rax, rax
    jz .out
    mov [hProc], rax
    mov dword [running], 1
    mov dword [resultState], RESULT_RUNNING
    mov dword [uiDone], 0
.out:
    leave
    ret

ui_stop:
    push rbp
    mov rbp, rsp
    cmp dword [running], 0
    je .out
    mov dword [lpStop], 1
    mov rcx, [hProc]
    test rcx, rcx
    jz .state
    call WaitForSingleObject
    mov rcx, [hProc]
    call CloseHandle
    mov qword [hProc], 0
.state:
    mov dword [running], 0
    mov dword [uiDone], 0
    mov dword [resultState], RESULT_STOPPED
    call log_close
.out:
    pop rbp
    ret

ui_finish:
    push rbp
    mov rbp, rsp
    mov dword [uiDone], 0
    cmp dword [running], 0
    je .out
    mov rcx, [hProc]
    test rcx, rcx
    jz .state
    call WaitForSingleObject
    mov rcx, [hProc]
    call CloseHandle
    mov qword [hProc], 0
.state:
    mov dword [running], 0
    cmp dword [failed], 0
    jne .fail
    cmp dword [lpAllocFail], 0
    jne .fail
    cmp dword [passCount], 0
    je .fail
    mov dword [resultState], RESULT_PASS
    jmp .done
.fail:
    mov dword [resultState], RESULT_FAIL
.done:
    call log_close
    mov dword [uiDirty], 1
.out:
    pop rbp
    ret
