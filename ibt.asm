bits 64
default rel

%ifndef SELFTEST
    %define SELFTEST 0
%endif

%include "ibt_macros.inc"

extern ExitProcess
extern GetModuleHandleW
extern GetModuleFileNameW
extern RegisterClassExW
extern CreateWindowExW
extern ShowWindow
extern UpdateWindow
extern GetMessageW
extern TranslateMessage
extern DispatchMessageW
extern DefWindowProcW
extern PostQuitMessage
extern PostMessageW
extern SendMessageW
extern LoadCursorW
extern LoadImageW
extern MessageBoxW
extern SetWindowTextW
extern GetWindowTextW
extern EnableWindow
extern SetTimer
extern KillTimer
extern DestroyWindow
extern GetSystemMetrics
extern wsprintfW
extern IsDialogMessageW
extern OpenClipboard
extern CloseClipboard
extern EmptyClipboard
extern SetClipboardData
extern TrackPopupMenu
extern CreatePopupMenu
extern AppendMenuW
extern DestroyMenu
extern GetSysColorBrush
extern SetBkMode
extern SetTextColor
extern CreateFontIndirectW
extern DeleteObject
extern SetWindowLongPtrW
extern InitCommonControlsEx
extern SetWindowSubclass
extern DefSubclassProc
extern RemoveWindowSubclass
extern WriteFile
extern CloseHandle
extern CreateFileW
extern CreateThread
extern VirtualAlloc
extern VirtualFree
extern QueryPerformanceCounter
extern QueryPerformanceFrequency
extern WaitForSingleObject
extern TlsAlloc
extern TlsGetValue
extern TlsSetValue
extern SetThreadPriority
extern GetCurrentThread
extern WakeByAddressAll
extern GetCommandLineW
extern GetProcessHeap
extern HeapAlloc
extern HeapFree
extern GetSystemInfo
extern GetLogicalProcessorInformationEx
extern GlobalMemoryStatusEx
extern GetTickCount64
extern lstrlenW
extern lstrcpyW
extern lstrcatW
extern MultiByteToWideChar
extern WideCharToMultiByte
extern GlobalAlloc
extern GlobalLock
extern GlobalUnlock
extern GetCursorPos
extern GetDlgCtrlID
extern InvalidateRect
extern AdjustWindowRectEx
extern SetWindowPos
extern RedrawWindow
extern SelectObject
extern DrawTextW
extern FillRect
extern BeginPaint
extern EndPaint
extern CreatePen
extern RoundRect
extern CreateRoundRectRgn
extern CreateCompatibleDC
extern DeleteDC
extern BitBlt
extern SetWindowRgn
extern GetStockObject
extern ShellExecuteW
extern SetCursor
extern SetForegroundWindow
extern CreateSolidBrush
extern GetClientRect
extern EnumChildWindows
extern SetBkColor
extern SetClassLongPtrW
extern SetWindowTheme
extern DwmSetWindowAttribute
extern GetClassNameW
extern GetWindowLongPtrW
extern GetProcAddress
extern LoadLibraryW
extern DrawFocusRect
extern lstrcmpiW
extern GetComboBoxInfo

WM_DESTROY          equ 2
WM_PAINT            equ 0Fh
WM_CLOSE            equ 10h
PS_SOLID            equ 0
NULL_BRUSH          equ 5
NULL_PEN            equ 8
; Permanent dark palette. COLORREF values are stored as 00BBGGRR.
COL_BG              equ 00181310h ; #101318
COL_PANEL           equ 00221B17h ; #171B22
COL_EDIT            equ 00302620h ; #202630
COL_FG              equ 00F4EDE8h ; #E8EDF4
COL_MUTED           equ 00A8978Bh ; #8B97A8
COL_BORDER          equ 004B3D34h ; #343D4B
COL_ACCENT          equ 00337AFFh ; #FF7A33
COL_ACCENT_DOWN     equ 00205ED9h ; #D95E20
COL_ACCENT_TEXT     equ 0016110Eh ; #0E1116
COL_SUCCESS         equ 0092D342h ; #42D392
COL_DANGER          equ 006C5DFFh ; #FF5D6C
GCLP_HBRBACKGROUND  equ -10
GWL_STYLE           equ -16
GWL_EXSTYLE         equ -20
BS_OWNERDRAW        equ 0Bh
SWP_NOZORDER        equ 4
SWP_FRAMECHANGED    equ 20h
WM_ERASEBKGND       equ 14h
WM_CTLCOLOREDIT     equ 0133h
WM_CTLCOLORLISTBOX  equ 0134h
BM_SETCHECK         equ 0F1h
OPAQUE              equ 2
DWMWA_DARK          equ 20
DWMWA_DARK_OLD      equ 19
WM_SETFONT          equ 30h
WM_COMMAND          equ 0111h
WM_TIMER            equ 0113h
WM_CONTEXTMENU      equ 7Bh
WM_NCDESTROY        equ 82h
WM_CTLCOLORSTATIC   equ 0138h
WM_CTLCOLORBTN      equ 0135h
WM_LINE             equ 8001h
WM_DONE             equ 8002h
WM_CTX              equ 8003h
WM_PHASE            equ 8004h
WM_SETICON          equ 80h
WM_SETCURSOR        equ 20h
STN_CLICKED         equ 0

WS_CAPTION          equ 00C00000h
WS_SYSMENU          equ 00080000h
WS_MINIMIZEBOX      equ 00020000h
WS_VISIBLE          equ 10000000h
WS_CHILD            equ 40000000h
WS_POPUP            equ 80000000h
WS_TABSTOP          equ 00010000h
WS_DISABLED         equ 08000000h
WS_VSCROLL          equ 00200000h
WS_CLIPSIBLINGS     equ 04000000h
WS_CLIPCHILDREN     equ 02000000h

WS_EX_CLIENTEDGE    equ 200h
WS_EX_DLGMODALFRAME equ 1
WS_EX_APPWINDOW     equ 40000h
WS_EX_CONTROLPARENT equ 10000h

BS_PUSHBUTTON       equ 0
BS_DEFPUSHBUTTON    equ 1
BS_AUTOCHECKBOX     equ 3
BS_GROUPBOX         equ 7
BS_NOTIFY           equ 4000h
ES_AUTOHSCROLL      equ 80h
ES_NUMBER           equ 2000h
CBS_DROPDOWNLIST    equ 3
CBS_HASSTRINGS      equ 200h
CBS_OWNERDRAWFIXED  equ 10h
CBN_SELCHANGE       equ 1
CBN_DROPDOWN        equ 7
SS_BITMAP           equ 0Eh
SS_LEFTNOWORDWRAP   equ 0Ch
SS_NOTIFY           equ 100h
SS_OWNERDRAW        equ 0Dh
SS_ETCHEDFRAME      equ 12h
WM_DRAWITEM         equ 2Bh
DT_LEFT             equ 0
DT_CENTER           equ 1
DT_RIGHT            equ 2
DT_VCENTER          equ 4
DT_SINGLELINE       equ 20h

CB_ADDSTRING        equ 143h
CB_SETCURSEL        equ 14Eh
CB_GETCURSEL        equ 147h
CB_GETLBTEXT        equ 148h
BM_GETCHECK         equ 0F0h
BST_CHECKED         equ 1
STM_SETIMAGE        equ 0172h
IMAGE_BITMAP        equ 0
IMAGE_ICON          equ 1
LR_DEFAULTSIZE      equ 40h
TRANSPARENT         equ 1
SRCCOPY             equ 00CC0020h
SW_SHOW             equ 5
SW_SHOWNORMAL       equ 1
SW_HIDE             equ 0
COLOR_BTNFACE       equ 15
COLOR_WINDOW        equ 5
IDC_ARROW           equ 32512
IDC_HAND            equ 32649
FW_NORMAL           equ 400
FW_BOLD             equ 700
DEFAULT_CHARSET     equ 1
CLEARTYPE_QUALITY   equ 5
GWLP_WNDPROC        equ -4

IDC_STRESS          equ 105
IDC_MB              equ 106
IDC_TIMES           equ 109
IDC_LOG             equ 112
IDC_THREADS         equ 114
IDC_START           equ 115
IDC_STOP            equ 116
IDC_ABOUT           equ 117
IDC_RESULTCARD      equ 124
IDC_FLAME           equ 119
IDC_PAYPAL          equ 120
IDC_BITS            equ 103
IDC_MBLBL           equ 107
IDC_RAMV            equ 111
IDM_COPY            equ 1001
IDM_XTREME          equ 1003
IDM_DEBUG           equ 1004

MB_OK               equ 0
MB_ICONINFORMATION  equ 40h
MB_ICONERROR        equ 10h
MB_ICONWARNING      equ 30h
MB_OKCANCEL         equ 1
IDOK                equ 1
IDCANCEL            equ 2
MF_STRING           equ 0
MF_CHECKED          equ 8
TPM_LEFTALIGN       equ 0
TPM_RETURNCMD       equ 100h
TPM_RIGHTBUTTON     equ 2
CREATE_NO_WINDOW    equ 08000000h
HIGH_PRIORITY_CLASS equ 80h
NORMAL_PRIORITY_CLASS equ 20h
STARTF_USESTDHANDLES equ 100h
STARTF_USESHOWWINDOW equ 1
HANDLE_FLAG_INHERIT equ 1
GENERIC_WRITE       equ 40000000h
FILE_APPEND_DATA    equ 4
CREATE_ALWAYS       equ 2
OPEN_ALWAYS         equ 4
FILE_ATTRIBUTE_NORMAL equ 80h
FILE_SHARE_READ     equ 1
GMEM_MOVEABLE       equ 2
CF_UNICODETEXT      equ 13
INVALID_FILE_ATTRIBUTES equ 0FFFFFFFFh
ICC_WIN95_CLASSES   equ 1
ICC_STANDARD_CLASSES equ 4000h

STYLE_GRP   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_ETCHEDFRAME)
STYLE_LBL   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS)
STYLE_HDR   equ (STYLE_LBL|SS_LEFTNOWORDWRAP)
STYLE_EDT   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|ES_AUTOHSCROLL|ES_NUMBER)
CBS_AUTOHSCROLL     equ 40h
HWND_BOTTOM         equ 1
SWP_NOSIZE          equ 1
SWP_NOMOVE          equ 2
SWP_NOACTIVATE      equ 10h
RDW_INVALIDATE      equ 1
RDW_ERASE           equ 4
RDW_ALLCHILDREN     equ 80h
RDW_UPDATENOW       equ 100h
STYLE_CMB   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_HASSTRINGS|CBS_AUTOHSCROLL)
STYLE_CHK   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|BS_AUTOCHECKBOX|BS_NOTIFY)
STYLE_BTN   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|BS_PUSHBUTTON|BS_NOTIFY)
STYLE_DEF   equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|BS_DEFPUSHBUTTON|BS_NOTIFY)
STYLE_RESULT equ (WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_OWNERDRAW|SS_NOTIFY)
STYLE_BMP   equ (WS_CHILD|WS_VISIBLE|SS_BITMAP)

UIC_STATIC  equ 0
UIC_BUTTON  equ 1
UIC_COMBO   equ 2
UIC_EDIT    equ 3
UIC_LIST    equ 4
UIF_BOLD    equ 1
UIF_SHOW    equ 2
UIF_COFFEE  equ 4
UIS_NONE    equ 255
UIS_STRESS  equ 0
UIS_MB      equ 1
UIS_TIMES   equ 2
UIS_LOG     equ 3
UIS_THREADS equ 4
UIS_START   equ 5
UIS_STOP    equ 6
UIS_ABOUT   equ 7
UIS_FLAME   equ 8
UIS_COFFEE  equ 9
UIS_BITS    equ 10
UIS_MBLBL   equ 11
UIS_RAMV    equ 12
UIS_RESULTCARD equ 13

RESULT_READY   equ 0
RESULT_RUNNING equ 1
RESULT_PASS    equ 2
RESULT_FAIL    equ 3
RESULT_STOPPED equ 4

%macro UI_CTL 11
    db %1, %2, %3, 0
    dw %4, %5, %6, %7, %8, 0
    dd %9, %10
    dd %11 - uiControls
%endmacro

struc STARTUPINFOW
    .cb             resd 1
    .pad            resd 1
    .lpReserved     resq 1
    .lpDesktop      resq 1
    .lpTitle        resq 1
    .dwX            resd 1
    .dwY            resd 1
    .dwXSize        resd 1
    .dwYSize        resd 1
    .dwXCountChars  resd 1
    .dwYCountChars  resd 1
    .dwFillAttribute resd 1
    .dwFlags        resd 1
    .wShowWindow    resw 1
    .cbReserved2    resw 1
    .pad2           resd 1
    .lpReserved2    resq 1
    .hStdInput      resq 1
    .hStdOutput     resq 1
    .hStdError      resq 1
endstruc
struc PROCESS_INFORMATION
    .hProcess       resq 1
    .hThread        resq 1
    .dwProcessId    resd 1
    .dwThreadId     resd 1
endstruc
struc SECURITY_ATTRIBUTES
    .nLength        resd 1
    .pad            resd 1
    .lpSD           resq 1
    .bInherit       resd 1
    .pad2           resd 1
endstruc
struc MEMSTAT
    .len            resd 1
    .load           resd 1
    .total          resq 1
    .avail          resq 1
    .totalPage      resq 1
    .availPage      resq 1
    .totalVirt      resq 1
    .availVirt      resq 1
    .availExt       resq 1
endstruc
struc SYSINFO
    .arch           resw 1
    .res            resw 1
    .page           resd 1
    .min            resq 1
    .max            resq 1
    .mask           resq 1
    .nproc          resd 1
    .procType       resd 1
    .allocGran      resd 1
    .level          resw 1
    .revision       resw 1
endstruc
struc LOGFONTW
    .lfHeight       resd 1
    .lfWidth        resd 1
    .lfEscapement   resd 1
    .lfOrientation  resd 1
    .lfWeight       resd 1
    .lfItalic       resb 1
    .lfUnderline    resb 1
    .lfStrikeOut    resb 1
    .lfCharSet      resb 1
    .lfOutPrecision resb 1
    .lfClipPrecision resb 1
    .lfQuality      resb 1
    .lfPitchAndFamily resb 1
    .lfFaceName     resw 32
endstruc

section .bss
hInst       resq 1
hMain       resq 1
wndcls      resb 80
dlgcls      resb 80
msg         resb 48
hFont       resq 1
hFontBold   resq 1
hIcon       resq 1
hFlame      resq 1
hCoffeeBmp  resq 1
hUiSlots:
hCmbStress  resq 1
hEdtMB      resq 1
hEdtTimes   resq 1
hChkLog     resq 1
hCmbThr     resq 1
hBtnStart   resq 1
hBtnStop    resq 1
hBtnAbout   resq 1
hFlameWnd   resq 1
hCoffeeWnd  resq 1
hLblBits    resq 1
hLblMB      resq 1
hLblRamV    resq 1
hResultCard resq 1
              resq 2 ; retain the legacy three-result-slot footprint
hBrBg       resq 1
hBrPanel    resq 1
hBrEdit     resq 1
fnAllowDark resq 1
fnSetAppMode resq 1
fnFlushMenu resq 1
hDlg        resq 1
dlgTextPtr  resq 1
dlgResult   resd 1
dlgFlags    resd 1
hProc       resq 1
hLog        resq 1
tick0       resq 1
flameIdx    resd 1
flameWait   resd 1
running     resd 1
xtreme      resd 1
debugOn     resd 1
haveResid   resd 1
failed      resd 1
passCount   resd 1
targetRuns  resd 1
stressMB    resd 1
availMB     resd 1
nproc       resd 1
lpPcores    resd 1
alignb 8
firstResid  resb 128
exeDir      resw 280
titleBuf    resw 160
tmpBuf      resw 400
tmpAnsi     resb 512
cpuAscii    resb 64
cpuWide     resw 64
lineAcc     resb 128
logPath     resw 280
memst       resb 64
sysi        resb 48
lf          resb 92
iccex       resb 8
ptxy        resd 2
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
lpAllocFail resd 1
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
alignb 64
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
resultTime  resw 80
resultSpeed resw 80
resultResid resw 80
hFontMetric resq 1
resultState resd 1
resultIndex resd 1

section .data
szClass     dw __utf16le__("MinWndClassX"),0
szDlgClass  dw __utf16le__("IBTDarkDialog"),0
szTitle     dw __utf16le__("IntelBurnTest v3.00 - by Thejuampi [Idle]"),0
szFail      dw __utf16le__("RegisterClassEx failed"),0
szFail2     dw __utf16le__("CreateWindowEx failed"),0
szCap       dw __utf16le__("IntelBurnTest"),0
szBtn       dw __utf16le__("BUTTON"),0
szStatic    dw __utf16le__("STATIC"),0
szEdit      dw __utf16le__("EDIT"),0
szCombo     dw __utf16le__("COMBOBOX"),0
szList      dw __utf16le__("LISTBOX"),0
szEmpty     dw 0
szFace      dw __utf16le__("Segoe UI"),0
szSettings  dw __utf16le__("TEST CONFIGURATION"),0
szOutput    dw __utf16le__("RESULTS"),0
szMode      dw __utf16le__("Mode:"),0
szBits      dw __utf16le__("64-bit"),0
szBitsAvx2  dw __utf16le__("64-bit AVX2"),0
szBitsAvx   dw __utf16le__("64-bit AVX"),0
szBitsSse2  dw __utf16le__("64-bit SSE2"),0
szStress    dw __utf16le__("Stress:"),0
szMB        dw __utf16le__("MB"),0
szTimes     dw __utf16le__("Times to run:"),0
szRam       dw __utf16le__("Free RAM:"),0
szLog       dw __utf16le__("Output results to results.log"),0
szThreads   dw __utf16le__("Threads:"),0
szStart     dw __utf16le__("&Start"),0
szStop      dw __utf16le__("&Stop"),0
szAbout     dw __utf16le__("&About"),0
szOK        dw __utf16le__("OK"),0
szCancel    dw __utf16le__("Cancel"),0
szDarkMode  dw __utf16le__("DarkMode_Explorer"),0
szDarkCfd   dw __utf16le__("DarkMode_CFD"),0
szUxTheme   dw __utf16le__("uxtheme.dll"),0
szChev      dw 25BEh,0
szFreeze    dw __utf16le__("THERMAL LOAD"),0
szTime      dw __utf16le__("TIME (S)"),0
szSpeed     dw __utf16le__("SPEED (GFLOPS)"),0
szResults   dw __utf16le__("RESIDUAL"),0
szTen       dw __utf16le__("10"),0
sz1024      dw __utf16le__("1024"),0
szStd       dw __utf16le__("Standard"),0
szHigh      dw __utf16le__("High"),0
szVHigh     dw __utf16le__("Very High"),0
szMax       dw __utf16le__("Maximum"),0
szCustom    dw __utf16le__("Custom"),0
szAll       dw __utf16le__("All"),0
szAuto      dw __utf16le__("Auto"),0
szT1        dw __utf16le__("1"),0
szT2        dw __utf16le__("2"),0
szT3        dw __utf16le__("3"),0
szT4        dw __utf16le__("4"),0
szT6        dw __utf16le__("6"),0
szT8        dw __utf16le__("8"),0
szT12       dw __utf16le__("12"),0
szT16       dw __utf16le__("16"),0
szT24       dw __utf16le__("24"),0
szT32       dw __utf16le__("32"),0
szT48       dw __utf16le__("48"),0
szT64       dw __utf16le__("64"),0
szT96       dw __utf16le__("96"),0
szT128      dw __utf16le__("128"),0
szCopy      dw __utf16le__("Copy"),0
szCopyAll   dw __utf16le__("Copy All"),0
szXtreme    dw __utf16le__("Xtreme Stress Mode"),0
szDbg       dw __utf16le__("Enable Debug Logging"),0
szIdle      dw __utf16le__("Idle"),0
szPreparing dw __utf16le__("Preparing test data"),0
szFinished  dw __utf16le__("Finished"),0
szStopped   dw __utf16le__("Stopped"),0
szFailure   dw __utf16le__("Failure"),0
szTitleFmt  dw __utf16le__("IntelBurnTest v3.00 - by Thejuampi [%s]"),0
szTitleProg dw __utf16le__("IntelBurnTest v3.00 - by Thejuampi [Running] (%u of %u Completed)"),0
szRamFmt    dw __utf16le__("%u MB"),0
szNumFmt    dw __utf16le__("%u"),0
szAboutCap  dw __utf16le__("About"),0
szAboutTxt  dw __utf16le__("IntelBurnTest"),13,10
            dw __utf16le__("by Thejuampi"),13,10
            dw __utf16le__("original by AgentGOD"),13,10
            dw __utf16le__("native rewrite, no .NET"),13,10,13,10
            dw __utf16le__("In-process AVX2 HPL. No linpack.exe, no DLLs."),13,10
            dw __utf16le__("Adequate cooling is required, especially the CPU."),0
szMemCap    dw __utf16le__("IntelBurnTest - Error"),0
szMemErr    dw __utf16le__("You cannot use more memory than you have available! Please change your stress settings."),0
szAllocErr  dw __utf16le__("Not enough memory for this stress size. Lower the MB setting."),0
szCritCap   dw __utf16le__("IntelBurnTest - Critical Error"),0
szFailTxt   dw __utf16le__("WARNING! Your system was found to be unstable under IntelBurnTest!"),13,10
            dw __utf16le__("Please check your cooling system and/or lower your overclock."),13,10,13,10
            dw __utf16le__("Test executed for %u seconds."),0
szOkCap     dw __utf16le__("IntelBurnTest - Success"),0
szOkTxt     dw __utf16le__("Success! Your system was able to maintain its stability while running IntelBurnTest."),13,10,13,10
            dw __utf16le__("Test completed successfully in %u seconds."),0
szDieTxt    dw __utf16le__("WARNING! The test stopped unexpectedly."),13,10,13,10
            dw __utf16le__("Test executed for %u seconds."),0
szXtremeAsk dw __utf16le__("Xtreme Stress Mode may cause the CPU to heat up more. Windows may not be usable."),13,10,13,10
            dw __utf16le__("Enable Xtreme Stress Mode?"),0
szLogHdr    dw __utf16le__("----------------------------"),13,10
            dw __utf16le__("Created by Thejuampi"),13,10
            dw __utf16le__("Stress Level: %s (%u MB)"),13,10
            dw __utf16le__("Time (s)    Speed (GFlops)    Result"),13,10,0
szOpenVerb  dw __utf16le__("open"),0
szCoffeeUrl dw __utf16le__("https://buymeacoffee.com/thejuampi"),0
szLogLine   dw __utf16le__("%s    %s    %s"),13,10,0
szLogEnd    dw __utf16le__("Test Result: %s"),13,10
            dw __utf16le__("Processor: %s"),13,10,0
szSuccessA  dw __utf16le__("Success."),0
szFailA     dw __utf16le__("Failure."),0
szStoppedA  dw __utf16le__("Stopped by user."),0
szLogName   dw __utf16le__("results.log"),0
szTab       dw 9,0
szCRLF      dw 13,10,0
flameDelay  db 1,2,2,1,2,1,2,2

align 4
uiControls:
    UI_CTL UIC_STATIC, UIF_BOLD, UIS_NONE, 99, 26, 10, 150, 18, STYLE_HDR, 0, szSettings
    UI_CTL UIC_STATIC, 0, UIS_NONE, 102, 24, 40, 38, 18, STYLE_LBL, 0, szMode
    UI_CTL UIC_STATIC, 0, UIS_BITS, IDC_BITS, 66, 40, 74, 18, STYLE_LBL, 0, szBits
    UI_CTL UIC_STATIC, 0, UIS_NONE, 104, 148, 40, 70, 18, STYLE_LBL, 0, szStress
    UI_CTL UIC_COMBO, 0, UIS_STRESS, IDC_STRESS, 222, 36, 105, 200, STYLE_CMB, 0, szEmpty
    UI_CTL UIC_EDIT, 0, UIS_MB, IDC_MB, 336, 37, 55, 26, STYLE_EDT|WS_DISABLED, WS_EX_CLIENTEDGE, sz1024
    UI_CTL UIC_STATIC, UIF_BOLD, UIS_MBLBL, IDC_MBLBL, 397, 40, 28, 18, STYLE_LBL, 0, szMB
    UI_CTL UIC_STATIC, 0, UIS_NONE, 108, 24, 78, 72, 18, STYLE_LBL, 0, szTimes
    UI_CTL UIC_EDIT, 0, UIS_TIMES, IDC_TIMES, 100, 74, 48, 26, STYLE_EDT, WS_EX_CLIENTEDGE, szTen
    UI_CTL UIC_STATIC, 0, UIS_NONE, 110, 306, 78, 62, 18, STYLE_LBL, 0, szRam
    UI_CTL UIC_STATIC, 0, UIS_RAMV, IDC_RAMV, 370, 78, 58, 18, STYLE_LBL, 0, szEmpty
    UI_CTL UIC_BUTTON, 0, UIS_LOG, IDC_LOG, 24, 108, 194, 22, STYLE_CHK, 0, szLog
    UI_CTL UIC_STATIC, 0, UIS_NONE, 113, 164, 78, 52, 18, STYLE_LBL, 0, szThreads
    UI_CTL UIC_COMBO, 0, UIS_THREADS, IDC_THREADS, 222, 74, 72, 200, STYLE_CMB, 0, szEmpty
    UI_CTL UIC_STATIC, UIF_BOLD, UIS_NONE, 118, 462, 10, 124, 18, STYLE_HDR, 0, szFreeze
    UI_CTL UIC_STATIC, UIF_SHOW, UIS_FLAME, IDC_FLAME, 462, 42, 53, 50, STYLE_BMP, 0, szEmpty
    UI_CTL UIC_BUTTON, 0, UIS_START, IDC_START, 526, 30, 70, 31, STYLE_BTN, 0, szStart
    UI_CTL UIC_BUTTON, 0, UIS_STOP, IDC_STOP, 526, 67, 70, 29, STYLE_BTN|WS_DISABLED, 0, szStop
    UI_CTL UIC_BUTTON, 0, UIS_ABOUT, IDC_ABOUT, 462, 105, 43, 26, STYLE_DEF, 0, szAbout
    UI_CTL UIC_STATIC, UIF_COFFEE, UIS_COFFEE, IDC_PAYPAL, 511, 107, 85, 23, STYLE_BMP|SS_NOTIFY, 0, szEmpty
    UI_CTL UIC_STATIC, UIF_BOLD, UIS_NONE, 98, 26, 146, 152, 18, STYLE_HDR, 0, szOutputV3
    UI_CTL UIC_STATIC, UIF_SHOW, UIS_RESULTCARD, IDC_RESULTCARD, 24, 170, 572, 204, STYLE_RESULT, 0, szEmpty
uiControlsEnd:

uiClasses:
    dd szStatic - uiControls
    dd szBtn - uiControls
    dd szCombo - uiControls
    dd szEdit - uiControls
    dd szList - uiControls

; Keep the validated data/BSS boundary stable. Numerical state keeps the same
; virtual addresses even as the presentation layer evolves.
szOutputV3 dw __utf16le__("STABILITY MONITOR"),0
times 011ECh-($-$$) db 0

section .v3str rdata align=16
szRunAgain  dw __utf16le__("&Run again"),0
szCardHead  dw __utf16le__("RUN PROGRESS"),0
szReady     dw __utf16le__("READY"),0
szRunning   dw __utf16le__("RUNNING"),0
szPassed    dw __utf16le__("PASSED"),0
szFailed    dw __utf16le__("FAILED"),0
szStoppedCard dw __utf16le__("STOPPED"),0
szRunFmt    dw __utf16le__("RUN %u / %u"),0
szReadyStart dw __utf16le__("READY TO START"),0
szWarming   dw __utf16le__("WARMING UP"),0
szDash      dw __utf16le__("--"),0
szGflops    dw __utf16le__("GFLOPS"),0
szLastRun   dw __utf16le__("LAST RUN"),0
szSecondsFmt dw __utf16le__("%s s"),0
szSignature dw __utf16le__("STABILITY SIGNATURE"),0
szAwaiting  dw __utf16le__("Waiting for the first completed run"),0
szReference dw __utf16le__("REFERENCE CAPTURED"),0
szMatchFmt  dw __utf16le__("MATCH - %u / %u CONSISTENT"),0
szPassMatch dw __utf16le__("ALL RUNS MATCH"),0
szMismatch  dw __utf16le__("RESIDUAL MISMATCH"),0
szNoRuns    dw __utf16le__("No completed runs yet."),0
szSummaryFmt dw __utf16le__("Run %u/%u | %s GFLOPS | residual %s"),0
szCopySummary dw __utf16le__("Copy summary"),0

section .text
global start

PROC_FRAME_ALIGNED start, 80h
    xor ecx, ecx
    call GetModuleHandleW
    mov [hInst], rax
    call lp_isa_probe
    call TlsAlloc
    mov [lpTlsIdx], eax
%if SELFTEST
    call GetCommandLineW
    mov r10, rax
.tscan:
    cmp word [r10], 0
    je .notest
    cmp word [r10], ' '
    jne .tnext
    cmp word [r10+2], '-'
    jne .tnext
    movzx eax, word [r10+4]
    or eax, 20h
    cmp eax, 't'
    jne .tnext
    movzx eax, word [r10+6]
    test eax, eax
    jz .dotest
    cmp eax, ' '
    je .dotest
.tnext:
    add r10, 2
    jmp .tscan
.dotest:
    call test_main
.notest:
%endif
    cld
    lea rdi, [wndcls]
    mov ecx, 80
    xor eax, eax
    rep stosb
    mov rcx, [hInst]
    mov edx, 1
    mov r8d, IMAGE_ICON
    xor r9d, r9d
    mov qword [rsp+20h], 0
    mov qword [rsp+28h], LR_DEFAULTSIZE
    call LoadImageW
    mov [hIcon], rax
    xor ecx, ecx
    mov edx, IDC_ARROW
    call LoadCursorW
    lea rcx, [wndcls]
    mov dword [rcx], 80
    mov dword [rcx+4], 3
    lea rdx, [wndproc]
    mov [rcx+8], rdx
    mov rdx, [hInst]
    mov [rcx+24], rdx
    mov rdx, [hIcon]
    mov [rcx+32], rdx
    mov [rcx+40], rax
    mov qword [rcx+48], 16
    lea rdx, [szClass]
    mov [rcx+64], rdx
    mov rdx, [hIcon]
    mov [rcx+72], rdx
    call RegisterClassExW
    test eax, eax
    jnz .okreg
    xor ecx, ecx
    lea rdx, [szFail]
    lea r8, [szCap]
    xor r9d, r9d
    call MessageBoxW
    xor ecx, ecx
    call ExitProcess
.okreg:
    lea rax, [rsp+60h]
    mov dword [rax], 0
    mov dword [rax+4], 0
    mov dword [rax+8], 620
    mov dword [rax+12], 400
    mov rcx, rax
    mov edx, 06CA0000h
    xor r8d, r8d
    mov r9d, 00050000h
    call AdjustWindowRectEx
    mov eax, [rsp+68h]
    sub eax, [rsp+60h]
    movsxd rax, eax
    mov [rsp+70h], rax
    mov eax, [rsp+6Ch]
    sub eax, [rsp+64h]
    movsxd rax, eax
    mov [rsp+78h], rax
    mov qword [rsp+20h], 100
    mov qword [rsp+28h], 100
    mov rax, [rsp+70h]
    mov [rsp+30h], rax
    mov rax, [rsp+78h]
    mov [rsp+38h], rax
    mov qword [rsp+40h], 0
    mov qword [rsp+48h], 0
    mov rax, [hInst]
    mov [rsp+50h], rax
    mov qword [rsp+58h], 0
    mov ecx, 00050000h
    lea rdx, [szClass]
    lea r8, [szTitle]
    mov r9d, 06CA0000h
    call CreateWindowExW
    mov [hMain], rax
    test rax, rax
    jnz .okwin
    xor ecx, ecx
    lea rdx, [szFail2]
    lea r8, [szCap]
    xor r9d, r9d
    call MessageBoxW
    xor ecx, ecx
    call ExitProcess
.okwin:
    call ui_init
    mov rcx, [hMain]
    mov edx, 1
    mov r8d, 100
    xor r9d, r9d
    mov qword [rsp+20h], 0
    call SetTimer
    mov rcx, [hMain]
    mov edx, 2
    mov r8d, 1000
    xor r9d, r9d
    mov qword [rsp+20h], 0
    call SetTimer
    mov rcx, [hMain]
    mov edx, WM_SETICON
    mov r8d, 1
    mov r9, [hIcon]
    call SendMessageW
    mov rcx, [hMain]
    mov edx, SW_SHOW
    call ShowWindow
    mov rcx, [hMain]
    call UpdateWindow
.loop:
    lea rcx, [msg]
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call GetMessageW
    test eax, eax
    jz .quit
    mov rcx, [hMain]
    lea rdx, [msg]
    call IsDialogMessageW
    test eax, eax
    jnz .loop
    lea rcx, [msg]
    call TranslateMessage
    lea rcx, [msg]
    call DispatchMessageW
    jmp .loop
.quit:
    xor ecx, ecx
    call ExitProcess

PROC_FRAME ui_init, 80h, rbx, rsi
    lea rcx, [iccex]
    mov dword [rcx], 8
    mov dword [rcx+4], ICC_WIN95_CLASSES|ICC_STANDARD_CLASSES
    call InitCommonControlsEx
    call fonts_create
    call bitmaps_load
    call theme_ux_init
    call theme_appmode
    call dialog_register
    call ui_create
    call theme_hook
    call ui_populate
    call ram_update
    call stress_toggle_custom
    call paths_init
    call theme_apply_rounded
    call cpu_brand_read
    call isa_bits_apply
    mov rcx, [hLblMB]
    lea rdx, [szMB]
    call SetWindowTextW
    mov rcx, [hMain]
    xor edx, edx
    mov r8d, RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW
    xor r9d, r9d
    call RedrawWindow
    ENDPROC_SAVED 80h, rbx, rsi

%include "ibt_ui.inc"
%include "ibt_theme.inc"

; UI experiments may release space before the numerical engine. Increase this
; compile-time padding by the exact released byte count to preserve the validated
; benchmark address without introducing a location-dependent assembler expression.
%assign UI_BENCH_PAD_BYTES 295
%rep UI_BENCH_PAD_BYTES
    db 090h
%endrep

%include "ibt_lpk.inc"
%include "bench_lib.inc"




; Kept after the benchmark engine so UI-only drawing code cannot move hot loops.
section .text$v3 code align=16

PROC_FRAME result_invalidate, 30h
    mov rcx, [hResultCard]
    test rcx, rcx
    jz .out
    xor edx, edx
    xor r8d, r8d
    call InvalidateRect
.out:
    ENDPROC

PROC_FRAME card_text_at, 58h, rbx, rsi, rdi, r12, r13
    mov rbx, rcx
    mov rsi, rdx
    mov r12, [rbp+30h]
    mov r13d, [rbp+38h]
    mov rdi, [rbp+40h]
    mov eax, r8d
    movzx ecx, ax
    mov [rsp+40h], ecx
    shr eax, 16
    mov [rsp+44h], eax
    mov eax, r9d
    movzx ecx, ax
    mov [rsp+48h], ecx
    shr eax, 16
    mov [rsp+4Ch], eax
    mov rcx, rbx
    mov edx, TRANSPARENT
    call SetBkMode
    mov rcx, rbx
    mov edx, r13d
    call SetTextColor
    mov rcx, rbx
    mov rdx, r12
    call SelectObject
    mov [rsp+50h], rax
    mov rcx, rbx
    mov rdx, rsi
    mov r8d, -1
    lea r9, [rsp+40h]
    mov [rsp+20h], rdi
    call DrawTextW
    mov rcx, rbx
    mov rdx, [rsp+50h]
    call SelectObject
    ENDPROC_SAVED 58h, rbx, rsi, rdi, r12, r13

PROC_FRAME card_bar, 60h, rbx, rsi, r12, r13
    mov rbx, rcx
    mov esi, edx
    mov r12d, r8d
    mov r13d, r9d
    mov ecx, NULL_PEN
    call GetStockObject
    mov rcx, rbx
    mov rdx, rax
    call SelectObject
    mov [rsp+40h], rax
    mov rcx, rbx
    mov rdx, [hBrPanel]
    call SelectObject
    mov [rsp+48h], rax
    mov rcx, rbx
    mov edx, 18
    mov r8d, 44
    mov r9d, 554
    mov qword [rsp+20h], 52
    mov qword [rsp+28h], 8
    mov qword [rsp+30h], 8
    call RoundRect
    test r12d, r12d
    jz .restore
    test esi, esi
    jz .restore
    mov eax, esi
    imul eax, 536
    xor edx, edx
    div r12d
    add eax, 18
    cmp eax, 554
    jbe .width_ok
    mov eax, 554
.width_ok:
    mov [rsp+58h], eax
    mov ecx, r13d
    call CreateSolidBrush
    mov [rsp+50h], rax
    mov rcx, rbx
    mov rdx, rax
    call SelectObject
    mov rcx, rbx
    mov edx, 18
    mov r8d, 44
    mov r9d, [rsp+58h]
    mov qword [rsp+20h], 52
    mov qword [rsp+28h], 8
    mov qword [rsp+30h], 8
    call RoundRect
    mov rcx, rbx
    mov rdx, [hBrPanel]
    call SelectObject
    mov rcx, [rsp+50h]
    call DeleteObject
.restore:
    mov rcx, rbx
    mov rdx, [rsp+48h]
    call SelectObject
    mov rcx, rbx
    mov rdx, [rsp+40h]
    call SelectObject
    ENDPROC_SAVED 60h, rbx, rsi, r12, r13

%macro CARD_LINE 8
    mov rcx, rsi
    lea rdx, [%1]
    mov r8d, ((%3) << 16) | (%2)
    mov r9d, ((%5) << 16) | (%4)
    mov rax, [%6]
    mov [rsp+20h], rax
    mov qword [rsp+28h], %7
    mov qword [rsp+30h], %8
    call card_text_at
%endmacro

PROC_FRAME result_draw, 0B0h, rbx, rsi, r12, r13
    mov rbx, rcx
    mov rsi, [rbx+32]
    mov r12d, [resultState]
    mov r13d, COL_MUTED
    cmp r12d, RESULT_RUNNING
    jne .not_running
    mov r13d, COL_ACCENT
    jmp .color_ready
.not_running:
    cmp r12d, RESULT_PASS
    jne .not_pass
    mov r13d, COL_SUCCESS
    jmp .color_ready
.not_pass:
    cmp r12d, RESULT_FAIL
    jne .color_ready
    mov r13d, COL_DANGER
.color_ready:
    mov rcx, rsi
    lea rdx, [rbx+40]
    mov r8, [hBrPanel]
    call FillRect
    xor ecx, ecx
    mov edx, 1
    mov r8d, r13d
    call CreatePen
    mov [rsp+70h], rax
    mov rcx, rsi
    mov rdx, rax
    call SelectObject
    mov [rsp+78h], rax
    mov rcx, rsi
    mov rdx, [hBrEdit]
    call SelectObject
    mov [rsp+80h], rax
    mov rcx, rsi
    xor edx, edx
    xor r8d, r8d
    mov r9d, [rbx+48]
    mov eax, [rbx+52]
    mov [rsp+20h], rax
    call round_rect_panel
    mov rcx, rsi
    mov rdx, [rsp+80h]
    call SelectObject
    mov rcx, rsi
    mov rdx, [rsp+78h]
    call SelectObject
    mov rcx, [rsp+70h]
    call DeleteObject

    CARD_LINE szCardHead, 18, 9, 300, 32, hFontBold, COL_MUTED, DT_LEFT|DT_VCENTER|DT_SINGLELINE
    lea rax, [szReady]
    cmp r12d, RESULT_RUNNING
    jne .status_pass
    lea rax, [szRunning]
    jmp .status_ready
.status_pass:
    cmp r12d, RESULT_PASS
    jne .status_fail
    lea rax, [szPassed]
    jmp .status_ready
.status_fail:
    cmp r12d, RESULT_FAIL
    jne .status_stopped
    lea rax, [szFailed]
    jmp .status_ready
.status_stopped:
    cmp r12d, RESULT_STOPPED
    jne .status_ready
    lea rax, [szStoppedCard]
.status_ready:
    mov [rsp+88h], rax
    mov rcx, rsi
    mov rdx, rax
    mov r8d, (9 << 16) | 340
    mov r9d, (32 << 16) | 554
    mov rax, [hFontBold]
    mov [rsp+20h], rax
    mov [rsp+28h], r13
    mov qword [rsp+30h], DT_RIGHT|DT_VCENTER|DT_SINGLELINE
    call card_text_at

    mov edx, [resultIndex]
    mov r8d, [targetRuns]
    test r8d, r8d
    jnz .bar_target
    mov r8d, 10
.bar_target:
    mov rcx, rsi
    mov r9d, r13d
    call card_bar

    cmp r12d, RESULT_READY
    jne .format_run
    CARD_LINE szReadyStart, 18, 57, 220, 78, hFont, COL_FG, DT_LEFT|DT_VCENTER|DT_SINGLELINE
    jmp .run_done
.format_run:
    mov r8d, [resultIndex]
    cmp r12d, RESULT_RUNNING
    jne .run_number
    cmp r8d, [targetRuns]
    jae .run_number
    inc r8d
.run_number:
    lea rcx, [titleBuf]
    lea rdx, [szRunFmt]
    mov r9d, [targetRuns]
    call wsprintfW
    CARD_LINE titleBuf, 18, 57, 220, 78, hFont, COL_FG, DT_LEFT|DT_VCENTER|DT_SINGLELINE
.run_done:
    cmp word [resultSpeed], 0
    jne .metric_value
    cmp r12d, RESULT_RUNNING
    jne .metric_dash
    CARD_LINE szWarming, 18, 78, 232, 126, hFontBold, COL_FG, DT_RIGHT|DT_VCENTER|DT_SINGLELINE
    jmp .metric_done
.metric_dash:
    CARD_LINE szDash, 18, 78, 232, 126, hFontMetric, COL_FG, DT_RIGHT|DT_VCENTER|DT_SINGLELINE
    jmp .metric_done
.metric_value:
    CARD_LINE resultSpeed, 18, 78, 232, 126, hFontMetric, COL_FG, DT_RIGHT|DT_VCENTER|DT_SINGLELINE
.metric_done:
    CARD_LINE szGflops, 242, 91, 332, 122, hFontBold, COL_MUTED, DT_LEFT|DT_VCENTER|DT_SINGLELINE
    CARD_LINE szLastRun, 390, 72, 554, 92, hFont, COL_MUTED, DT_RIGHT|DT_VCENTER|DT_SINGLELINE
    cmp word [resultTime], 0
    jne .time_value
    CARD_LINE szDash, 390, 93, 554, 120, hFontBold, COL_FG, DT_RIGHT|DT_VCENTER|DT_SINGLELINE
    jmp .time_done
.time_value:
    lea rcx, [tmpBuf]
    lea rdx, [szSecondsFmt]
    lea r8, [resultTime]
    call wsprintfW
    CARD_LINE tmpBuf, 390, 93, 554, 120, hFontBold, COL_FG, DT_RIGHT|DT_VCENTER|DT_SINGLELINE
.time_done:
    CARD_LINE szSignature, 18, 137, 270, 158, hFontBold, COL_MUTED, DT_LEFT|DT_VCENTER|DT_SINGLELINE
    cmp dword [resultIndex], 0
    jne .signature_value
    CARD_LINE szAwaiting, 18, 163, 554, 192, hFont, COL_MUTED, DT_LEFT|DT_VCENTER|DT_SINGLELINE
    jmp .draw_done
.signature_value:
    CARD_LINE resultResid, 18, 163, 286, 192, hFont, COL_FG, DT_LEFT|DT_VCENTER|DT_SINGLELINE
    cmp r12d, RESULT_PASS
    jne .match_fail
    lea rax, [szPassMatch]
    jmp .match_draw
.match_fail:
    cmp r12d, RESULT_FAIL
    jne .match_reference
    lea rax, [szMismatch]
    jmp .match_draw
.match_reference:
    cmp dword [resultIndex], 1
    jne .match_format
    lea rax, [szReference]
    jmp .match_draw
.match_format:
    lea rcx, [titleBuf]
    lea rdx, [szMatchFmt]
    mov r8d, [passCount]
    mov r9d, [resultIndex]
    call wsprintfW
    lea rax, [titleBuf]
.match_draw:
    mov rcx, rsi
    mov rdx, rax
    mov r8d, (163 << 16) | 286
    mov r9d, (192 << 16) | 554
    mov rax, [hFontBold]
    mov [rsp+20h], rax
    mov [rsp+28h], r13
    mov qword [rsp+30h], DT_RIGHT|DT_VCENTER|DT_SINGLELINE
    call card_text_at
.draw_done:
    mov eax, 1
    ENDPROC_SAVED 0B0h, rbx, rsi, r12, r13

%unmacro CARD_LINE 8

section .text

round_button:
    mov eax, 10
    jmp round_window

round_small:
    mov eax, 8

PROC_FRAME round_window, 40h, rbx, r12
    mov rbx, rcx
    mov r12d, eax
    lea rdx, [rsp+30h]
    call GetClientRect
    xor ecx, ecx
    xor edx, edx
    mov r8d, [rsp+38h]
    mov r9d, [rsp+3Ch]
    mov [rsp+20h], r12
    mov [rsp+28h], r12
    call CreateRoundRectRgn
    test rax, rax
    jz .invalidate
    mov rcx, rbx
    mov rdx, rax
    mov r8d, 1
    call SetWindowRgn
.invalidate:
    mov rcx, rbx
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
    ENDPROC_SAVED 40h, rbx, r12

PROC_FRAME dialog_button_init, 28h, rbx
    mov rbx, rcx
    call apply_font
    mov rcx, rbx
    call round_button
    ENDPROC_SAVED 28h, rbx

PROC_FRAME coffee_init, 30h
    mov edx, STM_SETIMAGE
    mov r8d, IMAGE_BITMAP
    mov r9, [hCoffeeBmp]
    call SendMessageW
    mov rcx, [hFlameWnd]
    lea rdx, [flame_sub]
    mov r8d, 4
    xor r9d, r9d
    call SetWindowSubclass
    ENDPROC

coffee_theme_keep:
    jmp round_button

round_rect_panel:
    mov eax, 12
    jmp round_rect_draw

round_rect_control:
    mov eax, 8

round_rect_draw:
    mov r10d, eax
    mov rax, [rsp+28h]
    sub rsp, 38h
    mov [rsp+20h], rax
    mov [rsp+28h], r10
    mov [rsp+30h], r10
    call RoundRect
    add rsp, 38h
    ret

PROC_FRAME round_controls, 20h
    mov rcx, [hEdtTimes]
    call round_small
    mov rcx, [hEdtMB]
    call round_small
    mov rcx, [hCmbStress]
    call round_small
    mov rcx, [hCmbThr]
    call round_small
    ENDPROC

PROC_FRAME theme_apply_rounded, 20h
    call theme_apply
    call round_controls
    ENDPROC

PROC_FRAME theme_list_rounded, 28h, rbx
    mov rbx, rcx
    call SetWindowTheme
    mov rcx, rbx
    call round_small
    ENDPROC_SAVED 28h, rbx

PROC_FRAME flame_sub, 0A8h, rbx, r12, r13
    mov [rbp+16], rcx
    mov [rbp+24], rdx
    mov [rbp+32], r8
    mov [rbp+40], r9
    cmp edx, WM_PAINT
    je .paint
    cmp edx, WM_ERASEBKGND
    je .erase
    cmp edx, WM_NCDESTROY
    je .destroy
.def:
    mov rcx, [rbp+16]
    mov edx, [rbp+24]
    mov r8, [rbp+32]
    mov r9, [rbp+40]
    mov rax, [rbp+48]
    mov [rsp+20h], rax
    mov rax, [rbp+56]
    mov [rsp+28h], rax
    call DefSubclassProc
    jmp .out
.destroy:
    mov rcx, [rbp+16]
    lea rdx, [flame_sub]
    mov r8, [rbp+48]
    call RemoveWindowSubclass
    jmp .def
.erase:
    mov eax, 1
    jmp .out
.paint:
    mov rcx, [rbp+16]
    lea rdx, [rsp+50h]
    call BeginPaint
    mov rbx, rax
    mov rcx, rbx
    call CreateCompatibleDC
    mov r12, rax
    mov rdx, [hFlame]
    mov rcx, r12
    call SelectObject
    mov r13, rax
    mov rcx, rbx
    xor edx, edx
    xor r8d, r8d
    mov r9d, 53
    mov qword [rsp+20h], 50
    mov [rsp+28h], r12
    imul eax, [flameIdx], 53
    mov [rsp+30h], rax
    mov qword [rsp+38h], 0
    mov qword [rsp+40h], SRCCOPY
    call BitBlt
    mov rcx, r12
    mov rdx, r13
    call SelectObject
    mov rcx, r12
    call DeleteDC
    mov rcx, [rbp+16]
    lea rdx, [rsp+50h]
    call EndPaint
    xor eax, eax
.out:
    ENDPROC_SAVED 0A8h, rbx, r12, r13

; Retain the legacy kernel imports and .text extent so the linker keeps the
; import thunks used by the numerical engine at their validated addresses.
; This block is never called.
compat_import_layout:
    call GetProcessHeap
    call HeapAlloc
    call HeapFree
    ret

; Preserve the legacy .text extent so the linker emits import thunks at the
; validated addresses referenced by the numerical engine.
times 20 db 090h
