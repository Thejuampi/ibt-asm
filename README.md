# IntelBurnTest ASM

A compact, native x64 Windows stress test written entirely in NASM assembly. The numerical core runs in-process and has no dependency on Intel LINPACK, MKL, the C runtime, .NET, or external DLLs beyond standard Windows system libraries.

The application provides a permanent dark interface, runtime ISA selection, optional result logging, and an animated thermal-load indicator. It targets modern Intel processors while retaining SSE2, AVX, and AVX2 execution paths.

> [!WARNING]
> This program intentionally drives the CPU at sustained maximum load. Use adequate cooling and monitor temperatures. You assume all risk associated with stress testing.

## Build

Requirements:

- Windows 10 or 11, x64
- NASM
- Visual Studio 2022 Build Tools with the Windows SDK
- GNU Make
- Python 3 for validation helpers
- Pillow only for regenerating graphical assets or running visual QA
- UPX only for the optional packed distribution build

Open an **x64 Native Tools Command Prompt for VS 2022**, change to the repository directory, and run:

```console
make
```

The executable is written to `bin/IntelBurnTest.exe`.

Useful targets:

```console
make selftest  # compile and run the internal numerical validation build
make visual    # exercise and capture the complete UI state set
make size      # print a routine-level code-size report
make assets    # regenerate the icon and bitmap resources
make packed    # create dist/IntelBurnTest.exe using UPX/LZMA
make clean
```

Tool names can be overridden when necessary, for example:

```console
make NASM=C:\tools\nasm.exe PYTHON=py
```

## Runtime behavior

- The benchmark and validation calculations run entirely in memory.
- No configuration or theme files are read.
- `results.log` is the only persistent runtime output, and is created only when logging is enabled.
- ISA capabilities are detected at startup; the best supported SSE2, AVX, or AVX2 path is selected automatically.
- The timed benchmark loops are alignment-checked during every normal build.

## Repository layout

```text
ibt.asm             Win64 entry point, state, resources, and shared definitions
ibt_ui.inc          window procedure and application behavior
ibt_theme.inc       dark-theme and custom-control drawing
ibt_lpk.inc         benchmark orchestration and numerical routines
bench_lib.inc       optimized matrix kernels
res/                minimal source and compiled graphical assets
tools/              asset generation, alignment checks, size report, and visual QA
```

## Performance validation

Performance-sensitive kernels are kept separate from UI and packaging code. `tools/asmcheck.py` verifies the expected hot-loop alignment residues after assembly. For meaningful comparisons, run repeated A/B tests under the same power plan, temperature, memory size, thread count, and background load.

## Credits

Created by **Thejuampi**. The original IntelBurnTest interface was created by AgentGOD; this repository is an independent native assembly rewrite of the benchmark application.

No license is granted unless a license file is added explicitly.
