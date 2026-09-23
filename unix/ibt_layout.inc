; ---- X11 drawing ---------------------------------------------------------

draw_ui:
    push rbp
    mov rbp, rsp
    mov dword [uiDirty], 0
    DRAW_RECT 0, 0, 620, 400, COL_BG, fill
    DRAW_PANEL 12, 18, 426, 124
    DRAW_PANEL 450, 18, 158, 124
    DRAW_PANEL 12, 154, 596, 234
    call draw_config
    call draw_thermal
    call draw_results
    call draw_assets
    cmp dword [uiDrop], 0
    je .nodrop
    call draw_dropdown
.nodrop:
    cmp dword [uiAbout], 0
    je .flush
    call draw_about
.flush:
    mov rdi, [xDisplay]
    call XFlush
    pop rbp
    ret

draw_config:
    push rbp
    mov rbp, rsp
    DRAW_RECT 20, 8, 166, 18, COL_BG, fill
    DRAW_LABEL 26, 24, szConfig, COL_FG, text_bold
    DRAW_LABEL 24, 57, szMode, COL_FG, text
    mov edi, 66
    mov esi, 57
    lea rdx, [szModeValue]
    cmp dword [lpIsaUse], 1
    jb .mode
    lea rdx, [szModeAvx]
    cmp dword [lpIsaUse], 2
    jb .mode
    lea rdx, [szModeAvx2]
.mode:
    mov ecx, COL_ACCENT
    call text
    DRAW_LABEL 148, 57, szStress, COL_FG, text
    DRAW_INPUT 222, 36, 105, 26
    mov eax, [stressChoice]
    lea rdx, [stressNames]
    mov rdx, [rdx+rax*8]
    DRAW_SELECTED 230, 54, COL_FG, text
    DRAW_CHEVRON 326, 49

    DRAW_INPUT 336, 37, 55, 26
    mov eax, [stressChoice]
    cmp eax, 3
    je .maxmb
    cmp eax, 4
    je .custommb
    lea rdx, [stressSizes]
    mov eax, [rdx+rax*4]
    jmp .mb
.maxmb:
    mov eax, [freeMB]
    sub eax, 256
    jnc .mb
    mov eax, 1
    jmp .mb
.custommb:
    mov eax, [customMB]
.mb:
    lea rdi, [dynBuf]
    call u32toa
    mov edi, 343
    mov esi, 55
    lea rdx, [dynBuf]
    mov ecx, COL_MUTED
    cmp dword [stressChoice], 4
    jne .mbtext
    mov ecx, COL_FG
.mbtext:
    call text
    DRAW_LABEL 397, 55, szMB, COL_FG, text_bold

    DRAW_LABEL 24, 92, szTimes, COL_FG, text
    DRAW_INPUT 100, 74, 48, 26
    mov eax, [timesValue]
    lea rdi, [dynBuf]
    call u32toa
    DRAW_LABEL 107, 92, dynBuf, COL_FG, text

    DRAW_LABEL 164, 92, szThreads, COL_FG, text
    DRAW_INPUT 222, 74, 72, 26
    mov eax, [threadChoice]
    lea rdx, [threadNames]
    mov rdx, [rdx+rax*8]
    DRAW_SELECTED 228, 92, COL_FG, text
    DRAW_CHEVRON 291, 87

    DRAW_LABEL 306, 92, szFree, COL_FG, text
    mov eax, [freeMB]
    lea rdi, [dynBuf]
    call u32toa
    DRAW_LABEL 370, 92, dynBuf, COL_FG, text

    DRAW_ROUND 24, 112, 13, 13, COL_BORDER, 2
    DRAW_ROUND 25, 113, 11, 11, COL_PANEL, 1
    cmp dword [logEnabled], 0
    je .logtxt
    call draw_log_check
.logtxt:
    DRAW_LABEL 43, 124, szLog, COL_FG, text
    pop rbp
    ret

draw_thermal:
    push rbp
    mov rbp, rsp
    DRAW_RECT 456, 8, 126, 18, COL_BG, fill
    DRAW_LABEL 462, 24, szThermal, COL_FG, text_bold
    mov edi, 526
    mov esi, 30
    mov edx, 70
    mov ecx, 31
    mov r8d, COL_ACCENT
    cmp dword [running], 0
    je .startfill
    mov r8d, COL_EDIT
.startfill:
    mov r9d, 5
    call round_fill
    mov edi, 561
    mov esi, 50
    lea rdx, [szStart]
    cmp dword [resultState], RESULT_READY
    jbe .startlabel
    lea rdx, [szRunAgain]
    mov edi, 561
.startlabel:
    mov ecx, COL_DARK
    cmp dword [running], 0
    je .starttxt
    mov ecx, COL_MUTED
.starttxt:
    call text_center
    mov edi, 526
    mov esi, 67
    mov edx, 70
    mov ecx, 29
    mov r8d, COL_EDIT
    cmp dword [running], 0
    je .stopfill
    mov r8d, COL_ACCENT
.stopfill:
    mov r9d, 5
    call round_fill
    mov edi, 561
    mov esi, 86
    lea rdx, [szStop]
    mov ecx, COL_MUTED
    cmp dword [running], 0
    je .stoptxt
    mov ecx, COL_DARK
.stoptxt:
    call text_center
    DRAW_INPUT 462, 105, 43, 26
    DRAW_LABEL 483, 123, szAbout, COL_FG, text_center
    pop rbp
    ret

draw_results:
    push rbp
    mov rbp, rsp
    DRAW_RECT 20, 144, 166, 18, COL_BG, fill
    DRAW_LABEL 26, 160, szMonitor, COL_FG, text_bold
    mov edi, 24
    mov esi, 170
    mov edx, 572
    mov ecx, 204
    mov r8d, COL_BORDER
    cmp dword [resultState], RESULT_PASS
    jne .cardfail
    mov r8d, COL_SUCCESS
    jmp .card
.cardfail:
    cmp dword [resultState], RESULT_FAIL
    jne .card
    mov r8d, COL_DANGER
.card:
    mov r9d, 6
    call round_fill
    DRAW_ROUND 25, 171, 570, 202, COL_EDIT, 5
    DRAW_LABEL 42, 196, szProgress, COL_MUTED, text_bold
    mov eax, [resultState]
    lea rdx, [szReady]
    cmp eax, RESULT_RUNNING
    jne .notrun
    lea rdx, [szRunning]
    jmp .state
.notrun:
    cmp eax, RESULT_PASS
    jne .notpass
    lea rdx, [szPassed]
    jmp .state
.notpass:
    cmp eax, RESULT_FAIL
    jne .notfail
    lea rdx, [szFailed]
    jmp .state
.notfail:
    cmp eax, RESULT_STOPPED
    jne .state
    lea rdx, [szStopped]
.state:
    mov edi, 578
    mov esi, 196
    mov ecx, COL_MUTED
    cmp eax, RESULT_PASS
    jne .statefail
    mov ecx, COL_SUCCESS
    jmp .statetext
.statefail:
    cmp eax, RESULT_FAIL
    jne .statetext
    mov ecx, COL_DANGER
.statetext:
    call text_right_bold
    DRAW_ROUND 42, 214, 536, 8, COL_PANEL, 4
    cmp dword [targetRuns], 0
    je .phase
    mov eax, [passCount]
    test eax, eax
    jz .phase
    imul eax, 536
    xor edx, edx
    div dword [targetRuns]
    cmp eax, 8
    jae .barwidth
    mov eax, 8
.barwidth:
    mov edx, eax
    mov edi, 42
    mov esi, 214
    mov ecx, 8
    mov r8d, COL_ACCENT
    cmp dword [resultState], RESULT_PASS
    jne .barfail
    mov r8d, COL_SUCCESS
    jmp .bar
.barfail:
    cmp dword [resultState], RESULT_FAIL
    jne .bar
    mov r8d, COL_DANGER
.bar:
    mov r9d, 4
    call round_fill
.phase:
    mov edi, 42
    mov esi, 244
    lea rdx, [szReadyStart]
    cmp dword [passCount], 0
    jne .runphase
    cmp dword [resultState], RESULT_RUNNING
    jne .phasetxt
    lea rdx, [szWarming]
.phasetxt:
    mov ecx, COL_FG
    call text
    jmp .metrics
.runphase:
    mov dword [dynBuf], 0204E5552h ; "RUN "
    lea rdi, [dynBuf+4]
    mov eax, [passCount]
    call u32toa
    mov byte [rdi], ' '
    inc rdi
    mov byte [rdi], '/'
    inc rdi
    mov byte [rdi], ' '
    inc rdi
    mov eax, [targetRuns]
    call u32toa
    lea rdx, [dynBuf]
    mov edi, 42
    mov ecx, COL_FG
    call text
.metrics:
    DRAW_LABEL 266, 283, szGflops, COL_MUTED, text_bold
    mov edi, 256
    mov esi, 285
    lea rdx, [szDash]
    cmp byte [resultSpeed], 0
    je .speed
    lea rdx, [resultSpeed]
.speed:
    mov ecx, COL_FG
    call text_right_large
    DRAW_LABEL 578, 258, szLastRun, COL_MUTED, text_right
    mov edi, 578
    mov esi, 283
    lea rdx, [szDash]
    cmp byte [resultTime], 0
    je .time
    lea rdx, [resultTime]
.time:
    cmp byte [resultTime], 0
    je .drawtime
    lea rsi, [resultTime]
    lea rdi, [dynBuf]
    call copyz
    dec rdi
    mov byte [rdi], ' '
    mov byte [rdi+1], 's'
    mov byte [rdi+2], 0
    lea rdx, [dynBuf]
.drawtime:
    mov edi, 578
    mov esi, 283
    mov ecx, COL_FG
    call text_right_bold
    cmp byte [resultTime], 0
    je .signature
.signature:
    DRAW_LABEL 42, 323, szSignature, COL_MUTED, text_bold
    mov edi, 42
    mov esi, 354
    lea rdx, [szAwaiting]
    cmp dword [haveResid], 0
    je .sigstatus
    lea rdx, [resultResid]
    mov ecx, COL_FG
    call text
    mov edi, 578
    mov esi, 354
    lea rdx, [szReference]
.sigstatus:
    cmp dword [failed], 0
    jne .mismatch
    cmp dword [haveResid], 0
    je .sig
    lea rdx, [szReference]
    cmp dword [resultState], RESULT_PASS
    jne .notcomplete
    lea rdx, [szAllMatch]
    jmp .sig
.notcomplete:
    cmp dword [passCount], 1
    jbe .sig
    lea rdx, [szConsistent]
    jmp .sig
.mismatch:
    mov edi, 578
    lea rdx, [szMismatch]
.sig:
    mov ecx, COL_MUTED
    cmp dword [resultState], RESULT_RUNNING
    jne .sigpass
    mov ecx, COL_ACCENT
    jmp .sigtext
.sigpass:
    cmp dword [resultState], RESULT_PASS
    jne .sigfail
    mov ecx, COL_SUCCESS
    jmp .sigtext
.sigfail:
    cmp dword [failed], 0
    je .sigtext
    mov ecx, COL_DANGER
.sigtext:
    cmp dword [haveResid], 0
    je .sigleft
    call text_right_bold
    jmp .sigdone
.sigleft:
    call text
.sigdone:
    pop rbp
    ret

draw_dropdown:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    cmp dword [uiDrop], 1
    jne .threads
    mov ebx, 5
    mov r12d, 222
    mov r13d, 62
    mov r14d, 105
    lea r10, [stressNames]
    mov r11d, [stressChoice]
    jmp .box
.threads:
    mov ebx, 8
    mov r12d, 222
    mov r13d, 100
    mov r14d, 72
    lea r10, [threadNames]
    mov r11d, [threadChoice]
.box:
    push r10
    push r11
    mov edi, r12d
    mov esi, r13d
    mov edx, r14d
    mov ecx, ebx
    imul ecx, 20
    add ecx, 2
    mov r8d, COL_BORDER
    call fill
    lea edi, [r12d+1]
    lea esi, [r13d+1]
    lea edx, [r14d-2]
    mov ecx, ebx
    imul ecx, 20
    mov r8d, COL_EDIT
    call fill
    pop r11
    pop r10
    mov eax, r11d
    imul eax, 20
    mov esi, r13d
    add esi, eax
    inc esi
    lea edi, [r12d+1]
    lea edx, [r14d-2]
    mov ecx, 20
    mov r8d, COL_SELECT
    push r10
    push r11
    call fill
    pop r11
    pop r10
    xor r11d, r11d
.row:
    mov rdx, [r10+r11*8]
    lea edi, [r12d+4]
    mov eax, r11d
    imul eax, 20
    mov esi, r13d
    add esi, eax
    add esi, 16
    mov ecx, COL_FG
    push r10
    push r11
    call text
    pop r11
    pop r10
    inc r11d
    cmp r11d, ebx
    jb .row
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

draw_assets:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov eax, [flameFrame]
    imul eax, 53
    mov qword [rsp], 462
    mov qword [rsp+8], 42
    mov qword [rsp+16], 53
    mov qword [rsp+24], 50
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov rcx, [flameImage]
    mov r8d, eax
    xor r9d, r9d
    call XPutImage

    mov qword [rsp], 511
    mov qword [rsp+8], 107
    mov qword [rsp+16], 85
    mov qword [rsp+24], 23
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov rcx, [coffeeImage]
    xor r8d, r8d
    xor r9d, r9d
    call XPutImage
    leave
    ret

draw_about:
    push rbp
    mov rbp, rsp
    DRAW_RECT 105, 90, 410, 240, COL_PANEL, fill
    DRAW_RECT 105, 90, 410, 240, COL_BORDER, outline
    DRAW_LABEL 132, 128, szAboutHead, COL_ACCENT, text
    DRAW_LABEL 132, 165, szAbout1, COL_FG, text
    DRAW_LABEL 132, 195, szAbout2, COL_MUTED, text
    DRAW_LABEL 132, 225, szAbout3, COL_MUTED, text
    DRAW_RECT 270, 294, 80, 34, COL_ACCENT, fill
    DRAW_LABEL 301, 317, szOK, COL_DARK, text
    pop rbp
    ret

; SysV helpers: fill(x,y,w,h,color), outline(x,y,w,h,color), text(x,y,s,color).
