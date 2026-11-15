; Change only the last adjacent helper range checked by the verifier.
; This guards against stopping the compact range table too early.
%assign late_mutations 0
%macro cmp 2
    %ifidni %2, 192
        %ifdef IBT_SECOND_ENGINE
            %ifndef IBT_THIRD_ENGINE
                %ifndef IBT_RESIDUAL_ENGINE
                    %assign late_mutations late_mutations + 1
                    cmp %1, 193
                %else
                    cmp %1, %2
                %endif
            %else
                cmp %1, %2
            %endif
        %else
            cmp %1, %2
        %endif
    %else
        cmp %1, %2
    %endif
%endmacro

%ifdef KERNEL_MUTATION_ELF
    %include 'elf.asm'
%else
    %include 'native/pe.asm'
%endif

%if late_mutations != 1
    %error Expected exactly one mutation in the last checked helper range
%endif
