BITMAP_HEADER_BYTES equ 14 + 40 + 16*4
FLAME_BMP_BYTES equ BITMAP_HEADER_BYTES + 212*50
COFFEE_BMP_BYTES equ BITMAP_HEADER_BYTES + 44*23

section .data
desktop_bitmaps:
    incbin "res/desktop-bitmaps.lz4"
desktop_bitmaps_end:

section .bss
alignb 4
flameBmp: resb FLAME_BMP_BYTES
coffeeBmp: resb COFFEE_BMP_BYTES
bitmaps_end:

section .text
%macro BITMAP_LENGTH 0
    cmp ecx, 15
    jne %%done
%%next:
    cmp rsi, rdx
    jae .fail
    movzx eax, byte [rsi]
    inc rsi
    add ecx, eax
    cmp al, 255
    je %%next
%%done:
%endmacro

desktop_bitmaps_decode:
    push rbx
    push rsi
    push rdi
    cld
    lea rsi, [desktop_bitmaps]
    lea rdx, [desktop_bitmaps_end]
    lea rdi, [flameBmp]
    lea r8, [bitmaps_end]
    mov r11, rdi
.sequence:
    cmp rsi, rdx
    jae .fail
    lodsb
    movzx ebx, al
    mov ecx, ebx
    shr ecx, 4
    BITMAP_LENGTH
    lea rax, [rsi+rcx]
    cmp rax, rdx
    ja .fail
    lea rax, [rdi+rcx]
    cmp rax, r8
    ja .fail
    rep movsb
    cmp rsi, rdx
    je .finish
    lea rax, [rsi+2]
    cmp rax, rdx
    ja .fail
    movzx r9d, word [rsi]
    mov rsi, rax
    test r9d, r9d
    jz .fail
    mov r10, rdi
    sub r10, r9
    cmp r10, r11
    jb .fail
    mov ecx, ebx
    and ecx, 15
    BITMAP_LENGTH
    add ecx, 4
    lea rax, [rdi+rcx]
    cmp rax, r8
    ja .fail
    xchg rsi, r10
    rep movsb
    mov rsi, r10
    jmp .sequence
.finish:
    xor eax, eax
    cmp rdi, r8
    sete al
    jmp .out
.fail:
    xor eax, eax
.out:
    pop rdi
    pop rsi
    pop rbx
    ret
%unmacro BITMAP_LENGTH 0
