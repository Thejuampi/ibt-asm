# Repository rules

The desktop build and test rules apply to Windows and Linux. For the EFI application and USB installer, use the [UEFI build instructions](uefi/README.md).

| Rule | Enforcement |
| --- | --- |
| Build and test the desktop application with NASM and make. Use the platform shell and loader to run native programs. Keep auxiliary runtimes and platform build tools out of the recipes. | The Makefiles assemble native PE and ELF files and run native tests. The CI build steps restrict the tool search path. Review changes to the recipes and CI setup for new dependencies. |
| Keep each desktop executable within its unpacked v3.1.0 size budget. | NASM checks the PE and ELF layouts. `make check` calls `make check-size` to read the assembled files. The caps are 54,784 bytes on Windows and 43,344 on Linux. Run `make check-size` on a release file after any step that changes its bytes. |
| Check numerical code in the exact executable candidate before promoting it. | The Makefiles run the standalone comparator before installing the candidate. `make check-verifier` tests equivalent encodings, early and late numerical mutations, and an invalid image. |
| Keep numerical symbol and state includes synchronized with assembly changes. | NASM rejects missing or duplicate labels. Review symbol mappings and state declarations alongside the code that uses them. Review an intentional numerical change independently before updating its frozen reference. |
| Exercise desktop startup and keep self-test fixtures out of the Windows release build. | Make builds the Windows release and extended self-test executables separately. `make check-gui` tests Windows startup from a path containing a flag and checks the displayed result fields in the self-test executable. Linux checks startup and graphical resources on X11. |
| Require desktop validation before merging to `main`. | GitHub requires the `windows` and `linux` checks from GitHub Actions. The workflow runs the build, numerical checks, verifier regressions, and graphical checks for each platform. |
| Attach release binaries from the tested revision. | The desktop workflow checks that the destination is a draft release and that it targets the tested SHA before attaching the files. |
| Report what the checks ran and what they observed. | Judgment. Distinguish build, numerical self-test, GUI startup, and a complete stress run. Evaluate performance with a matched workload and verify the published file or URL when reporting delivery. |

Use the [desktop regression checks](docs/desktop-validation.md) to replay the recorded failures.
