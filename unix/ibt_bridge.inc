u32toa:
    push rbx
    push rcx
    push rdx
    mov ebx, 10
    xor ecx, ecx
    test eax, eax
    jnz .digits
    mov byte [rdi], '0'
    inc rdi
    jmp .term
.digits:
    xor edx, edx
    div ebx
    push rdx
    inc ecx
    test eax, eax
    jnz .digits
.emit:
    pop rax
    add al, '0'
    stosb
    loop .emit
.term:
    mov byte [rdi], 0
    pop rdx
    pop rcx
    pop rbx
    ret

streq:
    xor eax, eax
.next:
    mov dl, [rdi]
    cmp dl, [rsi]
    jne .no
    test dl, dl
    jz .yes
    inc rdi
    inc rsi
    jmp .next
.yes:
    mov eax, 1
.no:
    ret

; ---- UI/core bridge, using the original Win64 internal calling convention --

thread_count:
    mov eax, [threadChoice]
    lea rdx, [threadValues]
    mov eax, [rdx+rax*4]
    test eax, eax
    jz .all
    cmp eax, -1
    je .auto
    cmp eax, [nproc]
    jbe .out
.all:
    mov eax, [nproc]
    ret
.auto:
    mov eax, [lpPcores]
.out:
    ret

title_status:
title_progress:
SetWindowTextW:
SendMessageW:
SetThreadPriority:
WakeByAddressAll:
    mov eax, 1
    ret

PostMessageW:
    cmp edx, WM_DONE
    jne .out
    mov dword [uiDone], 1
    mov dword [uiDirty], 1
.out:
    mov eax, 1
    ret

GetLogicalProcessorInformationEx:
    xor eax, eax
    ret

QueryPerformanceFrequency:
    mov qword [rcx], 1000000000
    mov eax, 1
    ret

QueryPerformanceCounter:
    push rdi
    push rsi
    sub rsp, 24
    mov [rsp+16], rcx
    mov edi, 1                    ; CLOCK_MONOTONIC
    mov rsi, rsp
    call clock_gettime
    mov rdx, [rsp]
    imul rdx, 1000000000
    add rdx, [rsp+8]
    mov rcx, [rsp+16]
    mov [rcx], rdx
    add rsp, 24
    pop rsi
    pop rdi
    mov eax, 1
    ret

VirtualAlloc:
    push rbx
    push rdi
    push rsi
    sub rsp, 32
    mov rbx, rdx
    lea rdi, [rsp+24]
    mov esi, 2097152               ; align large matrices for 2 MiB THPs
    mov rdx, rbx
    call posix_memalign
    test eax, eax
    jnz .fail
    mov rdi, [rsp+24]
    mov rsi, rbx
    mov edx, 14                    ; MADV_HUGEPAGE
    mov eax, 28                    ; madvise
    syscall
    mov rax, [rsp+24]
    jmp .out
.fail:
    xor eax, eax
.out:
    add rsp, 32
    pop rsi
    pop rdi
    pop rbx
    ret

VirtualFree:
    push rdi
    push rsi
    sub rsp, 8
    mov rdi, rcx
    call free
    add rsp, 8
    pop rsi
    pop rdi
    mov eax, 1
    ret

TlsAlloc:
    push rdi
    push rsi
    sub rsp, 8
    lea rdi, [tlsKey]
    xor esi, esi
    call pthread_key_create
    test eax, eax
    jnz .fail
    mov eax, [tlsKey]
    jmp .out
.fail:
    mov eax, -1
.out:
    add rsp, 8
    pop rsi
    pop rdi
    ret

TlsGetValue:
    push rdi
    push rsi
    sub rsp, 8
    mov edi, [tlsKey]
    call pthread_getspecific
    add rsp, 8
    pop rsi
    pop rdi
    ret

TlsSetValue:
    push rdi
    push rsi
    sub rsp, 8
    mov edi, [tlsKey]
    mov rsi, rdx
    call pthread_setspecific
    xor eax, 1
    add rsp, 8
    pop rsi
    pop rdi
    ret

CreateThread:
    push rbx
    push rdi
    push rsi
    sub rsp, 32
    mov [rsp], r8
    mov [rsp+8], r9
    mov edi, 8
    call malloc
    test rax, rax
    jz .fail
    mov rbx, rax
    mov rdi, rax
    xor esi, esi
    mov rdx, [rsp]
    mov rcx, [rsp+8]
    call pthread_create
    test eax, eax
    jnz .free
    mov rax, rbx
    jmp .out
.free:
    mov rdi, rbx
    call free
.fail:
    xor eax, eax
.out:
    add rsp, 32
    pop rsi
    pop rdi
    pop rbx
    ret

WaitForSingleObject:
    push rdi
    push rsi
    sub rsp, 8
    mov rdi, [rcx]
    xor esi, esi
    call pthread_join
    add rsp, 8
    pop rsi
    pop rdi
    ret

CloseHandle:
    push rdi
    push rsi
    sub rsp, 8
    mov rdi, rcx
    call free
    add rsp, 8
    pop rsi
    pop rdi
    ret

isqrt:
    push rbx
    mov r8, rcx
    cmp r8, 2
    jb .small
    mov rax, r8
.loop:
    mov rbx, rax
    mov rax, r8
    xor rdx, rdx
    div rbx
    add rax, rbx
    shr rax, 1
    cmp rax, rbx
    jb .loop
    mov rax, rbx
    pop rbx
    ret
.small:
    mov rax, r8
    pop rbx
    ret

post_line:
    push rbx
    push rsi
    push rdi
    lea rsi, [lineAcc]
    lea rdi, [resultTime]
    call copyz
    lea rsi, [lineAcc+32]
    lea rdi, [resultSpeed]
    call copyz
    lea rsi, [lineAcc+64]
    lea rdi, [resultResid]
    call copyz
    inc dword [resultIndex]
    cmp dword [haveResid], 0
    jne .compare
    lea rsi, [lineAcc+64]
    lea rdi, [firstResid]
    call copyz
    mov dword [haveResid], 1
    jmp .pass
.compare:
    lea rsi, [lineAcc+64]
    lea rdi, [firstResid]
.cmp:
    mov al, [rsi]
    cmp al, [rdi]
    jne .bad
    test al, al
    jz .pass
    inc rsi
    inc rdi
    jmp .cmp
.bad:
    mov dword [failed], 1
    mov dword [lpStop], 1
    mov dword [resultState], RESULT_FAIL
    jmp .logged
.pass:
    inc dword [passCount]
.logged:
    call log_line
    mov dword [uiDirty], 1
    pop rdi
    pop rsi
    pop rbx
    ret

copyz:
    lodsb
    stosb
    test al, al
    jnz copyz
    ret

; Logging uses raw Linux syscalls so it adds no runtime dependency.
log_open:
    cmp dword [logEnabled], 0
    je .out
    mov eax, 257                  ; openat
    mov edi, -100                ; AT_FDCWD
    lea rsi, [logName]
    mov edx, 577                 ; O_WRONLY|O_CREAT|O_TRUNC
    mov r10d, 420                ; 0644
    syscall
    test eax, eax
    js .out
    mov [logFd], eax
    mov edi, eax
    lea rsi, [logHeader]
    mov edx, logHeaderLen
    mov eax, 1
    syscall
.out:
    ret

log_line:
    mov edi, [logFd]
    cmp edi, 0
    jl .out
    lea rsi, [lineAcc]
    call log_string
    lea rsi, [szSpaces]
    call log_string
    lea rsi, [lineAcc+32]
    call log_string
    lea rsi, [szSpaces]
    call log_string
    lea rsi, [lineAcc+64]
    call log_string
    lea rsi, [szNL]
    call log_string
.out:
    ret

log_string:
    push rsi
    xor edx, edx
.len:
    cmp byte [rsi+rdx], 0
    je .write
    inc edx
    jmp .len
.write:
    mov eax, 1
    syscall
    pop rsi
    ret

log_close:
    mov edi, [logFd]
    cmp edi, 0
    jl .out
    mov eax, 3
    syscall
    mov dword [logFd], -1
.out:
    ret

szSpaces db '    ',0
szNL db 10,0

; The same numerical source used by IntelBurnTest.exe.
