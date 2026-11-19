# Reproduce the numerical verification

Run from the repository root on Windows or Linux:

```console
make check-verifier
```

The target runs the comparator's 37 instruction-level regression cases. The
standalone checker maps the application file as read-only data and compares
29 numerical routines and seven helper ranges with the fixed reference. NASM
then assembles four applications with equivalent MOV encodings, added NOPs,
longer branches, or shifted addresses. Each variant goes through the same checker.

The target also checks an application with a changed numerical constant and
another with a changed thread-count comparison in the producer's fill helper.
Both must return the mismatch status. An input with an invalid image layout
must return a separate status. The wrappers check that NASM applied each change.

The cases cover the listed representations and mutations. `make check` also
runs the numerical self-test on the actual application executable.
