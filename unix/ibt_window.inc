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
