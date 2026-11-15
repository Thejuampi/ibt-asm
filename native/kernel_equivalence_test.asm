; Build the real application with representation-only changes in its kernels.
; The verifier must accept each variant without weakening its negative checks.
%assign equivalent_movs 0
%assign equivalent_nops 0
%assign equivalent_jumps 0

%ifdef KERNEL_EQUIVALENT_MOV
%macro mov 2
    %ifidni %1, rsi
        %ifidni %2, rcx
            ; Alternate encoding of `mov rsi, rcx`: 48 8b f1 instead of 48 89 ce.
            db 0x48, 0x8b, 0xf1
            %assign equivalent_movs equivalent_movs + 1
        %else
            mov %1, %2
        %endif
    %else
        mov %1, %2
    %endif
%endmacro
%endif

%ifdef KERNEL_EQUIVALENT_BRANCH
%macro jmp 1
    %ifidni %1, .i
        ; Force the long form of a jump whose short form has the same target.
        jmp near %1
        %assign equivalent_jumps equivalent_jumps + 1
    %else
        jmp %1
    %endif
%endmacro
%endif

%ifdef KERNEL_EQUIVALENT_NOP
%macro add 2
    %ifidni %1, r12d
        %ifidni %2, 1325
            nop
            %assign equivalent_nops equivalent_nops + 1
        %endif
    %endif
    add %1, %2
%endmacro
%endif
%ifdef KERNEL_EQUIVALENT_ELF
    %include 'elf.asm'
%else
    %include 'native/pe.asm'
%endif

%ifdef KERNEL_EQUIVALENT_MOV
    %if equivalent_movs < 2
        %error Expected alternate MOV encoding in both numerical instances
    %endif
%endif
%ifdef KERNEL_EQUIVALENT_NOP
    %if equivalent_nops != 2
        %error Expected a NOP in both numerical instances
    %endif
%endif
%ifdef KERNEL_EQUIVALENT_BRANCH
    %if equivalent_jumps < 2
        %error Expected a long-form jump in both numerical instances
    %endif
%endif
