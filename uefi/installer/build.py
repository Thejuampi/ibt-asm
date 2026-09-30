"""Build a single Windows x64 USB installer with the matching EFI payload."""
import hashlib
import os
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
UEFI = HERE.parent
OUT = UEFI / "out/release"
BUILD = UEFI / "build/installer"


def build():
    efi = OUT / "EFI/BOOT/BOOTX64.EFI"
    readme = OUT / "README.TXT"
    if not efi.is_file() or not readme.is_file():
        raise SystemExit("First run: python uefi/build.py")
    BUILD.mkdir(parents=True, exist_ok=True)
    generated = BUILD / "BuildPayload.cs"
    generated.write_text(
        'namespace IntelBurnTestUsb { internal static class BuildPayload {\n' +
        f'  internal const string EfiHash = "{hashlib.sha256(efi.read_bytes()).hexdigest()}";\n' +
        f'  internal const string ReadmeHash = "{hashlib.sha256(readme.read_bytes()).hexdigest()}";\n' +
        '} }\n', encoding="ascii")
    compiler = Path(os.environ.get("WINDIR", "C:/Windows")) / "Microsoft.NET/Framework64/v4.0.30319/csc.exe"
    if not compiler.is_file():
        raise SystemExit("Build on Windows with .NET Framework 4.8 installed.")
    shared = [str(HERE / name) for name in ("Core.cs", "Hardware.cs", "Desktop.cs")] + [str(generated)]
    common = [str(compiler), "/nologo", "/optimize+", "/platform:x64", "/warnaserror+", "/utf8output",
              "/reference:System.Windows.Forms.dll", "/reference:System.Drawing.dll", "/reference:System.Management.dll",
              f"/resource:{efi},IBT.EFI", f"/resource:{readme},IBT.README"]
    target = OUT / "IntelBurnTest-USB-Setup.exe"
    subprocess.run(common + ["/target:winexe", "/main:IntelBurnTestUsb.Program",
                   f"/win32manifest:{HERE / 'app.manifest'}", f"/win32icon:{UEFI.parent / 'res/app.ico'}",
                   f"/out:{target}"] + shared, check=True)
    if (HERE / "Checks.cs").is_file():
        qa = BUILD / "InstallerChecks.exe"
        subprocess.run(common + ["/target:exe", "/main:IntelBurnTestUsb.Checks", f"/out:{qa}"] +
                       shared + [str(HERE / "Checks.cs")], check=True)
    print(f"Installer: {target} ({target.stat().st_size:,} bytes)")
    return target


if __name__ == "__main__":
    build()
