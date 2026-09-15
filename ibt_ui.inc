wndproc:
    cmp edx, WM_DESTROY
    je .destroy
    cmp edx, WM_PAINT
    je .paint
    cmp edx, WM_CLOSE
    je .close
    cmp edx, WM_COMMAND
    je .cmd
    cmp edx, WM_TIMER
    je .timer
    cmp edx, WM_CTLCOLORSTATIC
    je .color
    cmp edx, WM_CTLCOLORBTN
    je .color
    cmp edx, WM_CTLCOLOREDIT
    je .color
    cmp edx, WM_CTLCOLORLISTBOX
    je .color
    cmp edx, WM_ERASEBKGND
    je .erase
    cmp edx, WM_DRAWITEM
    je .draw
    cmp edx, WM_CONTEXTMENU
    je .ctx
    cmp edx, WM_LINE
    je .line
    cmp edx, WM_DONE
    je .done
    cmp edx, WM_PHASE
    je .phase
    cmp edx, WM_SETCURSOR
    je .setcur
    jmp DefWindowProcW

.paint:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 88h
    mov [rsp+78h], rcx
    lea rdx, [rsp+30h]
    call BeginPaint
    mov rbx, rax
    xor ecx, ecx
    mov edx, 1
    mov r8d, COL_BORDER
    call CreatePen
    mov r12, rax
    mov rcx, rbx
    mov rdx, r12
    call SelectObject
    mov r13, rax
    mov rcx, rbx
    mov rdx, [hBrPanel]
    call SelectObject
    mov rcx, rbx
    mov edx, 12
    mov r8d, 18
    mov r9d, 438
    mov qword [rsp+20h], 142
    call round_rect_panel
    mov rcx, rbx
    mov edx, 450
    mov r8d, 18
    mov r9d, 608
    mov qword [rsp+20h], 142
    call round_rect_panel
    mov rcx, rbx
    mov edx, 12
    mov r8d, 154
    mov r9d, 608
    mov qword [rsp+20h], 388
    call round_rect_panel
    mov rcx, rbx
    mov rdx, r13
    call SelectObject
    mov rcx, r12
    call DeleteObject
    mov rcx, [rsp+78h]
    lea rdx, [rsp+30h]
    call EndPaint
    xor eax, eax
    add rsp, 88h
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.destroy:
    push rbx
    sub rsp, 20h
    mov rbx, rcx
    mov edx, 1
    call KillTimer
    mov rcx, rbx
    mov edx, 2
    call KillTimer
    call bitmaps_free
    call theme_free
    xor ecx, ecx
    call PostQuitMessage
    xor eax, eax
    add rsp, 20h
    pop rbx
    ret

.close:
    push rbx
    sub rsp, 20h
    mov rbx, rcx
    cmp dword [running], 0
    je .c2
    call stop_burn
.c2:
    mov rcx, rbx
    call DestroyWindow
    xor eax, eax
    add rsp, 20h
    pop rbx
    ret

.cmd:
    sub rsp, 28h
    mov eax, r8d
    movzx ecx, ax
    shr eax, 16
    cmp ecx, IDC_START
    je .start
    cmp ecx, IDC_STOP
    je .stop
    cmp ecx, IDC_ABOUT
    je .about
    cmp ecx, IDC_PAYPAL
    je .coffee
    cmp ecx, IDC_STRESS
    je .cmbstress
    cmp ecx, IDC_THREADS
    je .cmbthr
    jmp .cmdend
.cmbstress:
    cmp eax, CBN_SELCHANGE
    je .stresschg
    cmp eax, CBN_DROPDOWN
    je .cmbdrop
    jmp .cmdend
.cmbthr:
    cmp eax, CBN_DROPDOWN
    je .cmbdrop
    jmp .cmdend
.cmbdrop:
    call theme_droplist
    jmp .cmdend
.start:
    call start_burn
    jmp .cmdend
.stop:
    call stop_burn
    jmp .cmdend
.about:
    call show_about
    jmp .cmdend
.stresschg:
    call stress_toggle_custom
    jmp .cmdend
.coffee:
    call open_coffee
    jmp .cmdend
.cmdend:
    xor eax, eax
    add rsp, 28h
    ret

.timer:
    sub rsp, 28h
    cmp r8d, 1
    je .flame
    cmp r8d, 2
    je .ram
    xor eax, eax
    add rsp, 28h
    ret
.flame:
    cmp dword [running], 0
    je .flamedone
    call flame_tick
.flamedone:
    xor eax, eax
    add rsp, 28h
    ret
.ram:
    call ram_update
    xor eax, eax
    add rsp, 28h
    ret

.phase:
    sub rsp, 28h
    call GetCurrentThread
    mov rcx, rax
    xor edx, edx
    test r8, r8
    jnz .pset
    mov edx, 1
.pset:
    call SetThreadPriority
    xor eax, eax
    add rsp, 28h
    ret

.setcur:
    cmp r8, [hCoffeeWnd]
    jne .setdef
    sub rsp, 28h
    xor ecx, ecx
    mov edx, IDC_HAND
    call LoadCursorW
    mov rcx, rax
    call SetCursor
    mov eax, 1
    add rsp, 28h
    ret
.setdef:
    jmp DefWindowProcW

.erase:
    push rbx
    push r12
    sub rsp, 38h
    mov rbx, rcx
    mov r12, r8
    lea rdx, [rsp+20h]
    call GetClientRect
    mov rcx, r12
    lea rdx, [rsp+20h]
    mov r8, [hBrBg]
    test r8, r8
    jnz .ef
    mov ecx, COLOR_BTNFACE
    call GetSysColorBrush
    mov r8, rax
    mov rcx, r12
    lea rdx, [rsp+20h]
.ef:
    call FillRect
    mov eax, 1
    add rsp, 38h
    pop r12
    pop rbx
    ret

.color:
    push rbx
    push r12
    sub rsp, 28h
    mov rbx, r8
    mov r12d, COL_FG
    cmp r9, [hLblBits]
    jne .color_type
    mov r12d, COL_ACCENT
.color_type:
    cmp edx, WM_CTLCOLOREDIT
    je .ced
    cmp edx, WM_CTLCOLORLISTBOX
    je .ced
    mov rcx, rbx
    mov edx, TRANSPARENT
    call SetBkMode
    mov rcx, rbx
    mov edx, r12d
    call SetTextColor
    mov rax, [hBrPanel]
    jmp .cdo
.ced:
    mov rcx, rbx
    mov edx, OPAQUE
    call SetBkMode
    mov rcx, rbx
    mov edx, COL_FG
    call SetTextColor
    mov rcx, rbx
    mov edx, COL_EDIT
    call SetBkColor
    mov rax, [hBrEdit]
    jmp .cdo
.cdo:
    add rsp, 28h
    pop r12
    pop rbx
    ret

.draw:
    mov eax, [r9+4]
    cmp eax, IDC_START
    je .drawbtn
    cmp eax, IDC_STOP
    je .drawbtn
    cmp eax, IDC_ABOUT
    je .drawbtn
    cmp eax, IDC_STRESS
    je .drawcmb
    cmp eax, IDC_THREADS
    je .drawcmb
    jmp .drawdef
.drawbtn:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 88h
    mov rbx, r9
    mov qword [rsp+78h], 0
    mov r8, [hBrEdit]
    mov eax, [rbx+4]
    cmp eax, IDC_START
    je .primaryfill
    cmp eax, IDC_STOP
    jne .normalfill
.primaryfill:
    test dword [rbx+16], 4
    jnz .normalfill
    mov ecx, COL_ACCENT
    test dword [rbx+16], 1
    jz .accentfill
    mov ecx, COL_ACCENT_DOWN
.accentfill:
    call CreateSolidBrush
    mov [rsp+78h], rax
    mov r8, rax
    jmp .fillbtn
.normalfill:
    test dword [rbx+16], 1
    jz .fillbtn
    mov r8, [hBrPanel]
.fillbtn:
    mov rcx, [rbx+32]
    lea rdx, [rbx+40]
    call FillRect
    mov rcx, [rsp+78h]
    test rcx, rcx
    jz .border
    call DeleteObject
.border:
    xor ecx, ecx
    mov edx, 1
    mov r8d, COL_BORDER
    mov eax, [rbx+4]
    cmp eax, IDC_START
    je .primarypen
    cmp eax, IDC_STOP
    jne .bpn
.primarypen:
    test dword [rbx+16], 4
    jnz .bpn
    mov r8d, COL_ACCENT
.bpn:
    call CreatePen
    mov r12, rax
    mov rcx, [rbx+32]
    mov rdx, r12
    call SelectObject
    mov r13, rax
    mov ecx, NULL_BRUSH
    call GetStockObject
    mov rcx, [rbx+32]
    mov rdx, rax
    call SelectObject
    mov rcx, [rbx+32]
    mov edx, [rbx+40]
    mov r8d, [rbx+44]
    mov r9d, [rbx+48]
    mov eax, [rbx+52]
    mov [rsp+20h], rax
    call round_rect_control
    mov rcx, [rbx+32]
    mov rdx, r13
    call SelectObject
    mov rcx, r12
    call DeleteObject
    mov rcx, [rbx+24]
    lea rdx, [rsp+40h]
    mov r8d, 32
    call GetWindowTextW
    mov rcx, [rbx+32]
    mov edx, TRANSPARENT
    call SetBkMode
    mov rcx, [rbx+32]
    mov edx, COL_FG
    mov eax, [rbx+4]
    cmp eax, IDC_START
    je .primarytext
    cmp eax, IDC_STOP
    jne .disabledtext
.primarytext:
    test dword [rbx+16], 4
    jnz .disabledtext
    mov edx, COL_ACCENT_TEXT
.disabledtext:
    test dword [rbx+16], 4
    jz .tc
    mov edx, COL_MUTED
.tc:
    call SetTextColor
    mov rcx, [rbx+32]
    mov rdx, [hFont]
    call SelectObject
    mov r12, rax
    mov rcx, [rbx+32]
    lea rdx, [rsp+40h]
    mov r8d, -1
    lea r9, [rbx+40]
    mov qword [rsp+20h], DT_CENTER|DT_VCENTER|DT_SINGLELINE
    call DrawTextW
    mov rcx, [rbx+32]
    mov rdx, r12
    call SelectObject
    test dword [rbx+16], 10h
    jz .bd
    mov rcx, [rbx+32]
    lea rdx, [rbx+40]
    call DrawFocusRect
.bd:
    mov eax, 1
    add rsp, 88h
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
.drawcmb:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 0A0h
    mov rbx, r9
    test dword [rbx+16], 1000h
    jnz .cdone
    mov r8, [hBrEdit]
    test dword [rbx+16], 1
    jz .cf
    mov ecx, COL_ACCENT
    call CreateSolidBrush
    mov r12, rax
    mov r8, rax
    jmp .cf2
.cf:
    xor r12, r12
.cf2:
    test r8, r8
    jnz .cf3
    mov ecx, COLOR_WINDOW
    call GetSysColorBrush
    mov r8, rax
.cf3:
    mov rcx, [rbx+32]
    lea rdx, [rbx+40]
    call FillRect
    test r12, r12
    jz .ct
    mov rcx, r12
    call DeleteObject
.ct:
    mov eax, [rbx+8]
    cmp eax, 0FFFFFFFFh
    je .cdone
    mov rcx, [rbx+24]
    mov edx, CB_GETLBTEXT
    mov r8d, eax
    lea r9, [rsp+40h]
    call SendMessageW
    mov rcx, [rbx+32]
    mov edx, TRANSPARENT
    call SetBkMode
    mov rcx, [rbx+32]
    mov edx, COL_FG
    test dword [rbx+16], 1
    jz .cc
    mov edx, 00FFFFFFh
.cc:
    call SetTextColor
    mov rcx, [rbx+32]
    mov rdx, [hFont]
    call SelectObject
    mov r12, rax
    add dword [rbx+40], 4
    mov rcx, [rbx+32]
    lea rdx, [rsp+40h]
    mov r8d, -1
    lea r9, [rbx+40]
    mov qword [rsp+20h], DT_LEFT|DT_VCENTER|DT_SINGLELINE
    call DrawTextW
    mov rcx, [rbx+32]
    mov rdx, r12
    call SelectObject
.cdone:
    mov eax, 1
    add rsp, 0A0h
    pop r12
    pop rbx
    pop rbp
    ret
.drawdef:
    jmp DefWindowProcW

.ctx:
    push rbx
    sub rsp, 20h
    mov rbx, r9
    mov rcx, r8
    call GetDlgCtrlID
    mov ecx, eax
    mov rdx, rbx
    call menu_context
    xor eax, eax
    add rsp, 20h
    pop rbx
    ret

.line:
    sub rsp, 28h
    mov rcx, r9
    call on_line
    xor eax, eax
    add rsp, 28h
    ret

.done:
    sub rsp, 28h
    call on_done
    xor eax, eax
    add rsp, 28h
    ret

dialog_register:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    lea rcx, [dlgcls]
    mov edx, 80
    call memzero
    xor ecx, ecx
    mov edx, IDC_ARROW
    call LoadCursorW
    lea rcx, [dlgcls]
    mov dword [rcx], 80
    mov dword [rcx+4], 3
    lea rdx, [dialog_proc]
    mov [rcx+8], rdx
    mov rdx, [hInst]
    mov [rcx+24], rdx
    mov [rcx+40], rax
    lea rdx, [szDlgClass]
    mov [rcx+64], rdx
    call RegisterClassExW
    leave
    ret

dialog_proc:
    cmp edx, WM_DESTROY
    je .destroy
    cmp edx, WM_CLOSE
    je .close
    cmp edx, WM_COMMAND
    je .command
    cmp edx, WM_ERASEBKGND
    je .erase
    cmp edx, WM_CTLCOLORSTATIC
    je .color
    cmp edx, WM_CTLCOLORBTN
    je .color
    jmp DefWindowProcW
.destroy:
    mov qword [hDlg], 0
    xor eax, eax
    ret
.close:
    mov dword [dlgResult], IDCANCEL
    mov rcx, [hDlg]
    jmp DestroyWindow
.command:
    mov eax, r8d
    and eax, 0FFFFh
    cmp eax, IDOK
    je .finish
    cmp eax, IDCANCEL
    jne .handled
.finish:
    mov [dlgResult], eax
    mov rcx, [hDlg]
    call DestroyWindow
.handled:
    xor eax, eax
    ret
.erase:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    mov [rbp-8], r8
    lea rdx, [rbp-20h]
    call GetClientRect
    mov rcx, [rbp-8]
    lea rdx, [rbp-20h]
    mov r8, [hBrPanel]
    call FillRect
    mov eax, 1
    leave
    ret
.color:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov [rbp-8], r8
    mov rcx, [rbp-8]
    mov edx, TRANSPARENT
    call SetBkMode
    mov rcx, [rbp-8]
    mov edx, COL_FG
    call SetTextColor
    mov rax, [hBrPanel]
    leave
    ret

dark_message_box:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    sub rsp, 0A0h
    mov rbx, rcx
    mov rsi, rdx
    mov rdi, r8
    mov r12d, r9d
    mov [dlgTextPtr], rsi
    mov [dlgFlags], r12d
    mov dword [dlgResult], IDCANCEL
    test rbx, rbx
    jz .sized
    mov rcx, rbx
    xor edx, edx
    call EnableWindow
.sized:
    mov dword [rsp+60h], 0
    mov dword [rsp+64h], 0
    mov dword [rsp+68h], 500
    mov dword [rsp+6Ch], 250
    lea rcx, [rsp+60h]
    mov edx, WS_POPUP|WS_CAPTION|WS_SYSMENU
    xor r8d, r8d
    mov r9d, WS_EX_DLGMODALFRAME
    call AdjustWindowRectEx
    mov eax, [rsp+68h]
    sub eax, [rsp+60h]
    movsxd rax, eax
    mov [rsp+70h], rax
    mov eax, [rsp+6Ch]
    sub eax, [rsp+64h]
    movsxd rax, eax
    mov [rsp+78h], rax
    xor ecx, ecx
    call GetSystemMetrics
    sub eax, [rsp+70h]
    sar eax, 1
    movsxd rax, eax
    mov [rsp+80h], rax
    mov ecx, 1
    call GetSystemMetrics
    sub eax, [rsp+78h]
    sar eax, 1
    movsxd rax, eax
    mov [rsp+88h], rax
    mov rax, [rsp+80h]
    mov [rsp+20h], rax
    mov rax, [rsp+88h]
    mov [rsp+28h], rax
    mov rax, [rsp+70h]
    mov [rsp+30h], rax
    mov rax, [rsp+78h]
    mov [rsp+38h], rax
    mov [rsp+40h], rbx
    mov qword [rsp+48h], 0
    mov rax, [hInst]
    mov [rsp+50h], rax
    mov qword [rsp+58h], 0
    mov ecx, WS_EX_DLGMODALFRAME
    lea rdx, [szDlgClass]
    mov r8, rdi
    mov r9d, WS_POPUP|WS_CAPTION|WS_SYSMENU
    call CreateWindowExW
    mov [hDlg], rax
    test rax, rax
    jz .restore
    mov rcx, rax
    call theme_window

    xor ecx, ecx
    lea rdx, [szStatic]
    mov r8, rsi
    mov r9d, STYLE_LBL
    mov qword [rsp+20h], 24
    mov qword [rsp+28h], 26
    mov qword [rsp+30h], 452
    mov qword [rsp+38h], 154
    mov rax, [hDlg]
    mov [rsp+40h], rax
    mov qword [rsp+48h], 10
    mov rax, [hInst]
    mov [rsp+50h], rax
    mov qword [rsp+58h], 0
    call CreateWindowExW
    mov rcx, rax
    call apply_font

    mov eax, 394
    test r12d, MB_OKCANCEL
    jz .okpos
    mov eax, 300
.okpos:
    movsxd rax, eax
    mov [rsp+20h], rax
    mov qword [rsp+28h], 200
    mov qword [rsp+30h], 84
    mov qword [rsp+38h], 32
    mov rax, [hDlg]
    mov [rsp+40h], rax
    mov qword [rsp+48h], IDOK
    mov rax, [hInst]
    mov [rsp+50h], rax
    mov qword [rsp+58h], 0
    xor ecx, ecx
    lea rdx, [szBtn]
    lea r8, [szOK]
    mov r9d, STYLE_DEF
    call CreateWindowExW
    mov rcx, rax
    call dialog_button_init

    test r12d, MB_OKCANCEL
    jz .children
    mov qword [rsp+20h], 394
    mov qword [rsp+28h], 200
    mov qword [rsp+30h], 84
    mov qword [rsp+38h], 32
    mov rax, [hDlg]
    mov [rsp+40h], rax
    mov qword [rsp+48h], IDCANCEL
    mov rax, [hInst]
    mov [rsp+50h], rax
    mov qword [rsp+58h], 0
    xor ecx, ecx
    lea rdx, [szBtn]
    lea r8, [szCancel]
    mov r9d, STYLE_BTN
    call CreateWindowExW
    mov rcx, rax
    call dialog_button_init
.children:
    mov rcx, [hDlg]
    lea rdx, [theme_onchild]
    xor r8d, r8d
    call EnumChildWindows
    mov rcx, [hDlg]
    mov edx, SW_SHOW
    call ShowWindow
    mov rcx, [hDlg]
    call UpdateWindow
    mov rcx, [hDlg]
    call SetForegroundWindow
.loop:
    cmp qword [hDlg], 0
    je .restore
    lea rcx, [msg]
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call GetMessageW
    test eax, eax
    jz .restore
    mov rcx, [hDlg]
    lea rdx, [msg]
    call IsDialogMessageW
    test eax, eax
    jnz .loop
    lea rcx, [msg]
    call TranslateMessage
    lea rcx, [msg]
    call DispatchMessageW
    jmp .loop
.restore:
    test rbx, rbx
    jz .result
    mov rcx, rbx
    mov edx, 1
    call EnableWindow
    mov rcx, rbx
    call SetForegroundWindow
.result:
    mov eax, [dlgResult]
    add rsp, 0A0h
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret


ui_create:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    sub rsp, 60h
    mov rbx, [hMain]
    mov rsi, [hInst]
    lea rdi, [uiControls]
    mov r14, rdi
    mov r13d, (uiControlsEnd-uiControls)/28
.next:
    movzx eax, byte [rdi]
    lea rdx, [uiClasses]
    movsxd rdx, dword [rdx+rax*4]
    add rdx, r14
    movsxd r8, dword [rdi+24]
    add r8, r14
    mov r9d, [rdi+16]
    mov ecx, [rdi+20]
    movzx eax, word [rdi+6]
    mov [rsp+20h], rax
    movzx eax, word [rdi+8]
    mov [rsp+28h], rax
    movzx eax, word [rdi+10]
    mov [rsp+30h], rax
    movzx eax, word [rdi+12]
    mov [rsp+38h], rax
    mov [rsp+40h], rbx
    movzx eax, word [rdi+4]
    mov [rsp+48h], rax
    mov [rsp+50h], rsi
    mov qword [rsp+58h], 0
    call CreateWindowExW
    mov r12, rax
    movzx eax, byte [rdi+2]
    cmp eax, UIS_NONE
    je .action
    lea rcx, [hUiSlots]
    mov [rcx+rax*8], r12
.action:
    movzx eax, byte [rdi+1]
    test eax, UIF_COFFEE
    jnz .coffee
    test eax, UIF_SHOW
    jnz .show
    mov rcx, r12
    test eax, UIF_BOLD
    jnz .bold
    call apply_font
    jmp .advance
.bold:
    call apply_font_bold
    jmp .advance
.show:
    mov rcx, r12
    mov edx, SW_SHOW
    call ShowWindow
    jmp .advance
.coffee:
    mov rcx, r12
    call coffee_init
.advance:
    add rdi, 28
    dec r13d
    jnz .next
    add rsp, 60h
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

apply_font:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov r8, [hFont]
    mov edx, WM_SETFONT
    mov r9d, 1
    call SendMessageW
    leave
    ret

apply_font_bold:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov r8, [hFontBold]
    mov edx, WM_SETFONT
    mov r9d, 1
    call SendMessageW
    leave
    ret

ui_populate:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 38h
    mov rbx, [hCmbStress]
    mov rcx, rbx
    mov edx, CB_ADDSTRING
    xor r8d, r8d
    lea r9, [szStd]
    call SendMessageW
    mov rcx, rbx
    mov edx, CB_ADDSTRING
    xor r8d, r8d
    lea r9, [szHigh]
    call SendMessageW
    mov rcx, rbx
    mov edx, CB_ADDSTRING
    xor r8d, r8d
    lea r9, [szVHigh]
    call SendMessageW
    mov rcx, rbx
    mov edx, CB_ADDSTRING
    xor r8d, r8d
    lea r9, [szMax]
    call SendMessageW
    mov rcx, rbx
    mov edx, CB_ADDSTRING
    xor r8d, r8d
    lea r9, [szCustom]
    call SendMessageW
    mov rcx, rbx
    mov edx, CB_SETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW

    mov rbx, [hCmbThr]
    lea r9, [szAll]
    call add_thr
    lea r9, [szAuto]
    call add_thr
    lea r9, [szT1]
    call add_thr
    lea r9, [szT2]
    call add_thr
    lea r9, [szT3]
    call add_thr
    lea r9, [szT4]
    call add_thr
    lea r9, [szT6]
    call add_thr
    lea r9, [szT8]
    call add_thr
    lea r9, [szT12]
    call add_thr
    lea r9, [szT16]
    call add_thr
    lea r9, [szT24]
    call add_thr
    lea r9, [szT32]
    call add_thr
    lea r9, [szT48]
    call add_thr
    lea r9, [szT64]
    call add_thr
    lea r9, [szT96]
    call add_thr
    lea r9, [szT128]
    call add_thr
    mov rcx, rbx
    mov edx, CB_SETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    add rsp, 38h
    pop rbx
    pop rbp
    ret

add_thr:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov rcx, rbx
    mov edx, CB_ADDSTRING
    xor r8d, r8d
    call SendMessageW
    leave
    ret

stress_toggle_custom:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 38h
    mov rcx, [hCmbStress]
    mov edx, CB_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov ebx, eax
    xor edx, edx
    cmp ebx, 4
    sete dl
    mov rcx, [hEdtMB]
    call EnableWindow
    call stress_mb
    cmp ebx, 4
    je .out
    lea rcx, [tmpBuf]
    lea rdx, [szNumFmt]
    mov r8d, [stressMB]
    call wsprintfW
    mov rcx, [hEdtMB]
    lea rdx, [tmpBuf]
    call SetWindowTextW
.out:
    add rsp, 38h
    pop rbx
    pop rbp
    ret

fonts_create:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    lea rcx, [lf]
    mov edx, LOGFONTW_size
    call memzero
    lea rax, [lf]
    mov dword [rax+LOGFONTW.lfHeight], -14
    mov dword [rax+LOGFONTW.lfWeight], FW_NORMAL
    mov byte [rax+LOGFONTW.lfCharSet], DEFAULT_CHARSET
    mov byte [rax+LOGFONTW.lfQuality], CLEARTYPE_QUALITY
    lea rcx, [rax+LOGFONTW.lfFaceName]
    lea rdx, [szFace]
    call lstrcpyW
    lea rcx, [lf]
    call CreateFontIndirectW
    mov [hFont], rax
    lea rax, [lf]
    mov dword [rax+LOGFONTW.lfWeight], FW_BOLD
    lea rcx, [lf]
    call CreateFontIndirectW
    mov [hFontBold], rax
    leave
    ret

bitmaps_load:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 38h
    mov rcx, [hInst]
    mov edx, 1
    mov r8d, IMAGE_ICON
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], LR_DEFAULTSIZE
    call LoadImageW
    mov [hIcon], rax
    xor ebx, ebx
.floop:
    mov rcx, [hInst]
    lea edx, [ebx+200]
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 2000h
    call LoadImageW
    lea rcx, [hFlame]
    mov [rcx+rbx*8], rax
    inc ebx
    cmp ebx, 2
    jb .floop
    add rsp, 38h
    pop rbx
    pop rbp
    ret

bitmaps_free:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 28h
    mov rcx, [hFont]
    test rcx, rcx
    jz .n1
    call DeleteObject
.n1:
    mov rcx, [hFontBold]
    test rcx, rcx
    jz .n2
    call DeleteObject
.n2:
    xor ebx, ebx
.fl:
    lea rax, [hFlame]
    mov rcx, [rax+rbx*8]
    test rcx, rcx
    jz .sk
    call DeleteObject
.sk:
    inc ebx
    cmp ebx, 2
    jb .fl
    add rsp, 28h
    pop rbx
    pop rbp
    ret

flame_show:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov rcx, [hFlameWnd]
    test rcx, rcx
    jz .out
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
.out:
    leave
    ret

flame_tick:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    cmp dword [running], 0
    je .out
    mov eax, [flameWait]
    test eax, eax
    jz .adv
    dec eax
    mov [flameWait], eax
    jnz .out
.adv:
    mov eax, [flameIdx]
    inc eax
    and eax, 7
    mov [flameIdx], eax
    lea rdx, [flameDelay]
    movzx ecx, byte [rdx+rax]
    mov [flameWait], ecx
    call flame_show
.out:
    leave
    ret

ram_update:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    lea rcx, [memst]
    mov dword [rcx+MEMSTAT.len], MEMSTAT_size
    call GlobalMemoryStatusEx
    test eax, eax
    jz .out
    mov rax, [memst+MEMSTAT.avail]
    shr rax, 20
    mov [availMB], eax
    lea rcx, [tmpBuf]
    lea rdx, [szRamFmt]
    mov r8d, eax
    call wsprintfW
    mov rcx, [hLblRamV]
    test rcx, rcx
    jz .out
    lea rdx, [tmpBuf]
    call SetWindowTextW
.out:
    leave
    ret

paths_init:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    xor ecx, ecx
    lea rdx, [exeDir]
    mov r8d, 260
    call GetModuleFileNameW
    lea rcx, [exeDir]
    call wlen
    lea rdx, [exeDir]
    lea rcx, [rdx+rax*2-2]
.trim:
    cmp rcx, rdx
    jb .done
    cmp word [rcx], '\'
    je .cut
    sub rcx, 2
    jmp .trim
.cut:
    add rcx, 2
    mov word [rcx], 0
.done:
    lea rcx, [sysi]
    call GetSystemInfo
    mov eax, [sysi+SYSINFO.nproc]
    mov [nproc], eax
    call lp_count_pcores
    leave
    ret

wlen:
    push rcx
    xor eax, eax
.l:
    cmp word [rcx], 0
    je .d
    inc eax
    add rcx, 2
    jmp .l
.d:
    pop rcx
    ret

memzero:
    push rdi
    mov rdi, rcx
    mov ecx, edx
    xor eax, eax
    rep stosb
    pop rdi
    ret

wtoi:
    xor eax, eax
    xor edx, edx
.l:
    movzx edx, word [rcx]
    test edx, edx
    jz .d
    cmp edx, '0'
    jb .d
    cmp edx, '9'
    ja .d
    imul eax, 10
    sub edx, '0'
    add eax, edx
    add rcx, 2
    jmp .l
.d:
    ret

isqrt:
    push rbx
    mov r8, rcx
    cmp r8, 2
    jb .small
    mov rax, r8
.newton:
    mov rbx, rax
    mov rax, r8
    xor rdx, rdx
    div rbx
    add rax, rbx
    shr rax, 1
    cmp rax, rbx
    jb .newton
    mov rax, rbx
    pop rbx
    ret
.small:
    mov rax, r8
    pop rbx
    ret

stress_mb:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 38h
    mov rcx, [hCmbStress]
    mov edx, CB_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov ebx, eax
    call ram_update
    mov ecx, [availMB]
    cmp ebx, 0
    je .std
    cmp ebx, -1
    je .std
    cmp ebx, 1
    je .high
    cmp ebx, 2
    je .vhigh
    cmp ebx, 3
    je .max
    mov rcx, [hEdtMB]
    lea rdx, [tmpBuf]
    mov r8d, 32
    call GetWindowTextW
    lea rcx, [tmpBuf]
    call wtoi
    jmp .bound
.std:
    mov eax, 1024
    jmp .bound
.high:
    mov eax, 2048
    jmp .bound
.vhigh:
    mov eax, 4096
    jmp .bound
.max:
    mov eax, ecx
    cmp eax, 256
    jbe .maxs
    sub eax, 256
    jmp .bound
.maxs:
    shr eax, 1
.bound:
    cmp eax, 8
    jae .c2
    mov eax, 8
.c2:
    mov edx, [availMB]
    test edx, edx
    jz .ok
    cmp eax, edx
    jbe .ok
    mov eax, edx
.ok:
    mov [stressMB], eax
    add rsp, 38h
    pop rbx
    pop rbp
    ret

thread_count:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    mov rcx, [hCmbThr]
    mov edx, CB_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    cmp eax, 0
    je .all
    cmp eax, -1
    je .all
    cmp eax, 1
    je .auto
    mov rcx, [hCmbThr]
    mov edx, CB_GETLBTEXT
    mov r8d, eax
    lea r9, [tmpBuf]
    call SendMessageW
    lea rcx, [tmpBuf]
    call wtoi
    jmp .clamp
.all:
    mov eax, [nproc]
    jmp .clamp
.auto:
    mov eax, [nproc]
.clamp:
    cmp eax, 1
    jae .c2
    mov eax, 1
.c2:
    mov edx, [nproc]
    cmp edx, 1
    jae .c3
    mov eax, 1
    jmp .ok
.c3:
    cmp eax, edx
    jbe .ok
    mov eax, edx
.ok:
    leave
    ret

title_set:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    lea rcx, [titleBuf]
    lea rdx, [szTitleFmt]
    mov r8, rcx
    mov r8, rdx
    lea rcx, [titleBuf]
    lea rdx, [szTitleFmt]
    call wsprintfW
    mov rcx, [hMain]
    lea rdx, [titleBuf]
    call SetWindowTextW
    leave
    ret

title_status:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov r8, rcx
    lea rcx, [titleBuf]
    lea rdx, [szTitleFmt]
    call wsprintfW
    mov rcx, [hMain]
    test rcx, rcx
    jz .o
    lea rdx, [titleBuf]
    call SetWindowTextW
.o:
    leave
    ret

title_progress:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov r8d, ecx
    mov r9d, [targetRuns]
    lea rcx, [titleBuf]
    lea rdx, [szTitleProg]
    call wsprintfW
    mov rcx, [hMain]
    test rcx, rcx
    jz .o
    lea rdx, [titleBuf]
    call SetWindowTextW
.o:
    leave
    ret

show_about:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov rcx, [hMain]
    lea rdx, [szAboutTxt]
    lea r8, [szAboutCap]
    mov r9d, MB_OK|MB_ICONINFORMATION
    call dark_message_box
    leave
    ret

open_coffee:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    mov rcx, [hMain]
    lea rdx, [szOpenVerb]
    lea r8, [szCoffeeUrl]
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], SW_SHOWNORMAL
    call ShellExecuteW
    leave
    ret

ui_running:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 20h
    mov ebx, ecx
    xor r12d, r12d
    test ebx, ebx
    setz r12b
    call GetCurrentThread
    mov rcx, rax
    mov edx, 1
    test ebx, ebx
    jnz .pri
    xor edx, edx
.pri:
    call SetThreadPriority
    mov rcx, [hBtnStart]
    mov edx, r12d
    call EnableWindow
    mov rcx, [hBtnStop]
    mov edx, ebx
    call EnableWindow
    mov rcx, [hCmbStress]
    mov edx, r12d
    call EnableWindow
    mov rcx, [hCmbThr]
    mov edx, r12d
    call EnableWindow
    mov rcx, [hEdtTimes]
    mov edx, r12d
    call EnableWindow
    xor eax, eax
    mov [flameIdx], eax
    mov [flameWait], eax
    mov rcx, [hFlameWnd]
    mov edx, SW_SHOW
    call ShowWindow
    xor eax, eax
    call flame_show
.fldone:
    add rsp, 20h
    pop r12
    pop rbx
    pop rbp
    ret

start_burn:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 48h
    cmp dword [running], 0
    jne .out
    mov rcx, [hEdtTimes]
    lea rdx, [tmpBuf]
    mov r8d, 16
    call GetWindowTextW
    lea rcx, [tmpBuf]
    call wtoi
    cmp eax, 1
    jae .t2
    mov eax, 1
.t2:
    mov [targetRuns], eax
    call stress_mb
    mov eax, [stressMB]
    mov edx, [availMB]
    cmp eax, edx
    jbe .memok
    mov rcx, [hMain]
    lea rdx, [szMemErr]
    lea r8, [szMemCap]
    mov r9d, MB_OK|MB_ICONERROR
    call dark_message_box
    jmp .out
.memok:
    call GetCurrentThread
    mov rcx, rax
    mov edx, 1
    call SetThreadPriority
    call lists_clear
    mov dword [haveResid], 0
    mov dword [failed], 0
    mov dword [passCount], 0
    mov dword [lpAllocFail], 0
    mov byte [firstResid], 0
    call log_open
    xor ecx, ecx
    xor edx, edx
    lea r8, [burn_thread]
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    call CreateThread
    test rax, rax
    jnz .throk
    call GetCurrentThread
    mov rcx, rax
    xor edx, edx
    call SetThreadPriority
    jmp .out
.throk:
    mov [hProc], rax
    mov dword [running], 1
    call GetTickCount64
    mov [tick0], rax
    lea rcx, [szPreparing]
    call title_status
    mov ecx, 1
    call ui_running
.out:
    add rsp, 48h
    pop rbx
    pop rbp
    ret

stop_burn:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    cmp dword [running], 0
    je .out
    mov dword [lpStop], 1
    mov rcx, [hProc]
    test rcx, rcx
    jz .cl
    mov edx, 30000
    call WaitForSingleObject
.cl:
    call cleanup_proc
    mov dword [running], 0
    lea rcx, [szStoppedA]
    call log_result
    call log_close
    lea rcx, [szStopped]
    call title_status
    xor ecx, ecx
    call ui_running
.out:
    leave
    ret

lists_clear:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov rcx, [hLstTime]
    mov edx, LB_RESETCONTENT
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov rcx, [hLstSpeed]
    mov edx, LB_RESETCONTENT
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov rcx, [hLstRes]
    mov edx, LB_RESETCONTENT
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    leave
    ret

post_line:
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, 30h
    mov rcx, [hMain]
    mov edx, WM_LINE
    xor r8d, r8d
    lea r9, [lineAcc]
    call SendMessageW
    leave
    ret

on_line:
    jmp parse_result

parse_result:
    push rbp
    mov rbp, rsp
    push rsi
    push rdi
    push r12
    push r13
    push r14
    sub rsp, 40h
    mov r14, rcx
    lea r13, [rcx+32]
    lea r12, [rcx+64]
    mov rcx, r14
    lea rdx, [tmpBuf]
    call ansi_to_wide
    mov rcx, [hLstTime]
    mov edx, LB_ADDSTRING
    xor r8d, r8d
    lea r9, [tmpBuf]
    call SendMessageW
    mov rcx, r13
    lea rdx, [tmpBuf]
    call ansi_to_wide
    mov rcx, [hLstSpeed]
    mov edx, LB_ADDSTRING
    xor r8d, r8d
    lea r9, [tmpBuf]
    call SendMessageW
    mov rcx, r12
    lea rdx, [tmpBuf]
    call ansi_to_wide
    mov rcx, [hLstRes]
    mov edx, LB_ADDSTRING
    xor r8d, r8d
    lea r9, [tmpBuf]
    call SendMessageW
    call lists_scroll
    cmp dword [haveResid], 0
    jne .cmp
    lea rdi, [firstResid]
    mov rsi, r12
.cp:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    test al, al
    jnz .cp
    mov dword [haveResid], 1
    jmp .incp
.cmp:
    lea rsi, [firstResid]
    mov rdi, r12
.cl:
    mov al, [rsi]
    mov dl, [rdi]
    cmp al, dl
    jne .bad
    test al, al
    jz .incp
    inc rsi
    inc rdi
    jmp .cl
.bad:
    mov dword [failed], 1
    call fail_now
    mov eax, 1
    jmp .leave
.incp:
    inc dword [passCount]
    lea rcx, [titleBuf]
    lea rdx, [szTitleProg]
    mov r8d, [passCount]
    mov r9d, [targetRuns]
    call wsprintfW
    mov rcx, [hMain]
    lea rdx, [titleBuf]
    call SetWindowTextW
    mov rcx, r14
    lea rdx, [tmpBuf]
    call ansi_to_wide
    mov rcx, r13
    lea rdx, [tmpBuf+160]
    call ansi_to_wide
    mov rcx, r12
    lea rdx, [tmpBuf+320]
    call ansi_to_wide
    call log_line_3
    mov eax, 1
    jmp .leave
.no:
    xor eax, eax
.leave:
    add rsp, 40h
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbp
    ret

ansi_to_wide:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov r8, rcx
    mov r9, rdx
    xor ecx, ecx
    xor edx, edx
    mov qword [rsp+20h], r9
    mov qword [rsp+28h], 80
    mov r9d, -1
    call MultiByteToWideChar
    leave
    ret

lists_scroll:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 28h
    mov rcx, [hLstTime]
    mov edx, LB_GETCOUNT
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    test eax, eax
    jz .out
    dec eax
    mov ebx, eax
    mov rcx, [hLstTime]
    mov edx, LB_SETTOPINDEX
    mov r8d, ebx
    xor r9d, r9d
    call SendMessageW
    mov rcx, [hLstSpeed]
    mov edx, LB_SETTOPINDEX
    mov r8d, ebx
    xor r9d, r9d
    call SendMessageW
    mov rcx, [hLstRes]
    mov edx, LB_SETTOPINDEX
    mov r8d, ebx
    xor r9d, r9d
    call SendMessageW
.out:
    add rsp, 28h
    pop rbx
    pop rbp
    ret

fail_now:
    mov dword [lpStop], 1
    mov dword [failed], 1
    ret

on_done:
    push rbp
    mov rbp, rsp
    sub rsp, 50h
    cmp dword [running], 0
    je .out
    call cleanup_proc
    mov dword [running], 0
    xor ecx, ecx
    call ui_running
    call GetTickCount64
    sub rax, [tick0]
    xor rdx, rdx
    mov rcx, 1000
    div rcx
    mov ebx, eax
    cmp dword [lpAllocFail], 0
    jne .alloc
    cmp dword [failed], 0
    jne .fail
    cmp dword [passCount], 0
    je .die
    lea rcx, [szFinished]
    call title_status
    lea rcx, [szSuccessA]
    call log_result
    call log_close
    lea rcx, [tmpBuf]
    lea rdx, [szOkTxt]
    mov r8d, ebx
    call wsprintfW
    mov rcx, [hMain]
    lea rdx, [tmpBuf]
    lea r8, [szOkCap]
    mov r9d, MB_OK|MB_ICONINFORMATION
    call dark_message_box
    jmp .out
.alloc:
    lea rcx, [szFailure]
    call title_status
    lea rcx, [szFailA]
    call log_result
    call log_close
    mov rcx, [hMain]
    lea rdx, [szAllocErr]
    lea r8, [szMemCap]
    mov r9d, MB_OK|MB_ICONERROR
    call dark_message_box
    jmp .out
.fail:
    lea rcx, [szFailure]
    call title_status
    lea rcx, [szFailA]
    call log_result
    call log_close
    lea rcx, [tmpBuf]
    lea rdx, [szFailTxt]
    mov r8d, ebx
    call wsprintfW
    mov rcx, [hMain]
    lea rdx, [tmpBuf]
    lea r8, [szCritCap]
    mov r9d, MB_OK|MB_ICONERROR
    call dark_message_box
    jmp .out
.die:
    lea rcx, [szFailure]
    call title_status
    lea rcx, [szFailA]
    call log_result
    call log_close
    lea rcx, [tmpBuf]
    lea rdx, [szDieTxt]
    mov r8d, ebx
    call wsprintfW
    mov rcx, [hMain]
    lea rdx, [tmpBuf]
    lea r8, [szCritCap]
    mov r9d, MB_OK|MB_ICONWARNING
    call dark_message_box
.out:
    leave
    ret

cleanup_proc:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov rcx, [hProc]
    test rcx, rcx
    jz .o
    call CloseHandle
    mov qword [hProc], 0
.o:
    leave
    ret

log_open:
    push rbp
    mov rbp, rsp
    sub rsp, 50h
    mov rcx, [hChkLog]
    mov edx, BM_GETCHECK
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    cmp eax, BST_CHECKED
    je .go
    cmp dword [debugOn], 0
    je .out
.go:
    lea rcx, [logPath]
    lea rdx, [exeDir]
    call lstrcpyW
    lea rcx, [logPath]
    lea rdx, [szLogName]
    call lstrcatW
    lea rcx, [logPath]
    mov edx, GENERIC_WRITE
    mov r8d, FILE_SHARE_READ
    xor r9d, r9d
    mov qword [rsp+20h], CREATE_ALWAYS
    mov qword [rsp+28h], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp+30h], 0
    call CreateFileW
    cmp rax, -1
    je .out
    mov [hLog], rax
    lea rcx, [tmpBuf]
    lea rdx, [szLogHdr]
    mov r8, [hCmbStress]
    mov rcx, [hCmbStress]
    mov edx, CB_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov rcx, [hCmbStress]
    mov edx, CB_GETLBTEXT
    mov r8d, eax
    lea r9, [cpuWide]
    call SendMessageW
    lea rcx, [tmpBuf]
    lea rdx, [szLogHdr]
    lea r8, [cpuWide]
    mov r9d, [stressMB]
    call wsprintfW
    lea rcx, [tmpBuf]
    call log_wstr
.out:
    leave
    ret

log_wstr:
    push rbp
    mov rbp, rsp
    sub rsp, 50h
    mov rdx, rcx
    mov rcx, [hLog]
    test rcx, rcx
    jz .out
    mov r8, rdx
    xor ecx, ecx
    xor edx, edx
    mov r9d, -1
    lea rax, [tmpAnsi]
    mov [rsp+20h], rax
    mov qword [rsp+28h], 500
    mov qword [rsp+30h], 0
    mov qword [rsp+38h], 0
    call WideCharToMultiByte
    mov r8d, eax
    dec r8d
    jle .out
    mov rcx, [hLog]
    lea rdx, [tmpAnsi]
    lea r9, [tmpBuf]
    mov qword [rsp+20h], 0
    call WriteFile
.out:
    leave
    ret

log_line_3:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    mov rcx, [hLog]
    test rcx, rcx
    jz .out
    lea rcx, [titleBuf]
    lea rdx, [szLogLine]
    lea r8, [tmpBuf]
    lea r9, [tmpBuf+160]
    sub rsp, 20h
    lea rax, [tmpBuf+320]
    mov [rsp+20h], rax
    call wsprintfW
    add rsp, 20h
    lea rcx, [titleBuf]
    call log_wstr
.out:
    leave
    ret

log_result:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    mov r8, rcx
    mov rcx, [hLog]
    test rcx, rcx
    jz .out
    lea rcx, [tmpBuf]
    lea rdx, [szLogEnd]
    lea r9, [cpuWide]
    call wsprintfW
    lea rcx, [tmpBuf]
    call log_wstr
.out:
    leave
    ret

log_close:
    push rbp
    mov rbp, rsp
    sub rsp, 30h
    mov rcx, [hLog]
    test rcx, rcx
    jz .out
    call CloseHandle
    mov qword [hLog], 0
.out:
    leave
    ret

subclass_proc:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 48h
    mov [rbp+16], rcx
    mov [rbp+24], rdx
    mov [rbp+32], r8
    mov [rbp+40], r9
    cmp edx, WM_CONTEXTMENU
    je .ctx
    cmp edx, WM_NCDESTROY
    je .ncd
.def:
    mov rcx, [rbp+16]
    mov edx, [rbp+24]
    mov r8, [rbp+32]
    mov r9, [rbp+40]
    mov rax, [rbp+48]
    mov [rsp+20h], rax
    mov rax, [rbp+56]
    mov [rsp+28h], rax
    call DefSubclassProc
    jmp .out
.ctx:
    mov rcx, [hMain]
    mov edx, WM_CTX
    mov r8, [rbp+48]
    mov r9, [rbp+40]
    call PostMessageW
    xor eax, eax
    jmp .out
.ncd:
    mov rcx, [rbp+16]
    lea rdx, [subclass_proc]
    mov r8, [rbp+48]
    call RemoveWindowSubclass
    jmp .def
.out:
    add rsp, 48h
    pop rbx
    pop rbp
    ret

menu_context:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push r12
    sub rsp, 48h
    mov ebx, ecx
    mov rsi, rdx
    call CreatePopupMenu
    mov r12, rax
    cmp ebx, IDC_START
    je .xt
    cmp ebx, IDC_LOG
    je .dbg
    jmp .copy
.xt:
    mov r9d, MF_STRING
    cmp dword [xtreme], 0
    je .xt2
    mov r9d, MF_STRING|MF_CHECKED
.xt2:
    mov rcx, r12
    mov edx, r9d
    mov r8d, IDM_XTREME
    lea r9, [szXtreme]
    call AppendMenuW
    jmp .track
.dbg:
    mov r9d, MF_STRING
    cmp dword [debugOn], 0
    je .db2
    mov r9d, MF_STRING|MF_CHECKED
.db2:
    mov rcx, r12
    mov edx, r9d
    mov r8d, IDM_DEBUG
    lea r9, [szDbg]
    call AppendMenuW
    jmp .track
.copy:
    mov rcx, r12
    mov edx, MF_STRING
    mov r8d, IDM_COPY
    lea r9, [szCopy]
    call AppendMenuW
    mov rcx, r12
    mov edx, MF_STRING
    mov r8d, IDM_COPYALL
    lea r9, [szCopyAll]
    call AppendMenuW
.track:
    cmp esi, -1
    je .cur
    movsx r8d, si
    sar esi, 16
    movsx r9d, si
    jmp .trk
.cur:
    lea rcx, [ptxy]
    call GetCursorPos
    mov r8d, [ptxy]
    mov r9d, [ptxy+4]
.trk:
    mov rcx, r12
    mov edx, TPM_LEFTALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON
    mov qword [rsp+20h], 0
    mov rax, [hMain]
    mov [rsp+28h], rax
    mov qword [rsp+30h], 0
    call TrackPopupMenu
    mov ebx, eax
    mov rcx, r12
    call DestroyMenu
    cmp ebx, IDM_XTREME
    je .dox
    cmp ebx, IDM_DEBUG
    je .dod
    cmp ebx, IDM_COPY
    je .doc
    cmp ebx, IDM_COPYALL
    je .doa
    jmp .out
.dox:
    cmp dword [xtreme], 0
    jne .offx
    mov rcx, [hMain]
    lea rdx, [szXtremeAsk]
    lea r8, [szXtreme]
    mov r9d, MB_OKCANCEL|MB_ICONWARNING
    call dark_message_box
    cmp eax, IDOK
    jne .out
    mov dword [xtreme], 1
    jmp .out
.offx:
    mov dword [xtreme], 0
    jmp .out
.dod:
    xor dword [debugOn], 1
    jmp .out
.doc:
    call clip_copy
    jmp .out
.doa:
    call clip_copyall
.out:
    add rsp, 48h
    pop r12
    pop rsi
    pop rbx
    pop rbp
    ret

clip_copy:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 38h
    xor ebx, ebx
.try:
    lea rax, [hLstTime]
    mov rcx, [rax+rbx*8]
    mov edx, LB_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    cmp eax, -1
    jne .have
    inc ebx
    cmp ebx, 3
    jb .try
    jmp .out
.have:
    lea rcx, [tmpBuf]
    mov word [rcx], 0
    mov edx, eax
    xor r8d, r8d
    call row_append
    lea rcx, [tmpBuf]
    call clip_set
.out:
    add rsp, 38h
    pop rbx
    pop rbp
    ret

clip_copyall:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    sub rsp, 38h
    mov rcx, [hLstTime]
    mov edx, LB_GETCOUNT
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov esi, eax
    test esi, esi
    jz .out
    call GetProcessHeap
    mov rcx, rax
    mov edx, HEAP_ZERO_MEMORY
    mov r8d, 32768
    call HeapAlloc
    test rax, rax
    jz .out
    mov rbx, rax
    mov word [rbx], 0
    xor edi, edi
.lp:
    cmp edi, esi
    jae .put
    mov rcx, rbx
    mov edx, edi
    mov r8d, 1
    call row_append
    inc edi
    jmp .lp
.put:
    mov rcx, rbx
    call clip_set
    call GetProcessHeap
    mov rcx, rax
    xor edx, edx
    mov r8, rbx
    call HeapFree
.out:
    add rsp, 38h
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

row_append:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push r12
    push r13
    sub rsp, 20h
    mov rbx, rcx
    mov r12d, edx
    mov r13d, r8d
    xor esi, esi
.column:
    lea rax, [hLstTime]
    mov rcx, [rax+rsi*8]
    mov edx, LB_GETTEXT
    mov r8d, r12d
    lea r9, [titleBuf]
    call SendMessageW
    mov rcx, rbx
    lea rdx, [titleBuf]
    call lstrcatW
    inc esi
    cmp esi, 3
    jae .last
    mov rcx, rbx
    lea rdx, [szTab]
    call lstrcatW
    jmp .column
.last:
    test r13d, r13d
    jz .out
    mov rcx, rbx
    lea rdx, [szCRLF]
    call lstrcatW
.out:
    add rsp, 20h
    pop r13
    pop r12
    pop rsi
    pop rbx
    pop rbp
    ret

clip_set:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push r12
    sub rsp, 38h
    mov rsi, rcx
    call lstrlenW
    inc eax
    shl eax, 1
    mov ebx, eax
    mov ecx, GMEM_MOVEABLE
    mov edx, ebx
    call GlobalAlloc
    test rax, rax
    jz .out
    mov r12, rax
    mov rcx, rax
    call GlobalLock
    mov rcx, rax
    mov rdx, rsi
    call lstrcpyW
    mov rcx, r12
    call GlobalUnlock
    xor ecx, ecx
    call OpenClipboard
    test eax, eax
    jz .out
    call EmptyClipboard
    mov ecx, CF_UNICODETEXT
    mov rdx, r12
    call SetClipboardData
    call CloseClipboard
.out:
    add rsp, 38h
    pop r12
    pop rsi
    pop rbx
    pop rbp
    ret

cpu_brand_read:
    push rbx
    push rdi
    sub rsp, 28h
    lea rdi, [cpuAscii]
    mov eax, 80000000h
    cpuid
    cmp eax, 80000004h
    jb .no
    mov eax, 80000002h
    cpuid
    mov [rdi], eax
    mov [rdi+4], ebx
    mov [rdi+8], ecx
    mov [rdi+12], edx
    mov eax, 80000003h
    cpuid
    mov [rdi+16], eax
    mov [rdi+20], ebx
    mov [rdi+24], ecx
    mov [rdi+28], edx
    mov eax, 80000004h
    cpuid
    mov [rdi+32], eax
    mov [rdi+36], ebx
    mov [rdi+40], ecx
    mov [rdi+44], edx
    mov byte [rdi+48], 0
    jmp .wide
.no:
    mov dword [rdi], 'CPU'
    mov byte [rdi+3], 0
.wide:
    lea rcx, [cpuAscii]
    lea rdx, [cpuWide]
    call ansi_to_wide
    add rsp, 28h
    pop rdi
    pop rbx
    ret
