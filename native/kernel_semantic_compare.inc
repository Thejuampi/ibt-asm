; Semantic comparison for two x86-64 instruction ranges.
;
; Requires kdec_decode from native/kernel_decode.inc.  It must write the
; documented 64-byte descriptor and return a nonzero instruction length.  The
; comparison fails closed on unsupported or truncated instructions.
;
; kcmp_compare_ranges:
;   RSI, RDI = candidate start and exclusive end
;   RDX, RCX = reference start and exclusive end
;   R8       = target resolver callback
;   R9       = callback context
;   EAX      = 1 equivalent, 0 different or unverifiable
;
; Resolver callback (custom ABI, preserves RBX/RBP/RSI/RDI/R12-R15):
;   RAX = raw target address; EDX = 0 candidate, 1 reference; RCX = context;
;   R8D = descriptor attributes, including RIP and direct-branch flags.
;   Return RAX = stable identity and clear carry, or set carry if unresolved.
;   Targets in other verified ranges and targets of RIP-relative operands must
;   be resolved by the caller; silently dropping a displacement is forbidden.
;
; kcmp_instruction_index:
;   RSI, RDI = start and exclusive end; RAX = raw target address.
;   RAX = zero-based non-NOP instruction index, or -1 if target is invalid.
;   A target at a NOP resolves to the next non-NOP, as in the old checker.
;
; Both routines preserve RBX/RBP/RSI/RDI/R12-R15.  Stack is 16-byte aligned
; before calls.  No stack red zone or platform runtime is used.

%define KCMP_RIP_TARGET    0x10000
%define KCMP_DIRECT_BRANCH 0x20000

kcmp_compare_ranges:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 0xf8
    mov r12, rsi
    mov r13, rdi
    mov r14, rdx
    mov r15, rcx
    mov rbx, r8
    mov [rbp-64], rsi             ; candidate start
    mov [rbp-72], rdx             ; reference start
    mov [rbp-80], r9              ; resolver context
    mov qword [rbp-232], 0        ; decoded non-NOP instructions
    test rbx, rbx
    jz .different
    cmp r12, r13
    jae .different
    cmp r14, r15
    jae .different

.next_candidate:
    cmp r12, r13
    je .candidate_end
    ja .different
    mov rsi, r12
    mov rdi, r13
    lea rdx, [rbp-160]            ; candidate descriptor
    call kdec_decode
    test eax, eax
    jz .different
    cmp eax, 15
    ja .different
    mov eax, eax
    add r12, rax
    jc .different
    cmp r12, r13
    ja .different
    cmp dword [rbp-156], 0        ; semantic ID: 0 is NOP
    je .next_candidate
    inc qword [rbp-232]
    jz .different

.next_reference:
    cmp r14, r15
    jae .different
    mov rsi, r14
    mov rdi, r15
    lea rdx, [rbp-224]            ; reference descriptor
    call kdec_decode
    test eax, eax
    jz .different
    cmp eax, 15
    ja .different
    mov eax, eax
    add r14, rax
    jc .different
    cmp r14, r15
    ja .different
    cmp dword [rbp-220], 0
    je .next_reference

    ; Length and absolute target are not representation-independent.  Compare
    ; the operation, attributes, typed operands, base/index and scale.
    mov eax, [rbp-156]
    cmp eax, [rbp-220]
    jne .different
    mov eax, [rbp-152]
    cmp eax, [rbp-216]
    jne .different
    mov [rbp-88], eax
    mov rax, [rbp-148]
    cmp rax, [rbp-212]
    jne .different
    mov rax, [rbp-140]
    cmp rax, [rbp-204]
    jne .different
    mov rax, [rbp-132]
    cmp rax, [rbp-196]
    jne .different
    mov eax, [rbp-124]
    cmp eax, [rbp-188]
    jne .different

    ; RIP displacement and direct branch immediate represent target addresses;
    ; ordinary displacements and immediates must remain exact.
    test dword [rbp-88], KCMP_RIP_TARGET
    jnz .skip_displacement
    mov rax, [rbp-120]
    cmp rax, [rbp-184]
    jne .different
.skip_displacement:
    test dword [rbp-88], KCMP_DIRECT_BRANCH
    jnz .skip_immediate
    mov rax, [rbp-112]
    cmp rax, [rbp-176]
    jne .different
.skip_immediate:
    test dword [rbp-88], KCMP_RIP_TARGET | KCMP_DIRECT_BRANCH
    jz .next_candidate

    ; A direct branch within the corresponding range is identified by the
    ; non-NOP instruction index.  This includes short/near encodings and
    ; branches landing on alignment NOPs.  Cross-range targets use the resolver.
    test dword [rbp-88], KCMP_DIRECT_BRANCH
    jz .external_target
    mov rax, [rbp-104]
    cmp rax, [rbp-64]
    jb .candidate_external
    cmp rax, r13
    jae .candidate_external
    mov rdx, [rbp-168]
    cmp rdx, [rbp-72]
    jb .different
    cmp rdx, r15
    jae .different
    mov rsi, [rbp-64]
    mov rdi, r13
    call kcmp_instruction_index
    cmp rax, -1
    je .different
    mov [rbp-96], rax
    mov rax, [rbp-168]
    mov rsi, [rbp-72]
    mov rdi, r15
    call kcmp_instruction_index
    cmp rax, -1
    je .different
    cmp rax, [rbp-96]
    jne .different
    jmp .next_candidate
.candidate_external:
    mov rax, [rbp-168]
    cmp rax, [rbp-72]
    jb .external_target
    cmp rax, r15
    jb .different

.external_target:
    mov rax, [rbp-104]
    xor edx, edx
    mov rcx, [rbp-80]
    mov r8d, [rbp-88]
    call rbx
    jc .different
    mov [rbp-96], rax
    mov rax, [rbp-168]
    mov edx, 1
    mov rcx, [rbp-80]
    mov r8d, [rbp-88]
    call rbx
    jc .different
    cmp rax, [rbp-96]
    jne .different
    jmp .next_candidate

.candidate_end:
    ; Permit only trailing NOP instructions on the reference side.
    cmp r14, r15
    je .equivalent
    ja .different
    mov rsi, r14
    mov rdi, r15
    lea rdx, [rbp-224]
    call kdec_decode
    test eax, eax
    jz .different
    cmp eax, 15
    ja .different
    mov eax, eax
    add r14, rax
    jc .different
    cmp r14, r15
    ja .different
    cmp dword [rbp-220], 0
    jne .different
    jmp .candidate_end
.equivalent:
    cmp qword [rbp-232], 0
    je .different
    mov eax, 1
    jmp .return
.different:
    xor eax, eax
.return:
    add rsp, 0xf8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret

kcmp_instruction_index:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 0x78
    mov r12, rsi
    mov r13, rdi
    mov r14, rax
    xor r15d, r15d
    cmp r12, r13
    jae .invalid
    cmp r14, r12
    jb .invalid
    cmp r14, r13
    jae .invalid
.scan:
    cmp r12, r13
    jae .invalid
    mov rsi, r12
    mov rdi, r13
    lea rdx, [rbp-128]
    call kdec_decode
    test eax, eax
    jz .invalid
    cmp eax, 15
    ja .invalid
    mov eax, eax
    lea rbx, [r12+rax]
    cmp rbx, r12
    jb .invalid
    cmp rbx, r13
    ja .invalid
    cmp r14, r12
    je .at_target
    jb .invalid
    cmp r14, rbx
    jb .invalid                   ; target is inside an instruction
    cmp dword [rbp-124], 0
    je .advance
    inc r15
    jz .invalid
.advance:
    mov r12, rbx
    jmp .scan
.at_target:
    cmp dword [rbp-124], 0
    jne .found
    mov r12, rbx                  ; target NOP: find the next real instruction
.after_target_nop:
    cmp r12, r13
    jae .invalid
    mov rsi, r12
    mov rdi, r13
    lea rdx, [rbp-128]
    call kdec_decode
    test eax, eax
    jz .invalid
    cmp eax, 15
    ja .invalid
    mov eax, eax
    add r12, rax
    jc .invalid
    cmp r12, r13
    ja .invalid
    cmp dword [rbp-124], 0
    je .after_target_nop
.found:
    mov rax, r15
    jmp .finish
.invalid:
    mov rax, -1
.finish:
    add rsp, 0x78
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
