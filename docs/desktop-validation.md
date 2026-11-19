# Replay the desktop regressions

Run the same targets as desktop CI from the repository root. Linux needs an X11 display for `check-gui`.

```console
make all check check-verifier check-gui
```

`check` measures the assembled executable and runs the numerical self-test. `check-verifier` checks both equivalent encodings and genuine numerical changes. `check-gui` starts the interface and checks platform-specific controls and resources. The Windows test also exercises the formatted result fields.

## Reject the oversized published files

Download `IntelBurnTest.exe` and `IntelBurnTest-linux-x64` from [v3.3.0](https://github.com/Thejuampi/ibt-asm/releases/tag/v3.3.0). The release checksum file identifies the original downloads. Keep the files in a separate directory. Use an absolute path for `SIZE_CHECK_FILE`.

| Host | Command | Expected failure |
| --- | --- | --- |
| Windows | `make check-size "SIZE_CHECK_FILE=C:/fixtures/IntelBurnTest.exe"` | The 98,304-byte EXE exceeds 54,784 bytes. |
| Linux | `make check-size SIZE_CHECK_FILE=/tmp/fixtures/IntelBurnTest-linux-x64` | The 172,008-byte ELF exceeds 43,344 bytes. |

NASM reports `TIMES value -1 is negative` for each oversized file. Run `make check-size` to check the executable built by the current source.

## Check the Windows application path

Run `make check-gui "GUI_CHECK_FILE=C:/fixtures/IntelBurnTest.exe"` with the v3.3.0 EXE. The target copies it to a path containing `-v` and launches it with `-g` from the build output directory. The original parser treats the pathname as a flag and enters the verifier. The test fails. Run `make check-gui` for the current executable.

## Check the Windows result fields

In a scratch copy of the current source, replace the `lp_emit` routine in `ibt_lpk.inc` with the routine from commit `2b3b01e`. Copy from `lp_emit:` up to `lp_itoa:`. Run `make check-gui`. The compiled extended self-test fails its checks for time, GFLOPS, and residual because the old formatter emits a single space-delimited record. Restore the current routine and run the target again.

## Check the tool search path

Run `make all` on a fresh checkout of commit `2b3b01e` with the same tool search path as the CI build step. Allow NASM, make, and the platform shell. The Windows recipe stops on `python tools/asmcheck.py`. The Linux recipe in `unix/` stops on `cc`. The current desktop source passes the build and native tests with the restricted tool paths.
