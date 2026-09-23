PROC_FRAME theme_ux_init, 28h, rbx
    lea rcx, [szUxTheme]
    call GetModuleHandleW
    test rax, rax
    jnz .gp
    lea rcx, [szUxTheme]
    call LoadLibraryW
    test rax, rax
    jz .o
.gp:
    mov rbx, rax
    mov rcx, rbx
    mov edx, 133
    call GetProcAddress
    mov [fnAllowDark], rax
    mov rcx, rbx
    mov edx, 135
    call GetProcAddress
    mov [fnSetAppMode], rax
    mov rcx, rbx
    mov edx, 136
    call GetProcAddress
    mov [fnFlushMenu], rax
.o:
    ENDPROC_SAVED 28h, rbx

PROC_FRAME theme_appmode, 30h
    mov rax, [fnSetAppMode]
    test rax, rax
    jz .fl
    mov ecx, 2
    call rax
.fl:
    mov rax, [fnFlushMenu]
    test rax, rax
    jz .o
    call rax
.o:
    ENDPROC

PROC_FRAME theme_brushes, 30h
    mov rcx, [hBrBg]
    test rcx, rcx
    jz .e1
    call DeleteObject
.e1:
    mov rcx, [hBrPanel]
    test rcx, rcx
    jz .e2
    call DeleteObject
.e2:
    mov rcx, [hBrEdit]
    test rcx, rcx
    jz .mk
    call DeleteObject
.mk:
    mov ecx, COL_BG
    call CreateSolidBrush
    mov [hBrBg], rax
    mov ecx, COL_PANEL
    call CreateSolidBrush
    mov [hBrPanel], rax
    mov ecx, COL_EDIT
    call CreateSolidBrush
    mov [hBrEdit], rax
    ENDPROC

PROC_FRAME theme_window, 38h, rbx
    test rcx, rcx
    jz .out
    mov rbx, rcx
    mov rax, [fnAllowDark]
    test rax, rax
    jz .dwm
    mov rcx, rbx
    mov edx, 1
    call rax
.dwm:
    mov dword [rsp+24h], 1
    mov rcx, rbx
    mov edx, DWMWA_DARK
    lea r8, [rsp+24h]
    mov r9d, 4
    call DwmSetWindowAttribute
    test eax, eax
    jz .out
    mov rcx, rbx
    mov edx, DWMWA_DARK_OLD
    lea r8, [rsp+24h]
    mov r9d, 4
    call DwmSetWindowAttribute
.out:
    ENDPROC_SAVED 38h, rbx

PROC_FRAME theme_title, 40h
    mov dword [rsp+24h], 1
    mov rcx, [hMain]
    test rcx, rcx
    jz .o
    mov rax, [fnAllowDark]
    test rax, rax
    jz .dw
    mov edx, 1
    call rax
    mov rcx, [hMain]
.dw:
    mov edx, DWMWA_DARK
    lea r8, [rsp+24h]
    mov r9d, 4
    call DwmSetWindowAttribute
    test eax, eax
    jz .o
    mov rcx, [hMain]
    mov edx, DWMWA_DARK_OLD
    lea r8, [rsp+24h]
    mov r9d, 4
    call DwmSetWindowAttribute
.o:
    ENDPROC

PROC_FRAME theme_onchild, 0A8h, rbx
    mov rbx, rcx
    lea rdx, [rsp+60h]
    mov r8d, 32
    call GetClassNameW
    test eax, eax
    jz .ok
    mov rax, [fnAllowDark]
    test rax, rax
    jz .th
    mov rcx, rbx
    mov edx, 1
    call rax
.th:
    lea rcx, [rsp+60h]
    lea rdx, [szStatic]
    call lstrcmpiW
    test eax, eax
    jz .ok
    lea rcx, [rsp+60h]
    lea rdx, [szEdit]
    call lstrcmpiW
    test eax, eax
    jz .edit
    lea rcx, [rsp+60h]
    lea rdx, [szCombo]
    call lstrcmpiW
    test eax, eax
    jz .cfd
    lea rdx, [szDarkMode]
    jmp .set
.edit:
    lea rdx, [szEmpty]
    lea r8, [szEmpty]
    jmp .set2
.cfd:
    lea rdx, [szDarkCfd]
.set:
    xor r8d, r8d
.set2:
    mov rcx, rbx
    call SetWindowTheme
.ok:
    mov eax, 1
    ENDPROC_SAVED 0A8h, rbx

PROC_FRAME theme_btn_style, 40h, rbx, rsi
    test rcx, rcx
    jz .o
    mov rbx, rcx
    mov edx, GWL_STYLE
    call GetWindowLongPtrW
    and eax, 0FFFFFFF0h
    or eax, BS_OWNERDRAW
    mov rcx, rbx
    mov edx, GWL_STYLE
    mov r8, rax
    call SetWindowLongPtrW
    mov rcx, rbx
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    mov qword [rsp+30h], SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED
    call SetWindowPos
    mov rcx, rbx
    xor edx, edx
    mov r8d, 1
    call round_button
.o:
    ENDPROC_SAVED 40h, rbx, rsi

PROC_FRAME theme_btns, 30h
    mov rcx, [hBtnStart]
    xor edx, edx
    call theme_btn_style
    mov rcx, [hBtnStop]
    xor edx, edx
    call theme_btn_style
    mov rcx, [hBtnAbout]
    mov edx, 1
    call theme_btn_style
    mov rcx, [hCoffeeWnd]
    xor edx, edx
    call coffee_theme_keep
    ENDPROC

PROC_FRAME theme_free, 30h
    mov rcx, [hBrBg]
    test rcx, rcx
    jz .e1
    call DeleteObject
.e1:
    mov rcx, [hBrPanel]
    test rcx, rcx
    jz .e2
    call DeleteObject
.e2:
    mov rcx, [hBrEdit]
    test rcx, rcx
    jz .done
    call DeleteObject
.done:
    mov qword [hBrBg], 0
    mov qword [hBrPanel], 0
    mov qword [hBrEdit], 0
    ENDPROC

PROC_FRAME theme_apply, 30h
    call theme_brushes
    call theme_appmode
    call theme_title
    mov rcx, [hMain]
    test rcx, rcx
    jz .o
    mov edx, GCLP_HBRBACKGROUND
    mov r8, [hBrBg]
    call SetClassLongPtrW
    call theme_btns
    UI_CHILD_CALL hCmbStress, theme_cmb_style
    UI_CHILD_CALL hCmbThr, theme_cmb_style
    UI_CHILD_CALL hEdtTimes, theme_edge
    UI_CHILD_CALL hEdtMB, theme_edge
    mov rcx, [hMain]
    lea rdx, [theme_onchild]
    xor r8d, r8d
    call EnumChildWindows
    xor eax, eax
    call flame_show
    UI_INVALIDATE hEdtTimes
    UI_INVALIDATE hEdtMB
    UI_INVALIDATE hCmbStress
    UI_INVALIDATE hCmbThr
    mov rcx, [hMain]
    xor edx, edx
    mov r8d, RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW
    xor r9d, r9d
    call RedrawWindow
.o:
    ENDPROC

PROC_FRAME theme_cmb_style, 38h, rbx
    test rcx, rcx
    jz .o
    mov rbx, rcx
    mov edx, GWL_STYLE
    call GetWindowLongPtrW
    and eax, 0FFFFFFEFh
    or eax, CBS_OWNERDRAWFIXED
    mov rcx, rbx
    mov edx, GWL_STYLE
    mov r8, rax
    call SetWindowLongPtrW
    mov rcx, rbx
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    mov qword [rsp+30h], SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED
    call SetWindowPos
.o:
    ENDPROC_SAVED 38h, rbx

PROC_FRAME theme_droplist, 20h
    UI_CHILD_CALL hCmbStress, theme_combo_list
    UI_CHILD_CALL hCmbThr, theme_combo_list
    ENDPROC

PROC_FRAME theme_combo_list, 68h, rbx
    test rcx, rcx
    jz .o
    mov rbx, rcx
    mov dword [rsp+20h], 64
    mov rcx, rbx
    lea rdx, [rsp+20h]
    call GetComboBoxInfo
    test eax, eax
    jz .o
    mov rcx, [rsp+50h]
    test rcx, rcx
    jz .lst
    lea rdx, [szEmpty]
    lea r8, [szEmpty]
    call SetWindowTheme
.lst:
    mov rcx, [rsp+58h]
    test rcx, rcx
    jz .o
    mov rbx, rcx
    mov rax, [fnAllowDark]
    test rax, rax
    jz .th
    mov edx, 1
    call rax
.th:
    mov rcx, rbx
    lea rdx, [szEmpty]
    lea r8, [szEmpty]
    call theme_list_rounded
.o:
    ENDPROC_SAVED 68h, rbx

PROC_FRAME theme_hook, 30h
    UI_SUBCLASS hEdtTimes, 2
    UI_SUBCLASS hEdtMB, 2
    UI_SUBCLASS hCmbStress, 3
    UI_SUBCLASS hCmbThr, 3
    ENDPROC

PROC_FRAME theme_edge, 38h, rbx
    test rcx, rcx
    jz .o
    mov rbx, rcx
    mov edx, GWL_EXSTYLE
    call GetWindowLongPtrW
    and eax, 0FFFFFDFFh
    mov rcx, rbx
    mov edx, GWL_EXSTYLE
    mov r8, rax
    call SetWindowLongPtrW
    mov rcx, rbx
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    mov qword [rsp+30h], SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED
    call SetWindowPos
.o:
    ENDPROC_SAVED 38h, rbx

PROC_FRAME control_sub, 0E0h, rbx, r12
    mov [rbp+16], rcx
    mov [rbp+24], rdx
    mov [rbp+32], r8
    mov [rbp+40], r9
    cmp edx, WM_NCDESTROY
    je .ncd
    cmp edx, WM_PAINT
    je .paint
    cmp edx, WM_ERASEBKGND
    je .erase
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
.ncd:
    mov rcx, [rbp+16]
    lea rdx, [control_sub]
    mov r8, [rbp+48]
    call RemoveWindowSubclass
    jmp .def
.erase:
    mov rcx, [rbp+16]
    lea rdx, [rsp+40h]
    call GetClientRect
    mov rcx, [rbp+32]
    lea rdx, [rsp+40h]
    mov r8, [hBrEdit]
    call FillRect
    mov eax, 1
    jmp .out
.paint:
    mov rcx, [rbp+16]
    lea rdx, [rsp+50h]
    call BeginPaint
    mov rbx, rax
    mov rcx, [rbp+16]
    lea rdx, [rsp+40h]
    call GetClientRect
    mov rcx, rbx
    lea rdx, [rsp+40h]
    mov r8, [hBrEdit]
    call FillRect
    xor ecx, ecx
    mov edx, 1
    mov r8d, COL_BORDER
    call CreatePen
    mov r12, rax
    mov rcx, rbx
    mov rdx, r12
    call SelectObject
    mov [rsp+38h], rax
    mov ecx, NULL_BRUSH
    call GetStockObject
    mov rcx, rbx
    mov rdx, rax
    call SelectObject
    mov rcx, rbx
    mov edx, [rsp+40h]
    mov r8d, [rsp+44h]
    mov r9d, [rsp+48h]
    mov eax, [rsp+4Ch]
    mov [rsp+20h], rax
    call round_rect_control
    mov rcx, rbx
    mov rdx, [rsp+38h]
    call SelectObject
    mov rcx, r12
    call DeleteObject
    cmp dword [rbp+48], 3
    je .combo_text
    mov rcx, [rbp+16]
    lea rdx, [rsp+0A0h]
    mov r8d, 32
    call GetWindowTextW
    mov rcx, [rbp+16]
    mov edx, GWL_STYLE
    call GetWindowLongPtrW
    test eax, WS_DISABLED
    mov r12d, COL_FG
    jz .tc
    mov r12d, COL_MUTED
.tc:
    add dword [rsp+40h], 3
    jmp .text_ready
.combo_text:
    mov dword [rsp+0A0h], 0
    mov rcx, [rbp+16]
    mov edx, CB_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    cmp eax, 0FFFFFFFFh
    je .txt
    mov rcx, [rbp+16]
    mov edx, CB_GETLBTEXT
    mov r8, rax
    lea r9, [rsp+0A0h]
    call SendMessageW
.txt:
    mov r12d, COL_FG
    add dword [rsp+40h], 4
    sub dword [rsp+48h], 16
.text_ready:
    mov rcx, rbx
    mov edx, TRANSPARENT
    call SetBkMode
    mov rcx, rbx
    mov edx, r12d
    call SetTextColor
    mov rcx, rbx
    mov rdx, [hFont]
    call SelectObject
    mov r12, rax
    mov rcx, rbx
    lea rdx, [rsp+0A0h]
    mov r8d, -1
    lea r9, [rsp+40h]
    mov qword [rsp+20h], DT_LEFT|DT_VCENTER|DT_SINGLELINE
    call DrawTextW
    cmp dword [rbp+48], 3
    jne .restore_font
    mov eax, [rsp+48h]
    add eax, 2
    mov [rsp+40h], eax
    add dword [rsp+48h], 16
    mov rcx, rbx
    lea rdx, [szChev]
    mov r8d, -1
    lea r9, [rsp+40h]
    mov qword [rsp+20h], DT_CENTER|DT_VCENTER|DT_SINGLELINE
    call DrawTextW
.restore_font:
    mov rcx, rbx
    mov rdx, r12
    call SelectObject
    mov rcx, [rbp+16]
    lea rdx, [rsp+50h]
    call EndPaint
    xor eax, eax
.out:
    ENDPROC_SAVED 0E0h, rbx, r12
