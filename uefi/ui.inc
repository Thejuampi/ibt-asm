; A bounded console view: configuration, progress, results, and next actions.
; All output happens on the BSP. uiBusy prevents timer reentry while drawing.
%macro UI_TEXT 4
    lea rcx, [%1]
    mov edx, %2
    mov r8d, %3
    mov r9d, %4
    call ui_text_at
%endmacro

%macro UI_NUMBER 3
    mov edx, %2
    mov r8d, %3
    call ui_position
    mov edx, [%1]
    call print_number
%endmacro

ui_init:
    FRAME 32
    mov qword [uiColumns], 80
    mov qword [uiRows], 25
    mov rcx, [conOut]
    mov rax, [rcx+OUT_MODE]
    mov edx, [rax+4]
    lea r8, [uiColumns]
    lea r9, [uiRows]
    call [rcx+OUT_QUERY_MODE]
    test rax, rax
    js .fallback
    cmp qword [uiColumns], 80
    jb .fallback
    cmp qword [uiRows], 25
    jae .center
.fallback:
    mov qword [uiColumns], 80
    mov qword [uiRows], 25
    mov rcx, [conOut]
    xor edx, edx
    call [rcx+OUT_SET_MODE]     ; UEFI mode zero is the standard 80x25 mode
.center:
    mov eax, [uiColumns]
    sub eax, UI_WIDTH
    shr eax, 1
    mov [uiOriginX], eax
    mov eax, [uiRows]
    sub eax, UI_HEIGHT
    shr eax, 1
    mov [uiOriginY], eax
    mov edx, UI_NORMAL
    call ui_color
    mov rcx, [conOut]
    call [rcx+OUT_CLEAR]
    xor edx, edx
    call ui_cursor_visible
    RETURN

ui_color:
    FRAME 32
    mov rcx, [conOut]
    call [rcx+OUT_ATTRIBUTE]
    RETURN

; Relative column in EDX, row in R8D.
ui_position:
    FRAME 32
    add edx, [uiOriginX]
    add r8d, [uiOriginY]
    mov rcx, [conOut]
    call [rcx+OUT_POSITION]
    RETURN

ui_cursor_visible:
    FRAME 32
    mov rcx, [conOut]
    call [rcx+OUT_CURSOR]
    RETURN

; Text in RCX, column in EDX, row in R8D, color in R9D.
ui_text_at:
    push rbx
    push rsi
    push rdi
    FRAME 32
    mov rsi, rcx
    mov ebx, edx
    mov edi, r8d
    mov edx, r9d
    call ui_color
    mov edx, ebx
    mov r8d, edi
    call ui_position
    mov rcx, rsi
    call puts
    leave
    pop rdi
    pop rsi
    pop rbx
    ret

ui_clear:
    push rbx
    FRAME 32
    xor edx, edx
    call ui_cursor_visible
    xor ebx, ebx
.row:
    lea rcx, [uiBlank]
    xor edx, edx
    mov r8d, ebx
    mov r9d, UI_NORMAL
    call ui_text_at
    inc ebx
    cmp ebx, UI_HEIGHT
    jb .row
    leave
    pop rbx
    ret

ui_render:
    cmp dword [uiBusy], 0
    jne .out
    push rbx
    push rsi
    push rdi
    FRAME 32
    mov dword [uiBusy], 1
    call ui_clear
    UI_TEXT szHeader, 0, 0, UI_BRIGHT
    UI_TEXT cpuBrand, 0, 1, UI_NORMAL
    UI_TEXT uiRule, 0, 2, UI_MUTED
    cmp dword [uiMode], UI_RUNNING
    je .locked
    cmp dword [uiMode], UI_RESULT
    je .locked
    UI_TEXT uiSizeLabel, 0, 3, UI_ACCENT
    UI_TEXT uiThreadsLabel, 28, 3, UI_ACCENT
    UI_TEXT uiRunsLabel, 47, 3, UI_ACCENT
    UI_TEXT uiResetLabel, 65, 3, UI_ACCENT
    jmp .values
.locked:
    UI_TEXT uiSizeLocked, 0, 3, UI_NORMAL
    UI_TEXT uiThreadsLocked, 28, 3, UI_NORMAL
    UI_TEXT uiRunsLocked, 47, 3, UI_NORMAL
.values:
    mov edx, UI_BRIGHT
    call ui_color
    UI_NUMBER stressMB, 0, 4
    lea rcx, [uiMiB]
    call puts
    UI_NUMBER chosenThreads, 28, 4
    UI_NUMBER targetRuns, 47, 4
    UI_TEXT uiAvailable, 0, 5, UI_MUTED
    mov edx, [maxStressMB]
    call print_number
    lea rcx, [uiMiB]
    call puts
    UI_TEXT uiModeLabel, 28, 5, UI_MUTED
    cmp dword [lpIsaForce], 0
    jne .manual
    lea rcx, [uiAutoOpen]
    call puts
.manual:
    call print_isa
    cmp dword [lpIsaForce], 0
    jne .table
    lea rcx, [uiClose]
    call puts
.table:
    UI_TEXT uiTableRun, 0, 10, UI_MUTED
    lea rcx, [uiTableTime]
    cmp dword [timingEstimated], 0
    je .time
    lea rcx, [uiTableTimeEst]
.time:
    mov edx, 10
    mov r8d, 10
    mov r9d, UI_MUTED
    call ui_text_at
    lea rcx, [uiTableGflops]
    cmp dword [timingEstimated], 0
    je .gflops
    lea rcx, [uiTableGflopsEst]
.gflops:
    mov edx, 24
    mov r8d, 10
    mov r9d, UI_MUTED
    call ui_text_at
    UI_TEXT uiTableResidual, 39, 10, UI_MUTED
    UI_TEXT uiTableResult, 61, 10, UI_MUTED
    UI_TEXT uiRule, 0, 11, UI_MUTED
    call ui_render_rows
    call ui_render_status
    cmp dword [uiMode], UI_RUNNING
    je .running
    cmp dword [uiMode], UI_RESULT
    je .result
    UI_TEXT uiStart, 0, 21, UI_ACCENT
    UI_TEXT uiSecondary, 0, 22, UI_NORMAL
    jmp .done
.running:
    UI_TEXT uiStop, 0, 21, UI_ACCENT
    jmp .done
.result:
    UI_TEXT uiActions, 0, 21, UI_ACCENT
    UI_TEXT uiResultHelp, 0, 22, UI_NORMAL
.done:
    mov dword [uiBusy], 0
    leave
    pop rdi
    pop rsi
    pop rbx
.out:
    ret

ui_render_rows:
    push rbx
    push rsi
    push rdi
    FRAME 32
    mov ebx, 1
    mov eax, [uiResultCount]
    test eax, eax
    jz .empty
    cmp eax, UI_HISTORY
    jbe .begin
    lea ebx, [rax-UI_HISTORY+1]
.begin:
    mov edi, 12
.row:
    lea eax, [rbx-1]
    and eax, UI_HISTORY-1
    shl eax, 7
    lea rsi, [uiResults]
    add rsi, rax
    mov edx, UI_NORMAL
    call ui_color
    xor edx, edx
    mov r8d, edi
    call ui_position
    mov edx, ebx
    call print_number
    mov edx, 10
    mov r8d, edi
    call ui_position
    lea rcx, [rsi+16]
    call puts
    mov edx, 24
    mov r8d, edi
    call ui_position
    lea rcx, [rsi+48]
    call puts
    mov edx, 39
    mov r8d, edi
    call ui_position
    lea rcx, [rsi+80]
    call puts
    lea rcx, [uiRowOk]
    mov r9d, UI_SUCCESS
    cmp dword [rsi+4], 0
    je .status
    lea rcx, [uiRowMismatch]
    mov r9d, UI_ERROR
    cmp dword [rsi+4], 1
    je .status
    lea rcx, [uiRowInvalid]
.status:
    mov edx, 61
    mov r8d, edi
    call ui_text_at
    inc edi
    inc ebx
    cmp ebx, [uiResultCount]
    jbe .row
    cmp dword [uiResultCount], UI_HISTORY
    jbe .estimate
    UI_TEXT uiShowing, 0, 20, UI_MUTED
    mov edx, [uiResultCount]
    sub edx, UI_HISTORY-1
    call print_number
    lea rcx, [uiTo]
    call puts
    mov edx, [uiResultCount]
    call print_number
    lea rcx, [uiLatest]
    call puts
    jmp .done
.empty:
    UI_TEXT uiNoResults, 0, 12, UI_MUTED
.estimate:
    cmp dword [timingEstimated], 0
    je .done
    UI_TEXT uiEstimatedHint, 0, 20, UI_MUTED
.done:
    leave
    pop rdi
    pop rsi
    pop rbx
    ret

; Wall time includes setup and all runs. Latch it when the test finishes.
ui_elapsed_time:
    lfence
    rdtsc
    shl rdx, 32
    or rax, rdx
    sub rax, [uiStartTick]
    xor edx, edx
    div qword [tscFrequency]
    mov [uiElapsed], rax
    ret

ui_print_elapsed:
    push rbx
    push rdi
    FRAME 32
    lea rdi, [asciiNumber]
    mov rax, [uiElapsed]
    mov ecx, 3600
    xor edx, edx
    div rcx
    mov ebx, edx
    mov edx, eax
    call lp_itoa
    mov byte [rdi], ':'
    inc rdi
    mov eax, ebx
    xor edx, edx
    mov ecx, 60
    div ecx
    mov ebx, edx
    xor edx, edx
    mov ecx, 10
    div ecx
    add al, '0'
    stosb
    add dl, '0'
    mov al, dl
    stosb
    mov al, ':'
    stosb
    mov eax, ebx
    xor edx, edx
    div ecx
    add al, '0'
    stosb
    add dl, '0'
    mov al, dl
    stosb
    mov byte [rdi], 0
    lea rcx, [asciiNumber]
    call puts
    leave
    pop rdi
    pop rbx
    ret

ui_render_status:
    FRAME 48
    UI_TEXT uiBlank, 0, 7, UI_NORMAL
    UI_TEXT uiBlank, 0, 8, UI_NORMAL
    cmp dword [uiMode], UI_RUNNING
    je .running
    cmp dword [uiMode], UI_RESULT
    je .result
    UI_TEXT uiReady, 0, 7, UI_BRIGHT
    UI_TEXT uiReadyHint, 0, 8, UI_NORMAL
    jmp .done
.running:
    cmp dword [stopRequested], 0
    je .progress
    UI_TEXT uiStopping, 0, 7, UI_WARNING
    jmp .bar
.progress:
    UI_TEXT uiRunning, 0, 7, UI_BRIGHT
    mov edx, [passCount]
    inc edx
    cmp edx, [targetRuns]
    cmova edx, [targetRuns]
    call print_number
    lea rcx, [uiOf]
    call puts
    mov edx, [targetRuns]
    call print_number
    lea rcx, [uiElapsedLabel]
    cmp dword [timingEstimated], 0
    je .elapsed
    lea rcx, [uiElapsedApprox]
.elapsed:
    call puts
    call ui_print_elapsed
    lea rcx, [uiStageSep]
    call puts
    lea rcx, [uiStagePrepare]
    cmp dword [uiStage], 1
    jb .stage
    lea rcx, [uiStageFill]
    je .stage
    lea rcx, [uiStageFactor]
    cmp dword [uiStage], 2
    je .stage
    lea rcx, [uiStageSolve]
    cmp dword [uiStage], 3
    je .stage
    lea rcx, [uiStageVerify]
.stage:
    call puts
.bar:
    call ui_progress_bar
    UI_TEXT uiBar, 0, 8, UI_ACCENT
    lea rcx, [szSpaces]
    call puts
    mov edx, [passCount]
    call print_number
    lea rcx, [szSlash]
    call puts
    mov edx, [targetRuns]
    call print_number
    lea rcx, [uiComplete]
    call puts
    jmp .done
.result:
    lea rcx, [uiPassed]
    mov r9d, UI_SUCCESS
    cmp dword [lpAllocFail], 0
    jne .startup_error
    cmp dword [dispatchFailed], 0
    jne .startup_error
    cmp dword [failed], 0
    jne .failed
    cmp dword [stopRequested], 0
    jne .stopped
    mov eax, [passCount]
    cmp eax, [targetRuns]
    je .verdict
.failed:
    lea rcx, [uiFailed]
    mov r9d, UI_ERROR
    jmp .verdict
.stopped:
    lea rcx, [uiStopped]
    mov r9d, UI_WARNING
    jmp .verdict
.startup_error:
    lea rcx, [uiCouldNotStart]
    mov r9d, UI_ERROR
.verdict:
    xor edx, edx
    mov r8d, 7
    call ui_text_at
    lea rcx, [uiElapsedLabel]
    cmp dword [timingEstimated], 0
    je .elapsed_result
    lea rcx, [uiElapsedApprox]
.elapsed_result:
    call puts
    call ui_print_elapsed
    mov edx, UI_NORMAL
    call ui_color
    xor edx, edx
    mov r8d, 8
    call ui_position
    mov edx, [passCount]
    call print_number
    lea rcx, [szSlash]
    call puts
    mov edx, [targetRuns]
    call print_number
    lea rcx, [uiMatched]
    mov eax, [passCount]
    cmp eax, [targetRuns]
    je .summary
    lea rcx, [uiIncomplete]
.summary:
    call puts
    lea rcx, [uiAllocHint]
    cmp dword [lpAllocFail], 0
    jne .hint
    lea rcx, [uiDispatchHint]
    cmp dword [dispatchFailed], 0
    jne .hint
    lea rcx, [uiMismatchHint]
    cmp dword [uiErrorKind], 1
    je .hint
    lea rcx, [uiInvalidHint]
    cmp dword [failed], 0
    jne .hint
    lea rcx, [uiStopHint]
    cmp dword [stopRequested], 0
    je .done
.hint:
    mov [rsp+32], rcx
    UI_TEXT uiBlank, 0, 20, UI_NORMAL
    mov rcx, [rsp+32]
    xor edx, edx
    mov r8d, 20
    mov r9d, UI_NORMAL
    call ui_text_at
.done:
    RETURN

ui_progress_bar:
    push rdi
    mov eax, [passCount]
    imul eax, 30
    xor edx, edx
    div dword [targetRuns]
    mov r8d, eax
    lea rdi, [uiBar]
    mov byte [rdi], '['
    inc rdi
    xor ecx, ecx
.cell:
    mov al, '-'
    cmp ecx, r8d
    jae .write
    mov al, '#'
.write:
    stosb
    inc ecx
    cmp ecx, 30
    jb .cell
    mov byte [rdi], ']'
    mov byte [rdi+1], 0
    pop rdi
    ret

ui_start:
    FRAME 32
    mov dword [uiMode], UI_RUNNING
    mov dword [uiStage], 0
    mov dword [uiResultCount], 0
    mov qword [uiElapsed], 0
    mov qword [uiTickSecond], -1
    mov dword [uiTickStage], -1
    mov dword [uiTickStop], -1
    lfence
    rdtsc
    shl rdx, 32
    or rax, rdx
    mov [uiStartTick], rax
    call ui_render
    RETURN

ui_finish:
    FRAME 32
    call ui_elapsed_time
    mov dword [uiMode], UI_RESULT
    call ui_render
    RETURN

; The timer is already preserving extended CPU state before entering here.
ui_tick:
    cmp dword [uiMode], UI_RUNNING
    jne .out
    cmp dword [uiBusy], 0
    jne .out
    FRAME 32
    mov dword [uiBusy], 1
    call ui_elapsed_time
    cmp rax, [uiTickSecond]
    jne .draw
    mov eax, [uiStage]
    cmp eax, [uiTickStage]
    jne .draw
    mov eax, [stopRequested]
    cmp eax, [uiTickStop]
    je .done
.draw:
    mov rax, [uiElapsed]
    mov [uiTickSecond], rax
    mov eax, [uiStage]
    mov [uiTickStage], eax
    mov eax, [stopRequested]
    mov [uiTickStop], eax
    call ui_render_status
.done:
    mov dword [uiBusy], 0
    leave
.out:
    ret

ui_record_result:
    push rsi
    push rdi
    FRAME 48
    mov eax, [resultIndex]
    mov [uiResultCount], eax
    dec eax
    and eax, UI_HISTORY-1
    shl eax, 7
    lea rdi, [uiResults]
    add rdi, rax
    mov eax, [resultIndex]
    mov [rdi], eax
    mov eax, [uiErrorKind]
    mov [rdi+4], eax
    add rdi, 16
    lea rsi, [lineAcc]
    mov ecx, 96
    rep movsb
    call ui_elapsed_time
    mov dword [uiBusy], 1
    mov edx, 12
    mov [rsp+32], edx
.clear_row:
    lea rcx, [uiBlank]
    xor edx, edx
    mov r8d, [rsp+32]
    mov r9d, UI_NORMAL
    call ui_text_at
    inc dword [rsp+32]
    cmp dword [rsp+32], 20
    jbe .clear_row
    call ui_render_rows
    call ui_render_status
    mov dword [uiBusy], 0
    leave
    pop rdi
    pop rsi
    ret

; Label in RCX, maximum in EDX, minimum in ESI, value address in RBX.
ui_edit_prompt:
    push rdi
    FRAME 64
    mov [rsp+32], rcx
    mov [rsp+40], edx
    UI_TEXT uiBlank, 0, 19, UI_NORMAL
    UI_TEXT uiBlank, 0, 20, UI_NORMAL
    UI_TEXT uiBlank, 0, 21, UI_NORMAL
    UI_TEXT uiBlank, 0, 22, UI_NORMAL
    cmp dword [uiEditError], 0
    je .range
    UI_TEXT uiRangeError, 0, 19, UI_ERROR
.range:
    mov rcx, [rsp+32]
    xor edx, edx
    mov r8d, 20
    mov r9d, UI_BRIGHT
    call ui_text_at
    lea rcx, [uiRangeOpen]
    call puts
    mov edx, esi
    call print_number
    lea rcx, [uiRangeTo]
    call puts
    mov edx, [rsp+40]
    call print_number
    lea rcx, [uiRangeClose]
    call puts
    mov edx, [rbx]
    call print_number
    UI_TEXT uiEditActions, 0, 22, UI_NORMAL
    lea rax, [targetRuns]
    cmp rbx, rax
    je .input
    lea rcx, [uiEditMax]
    call puts
.input:
    UI_TEXT uiNewValue, 0, 21, UI_ACCENT
    mov edx, 1
    call ui_cursor_visible
    leave
    pop rdi
    ret

ui_help:
    push rbx
    FRAME 32
    mov ebx, [uiMode]
    mov dword [uiMode], UI_OVERLAY
    call ui_clear
    UI_TEXT szHeader, 0, 0, UI_BRIGHT
    UI_TEXT uiRule, 0, 2, UI_MUTED
    UI_TEXT uiHelpTitle, 0, 4, UI_ACCENT
    UI_TEXT uiHelpSize, 0, 6, UI_NORMAL
    UI_TEXT uiHelpThreads, 0, 8, UI_NORMAL
    UI_TEXT uiHelpRuns, 0, 10, UI_NORMAL
    UI_TEXT uiHelpTime, 0, 12, UI_NORMAL
    UI_TEXT uiHelpResidual, 0, 13, UI_NORMAL
    UI_TEXT uiHelpPassed, 0, 15, UI_NORMAL
    UI_TEXT uiHelpStopped, 0, 16, UI_NORMAL
    UI_TEXT uiHelpStopDelay, 0, 18, UI_NORMAL
    UI_TEXT uiHelpCooling, 0, 19, UI_NORMAL
    UI_TEXT uiBack, 0, 22, UI_ACCENT
.key:
    call read_key
    cmp al, 13
    je .back
    cmp word [inputKey], SCAN_ESC
    je .back
    cmp al, 27
    jne .key
.back:
    mov [uiMode], ebx
    leave
    pop rbx
    ret

ui_advanced:
    push rbx
    FRAME 32
    mov dword [uiMode], UI_OVERLAY
.draw:
    call ui_clear
    UI_TEXT szHeader, 0, 0, UI_BRIGHT
    UI_TEXT uiRule, 0, 2, UI_MUTED
    UI_TEXT uiAdvancedTitle, 0, 4, UI_ACCENT
    UI_TEXT uiAdvancedHint, 0, 6, UI_NORMAL
    UI_TEXT uiModeLabel, 0, 8, UI_NORMAL
    cmp dword [lpIsaForce], 0
    jne .mode
    lea rcx, [uiAutoOpen]
    call puts
.mode:
    call print_isa
    cmp dword [lpIsaForce], 0
    jne .options
    lea rcx, [uiClose]
    call puts
.options:
    UI_TEXT uiAdvancedAuto, 0, 11, UI_BRIGHT
    UI_TEXT uiAdvancedSse2, 0, 13, UI_BRIGHT
    test dword [lpIsa], 2
    jz .no_avx
    UI_TEXT uiAdvancedAvx, 0, 15, UI_BRIGHT
    mov eax, [lpIsa]
    and eax, 12
    cmp eax, 12
    jne .no_avx
    UI_TEXT uiAdvancedAvx2, 0, 17, UI_BRIGHT
.no_avx:
    UI_TEXT uiBack, 0, 22, UI_ACCENT
.key:
    call read_key
    cmp al, 13
    je .back
    cmp word [inputKey], SCAN_ESC
    je .back
    cmp al, 27
    je .back
    or al, 20h
    xor ebx, ebx
    cmp al, 'a'
    je .set
    mov ebx, 1
    cmp al, '1'
    je .set
    mov ebx, 2
    cmp al, '2'
    je .avx
    mov ebx, 3
    cmp al, '3'
    jne .key
    mov eax, [lpIsa]
    and eax, 12
    cmp eax, 12
    jne .unsupported
.avx:
    test dword [lpIsa], 2
    jz .unsupported
.set:
    mov [lpIsaForce], ebx
    call select_isa
    jmp .draw
.unsupported:
    UI_TEXT uiUnavailable, 0, 20, UI_WARNING
    jmp .key
.back:
    mov dword [uiMode], UI_READY
    leave
    pop rbx
    ret

%unmacro UI_TEXT 4
%unmacro UI_NUMBER 3
