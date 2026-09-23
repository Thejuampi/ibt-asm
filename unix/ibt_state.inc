section .bss
align 64
xDisplay     resq 1
xWindow      resq 1
xGC          resq 1
xDelete      resq 1
xEvent       resb 192
xHints       resb 80
xScreen      resd 1
xDepth       resd 1
alignb 8
xVisual      resq 1
xColormap    resq 1
xftDraw      resq 1
xftFont      resq 1
xftFontBold  resq 1
xftFontLarge resq 1
xftColors    resb 16*7
flameImage   resq 1
coffeeImage  resq 1
flameFrame   resd 1
flameTick    resd 1
alignb 16
flamePixels  resd 424*50
coffeePixels resd 85*23

uiDirty      resd 1
uiDone       resd 1
uiQuit       resd 1
uiFocus      resd 1
uiAbout      resd 1
uiSmoke      resd 1
uiDrop       resd 1
stressChoice resd 1
threadChoice resd 1
timesValue   resd 1
customMB     resd 1
freeMB       resd 1
logEnabled   resd 1
logFd        resd 1
keyBuf       resb 16
dynBuf       resb 96

; State shared with the original numerical engine.
hMain       resq 1
hLblBits    resq 1
hProc       resq 1
running     resd 1
xtreme      resd 1
haveResid   resd 1
failed      resd 1
passCount   resd 1
targetRuns  resd 1
stressMB    resd 1
availMB     resd 1
nproc       resd 1
lpPcores    resd 1
lpAllocFail resd 1
resultState resd 1
resultIndex resd 1
firstResid  resb 128
lineAcc     resb 128
resultTime  resb 80
resultSpeed resb 80
resultResid resb 80

lpA         resq 1
lpA0        resq 1
lpA1        resq 1
lpX         resq 1
lpX0        resq 1
lpX1        resq 1
lpResidX    resq 1
lpB         resq 1
lpIpiv      resq 1
lpWrk       resq 1
lpPackA     resq 1
lpPackB     resq 1
lpN         resd 1
lpLDA       resd 1
lpNthr      resd 1
lpStop      resd 1
lpT0        resd 1
lpTJ        resd 1
lpTNB       resd 1
lpSwN       resd 1
lpTest      resd 1
lpIsa       resd 1
lpIsaUse    resd 1
lpIsaForce  resd 1
lpTlsIdx    resd 1
alignb 64
lpDie       resd 1
alignb 64
lpPhase     resd 1
alignb 64
lpDone      resd 1
alignb 64
lpNext      resd 1
alignb 64
lpHere      resd 1
alignb 64
lpJob       resd 1
alignb 64
lpCurBuf    resd 1
lpNBuf      resd 1
alignb 8
lpNorm0     resq 1
lpTmpR      resq 1
lpTmpN      resq 1
lpTmpT      resq 1
lpGM        resd 1
lpGN        resd 1
lpGK        resd 1
alignb 8
lpGA        resq 1
lpGB        resq 1
lpGC        resq 1
lpGLDA      resq 1
lpFreq      resq 1
lpAccP      resq 1
lpAccT      resq 1
lpAccG      resq 1
lpAccS      resq 1
lpQ0        resq 1
lpWorkers   resq 32
lpSeen      resd 32
lpWparm     resd 32
tlsKey      resd 1

section .data
align 8
szTitle       db 'IntelBurnTest v3.00 - by Thejuampi',0
szFont        db 'Segoe UI,DejaVu Sans:size=9',0
szFontBold    db 'Segoe UI,DejaVu Sans:bold:size=9',0
szFontLarge   db 'Segoe UI,DejaVu Sans:bold:size=20',0
szDelete      db 'WM_DELETE_WINDOW',0
szSelftest    db '--selftest',0
szSmoketest   db '--smoketest',0
szCoffeeCmd   db 'xdg-open https://buymeacoffee.com/thejuampi >/dev/null 2>&1 &',0

szConfig      db 'TEST CONFIGURATION',0
szMode        db 'Mode:',0
szModeValue   db '64-bit',0
szModeAvx     db '64-bit AVX',0
szModeAvx2    db '64-bit AVX2',0
szStress      db 'Stress:',0
szMB          db 'MB',0
szTimes       db 'Times to run:',0
szThreads     db 'Threads:',0
szFree        db 'Free',0
szLog         db 'Output results to results.log',0
szThermal     db 'THERMAL LOAD',0
szStart       db 'Start',0
szRunAgain    db 'Run again',0
szStop        db 'Stop',0
szAbout       db 'About',0
szCoffee      db 'Coffee',0
szMonitor     db 'STABILITY MONITOR',0
szProgress    db 'RUN PROGRESS',0
szReady       db 'READY',0
szRunning     db 'RUNNING',0
szPassed      db 'PASSED',0
szFailed      db 'FAILED',0
szStopped     db 'STOPPED',0
szRunPrefix   db 'RUN ',0
szReadyStart  db 'READY TO START',0
szWarming     db 'WARMING UP',0
szGflops      db 'GFLOPS',0
szLastRun     db 'LAST RUN',0
szSignature   db 'STABILITY SIGNATURE',0
szAwaiting    db 'Waiting for the first completed run',0
szReference   db 'REFERENCE CAPTURED',0
szConsistent  db 'CONSISTENT',0
szAllMatch    db 'ALL RUNS MATCH',0
szMismatch    db 'RESIDUAL MISMATCH',0
szDash        db '--',0
szSeconds     db 's',0
szSlash       db ' / ',0

szStd         db 'Standard',0
szHigh        db 'High',0
szVeryHigh    db 'Very High',0
szMaximum     db 'Maximum',0
szCustom      db 'Custom',0
stressNames   dq szStd,szHigh,szVeryHigh,szMaximum,szCustom
stressSizes   dd 1024,2048,4096,0,0

szAll         db 'All',0
szAuto        db 'Auto',0
szOne         db '1',0
szTwo         db '2',0
szFour        db '4',0
szEight       db '8',0
szSixteen     db '16',0
szThirtyTwo   db '32',0
threadNames   dq szAll,szAuto,szOne,szTwo,szFour,szEight,szSixteen,szThirtyTwo
threadValues  dd 0,-1,1,2,4,8,16,32

szAboutHead   db 'IntelBurnTest',0
szAbout1      db 'by Thejuampi - original by AgentGOD',0
szAbout2      db 'Native x86-64 calculation core on Unix/X11.',0
szAbout3      db 'Adequate CPU cooling is required.',0
szOK          db 'OK',0

szBitsSse2    db '64-bit SSE2',0
szBitsAvx     db '64-bit AVX',0
szBitsAvx2    db '64-bit AVX2',0
szPreparing   db 'Preparing test data',0

logName       db 'results.log',0
logHeader     db 'IntelBurnTest Unix',10,'Time (s)    Speed (GFlops)    Result',10
logHeaderLen  equ $-logHeader

align 4
flameBmp:
    incbin "../res/flame4.bmp"
coffeeBmp:
    incbin "../res/coffee4.bmp"

; XRenderColor values corresponding to the seven UI text colors.
xftRenderColors:
    dw 0E8E8h,0EDEDh,0F4F4h,0FFFFh
    dw 08B8Bh,09797h,0A8A8h,0FFFFh
    dw 0FFFFh,07A7Ah,03333h,0FFFFh
    dw 00E0Eh,01111h,01616h,0FFFFh
    dw 04242h,0D3D3h,09292h,0FFFFh
    dw 0FFFFh,05D5Dh,06C6Ch,0FFFFh
    dw 0,0,0,0FFFFh
