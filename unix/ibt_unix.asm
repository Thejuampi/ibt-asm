bits 64
default rel

; IntelBurnTest Unix frontend (Linux/x86-64 + X11).
; The numerical engine is included verbatim from the Windows build below.

%define SELFTEST 0
%include "ibt_draw.inc"

global _start

extern XOpenDisplay
extern XDefaultRootWindow
extern XDefaultScreen
extern XDefaultVisual
extern XDefaultColormap
extern XDefaultDepth
extern XCreateSimpleWindow
extern XCreateImage
extern XPutImage
extern XCreateGC
extern XSelectInput
extern XStoreName
extern XInternAtom
extern XSetWMProtocols
extern XSetWMNormalHints
extern XMapWindow
extern XPending
extern XNextEvent
extern XLookupString
extern XSetForeground
extern XFillRectangle
extern XFillArc
extern XDrawRectangle
extern XDrawLine
extern XFillPolygon
extern XFlush
extern XCloseDisplay
extern XftDrawCreate
extern XftFontOpenName
extern XftColorAllocValue
extern XftDrawStringUtf8
extern XftTextExtentsUtf8

extern malloc
extern free
extern posix_memalign
extern pthread_create
extern pthread_join
extern pthread_key_create
extern pthread_getspecific
extern pthread_setspecific
extern clock_gettime
extern sysconf
extern sysinfo
extern usleep
extern system

WM_PHASE       equ 08004h
WM_DONE        equ 08002h
RESULT_READY   equ 0
RESULT_RUNNING equ 1
RESULT_PASS    equ 2
RESULT_FAIL    equ 3
RESULT_STOPPED equ 4

ExposureMask       equ 1 << 15
KeyPressMask       equ 1 << 0
ButtonPressMask    equ 1 << 2
StructureNotifyMask equ 1 << 17

COL_BG       equ 0101318h
COL_PANEL    equ 0171B22h
COL_EDIT     equ 0202630h
COL_FG       equ 0E8EDF4h
COL_MUTED    equ 08B97A8h
COL_BORDER   equ 0343D4Bh
COL_ACCENT   equ 0FF7A33h
COL_DARK     equ 00E1116h
COL_SUCCESS  equ 042D392h
COL_DANGER   equ 0FF5D6Ch
COL_YELLOW   equ 0FFD83Dh
COL_CHECK    equ 060CDFFh
COL_SELECT   equ 0078D4h

%include "ibt_state.inc"

section .text

%include "ibt_app.inc"
%include "ibt_window.inc"
%include "ibt_events.inc"
%include "ibt_layout.inc"
%include "ibt_draw_primitives.inc"
%include "ibt_bridge.inc"
%include "../ibt_lpk.inc"
%include "../bench_lib.inc"
