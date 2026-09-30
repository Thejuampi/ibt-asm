"""Drive the release EFI console through QMP and capture its actual VGA output.

Run on Linux/WSL with QEMU and OVMF. Review the resulting PPM screenshots;
this driver does not infer a stability verdict from a fixed delay.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import socket
import struct
import subprocess
import tempfile
import time


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("image", type=Path)
    p.add_argument("output", type=Path)
    p.add_argument("--boot-wait", type=float, default=5)
    p.add_argument("--map", type=Path, help="linker map from the matching release build")
    a = p.parse_args()
    output = a.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    qemu = shutil.which("qemu-system-x86_64")
    if not qemu:
        p.error("qemu-system-x86_64 is required")
    map_path = a.map or Path(__file__).resolve().parent / "build/release/ibt_uefi.map"
    symbols = {name: int(address, 16) for name, address in re.findall(
        r"^\s*[\da-fA-F]+:[\da-fA-F]+\s+(\w+)\s+([\da-fA-F]{16})",
        map_path.read_text(), re.MULTILINE)}
    for required in ("szHeader", "testActive", "passCount", "lpNorm0", "stopRequested",
                     "uiMode", "uiResultCount", "uiStartTick", "uiElapsed"):
        if required not in symbols:
            p.error(f"{map_path} lacks {required}; rebuild the release with build.py")
    with tempfile.TemporaryDirectory(prefix="ibt-ui-") as temp:
        work = Path(temp)
        shutil.copyfile("/usr/share/OVMF/OVMF_VARS_4M.fd", work / "vars.fd")
        qmp_socket = work / "qmp.sock"
        args = [qemu, "-machine", "q35", "-accel", "tcg,thread=multi",
                "-cpu", "max", "-smp", "4", "-m", "256", "-display", "none", "-vga", "virtio",
                "-serial", "none", "-monitor", "none", "-net", "none", "-no-reboot",
                "-qmp", f"unix:{qmp_socket},server=on,wait=off",
                "-drive", "if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd",
                "-drive", f"if=pflash,format=raw,file={work / 'vars.fd'}",
                "-device", "qemu-xhci,id=xhci", "-device", "usb-kbd,bus=xhci.0",
                "-drive", f"if=none,id=stick,format=raw,readonly=on,file={a.image.resolve()}",
                "-device", "usb-storage,bus=xhci.0,drive=stick,bootindex=1"]
        with (output / "qemu-ui.log").open("wb") as errors:
            process = subprocess.Popen(args, stdout=errors, stderr=subprocess.STDOUT)
            connection = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            connection.settimeout(10)
            try:
                deadline = time.monotonic()+10
                while not qmp_socket.exists():
                    if time.monotonic() > deadline or process.poll() is not None:
                        raise RuntimeError("QEMU failed to open QMP")
                    time.sleep(0.1)
                connection.connect(str(qmp_socket))
                stream = connection.makefile("rwb")
                json.loads(stream.readline())

                def command(name, arguments=None):
                    request = {"execute": name}
                    if arguments is not None:
                        request["arguments"] = arguments
                    stream.write(json.dumps(request).encode()+b"\n")
                    stream.flush()
                    while True:
                        response = json.loads(stream.readline())
                        if "error" in response:
                            raise RuntimeError(response["error"])
                        if "return" in response:
                            return response["return"]

                def key(name):
                    before = state("uiKeySerial")
                    mode = state("uiMode")
                    command("send-key", {"keys": [{"type": "qcode", "data": name}], "hold-time": 50})
                    time.sleep(0.15)
                    if mode != 1:
                        wait_for(lambda: state("uiKeySerial") != before)
                        # Starting a test enters computation rather than the next input prompt.
                        if not (mode in (0, 2) and name in ("ret", "s", "r")):
                            wait_for(lambda: state("uiAwaitKey") == 1)

                def keys(sequence):
                    for name in sequence:
                        key(name)

                def screenshot(name):
                    wait_for(lambda: state("uiBusy") == 0 and
                             (state("uiMode") == 1 or state("uiAwaitKey") == 1))
                    target = output / (name+".ppm")
                    command("screendump", {"filename": str(target)})
                    print(f"Captured {target}", flush=True)

                def memory(address, length):
                    target = work / "guest.bin"
                    command("pmemsave", {"val": address, "size": length, "filename": str(target)})
                    return target.read_bytes()

                # Locate the relocated application, without assuming its load
                # address. The data section starts with the relocated entry
                # anchor, which distinguishes it from firmware disk caches.
                def image_base():
                    efi = (a.image.parent / "EFI/BOOT/BOOTX64.EFI").read_bytes()
                    pe = struct.unpack_from("<I", efi, 0x3C)[0]
                    entry = struct.unpack_from("<I", efi, pe+24+16)[0]
                    sections = struct.unpack_from("<H", efi, pe+6)[0]
                    table = pe+24+struct.unpack_from("<H", efi, pe+20)[0]
                    data_rva = next(struct.unpack_from("<I", efi, table+i*40+12)[0]
                                    for i in range(sections) if efi[table+i*40:table+i*40+8].rstrip(b"\0") == b".data")
                    scan_start = 224*1024*1024
                    data = memory(scan_start, 32*1024*1024)
                    marker = b"Intel Burn Test 3.2"
                    start = 0
                    while True:
                        offset = data.find(marker, start)
                        if offset == -1:
                            raise RuntimeError("unable to locate the relocated EFI application")
                        base = scan_start+offset-symbols["szHeader"]
                        anchor_offset = base+data_rva-scan_start
                        if 0 <= anchor_offset <= len(data)-8 and struct.unpack_from("<Q", data, anchor_offset)[0] == base+entry:
                            return base
                        start = offset+1

                def state(name, size=4):
                    return int.from_bytes(memory(base+symbols[name], size), "little")

                def wait_for(condition, timeout=60):
                    deadline = time.monotonic()+timeout
                    while not condition():
                        if time.monotonic() > deadline:
                            raise TimeoutError("console state did not reach the expected condition")
                        time.sleep(0.1)

                def completed(previous_tick, runs=2):
                    wait_for(lambda: state("uiMode") == 2 and state("passCount") == runs
                             and state("uiStartTick", 8) != previous_tick, timeout=120)
                    if state("uiResultCount") != runs:
                        raise AssertionError("result table has an incorrect run count")
                    for flag in ("failed", "lpAllocFail", "dispatchFailed", "stopRequested"):
                        if state(flag):
                            raise AssertionError(f"completed UI run has {flag} set")
                    wait_for(lambda: state("uiBusy") == 0 and state("uiAwaitKey") == 1)
                    time.sleep(0.2)

                command("qmp_capabilities")
                time.sleep(a.boot_wait)
                base = image_base()
                wait_for(lambda: state("uiMode") == 0)
                screenshot("01-ready")
                key("h")
                wait_for(lambda: state("uiMode") == 4)
                screenshot("02-help")
                key("esc")
                wait_for(lambda: state("uiMode") == 0)
                keys(["m", "1", "ret", "r", "2", "0", "backspace", "ret"])
                if state("stressMB") != 1 or state("targetRuns") != 2:
                    screenshot("debug-edit")
                    raise AssertionError(f"numeric editing/backspace did not apply: Size={state('stressMB')}, "
                                         f"Runs={state('targetRuns')}, mode={state('uiMode')}")
                keys(["m", "9", "esc"])
                if state("stressMB") != 1:
                    raise AssertionError("Escape did not cancel numeric input")
                keys(["m", "0", "ret"])
                wait_for(lambda: state("uiEditError") == 1)
                if state("stressMB") != 1 or state("uiMode") != 3:
                    raise AssertionError("invalid input replaced the previous setting")
                screenshot("03-invalid-input")
                keys(["1", "ret", "ret"])
                completed(0)
                if state("lpIsaUse") != 2 or state("lpIsaForce") != 0:
                    raise AssertionError("default mode did not auto-select AVX2 + FMA")
                screenshot("04-auto-result")
                previous_tick = state("uiStartTick", 8)
                previous_norm = state("lpNorm0", 8)
                key("ret")
                completed(previous_tick)
                if state("lpNorm0", 8) != previous_norm:
                    raise AssertionError("Rerun did not reproduce the same result")
                screenshot("05-rerun-result")
                for selector, isa, name in [("1", 0, "06-sse2-result"), ("2", 1, "07-avx-result")]:
                    previous_tick = state("uiStartTick", 8)
                    keys(["c", "f2", selector, "ret", "ret"])
                    completed(previous_tick)
                    if state("lpIsaUse") != isa:
                        raise AssertionError("Advanced did not select the requested mode")
                    screenshot(name)
                keys(["c", "d"])
                wait_for(lambda: state("uiMode") == 0)
                if (state("targetRuns") != 10 or state("chosenThreads") != state("nproc")
                        or state("lpIsaForce") != 0 or state("stressMB") != min(1024, state("maxStressMB"))):
                    raise AssertionError("Reset did not restore the defaults")
                screenshot("08-reset")
                previous_tick = state("uiStartTick", 8)
                keys(["m", "1", "ret", "ret"])
                completed(previous_tick, runs=10)
                expected = [9, 10, 3, 4, 5, 6, 7, 8]
                history = state("uiResults", 1024)
                if [history >> (i*128*8) & 0xffffffff for i in range(8)] != expected:
                    raise AssertionError("latest-run table did not retain its bounded history")
                screenshot("09-latest-runs")
                keys(["c", "m", "8", "ret", "r", "2", "ret"])
                wait_for(lambda: state("stressMB") == 8)
                time.sleep(1)
                key("ret")
                wait_for(lambda: state("testActive") == 1)
                wait_for(lambda: state("uiElapsed", 8) >= 2)
                screenshot("10-running")
                key("f7")
                if state("stopRequested"):
                    raise AssertionError("F7 was incorrectly treated as Escape")
                key("esc")
                wait_for(lambda: state("stopRequested") == 1 and state("uiMode") == 2)
                if state("passCount") >= 2:
                    raise AssertionError("cancelled UI run completed the requested test")
                time.sleep(1)
                screenshot("11-stop-result")
                command("quit")
                process.wait(timeout=10)
            finally:
                connection.close()
                if process.poll() is None:
                    process.kill()
                    process.wait()
    (output / "result.json").write_text(json.dumps({
        "result": "passed", "image_sha256": hashlib.sha256(a.image.read_bytes()).hexdigest(),
        "checks": ["configuration", "help", "inline validation", "backspace", "input cancellation",
                   "automatic AVX2 + FMA", "rerun", "SSE2", "AVX", "reset", "latest runs",
                   "live progress", "keyboard cancellation"],
        "screenshots": [p.name for p in sorted(output.glob("*.ppm"))]
    }, indent=2)+"\n")
    print("Console UX, three ISA modes, Rerun, Reset, and keyboard cancellation passed; inspect captures.")


if __name__ == "__main__":
    main()
