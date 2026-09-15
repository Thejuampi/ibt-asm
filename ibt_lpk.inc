MEM_COMMIT      equ 1000h
MEM_RESERVE     equ 2000h
MEM_RELEASE     equ 8000h
PAGE_READWRITE  equ 4
INFINITE        equ 0FFFFFFFFh
THR_HIGHEST     equ 2
THR_TIMECRIT    equ 15
THR_NORMAL      equ 0
NB              equ 384
PB              equ 64
MC              equ 24
NC              equ 48
BPG             equ 32*NB
APS             equ MC*NB*8

section .data
align 32
lpC10       dq 10.0
lpC1        dq 1.0
lpC0        dq 0.0
lpHalf      dq 0.5
lpTwo24     dq 16777216.0
lpEps       dq 2.220446049250313e-16
lpTiny      dq 1.0e-300
lpTwo3      dq 0.6666666666666666
lp1e9       dq 1.0e9
lpC16       dq 16.0
lpMaskAbs   dq 7FFFFFFFFFFFFFFFh, 7FFFFFFFFFFFFFFFh, 7FFFFFFFFFFFFFFFh, 7FFFFFFFFFFFFFFFh
%if SELFTEST
szTestOut   dw __utf16le__("lpk_test.txt"),0
szProfOut   dw __utf16le__("lpk_prof.txt"),0
%endif

section .text

%if SELFTEST
test_main:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    and rsp, -16
    sub rsp, 80h
    call paths_init
    mov dword [lpTest], 1
    mov dword [failed], 0
    mov dword [lpIsaForce], 0
    call lp_isa_probe
    lea rcx, [lpFreq]
    call QueryPerformanceFrequency
    xor r12d, r12d
    call lp_test_open
    cmp rax, -1
    je .die
    mov r15, rax
    mov dword [lpNthr], 1
    mov edi, 8
    call lp_test_case
    mov edi, 24
    call lp_test_case
    mov edi, 40
    call lp_test_case
    mov edi, 68
    call lp_test_case
    mov edi, 72
    call lp_test_case
    mov edi, 96
    call lp_test_case
    mov edi, 256
    call lp_test_case
    mov edi, 1024
    call lp_test_case
    mov edi, 2048
    call lp_test_case
    mov eax, [nproc]
    test eax, eax
    jnz .nth
    mov eax, 20
.nth:
    mov [lpNthr], eax
    mov edi, 1024
    call lp_test_case
    mov edi, 2048
    call lp_test_case
    mov edi, 4096
    call lp_test_case
    mov dword [lpNthr], 1
    mov edi, 4096
    call lp_test_case
    mov eax, [nproc]
    test eax, eax
    jnz .nth2
    mov eax, 20
.nth2:
    mov [lpNthr], eax
    mov dword [lpNthr], 1
    call lp_test_dup
    call lp_test_diffn
    mov dword [lpIsaForce], 1
    call lp_isa_probe
    mov edi, 40
    call lp_test_case
    mov edi, 68
    call lp_test_case
    mov edi, 96
    call lp_test_case
    mov dword [lpIsaForce], 0
    call lp_isa_probe
    mov dword [lpNthr], 2
    mov edi, 72
    call lp_test_case
    mov rcx, r15
    call CloseHandle
    test r12d, r12d
    jnz .die
    xor ecx, ecx
    call ExitProcess
.die:
    mov ecx, 1
    call ExitProcess

lp_test_open:
    push rbp
    mov rbp, rsp
    sub rsp, 40h
    lea rcx, [logPath]
    lea rdx, [exeDir]
    call lstrcpyW
    lea rcx, [logPath]
    lea rdx, [szTestOut]
    call lstrcatW
    lea rcx, [logPath]
    mov edx, GENERIC_WRITE
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+20h], CREATE_ALWAYS
    mov qword [rsp+28h], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp+30h], 0
    call CreateFileW
    leave
    ret

lp_test_wr:
    push rbp
    mov rbp, rsp
    push rsi
    sub rsp, 38h
    lea rsi, [lineAcc]
    xor ecx, ecx
.ln:
    cmp byte [rsi+rcx], 0
    je .lnd
    inc ecx
    jmp .ln
.lnd:
    mov rdx, rsi
    mov r8d, ecx
    mov rcx, r15
    lea r9, [rsp+20h]
    mov qword [rsp+20h], 0
    call WriteFile
    mov byte [rsp+30h], 10
    mov rcx, r15
    lea rdx, [rsp+30h]
    mov r8d, 1
    lea r9, [rsp+20h]
    mov qword [rsp+20h], 0
    call WriteFile
    add rsp, 38h
    pop rsi
    pop rbp
    ret

lp_prof_wr:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    and rsp, -16
    sub rsp, 60h
    lea rcx, [logPath]
    lea rdx, [exeDir]
    call lstrcpyW
    lea rcx, [logPath]
    lea rdx, [szProfOut]
    call lstrcatW
    lea rcx, [logPath]
    mov edx, FILE_APPEND_DATA
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+20h], OPEN_ALWAYS
    mov qword [rsp+28h], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp+30h], 0
    call CreateFileW
    cmp rax, -1
    je .o
    mov rbx, rax
    lea rdi, [lineAcc]
    mov rax, [lpFreq]
    test rax, rax
    jnz .fr
    mov rax, 1
.fr:
    movsd xmm5, xmm2
    mov byte [rdi], 'P'
    inc rdi
    mov byte [rdi], '='
    inc rdi
    cvtsi2sd xmm0, qword [lpAccP]
    divsd xmm0, xmm5
    mov ecx, 4
    call lp_ftoa
    mov byte [rdi], ' '
    inc rdi
    mov byte [rdi], 'T'
    inc rdi
    mov byte [rdi], '='
    inc rdi
    cvtsi2sd xmm0, qword [lpAccT]
    divsd xmm0, xmm5
    mov ecx, 4
    call lp_ftoa
    mov byte [rdi], ' '
    inc rdi
    mov byte [rdi], 'G'
    inc rdi
    mov byte [rdi], '='
    inc rdi
    cvtsi2sd xmm0, qword [lpAccG]
    divsd xmm0, xmm5
    mov ecx, 4
    call lp_ftoa
    mov byte [rdi], ' '
    inc rdi
    mov byte [rdi], 'S'
    inc rdi
    mov byte [rdi], '='
    inc rdi
    cvtsi2sd xmm0, qword [lpAccS]
    divsd xmm0, xmm5
    mov ecx, 4
    call lp_ftoa
    mov byte [rdi], 0
    lea rsi, [lineAcc]
    xor ecx, ecx
.ln:
    cmp byte [rsi+rcx], 0
    je .lnd
    inc ecx
    jmp .ln
.lnd:
    mov r8d, ecx
    mov rdx, rsi
    mov rcx, rbx
    lea r9, [rsp+40h]
    mov qword [rsp+20h], 0
    call WriteFile
    mov byte [rsp+50h], 10
    mov rcx, rbx
    lea rdx, [rsp+50h]
    mov r8d, 1
    lea r9, [rsp+40h]
    mov qword [rsp+20h], 0
    call WriteFile
    mov rcx, rbx
    call CloseHandle
.o:
    lea rsp, [rbp-24]
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

lp_test_case:
    push rdi
    mov dword [lpN], edi
    call lp_run_n
    pop rdi
    test eax, eax
    jz .ok
    inc r12d
.ok:
    call lp_test_wr
    ret

lp_test_dup:
    mov dword [lpN], 32
    mov dword [lpNthr], 1
    call lp_run_n
    test eax, eax
    jz .a
    inc r12d
    jmp .wr
.a:
    mov rax, [lpTmpN]
    mov [lpNorm0], rax
    call lp_run_n
    test eax, eax
    jz .b
    inc r12d
    jmp .wr
.b:
    mov rax, [lpTmpN]
    cmp rax, [lpNorm0]
    je .wr
    inc r12d
.wr:
    call lp_test_wr
    ret

lp_test_diffn:
    mov dword [lpN], 8
    mov dword [lpNthr], 1
    call lp_run_n
    mov rax, [lpTmpN]
    mov [lpNorm0], rax
    mov dword [lpN], 24
    call lp_run_n
    mov rax, [lpTmpN]
    cmp rax, [lpNorm0]
    jne .ok
    inc r12d
.ok:
    ret

lp_run_n:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 48h
    mov dword [lpStop], 0
    mov dword [lpDie], 0
    mov dword [lpPhase], 0
    mov dword [lpDone], 0
    call lp_alloc_n
    test eax, eax
    jz .fail
    cmp dword [lpNthr], 2
    jb .nw
    call lp_workers_on
.nw:
    call lp_fill
    lea rcx, [rsp+40h]
    call QueryPerformanceCounter
    mov rbx, [rsp+40h]
    call lp_dgetrf
    call lp_acc_from_q
    call lp_dgetrs
    lea rcx, [lpAccS]
    call lp_acc_add
    lea rcx, [rsp+40h]
    call QueryPerformanceCounter
    mov rax, [rsp+40h]
    sub rax, rbx
    jnz .nz
    inc rax
.nz:
    cvtsi2sd xmm1, rax
    mov rax, [lpFreq]
    test rax, rax
    jnz .fr
    mov rax, 1
    mov [lpFreq], rax
.fr:
    cvtsi2sd xmm2, rax
    divsd xmm1, xmm2
    movsd [lpTmpT], xmm1
    call lp_resid
    call lp_prof_wr
    cmp dword [lpNthr], 2
    jb .nf
    call lp_workers_off
.nf:
    call lp_emit
    call lp_free
    movsd xmm0, [lpTmpN]
    ucomisd xmm0, [lpC16]
    jp .fail
    ja .fail
    xor eax, eax
    add rsp, 48h
    pop rbx
    pop rbp
    ret
.fail:
    call lp_workers_off
    call lp_free
    mov eax, 1
    add rsp, 48h
    pop rbx
    pop rbp
    ret

%endif
lp_isa_probe:
    push rbx
    push rsi
    mov dword [lpIsa], 1
    mov eax, 1
    cpuid
    mov esi, ecx
    mov eax, ecx
    and eax, (1 << 27) | (1 << 28)
    cmp eax, (1 << 27) | (1 << 28)
    jne .pick
    xor ecx, ecx
    xgetbv
    and eax, 6
    cmp eax, 6
    jne .pick
    or dword [lpIsa], 2
    test esi, (1 << 12)
    jz .nf
    or dword [lpIsa], 8
.nf:
    mov eax, 7
    xor ecx, ecx
    cpuid
    test ebx, (1 << 5)
    jz .pick
    or dword [lpIsa], 4
.pick:
    xor edx, edx
    test dword [lpIsa], 2
    jz .force
    mov edx, 1
    mov eax, [lpIsa]
    and eax, 12
    cmp eax, 12
    jne .force
    mov edx, 2
.force:
    mov eax, [lpIsaForce]
    test eax, eax
    jz .store
    dec eax
    cmp eax, edx
    jbe .cap
    mov eax, edx
.cap:
    mov edx, eax
.store:
    mov [lpIsaUse], edx
    pop rsi
    pop rbx
    ret

lp_vzero:
    test dword [lpIsa], 2
    jz .r
    vzeroupper
.r:
    ret

align 32
lp_wake:
    sub rsp, 28h
    call WakeByAddressAll
    add rsp, 28h
    ret

align 32
lp_acc_from_q:
    sub rsp, 28h
    lea rcx, [lpQ0]
    call QueryPerformanceCounter
    add rsp, 28h
    ret

align 32
lp_acc_add:
    push rbx
    sub rsp, 30h
    mov rbx, rcx
    lea rcx, [rsp+20h]
    call QueryPerformanceCounter
    mov rax, [rsp+20h]
    sub rax, [lpQ0]
    add [rbx], rax
    add rsp, 30h
    pop rbx
    ret

lp_count_pcores:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    and rsp, -16
    mov eax, 16384
.pr:
    cmp eax, 4096
    jb .pl
    sub rsp, 4096
    mov byte [rsp], 0
    sub eax, 4096
    jmp .pr
.pl:
    sub rsp, rax
    sub rsp, 40h
    mov dword [rsp+20h], 16384
    xor ecx, ecx
    lea rdx, [rsp+40h]
    lea r8, [rsp+20h]
    call GetLogicalProcessorInformationEx
    test eax, eax
    jz .fail
    lea rsi, [rsp+40h]
    mov edx, [rsp+20h]
    lea rdi, [rsi+rdx]
    xor ebx, ebx
.walk:
    cmp rsi, rdi
    jae .done
    mov eax, [rsi]
    mov ecx, [rsi+4]
    cmp ecx, 32
    jb .fail
    test eax, eax
    jnz .next
    cmp byte [rsi+9], 0
    jne .next
    inc ebx
.next:
    mov eax, [rsi+4]
    add rsi, rax
    jmp .walk
.done:
    test ebx, ebx
    jz .fail
    mov [lpPcores], ebx
    jmp .out
.fail:
    mov eax, [nproc]
    add eax, 1
    shr eax, 1
    cmp eax, 1
    jae .fb
    mov eax, 1
.fb:
    mov [lpPcores], eax
.out:
    lea rsp, [rbp-24]
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

isa_bits_apply:
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    mov rcx, [hLblBits]
    test rcx, rcx
    jz .o
    lea rdx, [szBitsSse2]
    cmp dword [lpIsaUse], 1
    jb .set
    lea rdx, [szBitsAvx]
    cmp dword [lpIsaUse], 2
    jb .set
    lea rdx, [szBitsAvx2]
.set:
    call SetWindowTextW
.o:
    leave
    ret

burn_thread:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    and rsp, -16
    sub rsp, 80h
    mov dword [lpStop], 0
    mov dword [lpDie], 0
    mov dword [lpPhase], 0
    mov dword [lpDone], 0
    lea rcx, [lpFreq]
    call QueryPerformanceFrequency
    cmp qword [lpFreq], 0
    jne .fq
    mov qword [lpFreq], 1
.fq:
    call thread_count
    cmp eax, 1
    jae .t1
    mov eax, 1
.t1:
    cmp eax, 32
    jbe .t2
    mov eax, 32
.t2:
    mov [lpNthr], eax
    mov rcx, -2
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .pri
    mov edx, THR_TIMECRIT
.pri:
    call SetThreadPriority
    lea rcx, [szPreparing]
    call title_status
    call lp_alloc
    test eax, eax
    jnz .gotmem
    mov dword [lpAllocFail], 1
    jmp .nodie
.gotmem:
    call lp_workers_on
    call lp_pack_ensure
    mov rax, [lpA0]
    mov [lpA], rax
    mov rax, [lpX0]
    mov [lpX], rax
    mov rcx, [lpA0]
    call lp_touch_a
    call lp_fill
    mov rcx, [hMain]
    test rcx, rcx
    jz .go
    mov edx, WM_PHASE
    mov r8d, 1
    xor r9d, r9d
    call SendMessageW
.go:
    xor r13d, r13d
    xor ecx, ecx
    call title_progress
.pass:
    cmp dword [lpStop], 0
    jne .fin
    cmp r13d, [targetRuns]
    jae .fin
    call lp_fill_x
    lea rcx, [rsp+60h]
    call QueryPerformanceCounter
    mov r12, [rsp+60h]
    call lp_dgetrf
    cmp dword [lpStop], 0
    jne .fin
    call lp_acc_from_q
    call lp_dgetrs
    lea rcx, [lpAccS]
    call lp_acc_add
    lea rcx, [rsp+60h]
    call QueryPerformanceCounter
    mov rax, [rsp+60h]
    sub rax, r12
    jnz .tick
    inc rax
.tick:
    cvtsi2sd xmm1, rax
    cvtsi2sd xmm2, qword [lpFreq]
    divsd xmm1, xmm2
    movsd [lpTmpT], xmm1
    mov rax, [lpX]
    mov [lpResidX], rax
    call lp_resid
    movsd xmm0, [lpTmpN]
    ucomisd xmm0, [lpC16]
    jp .bad
    ja .bad
    call lp_emit
    inc r13d
    cmp r13d, [targetRuns]
    jae .fin
    cmp dword [lpStop], 0
    jne .fin
    mov rcx, [lpA]
    call lp_fill_a
    jmp .pass
.bad:
    call lp_emit
    mov dword [failed], 1
    mov dword [lpStop], 1
.fin:
    call lp_workers_off
    call lp_free
.nodie:
    call lp_vzero
    cmp dword [lpTest], 0
    jne .skip
    mov rcx, [hMain]
    mov edx, WM_DONE
    xor r8d, r8d
    xor r9d, r9d
    call PostMessageW
.skip:
    xor eax, eax
    leave
    ret

lp_alloc:
    push rbp
    mov rbp, rsp
    push rbx
    and rsp, -16
    sub rsp, 40h
    mov eax, [stressMB]
    mov ecx, eax
    shl rcx, 17
    call isqrt
    and eax, ~7
    cmp eax, 8
    jae .n
    mov eax, 8
.n:
    mov [lpN], eax
    call lp_alloc_n
    leave
    ret

lp_alloc_n:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    and rsp, -16
    sub rsp, 40h
    mov eax, [lpN]
    cmp eax, 8
    jae .n8
    mov eax, 8
.n8:
    mov [lpN], eax
    lea edx, [eax+8]
    mov [lpLDA], edx
    movsxd rcx, edx
    movsxd rax, eax
    imul rcx, rax
    shl rcx, 3
    mov rbx, rcx
    xor ecx, ecx
    mov rdx, rbx
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    test rax, rax
    jz .fail
    mov [lpA], rax
    mov [lpA0], rax
    mov qword [lpA1], 0
    mov dword [lpNBuf], 1
    movsxd rcx, dword [lpN]
    add rcx, 8
    shl rcx, 3
    mov rsi, rcx
    xor ecx, ecx
    mov rdx, rsi
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [lpX], rax
    mov [lpX0], rax
    mov qword [lpX1], 0
    xor ecx, ecx
    mov rdx, rsi
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [lpB], rax
    xor ecx, ecx
    mov rdx, rsi
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [lpWrk], rax
    movsxd rcx, dword [lpN]
    shl rcx, 2
    add rcx, 64
    xor eax, eax
    xchg rax, rcx
    mov rdx, rax
    xor ecx, ecx
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [lpIpiv], rax
    cmp qword [lpA], 0
    je .fail
    cmp qword [lpX], 0
    je .fail
    cmp qword [lpB], 0
    je .fail
    cmp qword [lpWrk], 0
    je .fail
    cmp qword [lpIpiv], 0
    je .fail
    movsxd rcx, dword [lpN]
    add rcx, 16
    imul rcx, NB
    shl rcx, 3
    xor eax, eax
    xchg rax, rcx
    mov rdx, rax
    xor ecx, ecx
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [lpPackA], rax
    test rax, rax
    jz .fail
    movsxd rcx, dword [lpN]
    add rcx, 16
    imul rcx, NB
    shl rcx, 3
    xor eax, eax
    xchg rax, rcx
    mov rdx, rax
    xor ecx, ecx
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    mov [lpPackB], rax
    mov dword [lpNBuf], 1
    mov qword [lpA1], 0
    mov qword [lpX1], 0
    mov rcx, [lpPackA]
    movsxd rdx, dword [lpN]
    add rdx, 16
    imul rdx, NB
    shl rdx, 3
    call lp_touch
    mov rcx, [lpPackB]
    movsxd rdx, dword [lpN]
    add rdx, 16
    imul rdx, NB
    shl rdx, 3
    call lp_touch
    mov eax, 1
    leave
    ret
.fail:
    xor eax, eax
    leave
    ret

lp_free:
    push rbp
    mov rbp, rsp
    push rbx
    and rsp, -16
    sub rsp, 40h
    mov rcx, [lpA0]
    test rcx, rcx
    jz .a1
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
.a1:
    mov rcx, [lpA1]
    test rcx, rcx
    jz .x0
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
.x0:
    mov rcx, [lpX0]
    test rcx, rcx
    jz .x1
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
.x1:
    mov rcx, [lpX1]
    test rcx, rcx
    jz .b
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
.b:
    mov qword [lpA], 0
    mov qword [lpA0], 0
    mov qword [lpA1], 0
    mov qword [lpX], 0
    mov qword [lpX0], 0
    mov qword [lpX1], 0
    mov qword [lpResidX], 0
    mov rcx, [lpB]
    test rcx, rcx
    jz .w
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
    mov qword [lpB], 0
.w:
    mov rcx, [lpWrk]
    test rcx, rcx
    jz .i
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
    mov qword [lpWrk], 0
.i:
    mov rcx, [lpIpiv]
    test rcx, rcx
    jz .o
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
    mov qword [lpIpiv], 0
.o:
    mov rcx, [lpPackA]
    test rcx, rcx
    jz .p
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
    mov qword [lpPackA], 0
.p:
    mov rcx, [lpPackB]
    test rcx, rcx
    jz .q
    xor edx, edx
    mov r8d, MEM_RELEASE
    call VirtualFree
    mov qword [lpPackB], 0
.q:
    mov qword [lpA0], 0
    call lp_pack_drop
    leave
    ret

lp_touch:
    test rcx, rcx
    jz .o
    test rdx, rdx
    jz .o
    lea rdx, [rcx+rdx]
.t:
    cmp rcx, rdx
    jae .o
    mov byte [rcx], 0
    add rcx, 4096
    jmp .t
.o:
    ret

lp_pack_ensure:
    push rbx
    sub rsp, 20h
    mov ecx, [lpTlsIdx]
    cmp ecx, 0FFFFFFFFh
    je .fail
    call TlsGetValue
    test rax, rax
    jnz .ok
    xor ecx, ecx
    mov edx, (NC/4)*BPG+APS+256
    mov r8d, MEM_COMMIT|MEM_RESERVE
    mov r9d, PAGE_READWRITE
    call VirtualAlloc
    test rax, rax
    jz .fail
    mov rbx, rax
    mov ecx, [lpTlsIdx]
    mov rdx, rbx
    call TlsSetValue
    mov rcx, rbx
    mov edx, (NC/4)*BPG+APS+256
    call lp_touch
    mov rax, rbx
.ok:
    add rsp, 20h
    pop rbx
    ret
.fail:
    xor eax, eax
    add rsp, 20h
    pop rbx
    ret

lp_pack_drop:
    push rbx
    sub rsp, 20h
    mov ecx, [lpTlsIdx]
    cmp ecx, 0FFFFFFFFh
    je .o
    call TlsGetValue
    test rax, rax
    jz .z
    mov rbx, rax
    xor edx, edx
    mov r8d, MEM_RELEASE
    mov rcx, rbx
    call VirtualFree
.z:
    mov ecx, [lpTlsIdx]
    xor edx, edx
    call TlsSetValue
.o:
    add rsp, 20h
    pop rbx
    ret

lp_touch_a:
    push rbx
    sub rsp, 20h
    test rcx, rcx
    jz .o
    mov [lpGA], rcx
    mov eax, [lpN]
    mov [lpGN], eax
    mov dword [lpJob], 8
    cmp dword [lpNthr], 2
    jb .ser
    cmp eax, 8
    jb .ser
    call lp_par
    jmp .sf
.ser:
    xor ecx, ecx
    mov edx, [lpN]
    call lp_touch_cols
.sf:
    sfence
.o:
    add rsp, 20h
    pop rbx
    ret

lp_touch_cols:
    push rbx
    push rsi
    push rdi
    push r13
    push r14
    mov r13, [lpGA]
    test r13, r13
    jz .out
    mov esi, [lpN]
    mov edi, [lpLDA]
    mov ebx, ecx
    mov r14d, edx
.j:
    cmp ebx, r14d
    jae .out
    cmp ebx, esi
    jae .out
    movsxd rax, ebx
    imul rax, rdi
    lea r9, [r13+rax*8]
    mov ecx, esi
    shl ecx, 3
    xor edx, edx
.p:
    cmp edx, ecx
    jae .ni
    mov byte [r9+rdx], 0
    add edx, 4096
    jmp .p
.ni:
    inc ebx
    jmp .j
.out:
    pop r14
    pop r13
    pop rdi
    pop rsi
    pop rbx
    ret

lp_fill_a:
    push rbx
    sub rsp, 20h
    test rcx, rcx
    jz .o
    mov [lpGA], rcx
    mov eax, [lpN]
    mov [lpGN], eax
    mov dword [lpJob], 5
    cmp dword [lpNthr], 2
    jb .ser
    cmp eax, 8
    jb .ser
    call lp_par
    jmp .sf
.ser:
    xor ecx, ecx
    mov edx, [lpN]
    call lp_fill_cols
.sf:
    sfence
.o:
    add rsp, 20h
    pop rbx
    ret

lp_fill_cols:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    mov r13, [lpGA]
    test r13, r13
    jz .out
    mov esi, [lpN]
    mov edi, [lpLDA]
    mov ebx, ecx
    mov r14d, edx
.j:
    cmp ebx, r14d
    jae .out
    cmp ebx, esi
    jae .out
    mov r12d, 1664525
    imul r12d, ebx
    add r12d, 1325
    movsxd rax, ebx
    imul rax, rdi
    lea r9, [r13+rax*8]
    xor ecx, ecx
.i:
    cmp ecx, esi
    jae .ni
    imul r12d, 1664525
    add r12d, 1013904223
    mov eax, r12d
    shr eax, 8
    and eax, 00FFFFFFh
    vcvtsi2sd xmm0, xmm0, rax
    vdivsd xmm0, xmm0, [lpTwo24]
    vsubsd xmm0, xmm0, [lpHalf]
    movsxd rdx, ecx
    vmovsd [r9+rdx*8], xmm0
    inc ecx
    jmp .i
.ni:
    inc ebx
    jmp .j
.out:
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

lp_fill_ones:
    push rsi
    mov rsi, rcx
    test rsi, rsi
    jz .o
    mov edx, [lpN]
    xor ecx, ecx
    vmovsd xmm1, [lpC1]
.l:
    cmp ecx, edx
    jae .o
    movsxd rax, ecx
    vmovsd [rsi+rax*8], xmm1
    inc ecx
    jmp .l
.o:
    pop rsi
    ret

lp_fill_x:
    mov rcx, [lpX]
    jmp lp_fill_ones

lp_fill_b:
    mov rcx, [lpB]
    jmp lp_fill_ones

lp_fill:
    push rcx
    mov rcx, [lpA]
    call lp_fill_a
    pop rcx
    call lp_fill_b
    jmp lp_fill_x

align 32
lp_dgetrf:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 20h
    xor rax, rax
    mov [lpAccP], rax
    mov [lpAccT], rax
    mov [lpAccG], rax
    mov [lpAccS], rax
    mov r15d, [lpN]
    mov r14d, [lpLDA]
    xor r12d, r12d
.jloop:
    cmp r12d, r15d
    jae .done
    cmp dword [lpStop], 0
    jne .done
    mov ebx, r15d
    sub ebx, r12d
    cmp ebx, NB
    jbe .jb
    mov ebx, NB
.jb:
    call lp_acc_from_q
    mov ecx, r12d
    mov edx, ebx
    call lp_panel
    lea rcx, [lpAccP]
    call lp_acc_add
    mov eax, r12d
    add eax, ebx
    cmp eax, r15d
    jae .next
    call lp_acc_from_q
    mov ecx, r12d
    mov edx, ebx
    call lp_dtrsm_left
    lea rcx, [lpAccT]
    call lp_acc_add
    call lp_acc_from_q
    mov ecx, r12d
    mov edx, ebx
    call lp_dgemm_update
    lea rcx, [lpAccG]
    call lp_acc_add
.next:
    add r12d, ebx
    jmp .jloop
.done:
    call lp_vzero
    add rsp, 20h
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_scale_col:
    test ecx, ecx
    jle .out
    cmp dword [lpIsaUse], 2
    jb .s
    vbroadcastsd ymm15, xmm0
    mov eax, ecx
    and eax, ~7
    jz .a4
    align 32
.a8:
    vmulpd ymm1, ymm15, [rdx]
    vmulpd ymm2, ymm15, [rdx+32]
    vmovupd [rdx], ymm1
    vmovupd [rdx+32], ymm2
    add rdx, 64
    sub eax, 8
    jnz .a8
.a4:
    test ecx, 4
    jz .t
    vmulpd ymm1, ymm15, [rdx]
    vmovupd [rdx], ymm1
    add rdx, 32
.t:
    and ecx, 3
    jz .out
.s:
    vmovsd xmm1, [rdx]
    vmulsd xmm1, xmm1, xmm0
    vmovsd [rdx], xmm1
    add rdx, 8
    sub ecx, 1
    jnz .s
.out:
    ret

align 32
lp_daxpy_neg:
    test ecx, ecx
    jle .out
    cmp dword [lpIsaUse], 2
    jb .s
    vbroadcastsd ymm15, xmm0
    mov eax, ecx
    and eax, ~7
    jz .a4
    align 32
.a8:
    vmovupd ymm1, [rdx]
    vmovupd ymm2, [rdx+32]
    vmovupd ymm3, [r8]
    vmovupd ymm4, [r8+32]
    vfnmadd231pd ymm3, ymm1, ymm15
    vfnmadd231pd ymm4, ymm2, ymm15
    vmovupd [r8], ymm3
    vmovupd [r8+32], ymm4
    add rdx, 64
    add r8, 64
    sub eax, 8
    jnz .a8
.a4:
    test ecx, 4
    jz .t
    vmovupd ymm1, [rdx]
    vmovupd ymm3, [r8]
    vfnmadd231pd ymm3, ymm1, ymm15
    vmovupd [r8], ymm3
    add rdx, 32
    add r8, 32
.t:
    and ecx, 3
    jz .out
.s:
    vmovsd xmm1, [rdx]
    vmulsd xmm1, xmm1, xmm0
    vmovsd xmm2, [r8]
    vsubsd xmm2, xmm2, xmm1
    vmovsd [r8], xmm2
    add rdx, 8
    add r8, 8
    sub ecx, 1
    jnz .s
.out:
    ret

align 32
lp_daxpy4_neg:
    test ecx, ecx
    jle .out
    vbroadcastsd ymm12, xmm0
    vbroadcastsd ymm13, xmm1
    vbroadcastsd ymm14, xmm2
    vbroadcastsd ymm15, xmm3
    mov eax, ecx
    and eax, ~7
    jz .a4
    align 32
.a8:
    vmovupd ymm0, [rdx]
    vmovupd ymm1, [rdx+32]
    vmovupd ymm2, [r8]
    vmovupd ymm3, [r8+32]
    vfnmadd231pd ymm2, ymm0, ymm12
    vfnmadd231pd ymm3, ymm1, ymm12
    vmovupd [r8], ymm2
    vmovupd [r8+32], ymm3
    vmovupd ymm2, [r9]
    vmovupd ymm3, [r9+32]
    vfnmadd231pd ymm2, ymm0, ymm13
    vfnmadd231pd ymm3, ymm1, ymm13
    vmovupd [r9], ymm2
    vmovupd [r9+32], ymm3
    vmovupd ymm2, [r10]
    vmovupd ymm3, [r10+32]
    vfnmadd231pd ymm2, ymm0, ymm14
    vfnmadd231pd ymm3, ymm1, ymm14
    vmovupd [r10], ymm2
    vmovupd [r10+32], ymm3
    vmovupd ymm2, [r11]
    vmovupd ymm3, [r11+32]
    vfnmadd231pd ymm2, ymm0, ymm15
    vfnmadd231pd ymm3, ymm1, ymm15
    vmovupd [r11], ymm2
    vmovupd [r11+32], ymm3
    add rdx, 64
    add r8, 64
    add r9, 64
    add r10, 64
    add r11, 64
    sub eax, 8
    jnz .a8
.a4:
    test ecx, 4
    jz .t
    vmovupd ymm0, [rdx]
    vmovupd ymm2, [r8]
    vfnmadd231pd ymm2, ymm0, ymm12
    vmovupd [r8], ymm2
    vmovupd ymm2, [r9]
    vfnmadd231pd ymm2, ymm0, ymm13
    vmovupd [r9], ymm2
    vmovupd ymm2, [r10]
    vfnmadd231pd ymm2, ymm0, ymm14
    vmovupd [r10], ymm2
    vmovupd ymm2, [r11]
    vfnmadd231pd ymm2, ymm0, ymm15
    vmovupd [r11], ymm2
    add rdx, 32
    add r8, 32
    add r9, 32
    add r10, 32
    add r11, 32
.t:
    and ecx, 3
    jz .out
    align 32
.s:
    vmovsd xmm0, [rdx]
    vmovsd xmm1, [r8]
    vfnmadd231sd xmm1, xmm0, xmm12
    vmovsd [r8], xmm1
    vmovsd xmm1, [r9]
    vfnmadd231sd xmm1, xmm0, xmm13
    vmovsd [r9], xmm1
    vmovsd xmm1, [r10]
    vfnmadd231sd xmm1, xmm0, xmm14
    vmovsd [r10], xmm1
    vmovsd xmm1, [r11]
    vfnmadd231sd xmm1, xmm0, xmm15
    vmovsd [r11], xmm1
    add rdx, 8
    add r8, 8
    add r9, 8
    add r10, 8
    add r11, 8
    sub ecx, 1
    jnz .s
.out:
    ret

align 32
lp_panel:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 40h
    mov r12d, ecx
    mov r13d, edx
    mov eax, ecx
    add eax, edx
    mov [lpSwN], eax
    xor r14d, r14d
.j:
    cmp r14d, r13d
    jae .out
    mov ebx, r13d
    sub ebx, r14d
    cmp ebx, PB
    jbe .jb
    mov ebx, PB
.jb:
    mov [rsp+24h], ebx
    mov ecx, r12d
    add ecx, r14d
    mov edx, ebx
    call lp_dgetf2
    mov eax, r13d
    sub eax, r14d
    sub eax, [rsp+24h]
    jle .next
    mov [rsp+20h], eax
    mov ebx, [rsp+24h]
    mov ecx, r12d
    add ecx, r14d
    mov [lpTJ], ecx
    mov [lpTNB], ebx
    add ecx, ebx
    mov [lpT0], ecx
    mov edx, [rsp+20h]
    mov [lpGN], edx
    xor ecx, ecx
    call lp_dtrsm_cols
    mov ebx, [rsp+24h]
    mov edi, [rsp+20h]
    mov eax, r12d
    add eax, r14d
    add eax, ebx
    mov esi, [lpN]
    sub esi, eax
    jle .next
    mov [lpGM], esi
    mov [lpGN], edi
    mov [lpGK], ebx
    movsxd r8, dword [lpLDA]
    mov [lpGLDA], r8
    mov ecx, r12d
    add ecx, r14d
    mov edx, ecx
    add edx, ebx
    movsxd rax, edx
    movsxd r9, ecx
    imul r9, r8
    add rax, r9
    shl rax, 3
    add rax, [lpA]
    mov [lpGA], rax
    movsxd rax, ecx
    movsxd r9, edx
    imul r9, r8
    add rax, r9
    shl rax, 3
    add rax, [lpA]
    mov [lpGB], rax
    movsxd rax, edx
    movsxd r9, edx
    imul r9, r8
    add rax, r9
    shl rax, 3
    add rax, [lpA]
    mov [lpGC], rax
    xor ecx, ecx
    mov edx, [lpGM]
    mov r8, [lpPackA]
    test r8, r8
    jz .np
    call lp_pack_a
.np:
    call lp_dgemm_neg
.next:
    add r14d, [rsp+24h]
    jmp .j
.out:
    mov ecx, r12d
    mov edx, r13d
    call lp_laswp_tail
    mov dword [lpSwN], 0
    call lp_vzero
    add rsp, 40h
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_laswp_tail:
    push rbx
    push rsi
    mov [lpTJ], ecx
    mov [lpTNB], edx
    mov eax, ecx
    add eax, edx
    mov [lpT0], eax
    mov edx, [lpN]
    sub edx, eax
    jle .out
    mov [lpGN], edx
    mov dword [lpJob], 4
    cmp dword [lpNthr], 2
    jb .ser
    cmp edx, 8
    jb .ser
    call lp_par
    jmp .out
.ser:
    xor ecx, ecx
    mov edx, [lpGN]
    call lp_laswp_cols
.out:
    pop rsi
    pop rbx
    ret

align 32
lp_laswp_cols:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov r12d, [lpTJ]
    mov r13d, [lpTNB]
    movsxd r8, dword [lpLDA]
    lea r11, [r8*8]
    mov r10, [lpA]
    mov r9, [lpIpiv]
    mov ebx, [lpT0]
    add ebx, ecx
    mov r15d, [lpT0]
    add r15d, edx
    align 32
.col:
    cmp ebx, r15d
    jae .out
    movsxd rax, ebx
    imul rax, r11
    lea rsi, [r10+rax]
    xor edi, edi
    align 32
.k:
    cmp edi, r13d
    jae .nc
    mov eax, r12d
    add eax, edi
    mov ecx, [r9+rax*4]
    cmp ecx, eax
    je .ns
    vmovsd xmm0, [rsi+rax*8]
    vmovsd xmm1, [rsi+rcx*8]
    vmovsd [rsi+rax*8], xmm1
    vmovsd [rsi+rcx*8], xmm0
.ns:
    inc edi
    jmp .k
.nc:
    inc ebx
    jmp .col
.out:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_dgetf2:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 40h
    mov r12d, ecx
    mov r13d, edx
    mov r15d, [lpN]
    movsxd r14, dword [lpLDA]
    lea r11, [r14*8]
    mov [rsp+20h], r11
    xor ebx, ebx
    align 32
.k:
    cmp ebx, r13d
    jae .out
    mov esi, r12d
    add esi, ebx
    mov ecx, esi
    mov edx, esi
    call lp_pivot
    mov edi, eax
    mov rcx, [lpIpiv]
    movsxd rax, esi
    mov [rcx+rax*4], edi
    cmp edi, esi
    je .nosw
    mov ecx, esi
    mov edx, edi
    call lp_swap_rows
.nosw:
    mov r11, [rsp+20h]
    mov rax, [lpA]
    movsxd rcx, esi
    lea rax, [rax+rcx*8]
    imul rcx, r11
    add rax, rcx
    vmovsd xmm0, [rax]
    vandpd xmm1, xmm0, [lpMaskAbs]
    vucomisd xmm1, [lpTiny]
    jb .nextk
    vmovsd xmm2, [lpC1]
    vdivsd xmm2, xmm2, xmm0
    mov [rsp+30h], rax
    mov ecx, r15d
    sub ecx, esi
    dec ecx
    mov [rsp+28h], ecx
    lea rdx, [rax+8]
    vmovapd xmm0, xmm2
    call lp_scale_col
    mov r10, [rsp+30h]
    lea rsi, [r10+8]
    mov [rsp+38h], rsi
    mov edi, ebx
    inc edi
    align 32
.g4:
    mov eax, r13d
    sub eax, edi
    cmp eax, 4
    jb .g1
    mov r11, [rsp+20h]
    mov rsi, [rsp+38h]
    mov r10, [rsp+30h]
    mov eax, edi
    sub eax, ebx
    imul rax, r11
    lea r8, [rsi+rax]
    vmovsd xmm0, [r10+rax]
    add rax, r11
    lea r9, [rsi+rax]
    vmovsd xmm1, [r10+rax]
    add rax, r11
    lea rdx, [rsi+rax]
    vmovsd xmm2, [r10+rax]
    add rax, r11
    lea r11, [rsi+rax]
    vmovsd xmm3, [r10+rax]
    mov r10, rdx
    mov rdx, rsi
    mov ecx, [rsp+28h]
    call lp_daxpy4_neg
    add edi, 4
    jmp .g4
    align 32
.g1:
    cmp edi, r13d
    jae .nextk
    mov r11, [rsp+20h]
    mov rsi, [rsp+38h]
    mov r10, [rsp+30h]
    mov eax, edi
    sub eax, ebx
    imul rax, r11
    lea r8, [rsi+rax]
    vmovsd xmm0, [r10+rax]
    mov ecx, [rsp+28h]
    mov rdx, rsi
    call lp_daxpy_neg
    inc edi
    jmp .g1
.nextk:
    inc ebx
    jmp .k
.out:
    add rsp, 40h
    call lp_vzero
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_pivot:
    push rbx
    push rsi
    mov ebx, edx
    mov r8d, [lpN]
    movsxd r9, dword [lpLDA]
    mov r10, [lpA]
    movsxd rax, ebx
    imul rax, r9
    add rax, rbx
    lea rsi, [r10+rax*8]
    vxorpd xmm0, xmm0, xmm0
    mov eax, ebx
    mov ecx, ebx
    align 32
.pl:
    cmp ecx, r8d
    jae .d
    vmovsd xmm1, [rsi]
    vandpd xmm2, xmm1, [lpMaskAbs]
    vucomisd xmm2, xmm0
    jbe .n
    vmovapd xmm0, xmm2
    mov eax, ecx
.n:
    add rsi, 8
    inc ecx
    jmp .pl
.d:
    pop rsi
    pop rbx
    ret

lp_swap_rows:
    cmp ecx, edx
    je .r
    push rsi
    push rdi
    push rbx
    mov esi, ecx
    mov edi, edx
    mov ebx, [lpSwN]
    test ebx, ebx
    jnz .c
    mov ebx, [lpN]
.c:
    movsxd r8, dword [lpLDA]
    lea r8, [r8*8]
    mov r9, [lpA]
    movsxd rax, esi
    lea rsi, [r9+rax*8]
    movsxd rax, edi
    lea rdi, [r9+rax*8]
    test ebx, ebx
    jz .x
.s:
    mov rax, [rsi]
    mov rcx, [rdi]
    mov [rsi], rcx
    mov [rdi], rax
    add rsi, r8
    add rdi, r8
    sub ebx, 1
    jnz .s
.x:
    pop rbx
    pop rdi
    pop rsi
.r:
    ret

lp_dtrsm_left:
    push rbx
    push rsi
    mov [lpTJ], ecx
    mov [lpTNB], edx
    mov eax, ecx
    add eax, edx
    mov [lpT0], eax
    mov edx, [lpN]
    sub edx, eax
    jle .out
    mov [lpGN], edx
    mov dword [lpJob], 1
    cmp dword [lpNthr], 2
    jb .ser
    cmp dword [lpGN], 8
    jb .ser
    call lp_par
    jmp .out
.ser:
    xor ecx, ecx
    mov edx, [lpGN]
    call lp_dtrsm_cols
.out:
    call lp_vzero
    pop rsi
    pop rbx
    ret

align 32
lp_dtrsm_cols:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 40h
    mov r12d, [lpTJ]
    mov r13d, [lpTNB]
    movsxd r14, dword [lpLDA]
    lea r11, [r14*8]
    mov [rsp+20h], r11
    mov r10, [lpA]
    mov ebx, [lpT0]
    add ebx, ecx
    mov r15d, [lpT0]
    add r15d, edx
    align 32
.col4:
    mov eax, r15d
    sub eax, ebx
    cmp eax, 4
    jb .col
    mov r11, [rsp+20h]
    mov r10, [lpA]
    movsxd rax, r12d
    lea rsi, [r10+rax*8]
    movsxd rax, ebx
    imul rax, r11
    add rsi, rax
    mov [rsp+28h], rsi
    add rsi, r11
    mov [rsp+30h], rsi
    add rsi, r11
    mov [rsp+38h], rsi
    add rsi, r11
    xor edi, edi
    align 32
.kk4:
    lea eax, [edi+1]
    cmp eax, r13d
    jae .nc4
    mov r11, [rsp+20h]
    mov r10, [lpA]
    mov ecx, r13d
    sub ecx, edi
    dec ecx
    mov eax, r12d
    add eax, edi
    movsxd rdx, eax
    inc eax
    movsxd r8, eax
    lea rax, [r10+r8*8]
    imul rdx, r11
    add rdx, rax
    lea eax, [edi+1]
    shl rax, 3
    mov r8, [rsp+28h]
    vmovsd xmm0, [r8+rdi*8]
    add r8, rax
    mov r9, [rsp+30h]
    vmovsd xmm1, [r9+rdi*8]
    add r9, rax
    mov r10, [rsp+38h]
    vmovsd xmm2, [r10+rdi*8]
    add r10, rax
    mov r11, rsi
    vmovsd xmm3, [r11+rdi*8]
    add r11, rax
    call lp_daxpy4_neg
    inc edi
    jmp .kk4
.nc4:
    add ebx, 4
    jmp .col4
    align 32
.col:
    cmp ebx, r15d
    jae .out
    mov r11, [rsp+20h]
    mov r10, [lpA]
    movsxd rax, r12d
    lea rsi, [r10+rax*8]
    movsxd rax, ebx
    imul rax, r11
    add rsi, rax
    xor edi, edi
    align 32
.kk:
    lea eax, [edi+1]
    cmp eax, r13d
    jae .nc
    movsxd rax, edi
    vmovsd xmm0, [rsi+rax*8]
    mov ecx, r13d
    sub ecx, edi
    dec ecx
    mov eax, r12d
    add eax, edi
    movsxd rdx, eax
    inc eax
    movsxd r8, eax
    mov r10, [lpA]
    lea rax, [r10+r8*8]
    mov r11, [rsp+20h]
    imul rdx, r11
    add rax, rdx
    mov edx, edi
    inc edx
    movsxd r9, edx
    lea r8, [rsi+r9*8]
    mov rdx, rax
    call lp_daxpy_neg
    inc edi
    jmp .kk
.nc:
    inc ebx
    jmp .col
.out:
    add rsp, 40h
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

lp_dgemm_update:
    push rbx
    push r12
    push r13
    mov r12d, ecx
    mov r13d, edx
    mov eax, [lpN]
    sub eax, r12d
    sub eax, r13d
    cmp eax, 1
    jl .out
    mov [lpGM], eax
    mov [lpGN], eax
    mov [lpGK], r13d
    mov r9d, [lpLDA]
    mov r8d, r12d
    add r8d, r13d
    movsxd rax, r8d
    movsxd rdx, r12d
    movsxd r9, r9d
    imul rdx, r9
    add rax, rdx
    shl rax, 3
    add rax, [lpA]
    mov [lpGA], rax
    movsxd rax, r12d
    movsxd rdx, r8d
    imul rdx, r9
    add rax, rdx
    shl rax, 3
    add rax, [lpA]
    mov [lpGB], rax
    movsxd rax, r8d
    movsxd rdx, r8d
    imul rdx, r9
    add rax, rdx
    shl rax, 3
    add rax, [lpA]
    mov [lpGC], rax
    mov [lpGLDA], r9
    call lp_dgemm_neg
.out:
    pop r13
    pop r12
    pop rbx
    ret

lp_dgemm_neg:
    push rbx
    push rsi
    push rdi
    mov eax, [lpGM]
    cmp eax, 1
    jl .done
    mov eax, [lpGN]
    cmp eax, 1
    jl .done
    mov dword [lpJob], 0
    xor ecx, ecx
    mov edx, [lpGM]
    mov r8, [lpPackA]
    test r8, r8
    jz .nopk
    call lp_pack_a
.nopk:
    cmp dword [lpNthr], 2
    jb .ser
    cmp dword [lpGN], 8
    jb .ser
    cmp dword [lpGM], 12
    jb .col
    cmp dword [lpGN], 192
    ja .col
    mov rax, [lpPackB]
    test rax, rax
    jz .col
    call lp_pack_b_all
    mov dword [lpJob], 2
    call lp_par
    jmp .done
.col:
    mov dword [lpJob], 0
    call lp_par
    jmp .done
.ser:
    xor ecx, ecx
    mov edx, [lpGN]
    call lp_dgemm_cols
.done:
    call lp_vzero
    pop rdi
    pop rsi
    pop rbx
    ret

lp_par:
    push rbx
    sub rsp, 30h
    mov dword [lpDone], 0
    mov dword [lpNext], 0
    mov dword [lpHere], 0
    lock inc dword [lpPhase]
    lea rcx, [lpPhase]
    call lp_wake
    call lp_steal
.wait:
    mov eax, [lpDone]
    mov edx, [lpNthr]
    dec edx
    cmp eax, edx
    jae .o
    pause
    jmp .wait
.o:
    add rsp, 30h
    pop rbx
    ret

lp_steal:
    push rbx
    sub rsp, 20h
.lp:
    mov eax, NC
    cmp dword [lpJob], 2
    jne .g
    mov eax, 12
.g:
    lock xadd dword [lpNext], eax
    cmp dword [lpJob], 2
    je .row
    cmp eax, [lpGN]
    jae .out
    mov ecx, eax
    add eax, NC
    mov edx, [lpGN]
    cmp eax, edx
    cmova eax, edx
    mov edx, eax
    cmp ecx, edx
    jae .out
    call lp_job_run
    jmp .lp
.row:
    cmp eax, [lpGM]
    jae .out
    mov ecx, eax
    add eax, 12
    mov edx, [lpGM]
    cmp eax, edx
    cmova eax, edx
    mov edx, eax
    cmp ecx, edx
    jae .out
    call lp_job_run
    jmp .lp
.out:
    add rsp, 20h
    pop rbx
    ret

lp_job_run:
    cmp dword [lpJob], 1
    je lp_dtrsm_cols
    cmp dword [lpJob], 2
    je lp_dgemm_ir
    cmp dword [lpJob], 4
    je lp_laswp_cols
    cmp dword [lpJob], 5
    je lp_fill_cols
    cmp dword [lpJob], 8
    je lp_touch_cols
    jmp lp_dgemm_cols

lp_worker:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    and rsp, -16
    sub rsp, 40h
    call lp_pack_ensure
    xor esi, esi
.loop:
    cmp dword [lpDie], 0
    jne .ret
    mov eax, [lpPhase]
    cmp eax, esi
    je .pause
    mov esi, eax
    cmp dword [lpDie], 0
    jne .ret
    lock inc dword [lpHere]
    call lp_steal
    lock inc dword [lpDone]
    lea rcx, [lpDone]
    call lp_wake
    jmp .loop
.pause:
    pause
    jmp .loop
.ret:
    call lp_pack_drop
    call lp_vzero
    xor eax, eax
    leave
    ret

lp_workers_on:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    and rsp, -16
    sub rsp, 40h
    mov esi, 1
    mov ebx, 1
.cr:
    cmp ebx, [lpNthr]
    jae .o
    xor ecx, ecx
    xor edx, edx
    lea r8, [lp_worker]
    mov r9d, ebx
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], 0
    call CreateThread
    test rax, rax
    jz .fail
    lea r11, [lpWorkers]
    movsxd rdx, ebx
    mov [r11+rdx*8], rax
    mov rcx, rax
    mov edx, THR_HIGHEST
    cmp dword [xtreme], 0
    je .pr
    mov edx, THR_TIMECRIT
.pr:
    call SetThreadPriority
    inc esi
    inc ebx
    jmp .cr
.fail:
    mov [lpNthr], esi
.o:
    leave
    ret

lp_workers_off:
    push rbp
    mov rbp, rsp
    push rbx
    and rsp, -16
    sub rsp, 40h
    mov dword [lpDie], 1
    lock inc dword [lpPhase]
    lea rcx, [lpPhase]
    call lp_wake
    mov ebx, 1
.w:
    cmp ebx, [lpNthr]
    jae .o
    lea r11, [lpWorkers]
    movsxd rdx, ebx
    mov rcx, [r11+rdx*8]
    test rcx, rcx
    jz .n
    mov edx, 5000
    call WaitForSingleObject
    lea r11, [lpWorkers]
    movsxd rdx, ebx
    mov rcx, [r11+rdx*8]
    call CloseHandle
    lea r11, [lpWorkers]
    movsxd rdx, ebx
    mov qword [r11+rdx*8], 0
.n:
    inc ebx
    jmp .w
.o:
    mov dword [lpDie], 0
    leave
    ret

align 32
lp_dgetrs:
    push rbx
    push rsi
    push rdi
    push r12
    mov esi, [lpN]
    mov r8d, [lpLDA]
    mov r9, [lpA]
    mov r10, [lpX]
    mov r11, [lpIpiv]
    xor ebx, ebx
.piv:
    cmp ebx, esi
    jae .fwd
    mov eax, [r11]
    lea r11, [r11+4]
    cmp eax, ebx
    je .pn
    movsxd rdx, ebx
    movsxd rcx, eax
    mov rax, [r10+rdx*8]
    mov rdi, [r10+rcx*8]
    mov [r10+rdx*8], rdi
    mov [r10+rcx*8], rax
.pn:
    inc ebx
    jmp .piv
.fwd:
    xor ebx, ebx
.f1:
    cmp ebx, esi
    jae .bck
    vxorpd xmm0, xmm0, xmm0
    xor ecx, ecx
    align 32
.f2:
    cmp ecx, ebx
    jae .f3
    movsxd rax, ebx
    movsxd rdx, ecx
    imul rdx, r8
    add rax, rdx
    vmovsd xmm1, [r9+rax*8]
    vmulsd xmm1, xmm1, [r10+rcx*8]
    vaddsd xmm0, xmm0, xmm1
    inc ecx
    jmp .f2
.f3:
    vmovsd xmm2, [r10+rbx*8]
    vsubsd xmm2, xmm2, xmm0
    vmovsd [r10+rbx*8], xmm2
    inc ebx
    jmp .f1
.bck:
    mov ebx, esi
    dec ebx
.b1:
    cmp ebx, 0
    jl .out
    vxorpd xmm0, xmm0, xmm0
    mov ecx, ebx
    inc ecx
    align 32
.b2:
    cmp ecx, esi
    jae .b3
    movsxd rax, ebx
    movsxd rdx, ecx
    imul rdx, r8
    add rax, rdx
    vmovsd xmm1, [r9+rax*8]
    vmulsd xmm1, xmm1, [r10+rcx*8]
    vaddsd xmm0, xmm0, xmm1
    inc ecx
    jmp .b2
.b3:
    movsxd rax, ebx
    movsxd rdx, ebx
    imul rdx, r8
    add rax, rdx
    vmovsd xmm2, [r9+rax*8]
    vmovsd xmm3, [r10+rbx*8]
    vsubsd xmm3, xmm3, xmm0
    vdivsd xmm3, xmm3, xmm2
    vmovsd [r10+rbx*8], xmm3
    dec ebx
    jmp .b1
.out:
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

align 32
lp_resid:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    sub rsp, 28h
    mov esi, [lpN]
    mov r8d, [lpLDA]
    mov r10, [lpResidX]
    test r10, r10
    jnz .hx
    mov r10, [lpX]
.hx:
    mov r11, [lpB]
    mov r12, [lpWrk]
    xor ebx, ebx
.z:
    cmp ebx, esi
    jae .p1
    movsxd rax, ebx
    mov qword [r12+rax*8], 0
    inc ebx
    jmp .z
.p1:
    xor r13d, r13d
.j1:
    cmp r13d, esi
    jae .an
    mov r14d, 1664525
    imul r14d, r13d
    add r14d, 1325
    xor ecx, ecx
.i1:
    cmp ecx, esi
    jae .nj1
    imul r14d, 1664525
    add r14d, 1013904223
    mov eax, r14d
    shr eax, 8
    and eax, 00FFFFFFh
    vcvtsi2sd xmm0, xmm0, rax
    vdivsd xmm0, xmm0, [lpTwo24]
    vsubsd xmm0, xmm0, [lpHalf]
    vandpd xmm1, xmm0, [lpMaskAbs]
    movsxd rax, ecx
    vaddsd xmm1, xmm1, [r12+rax*8]
    vmovsd [r12+rax*8], xmm1
    inc ecx
    jmp .i1
.nj1:
    inc r13d
    jmp .j1
.an:
    vxorpd xmm5, xmm5, xmm5
    xor ebx, ebx
.am:
    cmp ebx, esi
    jae .xn
    movsxd rax, ebx
    vmaxsd xmm5, xmm5, [r12+rax*8]
    inc ebx
    jmp .am
.xn:
    vxorpd xmm6, xmm6, xmm6
    xor ebx, ebx
.xx:
    cmp ebx, esi
    jae .bn
    vmovsd xmm1, [r10+rbx*8]
    vandpd xmm2, xmm1, [lpMaskAbs]
    vmaxsd xmm6, xmm6, xmm2
    inc ebx
    jmp .xx
.bn:
    vxorpd xmm7, xmm7, xmm7
    xor ebx, ebx
.bb:
    cmp ebx, esi
    jae .r0
    vmovsd xmm1, [r11+rbx*8]
    vandpd xmm2, xmm1, [lpMaskAbs]
    vmaxsd xmm7, xmm7, xmm2
    inc ebx
    jmp .bb
.r0:
    xor ebx, ebx
.rz:
    cmp ebx, esi
    jae .p2
    movsxd rax, ebx
    vmovsd xmm0, [r11+rax*8]
    vxorpd xmm1, xmm1, xmm1
    vsubsd xmm1, xmm1, xmm0
    vmovsd [r12+rax*8], xmm1
    inc ebx
    jmp .rz
.p2:
    xor r13d, r13d
.j2:
    cmp r13d, esi
    jae .rr
    mov r14d, 1664525
    imul r14d, r13d
    add r14d, 1325
    movsxd rax, r13d
    vmovsd xmm4, [r10+rax*8]
    xor ecx, ecx
.i2:
    cmp ecx, esi
    jae .nj2
    imul r14d, 1664525
    add r14d, 1013904223
    mov eax, r14d
    shr eax, 8
    and eax, 00FFFFFFh
    vcvtsi2sd xmm0, xmm0, rax
    vdivsd xmm0, xmm0, [lpTwo24]
    vsubsd xmm0, xmm0, [lpHalf]
    vmulsd xmm0, xmm0, xmm4
    movsxd rax, ecx
    vaddsd xmm0, xmm0, [r12+rax*8]
    vmovsd [r12+rax*8], xmm0
    inc ecx
    jmp .i2
.nj2:
    inc r13d
    jmp .j2
.rr:
    vxorpd xmm4, xmm4, xmm4
    xor ebx, ebx
.ri:
    cmp ebx, esi
    jae .norm
    movsxd rax, ebx
    vmovsd xmm0, [r12+rax*8]
    vandpd xmm2, xmm0, [lpMaskAbs]
    vmaxsd xmm4, xmm4, xmm2
    inc ebx
    jmp .ri
.norm:
    vmovsd [lpTmpR], xmm4
    vmulsd xmm5, xmm5, xmm6
    vaddsd xmm5, xmm5, xmm7
    vcvtsi2sd xmm0, xmm0, rsi
    vmulsd xmm5, xmm5, xmm0
    vmulsd xmm5, xmm5, [lpEps]
    vmovsd xmm0, xmm4
    vdivsd xmm0, xmm0, xmm5
    vmovsd [lpTmpN], xmm0
    add rsp, 28h
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

lp_emit:
    push rbx
    push rsi
    push rdi
    sub rsp, 28h
    lea rdi, [lineAcc]
%if SELFTEST
    mov edx, [lpN]
    call lp_itoa
    mov byte [rdi], ' '
    inc rdi
    mov edx, [lpLDA]
    call lp_itoa
    mov byte [rdi], ' '
    inc rdi
    mov byte [rdi], '4'
    inc rdi
    mov byte [rdi], ' '
    inc rdi
%endif
    movsd xmm0, [lpTmpT]
    mov ecx, 3
    call lp_ftoa
%if SELFTEST
    mov byte [rdi], ' '
    inc rdi
%else
    mov byte [rdi], 0
    lea rdi, [lineAcc+32]
%endif
    mov eax, [lpN]
    cvtsi2sd xmm1, rax
    movsd xmm2, xmm1
    mulsd xmm1, xmm1
    mulsd xmm1, xmm2
    mulsd xmm1, [lpTwo3]
    divsd xmm1, [lpTmpT]
    divsd xmm1, [lp1e9]
    movsd xmm0, xmm1
    mov ecx, 4
    call lp_ftoa
%if SELFTEST
    mov byte [rdi], ' '
    inc rdi
    movsd xmm0, [lpTmpR]
    call lp_sci
    mov byte [rdi], ' '
    inc rdi
%else
    mov byte [rdi], 0
    lea rdi, [lineAcc+64]
%endif
    movsd xmm0, [lpTmpN]
    call lp_sci
    mov byte [rdi], 0
%if SELFTEST
    cmp dword [lpTest], 0
    jne .skip
%endif
    call post_line
%if SELFTEST
.skip:
%endif
    add rsp, 28h
    pop rdi
    pop rsi
    pop rbx
    xor eax, eax
    ret

lp_itoa:
    push rbx
    push rsi
    mov eax, edx
    mov ebx, 10
    xor ecx, ecx
    test eax, eax
    jnz .d
    mov byte [rdi], '0'
    inc rdi
    pop rsi
    pop rbx
    ret
.d:
    xor edx, edx
    div ebx
    add dl, '0'
    push rdx
    inc ecx
    test eax, eax
    jnz .d
.p:
    pop rax
    mov [rdi], al
    inc rdi
    dec ecx
    jnz .p
    pop rsi
    pop rbx
    ret

lp_ftoa:
    push rbx
    push rsi
    movq rax, xmm0
    test rax, rax
    jns .pos
    mov byte [rdi], '-'
    inc rdi
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movsd xmm0, xmm1
.pos:
    mov ebx, ecx
    movsd xmm2, [lpC1]
    xor ecx, ecx
.sc:
    cmp ecx, ebx
    jae .scd
    mulsd xmm2, [lpC10]
    inc ecx
    jmp .sc
.scd:
    mulsd xmm0, xmm2
    addsd xmm0, [lpHalf]
    cvttsd2si rax, xmm0
    mov r8d, ebx
    mov r9, rax
    mov r10d, 1
    xor ecx, ecx
.pw:
    cmp ecx, r8d
    jae .pwd
    imul r10d, 10
    inc ecx
    jmp .pw
.pwd:
    mov rax, r9
    xor rdx, rdx
    movsxd r10, r10d
    div r10
    mov edx, eax
    push rdx
    mov rax, r9
    xor rdx, rdx
    div r10
    push rdx
    pop rsi
    pop rdx
    call lp_itoa
    mov byte [rdi], '.'
    inc rdi
    mov eax, esi
    mov ebx, r8d
    test ebx, ebx
    jz .o
    push rax
    mov ecx, ebx
    dec ecx
    mov r10d, 1
    test ecx, ecx
    jz .no
.ld:
    imul r10d, 10
    dec ecx
    jnz .ld
.no:
    pop rax
.dg:
    xor edx, edx
    div r10d
    add al, '0'
    mov [rdi], al
    inc rdi
    mov eax, edx
    mov edx, r10d
    mov r10d, 10
    cmp edx, 10
    jbe .last
    push rax
    mov eax, edx
    xor edx, edx
    div r10d
    mov r10d, eax
    pop rax
    jmp .dg
.last:
    add al, '0'
    mov [rdi], al
    inc rdi
.o:
    pop rsi
    pop rbx
    ret

lp_sci:
    push rbx
    push rsi
    movq rax, xmm0
    test rax, rax
    jns .p
    mov byte [rdi], '-'
    inc rdi
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movsd xmm0, xmm1
.p:
    xor ebx, ebx
    ucomisd xmm0, [lpC0]
    je .z
.up:
    comisd xmm0, [lpC10]
    jb .dn
    divsd xmm0, [lpC10]
    inc ebx
    jmp .up
.dn:
    comisd xmm0, [lpC1]
    jae .d
    mulsd xmm0, [lpC10]
    dec ebx
    jmp .dn
.d:
    mov ecx, 6
    call lp_ftoa
    mov byte [rdi], 'e'
    inc rdi
    test ebx, ebx
    jns .ep
    mov byte [rdi], '-'
    inc rdi
    neg ebx
    jmp .ez
.ep:
    mov byte [rdi], '+'
    inc rdi
.ez:
    cmp ebx, 10
    jae .e2
    mov byte [rdi], '0'
    inc rdi
.e2:
    mov edx, ebx
    call lp_itoa
    pop rsi
    pop rbx
    ret
.z:
    mov ecx, 8
.zl:
    mov byte [rdi], '0'
    inc rdi
    dec ecx
    jnz .zl
    mov byte [rdi], 'e'
    inc rdi
    mov byte [rdi], '+'
    inc rdi
    mov byte [rdi], '0'
    inc rdi
    mov byte [rdi], '0'
    inc rdi
    pop rsi
    pop rbx
    ret
