"""Build a dependency-free x64 EFI application and package a USB image."""
import argparse
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import zipfile

from make_usb_image import make_image

HERE = Path(__file__).resolve().parent
QA_CASES = {"normal": 0, "alloc": 1, "pack-alloc": 2, "mismatch": 3, "stop": 4}


def tool(name, override=None):
    if override:
        return override
    found = shutil.which(name)
    if found:
        return found
    if os.name == "nt":
        candidate = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "LLVM/bin" / (name + ".exe")
        if candidate.is_file():
            return str(candidate)
    raise SystemExit(f"Missing {name}; install NASM and LLVM/lld or pass --{name}.")


def validate_pe(path):
    data = path.read_bytes()
    if data[:2] != b"MZ":
        raise ValueError("EFI image has no DOS header")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe+4] != b"PE\0\0":
        raise ValueError("EFI image has no PE header")
    machine = struct.unpack_from("<H", data, pe+4)[0]
    opt = pe+24
    magic = struct.unpack_from("<H", data, opt)[0]
    subsystem = struct.unpack_from("<H", data, opt+68)[0]
    if (machine, magic, subsystem) != (0x8664, 0x20B, 10):
        raise ValueError("expected an x64 PE32+ EFI application")
    if struct.unpack_from("<II", data, opt+120) != (0, 0):
        raise ValueError("EFI image must have no DLL imports")
    if not all(struct.unpack_from("<II", data, opt+112+5*8)):
        raise ValueError("EFI image must be relocatable")
    if not struct.unpack_from("<I", data, opt+16)[0]:
        raise ValueError("EFI image must have an entry point")
    return len(data)


def build(qa=False, case="normal", nasm=None, linker=None):
    build_dir = HERE / "build" / (f"qa-{case}" if qa else "release")
    output = HERE / "out" / (f"qa-{case}" if qa else "release")
    boot = output / "EFI/BOOT"
    build_dir.mkdir(parents=True, exist_ok=True)
    boot.mkdir(parents=True, exist_ok=True)
    obj = build_dir / "ibt_uefi.obj"
    target = boot / "BOOTX64.EFI"
    # Only labels/constants in the numerical sources are renamed. Shared job
    # state and firmware bridges intentionally stay common to both variants.
    symbols = set()
    for name in ("ibt_lpk.inc", "bench_lib.inc"):
        source = (HERE.parent / name).read_text(encoding="utf-8")
        symbols.update(re.findall(r"^([A-Za-z_]\w*):", source, re.MULTILINE))
        symbols.update(re.findall(r"^([A-Za-z_]\w*)\s+(?:dq|dw|db)\b", source, re.MULTILINE))
    (build_dir / "sse2_symbols.inc").write_text(
        "".join(f"%define {s} uefi_sse2_{s}\n" for s in sorted(symbols)), encoding="ascii")
    (build_dir / "sse2_symbols_end.inc").write_text(
        "".join(f"%undef {s}\n" for s in sorted(symbols)), encoding="ascii")
    args = [tool("nasm", nasm), "-f", "win64", "-O2", "-w+error",
            f"-DUEFI_QA={int(qa)}", f"-DUEFI_QA_CASE={QA_CASES[case]}",
            "-I", str(build_dir)+os.sep,
            "-l", str(build_dir / "ibt_uefi.lst"), "-o", str(obj), "ibt_uefi.asm"]
    subprocess.run(args, cwd=HERE, check=True)
    subprocess.run([tool("lld-link", linker), "/nologo", "/subsystem:efi_application",
                    "/entry:efi_main", "/nodefaultlib", "/machine:x64", "/base:0",
                    "/dynamicbase", "/nxcompat", "/timestamp:0", "/opt:ref",
                    f"/map:{build_dir / 'ibt_uefi.map'}",
                    f"/out:{target}", str(obj)], cwd=HERE, check=True)
    size = validate_pe(target)
    readme = (
        "Intel Burn Test 3.2\r\n\r\n"
        "Boot this USB from the firmware's UEFI boot menu.\r\n"
        "Secure Boot: this development binary is unsigned.\r\n"
        "Automatic SSE2 / AVX / AVX2+FMA selection.\r\n"
        "Enter=Start / Rerun M=Size R=Runs T=Threads D=Reset\r\n"
        "C=Change settings F2=Advanced H=Help Escape=Stop Q=Exit\r\n"
        "Parallel mode reserves the BSP for firmware; up to 32 APs compute.\r\n"
        "Input response depends on firmware; it can wait until a pass ends.\r\n"
        "Use adequate CPU cooling. Results stay in memory.\r\n"
    ).encode("ascii")
    (output / "README.TXT").write_bytes(readme)
    disk = output / "ibt-uefi.img"
    make_image(target.read_bytes(), readme, disk)
    if not qa:
        archive = output / "ibt-uefi-usb.zip"
        with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED) as z:
            z.write(target, "EFI/BOOT/BOOTX64.EFI")
            z.writestr("README.TXT", readme)
    print(f"EFI: {target} ({size:,} bytes; no imports; relocatable)")
    print(f"USB image: {disk}")
    return target, disk


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--qa", action="store_true", help="build an emulator-only self-test")
    p.add_argument("--installer", action="store_true", help="also build the Windows USB installer")
    p.add_argument("--case", choices=QA_CASES, default="normal")
    p.add_argument("--nasm")
    p.add_argument("--lld-link", dest="linker")
    args = p.parse_args()
    if args.case != "normal" and not args.qa:
        p.error("fault injection is allowed only with --qa")
    if args.installer and args.qa:
        p.error("the installer must embed the release build")
    build(args.qa, args.case, args.nasm, args.linker)
    if args.installer:
        from installer.build import build as build_installer
        build_installer()


if __name__ == "__main__":
    main()
