; Resolve a decoded external target to an identity shared by both images.
;
; Input: RAX target address, EDX side (0 loaded image, 1 reference file),
;        RCX context. R8D contains decoder attributes.
; Output: RAX stable symbol ID and CF=0, or CF=1 if the target is unknown.
;
; Context qwords: loaded image base, reference buffer, reference RVA bias,
;                 loaded symbol entries, reference symbol entries, entry count.
; Entries are 32-bit RVAs in identical, explicitly named order. Exact matching
; is intentional: guessing a nearby symbol could accept an incorrect target.
kernel_resolve_symbol:
    test edx, edx
    jnz .reference
    sub rax, [rcx]
    jc .unknown
    mov r10, [rcx+24]
    jmp .find
.reference:
    sub rax, [rcx+8]
    jc .unknown
    add rax, [rcx+16]
    jc .unknown
    mov r10, [rcx+32]
.find:
    mov r11, rax
    shr r11, 32
    jnz .unknown
    mov r9, [rcx+40]
    xor edx, edx
.next:
    cmp rdx, r9
    jae .unknown
    cmp eax, [r10+rdx*4]
    je .found
    inc rdx
    jmp .next
.found:
    lea rax, [rdx+1]
    clc
    ret
.unknown:
    stc
    ret
