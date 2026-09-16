import ctypes
import os
import subprocess
import time
from ctypes import wintypes

from PIL import Image


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BIN = os.path.join(ROOT, "bin")
EXE = os.path.abspath(
    os.environ.get("IBT_EXE", os.path.join(BIN, "IntelBurnTest.exe"))
)
RUN_DIR = os.path.dirname(EXE)
OUT = os.path.join(ROOT, "compare", "visual-qa")

u = ctypes.windll.user32
g = ctypes.windll.gdi32

WM_CLOSE = 0x0010
WM_COMMAND = 0x0111
WM_SETTEXT = 0x000C
CB_SETCURSEL = 0x014E
CB_SHOWDROPDOWN = 0x014F
CBN_SELCHANGE = 1
BM_CLICK = 0x00F5
BM_SETCHECK = 0x00F1
BST_CHECKED = 1
SRCCOPY = 0x00CC0020

IDC_STRESS = 105
IDC_MB = 106
IDC_TIMES = 109
IDC_LOG = 112
IDC_START = 115
IDC_STOP = 116

u.FindWindowW.restype = wintypes.HWND
u.GetDlgItem.restype = wintypes.HWND


class RECT(ctypes.Structure):
    _fields_ = [
        ("left", ctypes.c_long),
        ("top", ctypes.c_long),
        ("right", ctypes.c_long),
        ("bottom", ctypes.c_long),
    ]


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [
        ("biSize", ctypes.c_uint32),
        ("biWidth", ctypes.c_int32),
        ("biHeight", ctypes.c_int32),
        ("biPlanes", ctypes.c_uint16),
        ("biBitCount", ctypes.c_uint16),
        ("biCompression", ctypes.c_uint32),
        ("biSizeImage", ctypes.c_uint32),
        ("biXPelsPerMeter", ctypes.c_int32),
        ("biYPelsPerMeter", ctypes.c_int32),
        ("biClrUsed", ctypes.c_uint32),
        ("biClrImportant", ctypes.c_uint32),
    ]


class BITMAPINFO(ctypes.Structure):
    _fields_ = [("bmiHeader", BITMAPINFOHEADER)]


class COMBOBOXINFO(ctypes.Structure):
    _fields_ = [
        ("cbSize", wintypes.DWORD),
        ("rcItem", RECT),
        ("rcButton", RECT),
        ("stateButton", wintypes.DWORD),
        ("hwndCombo", wintypes.HWND),
        ("hwndItem", wintypes.HWND),
        ("hwndList", wintypes.HWND),
    ]


def bitmap_to_image(hdc, mdc, hbmp, width, height):
    bi = BITMAPINFO()
    bi.bmiHeader.biSize = 40
    bi.bmiHeader.biWidth = width
    bi.bmiHeader.biHeight = -height
    bi.bmiHeader.biPlanes = 1
    bi.bmiHeader.biBitCount = 32
    buf = (ctypes.c_char * (width * height * 4))()
    g.GetDIBits(mdc, hbmp, 0, height, buf, ctypes.byref(bi), 0)
    return Image.frombuffer(
        "RGBX", (width, height), bytes(buf), "raw", "BGRX", 0, 1
    ).convert("RGB")


def grab_window(hwnd):
    rc = RECT()
    u.GetWindowRect(hwnd, ctypes.byref(rc))
    width, height = rc.right - rc.left, rc.bottom - rc.top
    hdc = u.GetWindowDC(hwnd)
    mdc = g.CreateCompatibleDC(hdc)
    hbmp = g.CreateCompatibleBitmap(hdc, width, height)
    old = g.SelectObject(mdc, hbmp)
    u.PrintWindow(hwnd, mdc, 2)
    image = bitmap_to_image(hdc, mdc, hbmp, width, height)
    g.SelectObject(mdc, old)
    g.DeleteObject(hbmp)
    g.DeleteDC(mdc)
    u.ReleaseDC(hwnd, hdc)
    return image, rc


def grab_screen(rc):
    width, height = rc.right - rc.left, rc.bottom - rc.top
    hdc = u.GetDC(0)
    mdc = g.CreateCompatibleDC(hdc)
    hbmp = g.CreateCompatibleBitmap(hdc, width, height)
    old = g.SelectObject(mdc, hbmp)
    g.BitBlt(mdc, 0, 0, width, height, hdc, rc.left, rc.top, SRCCOPY)
    image = bitmap_to_image(hdc, mdc, hbmp, width, height)
    g.SelectObject(mdc, old)
    g.DeleteObject(hbmp)
    g.DeleteDC(mdc)
    u.ReleaseDC(0, hdc)
    return image


def save_window(name, hwnd):
    image, _ = grab_window(hwnd)
    path = os.path.join(OUT, name + ".png")
    image.save(path)
    print(name, image.size)


def find_exact_for_process(title, pid):
    found = []

    @ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    def visit(hwnd, _):
        owner_pid = wintypes.DWORD()
        u.GetWindowThreadProcessId(hwnd, ctypes.byref(owner_pid))
        if owner_pid.value == pid and window_title(hwnd) == title:
            found.append(hwnd)
            return False
        return True

    u.EnumWindows(visit, 0)
    return found[0] if found else 0


def wait_exact(title, pid, timeout=15):
    end = time.time() + timeout
    while time.time() < end:
        hwnd = find_exact_for_process(title, pid)
        if hwnd:
            return hwnd
        time.sleep(0.03)
    return 0


def window_title(hwnd):
    buf = ctypes.create_unicode_buffer(256)
    u.GetWindowTextW(hwnd, buf, 256)
    return buf.value


def set_text(hwnd, value):
    buf = ctypes.create_unicode_buffer(value)
    u.SendMessageW(hwnd, WM_SETTEXT, 0, buf)


def main():
    os.makedirs(OUT, exist_ok=True)
    for name in os.listdir(OUT):
        if name.lower().endswith(".png"):
            os.remove(os.path.join(OUT, name))
    subprocess.run(
        ["taskkill", "/F", "/IM", "IntelBurnTest.exe"],
        capture_output=True,
        text=True,
    )
    proc = subprocess.Popen([EXE], cwd=RUN_DIR)
    main_hwnd = wait_exact("IntelBurnTest v3.00 - by Thejuampi [Idle]", proc.pid)
    if not main_hwnd:
        proc.kill()
        raise SystemExit("NO_MAIN_WINDOW")
    u.SetForegroundWindow(main_hwnd)
    time.sleep(0.4)
    save_window("01-idle", main_hwnd)

    stress = u.GetDlgItem(main_hwnd, IDC_STRESS)
    u.SendMessageW(stress, CB_SHOWDROPDOWN, 1, None)
    time.sleep(0.35)
    main_image, main_rect = grab_window(main_hwnd)
    combo_info = COMBOBOXINFO()
    combo_info.cbSize = ctypes.sizeof(COMBOBOXINFO)
    if not u.GetComboBoxInfo(stress, ctypes.byref(combo_info)) or not combo_info.hwndList:
        proc.kill()
        raise SystemExit("NO_COMBO_LIST")
    list_image, list_rect = grab_window(combo_info.hwndList)
    main_image.paste(
        list_image,
        (list_rect.left - main_rect.left, list_rect.top - main_rect.top),
    )
    main_image.save(os.path.join(OUT, "02-stress-dropdown.png"))
    print("02-stress-dropdown", (main_rect.right - main_rect.left, main_rect.bottom - main_rect.top))
    u.SendMessageW(stress, CB_SHOWDROPDOWN, 0, None)

    u.SendMessageW(stress, CB_SETCURSEL, 4, None)
    u.SendMessageW(
        main_hwnd,
        WM_COMMAND,
        (CBN_SELCHANGE << 16) | IDC_STRESS,
        stress,
    )
    log = u.GetDlgItem(main_hwnd, IDC_LOG)
    u.SendMessageW(log, BM_SETCHECK, BST_CHECKED, None)
    time.sleep(0.25)
    save_window("03-custom-log-enabled", main_hwnd)

    about_btn = u.GetDlgItem(main_hwnd, 117)
    u.PostMessageW(about_btn, BM_CLICK, 0, 0)
    about = wait_exact("About", proc.pid, 5)
    if not about:
        proc.kill()
        raise SystemExit("NO_ABOUT_DIALOG")
    time.sleep(0.2)
    save_window("04-about-dialog", about)
    u.PostMessageW(about, WM_CLOSE, 0, 0)
    time.sleep(0.2)

    set_text(u.GetDlgItem(main_hwnd, IDC_MB), "1024")
    set_text(u.GetDlgItem(main_hwnd, IDC_TIMES), "10")
    start = u.GetDlgItem(main_hwnd, IDC_START)
    u.PostMessageW(start, BM_CLICK, 0, 0)
    deadline = time.time() + 30
    while time.time() < deadline:
        title = window_title(main_hwnd)
        if "Preparing" in title or "Running" in title:
            save_window("05-running", main_hwnd)
            break
        time.sleep(0.03)
    else:
        proc.kill()
        raise SystemExit("NO_RUNNING_STATE")

    stop = u.GetDlgItem(main_hwnd, IDC_STOP)
    u.PostMessageW(stop, BM_CLICK, 0, 0)
    deadline = time.time() + 45
    while time.time() < deadline:
        if "[Stopped]" in window_title(main_hwnd):
            save_window("06-stopped", main_hwnd)
            break
        time.sleep(0.03)
    else:
        proc.kill()
        raise SystemExit("NO_STOPPED_STATE")

    set_text(u.GetDlgItem(main_hwnd, IDC_TIMES), "3")
    u.PostMessageW(start, BM_CLICK, 0, 0)
    captured_reference = False
    captured_match = False
    critical_dialog = 0
    finished = False
    deadline = time.time() + 120
    while time.time() < deadline:
        title = window_title(main_hwnd)
        if not captured_reference and "(1 of 3 Completed)" in title:
            save_window("07-reference-captured", main_hwnd)
            captured_reference = True
        if not captured_match and "(2 of 3 Completed)" in title:
            save_window("08-consistent", main_hwnd)
            captured_match = True
        if "[Finished]" in title:
            finished = True
            break
        critical_dialog = find_exact_for_process(
            "IntelBurnTest - Critical Error", proc.pid
        )
        if critical_dialog:
            break
        time.sleep(0.03)
    missing = []
    if not captured_reference:
        missing.append("REFERENCE_CAPTURE")
    if not captured_match:
        missing.append("MATCH_CAPTURE")
    if missing:
        proc.kill()
        raise SystemExit("MISSING_" + "_AND_".join(missing))
    if critical_dialog:
        save_window("09-critical-error", critical_dialog)
        proc.kill()
        raise SystemExit("BENCHMARK_REPORTED_CRITICAL_ERROR")
    if not finished:
        proc.kill()
        raise SystemExit("NO_FINISHED_STATE")
    time.sleep(0.25)
    save_window("09-finished", main_hwnd)

    log_path = os.path.join(RUN_DIR, "results.log")
    if not os.path.isfile(log_path) or os.path.getsize(log_path) < 80:
        proc.kill()
        raise SystemExit("RESULT_LOG_NOT_WRITTEN")
    with open(log_path, "rb") as handle:
        print("results.log", os.path.getsize(log_path), handle.read(48))

    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
    print("visual QA captures:", OUT)


if __name__ == "__main__":
    main()
