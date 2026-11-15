# Numerical verifier: reproducible check

Run from the repository root on Windows or Linux:

```console
make check-verifier
```

The target first runs the comparator's 34 instruction-level regression cases.
It verifies the actual application, then uses NASM to assemble four
separate applications with representation-only changes: an alternative MOV
encoding, added NOPs, longer branch encodings, and shifted addresses. Each
application must pass its own normal verification of all 53 routines.

It also assembles an application with a numerical constant changed in both
instances. The target succeeds only when that application is rejected with the
expected failure status. The test wrappers check that their intended MOV, NOP,
branch and mutation changes were applied. Neither the numerical source nor the
reference images are edited by this check.

These tests exercise specific equivalent representations and one numerical
mutation. They do not prove acceptance of every possible equivalent encoding,
rejection of every possible mutation, or correctness for all numerical inputs.
The numerical self-test is a separate check.
