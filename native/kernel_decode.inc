; Closed x86-64 instruction decoder for numerical-kernel verification.
;
; kdec_decode: RSI = first byte, RDI = end (exclusive), RDX = 64-byte output.
; EAX returns instruction length, or zero for a truncated/unsupported form.
; RBX, RBP, RSI, RDI and R12-R15 are preserved. Other registers/flags are clobbered.
;
; Descriptor (little endian, zero initialized):
;   +0  dword length             +4  dword instruction ID (zero = NOP)
;   +8  dword attributes         +12,+16,+20,+24 four operand descriptors
;   +28 dword memory base        +32 dword memory index  +36 dword scale
;   +40 qword signed displacement, +48 qword immediate, +56 qword target
; Attributes: low byte = GPR operand width (8,16,32,64); bit 8 = LOCK;
; bit 9 = VEX; bit 10 = 256-bit VEX; bit 11 = meaningful VEX.W;
; bits 12-13 = mandatory prefix
; (0 none, 1 66, 2 F3, 3 F2); bit 14 = address-size 32; bit 16 = RIP target;
; bit 17 = direct branch target. A caller must resolve targets, never mask them.
; Operand: 0 absent; 0x100|n = GPR n; 0x200|n = vector register n;
; 0x300 = memory; 0x400 = immediate; 0x500 = direct branch target.
; GPR byte-registers AH/CH/DH/BH use IDs 16..19, distinct from SPL/BPL/SIL/DIL.
; Memory base/index 0..15 are registers, 0xff means none; scale is 1,2,4,8.
; RIP-relative operands have no base/index, displacement zero and bit 16 set.
; Unknown encodings fail closed. Supported classes are explicitly listed below.

%define KD_LEN       0
%define KD_OP        4
%define KD_ATTR      8
%define KD_ARG0      12
%define KD_ARG1      16
%define KD_ARG2      20
%define KD_ARG3      24
%define KD_BASE      28
%define KD_INDEX     32
%define KD_SCALE     36
%define KD_DISP      40
%define KD_IMM       48
%define KD_TARGET    56
%define KD_RIP       (1 << 16)
%define KD_BRANCH    (1 << 17)

; Local stack frame: a padded copy guarantees bounded reads even on truncation.
%define KL_BUF       0
%define KL_AVAIL     32
%define KL_WIDTH     36
%define KL_PP        40
%define KL_LOCK      44
%define KL_ADDR32    48
%define KL_REX       52
%define KL_R         56
%define KL_X         60
%define KL_B         64
%define KL_W         68
%define KL_MAP       72
%define KL_VEX       76
%define KL_VVVV      80
%define KL_L         84
%define KL_OPCODE    88
%define KL_MODRM     92
%define KL_MOD       96
%define KL_GROUP     100
%define KL_REG       104
%define KL_RM        108
%define KL_IMMSZ     112
%define KL_IMMMODE   116
%define KL_FORM      120
%define KL_RIPDISP   124
%define KL_REGTYPE   128
%define KL_RMTYPE    132
%define KL_LOCKOK    136
%define KL_FRAME     160

; Operand form IDs for the common ModRM path.
%define KF_RM_REG    1
%define KF_REG_RM    2
%define KF_RM        3
%define KF_RM_IMM    4
%define KF_REG_RM_IMM 5
%define KF_VEC3      6
%define KF_VEC2_REG_RM 7
%define KF_VEC2_RM_REG 8
%define KF_VEC4      9

kdec_decode:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    sub rsp, KL_FRAME
    mov r12, rsi                    ; original instruction address
    mov r13, rdi                    ; original end
    mov r14, rdx                    ; descriptor
    cmp r13, r12
    jbe .fail
    xor eax, eax
    mov rdi, r14
    mov ecx, 8
    cld
    rep stosq
    mov dword [r14+KD_BASE], 0xff
    mov dword [r14+KD_INDEX], 0xff
    mov dword [r14+KD_SCALE], 1
    lea rdi, [rsp+KL_BUF]
    mov ecx, 4
    rep stosq
    mov rax, r13
    sub rax, r12
    cmp rax, 15
    jbe .copy_count
    mov eax, 15
.copy_count:
    mov [rsp+KL_AVAIL], eax
    mov ecx, eax
    mov rsi, r12
    lea rdi, [rsp+KL_BUF]
    rep movsb
    lea r15, [rsp+KL_BUF]
    mov dword [rsp+KL_WIDTH], 32
    mov dword [rsp+KL_PP], 0
    mov dword [rsp+KL_LOCK], 0
    mov dword [rsp+KL_ADDR32], 0
    mov dword [rsp+KL_REX], 0
    mov dword [rsp+KL_R], 0
    mov dword [rsp+KL_X], 0
    mov dword [rsp+KL_B], 0
    mov dword [rsp+KL_W], 0
    mov dword [rsp+KL_MAP], 0
    mov dword [rsp+KL_VEX], 0
    mov dword [rsp+KL_VVVV], 0
    mov dword [rsp+KL_L], 0
    mov dword [rsp+KL_IMMSZ], 0
    mov dword [rsp+KL_IMMMODE], 0
    mov dword [rsp+KL_FORM], 0
    mov dword [rsp+KL_RIPDISP], 0
    mov dword [rsp+KL_REGTYPE], 0
    mov dword [rsp+KL_RMTYPE], 0
    mov dword [rsp+KL_LOCKOK], 0

.prefix:
    movzx eax, byte [r15]
    cmp al, 0x40
    jb .legacy_prefix
    cmp al, 0x4f
    ja .legacy_prefix
    cmp dword [rsp+KL_REX], 0
    jne .fail
    or eax, 0x100
    mov [rsp+KL_REX], eax
    mov ecx, eax
    and ecx, 1
    mov [rsp+KL_B], ecx
    mov ecx, eax
    shr ecx, 1
    and ecx, 1
    mov [rsp+KL_X], ecx
    mov ecx, eax
    shr ecx, 2
    and ecx, 1
    mov [rsp+KL_R], ecx
    shr eax, 3
    and eax, 1
    mov [rsp+KL_W], eax
    inc r15
    jmp .prefix
.legacy_prefix:
    cmp al, 0x66
    je .p66
    cmp al, 0xf3
    je .pf3
    cmp al, 0xf2
    je .pf2
    cmp al, 0xf0
    je .plock
    cmp al, 0x67
    je .paddr
    cmp al, 0x2e                    ; segment/hint prefixes are unsupported
    je .fail
    cmp al, 0x36
    je .fail
    cmp al, 0x3e
    je .fail
    cmp al, 0x26
    je .fail
    cmp al, 0x64
    je .fail
    cmp al, 0x65
    je .fail
    cmp al, 0xc5
    je .vex2
    cmp al, 0xc4
    je .vex3
    jmp .legacy_opcode
.p66:
    mov ecx, 1
    jmp .setpp
.pf3:
    mov ecx, 2
    jmp .setpp
.pf2:
    mov ecx, 3
.setpp:
    cmp dword [rsp+KL_REX], 0       ; REX before legacy has different meaning
    jne .fail
    cmp dword [rsp+KL_PP], 0
    jne .fail
    mov [rsp+KL_PP], ecx
    inc r15
    jmp .prefix
.plock:
    cmp dword [rsp+KL_REX], 0
    jne .fail
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    mov dword [rsp+KL_LOCK], 1
    inc r15
    jmp .prefix
.paddr:
    cmp dword [rsp+KL_REX], 0
    jne .fail
    cmp dword [rsp+KL_ADDR32], 0
    jne .fail
    mov dword [rsp+KL_ADDR32], 1
    inc r15
    jmp .prefix
.vex2:
    cmp dword [rsp+KL_REX], 0
    jne .fail
    cmp dword [rsp+KL_PP], 0
    jne .fail
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    mov dword [rsp+KL_VEX], 1
    mov dword [rsp+KL_MAP], 1
    movzx eax, byte [r15+1]
    mov ecx, eax
    shr ecx, 7
    xor ecx, 1
    mov [rsp+KL_R], ecx
    mov ecx, eax
    shr ecx, 3
    not ecx
    and ecx, 15
    mov [rsp+KL_VVVV], ecx
    mov ecx, eax
    shr ecx, 2
    and ecx, 1
    mov [rsp+KL_L], ecx
    and eax, 3
    mov [rsp+KL_PP], eax
    add r15, 2
    jmp .vex_opcode
.vex3:
    cmp dword [rsp+KL_REX], 0
    jne .fail
    cmp dword [rsp+KL_PP], 0
    jne .fail
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    mov dword [rsp+KL_VEX], 1
    movzx eax, byte [r15+1]
    mov ecx, eax
    and ecx, 0x1f
    cmp ecx, 1
    jb .fail
    cmp ecx, 3
    ja .fail
    mov [rsp+KL_MAP], ecx
    mov ecx, eax
    shr ecx, 7
    xor ecx, 1
    mov [rsp+KL_R], ecx
    mov ecx, eax
    shr ecx, 6
    and ecx, 1
    xor ecx, 1
    mov [rsp+KL_X], ecx
    mov ecx, eax
    shr ecx, 5
    and ecx, 1
    xor ecx, 1
    mov [rsp+KL_B], ecx
    movzx eax, byte [r15+2]
    mov ecx, eax
    shr ecx, 7
    mov [rsp+KL_W], ecx
    mov ecx, eax
    shr ecx, 3
    not ecx
    and ecx, 15
    mov [rsp+KL_VVVV], ecx
    mov ecx, eax
    shr ecx, 2
    and ecx, 1
    mov [rsp+KL_L], ecx
    and eax, 3
    mov [rsp+KL_PP], eax
    add r15, 3
.vex_opcode:
    cmp dword [rsp+KL_W], 0
    je .vex_read_opcode
    mov dword [rsp+KL_WIDTH], 64
.vex_read_opcode:
    movzx eax, byte [r15]
    inc r15
    mov [rsp+KL_OPCODE], eax
    jmp .dispatch_vex

.legacy_opcode:
    cmp dword [rsp+KL_PP], 1
    jne .width_rex
    mov dword [rsp+KL_WIDTH], 16
.width_rex:
    cmp dword [rsp+KL_W], 0
    je .read_opcode
    mov dword [rsp+KL_WIDTH], 64
.read_opcode:
    movzx eax, byte [r15]
    inc r15
    cmp al, 0x0f
    jne .legacy_one
    mov dword [rsp+KL_MAP], 1
    movzx eax, byte [r15]
    inc r15
    mov [rsp+KL_OPCODE], eax
    jmp .dispatch_0f
.legacy_one:
    mov [rsp+KL_OPCODE], eax
    jmp .dispatch_legacy

; Canonical IDs for the supported integer instructions.
%define KO_MOV       0x1001
%define KO_ADD       0x1002
%define KO_SUB       0x1003
%define KO_XOR       0x1004
%define KO_CMP       0x1005
%define KO_TEST      0x1006
%define KO_AND       0x1007
%define KO_OR        0x1008
%define KO_ADC       0x1009
%define KO_SBB       0x100a
%define KO_LEA       0x100b
%define KO_MOVSXD    0x100c
%define KO_IMUL      0x100d
%define KO_CMOVA     0x100e
%define KO_XADD      0x100f
%define KO_PUSH      0x1010
%define KO_POP       0x1011
%define KO_RET       0x1012
%define KO_LEAVE     0x1013
%define KO_PREFETCH  0x1014
%define KO_PAUSE     0x1015
%define KO_SFENCE    0x1016
%define KO_SHIFT     0x1100
%define KO_GROUPF7   0x1200
%define KO_GROUPFF   0x1300
%define KO_JCC       0x2000
%define KO_CALL      0x2020
%define KO_JMP       0x2021

.dispatch_legacy:
    cmp dword [rsp+KL_PP], 2
    jne .not_pause
    cmp eax, 0x90
    jne .fail
    cmp dword [rsp+KL_REX], 0
    jne .fail
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    mov dword [r14+KD_OP], KO_PAUSE
    mov dword [rsp+KL_WIDTH], 0
    jmp .finish
.not_pause:
    cmp dword [rsp+KL_PP], 2       ; REP/REPNZ is not a generic integer prefix
    jae .fail
    cmp eax, 0x90
    je .nop90
    cmp eax, 0x50
    jb .try_pop
    cmp eax, 0x57
    jbe .push_reg
.try_pop:
    cmp eax, 0x58
    jb .try_immreg
    cmp eax, 0x5f
    jbe .pop_reg
.try_immreg:
    cmp eax, 0xb8
    jb .try_jcc
    cmp eax, 0xbf
    jbe .mov_immreg
.try_jcc:
    cmp eax, 0x70
    jb .try_branch
    cmp eax, 0x7f
    jbe .jcc_short
.try_branch:
    cmp eax, 0xe8
    je .call_near
    cmp eax, 0xe9
    je .jmp_near
    cmp eax, 0xeb
    je .jmp_short
    cmp eax, 0xc3
    je .ret
    cmp eax, 0xc9
    je .leave
    cmp eax, 0x25
    je .and_acc
    cmp eax, 0x2d
    je .sub_acc
    cmp eax, 0x3d
    je .cmp_acc
    cmp eax, 0xa9
    je .test_acc

    ; Integer ModRM forms. The two direction encodings share an ID and
    ; reverse their decoded operands, not their bytes.
    cmp eax, 0x01
    je .add_rm_reg
    cmp eax, 0x03
    je .add_reg_rm
    cmp eax, 0x29
    je .sub_rm_reg
    cmp eax, 0x2b
    je .sub_reg_rm
    cmp eax, 0x31
    je .xor_rm_reg
    cmp eax, 0x33
    je .xor_reg_rm
    cmp eax, 0x39
    je .cmp_rm_reg
    cmp eax, 0x3b
    je .cmp_reg_rm
    cmp eax, 0x89
    je .mov_rm_reg
    cmp eax, 0x8b
    je .mov_reg_rm
    cmp eax, 0x85
    je .test_rm_reg
    cmp eax, 0x8d
    je .lea_reg_rm
    cmp eax, 0x63
    je .movsxd
    cmp eax, 0x69
    je .imul_imm32
    cmp eax, 0x6b
    je .imul_imm8
    cmp eax, 0x81
    je .group81
    cmp eax, 0x83
    je .group83
    cmp eax, 0xc1
    je .shift_imm
    cmp eax, 0xd1
    je .shift_one
    cmp eax, 0xd3
    je .shift_cl
    cmp eax, 0xc6
    je .mov_imm8
    cmp eax, 0xc7
    je .mov_imm
    cmp eax, 0xf7
    je .groupf7
    cmp eax, 0xff
    je .groupff
    jmp .fail

.nop90:
    ; 41 90 is XCHG with r8, not NOP.
    cmp dword [rsp+KL_B], 0
    jne .fail
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    jmp .finish_nop
.push_reg:
    mov dword [r14+KD_OP], KO_PUSH
    jmp .stack_reg
.pop_reg:
    mov dword [r14+KD_OP], KO_POP
.stack_reg:
    and eax, 7
    mov ecx, [rsp+KL_B]
    shl ecx, 3
    or eax, ecx
    or eax, 0x100
    mov [r14+KD_ARG0], eax
    cmp dword [rsp+KL_PP], 1
    je .stack16
    mov dword [rsp+KL_WIDTH], 64
    jmp .finish
.stack16:
    cmp dword [rsp+KL_W], 0
    jne .fail                     ; mixed stack size prefixes are unsupported
    mov dword [rsp+KL_WIDTH], 16
    jmp .finish
.mov_immreg:
    mov dword [r14+KD_OP], KO_MOV
    and eax, 7
    mov ecx, [rsp+KL_B]
    shl ecx, 3
    or eax, ecx
    or eax, 0x100
    mov [r14+KD_ARG0], eax
    mov dword [r14+KD_ARG1], 0x400
    mov eax, [rsp+KL_WIDTH]
    shr eax, 3
    mov [rsp+KL_IMMSZ], eax
    mov dword [rsp+KL_IMMMODE], 1  ; zero extend full-width MOV immediate
    jmp .read_imm
.jcc_short:
    sub eax, 0x70
    add eax, KO_JCC
    mov [r14+KD_OP], eax
    mov dword [rsp+KL_IMMSZ], 1
    jmp .relative
.call_near:
    mov dword [r14+KD_OP], KO_CALL
    mov dword [rsp+KL_IMMSZ], 4
    jmp .relative
.jmp_near:
    mov dword [r14+KD_OP], KO_JMP
    mov dword [rsp+KL_IMMSZ], 4
    jmp .relative
.jmp_short:
    mov dword [r14+KD_OP], KO_JMP
    mov dword [rsp+KL_IMMSZ], 1
.relative:
    cmp dword [rsp+KL_PP], 0
    jne .fail                     ; operand-size branch variants not supported
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    mov dword [r14+KD_ARG0], 0x500
    or dword [r14+KD_ATTR], KD_BRANCH
    mov dword [rsp+KL_IMMMODE], 2 ; signed relative target
    jmp .read_imm
.ret:
    mov dword [r14+KD_OP], KO_RET
    cmp dword [rsp+KL_PP], 1
    jne .ret64
    cmp dword [rsp+KL_W], 0
    jne .fail                     ; do not guess mixed size-prefix semantics
    jmp .finish
.ret64:
    mov dword [rsp+KL_WIDTH], 64
    jmp .finish
.leave:
    mov dword [r14+KD_OP], KO_LEAVE
    cmp dword [rsp+KL_PP], 1
    jne .leave64
    cmp dword [rsp+KL_W], 0
    jne .fail                     ; do not guess mixed size-prefix semantics
    jmp .finish
.leave64:
    mov dword [rsp+KL_WIDTH], 64
    jmp .finish
.and_acc:
    mov dword [r14+KD_OP], KO_AND
    jmp .acc_imm
.sub_acc:
    mov dword [r14+KD_OP], KO_SUB
    jmp .acc_imm
.cmp_acc:
    mov dword [r14+KD_OP], KO_CMP
    jmp .acc_imm
.test_acc:
    mov dword [r14+KD_OP], KO_TEST
.acc_imm:
    mov dword [r14+KD_ARG0], 0x100
    mov dword [r14+KD_ARG1], 0x400
    mov dword [rsp+KL_IMMSZ], 4
    cmp dword [rsp+KL_WIDTH], 16
    jne .acc_width_done
    mov dword [rsp+KL_IMMSZ], 2
.acc_width_done:
    mov dword [rsp+KL_IMMMODE], 3
    jmp .read_imm

.add_rm_reg:
    mov dword [r14+KD_OP], KO_ADD
    jmp .rm_reg
.add_reg_rm:
    mov dword [r14+KD_OP], KO_ADD
    jmp .reg_rm
.sub_rm_reg:
    mov dword [r14+KD_OP], KO_SUB
    jmp .rm_reg
.sub_reg_rm:
    mov dword [r14+KD_OP], KO_SUB
    jmp .reg_rm
.xor_rm_reg:
    mov dword [r14+KD_OP], KO_XOR
    jmp .rm_reg
.xor_reg_rm:
    mov dword [r14+KD_OP], KO_XOR
    jmp .reg_rm
.cmp_rm_reg:
    mov dword [r14+KD_OP], KO_CMP
    jmp .rm_reg
.cmp_reg_rm:
    mov dword [r14+KD_OP], KO_CMP
    jmp .reg_rm
.mov_rm_reg:
    mov dword [r14+KD_OP], KO_MOV
    jmp .rm_reg
.mov_reg_rm:
    mov dword [r14+KD_OP], KO_MOV
    jmp .reg_rm
.test_rm_reg:
    mov dword [r14+KD_OP], KO_TEST
    jmp .rm_reg
.lea_reg_rm:
    mov dword [r14+KD_OP], KO_LEA
    jmp .reg_rm
.movsxd:
    cmp dword [rsp+KL_WIDTH], 64
    jne .fail
    mov dword [r14+KD_OP], KO_MOVSXD
    jmp .reg_rm
.imul_imm32:
    mov dword [rsp+KL_IMMSZ], 4
    cmp dword [rsp+KL_WIDTH], 16
    jne .imul_imm_common
    mov dword [rsp+KL_IMMSZ], 2
    jmp .imul_imm_common
.imul_imm8:
    mov dword [rsp+KL_IMMSZ], 1
.imul_imm_common:
    mov dword [r14+KD_OP], KO_IMUL
    mov dword [rsp+KL_IMMMODE], 3
    mov dword [rsp+KL_FORM], KF_REG_RM_IMM
    jmp .modrm
.group81:
    mov dword [rsp+KL_IMMSZ], 4
    cmp dword [rsp+KL_WIDTH], 16
    jne .group_immediate
    mov dword [rsp+KL_IMMSZ], 2
    jmp .group_immediate
.group83:
    mov dword [rsp+KL_IMMSZ], 1
.group_immediate:
    mov dword [rsp+KL_IMMMODE], 3
    mov dword [rsp+KL_FORM], KF_RM_IMM
    jmp .modrm
.shift_imm:
    mov dword [rsp+KL_IMMSZ], 1
    mov dword [rsp+KL_IMMMODE], 1
    mov dword [rsp+KL_FORM], KF_RM_IMM
    jmp .modrm
.shift_one:
    mov qword [r14+KD_IMM], 1
    mov dword [rsp+KL_FORM], KF_RM_IMM
    jmp .modrm
.shift_cl:
    mov dword [rsp+KL_FORM], KF_RM_REG
    jmp .modrm
.mov_imm8:
    mov dword [rsp+KL_WIDTH], 8
    mov dword [rsp+KL_IMMSZ], 1
    jmp .mov_imm_common
.mov_imm:
    mov dword [rsp+KL_IMMSZ], 4
    cmp dword [rsp+KL_WIDTH], 16
    jne .mov_imm_common
    mov dword [rsp+KL_IMMSZ], 2
.mov_imm_common:
    mov dword [r14+KD_OP], KO_MOV
    mov dword [rsp+KL_IMMMODE], 3
    mov dword [rsp+KL_FORM], KF_RM_IMM
    jmp .modrm
.groupf7:
    mov dword [rsp+KL_FORM], KF_RM
    jmp .modrm
.groupff:
    mov dword [rsp+KL_FORM], KF_RM
    jmp .modrm
.rm_reg:
    mov dword [rsp+KL_FORM], KF_RM_REG
    jmp .modrm
.reg_rm:
    mov dword [rsp+KL_FORM], KF_REG_RM
    jmp .modrm

.dispatch_0f:
    cmp eax, 0x80
    jb .not_jcc_near
    cmp eax, 0x8f
    ja .not_jcc_near
    sub eax, 0x80
    add eax, KO_JCC
    mov [r14+KD_OP], eax
    mov dword [rsp+KL_IMMSZ], 4
    jmp .relative
.not_jcc_near:
    cmp dword [rsp+KL_PP], 2
    jae .fail
    cmp eax, 0x1f
    je .nop_modrm
    cmp eax, 0x18
    je .prefetch
    cmp eax, 0x47
    je .cmova
    cmp eax, 0xae
    je .sfence
    cmp eax, 0xaf
    je .imul_reg
    cmp eax, 0xc1
    je .xadd
    jmp .fail
.sfence:
    lea rcx, [rsp+KL_BUF+2]
    cmp r15, rcx
    jne .fail
    cmp byte [r15], 0xf8
    jne .fail
    inc r15
    mov dword [r14+KD_OP], KO_SFENCE
    jmp .finish
.nop_modrm:
    cmp dword [rsp+KL_LOCK], 0
    jne .fail
    mov dword [rsp+KL_FORM], KF_RM
    jmp .modrm
.prefetch:
    mov dword [r14+KD_OP], KO_PREFETCH
    mov dword [rsp+KL_FORM], KF_RM
    jmp .modrm
.cmova:
    mov dword [r14+KD_OP], KO_CMOVA
    jmp .reg_rm
.imul_reg:
    mov dword [r14+KD_OP], KO_IMUL
    jmp .reg_rm
.xadd:
    mov dword [r14+KD_OP], KO_XADD
    jmp .rm_reg

.dispatch_vex:
    ; Retain map/opcode/mandatory prefix/W/L, but normalize the inverted
    ; VEX register bits, so VEX2 and VEX3 encode the same descriptor.
    mov dword [rsp+KL_REGTYPE], 1
    mov dword [rsp+KL_RMTYPE], 1
    mov ecx, [rsp+KL_MAP]
    shl ecx, 8
    or ecx, eax
    or ecx, 0x10000
    mov [r14+KD_OP], ecx
    cmp dword [rsp+KL_MAP], 1
    je .vex_0f
    cmp dword [rsp+KL_MAP], 2
    je .vex_0f38
    cmp dword [rsp+KL_MAP], 3
    je .vex_0f3a
    jmp .fail
.vex_0f:
    ; Except integer conversion, the supported 0F vector forms are WIG:
    ; the encoded VEX.W bit is ignored by the CPU.
    cmp eax, 0x2a
    je .vex_0f_w_set
    mov dword [rsp+KL_W], 0
    mov dword [rsp+KL_WIDTH], 32
.vex_0f_w_set:
    ; Scalar forms require L=0 for portable behavior across CPU generations.
    cmp eax, 0x2e
    je .vex_0f_ignore_l
    cmp dword [rsp+KL_PP], 2
    jb .vex_0f_l_set
    cmp eax, 0x10
    je .vex_0f_ignore_l
    cmp eax, 0x11
    je .vex_0f_ignore_l
    cmp eax, 0x2a
    je .vex_0f_ignore_l
    cmp eax, 0x58
    je .vex_0f_ignore_l
    cmp eax, 0x59
    je .vex_0f_ignore_l
    cmp eax, 0x5c
    je .vex_0f_ignore_l
    cmp eax, 0x5e
    je .vex_0f_ignore_l
    cmp eax, 0x5f
    jne .vex_0f_l_set
.vex_0f_ignore_l:
    cmp dword [rsp+KL_L], 0
    jne .fail
.vex_0f_l_set:
    cmp eax, 0x10
    je .vex_move_load
    cmp eax, 0x11
    je .vex_move_store
    cmp eax, 0x28
    je .vex_aligned_load
    cmp eax, 0x29
    je .vex_aligned_store
    cmp eax, 0x14
    je .vex_three
    cmp eax, 0x15
    je .vex_three
    cmp eax, 0x2a
    je .vex_convert
    cmp eax, 0x2e
    je .vex_compare
    cmp eax, 0x54
    je .vex_three
    cmp eax, 0x57
    je .vex_three
    cmp eax, 0x58
    je .vex_three
    cmp eax, 0x59
    je .vex_three
    cmp eax, 0x5c
    je .vex_three
    cmp eax, 0x5e
    je .vex_three
    cmp eax, 0x5f
    je .vex_three
    jmp .fail
.vex_0f38:
    cmp eax, 0x19
    je .vex_unary
    cmp eax, 0xbc
    je .vex_three
    cmp eax, 0xbd
    je .vex_three
    jmp .fail
.vex_0f3a:
    cmp eax, 0x06
    jne .fail
    mov dword [rsp+KL_IMMSZ], 1
    mov dword [rsp+KL_IMMMODE], 1
    mov dword [rsp+KL_FORM], KF_VEC4
    jmp .modrm
.vex_convert:
    ; Integer source, vector destination and first source.
    mov dword [rsp+KL_RMTYPE], 0
    jmp .vex_three
.vex_compare:
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [rsp+KL_FORM], KF_VEC2_REG_RM
    jmp .modrm
.vex_unary:
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [rsp+KL_FORM], KF_VEC2_REG_RM
    jmp .modrm
.vex_three:
    mov dword [rsp+KL_FORM], KF_VEC3
    jmp .modrm
.vex_aligned_load:
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [r14+KD_OP], 0x10128
    mov dword [rsp+KL_FORM], KF_VEC2_REG_RM
    jmp .modrm
.vex_aligned_store:
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [r14+KD_OP], 0x10128
    mov dword [rsp+KL_FORM], KF_VEC2_RM_REG
    jmp .modrm
.vex_move_load:
    cmp dword [rsp+KL_PP], 2
    jae .vex_scalar_load
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [r14+KD_OP], 0x10110
    mov dword [rsp+KL_FORM], KF_VEC2_REG_RM
    jmp .modrm
.vex_move_store:
    cmp dword [rsp+KL_PP], 2
    jae .vex_scalar_store
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [r14+KD_OP], 0x10110
    mov dword [rsp+KL_FORM], KF_VEC2_RM_REG
    jmp .modrm
.vex_scalar_load:
    ; Register form has three operands; memory form has two and reserves vvvv.
    mov dword [rsp+KL_FORM], KF_VEC3
    jmp .modrm
.vex_scalar_store:
    ; The common scalar store is a two-operand memory form. The rare register
    ; form is deliberately rejected rather than normalized incorrectly.
    mov dword [rsp+KL_FORM], KF_VEC2_RM_REG
    jmp .modrm

.modrm:
    movzx eax, byte [r15]
    inc r15
    mov [rsp+KL_MODRM], eax
    mov ecx, eax
    shr ecx, 6
    mov [rsp+KL_MOD], ecx
    mov ecx, eax
    shr ecx, 3
    and ecx, 7
    mov [rsp+KL_GROUP], ecx
    mov edx, [rsp+KL_R]
    shl edx, 3
    or ecx, edx
    mov [rsp+KL_REG], ecx
    and eax, 7
    mov [rsp+KL_RM], eax

    ; The opcode group extension is the raw reg field, never REX.R.
    cmp dword [rsp+KL_VEX], 0
    jne .modrm_vector_special
    cmp dword [rsp+KL_MAP], 1
    je .modrm_0f_special
    mov eax, [rsp+KL_OPCODE]
    cmp eax, 0x81
    je .modrm_group1
    cmp eax, 0x83
    je .modrm_group1
    cmp eax, 0xc1
    je .modrm_shift
    cmp eax, 0xd1
    je .modrm_shift
    cmp eax, 0xd3
    je .modrm_shift
    cmp eax, 0xc6
    je .modrm_mov_group
    cmp eax, 0xc7
    je .modrm_mov_group
    cmp eax, 0xf7
    je .modrm_f7
    cmp eax, 0xff
    je .modrm_ff
    jmp .modrm_operand
.modrm_0f_special:
    mov eax, [rsp+KL_OPCODE]
    cmp eax, 0x1f
    jne .not_nop_group
    cmp dword [rsp+KL_GROUP], 0
    jne .fail
    jmp .modrm_operand
.not_nop_group:
    cmp eax, 0x18
    jne .modrm_operand
    cmp dword [rsp+KL_MOD], 3
    je .fail
    cmp dword [rsp+KL_GROUP], 3
    ja .fail
    mov eax, [rsp+KL_GROUP]
    add eax, 0x1500
    mov [r14+KD_OP], eax
    jmp .modrm_operand
.modrm_group1:
    mov eax, [rsp+KL_GROUP]
    cmp eax, 0
    je .group_add
    cmp eax, 1
    je .group_or
    cmp eax, 2
    je .group_adc
    cmp eax, 3
    je .group_sbb
    cmp eax, 4
    je .group_and
    cmp eax, 5
    je .group_sub
    cmp eax, 6
    je .group_xor
    mov eax, KO_CMP
    jmp .group_set
.group_add: mov eax, KO_ADD
    jmp .group_set
.group_or:  mov eax, KO_OR
    jmp .group_set
.group_adc: mov eax, KO_ADC
    jmp .group_set
.group_sbb: mov eax, KO_SBB
    jmp .group_set
.group_and: mov eax, KO_AND
    jmp .group_set
.group_sub: mov eax, KO_SUB
    jmp .group_set
.group_xor: mov eax, KO_XOR
.group_set:
    mov [r14+KD_OP], eax
    jmp .modrm_operand
.modrm_shift:
    mov eax, [rsp+KL_GROUP]
    ; /6 is undocumented alias; do not silently equate it with /4.
    cmp eax, 6
    je .fail
    add eax, KO_SHIFT
    mov [r14+KD_OP], eax
    jmp .modrm_operand
.modrm_mov_group:
    cmp dword [rsp+KL_GROUP], 0
    jne .fail
    jmp .modrm_operand
.modrm_f7:
    mov eax, [rsp+KL_GROUP]
    cmp eax, 1
    je .fail
    test eax, eax
    jnz .f7_unary
    mov dword [r14+KD_OP], KO_TEST
    mov dword [rsp+KL_FORM], KF_RM_IMM
    mov dword [rsp+KL_IMMSZ], 4
    cmp dword [rsp+KL_WIDTH], 16
    jne .f7_imm_ready
    mov dword [rsp+KL_IMMSZ], 2
.f7_imm_ready:
    mov dword [rsp+KL_IMMMODE], 3
    jmp .modrm_operand
.f7_unary:
    add eax, KO_GROUPF7
    mov [r14+KD_OP], eax
    jmp .modrm_operand
.modrm_ff:
    mov eax, [rsp+KL_GROUP]
    cmp eax, 0
    je .ff_group_ok
    cmp eax, 1
    je .ff_group_ok
    cmp eax, 2
    je .ff_control
    cmp eax, 4
    je .ff_control
    cmp eax, 6
    jne .fail
.ff_control:
    cmp dword [rsp+KL_PP], 1
    jne .ff_control64
    cmp eax, 6
    jne .fail                     ; do not guess 16-bit indirect control flow
    cmp dword [rsp+KL_W], 0
    jne .fail                     ; mixed stack size prefixes are unsupported
    mov dword [rsp+KL_WIDTH], 16
    jmp .ff_group_ok
.ff_control64:
    mov dword [rsp+KL_WIDTH], 64
.ff_group_ok:
    add eax, KO_GROUPFF
    mov [r14+KD_OP], eax
    jmp .modrm_operand
.modrm_vector_special:
    cmp dword [rsp+KL_MAP], 1
    jne .modrm_operand
    cmp dword [rsp+KL_OPCODE], 0x10
    jne .vector_store_check
    cmp dword [rsp+KL_PP], 2
    jb .modrm_operand
    cmp dword [rsp+KL_MOD], 3
    je .modrm_operand
    cmp dword [rsp+KL_VVVV], 0
    jne .fail
    mov dword [rsp+KL_FORM], KF_VEC2_REG_RM
    jmp .modrm_operand
.vector_store_check:
    cmp dword [rsp+KL_OPCODE], 0x11
    jne .modrm_operand
    cmp dword [rsp+KL_PP], 2
    jb .modrm_operand
    cmp dword [rsp+KL_MOD], 3
    je .fail
    cmp dword [rsp+KL_VVVV], 0
    jne .fail

.modrm_operand:
    cmp dword [rsp+KL_ADDR32], 0
    je .address_size_ok
    cmp dword [rsp+KL_MOD], 0
    jne .address_size_ok
    cmp dword [rsp+KL_RM], 5
    je .fail                        ; EIP-relative form is not implemented
.address_size_ok:
    cmp dword [rsp+KL_MOD], 3
    jne .memory_operand
    mov eax, [rsp+KL_RM]
    mov ecx, [rsp+KL_B]
    shl ecx, 3
    or eax, ecx
    cmp dword [rsp+KL_RMTYPE], 0
    jne .rm_vector
    cmp dword [rsp+KL_WIDTH], 8
    jne .rm_gpr
    cmp dword [rsp+KL_REX], 0
    jne .rm_gpr
    cmp eax, 4
    jb .rm_gpr
    cmp eax, 7
    ja .rm_gpr
    add eax, 12                     ; AH..BH are distinct from SPL..DIL
.rm_gpr:
    or eax, 0x100
    jmp .rm_ready
.rm_vector:
    or eax, 0x200
.rm_ready:
    mov [rsp+KL_RM], eax
    cmp dword [r14+KD_OP], KO_LEA
    je .fail                        ; LEA with register ModRM is invalid
    jmp .reg_operand

.memory_operand:
    mov eax, [rsp+KL_RM]
    cmp eax, 4
    je .sib
    cmp eax, 5
    jne .memory_base
    cmp dword [rsp+KL_MOD], 0
    jne .memory_base
    movsxd rax, dword [r15]
    add r15, 4
    mov [rsp+KL_RIPDISP], eax
    or dword [r14+KD_ATTR], KD_RIP
    jmp .memory_done
.memory_base:
    mov ecx, [rsp+KL_B]
    shl ecx, 3
    or eax, ecx
    mov [r14+KD_BASE], eax
    jmp .memory_displacement
.sib:
    movzx eax, byte [r15]
    inc r15
    mov ecx, eax
    shr ecx, 6
    mov edx, 1
    shl edx, cl
    mov [r14+KD_SCALE], edx
    mov ecx, eax
    shr ecx, 3
    and ecx, 7
    cmp ecx, 4
    jne .sib_index
    cmp dword [rsp+KL_X], 0
    je .sib_no_index
.sib_index:
    mov edx, [rsp+KL_X]
    shl edx, 3
    or ecx, edx
    mov [r14+KD_INDEX], ecx
.sib_no_index:
    cmp dword [r14+KD_INDEX], 0xff
    jne .sib_index_done
    mov dword [r14+KD_SCALE], 1    ; no-index scale bits have no effect
.sib_index_done:
    and eax, 7
    cmp eax, 5
    jne .sib_base
    cmp dword [rsp+KL_MOD], 0
    jne .sib_base
    movsxd rax, dword [r15]
    add r15, 4
    mov [r14+KD_DISP], rax
    jmp .memory_done
.sib_base:
    mov ecx, [rsp+KL_B]
    shl ecx, 3
    or eax, ecx
    mov [r14+KD_BASE], eax
.memory_displacement:
    cmp dword [rsp+KL_MOD], 1
    je .disp8
    cmp dword [rsp+KL_MOD], 2
    je .disp32
    jmp .memory_done
.disp8:
    movsx rax, byte [r15]
    inc r15
    mov [r14+KD_DISP], rax
    jmp .memory_done
.disp32:
    movsxd rax, dword [r15]
    add r15, 4
    mov [r14+KD_DISP], rax
.memory_done:
    mov dword [rsp+KL_RM], 0x300

.reg_operand:
    ; Multi-byte NOPs do not read memory; discard every decoded operand.
    cmp dword [rsp+KL_VEX], 0
    jne .lock_validation
    cmp dword [rsp+KL_MAP], 1
    jne .lock_validation
    cmp dword [rsp+KL_OPCODE], 0x1f
    je .finish_nop
.lock_validation:
    cmp dword [rsp+KL_LOCK], 0
    je .lock_ok
    cmp dword [rsp+KL_MOD], 3
    je .fail
    cmp dword [rsp+KL_FORM], KF_RM_REG
    je .lock_check_op
    cmp dword [rsp+KL_FORM], KF_RM_IMM
    je .lock_check_op
    cmp dword [rsp+KL_FORM], KF_RM
    jne .fail
    mov eax, [r14+KD_OP]
    cmp eax, KO_GROUPFF
    je .lock_ok
    cmp eax, KO_GROUPFF+1
    je .lock_ok
    jmp .fail
.lock_check_op:
    mov eax, [r14+KD_OP]
    cmp eax, KO_ADD
    je .lock_ok
    cmp eax, KO_SUB
    je .lock_ok
    cmp eax, KO_XOR
    je .lock_ok
    cmp eax, KO_AND
    je .lock_ok
    cmp eax, KO_OR
    je .lock_ok
    cmp eax, KO_ADC
    je .lock_ok
    cmp eax, KO_SBB
    je .lock_ok
    cmp eax, KO_XADD
    jne .fail
.lock_ok:
    mov dword [rsp+KL_LOCKOK], 1

.assign_operands:
    mov eax, [rsp+KL_REG]
    cmp dword [rsp+KL_REGTYPE], 0
    jne .reg_vector
    cmp dword [rsp+KL_WIDTH], 8
    jne .reg_gpr
    cmp dword [rsp+KL_REX], 0
    jne .reg_gpr
    cmp eax, 4
    jb .reg_gpr
    cmp eax, 7
    ja .reg_gpr
    add eax, 12
.reg_gpr:
    or eax, 0x100
    jmp .reg_ready
.reg_vector:
    or eax, 0x200
.reg_ready:
    mov ecx, [rsp+KL_RM]
    mov edx, [rsp+KL_FORM]
    cmp edx, KF_RM_REG
    je .form_rm_reg
    cmp edx, KF_REG_RM
    je .form_reg_rm
    cmp edx, KF_RM
    je .form_rm
    cmp edx, KF_RM_IMM
    je .form_rm_imm
    cmp edx, KF_REG_RM_IMM
    je .form_reg_rm_imm
    cmp edx, KF_VEC3
    je .form_vec3
    cmp edx, KF_VEC2_REG_RM
    je .form_reg_rm
    cmp edx, KF_VEC2_RM_REG
    je .form_rm_reg
    cmp edx, KF_VEC4
    je .form_vec4
    jmp .fail
.form_rm_reg:
    mov [r14+KD_ARG0], ecx
    mov [r14+KD_ARG1], eax
    cmp dword [rsp+KL_OPCODE], 0xd3
    jne .after_form
    cmp dword [rsp+KL_VEX], 0
    jne .after_form
    mov dword [r14+KD_ARG1], 0x8101 ; CL, independent of operation width
    jmp .after_form
.form_reg_rm:
    mov [r14+KD_ARG0], eax
    mov [r14+KD_ARG1], ecx
    jmp .after_form
.form_rm:
    mov [r14+KD_ARG0], ecx
    jmp .after_form
.form_rm_imm:
    mov [r14+KD_ARG0], ecx
    mov dword [r14+KD_ARG1], 0x400
    jmp .after_form
.form_reg_rm_imm:
    mov [r14+KD_ARG0], eax
    mov [r14+KD_ARG1], ecx
    mov dword [r14+KD_ARG2], 0x400
    jmp .after_form
.form_vec3:
    mov [r14+KD_ARG0], eax
    mov edx, [rsp+KL_VVVV]
    or edx, 0x200
    mov [r14+KD_ARG1], edx
    mov [r14+KD_ARG2], ecx
    jmp .after_form
.form_vec4:
    mov [r14+KD_ARG0], eax
    mov edx, [rsp+KL_VVVV]
    or edx, 0x200
    mov [r14+KD_ARG1], edx
    mov [r14+KD_ARG2], ecx
    mov dword [r14+KD_ARG3], 0x400
.after_form:
    cmp dword [rsp+KL_IMMSZ], 0
    jne .read_imm
    jmp .finish

.read_imm:
    mov ecx, [rsp+KL_IMMSZ]
    cmp ecx, 1
    je .imm8
    cmp ecx, 2
    je .imm16
    cmp ecx, 4
    je .imm32
    cmp ecx, 8
    je .imm64
    jmp .fail
.imm8:
    movzx eax, byte [r15]
    inc r15
    cmp dword [rsp+KL_IMMMODE], 1
    je .imm_value
    movsx rax, al
    jmp .imm_value
.imm16:
    movzx eax, word [r15]
    add r15, 2
    cmp dword [rsp+KL_IMMMODE], 1
    je .imm_value
    movsx rax, ax
    jmp .imm_value
.imm32:
    mov eax, [r15]
    add r15, 4
    cmp dword [rsp+KL_IMMMODE], 1
    je .imm_value
    movsxd rax, eax
    jmp .imm_value
.imm64:
    mov rax, [r15]
    add r15, 8
.imm_value:
    cmp dword [rsp+KL_IMMMODE], 3
    jne .store_imm
    mov ecx, [rsp+KL_WIDTH]
    cmp ecx, 64
    je .store_imm
    cmp ecx, 32
    je .imm_mask32
    cmp ecx, 16
    je .imm_mask16
    cmp ecx, 8
    jne .fail
    and eax, 0xff
    jmp .store_imm
.imm_mask16:
    and eax, 0xffff
    jmp .store_imm
.imm_mask32:
    mov eax, eax
.store_imm:
    mov [r14+KD_IMM], rax
    jmp .finish

.finish_nop:
    ; NOP carries no semantic operands or attributes, regardless of length.
    xor eax, eax
    mov rdi, r14
    mov ecx, 8
    rep stosq
    jmp .length
.finish:
    cmp dword [rsp+KL_ADDR32], 0
    je .address_complete
    cmp dword [r14+KD_ARG0], 0x300
    je .address_record
    cmp dword [r14+KD_ARG1], 0x300
    je .address_record
    cmp dword [r14+KD_ARG2], 0x300
    je .address_record
    cmp dword [r14+KD_ARG3], 0x300
    jne .address_complete
.address_record:
    or dword [r14+KD_ATTR], 1 << 14
.address_complete:
    cmp dword [rsp+KL_LOCK], 0
    je .lock_complete
    cmp dword [rsp+KL_LOCKOK], 1
    jne .fail
    or dword [r14+KD_ATTR], 1 << 8
.lock_complete:
    mov eax, [rsp+KL_WIDTH]
    test dword [r14+KD_ATTR], KD_BRANCH
    jz .record_width
    xor eax, eax                   ; short and near branches have same width
.record_width:
    or [r14+KD_ATTR], eax
    cmp dword [rsp+KL_VEX], 0
    je .length
    or dword [r14+KD_ATTR], 1 << 9
    cmp dword [rsp+KL_L], 0
    je .vex_l_done
    or dword [r14+KD_ATTR], 1 << 10
.vex_l_done:
    cmp dword [rsp+KL_W], 0
    je .vex_w_done
    or dword [r14+KD_ATTR], 1 << 11
.vex_w_done:
    mov eax, [rsp+KL_PP]
    shl eax, 12
    or [r14+KD_ATTR], eax
.length:
    lea rax, [rsp+KL_BUF]
    mov rcx, r15
    sub rcx, rax
    test ecx, ecx
    jz .fail
    cmp ecx, 15
    ja .fail
    cmp ecx, [rsp+KL_AVAIL]
    ja .fail
    mov [r14+KD_LEN], ecx
    test dword [r14+KD_ATTR], KD_RIP
    jz .branch_target
    movsxd rax, dword [rsp+KL_RIPDISP]
    add rax, r12
    add rax, rcx
    mov [r14+KD_TARGET], rax
.branch_target:
    test dword [r14+KD_ATTR], KD_BRANCH
    jz .success
    mov rax, [r14+KD_IMM]
    add rax, r12
    add rax, rcx
    mov [r14+KD_TARGET], rax
    mov qword [r14+KD_IMM], 0
.success:
    mov eax, ecx
    jmp .return
.fail:
    xor eax, eax
.return:
    add rsp, KL_FRAME
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
