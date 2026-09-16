import hashlib
import struct
import sys
from pathlib import Path


# Linked .text offsets from the validated d372a01 baseline.  The five allowed
# differences are rel32 low bytes for calls into presentation/control helpers;
# none belongs to the arithmetic, packing, solve, residual, or worker loops.
CORE_START = 0x2E94
CORE_END = 0x5E48
BASELINE_SHA256 = "a3af5de02b4d9daf59d49fcc2756dcddfcb9d90ad86ed8d676c18c1ce4a07615"
ALLOWED_RELOCATIONS = {
    0x0288: (0x29, 0x74),
    0x02CD: (0xB1, 0xFC),
    0x0345: (0x71, 0xBC),
    0x0488: (0x40, 0x8B),
    0x1EB7: (0x42, 0xA2),
}


def text_raw_offset(image):
    if image[:2] != b"MZ":
        raise ValueError("not a PE image")
    pe = struct.unpack_from("<I", image, 0x3C)[0]
    if image[pe : pe + 4] != b"PE\0\0":
        raise ValueError("invalid PE signature")
    section_count = struct.unpack_from("<H", image, pe + 6)[0]
    optional_size = struct.unpack_from("<H", image, pe + 20)[0]
    section_table = pe + 24 + optional_size
    for index in range(section_count):
        entry = section_table + index * 40
        name = image[entry : entry + 8].rstrip(b"\0")
        if name == b".text":
            return struct.unpack_from("<I", image, entry + 20)[0]
    raise ValueError("PE image has no .text section")


def main():
    target = Path(sys.argv[1] if len(sys.argv) > 1 else "bin/IntelBurnTest.exe")
    image = target.read_bytes()
    raw = text_raw_offset(image)
    region = bytearray(image[raw + CORE_START : raw + CORE_END])
    if len(region) != CORE_END - CORE_START:
        raise SystemExit("benchcheck: truncated numerical region")

    for offset, (baseline, expected) in ALLOWED_RELOCATIONS.items():
        actual = region[offset]
        if actual != expected:
            raise SystemExit(
                "benchcheck: unexpected relocation byte at +0x%04x: "
                "expected 0x%02x, got 0x%02x" % (offset, expected, actual)
            )
        region[offset] = baseline

    digest = hashlib.sha256(region).hexdigest()
    if digest != BASELINE_SHA256:
        raise SystemExit(
            "benchcheck: numerical code differs from baseline\n"
            "expected %s\nactual   %s" % (BASELINE_SHA256, digest)
        )

    print(
        "benchcheck: %d numerical bytes match d372a01; "
        "%d audited control relocations ignored"
        % (len(region), len(ALLOWED_RELOCATIONS))
    )


if __name__ == "__main__":
    main()
