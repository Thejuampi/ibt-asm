# IntelBurnTest for Unix

Native Linux/x86-64 port using X11. The window reproduces the Windows edition's
620 x 400 client area, control geometry, dark palette, original flame and
coffee assets, run states, residual comparison, and optional `results.log`
output. The calculation is not a reimplementation: this build includes the
repository's existing
`ibt_lpk.inc` and `bench_lib.inc` directly.

Large calculation buffers are aligned for 2 MiB transparent huge pages and
use Linux's best-effort `MADV_HUGEPAGE` hint to reduce TLB overhead.

## Build

Requirements: NASM, a C linker, pthreads, the X11 and Xft development libraries,
and UPX.

```sh
cd unix
make
./IntelBurnTest
```

The build produces `IntelBurnTest.unpacked` for debugging, packs the distributable
`IntelBurnTest` with UPX/LZMA, runs a headless numerical self-test, and fails if
the distributable exceeds 23 KiB (23,552 bytes). You can repeat the test with:

```sh
./IntelBurnTest --selftest
```

For UI smoke testing, `./IntelBurnTest --smoketest` opens a 1 MB, two-run test
with logging enabled; close the window after the result appears.

The native target is Linux on x86-64. Other Unix systems need small changes in
the syscall numbers used only for logging and process exit.
