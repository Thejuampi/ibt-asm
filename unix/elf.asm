; Build a Linux x86-64 ELF image directly with NASM.
bits 64
default rel
%define ELF_BASE 0x400000
%define ELF_NATIVE 1

section .header start=0 vstart=ELF_BASE
    db 0x7f, 'ELF', 2, 1, 1, 0
    times 8 db 0
    dw 2, 0x3e
    dd 1
    dq _start, 64, 0
    dd 0
    dw 64, 56, 5, 0, 0, 0
    ; Interpreter.
    dd 3, 4
    dq elf_interpreter - ELF_BASE, elf_interpreter, elf_interpreter
    dq elf_interpreter_end - elf_interpreter, elf_interpreter_end - elf_interpreter, 1
    ; Read-only executable image, including ELF and program headers.
    dd 1, 5
    dq 0, ELF_BASE, ELF_BASE
    dq elf_text_end - ELF_BASE, elf_text_end - ELF_BASE, 0x1000
    ; Writable data and zero-initialized application state.
    dd 1, 6
    dq elf_data_start - ELF_BASE, elf_data_start, elf_data_start
    dq elf_data_file_size, elf_data_mem_size, 0x1000
    ; Dynamic linker metadata.
    dd 2, 6
    dq elf_dynamic - ELF_BASE, elf_dynamic, elf_dynamic
    dq elf_dynamic_end - elf_dynamic, elf_dynamic_end - elf_dynamic, 8
    ; The stack is writable, not executable.
    dd 0x6474e551, 6
    dq 0, 0, 0, 0, 0, 16
elf_interpreter: db '/lib64/ld-linux-x86-64.so.2', 0
elf_interpreter_end:
    ; Stable file offsets for the numerical reference ranges.
    times 0x200 - ($ - $$) db 0
%ifdef IBT_COMPACT_PIPELINE
    db 'KRN2'
%else
    db 'KRN1'
%endif
    dd 4
    dd lp_fill_cols - ELF_BASE, lp_emit - lp_fill_cols
    dd lp_pack_a - ELF_BASE, lp_core_end - lp_pack_a
    dd dual_lp_fill_cols - ELF_BASE, dual_core_end - dual_lp_fill_cols
    dd verify_lp_resid - ELF_BASE, verify_core_end - verify_lp_resid
%assign kernel_count 0
%macro KERNEL_LPK 1
    dd %1 - lp_fill_cols
%assign kernel_count kernel_count + 1
%endmacro
%macro KERNEL_BENCH 1
    dd %1 - lp_pack_a
%assign kernel_count kernel_count + 1
%endmacro
%include 'reference/numerical/kernel_table.inc'
%unmacro KERNEL_LPK 1
%unmacro KERNEL_BENCH 1
%if kernel_count != 26
    %error The numerical reference must name all 26 primary routines
%endif
%ifdef IBT_COMPACT_PIPELINE
    dd 0, dual_lp_fill_ones - dual_lp_fill_cols
    times kernel_count - 2 dd 0
%else
%macro KERNEL_LPK 1
    dd dual_%1 - dual_lp_fill_cols
%endmacro
%macro KERNEL_BENCH 1
    dd dual_%1 - dual_lp_fill_cols
%endmacro
%include 'reference/numerical/kernel_table.inc'
%unmacro KERNEL_LPK 1
%unmacro KERNEL_BENCH 1
%endif
    dd verify_lp_resid - verify_lp_resid
    dd elf_kernel_symbols - ELF_BASE

section .text follows=.header vfollows=.header align=16 valign=16
elf_text_start:
section .data follows=.text vfollows=.text align=4096 valign=4096
elf_data_start:
section .dynamic follows=.data vfollows=.data align=16 valign=16
elf_dynamic_start:
section .elf_bss nobits follows=.dynamic align=64
elf_bss_start:
%ifdef KERNEL_EQUIVALENT_ADDRESS
    resb 64
%endif

%macro extern 1
%endmacro
%macro section 1-*
    %ifidni %1, .bss
        [section .elf_bss]
    %else
        [section %1]
    %endif
%endmacro
%include 'ibt_unix.asm'

section .data
elf_data_primary_end:
section .elf_bss
elf_build_source: resq 1
elf_build_destination: resq 1
elf_bss_end:

section .text
elf_application_text_end:
; Keep the original numerical instruction-span budgets without external tools.
times ((lp_core_end - lp_isa_probe <= 16384) - 1) db 0
times ((dual_core_end - dual_lp_vzero <= 16384) - 1) db 0
times ((elf_application_text_end - lp_isa_probe <= 32768) - 1) db 0
; Imported functions use one indirect jump each; the dynamic loader fills libc
; slots and elf_load_gui resolves the optional X11 libraries on GUI startup.
%macro ELF_LIBC 1
%1: jmp [rel elf_got_%1]
%endmacro
%macro ELF_X11 1
    ELF_LIBC %1
%endmacro
%macro ELF_XFT 1
    ELF_LIBC %1
%endmacro
%include 'elf_imports.inc'
%unmacro ELF_LIBC 1
%unmacro ELF_X11 1
%unmacro ELF_XFT 1

elf_load_gui:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    lea rdi, [rel elf_x11_library]
    mov esi, 2
    call dlopen
    test rax, rax
    jz .fail
    mov rbx, rax
    lea rdi, [rel elf_xft_library]
    mov esi, 2
    call dlopen
    test rax, rax
    jz .fail
    mov r12, rax
%macro ELF_LIBC 1
%endmacro
%macro ELF_X11 1
    mov rdi, rbx
    lea rsi, [rel elf_name_%1]
    call dlsym
    test rax, rax
    jz .fail
    mov [rel elf_got_%1], rax
%endmacro
%macro ELF_XFT 1
    mov rdi, r12
    lea rsi, [rel elf_name_%1]
    call dlsym
    test rax, rax
    jz .fail
    mov [rel elf_got_%1], rax
%endmacro
%include 'elf_imports.inc'
%unmacro ELF_LIBC 1
%unmacro ELF_X11 1
%unmacro ELF_XFT 1
    mov eax, 1
    jmp .done
.fail:
    xor eax, eax
.done:
    pop r12
    pop rbx
    pop rbp
    ret
elf_report_error:
    mov eax, 1
    mov edi, 2
    syscall
    ret

align 4, db 0
elf_kernel_symbols:
    db 'KSY1'
    dd (elf_kernel_symbol_end - elf_kernel_symbol_entries) / 4
elf_kernel_symbol_entries:
%macro KERNEL_SYMBOL 1-2
%ifdef IBT_COMPACT_PIPELINE
    %if %0 = 2
        dd 0
        %exitmacro
    %endif
%endif
    dd %1 - ELF_BASE
%endmacro
%include 'native/kernel_symbol_table.inc'
%unmacro KERNEL_SYMBOL 1-2
elf_kernel_symbol_end:
elf_text_end:

section .dynamic
elf_dynamic:
    dq 1, elf_libc_name - elf_strings
    dq 1, elf_pthread_name - elf_strings
    dq 1, elf_libdl_name - elf_strings
    dq 4, elf_hash
    dq 5, elf_strings
    dq 6, elf_symbols
    dq 7, elf_relocations
    dq 8, elf_relocations_end - elf_relocations
    dq 9, 24
    dq 10, elf_strings_end - elf_strings
    dq 11, 24
    dq 0, 0
elf_dynamic_end:

align 8, db 0
%macro ELF_LIBC 1
elf_got_%1: dq 0
%endmacro
%macro ELF_X11 1
    ELF_LIBC %1
%endmacro
%macro ELF_XFT 1
    ELF_LIBC %1
%endmacro
%include 'elf_imports.inc'
%unmacro ELF_LIBC 1
%unmacro ELF_X11 1
%unmacro ELF_XFT 1

align 8, db 0
elf_symbols:
    times 24 db 0
%assign elf_symbol_index 1
%macro ELF_LIBC 1
elf_index_%1 equ elf_symbol_index
    dd elf_name_%1 - elf_strings
    db 0x12, 0
    dw 0
    dq 0, 0
%assign elf_symbol_index elf_symbol_index + 1
%endmacro
%macro ELF_X11 1
%endmacro
%macro ELF_XFT 1
%endmacro
%include 'elf_imports.inc'
%unmacro ELF_LIBC 1
%unmacro ELF_X11 1
%unmacro ELF_XFT 1
elf_symbol_count equ elf_symbol_index

align 8, db 0
elf_relocations:
%macro ELF_LIBC 1
    dq elf_got_%1, (elf_index_%1 * 0x100000000) + 6, 0
%endmacro
%macro ELF_X11 1
%endmacro
%macro ELF_XFT 1
%endmacro
%include 'elf_imports.inc'
%unmacro ELF_LIBC 1
%unmacro ELF_X11 1
%unmacro ELF_XFT 1
elf_relocations_end:

align 4, db 0
elf_hash:
    dd 1, elf_symbol_count, 0
    times elf_symbol_count dd 0

elf_strings:
    db 0
elf_libc_name: db 'libc.so.6', 0
elf_pthread_name: db 'libpthread.so.0', 0
elf_libdl_name: db 'libdl.so.2', 0
elf_build_test_option: db '--build-test', 0
elf_prepare_test_option: db '--prepare-test', 0
elf_gui_test_option: db '--gui-test', 0
elf_gui_resources_error: db 'GUI test: font, drawing context or image unavailable', 10
elf_gui_resources_error_end:
elf_gui_display_error: db 'GUI test: cannot open X11 display', 10
elf_gui_display_error_end:
elf_gui_import_error: db 'Cannot load the X11 or Xft libraries or their functions', 10
elf_gui_import_error_end:
elf_x11_library: db 'libX11.so.6', 0
elf_xft_library: db 'libXft.so.2', 0
%macro ELF_LIBC 1
elf_name_%1:
    %defstr %%name %1
    db %%name, 0
%endmacro
%macro ELF_X11 1
    ELF_LIBC %1
%endmacro
%macro ELF_XFT 1
    ELF_LIBC %1
%endmacro
%include 'elf_imports.inc'
%unmacro ELF_LIBC 1
%unmacro ELF_X11 1
%unmacro ELF_XFT 1
elf_strings_end:
elf_dynamic_data_end:

elf_data_file_size equ ((elf_data_primary_end - elf_data_start + 15) & ~15) + elf_dynamic_data_end - elf_dynamic_start
elf_data_mem_size equ ((elf_data_file_size + 63) & ~63) + elf_bss_end - elf_bss_start
%ifdef APP_FILE_LIMIT
elf_file_size equ ((0x300 + elf_text_end - elf_text_start + 0xfff) & ~0xfff) + elf_data_file_size
times ((elf_file_size <= APP_FILE_LIMIT) - 1) db 0
%endif
