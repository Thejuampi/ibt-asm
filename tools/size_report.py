import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LISTING = ROOT / "bin" / "ibt.lst"


def main():
    state = None
    pending = None
    labels = []
    address_re = re.compile(r"^\s*\d+\s+([0-9A-F]{8})\s+")
    label_re = re.compile(r"^([A-Za-z_$?@][\w$?@]*):$")
    for line in LISTING.read_text(encoding="utf-8", errors="replace").splitlines():
        source = re.sub(r"^\s*\d+(?:\s+[0-9A-F]{8}(?:\s+\S+)?)?\s*(?:<\d+>\s*)?", "", line).strip()
        if source.startswith("section "):
            state = source.split()[1]
            pending = None
            continue
        if state != ".text":
            continue
        match = label_re.match(source)
        if match:
            pending = match.group(1)
            continue
        if pending:
            address = address_re.match(line)
            if address:
                labels.append((int(address.group(1), 16), pending))
                pending = None
    labels = sorted(set(labels))
    rows = []
    for index, (address, name) in enumerate(labels[:-1]):
        end = labels[index + 1][0]
        if end >= address:
            rows.append((end - address, address, name))
    for size, address, name in sorted(rows, reverse=True)[:80]:
        print(f"{size:5d}  0x{address:04X}  {name}")


if __name__ == "__main__":
    main()
