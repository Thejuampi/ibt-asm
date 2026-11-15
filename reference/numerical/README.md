# Numerical verification references

The application checks the 26 original numerical routines, their 26
second-instance counterparts, and the independent residual routine: 53 in
total. Adjacent helpers covered by the previous verifier are checked too.
The fixed platform images are assembled from the preserved numerical
source and instance mappings. Each image contains the symbol addresses needed
to identify routines, state, and shared data. `make` never rewrites the images.

The verifier decodes supported x86-64 instructions and compares their
operations and operands. It ignores NOPs and normalizes alternate encodings,
internal branch targets, and addresses resolved to the same named symbol. It
rejects different instructions, operands or targets and fails closed on an
unsupported instruction or unknown target. Its decoder covers the forms used
by these routines; it is not a general proof that arbitrary instruction
sequences have the same behavior. The numerical self-test is separate.

The historical `*-nasm3.bin` images are retained but are not used by the
current verifier because they predate its symbol table. Different supported
NASM encodings are compared against the same platform reference.

Run `make check-verifier` to exercise the actual application with equivalent
encoding, NOP, branch and address variants, and a numerical mutation that must
be rejected. NASM and the resulting executables are the build and test tools.

Do not regenerate these images merely to make a failing build pass. A deliberate
change to numerical code or reference layout requires an independent review of
the intended behavior before updating the reference source and images.
