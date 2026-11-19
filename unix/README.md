# IntelBurnTest for Unix

Native Linux/x86-64 port using X11. The window reproduces the Windows edition's
620 x 400 client area, control geometry, dark palette, original flame and
coffee assets, run states, residual comparison, and optional `results.log`
output. The calculation is not a reimplementation: this build includes the
repository's existing
`ibt_lpk.inc` and `bench_lib.inc` directly.

Large calculation buffers are aligned for 2 MiB transparent huge pages and
use Linux's best-effort `MADV_HUGEPAGE` hint to reduce TLB overhead.

`ibt_unix.asm` is the small assembly entrypoint. Its ordered includes keep
state (`ibt_state.inc`), startup (`ibt_app.inc`), X11 setup (`ibt_window.inc`),
events (`ibt_events.inc`), layout (`ibt_layout.inc`), drawing primitives
(`ibt_draw_primitives.inc`), and core/runtime bridges (`ibt_bridge.inc`)
separate. `ibt_draw.inc` contains the presentation macros. The original
numerical includes remain together at the end of the assembly unit.

## Build

Requirements: NASM, GNU Make, glibc, and the X11 and Xft runtime libraries.
The x86-64 glibc loader is expected at `/lib64/ld-linux-x86-64.so.2`.

```sh
cd unix
make
./IntelBurnTest
```

NASM produces the uncompressed `IntelBurnTest` executable with a 43,344-byte
limit. The ELF loader
allocates and clears the zero-initialized state in memory. The build verifies
29 numerical routines and seven helper ranges in the candidate. It checks the
X11 and Xft imports and runs a headless
self-test with six rounds on 1024 x 1024 matrices. The self-test sets the compute
thread count to four. A failed check stops the build. NASM checks the 16 KiB
instruction-span budgets for the solver and producer and the combined 32 KiB
budget with the scheduler. These limits
do not establish instruction-cache residency, throughput, or package power.
You can repeat the self-test with:

```sh
./IntelBurnTest --selftest
```

Run `make check-gui` on an X11 desktop to open the interface, check its drawing
resources, and close it automatically.

For UI smoke testing, `./IntelBurnTest --smoketest` opens a 1 MB, two-run test
with logging enabled; close the window after the result appears.

The native target is Linux on x86-64. Other Unix systems need small changes in
the syscall numbers used only for logging and process exit.
