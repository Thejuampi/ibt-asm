# IntelBurnTest standalone UEFI

This port boots directly from a USB drive on an **x64 UEFI PC**. It loads no
operating system, C runtime, DLL, Intel LINPACK executable, or external numerical
library. The application, platform bridges, and numerical paths are NASM assembly.

UEFI initializes the machine and provides the console, keyboard, page allocator,
timer, and multiprocessor dispatcher. Boot Services stay active throughout the
test; this application does not call `ExitBootServices()`. It is an OS-free
firmware application, rather than a kernel with its own USB and chipset drivers.

## Build

Requirements: Python 3, NASM, and LLVM's `lld-link`. No Windows SDK, EDK II build,
GNU-EFI library, C compiler, or local operating-system image is required.

From the repository root, on Windows or Linux:

```console
python uefi/build.py
```

The build validates that the result is a relocatable x64 PE32+ EFI application
with **no imports**, then writes:

```text
uefi/out/release/EFI/BOOT/BOOTX64.EFI
uefi/out/release/ibt-uefi-usb.zip
uefi/out/release/ibt-uefi.img
```

The ZIP contains the boot directory and usage instructions. The 64 MiB raw disk
image contains an MBR and a valid FAT32 partition with the same boot executable.
Its size is the size of the USB image, not the application's memory requirement.
Override tools with `--nasm PATH` and `--lld-link PATH` when necessary.

On Windows, `python uefi/build.py --installer` also builds
`IntelBurnTest-USB-Setup.exe`. The installer embeds the matching release EFI
and instructions; its separate Windows implementation is described in
[`installer/README.md`](installer/README.md).

## Boot from USB

1. On Windows, open **IntelBurnTest-USB-Setup.exe**, select your USB, and click
   **Prepare USB**. FAT32 USBs keep their files. Other USBs require explicitly
   selecting the format option and typing **ERASE** in a confirmation dialog.
   Formatting deletes every partition and file on the selected USB; back up
   anything you want to keep first.
2. The executable is unsigned. Boot with Secure Boot disabled, or
   sign it with a key trusted by your firmware.
3. Restart and choose the USB's **UEFI** entry in the firmware's boot menu.
4. Configure **Size**, **Threads**, and **Runs**, then press **Enter**.

For manual installation, extract `ibt-uefi-usb.zip` into the root of an existing
FAT32 USB partition. The resulting path must be `EFI/BOOT/BOOTX64.EFI`; preserve
an existing file at that path before replacing it.

Alternatively, write `ibt-uefi.img` as a raw disk image using your USB imaging
tool. Writing a raw image replaces the destination drive's partition table and
contents. The build script creates ordinary files and never opens a disk device.
Legacy BIOS/CSM boot and 32-bit UEFI are not supported.

| Key | Action |
|---|---|
| **Enter** | Start; after a result, rerun with the same settings |
| **M** | Edit Size in MiB; **A** uses the available maximum |
| **R** | Enter the number of runs, from 2 to 100000 |
| **T** | Edit Threads; **A** selects all available compute threads |
| **D** | Reset settings to defaults: up to 1024 MiB, all compute threads, 10 runs, Auto |
| **C** | Change settings after a result |
| **F2** | Advanced: choose Auto, SSE2, AVX, or AVX2+FMA; unsupported modes are unavailable |
| **H / F1** | Help with settings, result columns, and test outcomes |
| **Escape** | Request stop; also cancel numeric input |
| **Q** | Return to firmware |

The console identifies the product as **Intel Burn Test 3.2**. Configuration,
execution, and results use the same bounded view. During a test, it displays
the current run, elapsed time, calculation stage, and completed-run progress.
The table aligns time, GFLOPS, residual, and a per-run result; long sessions
show the latest eight runs with their original run numbers. Timing estimates
are marked `~` instead of filling the main screen with platform details.

Numeric editing shows the current value and valid range. Empty **Enter** keeps
the value, **Escape** cancels, and invalid values leave the editor open with an
inline explanation. Results remain visible until **Rerun**, **Change settings**,
or **Exit** is selected. Error messages suggest a concrete next action. Advanced
instruction controls and explanatory help are separate from the primary flow.

## Numerical paths

**Auto selects AVX2+FMA, then AVX, then SSE2**, according to the capabilities
shared by the participating processors. The app initializes SSE/XSAVE/XCR0 on
each computing CPU instead of depending on an operating system to enable AVX.
An ISA limit can only reduce the selection; it cannot enable unsupported
instructions. Selection is based on CPU capabilities, not a watt measurement.
Firmware power limits, clock policy, and cooling still affect the workload.

Both the native AVX paths and the SSE2 path are compiled from the repository's
`ibt_lpk.inc` and `bench_lib.inc`. The UEFI build's SSE2 variant lowers scalar
VEX operations to their legacy SSE2 equivalents. The fast AVX/FMA variant retains
native VEX instructions. Small opt-in `IBT_PORTABLE_ISA` guards fix the inherited
four-column routine's unconditional FMA dependency and make AVX packing use
eight-row panels consistently. These guards are disabled in the existing
Windows and Unix builds.

Each pass generates the same deterministic matrix, factors and solves it, then
computes the normalized residual. A completed pass must have a finite,
nonnegative residual no greater than 16. **PASSED** requires all requested runs
and bit-for-bit matching normalized residuals. A cancelled test, an allocation
failure, or a multiprocessor error never produces a stability pass.

## Firmware and memory behavior

- Parallel execution uses blocking PI MP Services `StartupAllAPs`, which remains
  supported after `ReadyToBoot`; it does not require asynchronous firmware AP
  dispatch or operating-system threads.
- The BSP stays with firmware and input. Up to **32 enabled, healthy APs** run
  the numerical engine, with one AP coordinating the other workers. A machine
  with 16 enabled logical processors offers 15 parallel compute threads. With
  one selected compute thread, the BSP runs the test directly. Missing MP
  Services or fewer than two available APs reduces the maximum to one thread.
- Memory and all per-CPU scratch buffers are allocated on the BSP before any
  AP runs. APs make no UEFI calls. Buffers are released only after blocking MP
  dispatch has returned and every AP has stopped using them.
- Only `EfiConventionalMemory` is counted. The maximum matrix size accounts for
  the largest contiguous range, padded matrix rows, vectors, packed panels,
  per-CPU scratch, allocation headers, and 32 MiB of firmware headroom. The UI
  currently caps matrices at 64 GiB. Allocation failure remains possible if
  firmware memory availability changes; reduce the selected memory and retry.
- A periodic firmware event checks Escape while computing. The callback
  preserves the enabled extended CPU state, including AVX registers. Firmware
  may defer callbacks or input while it dispatches APs; the request is also
  checked between passes. Immediate cancellation is not guaranteed on every
  firmware implementation.
- The boot watchdog is disabled during the application. Timing uses a TSC
  frequency from CPUID or calibration against firmware `Stall`. When invariant
  TSC is unavailable, the UI labels timing and GFLOPS as estimates.
- Results are displayed in the console and remain in memory. File logging,
  temperature monitoring, graphical parity with the desktop UI, and legacy
  BIOS support are not implemented in this port.

## Emulator verification

Install QEMU and OVMF on the test host. For example, in Debian/Ubuntu or WSL:

```console
sudo apt-get install qemu-system-x86 ovmf
```

Build the emulator-only self-test on the Windows build host:

```console
python uefi/build.py --qa
```

Then on a Linux/WSL test host, from the repository root:

```console
python3 uefi/qemu_check.py uefi/out/qa-normal/ibt-uefi.img --cpu max --smp 4 --expect-isa "AVX2 + FMA" --expect-cases 39
python3 uefi/qemu_check.py uefi/out/qa-normal/ibt-uefi.img --cpu SandyBridge --smp 4 --expect-isa AVX --expect-cases 26
python3 uefi/qemu_check.py uefi/out/qa-normal/ibt-uefi.img --cpu Conroe --smp 4 --expect-isa SSE2 --expect-cases 13
```

The self-test runs each available ISA at matrix sizes 8, 24, 40, 68, 72, 96,
256, 384, 400, 768, and 1024. It repeats each pass, exercises parallel cases at
400 and 768, and checks that cleanup returns all application-allocated pages.
Conroe exposes no AVX; Sandy Bridge exposes AVX without AVX2 or FMA. These CPU
models verify that fallback paths run on CPUs actually missing those extensions.
Emulation verifies software behavior, not physical cooling or maximum power.

Fault-injection builds exercise early allocation failure, the last matrix
packing-buffer allocation failure, a residual mismatch hidden below displayed
precision, and cancellation. Example:

```console
python uefi/build.py --qa --case pack-alloc
python3 uefi/qemu_check.py uefi/out/qa-pack-alloc/ibt-uefi.img
```

Other case names: `alloc`, `mismatch`, and `stop`. `--log FILE` retains the serial
test transcript and JSON evidence, including image SHA-256 and selected ISA.
Pass `--ovmf-code PATH` and `--ovmf-vars PATH` for nonstandard firmware locations.
QA images contain emulator debug-port I/O and are **not** the USB distributable.
The release build excludes that instrumentation.

The release image also has an interactive-console regression check. On the
Linux/WSL test host, after `python uefi/build.py`:

```console
python3 uefi/ui_check.py uefi/out/release/ibt-uefi.img uefi/out/ui-verified
```

It sends keyboard input through QEMU, verifies completed runs in Auto, SSE2,
and AVX modes, checks help, editing, inline validation, Reset, Rerun, bounded
result history, live progress, and cancellation of a running test with Escape.
It reads the matching release linker map to synchronize with application state
and saves screenshots for visual review. This check uses the release executable,
without emulator-only fault injection.

## Sources

- [UEFI removable-media boot behavior](https://uefi.org/specs/UEFI/2.11/03_Boot_Manager.html)
- [UEFI x64 execution and calling conventions](https://uefi.org/specs/UEFI/2.11/02_Overview.html)
- [TianoCore's PI MP Services definitions and dispatch contracts](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/MpService.h)
