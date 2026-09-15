align 32
lp_pack_a:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    mov r12, [lpGA]
    test r12, r12
    jz .out
    test r8, r8
    jz .out
    mov r13, r8
    mov ebx, edx
    sub ebx, ecx
    cmp ebx, 8
    jb .out
    cmp dword [lpGK], 1
    jl .out
    movsxd rax, ecx
    lea r12, [r12+rax*8]
    mov r8, [lpGLDA]
    lea r8, [r8*8]
    xor r9d, r9d
.st12:
    lea eax, [r9+12]
    cmp eax, ebx
    ja .st8
    movsxd rax, r9d
    lea rsi, [r12+rax*8]
    mov eax, r9d
    imul eax, [lpGK]
    shl rax, 3
    lea rdi, [r13+rax]
    mov ecx, [lpGK]
    align 32
.pk12:
    vmovupd ymm0, [rsi]
    vmovupd ymm1, [rsi+32]
    vmovupd ymm2, [rsi+64]
    vmovapd [rdi], ymm0
    vmovapd [rdi+32], ymm1
    vmovapd [rdi+64], ymm2
    add rsi, r8
    add rdi, 96
    sub ecx, 1
    jnz .pk12
    add r9d, 12
    jmp .st12
.st8:
    lea eax, [r9+8]
    cmp eax, ebx
    ja .out
    movsxd rax, r9d
    lea rsi, [r12+rax*8]
    mov eax, r9d
    imul eax, [lpGK]
    shl rax, 3
    lea rdi, [r13+rax]
    mov ecx, [lpGK]
    align 32
.pk8:
    vmovupd ymm0, [rsi]
    vmovupd ymm1, [rsi+32]
    vmovapd [rdi], ymm0
    vmovapd [rdi+32], ymm1
    add rsi, r8
    add rdi, 64
    sub ecx, 1
    jnz .pk8
.out:
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_pack_b:
    push rsi
    push rdi
    mov rdi, rdx
    mov rsi, [lpGB]
    mov r8, [lpGLDA]
    lea r8, [r8*8]
    movsxd rax, ecx
    imul rax, r8
    add rsi, rax
    lea r9, [rsi+r8]
    lea r10, [r9+r8]
    lea r11, [r10+r8]
    mov ecx, [lpGK]
    test ecx, ecx
    jz .o
    mov eax, ecx
    and eax, ~3
    jz .tail
    align 32
.pk4:
    vmovupd ymm0, [rsi]
    vmovupd ymm1, [r9]
    vmovupd ymm2, [r10]
    vmovupd ymm3, [r11]
    vunpcklpd ymm4, ymm0, ymm1
    vunpckhpd ymm5, ymm0, ymm1
    vunpcklpd ymm6, ymm2, ymm3
    vunpckhpd ymm7, ymm2, ymm3
    vperm2f128 ymm0, ymm4, ymm6, 20h
    vperm2f128 ymm1, ymm5, ymm7, 20h
    vperm2f128 ymm2, ymm4, ymm6, 31h
    vperm2f128 ymm3, ymm5, ymm7, 31h
    vmovapd [rdi], ymm0
    vmovapd [rdi+32], ymm1
    vmovapd [rdi+64], ymm2
    vmovapd [rdi+96], ymm3
    add rsi, 32
    add r9, 32
    add r10, 32
    add r11, 32
    add rdi, 128
    sub eax, 4
    jnz .pk4
.tail:
    and ecx, 3
    jz .o
    align 32
.pk:
    vmovsd xmm0, [rsi]
    vmovsd xmm1, [r9]
    vmovsd xmm2, [r10]
    vmovsd xmm3, [r11]
    vmovsd [rdi], xmm0
    vmovsd [rdi+8], xmm1
    vmovsd [rdi+16], xmm2
    vmovsd [rdi+24], xmm3
    add rsi, 8
    add r9, 8
    add r10, 8
    add r11, 8
    add rdi, 32
    sub ecx, 1
    jnz .pk
.o:
    pop rdi
    pop rsi
    ret

align 32
lp_pack_b_all:
    push rbx
    push rdi
    mov rdi, [lpPackB]
    test rdi, rdi
    jz .o
    xor ebx, ebx
.j:
    mov eax, [lpGN]
    and eax, ~3
    cmp ebx, eax
    jae .o
    mov ecx, ebx
    mov eax, ebx
    shr eax, 2
    imul eax, BPG
    lea rdx, [rdi+rax]
    call lp_pack_b
    add ebx, 4
    jmp .j
.o:
    pop rdi
    pop rbx
    ret

align 32
lp_dgemm_ir:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    and rsp, -64
    sub rsp, 40h
    mov r12d, ecx
    mov r13d, edx
    cmp r12d, r13d
    jae .done
    cmp dword [lpIsaUse], 1
    jb .sse
    mov eax, [lpGN]
    and eax, ~3
    mov [rsp], eax
    mov esi, r12d
    cmp dword [lpIsaUse], 2
    jb .i8
.i12:
    lea eax, [esi+12]
    cmp eax, r13d
    ja .i8
    mov r15, [lpPackA]
    test r15, r15
    jz .done
    mov eax, esi
    imul eax, [lpGK]
    shl rax, 3
    add r15, rax
    xor edi, edi
    mov ebx, [rsp]
    align 32
.p12:
    cmp edi, ebx
    jae .p12d
    mov rcx, r15
    mov eax, edi
    shr eax, 2
    imul eax, BPG
    mov rdx, [lpPackB]
    add rdx, rax
    mov r8d, esi
    mov r9d, edi
    call lp_k12x4
    add edi, 4
    jmp .p12
.p12d:
    add esi, 12
    jmp .i12
.i8:
    lea eax, [esi+8]
    cmp eax, r13d
    ja .irem
    mov r15, [lpPackA]
    test r15, r15
    jz .done
    mov eax, esi
    imul eax, [lpGK]
    shl rax, 3
    add r15, rax
    xor edi, edi
    mov ebx, [rsp]
    align 32
.p8:
    cmp edi, ebx
    jae .p8d
    mov rcx, r15
    mov eax, edi
    shr eax, 2
    imul eax, BPG
    mov rdx, [lpPackB]
    add rdx, rax
    mov r8d, esi
    mov r9d, edi
    cmp dword [lpIsaUse], 2
    jb .kavx
    call lp_k8x4
    jmp .kdn
.kavx:
    call lp_k8x4_avx
.kdn:
    add edi, 4
    jmp .p8
.p8d:
    add esi, 8
    jmp .i8
.irem:
    cmp esi, r13d
    jae .jrem
    xor edi, edi
    mov ebx, [lpGN]
.ir:
    cmp edi, ebx
    jae .ird
    mov ecx, esi
    mov edx, edi
    call lp_krest
    inc edi
    jmp .ir
.ird:
    inc esi
    jmp .irem
.jrem:
    mov edi, [rsp]
    cmp edi, [lpGN]
    jae .done
.j1:
    cmp edi, [lpGN]
    jae .done
    mov esi, r12d
.j1i:
    cmp esi, r13d
    jae .nj1
    mov ecx, esi
    mov edx, edi
    call lp_krest
    inc esi
    jmp .j1i
.nj1:
    inc edi
    jmp .j1
.sse:
    mov ebx, r12d
.sj:
    cmp ebx, r13d
    jae .done
    xor esi, esi
.si:
    cmp esi, [lpGN]
    jae .sn
    mov ecx, ebx
    mov edx, esi
    call lp_krest
    inc esi
    jmp .si
.sn:
    inc ebx
    jmp .sj
.done:
    lea rsp, [rbp-56]
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

align 32
lp_dgemm_cols:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    and rsp, -64
    sub rsp, 40h
    mov [rsp+20h], rcx
    mov [rsp+28h], rdx
    call lp_pack_ensure
    mov rcx, [rsp+20h]
    mov rdx, [rsp+28h]
    test rax, rax
    jnz .gotpk
    mov eax, (NC/4)*BPG+APS+256
.prb:
    cmp eax, 4096
    jb .prl
    sub rsp, 4096
    mov byte [rsp], 0
    sub eax, 4096
    jmp .prb
.prl:
    sub rsp, rax
    lea rax, [rsp+64]
.gotpk:
    mov r14, rax
    mov r12d, ecx
    mov r13d, edx
    cmp dword [lpIsaUse], 1
    jb .sse
.jnc:
    mov eax, r13d
    sub eax, r12d
    cmp eax, 4
    jb .jrem
    cmp eax, NC
    jbe .ncap
    mov eax, NC
.ncap:
    and eax, ~3
    mov [rsp], eax
    xor edi, edi
.pkb:
    cmp edi, [rsp]
    jae .pkbd
    lea ecx, [r12d+edi]
    mov eax, edi
    shr eax, 2
    imul eax, BPG
    lea rdx, [r14+rax]
    call lp_pack_b
    add edi, 4
    jmp .pkb
.pkbd:
    xor r15d, r15d
.ic:
    cmp r15d, [lpGM]
    jae .incj
    mov eax, r15d
    add eax, MC
    cmp eax, [lpGM]
    cmova eax, [lpGM]
    mov [rsp+4], r15d
    mov [rsp+8], eax
    mov r8, [lpPackA]
    test r8, r8
    jnz .useA
    mov ecx, r15d
    mov edx, eax
    mov r8, r14
    add r8, (NC/4)*BPG
    call lp_pack_a
.useA:
    mov esi, [rsp+4]
    cmp dword [lpIsaUse], 2
    jb .i8
.i12:
    lea eax, [esi+12]
    cmp eax, [rsp+8]
    ja .i8
    mov r15, [lpPackA]
    test r15, r15
    jnz .a12
    lea r15, [r14+(NC/4)*BPG]
    mov eax, esi
    sub eax, [rsp+4]
    imul eax, [lpGK]
    shl rax, 3
    add r15, rax
    jmp .g12
.a12:
    mov eax, esi
    imul eax, [lpGK]
    shl rax, 3
    add r15, rax
.g12:
    xor edi, edi
    mov ebx, [rsp]
    align 32
.p12:
    cmp edi, ebx
    jae .p12d
    mov rcx, r15
    mov eax, edi
    shr eax, 2
    imul eax, BPG
    lea rdx, [r14+rax]
    mov r8d, esi
    lea r9d, [r12d+edi]
    call lp_k12x4
    add edi, 4
    jmp .p12
.p12d:
    add esi, 12
    jmp .i12
.i8:
    lea eax, [esi+8]
    cmp eax, [rsp+8]
    ja .irem
    mov r15, [lpPackA]
    test r15, r15
    jnz .a8
    lea r15, [r14+(NC/4)*BPG]
    mov eax, esi
    sub eax, [rsp+4]
    imul eax, [lpGK]
    shl rax, 3
    add r15, rax
    jmp .g8
.a8:
    mov eax, esi
    imul eax, [lpGK]
    shl rax, 3
    add r15, rax
.g8:
    xor edi, edi
    mov ebx, [rsp]
    align 32
.p8:
    cmp edi, ebx
    jae .p8d
    mov rcx, r15
    mov eax, edi
    shr eax, 2
    imul eax, BPG
    lea rdx, [r14+rax]
    mov r8d, esi
    lea r9d, [r12d+edi]
    cmp dword [lpIsaUse], 2
    jb .kavx
    call lp_k8x4
    jmp .kdn
.kavx:
    call lp_k8x4_avx
.kdn:
    add edi, 4
    jmp .p8
.p8d:
    add esi, 8
    jmp .i8
.irem:
    cmp esi, [rsp+8]
    jae .nxti
    xor edi, edi
    mov ebx, [rsp]
.ir:
    cmp edi, ebx
    jae .ird
    mov ecx, esi
    lea edx, [r12d+edi]
    call lp_krest
    inc edi
    jmp .ir
.ird:
    inc esi
    jmp .irem
.nxti:
    mov r15d, [rsp+8]
    jmp .ic
.incj:
    add r12d, [rsp]
    jmp .jnc
.jrem:
    cmp r12d, r13d
    jae .done
    xor esi, esi
.j1i:
    cmp esi, [lpGM]
    jae .nj1
    mov ecx, esi
    mov edx, r12d
    call lp_krest
    inc esi
    jmp .j1i
.nj1:
    inc r12d
    jmp .jrem
.sse:
    mov ebx, r12d
.sj:
    cmp ebx, r13d
    jae .done
    xor esi, esi
.si:
    cmp esi, [lpGM]
    jae .sn
    mov ecx, esi
    mov edx, ebx
    call lp_krest
    inc esi
    jmp .si
.sn:
    inc ebx
    jmp .sj
.done:
    lea rsp, [rbp-56]
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

align 32
lp_k12x4:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov rsi, rcx
    mov rdi, rdx
    mov r14, [lpGLDA]
    lea r10, [r14*8]
    mov rbx, [lpGC]
    movsxd rax, r8d
    lea rbx, [rbx+rax*8]
    movsxd rax, r9d
    imul rax, r10
    add rbx, rax
    lea r12, [rbx+r10]
    lea r13, [r12+r10]
    lea r15, [r13+r10]
    vmovapd ymm0, [rbx]
    vmovapd ymm1, [rbx+32]
    vmovapd ymm2, [rbx+64]
    vmovapd ymm3, [r12]
    vmovapd ymm4, [r12+32]
    vmovapd ymm5, [r12+64]
    vmovapd ymm6, [r13]
    vmovapd ymm7, [r13+32]
    vmovapd ymm8, [r13+64]
    vmovapd ymm9, [r15]
    vmovapd ymm10, [r15+32]
    vmovapd ymm11, [r15+64]
    prefetcht0 [rbx+96]
    prefetcht0 [r12+96]
    prefetcht0 [r13+96]
    prefetcht0 [r15+96]
    prefetcht0 [rbx+r10]
    mov ecx, [lpGK]
    mov eax, ecx
    and eax, ~3
    jz .tail
    align 32
.u4:
    vmovapd ymm12, [rsi]
    vmovapd ymm13, [rsi+32]
    vmovapd ymm14, [rsi+64]
    prefetcht0 [rsi+384]
    prefetcht0 [rdi+256]
    vbroadcastsd ymm15, [rdi]
    vfnmadd231pd ymm0, ymm12, ymm15
    vfnmadd231pd ymm1, ymm13, ymm15
    vfnmadd231pd ymm2, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+8]
    vfnmadd231pd ymm3, ymm12, ymm15
    vfnmadd231pd ymm4, ymm13, ymm15
    vfnmadd231pd ymm5, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+16]
    vfnmadd231pd ymm6, ymm12, ymm15
    vfnmadd231pd ymm7, ymm13, ymm15
    vfnmadd231pd ymm8, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+24]
    vfnmadd231pd ymm9, ymm12, ymm15
    vfnmadd231pd ymm10, ymm13, ymm15
    vfnmadd231pd ymm11, ymm14, ymm15
    vmovapd ymm12, [rsi+96]
    vmovapd ymm13, [rsi+128]
    vmovapd ymm14, [rsi+160]
    vbroadcastsd ymm15, [rdi+32]
    vfnmadd231pd ymm0, ymm12, ymm15
    vfnmadd231pd ymm1, ymm13, ymm15
    vfnmadd231pd ymm2, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+40]
    vfnmadd231pd ymm3, ymm12, ymm15
    vfnmadd231pd ymm4, ymm13, ymm15
    vfnmadd231pd ymm5, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+48]
    vfnmadd231pd ymm6, ymm12, ymm15
    vfnmadd231pd ymm7, ymm13, ymm15
    vfnmadd231pd ymm8, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+56]
    vfnmadd231pd ymm9, ymm12, ymm15
    vfnmadd231pd ymm10, ymm13, ymm15
    vfnmadd231pd ymm11, ymm14, ymm15
    vmovapd ymm12, [rsi+192]
    vmovapd ymm13, [rsi+224]
    vmovapd ymm14, [rsi+256]
    vbroadcastsd ymm15, [rdi+64]
    vfnmadd231pd ymm0, ymm12, ymm15
    vfnmadd231pd ymm1, ymm13, ymm15
    vfnmadd231pd ymm2, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+72]
    vfnmadd231pd ymm3, ymm12, ymm15
    vfnmadd231pd ymm4, ymm13, ymm15
    vfnmadd231pd ymm5, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+80]
    vfnmadd231pd ymm6, ymm12, ymm15
    vfnmadd231pd ymm7, ymm13, ymm15
    vfnmadd231pd ymm8, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+88]
    vfnmadd231pd ymm9, ymm12, ymm15
    vfnmadd231pd ymm10, ymm13, ymm15
    vfnmadd231pd ymm11, ymm14, ymm15
    vmovapd ymm12, [rsi+288]
    vmovapd ymm13, [rsi+320]
    vmovapd ymm14, [rsi+352]
    add rsi, 384
    vbroadcastsd ymm15, [rdi+96]
    vfnmadd231pd ymm0, ymm12, ymm15
    vfnmadd231pd ymm1, ymm13, ymm15
    vfnmadd231pd ymm2, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+104]
    vfnmadd231pd ymm3, ymm12, ymm15
    vfnmadd231pd ymm4, ymm13, ymm15
    vfnmadd231pd ymm5, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+112]
    vfnmadd231pd ymm6, ymm12, ymm15
    vfnmadd231pd ymm7, ymm13, ymm15
    vfnmadd231pd ymm8, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+120]
    add rdi, 128
    vfnmadd231pd ymm9, ymm12, ymm15
    vfnmadd231pd ymm10, ymm13, ymm15
    vfnmadd231pd ymm11, ymm14, ymm15
    sub eax, 4
    jnz .u4
.tail:
    and ecx, 3
    jz .st
.t1:
    vmovapd ymm12, [rsi]
    vmovapd ymm13, [rsi+32]
    vmovapd ymm14, [rsi+64]
    vbroadcastsd ymm15, [rdi]
    vfnmadd231pd ymm0, ymm12, ymm15
    vfnmadd231pd ymm1, ymm13, ymm15
    vfnmadd231pd ymm2, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+8]
    vfnmadd231pd ymm3, ymm12, ymm15
    vfnmadd231pd ymm4, ymm13, ymm15
    vfnmadd231pd ymm5, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+16]
    vfnmadd231pd ymm6, ymm12, ymm15
    vfnmadd231pd ymm7, ymm13, ymm15
    vfnmadd231pd ymm8, ymm14, ymm15
    vbroadcastsd ymm15, [rdi+24]
    vfnmadd231pd ymm9, ymm12, ymm15
    vfnmadd231pd ymm10, ymm13, ymm15
    vfnmadd231pd ymm11, ymm14, ymm15
    add rsi, 96
    add rdi, 32
    sub ecx, 1
    jnz .t1
.st:
    vmovapd [rbx], ymm0
    vmovapd [rbx+32], ymm1
    vmovapd [rbx+64], ymm2
    vmovapd [r12], ymm3
    vmovapd [r12+32], ymm4
    vmovapd [r12+64], ymm5
    vmovapd [r13], ymm6
    vmovapd [r13+32], ymm7
    vmovapd [r13+64], ymm8
    vmovapd [r15], ymm9
    vmovapd [r15+32], ymm10
    vmovapd [r15+64], ymm11
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_k8x4:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov rsi, rcx
    mov rdi, rdx
    mov r14, [lpGLDA]
    lea r10, [r14*8]
    mov rbx, [lpGC]
    movsxd rax, r8d
    lea rbx, [rbx+rax*8]
    movsxd rax, r9d
    imul rax, r10
    add rbx, rax
    lea r12, [rbx+r10]
    lea r13, [r12+r10]
    lea r15, [r13+r10]
    vmovapd ymm0, [rbx]
    vmovapd ymm1, [rbx+32]
    vmovapd ymm2, [r12]
    vmovapd ymm3, [r12+32]
    vmovapd ymm4, [r13]
    vmovapd ymm5, [r13+32]
    vmovapd ymm6, [r15]
    vmovapd ymm7, [r15+32]
    prefetcht0 [rbx+r10*4]
    mov ecx, [lpGK]
    mov eax, ecx
    and eax, ~3
    jz .tail
    align 32
.u4:
    vmovapd ymm14, [rsi]
    vmovapd ymm15, [rsi+32]
    prefetcht0 [rsi+512]
    prefetcht0 [rdi+256]
    vbroadcastsd ymm8, [rdi]
    vbroadcastsd ymm9, [rdi+8]
    vbroadcastsd ymm10, [rdi+16]
    vbroadcastsd ymm11, [rdi+24]
    vfnmadd231pd ymm0, ymm14, ymm8
    vfnmadd231pd ymm1, ymm15, ymm8
    vfnmadd231pd ymm2, ymm14, ymm9
    vfnmadd231pd ymm3, ymm15, ymm9
    vfnmadd231pd ymm4, ymm14, ymm10
    vfnmadd231pd ymm5, ymm15, ymm10
    vfnmadd231pd ymm6, ymm14, ymm11
    vfnmadd231pd ymm7, ymm15, ymm11
    vmovapd ymm14, [rsi+64]
    vmovapd ymm15, [rsi+96]
    vbroadcastsd ymm8, [rdi+32]
    vbroadcastsd ymm9, [rdi+40]
    vbroadcastsd ymm10, [rdi+48]
    vbroadcastsd ymm11, [rdi+56]
    vfnmadd231pd ymm0, ymm14, ymm8
    vfnmadd231pd ymm1, ymm15, ymm8
    vfnmadd231pd ymm2, ymm14, ymm9
    vfnmadd231pd ymm3, ymm15, ymm9
    vfnmadd231pd ymm4, ymm14, ymm10
    vfnmadd231pd ymm5, ymm15, ymm10
    vfnmadd231pd ymm6, ymm14, ymm11
    vfnmadd231pd ymm7, ymm15, ymm11
    vmovapd ymm14, [rsi+128]
    vmovapd ymm15, [rsi+160]
    vbroadcastsd ymm8, [rdi+64]
    vbroadcastsd ymm9, [rdi+72]
    vbroadcastsd ymm10, [rdi+80]
    vbroadcastsd ymm11, [rdi+88]
    vfnmadd231pd ymm0, ymm14, ymm8
    vfnmadd231pd ymm1, ymm15, ymm8
    vfnmadd231pd ymm2, ymm14, ymm9
    vfnmadd231pd ymm3, ymm15, ymm9
    vfnmadd231pd ymm4, ymm14, ymm10
    vfnmadd231pd ymm5, ymm15, ymm10
    vfnmadd231pd ymm6, ymm14, ymm11
    vfnmadd231pd ymm7, ymm15, ymm11
    vmovapd ymm14, [rsi+192]
    vmovapd ymm15, [rsi+224]
    add rsi, 256
    vbroadcastsd ymm8, [rdi+96]
    vbroadcastsd ymm9, [rdi+104]
    vbroadcastsd ymm10, [rdi+112]
    vbroadcastsd ymm11, [rdi+120]
    add rdi, 128
    vfnmadd231pd ymm0, ymm14, ymm8
    vfnmadd231pd ymm1, ymm15, ymm8
    vfnmadd231pd ymm2, ymm14, ymm9
    vfnmadd231pd ymm3, ymm15, ymm9
    vfnmadd231pd ymm4, ymm14, ymm10
    vfnmadd231pd ymm5, ymm15, ymm10
    vfnmadd231pd ymm6, ymm14, ymm11
    vfnmadd231pd ymm7, ymm15, ymm11
    sub eax, 4
    jnz .u4
.tail:
    and ecx, 3
    jz .st
.t1:
    vmovapd ymm14, [rsi]
    vmovapd ymm15, [rsi+32]
    add rsi, 64
    vbroadcastsd ymm8, [rdi]
    vbroadcastsd ymm9, [rdi+8]
    vbroadcastsd ymm10, [rdi+16]
    vbroadcastsd ymm11, [rdi+24]
    add rdi, 32
    vfnmadd231pd ymm0, ymm14, ymm8
    vfnmadd231pd ymm1, ymm15, ymm8
    vfnmadd231pd ymm2, ymm14, ymm9
    vfnmadd231pd ymm3, ymm15, ymm9
    vfnmadd231pd ymm4, ymm14, ymm10
    vfnmadd231pd ymm5, ymm15, ymm10
    vfnmadd231pd ymm6, ymm14, ymm11
    vfnmadd231pd ymm7, ymm15, ymm11
    sub ecx, 1
    jnz .t1
.st:
    vmovapd [rbx], ymm0
    vmovapd [rbx+32], ymm1
    vmovapd [r12], ymm2
    vmovapd [r12+32], ymm3
    vmovapd [r13], ymm4
    vmovapd [r13+32], ymm5
    vmovapd [r15], ymm6
    vmovapd [r15+32], ymm7
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_k8x4_avx:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov rsi, rcx
    mov rdi, rdx
    mov r14, [lpGLDA]
    lea r10, [r14*8]
    mov rbx, [lpGC]
    movsxd rax, r8d
    lea rbx, [rbx+rax*8]
    movsxd rax, r9d
    imul rax, r10
    add rbx, rax
    lea r12, [rbx+r10]
    lea r13, [r12+r10]
    lea r15, [r13+r10]
    vmovupd ymm0, [rbx]
    vmovupd ymm1, [rbx+32]
    vmovupd ymm2, [r12]
    vmovupd ymm3, [r12+32]
    vmovupd ymm4, [r13]
    vmovupd ymm5, [r13+32]
    vmovupd ymm6, [r15]
    vmovupd ymm7, [r15+32]
    mov ecx, [lpGK]
    test ecx, ecx
    jz .st
.kk:
    vmovapd ymm14, [rsi]
    vmovapd ymm15, [rsi+32]
    add rsi, 64
    vbroadcastsd ymm8, [rdi]
    vbroadcastsd ymm9, [rdi+8]
    vbroadcastsd ymm10, [rdi+16]
    vbroadcastsd ymm11, [rdi+24]
    add rdi, 32
    vmulpd ymm13, ymm14, ymm8
    vsubpd ymm0, ymm0, ymm13
    vmulpd ymm13, ymm15, ymm8
    vsubpd ymm1, ymm1, ymm13
    vmulpd ymm13, ymm14, ymm9
    vsubpd ymm2, ymm2, ymm13
    vmulpd ymm13, ymm15, ymm9
    vsubpd ymm3, ymm3, ymm13
    vmulpd ymm13, ymm14, ymm10
    vsubpd ymm4, ymm4, ymm13
    vmulpd ymm13, ymm15, ymm10
    vsubpd ymm5, ymm5, ymm13
    vmulpd ymm13, ymm14, ymm11
    vsubpd ymm6, ymm6, ymm13
    vmulpd ymm13, ymm15, ymm11
    vsubpd ymm7, ymm7, ymm13
    sub ecx, 1
    jnz .kk
.st:
    vmovupd [rbx], ymm0
    vmovupd [rbx+32], ymm1
    vmovupd [r12], ymm2
    vmovupd [r12+32], ymm3
    vmovupd [r13], ymm4
    vmovupd [r13+32], ymm5
    vmovupd [r15], ymm6
    vmovupd [r15+32], ymm7
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_krest:
    push rbx
    push rsi
    push rdi
    mov rsi, [lpGA]
    mov rdi, [lpGB]
    mov rbx, [lpGC]
    mov r8, [lpGLDA]
    lea r9, [r8*8]
    movsxd rax, ecx
    lea rsi, [rsi+rax*8]
    lea rbx, [rbx+rax*8]
    movsxd rax, edx
    imul rax, r9
    add rdi, rax
    add rbx, rax
    vxorpd xmm0, xmm0, xmm0
    mov ecx, [lpGK]
    test ecx, ecx
    jz .s
    align 32
.kr:
    vmovsd xmm1, [rsi]
    vmulsd xmm1, xmm1, [rdi]
    vaddsd xmm0, xmm0, xmm1
    add rsi, r9
    add rdi, 8
    sub ecx, 1
    jnz .kr
.s:
    vmovsd xmm2, [rbx]
    vsubsd xmm2, xmm2, xmm0
    vmovsd [rbx], xmm2
    pop rdi
    pop rsi
    pop rbx
    ret
