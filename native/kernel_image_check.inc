%ifdef KERNEL_TEST_ELF
KERNEL_HEADER_SIZE equ 0x300
KERNEL_IMAGE_BIAS  equ 0
KERNEL_CODE_START  equ 0x300
%else
KERNEL_HEADER_SIZE equ 0x400
KERNEL_IMAGE_BIAS  equ 0xc00
KERNEL_CODE_START  equ 0x1000
%endif
KERNEL_FILE_LIMIT equ 0x1000000

; Read code bytes as data. The synthetic PE base translates RIP targets
; to RVAs even if their destination has no file bytes, such as BSS.
verify_candidate:
    sub rsp, 0x80
    call map_kernel_image
    test rax, rax
    jz .invalid
    lea rcx, [rax-KERNEL_IMAGE_BIAS]
    mov [rsp+0x20], rcx
%ifdef IBT_COMPACT_PIPELINE
    mov ecx, 'KRN2'
%else
    mov ecx, 'KRN1'
%endif
    call kernel_image_symbols
    test rax, rax
    jz .invalid
    mov [rsp+0x38], rax
    mov [rsp+0x58], rdx
    lea rdi, [kernel_reference_path]
    call map_kernel_image
    test rax, rax
    jz .invalid
    mov [rsp+0x28], rax
    mov ecx, 'KRN1'
    call kernel_image_symbols
    test rax, rax
    jz .invalid
    mov qword [rsp+0x30], KERNEL_IMAGE_BIAS
    mov [rsp+0x40], rax
    mov qword [rsp+0x48], kernel_symbol_count
    mov [rsp+0x60], rdx
%define KERNEL_CONTEXT rsp+0x20
%define KERNEL_INVALID .invalid
%define KERNEL_DIFFERENT .different
%include 'native/kernel_verify_ranges.inc'
%undef KERNEL_CONTEXT
%undef KERNEL_INVALID
%undef KERNEL_DIFFERENT
    xor ecx, ecx
    jmp test_exit
.different:
    mov ecx, 1
    jmp test_exit
.invalid:
    mov ecx, 2
    jmp test_exit

; Accept only the native image layout used by the application and references.
; RAX = mapped file, RDX = file size, ECX = metadata signature.
; Returns symbols and exclusive code RVA.
kernel_image_symbols:
    cmp rdx, KERNEL_HEADER_SIZE
    jb .invalid
    mov r8, rax
    mov r10d, ecx
%ifdef KERNEL_TEST_ELF
    cmp dword [r8], 0x464c457f
    jne .invalid
    cmp dword [r8+4], 0x00010102
    jne .invalid
    cmp dword [r8+16], 0x003e0002
    jne .invalid
    cmp qword [r8+32], 64
    jne .invalid
    cmp word [r8+54], 56
    jne .invalid
    cmp word [r8+56], 2
    jb .invalid
    lea rsi, [r8+64+56]
    cmp dword [rsi], 1
    jne .invalid
    cmp dword [rsi+4], 5
    jne .invalid
    cmp qword [rsi+8], 0
    jne .invalid
    cmp qword [rsi+16], 0x400000
    jne .invalid
    mov r11, [rsi+32]
    cmp r11, rdx
    ja .invalid
    cmp r11, [rsi+40]
    ja .invalid
%else
    cmp word [r8], 'MZ'
    jne .invalid
    mov eax, [r8+0x3c]
    cmp eax, 0x40
    jb .invalid
    cmp eax, 0x300 - (24+0xf0+40)
    ja .invalid
    lea rsi, [r8+rax]
    cmp dword [rsi], 0x00004550
    jne .invalid
    cmp word [rsi+4], 0x8664
    jne .invalid
    cmp word [rsi+6], 1
    jb .invalid
    cmp word [rsi+20], 0xf0
    jne .invalid
    cmp word [rsi+24], 0x20b
    jne .invalid
    add rsi, 24+0xf0
    mov rax, '.text'
    cmp [rsi], rax
    jne .invalid
    cmp dword [rsi+12], KERNEL_CODE_START
    jne .invalid
    cmp dword [rsi+20], KERNEL_HEADER_SIZE
    jne .invalid
    cmp dword [rsi+36], 0x60000020
    jne .invalid
    mov ecx, [rsi+16]
    lea rax, [rcx+KERNEL_HEADER_SIZE]
    cmp rax, rdx
    ja .invalid
    mov eax, [rsi+8]
    cmp eax, ecx
    cmova eax, ecx
    lea r11, [rax+KERNEL_HEADER_SIZE]
%endif
    cmp dword [r8+KERNEL_HEADER_SIZE-0x100], r10d
    jne .invalid
    cmp dword [r8+KERNEL_HEADER_SIZE-0xfc], 4
    jne .invalid
    mov edx, [r8+KERNEL_HEADER_SIZE-4]
    cmp edx, KERNEL_HEADER_SIZE
    jb .invalid
    lea rax, [rdx+8+4*kernel_symbol_count]
    cmp rax, r11
    ja .invalid
    lea rax, [r8+rdx]
    cmp dword [rax], 'KSY1'
    jne .invalid
    cmp dword [rax+4], kernel_symbol_count
    jne .invalid
    add rax, 8
    add rdx, KERNEL_IMAGE_BIAS
    ret
.invalid:
    xor eax, eax
    ret

; RDI = path, RAX = file mapping (zero on error), RDX = file size.
; Mapping lifetimes are bounded by the one-shot checker process.
map_kernel_image:
%ifdef KERNEL_TEST_ELF
    push rbx
    push r12
    push r13
    mov eax, 2
    xor esi, esi
    xor edx, edx
    syscall
    test rax, rax
    js .error
    mov rbx, rax
    xor r13d, r13d
    mov eax, 8
    mov rdi, rbx
    xor esi, esi
    mov edx, 2
    syscall
    test rax, rax
    jle .close
    cmp rax, KERNEL_FILE_LIMIT
    ja .close
    mov r12, rax
    mov eax, 9
    xor edi, edi
    mov rsi, r12
    mov edx, 1
    mov r10d, 2
    mov r8, rbx
    xor r9d, r9d
    syscall
    cmp rax, -4095
    jae .close
    mov r13, rax
.close:
    mov eax, 3
    mov rdi, rbx
    syscall
    mov rax, r13
    mov rdx, r12
    jmp .out
.error:
    xor eax, eax
.out:
    pop r13
    pop r12
    pop rbx
    ret
kernel_reference_path: db '../reference/numerical/linux.bin', 0
%else
    push rbx
    push rsi
    push rdi
    sub rsp, 0x40
    mov rcx, rdi
    mov edx, 0x80000000            ; GENERIC_READ
    mov r8d, 1                    ; FILE_SHARE_READ
    xor r9d, r9d
    mov qword [rsp+0x20], 3        ; OPEN_EXISTING
    mov qword [rsp+0x28], 0x80     ; FILE_ATTRIBUTE_NORMAL
    mov qword [rsp+0x30], 0
    call [iat_CreateFileW]
    cmp rax, -1
    je .error
    mov rbx, rax
    xor esi, esi
    mov rcx, rbx
    lea rdx, [rsp+0x38]
    call [iat_GetFileSizeEx]
    test eax, eax
    jz .close
    cmp qword [rsp+0x38], 0
    je .close
    cmp qword [rsp+0x38], KERNEL_FILE_LIMIT
    ja .close
    mov rcx, rbx
    xor edx, edx
    mov r8d, 2                    ; PAGE_READONLY
    xor r9d, r9d
    mov qword [rsp+0x20], 0
    mov qword [rsp+0x28], 0
    call [iat_CreateFileMappingW]
    test rax, rax
    jz .close
    mov rdi, rax
    mov rcx, rdi
    mov edx, 4                    ; FILE_MAP_READ
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+0x20], 0
    call [iat_MapViewOfFile]
    mov rsi, rax
    mov rcx, rdi
    call [iat_CloseHandle]
.close:
    mov rcx, rbx
    call [iat_CloseHandle]
    mov rax, rsi
    mov rdx, [rsp+0x38]
    jmp .out
.error:
    xor eax, eax
.out:
    add rsp, 0x40
    pop rdi
    pop rsi
    pop rbx
    ret
kernel_reference_path: dw __utf16le__('reference/numerical/windows.bin'), 0
%endif
