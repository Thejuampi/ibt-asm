"""Boot an IBT QA USB image under OVMF. Requires QEMU and OVMF firmware."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time


def check(image, cpu="max", smp=4, timeout=240, code=None, variables=None,
          log_path=None, expected_isa=None, expected_cases=None):
    qemu = shutil.which("qemu-system-x86_64")
    if not qemu:
        raise SystemExit("qemu-system-x86_64 is required (Windows or Linux/WSL).")
    code = Path(code or "/usr/share/OVMF/OVMF_CODE_4M.fd")
    variables = Path(variables or "/usr/share/OVMF/OVMF_VARS_4M.fd")
    if not code.is_file() or not variables.is_file():
        raise SystemExit("Pass --ovmf-code and --ovmf-vars for the installed OVMF firmware.")
    with tempfile.TemporaryDirectory(prefix="ibt-uefi-") as temp:
        work = Path(temp)
        var_copy = work / "vars.fd"
        shutil.copyfile(variables, var_copy)
        log = work / "debug.log"
        args = [qemu, "-machine", "q35", "-accel", "tcg,thread=multi",
                "-cpu", cpu, "-smp", str(smp), "-m", "256", "-display", "none",
                "-monitor", "none", "-serial", "none", "-net", "none", "-no-reboot",
                "-drive", f"if=pflash,format=raw,readonly=on,file={code}",
                "-drive", f"if=pflash,format=raw,file={var_copy}",
                "-device", "qemu-xhci,id=xhci",
                "-drive", f"if=none,id=stick,format=raw,readonly=on,file={Path(image).resolve()}",
                "-device", "usb-storage,bus=xhci.0,drive=stick,bootindex=1",
                "-debugcon", f"file:{log}", "-global", "isa-debugcon.iobase=0xe9",
                "-device", "isa-debug-exit,iobase=0xf4,iosize=0x04"]
        started = time.monotonic()
        printed = 0
        with (work / "qemu.log").open("w+b") as errors:
            process = subprocess.Popen(args, stdout=errors, stderr=subprocess.STDOUT)
            try:
                while process.poll() is None:
                    if log.exists():
                        data = log.read_bytes()
                        if len(data) > printed:
                            print(data[printed:].decode("ascii", errors="replace"), end="", flush=True)
                            printed = len(data)
                    if time.monotonic()-started > timeout:
                        raise TimeoutError(f"UEFI boot/test timed out after {timeout}s")
                    time.sleep(0.2)
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait()
            data = log.read_bytes() if log.exists() else b""
            print(data[printed:].decode("ascii", errors="replace"), end="", flush=True)
            errors.seek(0)
            error_text = errors.read().decode(errors="replace")
            if log_path:
                Path(log_path).parent.mkdir(parents=True, exist_ok=True)
                Path(log_path).write_bytes(data)
            if process.returncode != 33 or b"IBT_QA_OK" not in data or b"IBT_QA_FAIL" in data:
                raise RuntimeError(f"QEMU exit={process.returncode}\n{error_text}")
        isa = re.findall(rb"(?:^|\n)(SSE2|AVX|AVX2 \+ FMA)\r?\n", data)
        if expected_isa and (not isa or isa[0].decode() != expected_isa):
            raise RuntimeError(f"automatic ISA selection differs from {expected_isa}")
        cases = re.search(rb"QA cases completed: (\d+)", data)
        completed = int(cases[1]) if cases else 0
        if expected_cases is not None and completed != expected_cases:
            raise RuntimeError(f"expected {expected_cases} numerical cases, got {completed}")
        elapsed = time.monotonic()-started
        if log_path:
            Path(str(log_path)+".json").write_text(json.dumps({
                "cpu": cpu, "firmware_cpus": smp, "elapsed_seconds": round(elapsed, 2),
                "image_sha256": hashlib.sha256(Path(image).read_bytes()).hexdigest(),
                "detected_isa": isa[0].decode() if isa else None,
                "numerical_cases": completed, "result": "passed"
            }, indent=2)+"\n", encoding="utf-8")
        print(f"Boot/self-test passed: CPU={cpu}, firmware CPUs={smp}, {elapsed:.1f}s")
        return data


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("image", type=Path)
    p.add_argument("--cpu", default="max")
    p.add_argument("--smp", type=int, default=4)
    p.add_argument("--timeout", type=int, default=240)
    p.add_argument("--ovmf-code")
    p.add_argument("--ovmf-vars")
    p.add_argument("--log", type=Path)
    p.add_argument("--expect-isa", choices=["SSE2", "AVX", "AVX2 + FMA"])
    p.add_argument("--expect-cases", type=int)
    a = p.parse_args()
    check(a.image, a.cpu, a.smp, a.timeout, a.ovmf_code, a.ovmf_vars,
          a.log, a.expect_isa, a.expect_cases)


if __name__ == "__main__":
    main()
