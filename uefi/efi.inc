; x64 offsets from UEFI 2.11 and PI 1.9. All firmware calls use EFIAPI.
ST_CONIN          equ 48
ST_CONOUT         equ 64
ST_RUNTIME        equ 88
ST_BOOT           equ 96
BS_ALLOC_PAGES    equ 40
BS_FREE_PAGES     equ 48
BS_MEMORY_MAP     equ 56
BS_CREATE_EVENT   equ 80
BS_SET_TIMER      equ 88
BS_WAIT_EVENT     equ 96
BS_CLOSE_EVENT    equ 112
BS_STALL          equ 248
BS_WATCHDOG       equ 256
BS_LOCATE         equ 320
RT_RESET          equ 104
MP_COUNT          equ 0
MP_INFO           equ 8
MP_START_ALL      equ 16
MP_WHOAMI         equ 48
IN_READ           equ 8
IN_WAIT           equ 16
SCAN_ESC          equ 017h
SCAN_F1           equ 00bh
SCAN_F2           equ 00ch
OUT_STRING        equ 8
OUT_QUERY_MODE    equ 24
OUT_SET_MODE      equ 32
OUT_ATTRIBUTE     equ 40
OUT_CLEAR         equ 48
OUT_POSITION      equ 56
OUT_CURSOR        equ 64
OUT_MODE          equ 72
MAP_BYTES         equ 131072
MAX_CPUS          equ 32
SCRATCH_BYTES     equ (48/4)*(32*384)+(24*384*8)+256
WM_PHASE          equ 08004h
WM_DONE           equ 08002h
UI_READY          equ 0
UI_RUNNING        equ 1
UI_RESULT         equ 2
UI_EDIT           equ 3
UI_OVERLAY        equ 4
UI_HISTORY        equ 8
UI_WIDTH          equ 76
UI_HEIGHT         equ 24
UI_NORMAL         equ 7
UI_MUTED          equ 7
UI_BRIGHT         equ 15
UI_ACCENT         equ 11
UI_SUCCESS        equ 10
UI_ERROR          equ 12
UI_WARNING        equ 14

; The shared private ABI does not always align RSP. Realign at each bridge.
%macro FRAME 1
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, %1
%endmacro
%macro RETURN 0
    leave
    ret
%endmacro

; Public firmware entry points also preserve the complete legacy FP state,
; integer nonvolatiles, CR0/CR4, and the original XCR0. Context at RSP+576.
%macro EFI_ENTER 0
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
    sub rsp, 640
    fxsave64 [rsp+64]
    mov qword [rsp+600], 0
%endmacro
%macro EFI_RETURN 0
    lea rcx, [rsp+576]
    call cpu_restore
    fxrstor64 [rsp+64]
    mov rax, [rsp+608]
    lea rsp, [rbp-56]
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
%endmacro
