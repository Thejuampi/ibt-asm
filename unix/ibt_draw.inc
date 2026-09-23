; Small, byte-for-byte transparent drawing call sites.  These keep the
; fixed 620x400 layout readable without adding code to the numerical core.

%macro DRAW_RECT 6
    mov edi, %1
    mov esi, %2
    mov edx, %3
    mov ecx, %4
    mov r8d, %5
    call %6
%endmacro

%macro DRAW_PANEL 4
    mov edi, %1
    mov esi, %2
    mov edx, %3
    mov ecx, %4
    call panel
%endmacro

%macro DRAW_INPUT 4
    mov edi, %1
    mov esi, %2
    mov edx, %3
    mov ecx, %4
    call input_box
%endmacro

%macro DRAW_ROUND 6
    mov edi, %1
    mov esi, %2
    mov edx, %3
    mov ecx, %4
    mov r8d, %5
    mov r9d, %6
    call round_fill
%endmacro

%macro DRAW_LABEL 5
    mov edi, %1
    mov esi, %2
    lea rdx, [%3]
    mov ecx, %4
    call %5
%endmacro

; rdx already names a value selected or formatted by the caller.
%macro DRAW_SELECTED 4
    mov edi, %1
    mov esi, %2
    mov ecx, %3
    call %4
%endmacro

%macro DRAW_CHEVRON 2
    mov edi, %1
    mov esi, %2
    call draw_chevron
%endmacro
