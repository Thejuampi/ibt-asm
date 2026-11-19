; Windows imports grouped by DLL.  The including file defines PE_DLL and PE_API.
PE_DLL kernel32, 'KERNEL32.dll'
PE_API ExitProcess
PE_API GetModuleHandleW
PE_API GetModuleFileNameW
PE_API FindResourceW
PE_API WriteFile
PE_API CloseHandle
PE_API CreateFileW
PE_API CreateThread
PE_API VirtualAlloc
PE_API VirtualFree
PE_API QueryPerformanceCounter
PE_API QueryPerformanceFrequency
PE_API WaitForSingleObject
PE_API TlsAlloc
PE_API TlsGetValue
PE_API TlsSetValue
PE_API SetThreadPriority
PE_API GetCurrentThread
PE_API GetCommandLineW
PE_API LocalFree
PE_API GetSystemInfo
PE_API GetLogicalProcessorInformationEx
PE_API GlobalMemoryStatusEx
PE_API GetTickCount64
PE_API lstrlenW
PE_API lstrcpyW
PE_API lstrcatW
PE_API MultiByteToWideChar
PE_API WideCharToMultiByte
PE_API GlobalAlloc
PE_API GlobalLock
PE_API GlobalUnlock
PE_API GetProcAddress
PE_API LoadLibraryW
PE_API lstrcmpiW
PE_END_DLL

PE_DLL user32, 'USER32.dll'
PE_API RegisterClassExW
PE_API CreateWindowExW
PE_API ShowWindow
PE_API UpdateWindow
PE_API GetMessageW
PE_API TranslateMessage
PE_API DispatchMessageW
PE_API DefWindowProcW
PE_API PostQuitMessage
PE_API PostMessageW
PE_API SendMessageW
PE_API LoadCursorW
PE_API LoadImageW
PE_API MessageBoxW
PE_API SetWindowTextW
PE_API GetWindowTextW
PE_API EnableWindow
PE_API SetTimer
PE_API KillTimer
PE_API DestroyWindow
PE_API GetSystemMetrics
PE_API wsprintfW
PE_API IsDialogMessageW
PE_API OpenClipboard
PE_API CloseClipboard
PE_API EmptyClipboard
PE_API SetClipboardData
PE_API TrackPopupMenu
PE_API CreatePopupMenu
PE_API AppendMenuW
PE_API DestroyMenu
PE_API DestroyIcon
PE_API GetSysColorBrush
PE_API SetWindowLongPtrW
PE_API GetCursorPos
PE_API GetDlgCtrlID
PE_API InvalidateRect
PE_API AdjustWindowRectEx
PE_API SetWindowPos
PE_API RedrawWindow
PE_API DrawTextW
PE_API FillRect
PE_API BeginPaint
PE_API EndPaint
PE_API SetWindowRgn
PE_API SetCursor
PE_API SetForegroundWindow
PE_API GetClientRect
PE_API GetDC
PE_API ReleaseDC
PE_API EnumChildWindows
PE_API SetClassLongPtrW
PE_API GetClassNameW
PE_API GetWindowLongPtrW
PE_API DrawFocusRect
PE_API GetComboBoxInfo
PE_END_DLL

PE_DLL gdi32, 'GDI32.dll'
PE_API SetBkMode
PE_API SetTextColor
PE_API CreateFontIndirectW
PE_API DeleteObject
PE_API SelectObject
PE_API CreatePen
PE_API RoundRect
PE_API CreateRoundRectRgn
PE_API CreateCompatibleDC
PE_API CreateDIBSection
PE_API DeleteDC
PE_API BitBlt
PE_API GetStockObject
PE_API CreateSolidBrush
PE_API SetBkColor
PE_END_DLL

PE_DLL comctl32, 'COMCTL32.dll'
PE_API InitCommonControlsEx
PE_ORD SetWindowSubclass, 410
PE_ORD RemoveWindowSubclass, 412
PE_ORD DefSubclassProc, 413
PE_END_DLL

PE_DLL synch, 'api-ms-win-core-synch-l1-2-0.dll'
PE_API WakeByAddressAll
PE_END_DLL

PE_DLL shell32, 'SHELL32.dll'
PE_API CommandLineToArgvW
PE_API ShellExecuteW
PE_END_DLL

PE_DLL uxtheme, 'UxTheme.dll'
PE_API SetWindowTheme
PE_END_DLL

PE_DLL dwmapi, 'dwmapi.dll'
PE_API DwmSetWindowAttribute
PE_END_DLL
