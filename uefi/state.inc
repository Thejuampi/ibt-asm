section .bss
alignb 64
bootServices    resq 1
runtimeServices resq 1
conIn           resq 1
conOut          resq 1
mpServices      resq 1
stopEvent       resq 1
timerXmask      resq 1
processorTotal  resq 1
processorEnabled resq 1
bspNumber       resq 1
processorInfo   resb 64         ; room for PI extended processor information
cpuNumbers      resq MAX_CPUS+1
cpuApicIds      resd MAX_CPUS+1
alignb 8
cpuTls          resq MAX_CPUS+1
scratchPages    resq MAX_CPUS+1
mapSize         resq 1
mapKey          resq 1
mapStride       resq 1
mapVersion      resd 1
alignb 8
freeBytes       resq 1
allocatedPages  resq 1
largestBytes    resq 1
tscFrequency    resq 1
keyEvent        resq 1
keyIndex        resq 1
inputKey        resb 4
cpuBrand        resb 64
asciiNumber     resb 40
wideText        resw 256
editValue       resd 1
maxStressMB     resd 1
chosenThreads   resd 1
apCount         resd 1
commonIsa       resd 1
timingEstimated resd 1
testActive      resd 1
stopRequested   resd 1
dispatchFailed  resd 1
passCompleted   resd 1
allocCountdown  resd 1
qaCounter       resd 1
qaCases         resd 1
uiMode          resd 1
uiBusy          resd 1
uiStage         resd 1
uiErrorKind     resd 1
uiOriginX       resd 1
uiOriginY       resd 1
uiResultCount   resd 1
uiEditError     resd 1
uiAwaitKey      resd 1
uiKeySerial     resd 1
uiStartTick     resq 1
uiElapsed       resq 1
uiTickSecond    resq 1
uiTickStage     resd 1
uiTickStop      resd 1
uiColumns       resq 1
uiRows          resq 1
uiResults       resb UI_HISTORY*128
uiBar           resb 33
alignb 64
apReady         resd 1
alignb 64
apFinished      resd 1

; State expected by the unchanged numerical includes.
hMain          resq 1
hLblBits       resq 1
running        resd 1
xtreme         resd 1
haveResid      resd 1
failed         resd 1
passCount      resd 1
targetRuns     resd 1
stressMB       resd 1
availMB        resd 1
nproc          resd 1
lpPcores       resd 1
lpAllocFail    resd 1
resultIndex    resd 1
firstResid     resb 128
lineAcc        resb 128
lpA            resq 1
lpA0           resq 1
lpA1           resq 1
lpX            resq 1
lpX0           resq 1
lpX1           resq 1
lpResidX       resq 1
lpB            resq 1
lpIpiv         resq 1
lpWrk          resq 1
lpPackA        resq 1
lpPackB        resq 1
lpN            resd 1
lpLDA          resd 1
lpNthr         resd 1
lpStop         resd 1
lpT0           resd 1
lpTJ           resd 1
lpTNB          resd 1
lpSwN          resd 1
lpTest         resd 1
lpIsa          resd 1
lpIsaUse       resd 1
lpIsaForce     resd 1
lpTlsIdx       resd 1
alignb 64
lpDie          resd 1
alignb 64
lpPhase        resd 1
alignb 64
lpDone         resd 1
alignb 64
lpNext         resd 1
alignb 64
lpHere         resd 1
alignb 64
lpJob          resd 1
alignb 64
lpCurBuf       resd 1
lpNBuf         resd 1
alignb 8
lpNorm0        resq 1
lpTmpR         resq 1
lpTmpN         resq 1
lpTmpT         resq 1
lpGM           resd 1
lpGN           resd 1
lpGK           resd 1
alignb 8
lpGA           resq 1
lpGB           resq 1
lpGC           resq 1
lpGLDA         resq 1
lpFreq         resq 1
lpAccP         resq 1
lpAccT         resq 1
lpAccG         resq 1
lpAccS         resq 1
lpQ0           resq 1
lpWorkers      resq 32
alignb 16
memoryMap      resb MAP_BYTES
alignb 64
timerXstate    resb 65536

section .data
align 8
relocationAnchor dq efi_main    ; retain a PE base-relocation directory
mpGuid dd 03fdda605h
       dw 0a76eh, 04f46h
       db 0adh,029h,012h,0f4h,053h,01bh,03dh,008h
defaultMxcsr dd 01f80h
szHeader        db 'Intel Burn Test 3.2',0
szRule          db '--------------------------------------------------------------',13,10,0
szCpu           db 'CPU: ',0
szMemory        db 'Matrix: ',0
szMemorySuffix  db ' MiB   |   Maximum available: ',0
szRuns          db 'Runs: ',0
szThreads       db '   |   Compute threads: ',0
szThreadMax     db ' / ',0
szMode          db '   |   ISA: ',0
szAuto          db 'Auto -> ',0
szLimited       db 'Limit -> ',0
szBitsSse2      db 'SSE2',0
szBitsAvx       db 'AVX',0
szBitsAvx2      db 'AVX2 + FMA',0
szUnsupported   db 'Unable to initialize the CPU execution state.',13,10,0
szMemoryError   db 'Unable to read a usable UEFI memory map.',13,10,0
szMemoryPrompt  db 'Matrix MiB (1 to available maximum): ',0
szRunsPrompt    db 'Runs (2 to 100000): ',0
szThreadPrompt  db 'Compute threads (1 to displayed maximum): ',0
szEditError     db 'Value outside the allowed range.',13,10,0
szPreparing     db 'Preparing test data...',13,10,0
szRunHeader     db 'Run       Seconds       GFLOPS         Normalized residual',13,10,0
szActive        db 'Escape stops the test. Firmware can defer input until a pass ends.',13,10,0
szPass          db 'PASSED: all requested runs completed with identical residuals.',13,10,0
szFail          db 'FAILED: numerical error or residual mismatch.',13,10,0
szStopped       db 'STOPPED: incomplete test; no stability verdict.',13,10,0
szAllocError    db 'ALLOCATION FAILED: reduce the memory or thread count.',13,10,0
szDispatchError db 'MULTIPROCESSOR ERROR: select one thread to test on the BSP.',13,10,0
szSpaces        db '    ',0
szSlash         db '/',0
szNewline       db 13,10,0
szBackspace     db 8,' ',8,0
szQaOk          db 'IBT_QA_OK',13,10,0
szQaFail        db 'IBT_QA_FAIL',13,10,0
szQaUnsupported db 'IBT_QA_UNSUPPORTED',13,10,0
szQaCase        db 'QA matrix N=',0
szQaWorkers     db ' threads=',0
szQaTotal       db 'QA cases completed: ',0
szQaNorm        db ' residual=',0
qaSizes         dd 8,24,40,68,72,96,256,384,400,768,1024,0

; User-facing console copy. Platform/build details belong in the README.
uiRule          times UI_WIDTH db '-'
                db 0
uiBlank         times UI_WIDTH db ' '
                db 0
uiSizeLabel     db '[M] Size',0
uiThreadsLabel  db '[T] Threads',0
uiRunsLabel     db '[R] Runs',0
uiResetLabel    db '[D] Reset',0
uiSizeLocked    db 'Size',0
uiThreadsLocked db 'Threads',0
uiRunsLocked    db 'Runs',0
uiMiB           db ' MiB',0
uiAvailable     db 'Available: ',0
uiModeLabel     db 'Mode: ',0
uiAutoOpen      db 'Auto (',0
uiClose         db ')',0
uiReady         db 'Ready',0
uiReadyHint     db 'Choose settings, then press Enter to start.',0
uiStart         db '[Enter] Start test',0
uiSecondary     db '[F2] Advanced    [H] Help    [Q] Exit',0
uiActions       db '[Enter] Rerun    [C] Change settings    [Q] Exit',0
uiResultHelp    db '[H] Help',0
uiStop          db '[Esc] Stop',0
uiStopping      db 'Stopping... Please wait.',0
uiRunning       db 'Running ',0
uiOf            db ' of ',0
uiElapsedLabel  db '    Elapsed ',0
uiElapsedApprox db '    ~Elapsed ',0
uiStageSep      db '    |    ',0
uiStagePrepare  db 'Preparing',0
uiStageFill     db 'Building matrix',0
uiStageFactor   db 'Stressing CPU',0
uiStageSolve    db 'Solving',0
uiStageVerify   db 'Checking results',0
uiComplete      db ' runs completed',0
uiPassed        db 'PASSED',0
uiFailed        db 'FAILED',0
uiStopped       db 'STOPPED',0
uiCouldNotStart db 'COULD NOT START',0
uiMatched       db ' runs completed. All results match.',0
uiIncomplete    db ' runs completed. Test incomplete.',0
uiMismatchHint  db 'Results did not match. Review CPU and memory settings.',0
uiInvalidHint   db 'A calculation returned an invalid result.',0
uiAllocHint     db 'Not enough memory. Reduce Size or Threads and try again.',0
uiDispatchHint  db 'Threads could not start. Try again with Threads set to 1.',0
uiStopHint      db 'The test was stopped before all runs finished.',0
uiTableRun      db 'Run',0
uiTableTime     db 'Time (s)',0
uiTableTimeEst  db '~Time (s)',0
uiTableGflops   db 'GFLOPS',0
uiTableGflopsEst db '~GFLOPS',0
uiTableResidual db 'Residual',0
uiTableResult   db 'Result',0
uiNoResults     db 'No runs yet.',0
uiRowOk         db 'OK',0
uiRowMismatch   db 'Mismatch',0
uiRowInvalid    db 'Invalid',0
uiShowing       db 'Showing runs ',0
uiTo            db ' - ',0
uiLatest        db ' (latest 8)',0
uiEstimatedHint db '~ marks estimated timing and performance.',0
uiEditSize      db 'Size in MiB',0
uiEditThreads   db 'Threads',0
uiEditRuns      db 'Runs',0
uiRangeOpen     db ' (',0
uiRangeTo       db ' to ',0
uiRangeClose    db ')    Current: ',0
uiNewValue      db 'New value: ',0
uiEditActions   db '[Enter] Apply / keep current    [Esc] Cancel',0
uiEditMax       db '    [A] Maximum',0
uiRangeError    db 'Enter a value in the range shown below.',0
uiAdvancedTitle db 'Advanced - instruction mode',0
uiAdvancedHint  db 'Auto chooses the strongest mode available on this CPU.',0
uiAdvancedAuto  db '[A] Auto (recommended)',0
uiAdvancedSse2  db '[1] SSE2',0
uiAdvancedAvx   db '[2] AVX',0
uiAdvancedAvx2  db '[3] AVX2 + FMA',0
uiUnavailable   db 'This mode is not available on this CPU.',0
uiBack          db '[Enter / Esc] Back',0
uiHelpTitle     db 'Help',0
uiHelpSize      db 'Size: memory for the matrix. The test also needs some working space.',0
uiHelpThreads   db 'Threads: use the maximum to keep the available CPUs busy.',0
uiHelpRuns      db 'Runs: repeated calculations check that results stay consistent.',0
uiHelpTime      db 'Time / GFLOPS: compute time and speed for each completed run.',0
uiHelpResidual  db 'Residual: solution error. Every completed run must match.',0
uiHelpPassed    db 'PASSED means all requested runs finished with matching results.',0
uiHelpStopped   db 'STOPPED means the test is incomplete; it does not count as a pass.',0
uiHelpStopDelay db 'Stopping may take until the current run finishes.',0
uiHelpCooling   db 'Use adequate cooling for sustained CPU load.',0
