; Assemble a deliberately incorrect numerical kernel without editing sources.
; The normal verifier must reject the changed generator constant.
%assign kernel_mutations 0
%macro add 2
    %ifidni %1, r12d
        %ifidni %2, 1325
            %assign kernel_mutations kernel_mutations + 1
            add %1, 1326
        %else
            add %1, %2
        %endif
    %else
        add %1, %2
    %endif
%endmacro

%ifdef KERNEL_MUTATION_ELF
    %include 'elf.asm'
%else
    %include 'native/pe.asm'
%endif

; The primary and dual routines must both receive the intended mutation.
%if kernel_mutations != 2
    %error Expected the numerical mutation in exactly two instances
%endif
