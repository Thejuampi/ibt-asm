; Firmware bridges. APs execute arithmetic only: no allocation, console, or
; event services on an AP. Blocking MP dispatch remains legal after ReadyToBoot.

cpu_initialize:
    ; RCX -> saved CR0, CR4, XCR0, XCR0-valid (public entry's local context).
    push rbx
    push rsi
    mov rsi, rcx
    mov rax, cr0
    mov [rsi], rax
    and rax, ~12                 ; clear EM and TS
    or rax, 2
    mov cr0, rax
    mov rax, cr4
    mov [rsi+8], rax
    or rax, (1<<9)|(1<<10)
    mov cr4, rax
    mov eax, 1
    cpuid
    and ecx, (1<<26)|(1<<28)
    cmp ecx, (1<<26)|(1<<28)
    jne .sse2
    mov rax, cr4
    or rax, 1<<18
    mov cr4, rax
    xor ecx, ecx
    xgetbv
    mov [rsi+16], eax
    mov [rsi+20], edx
    mov qword [rsi+24], 1
    or eax, 7                    ; x87 + SSE + AVX, retaining other components
    xsetbv
.sse2:
    ldmxcsr [defaultMxcsr]
    pop rsi
    pop rbx
    ret

cpu_restore:
    cmp qword [rcx+24], 0
    je .control
    mov r8, rcx
    mov eax, [r8+16]
    mov edx, [r8+20]
    xor ecx, ecx
    xsetbv
    mov rcx, r8
.control:
    ; A zero context denotes an entry which never changed CPU control state.
    cmp qword [rcx], 0
    je .out
    mov rax, [rcx+8]
    mov cr4, rax
    mov rax, [rcx]
    mov cr0, rax
.out:
    ret

cpu_apic_id:
    push rbx
    mov eax, 0
    cpuid
    cmp eax, 0bh
    jb .extended
    mov eax, 0bh
    xor ecx, ecx
    cpuid
    test ebx, ebx
    jz .extended
    mov eax, edx
    jmp .out
.extended:
    mov eax, 80000000h
    cpuid
    cmp eax, 8000001eh
    jb .legacy
    mov eax, 80000001h
    cpuid
    test ecx, 1<<22              ; AMD topology extensions
    jz .legacy
    mov eax, 8000001eh
    cpuid
    jmp .out
.legacy:
    mov eax, 1
    cpuid
    mov eax, ebx
    shr eax, 24
.out:
    pop rbx
    ret

current_slot:
    call cpu_apic_id
    xor edx, edx
    lea r8, [cpuApicIds]
.find:
    cmp eax, [r8+rdx*4]
    je .found
    inc edx
    cmp edx, [apCount]
    jbe .find
    mov eax, -1
    ret
.found:
    mov eax, edx
    ret

processors_init:
    push rbx
    push rsi
    FRAME 96
    mov dword [nproc], 1
    call cpu_apic_id
    mov [cpuApicIds], eax
    lea rcx, [mpGuid]
    xor edx, edx
    lea r8, [mpServices]
    mov rax, [bootServices]
    call [rax+BS_LOCATE]
    test rax, rax
    js .out
    mov rcx, [mpServices]
    lea rdx, [processorTotal]
    lea r8, [processorEnabled]
    call [rcx+MP_COUNT]
    test rax, rax
    js .no_mp
    cmp qword [processorTotal], 4096
    ja .no_mp
    mov rcx, [mpServices]
    lea rdx, [bspNumber]
    call [rcx+MP_WHOAMI]
    test rax, rax
    js .no_mp
    mov rax, [bspNumber]
    mov [cpuNumbers], rax
    xor ebx, ebx
    xor esi, esi
.enumerate:
    cmp rbx, [processorTotal]
    jae .counted
    cmp ebx, [bspNumber]
    je .next
    mov rcx, [mpServices]
    mov rdx, rbx
    lea r8, [processorInfo]
    call [rcx+MP_INFO]
    test rax, rax
    js .next
    mov eax, [processorInfo+8]
    and eax, 6
    cmp eax, 6
    jne .next
    inc esi
    lea rdx, [cpuNumbers]
    mov [rdx+rsi*8], rbx
    lea rdx, [cpuApicIds]
    mov eax, [processorInfo]
    mov [rdx+rsi*4], eax
    cmp esi, MAX_CPUS
    jae .counted
.next:
    inc ebx
    jmp .enumerate
.counted:
    mov [apCount], esi
    cmp esi, 2
    jb .out
    mov [nproc], esi
    mov rcx, [mpServices]
    lea rdx, [ap_probe]
    xor r8d, r8d
    xor r9d, r9d
    mov qword [rsp+32], 0
    mov qword [rsp+40], 0
    mov qword [rsp+48], 0
    call [rcx+MP_START_ALL]
    test rax, rax
    jns .out
.no_mp:
    mov dword [nproc], 1
.out:
    leave
    pop rsi
    pop rbx
    ret

ap_probe:
    EFI_ENTER
    mov qword [rsp+576], 0
    call current_slot
    cmp eax, 1
    jl .done
    mov edi, 1
    mov eax, 1
    cpuid
    mov esi, ecx
    and ecx, (1<<26)|(1<<28)
    cmp ecx, (1<<26)|(1<<28)
    jne .merge
    or edi, 2
    test esi, 1<<12
    jz .no_fma
    or edi, 8
.no_fma:
    xor eax, eax
    cpuid
    cmp eax, 7
    jb .merge
    mov eax, 7
    xor ecx, ecx
    cpuid
    test ebx, 1<<5
    jz .merge
    or edi, 4
.merge:
    lock and [commonIsa], edi
.done:
    mov qword [rsp+608], 0
    EFI_RETURN

select_isa:
    FRAME 32
    call lp_isa_probe
    mov eax, [commonIsa]
    and [lpIsa], eax
    xor edx, edx
    test dword [lpIsa], 2
    jz .cap
    mov edx, 1
    mov eax, [lpIsa]
    and eax, 12
    cmp eax, 12
    jne .cap
    mov edx, 2
.cap:
    mov eax, [lpIsaForce]
    test eax, eax
    jz .store
    dec eax
    cmp eax, edx
    cmovb edx, eax
.store:
    mov [lpIsaUse], edx
    RETURN

timer_init:
    push rbx
    FRAME 64
    test dword [lpIsa], 2
    jz .create
    mov eax, 0dh
    xor ecx, ecx
    cpuid
    cmp ebx, 65536
    ja .failed
.create:
    mov ecx, 080000200h           ; EVT_TIMER | EVT_NOTIFY_SIGNAL
    mov edx, 8                   ; TPL_CALLBACK
    lea r8, [input_timer]
    xor r9d, r9d
    lea rax, [stopEvent]
    mov [rsp+32], rax
    mov rax, [bootServices]
    call [rax+BS_CREATE_EVENT]
    test rax, rax
    js .failed
    mov rcx, [stopEvent]
    mov edx, 1                   ; TimerPeriodic
    mov r8d, 1000000             ; 100 ms in 100 ns units
    mov rax, [bootServices]
    call [rax+BS_SET_TIMER]
    test rax, rax
    jns .out
    mov rcx, [stopEvent]
    mov rax, [bootServices]
    call [rax+BS_CLOSE_EVENT]
.failed:
    mov qword [stopEvent], 0
.out:
    leave
    pop rbx
    ret

input_timer:
    EFI_ENTER
    mov qword [rsp+576], 0
    cmp dword [testActive], 0
    je .done
    test dword [lpIsa], 2
    jz .poll
    xor ecx, ecx
    xgetbv
    mov [timerXmask], eax
    mov [timerXmask+4], edx
    xsave64 [timerXstate]
.poll:
    call poll_stop
%if !UEFI_QA
    call ui_tick
%endif
    test dword [lpIsa], 2
    jz .done
    mov eax, [timerXmask]
    mov edx, [timerXmask+4]
    xrstor64 [timerXstate]
.done:
    mov qword [rsp+608], 0
    EFI_RETURN

poll_stop:
    FRAME 32
    mov rcx, [conIn]
    test rcx, rcx
    jz .out
    lea rdx, [inputKey]
    call [rcx+IN_READ]
    test rax, rax
    jnz .out
    cmp word [inputKey], SCAN_ESC
    je .stop
    cmp word [inputKey+2], 27
    jne .out
.stop:
    mov dword [stopRequested], 1
    mov dword [lpStop], 1
.out:
    RETURN

clock_init:
    push rbx
    FRAME 48
    mov dword [timingEstimated], 1
    mov eax, 80000000h
    cpuid
    cmp eax, 80000007h
    jb .frequency
    mov eax, 80000007h
    cpuid
    test edx, 1<<8
    jz .frequency
    mov dword [timingEstimated], 0
.frequency:
    xor eax, eax
    cpuid
    cmp eax, 15h
    jb .calibrate
    mov eax, 15h
    xor ecx, ecx
    cpuid
    test eax, eax
    jz .calibrate
    test ebx, ebx
    jz .calibrate
    test ecx, ecx
    jz .calibrate
    mov r8d, eax
    mov eax, ebx
    imul rax, rcx
    xor edx, edx
    div r8
    test rax, rax
    jnz .store
.calibrate:
    lfence
    rdtsc
    shl rdx, 32
    or rax, rdx
    mov [rsp+32], rax
    mov ecx, 100000
    mov rax, [bootServices]
    call [rax+BS_STALL]
    lfence
    rdtsc
    shl rdx, 32
    or rax, rdx
    sub rax, [rsp+32]
    imul rax, 10
.store:
    cmp rax, 1
    jae .valid
    mov eax, 1
.valid:
    mov [tscFrequency], rax
    mov [lpFreq], rax
    leave
    pop rbx
    ret

QueryPerformanceFrequency:
    mov rax, [tscFrequency]
    mov [rcx], rax
    mov eax, 1
    ret

QueryPerformanceCounter:
%if UEFI_QA && UEFI_QA_CASE = 4
    inc dword [qaCounter]
    cmp dword [qaCounter], 3
    jne .read
    mov dword [stopRequested], 1
    mov dword [lpStop], 1
.read:
%endif
    lfence
    rdtsc
    shl rdx, 32
    or rax, rdx
    mov [rcx], rax
    mov eax, 1
    ret

VirtualAlloc:
    push rdi
    FRAME 64
    mov [rsp+40], rdx
    call current_slot
    test eax, eax              ; firmware memory services are BSP-only
    jnz .fail
%if UEFI_QA
    cmp dword [allocCountdown], 0
    je .allocate
    dec dword [allocCountdown]
    jz .fail
.allocate:
%endif
    mov r8, [rsp+40]
    add r8, 4095
    jc .fail
    shr r8, 12
    inc r8                     ; a private metadata page keeps payload aligned
    mov [rsp+48], r8
    mov qword [rsp+32], 0
    xor ecx, ecx               ; AllocateAnyPages
    mov edx, 2                 ; EfiLoaderData
    lea r9, [rsp+32]
    mov rax, [bootServices]
    call [rax+BS_ALLOC_PAGES]
    test rax, rax
    js .fail
    mov rdi, [rsp+32]
    mov rcx, [rsp+48]
    shl rcx, 9
    xor eax, eax
    rep stosq                  ; preserve VirtualAlloc's zero-filled contract
    mov rax, [rsp+32]
    mov rdx, [rsp+48]
    mov [rax], rdx
    add [allocatedPages], rdx
    add rax, 4096
    leave
    pop rdi
    ret
.fail:
    mov dword [lpAllocFail], 1
    mov dword [lpStop], 1
    xor eax, eax
    leave
    pop rdi
    ret

VirtualFree:
    test rcx, rcx
    jz .ok
    cmp qword [rcx-4096+8], 0
    jne .ok                    ; AP scratch is released after MP joins, by BSP
    jmp pages_release
.ok:
    mov eax, 1
    ret

pages_release:
    FRAME 48
    sub rcx, 4096
    mov rdx, [rcx]
    mov [rsp+32], rdx
    mov rax, [bootServices]
    call [rax+BS_FREE_PAGES]
    test rax, rax
    jnz .failed
    mov rdx, [rsp+32]
    sub [allocatedPages], rdx
    mov eax, 1
    RETURN
.failed:
    mov dword [failed], 1
    xor eax, eax
    RETURN

TlsAlloc:
    xor eax, eax
    ret
TlsGetValue:
    FRAME 32
    call current_slot
    test eax, eax
    js .none
    lea rdx, [cpuTls]
    mov rax, [rdx+rax*8]
    RETURN
.none:
    xor eax, eax
    RETURN
TlsSetValue:
    FRAME 48
    mov [rsp+32], rdx
    call current_slot
    test eax, eax
    js .none
    lea rdx, [cpuTls]
    mov rcx, [rsp+32]
    mov [rdx+rax*8], rcx
    mov eax, 1
    RETURN
.none:
    xor eax, eax
    RETURN

; Unused Win32 orchestration symbols retained by the shared engine.
; The UEFI frontend dispatches with blocking MP Services instead of threads.
CreateThread:
GetLogicalProcessorInformationEx:
    xor eax, eax
    ret
WaitForSingleObject:
CloseHandle:
SetThreadPriority:
SetWindowTextW:
SendMessageW:
PostMessageW:
WakeByAddressAll:
title_status:
title_progress:
    mov eax, 1
    ret
thread_count:
    mov eax, [chosenThreads]
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
    xor edx, edx
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

memory_update:
    push rbx
    push rsi
    push rdi
    FRAME 64
    mov qword [mapSize], MAP_BYTES
    lea rcx, [mapSize]
    lea rdx, [memoryMap]
    lea r8, [mapKey]
    lea r9, [mapStride]
    lea rax, [mapVersion]
    mov [rsp+32], rax
    mov rax, [bootServices]
    call [rax+BS_MEMORY_MAP]
    test rax, rax
    js .fail
    cmp qword [mapStride], 40
    jb .fail
    mov qword [freeBytes], 0
    mov qword [largestBytes], 0
    lea rsi, [memoryMap]
    mov rdi, rsi
    add rdi, [mapSize]
.range:
    mov rax, rsi
    add rax, [mapStride]
    cmp rax, rdi
    ja .summed
    cmp dword [rsi], 7         ; EfiConventionalMemory, never reserved/MMIO
    jne .next
    mov rax, [rsi+24]
    shl rax, 12
    add [freeBytes], rax
    cmp rax, [largestBytes]
    jbe .next
    mov [largestBytes], rax
.next:
    add rsi, [mapStride]
    jmp .range
.summed:
    mov rax, [freeBytes]
    shr rax, 20
    mov [availMB], eax
    xor ebx, ebx
    mov edi, 65536             ; cap UI input and arithmetic to 64 GiB matrices
    cmp rax, rdi
    cmovb edi, eax
.search:
    cmp ebx, edi
    jae .done
    lea esi, [rbx+rdi+1]
    shr esi, 1
    mov ecx, esi
    call memory_budget_ok
    test eax, eax
    jz .too_large
    mov ebx, esi
    jmp .search
.too_large:
    lea edi, [rsi-1]
    jmp .search
.done:
    mov [maxStressMB], ebx
    mov eax, ebx
    leave
    pop rdi
    pop rsi
    pop rbx
    ret
.fail:
    mov dword [maxStressMB], 0
    xor eax, eax
    leave
    pop rdi
    pop rsi
    pop rbx
    ret

memory_budget_ok:
    FRAME 32
    shl rcx, 17
    call isqrt
    and eax, ~7
    mov r8, rax               ; N
    lea rdx, [rax+8]
    imul rax, rdx
    shl rax, 3                ; exact padded matrix bytes
    add rax, 8192             ; page rounding + allocation header
    cmp rax, [largestBytes]
    ja .no
    lea rdx, [r8+16]
    imul rdx, 384*8*2         ; both panel packing buffers
    add rax, rdx
    imul r8, 28               ; three double vectors + pivot vector
    add rax, r8
    mov edx, [nproc]
    inc edx
    imul rdx, SCRATCH_BYTES+8192
    add rax, rdx
    add rax, 32*1024*1024+65536 ; firmware headroom and allocation metadata
    cmp rax, [freeBytes]
    ja .no
    mov eax, 1
    RETURN
.no:
    xor eax, eax
    RETURN
