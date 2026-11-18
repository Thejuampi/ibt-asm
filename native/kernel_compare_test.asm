; Standalone regression tests for the production semantic comparator.
; NASM -f bin [-D KERNEL_TEST_ELF=1] -o <output> native/kernel_compare_test.asm
; Returns 0 on success, otherwise the ordinal of the first failing case.
bits 64
default rel
%ifdef KERNEL_TEST_ELF
%define TEST_BASE 0x400000
section .header start=0 vstart=TEST_BASE
db 0x7f,'ELF',2,1,1,0
times 8 db 0
dw 2,0x3e
dd 1
dq _start,64,0
dd 0
dw 64,56,1,0,0,0
dd 1,5
dq 0,TEST_BASE,TEST_BASE
dq file_end-TEST_BASE,file_end-TEST_BASE,0x1000
times 0x1000-($-$$) db 0
section .text start=0x1000 vstart=TEST_BASE+0x1000
%else
%define TEST_BASE 0x140000000
section .header start=0 vstart=TEST_BASE
db 'MZ'
times 0x3c-($-$$) db 0
dd pe_header-$$
pe_header:
db 'PE',0,0
dw 0x8664,2
dd 0,0,0
dw 0xf0,0x22
dw 0x20b
db 0,0
dd text_raw_size,imports_raw_size,0
dd _start-TEST_BASE,text_start-TEST_BASE
dq TEST_BASE
dd 0x1000,0x200
dw 6,0,0,0,6,0
dd 0
dd imports-TEST_BASE + ((imports_data_end-imports+0xfff)&~0xfff)
dd 0x200,0
dw 3,0x100
dq 0x100000,0x1000,0x100000,0x1000
dd 0,16
dd 0,0
dd imports-TEST_BASE,imports_end-imports
times 14 dq 0
db '.text',0,0,0
dd text_end-text_start,text_start-TEST_BASE
dd text_raw_size,0x200
dd 0,0
dw 0,0
dd 0x60000020
db '.idata',0,0
dd imports_data_end-imports,imports-TEST_BASE
dd imports_raw_size,0x200+text_raw_size
dd 0,0
dw 0,0
dd 0xc0000040
times 0x200-($-$$) db 0
section .text start=0x200 vstart=TEST_BASE+0x1000
%endif

text_start:
_start:
%ifdef KERNEL_TEST_ELF
    sub rsp,0x20                 ; keep stack aligned for the comparator
%else
    sub rsp,0x28                 ; alignment and Windows home space
%endif
%assign test_no 0
%macro CHECK 5
    %assign test_no test_no+1
    lea rsi,[%1]
    lea rdi,[%2]
    lea rdx,[%3]
    lea rcx,[%4]
    lea r8,[resolve]
    xor r9d,r9d
    call kcmp_compare_ranges
    cmp eax,%5
    je %%pass
    mov ecx,test_no
    jmp test_exit
%%pass:
%endmacro
    CHECK mov_a,mov_a_end,mov_b,mov_b_end,1
    CHECK mov_a,mov_a_end,mov_wrong,mov_wrong_end,0
    CHECK nop_a,nop_a_end,nop_b,nop_b_end,1
    CHECK branch_a,branch_a_end,branch_b,branch_b_end,1
    CHECK branch_a,branch_a_end,branch_wrong,branch_wrong_end,0
    CHECK imm_a,imm_a_end,imm_b,imm_b_end,1
    CHECK imm_a,imm_a_end,imm_wrong,imm_wrong_end,0
    CHECK vex_a,vex_a_end,vex_b,vex_b_end,1
    CHECK rip_a,rip_a_end,rip_b,rip_b_end,1
    CHECK rip_a,rip_a_end,rip_wrong,rip_wrong_end,0
    CHECK disp_a,disp_a_end,disp_b,disp_b_end,1
    CHECK disp_a,disp_a_end,disp_wrong,disp_wrong_end,0
    CHECK mov_a,mov_a_end,truncated,truncated_end,0
    CHECK backwards_a,backwards_a_end,backwards_b,backwards_b_end,1
    CHECK backwards_a,backwards_a_end,backwards_wrong,backwards_wrong_end,0
    CHECK leave64,leave64_end,leave16,leave16_end,0
    CHECK leave16,leave16_end,leave16,leave16_end,1
    CHECK push16,push16_end,push16,push16_end,1
    CHECK push64,push64_end,push16,push16_end,0
    CHECK push16,push16_end,push16_rex,push16_rex_end,0
    CHECK push64,push64_end,push16_rex,push16_rex_end,0
    CHECK pop16,pop16_end,pop16,pop16_end,1
    CHECK pop64,pop64_end,pop16,pop16_end,0
    CHECK pop16,pop16_end,pop16_rex,pop16_rex_end,0
    CHECK pop64,pop64_end,pop16_rex,pop16_rex_end,0
    CHECK pushm16,pushm16_end,pushm16,pushm16_end,1
    CHECK pushm64,pushm64_end,pushm16,pushm16_end,0
    CHECK pushm16,pushm16_end,pushm16_rex,pushm16_rex_end,0
    CHECK pushm64,pushm64_end,pushm16_rex,pushm16_rex_end,0
    CHECK unsupported,unsupported_end,unsupported,unsupported_end,0
    CHECK leave_mixed,leave_mixed_end,leave_mixed,leave_mixed_end,0
    CHECK ret_mixed,ret_mixed_end,ret_mixed,ret_mixed_end,0
    CHECK call16,call16_end,call16,call16_end,0
    CHECK jmp16,jmp16_end,jmp16,jmp16_end,0
    xor ecx,ecx
test_exit:
%ifdef KERNEL_TEST_ELF
    mov edi,ecx
    mov eax,60
    syscall
%else
    call [rel iat]
%endif

resolve:
    lea r10,[target_a]
    cmp rax,r10
    je .one
    lea r10,[target_b]
    cmp rax,r10
    je .one
    lea r10,[target_wrong]
    cmp rax,r10
    je .two
    stc
    ret
.one:
    mov eax,1
    clc
    ret
.two:
    mov eax,2
    clc
    ret

%include 'native/kernel_decode.inc'
%include 'native/kernel_semantic_compare.inc'

mov_a: db 0x48,0x89,0xce
mov_a_end:
mov_b: db 0x48,0x8b,0xf1
mov_b_end:
mov_wrong: db 0x48,0x89,0xc6
mov_wrong_end:
nop_a: db 0x90,0x48,0x89,0xce,0x90
nop_a_end:
nop_b: db 0x48,0x8b,0xf1
nop_b_end:
branch_a: db 0x74
    db branch_a_target-($+1)
    db 0x90
branch_a_target: db 0xc3
branch_a_end:
branch_b: db 0x0f,0x84
    dd branch_b_target-($+4)
    db 0x90,0x90
branch_b_target: db 0xc3
branch_b_end:
branch_wrong: db 0x74
    db branch_wrong_target-($+1)
    db 0x90,0x50
branch_wrong_target: db 0xc3
branch_wrong_end:
imm_a: db 0x83,0xc0,1
imm_a_end:
imm_b: db 0x81,0xc0,1,0,0,0
imm_b_end:
imm_wrong: db 0x83,0xc0,2
imm_wrong_end:
vex_a: db 0xc5,0xf9,0x10,0xc1
vex_a_end:
vex_b: db 0xc4,0xe1,0x79,0x10,0xc1
vex_b_end:
rip_a: db 0x48,0x8b,0x05
    dd target_a-($+4)
rip_a_end:
rip_b: db 0x48,0x8b,0x05
    dd target_b-($+4)
rip_b_end:
rip_wrong: db 0x48,0x8b,0x05
    dd target_wrong-($+4)
rip_wrong_end:
disp_a: db 0x48,0x8b,0x47,1
disp_a_end:
disp_b: db 0x48,0x8b,0x87,1,0,0,0
disp_b_end:
disp_wrong: db 0x48,0x8b,0x47,2
disp_wrong_end:
truncated: db 0x48,0x8b
truncated_end:
backwards_a: db 0x48,0x89,0xce,0x75
    db backwards_a-($+1)
backwards_a_end:
backwards_b: db 0x48,0x8b,0xf1,0x90,0x0f,0x85
    dd backwards_b-($+4)
backwards_b_end:
backwards_wrong: db 0x48,0x8b,0xf1
backwards_wrong_branch: db 0x75
    db backwards_wrong_branch-($+1)
backwards_wrong_end:
leave64: db 0xc9
leave64_end:
leave16: db 0x66,0xc9
leave16_end:
push64: db 0x50
push64_end:
push16: db 0x66,0x50
push16_end:
push16_rex: db 0x66,0x48,0x50
push16_rex_end:
pop64: db 0x58
pop64_end:
pop16: db 0x66,0x58
pop16_end:
pop16_rex: db 0x66,0x48,0x58
pop16_rex_end:
pushm64: db 0xff,0x30
pushm64_end:
pushm16: db 0x66,0xff,0x30
pushm16_end:
pushm16_rex: db 0x66,0x48,0xff,0x30
pushm16_rex_end:
unsupported: db 0x0f,0x0b
unsupported_end:
leave_mixed: db 0x66,0x48,0xc9
leave_mixed_end:
ret_mixed: db 0x66,0x48,0xc3
ret_mixed_end:
call16: db 0x66,0xff,0xd0
call16_end:
jmp16: db 0x66,0xff,0xe0
jmp16_end:
target_a: db 0
target_b: db 0
target_wrong: db 0
%ifdef KERNEL_TEST_ELF
file_end:
%else
text_end:
section .idata follows=.text vfollows=.text align=512 valign=4096
imports:
dd int_table-TEST_BASE,0,0,dll_name-TEST_BASE,iat-TEST_BASE
times 5 dd 0
imports_end:
int_table: dq hint_name-TEST_BASE,0
iat: dq hint_name-TEST_BASE,0
dll_name: db 'KERNEL32.dll',0
align 2,db 0
hint_name: dw 0
db 'ExitProcess',0
imports_data_end:
text_raw_size equ (text_end-text_start+0x1ff)&~0x1ff
imports_raw_size equ (imports_data_end-imports+0x1ff)&~0x1ff
times imports_raw_size-(imports_data_end-imports) db 0
%endif
