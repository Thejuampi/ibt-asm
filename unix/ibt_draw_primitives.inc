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
