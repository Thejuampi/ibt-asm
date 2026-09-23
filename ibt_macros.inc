; Readability helpers for ordinary Win64 procedures.
;
; These macros are intentionally transparent: they expand to the exact instruction
; sequences previously written at every call site. List nonvolatile registers in
; push order; ENDPROC_SAVED restores them in reverse order automatically.
; Performance-critical numerical kernels remain explicit in bench_lib.inc and
; ibt_lpk.inc.

%macro PROC_FRAME 2-*
    %xdefine %%stack_size %2
    %assign %%saved_count %0 - 2
%1:
    push rbp
    mov rbp, rsp
    %rotate 2
    %rep %%saved_count
        push %1
        %rotate 1
    %endrep
    sub rsp, %%stack_size
%endmacro

; Entry points that need to realign an inherited stack before reserving locals.
%macro PROC_FRAME_ALIGNED 2
%1:
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, %2
%endmacro

%macro ENDPROC 0
    leave
    ret
%endmacro

%macro ENDPROC_SAVED 2-*
    %xdefine %%stack_size %1
    %assign %%saved_count %0 - 1
    add rsp, %%stack_size
    %rep %%saved_count
        %rotate -1
        pop %1
    %endrep
    pop rbp
    ret
%endmacro

; Structured bottom-tested loops. The condition remains an explicit x86 jump
; mnemonic, so flag usage stays visible while the bookkeeping label disappears.
%macro DO 0
    %push do_loop
%$top:
%endmacro

%macro WHILE 1
    %1 %$top
    %pop
%endmacro

; Fixed UI control call sites.  Like PROC_FRAME, these expand to the original
; instructions; the validated numerical region stays byte-for-byte unchanged.
%macro UI_CHILD_CALL 2
    mov rcx, [%1]
    call %2
%endmacro

%macro UI_INVALIDATE 1
    mov rcx, [%1]
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
%endmacro

%macro UI_SUBCLASS 2
    mov rcx, [%1]
    lea rdx, [control_sub]
    mov r8d, %2
    xor r9d, r9d
    call SetWindowSubclass
%endmacro
