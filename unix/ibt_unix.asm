bits 64
default rel

; IntelBurnTest Unix frontend (Linux/x86-64 + X11).
; The numerical engine is included verbatim from the Windows build below.

%define SELFTEST 0
%include "ibt_draw.inc"

global _start

extern XOpenDisplay
extern XDefaultRootWindow
extern XDefaultScreen
extern XDefaultVisual
extern XDefaultColormap
extern XDefaultDepth
extern XCreateSimpleWindow
extern XCreateImage
extern XPutImage
extern XCreateGC
extern XSelectInput
extern XStoreName
extern XInternAtom
extern XSetWMProtocols
extern XSetWMNormalHints
extern XMapWindow
extern XPending
extern XNextEvent
extern XLookupString
extern XSetForeground
extern XFillRectangle
extern XFillArc
extern XDrawRectangle
extern XDrawLine
extern XFillPolygon
extern XFlush
extern XCloseDisplay
extern XftDrawCreate
extern XftFontOpenName
extern XftColorAllocValue
extern XftDrawStringUtf8
extern XftTextExtentsUtf8

extern malloc
extern free
extern posix_memalign
extern pthread_create
extern pthread_join
extern pthread_key_create
extern pthread_getspecific
extern pthread_setspecific
extern clock_gettime
extern sysconf
extern sysinfo
extern usleep
extern system

WM_PHASE       equ 08004h
WM_DONE        equ 08002h
RESULT_READY   equ 0
RESULT_RUNNING equ 1
RESULT_PASS    equ 2
RESULT_FAIL    equ 3
RESULT_STOPPED equ 4

ExposureMask       equ 1 << 15
KeyPressMask       equ 1 << 0
ButtonPressMask    equ 1 << 2
StructureNotifyMask equ 1 << 17

COL_BG       equ 0101318h
COL_PANEL    equ 0171B22h
COL_EDIT     equ 0202630h
COL_FG       equ 0E8EDF4h
COL_MUTED    equ 08B97A8h
COL_BORDER   equ 0343D4Bh
COL_ACCENT   equ 0FF7A33h
COL_DARK     equ 00E1116h
COL_SUCCESS  equ 042D392h
COL_DANGER   equ 0FF5D6Ch
COL_YELLOW   equ 0FFD83Dh
COL_CHECK    equ 060CDFFh
COL_SELECT   equ 0078D4h

section .bss
align 64
xDisplay     resq 1
xWindow      resq 1
xGC          resq 1
xDelete      resq 1
xEvent       resb 192
xHints       resb 80
xScreen      resd 1
xDepth       resd 1
alignb 8
xVisual      resq 1
xColormap    resq 1
xftDraw      resq 1
xftFont      resq 1
xftFontBold  resq 1
xftFontLarge resq 1
xftColors    resb 16*7
flameImage   resq 1
coffeeImage  resq 1
flameFrame   resd 1
flameTick    resd 1
alignb 16
flamePixels  resd 424*50
coffeePixels resd 85*23

uiDirty      resd 1
uiDone       resd 1
uiQuit       resd 1
uiFocus      resd 1
uiAbout      resd 1
uiSmoke      resd 1
uiDrop       resd 1
stressChoice resd 1
threadChoice resd 1
timesValue   resd 1
customMB     resd 1
freeMB       resd 1
logEnabled   resd 1
logFd        resd 1
keyBuf       resb 16
dynBuf       resb 96

; State shared with the original numerical engine.
hMain       resq 1
hLblBits    resq 1
hProc       resq 1
running     resd 1
xtreme      resd 1
haveResid   resd 1
failed      resd 1
passCount   resd 1
targetRuns  resd 1
stressMB    resd 1
availMB     resd 1
nproc       resd 1
lpPcores    resd 1
lpAllocFail resd 1
resultState resd 1
resultIndex resd 1
firstResid  resb 128
lineAcc     resb 128
resultTime  resb 80
resultSpeed resb 80
resultResid resb 80

lpA         resq 1
lpA0        resq 1
lpA1        resq 1
lpX         resq 1
lpX0        resq 1
lpX1        resq 1
lpResidX    resq 1
lpB         resq 1
lpIpiv      resq 1
lpWrk       resq 1
lpPackA     resq 1
lpPackB     resq 1
lpN         resd 1
lpLDA       resd 1
lpNthr      resd 1
lpStop      resd 1
lpT0        resd 1
lpTJ        resd 1
lpTNB       resd 1
lpSwN       resd 1
lpTest      resd 1
lpIsa       resd 1
lpIsaUse    resd 1
lpIsaForce  resd 1
lpTlsIdx    resd 1
alignb 64
lpDie       resd 1
alignb 64
lpPhase     resd 1
alignb 64
lpDone      resd 1
alignb 64
lpNext      resd 1
alignb 64
lpHere      resd 1
alignb 64
lpJob       resd 1
alignb 64
lpCurBuf    resd 1
lpNBuf      resd 1
alignb 8
lpNorm0     resq 1
lpTmpR      resq 1
lpTmpN      resq 1
lpTmpT      resq 1
lpGM        resd 1
lpGN        resd 1
lpGK        resd 1
alignb 8
lpGA        resq 1
lpGB        resq 1
lpGC        resq 1
lpGLDA      resq 1
lpFreq      resq 1
lpAccP      resq 1
lpAccT      resq 1
lpAccG      resq 1
lpAccS      resq 1
lpQ0        resq 1
lpWorkers   resq 32
lpSeen      resd 32
lpWparm     resd 32
tlsKey      resd 1

section .data
align 8
szTitle       db 'IntelBurnTest v3.00 - by Thejuampi',0
szFont        db 'Segoe UI,DejaVu Sans:size=9',0
szFontBold    db 'Segoe UI,DejaVu Sans:bold:size=9',0
szFontLarge   db 'Segoe UI,DejaVu Sans:bold:size=20',0
szDelete      db 'WM_DELETE_WINDOW',0
szSelftest    db '--selftest',0
szSmoketest   db '--smoketest',0
szCoffeeCmd   db 'xdg-open https://buymeacoffee.com/thejuampi >/dev/null 2>&1 &',0

szConfig      db 'TEST CONFIGURATION',0
szMode        db 'Mode:',0
szModeValue   db '64-bit',0
szModeAvx     db '64-bit AVX',0
szModeAvx2    db '64-bit AVX2',0
szStress      db 'Stress:',0
szMB          db 'MB',0
szTimes       db 'Times to run:',0
szThreads     db 'Threads:',0
szFree        db 'Free',0
szLog         db 'Output results to results.log',0
szThermal     db 'THERMAL LOAD',0
szStart       db 'Start',0
szRunAgain    db 'Run again',0
szStop        db 'Stop',0
szAbout       db 'About',0
szCoffee      db 'Coffee',0
szMonitor     db 'STABILITY MONITOR',0
szProgress    db 'RUN PROGRESS',0
szReady       db 'READY',0
szRunning     db 'RUNNING',0
szPassed      db 'PASSED',0
szFailed      db 'FAILED',0
szStopped     db 'STOPPED',0
szRunPrefix   db 'RUN ',0
szReadyStart  db 'READY TO START',0
szWarming     db 'WARMING UP',0
szGflops      db 'GFLOPS',0
szLastRun     db 'LAST RUN',0
szSignature   db 'STABILITY SIGNATURE',0
szAwaiting    db 'Waiting for the first completed run',0
szReference   db 'REFERENCE CAPTURED',0
szConsistent  db 'CONSISTENT',0
szAllMatch    db 'ALL RUNS MATCH',0
szMismatch    db 'RESIDUAL MISMATCH',0
szDash        db '--',0
szSeconds     db 's',0
szSlash       db ' / ',0

szStd         db 'Standard',0
szHigh        db 'High',0
szVeryHigh    db 'Very High',0
szMaximum     db 'Maximum',0
szCustom      db 'Custom',0
stressNames   dq szStd,szHigh,szVeryHigh,szMaximum,szCustom
stressSizes   dd 1024,2048,4096,0,0

szAll         db 'All',0
szAuto        db 'Auto',0
szOne         db '1',0
szTwo         db '2',0
szFour        db '4',0
szEight       db '8',0
szSixteen     db '16',0
szThirtyTwo   db '32',0
threadNames   dq szAll,szAuto,szOne,szTwo,szFour,szEight,szSixteen,szThirtyTwo
threadValues  dd 0,-1,1,2,4,8,16,32

szAboutHead   db 'IntelBurnTest',0
szAbout1      db 'by Thejuampi - original by AgentGOD',0
szAbout2      db 'Native x86-64 calculation core on Unix/X11.',0
szAbout3      db 'Adequate CPU cooling is required.',0
szOK          db 'OK',0

szBitsSse2    db '64-bit SSE2',0
szBitsAvx     db '64-bit AVX',0
szBitsAvx2    db '64-bit AVX2',0
szPreparing   db 'Preparing test data',0

logName       db 'results.log',0
logHeader     db 'IntelBurnTest Unix',10,'Time (s)    Speed (GFlops)    Result',10
logHeaderLen  equ $-logHeader

align 4
flameBmp:
    incbin "../res/flame4.bmp"
coffeeBmp:
    incbin "../res/coffee4.bmp"

; XRenderColor values corresponding to the seven UI text colors.
xftRenderColors:
    dw 0E8E8h,0EDEDh,0F4F4h,0FFFFh
    dw 08B8Bh,09797h,0A8A8h,0FFFFh
    dw 0FFFFh,07A7Ah,03333h,0FFFFh
    dw 00E0Eh,01111h,01616h,0FFFFh
    dw 04242h,0D3D3h,09292h,0FFFFh
    dw 0FFFFh,05D5Dh,06C6Ch,0FFFFh
    dw 0,0,0,0FFFFh

section .text

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

ui_open:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    xor edi, edi
    call XOpenDisplay
    test rax, rax
    jz .fail
    mov [xDisplay], rax
    mov rdi, rax
    call XDefaultRootWindow
    mov rbx, rax
    sub rsp, 32
    mov qword [rsp], 0             ; border width
    mov qword [rsp+8], COL_BORDER
    mov qword [rsp+16], COL_BG
    mov rdi, [xDisplay]
    mov rsi, rbx
    mov edx, 100
    mov ecx, 100
    mov r8d, 620
    mov r9d, 400
    call XCreateSimpleWindow
    add rsp, 32
    test rax, rax
    jz .fail
    mov [xWindow], rax
    mov [hMain], rax
    mov rdi, [xDisplay]
    mov rsi, rax
    xor edx, edx
    xor ecx, ecx
    call XCreateGC
    mov [xGC], rax

    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov edx, ExposureMask|KeyPressMask|ButtonPressMask|StructureNotifyMask
    call XSelectInput
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    lea rdx, [szTitle]
    call XStoreName

    lea rdi, [xHints]
    xor eax, eax
    mov ecx, 10
    rep stosq
    mov qword [xHints], (1<<4)|(1<<5)
    mov dword [xHints+24], 620
    mov dword [xHints+28], 400
    mov dword [xHints+32], 620
    mov dword [xHints+36], 400
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    lea rdx, [xHints]
    call XSetWMNormalHints

    mov rdi, [xDisplay]
    lea rsi, [szDelete]
    xor edx, edx
    call XInternAtom
    mov [xDelete], rax
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    lea rdx, [xDelete]
    mov ecx, 1
    call XSetWMProtocols

    mov rdi, [xDisplay]
    call XDefaultScreen
    mov [xScreen], eax
    mov rdi, [xDisplay]
    mov esi, [xScreen]
    call XDefaultVisual
    mov [xVisual], rax
    mov rdi, [xDisplay]
    mov esi, [xScreen]
    call XDefaultDepth
    mov [xDepth], eax
    mov rdi, [xDisplay]
    mov esi, [xScreen]
    call XDefaultColormap
    mov [xColormap], rax
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xVisual]
    mov rcx, [xColormap]
    call XftDrawCreate
    mov [xftDraw], rax
    mov rdi, [xDisplay]
    mov esi, [xScreen]
    lea rdx, [szFont]
    call XftFontOpenName
    mov [xftFont], rax
    mov rdi, [xDisplay]
    mov esi, [xScreen]
    lea rdx, [szFontBold]
    call XftFontOpenName
    mov [xftFontBold], rax
    mov rdi, [xDisplay]
    mov esi, [xScreen]
    lea rdx, [szFontLarge]
    call XftFontOpenName
    mov [xftFontLarge], rax
    xor ebx, ebx
.colors:
    mov rdi, [xDisplay]
    mov rsi, [xVisual]
    mov rdx, [xColormap]
    lea rcx, [xftRenderColors+rbx*8]
    mov r8, rbx
    shl r8, 4
    lea r8, [xftColors+r8]
    call XftColorAllocValue
    inc ebx
    cmp ebx, 7
    jb .colors
    call assets_init
.map:
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    call XMapWindow
    mov dword [uiDirty], 1
    mov dword [resultState], RESULT_READY
    mov eax, 1
    leave
    ret
.fail:
    xor eax, eax
    leave
    ret

ui_shutdown:
    push rbp
    mov rbp, rsp
    cmp dword [running], 0
    je .close
    call ui_stop
.close:
    mov rdi, [xDisplay]
    test rdi, rdi
    jz .out
    call XCloseDisplay
.out:
    pop rbp
    ret

assets_init:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    lea rdi, [flameBmp]
    lea rsi, [flamePixels]
    call decode_bmp4
    lea rdi, [coffeeBmp]
    lea rsi, [coffeePixels]
    call decode_bmp4

    mov qword [rsp], 424
    mov qword [rsp+8], 50
    mov qword [rsp+16], 32
    mov qword [rsp+24], 0
    mov rdi, [xDisplay]
    mov rsi, [xVisual]
    mov edx, [xDepth]
    mov ecx, 2                    ; ZPixmap
    xor r8d, r8d
    lea r9, [flamePixels]
    call XCreateImage
    mov [flameImage], rax

    mov qword [rsp], 85
    mov qword [rsp+8], 23
    mov qword [rsp+16], 32
    mov qword [rsp+24], 0
    mov rdi, [xDisplay]
    mov rsi, [xVisual]
    mov edx, [xDepth]
    mov ecx, 2
    xor r8d, r8d
    lea r9, [coffeePixels]
    call XCreateImage
    mov [coffeeImage], rax
    leave
    ret

; Decode the 4-bit, bottom-up BMP resources into native 32-bit XImage pixels.
; rdi = BMP, rsi = destination.
decode_bmp4:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8
    mov rbx, rdi
    mov rdi, rsi
    mov r12d, [rbx+18]
    mov r13d, [rbx+22]
    mov eax, r12d
    inc eax
    shr eax, 1
    add eax, 3
    and eax, -4
    mov r14d, eax
    mov edx, [rbx+10]
    lea r15, [rbx+rdx]
    lea r11, [rbx+54]
    xor ecx, ecx
.row:
    mov eax, r13d
    dec eax
    sub eax, ecx
    imul eax, r14d
    lea r9, [r15+rax]
    xor r8d, r8d
.pixel:
    mov eax, r8d
    shr eax, 1
    movzx edx, byte [r9+rax]
    test r8b, 1
    jnz .low
    shr edx, 4
.low:
    and edx, 15
    mov eax, [r11+rdx*4]
    and eax, 00FFFFFFh
    stosd
    inc r8d
    cmp r8d, r12d
    jb .pixel
    inc ecx
    cmp ecx, r13d
    jb .row
    add rsp, 8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

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
fill:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8
    mov ebx, edi
    mov r12d, esi
    mov r13d, edx
    mov r14d, ecx
    mov r15d, r8d
    mov rdi, [xDisplay]
    mov rsi, [xGC]
    mov edx, r15d
    call XSetForeground
    mov dword [rsp], r14d
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov ecx, ebx
    mov r8d, r12d
    mov r9d, r13d
    call XFillRectangle
    add rsp, 8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

outline:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8
    mov ebx, edi
    mov r12d, esi
    mov r13d, edx
    mov r14d, ecx
    mov r15d, r8d
    mov rdi, [xDisplay]
    mov rsi, [xGC]
    mov edx, r15d
    call XSetForeground
    mov dword [rsp], r14d
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov ecx, ebx
    mov r8d, r12d
    mov r9d, r13d
    call XDrawRectangle
    add rsp, 8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; circle_fill(x, y, diameter, color)
circle_fill:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 32
    mov ebx, edi
    mov r12d, esi
    mov r13d, edx
    mov r14d, ecx
    mov rdi, [xDisplay]
    mov rsi, [xGC]
    mov edx, r14d
    call XSetForeground
    mov dword [rsp], r13d
    mov qword [rsp+8], 0
    mov qword [rsp+16], 23040       ; 360 degrees * 64
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov ecx, ebx
    mov r8d, r12d
    mov r9d, r13d
    call XFillArc
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; round_fill(x, y, width, height, color, radius)
round_fill:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 24
    mov ebx, edi
    mov r12d, esi
    mov r13d, edx
    mov r14d, ecx
    mov r15d, r8d
    mov [rsp+16], r9d
    mov eax, r9d
    add eax, eax
    mov [rsp+12], eax

    mov edi, ebx
    add edi, r9d
    mov esi, r12d
    mov edx, r13d
    sub edx, eax
    mov ecx, r14d
    mov r8d, r15d
    call fill
    mov edi, ebx
    mov esi, r12d
    add esi, [rsp+16]
    mov edx, r13d
    mov ecx, r14d
    sub ecx, [rsp+12]
    mov r8d, r15d
    call fill

    mov edi, ebx
    mov esi, r12d
    mov edx, [rsp+12]
    mov ecx, r15d
    call circle_fill
    mov edi, ebx
    add edi, r13d
    sub edi, [rsp+12]
    mov esi, r12d
    mov edx, [rsp+12]
    mov ecx, r15d
    call circle_fill
    mov edi, ebx
    mov esi, r12d
    add esi, r14d
    sub esi, [rsp+12]
    mov edx, [rsp+12]
    mov ecx, r15d
    call circle_fill
    mov edi, ebx
    add edi, r13d
    sub edi, [rsp+12]
    mov esi, r12d
    add esi, r14d
    sub esi, [rsp+12]
    mov edx, [rsp+12]
    mov ecx, r15d
    call circle_fill
    add rsp, 24
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

panel:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    mov ebx, edi
    mov r12d, esi
    mov r13d, edx
    mov r14d, ecx
    mov r8d, COL_BORDER
    mov r9d, 6
    call round_fill
    lea edi, [ebx+1]
    lea esi, [r12d+1]
    lea edx, [r13d-2]
    lea ecx, [r14d-2]
    mov r8d, COL_PANEL
    mov r9d, 5
    call round_fill
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

input_box:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    mov ebx, edi
    mov r12d, esi
    mov r13d, edx
    mov r14d, ecx
    mov r8d, COL_BORDER
    mov r9d, 4
    call round_fill
    lea edi, [ebx+1]
    lea esi, [r12d+1]
    lea edx, [r13d-2]
    lea ecx, [r14d-2]
    mov r8d, COL_EDIT
    mov r9d, 3
    call round_fill
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

draw_chevron:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 32
    mov ebx, edi
    mov r12d, esi
    mov eax, ebx
    sub eax, 3
    mov [rsp+8], ax
    mov eax, r12d
    sub eax, 2
    mov [rsp+10], ax
    mov eax, ebx
    add eax, 3
    mov [rsp+12], ax
    mov eax, r12d
    sub eax, 2
    mov [rsp+14], ax
    mov [rsp+16], bx
    mov eax, r12d
    add eax, 2
    mov [rsp+18], ax
    mov rdi, [xDisplay]
    mov rsi, [xGC]
    mov edx, COL_MUTED
    call XSetForeground
    mov qword [rsp], 0
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    lea rcx, [rsp+8]
    mov r8d, 3
    xor r9d, r9d
    call XFillPolygon
    add rsp, 32
    pop r12
    pop rbx
    pop rbp
    ret

draw_log_check:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov edi, 24
    mov esi, 112
    mov edx, 13
    mov ecx, 13
    mov r8d, COL_CHECK
    mov r9d, 3
    call round_fill
    mov rdi, [xDisplay]
    mov rsi, [xGC]
    mov edx, COL_FG
    call XSetForeground
    mov qword [rsp], 121
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov ecx, 27
    mov r8d, 118
    mov r9d, 30
    call XDrawLine
    mov qword [rsp], 115
    mov rdi, [xDisplay]
    mov rsi, [xWindow]
    mov rdx, [xGC]
    mov ecx, 30
    mov r8d, 121
    mov r9d, 35
    call XDrawLine
    leave
    ret

text:
    mov r8, [xftFont]
    jmp text_font

text_bold:
    mov r8, [xftFontBold]
    jmp text_font

text_large:
    mov r8, [xftFontLarge]

text_font:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 24
    mov ebx, edi
    mov r12d, esi
    mov r13, rdx
    mov r14d, ecx
    mov r15, rdx
    mov [rsp+16], r8
.len:
    cmp byte [r15], 0
    je .got
    inc r15
    jmp .len
.got:
    sub r15, r13
    xor eax, eax
    cmp r14d, COL_MUTED
    jne .c1
    mov eax, 1
    jmp .color
.c1:
    cmp r14d, COL_ACCENT
    jne .c2
    mov eax, 2
    jmp .color
.c2:
    cmp r14d, COL_DARK
    jne .c3
    mov eax, 3
    jmp .color
.c3:
    cmp r14d, COL_SUCCESS
    jne .c4
    mov eax, 4
    jmp .color
.c4:
    cmp r14d, COL_DANGER
    jne .c5
    mov eax, 5
    jmp .color
.c5:
    test r14d, r14d
    jne .color
    mov eax, 6
.color:
    shl rax, 4
    lea rsi, [xftColors+rax]
    mov dword [rsp], r15d
    mov rdi, [xftDraw]
    mov rdx, [rsp+16]
    mov ecx, ebx
    mov r8d, r12d
    mov r9, r13
    call XftDrawStringUtf8
    add rsp, 24
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

measure_text:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 32
    mov rbx, rdi
    mov r12, rsi
    xor ecx, ecx
.len:
    cmp byte [r12+rcx], 0
    je .draw
    inc ecx
    jmp .len
.draw:
    mov rdi, [xDisplay]
    mov rsi, rbx
    mov rdx, r12
    lea r8, [rsp+16]
    call XftTextExtentsUtf8
    movsx eax, word [rsp+24]
    add rsp, 32
    pop r12
    pop rbx
    pop rbp
    ret

text_right:
    mov r8, [xftFont]
    jmp text_right_font
text_right_bold:
    mov r8, [xftFontBold]
    jmp text_right_font
text_right_large:
    mov r8, [xftFontLarge]
text_right_font:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8
    mov ebx, edi
    mov r12d, esi
    mov r13, rdx
    mov r14d, ecx
    mov r15, r8
    mov rdi, r15
    mov rsi, r13
    call measure_text
    mov edi, ebx
    sub edi, eax
    mov esi, r12d
    mov rdx, r13
    mov ecx, r14d
    mov r8, r15
    call text_font
    add rsp, 8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

text_center:
    mov r8, [xftFont]
    jmp text_center_font
text_center_bold:
    mov r8, [xftFontBold]
text_center_font:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8
    mov ebx, edi
    mov r12d, esi
    mov r13, rdx
    mov r14d, ecx
    mov r15, r8
    mov rdi, r15
    mov rsi, r13
    call measure_text
    sar eax, 1
    mov edi, ebx
    sub edi, eax
    mov esi, r12d
    mov rdx, r13
    mov ecx, r14d
    mov r8, r15
    call text_font
    add rsp, 8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; eax=value, rdi=destination. Returns rdi just past the terminating NUL.
u32toa:
    push rbx
    push rcx
    push rdx
    mov ebx, 10
    xor ecx, ecx
    test eax, eax
    jnz .digits
    mov byte [rdi], '0'
    inc rdi
    jmp .term
.digits:
    xor edx, edx
    div ebx
    push rdx
    inc ecx
    test eax, eax
    jnz .digits
.emit:
    pop rax
    add al, '0'
    stosb
    loop .emit
.term:
    mov byte [rdi], 0
    pop rdx
    pop rcx
    pop rbx
    ret

streq:
    xor eax, eax
.next:
    mov dl, [rdi]
    cmp dl, [rsi]
    jne .no
    test dl, dl
    jz .yes
    inc rdi
    inc rsi
    jmp .next
.yes:
    mov eax, 1
.no:
    ret

; ---- UI/core bridge, using the original Win64 internal calling convention --

thread_count:
    mov eax, [threadChoice]
    lea rdx, [threadValues]
    mov eax, [rdx+rax*4]
    test eax, eax
    jz .all
    cmp eax, -1
    je .auto
    cmp eax, [nproc]
    jbe .out
.all:
    mov eax, [nproc]
    ret
.auto:
    mov eax, [lpPcores]
.out:
    ret

title_status:
title_progress:
SetWindowTextW:
SendMessageW:
SetThreadPriority:
WakeByAddressAll:
    mov eax, 1
    ret

PostMessageW:
    cmp edx, WM_DONE
    jne .out
    mov dword [uiDone], 1
    mov dword [uiDirty], 1
.out:
    mov eax, 1
    ret

GetLogicalProcessorInformationEx:
    xor eax, eax
    ret

QueryPerformanceFrequency:
    mov qword [rcx], 1000000000
    mov eax, 1
    ret

QueryPerformanceCounter:
    push rdi
    push rsi
    sub rsp, 24
    mov [rsp+16], rcx
    mov edi, 1                    ; CLOCK_MONOTONIC
    mov rsi, rsp
    call clock_gettime
    mov rdx, [rsp]
    imul rdx, 1000000000
    add rdx, [rsp+8]
    mov rcx, [rsp+16]
    mov [rcx], rdx
    add rsp, 24
    pop rsi
    pop rdi
    mov eax, 1
    ret

VirtualAlloc:
    push rbx
    push rdi
    push rsi
    sub rsp, 32
    mov rbx, rdx
    lea rdi, [rsp+24]
    mov esi, 2097152               ; align large matrices for 2 MiB THPs
    mov rdx, rbx
    call posix_memalign
    test eax, eax
    jnz .fail
    mov rdi, [rsp+24]
    mov rsi, rbx
    mov edx, 14                    ; MADV_HUGEPAGE
    mov eax, 28                    ; madvise
    syscall
    mov rax, [rsp+24]
    jmp .out
.fail:
    xor eax, eax
.out:
    add rsp, 32
    pop rsi
    pop rdi
    pop rbx
    ret

VirtualFree:
    push rdi
    push rsi
    sub rsp, 8
    mov rdi, rcx
    call free
    add rsp, 8
    pop rsi
    pop rdi
    mov eax, 1
    ret

TlsAlloc:
    push rdi
    push rsi
    sub rsp, 8
    lea rdi, [tlsKey]
    xor esi, esi
    call pthread_key_create
    test eax, eax
    jnz .fail
    mov eax, [tlsKey]
    jmp .out
.fail:
    mov eax, -1
.out:
    add rsp, 8
    pop rsi
    pop rdi
    ret

TlsGetValue:
    push rdi
    push rsi
    sub rsp, 8
    mov edi, [tlsKey]
    call pthread_getspecific
    add rsp, 8
    pop rsi
    pop rdi
    ret

TlsSetValue:
    push rdi
    push rsi
    sub rsp, 8
    mov edi, [tlsKey]
    mov rsi, rdx
    call pthread_setspecific
    xor eax, 1
    add rsp, 8
    pop rsi
    pop rdi
    ret

CreateThread:
    push rbx
    push rdi
    push rsi
    sub rsp, 32
    mov [rsp], r8
    mov [rsp+8], r9
    mov edi, 8
    call malloc
    test rax, rax
    jz .fail
    mov rbx, rax
    mov rdi, rax
    xor esi, esi
    mov rdx, [rsp]
    mov rcx, [rsp+8]
    call pthread_create
    test eax, eax
    jnz .free
    mov rax, rbx
    jmp .out
.free:
    mov rdi, rbx
    call free
.fail:
    xor eax, eax
.out:
    add rsp, 32
    pop rsi
    pop rdi
    pop rbx
    ret

WaitForSingleObject:
    push rdi
    push rsi
    sub rsp, 8
    mov rdi, [rcx]
    xor esi, esi
    call pthread_join
    add rsp, 8
    pop rsi
    pop rdi
    ret

CloseHandle:
    push rdi
    push rsi
    sub rsp, 8
    mov rdi, rcx
    call free
    add rsp, 8
    pop rsi
    pop rdi
    ret

isqrt:
    push rbx
    mov r8, rcx
    cmp r8, 2
    jb .small
    mov rax, r8
.loop:
    mov rbx, rax
    mov rax, r8
    xor rdx, rdx
    div rbx
    add rax, rbx
    shr rax, 1
    cmp rax, rbx
    jb .loop
    mov rax, rbx
    pop rbx
    ret
.small:
    mov rax, r8
    pop rbx
    ret

post_line:
    push rbx
    push rsi
    push rdi
    lea rsi, [lineAcc]
    lea rdi, [resultTime]
    call copyz
    lea rsi, [lineAcc+32]
    lea rdi, [resultSpeed]
    call copyz
    lea rsi, [lineAcc+64]
    lea rdi, [resultResid]
    call copyz
    inc dword [resultIndex]
    cmp dword [haveResid], 0
    jne .compare
    lea rsi, [lineAcc+64]
    lea rdi, [firstResid]
    call copyz
    mov dword [haveResid], 1
    jmp .pass
.compare:
    lea rsi, [lineAcc+64]
    lea rdi, [firstResid]
.cmp:
    mov al, [rsi]
    cmp al, [rdi]
    jne .bad
    test al, al
    jz .pass
    inc rsi
    inc rdi
    jmp .cmp
.bad:
    mov dword [failed], 1
    mov dword [lpStop], 1
    mov dword [resultState], RESULT_FAIL
    jmp .logged
.pass:
    inc dword [passCount]
.logged:
    call log_line
    mov dword [uiDirty], 1
    pop rdi
    pop rsi
    pop rbx
    ret

copyz:
    lodsb
    stosb
    test al, al
    jnz copyz
    ret

; Logging uses raw Linux syscalls so it adds no runtime dependency.
log_open:
    cmp dword [logEnabled], 0
    je .out
    mov eax, 257                  ; openat
    mov edi, -100                ; AT_FDCWD
    lea rsi, [logName]
    mov edx, 577                 ; O_WRONLY|O_CREAT|O_TRUNC
    mov r10d, 420                ; 0644
    syscall
    test eax, eax
    js .out
    mov [logFd], eax
    mov edi, eax
    lea rsi, [logHeader]
    mov edx, logHeaderLen
    mov eax, 1
    syscall
.out:
    ret

log_line:
    mov edi, [logFd]
    cmp edi, 0
    jl .out
    lea rsi, [lineAcc]
    call log_string
    lea rsi, [szSpaces]
    call log_string
    lea rsi, [lineAcc+32]
    call log_string
    lea rsi, [szSpaces]
    call log_string
    lea rsi, [lineAcc+64]
    call log_string
    lea rsi, [szNL]
    call log_string
.out:
    ret

log_string:
    push rsi
    xor edx, edx
.len:
    cmp byte [rsi+rdx], 0
    je .write
    inc edx
    jmp .len
.write:
    mov eax, 1
    syscall
    pop rsi
    ret

log_close:
    mov edi, [logFd]
    cmp edi, 0
    jl .out
    mov eax, 3
    syscall
    mov dword [logFd], -1
.out:
    ret

szSpaces db '    ',0
szNL db 10,0

; The same numerical source used by IntelBurnTest.exe.
%include "../ibt_lpk.inc"
%include "../bench_lib.inc"
