# Numerical verification references

The build checks the 26 original numerical routines, both producer fill
routines, and the independent residual routine. It also checks seven helper
ranges. The standalone checker reads the executable candidate as data before
`make` installs that file. The fixed platform images come from the preserved
numerical source and instance mappings. The symbol tables match the identities
of routines, state, and shared data in the candidate and reference.

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

Run `make check-verifier` to compare the application and its equivalent encoding,
NOP, branch, and address variants. The checker must reject two mutations and
distinguish a malformed image from a numerical difference.

Do not regenerate these images merely to make a failing build pass. A deliberate
change to numerical code or reference layout requires an independent review of
the intended behavior before updating the reference source and images.
