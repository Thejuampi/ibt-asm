import re
import sys

LST = r"C:\Users\Juan\Desktop\ibt-asm\bin\ibt.lst"
HOT = re.compile(
    r"^(lp_k|lp_pack|lp_dgemm|lp_dget|lp_dtrsm|lp_scale|lp_daxpy|lp_pivot|lp_steal|lp_par|lp_worker|lp_panel)"
)
SSE = re.compile(
    r"(?<![a-z])(movsd|xorpd|andpd|orpd|mulpd|addsd|subsd|mulsd|divsd|ucomisd|comisd|movapd|movaps|xorps)\b"
)
VEX = re.compile(r"\b(v[a-z][a-z0-9]+)\b")
JCC = re.compile(r"\b(j[a-z]+|loop)\s+(\.?[A-Za-z_][A-Za-z0-9_]*)\b", re.I)
LAB = re.compile(r"^(\.?[A-Za-z_][A-Za-z0-9_]*):")
ALIGN = re.compile(r"\balign\s+(\d+)\b")
APD = re.compile(r"\bvmova(pd|ps)\b")
LINE = re.compile(
    r"^\s*(\d+)\s+([0-9A-Fa-f]{8})?\s*(?:[0-9A-Fa-f]{2}(?: [0-9A-Fa-f]{2})*)?\s*(?:<\d+>\s*)?(.*)$"
)


def parse_lst(path):
    rows = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for raw in f:
            m = LINE.match(raw.rstrip("\n"))
            if not m:
                continue
            addr = int(m.group(2), 16) if m.group(2) else None
            rows.append((int(m.group(1)), addr, (m.group(3) or "").strip()))
    return rows


def bind_labels(rows):
    labels = {}
    pending = []
    for _, addr, src in rows:
        m = LAB.match(src)
        if m:
            pending.append(m.group(1))
        if addr is not None and pending:
            for lab in pending:
                if lab not in labels:
                    labels[lab] = addr
            pending = []
    return labels


def main():
    rows = parse_lst(LST)
    labels = bind_labels(rows)
    hits = []

    if labels.get("lp_isa_probe") != 0x2E94:
        hits.append(
            (
                "bench-anchor",
                "lp_isa_probe",
                "expected 0x2e94, got %s"
                % hex(labels.get("lp_isa_probe", -1)),
            )
        )

    loops = {}
    for _, addr, src in rows:
        if addr is None:
            continue
        m = JCC.search(src)
        if not m:
            continue
        tgt = m.group(2)
        if tgt in labels and labels[tgt] <= addr:
            loops[tgt] = labels[tgt]

    for lab, addr in sorted(loops.items(), key=lambda x: x[1]):
        if lab.startswith(".") and lab not in (
            ".u2",
            ".u4",
            ".pk12",
            ".pk8",
            ".pk4",
            ".pk",
            ".p12",
            ".p8",
            ".kr",
            ".kk",
            ".kk4",
            ".k",
            ".gc",
            ".g4",
            ".pl",
            ".a8",
            ".col",
            ".col4",
            ".jloop",
            ".i12",
            ".t1",
        ):
            continue
        if addr % 32:
            hits.append(("align32-loop", lab, "%s %%32=%d" % (hex(addr), addr % 32)))

    for lab in (
        "lp_k12x4",
        "lp_k8x4",
        "lp_k8x4_avx",
        "lp_krest",
        "lp_pack_a",
        "lp_pack_b",
        "lp_dgemm_cols",
        "lp_dgemm_ir",
        "lp_pack_b_all",
        "lp_dgetrf",
        "lp_dgetf2",
        "lp_panel",
        "lp_dgetrs",
        "lp_dtrsm_cols",
        "lp_scale_col",
        "lp_daxpy_neg",
        "lp_daxpy4_neg",
        "lp_pivot",
    ):
        if lab not in labels:
            hits.append(("missing", lab, "not in listing"))
            continue
        a = labels[lab]
        if a % 32:
            hits.append(("align32-fn", lab, "%s %%32=%d" % (hex(a), a % 32)))

    prev_align = None
    last_fn = None
    sse_in_avx_fn = {}
    has_vex = set()
    for _, addr, src in rows:
        m = LAB.match(src)
        if m and not m.group(1).startswith("."):
            last_fn = m.group(1)
        am = ALIGN.search(src)
        if am:
            prev_align = int(am.group(1))
        if m and m.group(1).startswith(".") and prev_align is not None:
            if m.group(1) in (".u2", ".u4", ".pk12", ".p12", ".a8") and prev_align < 32:
                hits.append(("weak-align", last_fn + m.group(1), "align %d" % prev_align))
            prev_align = None
        if last_fn and HOT.match(last_fn or ""):
            if VEX.search(src):
                has_vex.add(last_fn)
            sm = SSE.search(src)
            if sm:
                sse_in_avx_fn.setdefault(last_fn, []).append(sm.group(1))
    for fn, ops in sse_in_avx_fn.items():
        if fn in has_vex:
            uniq = sorted(set(ops))
            hits.append(("sse-after-avx", fn, " ".join(uniq)))

    kindn = {}
    block = 0
    for k, a, b in hits:
        kindn[k] = kindn.get(k, 0) + 1
        print("%-16s %-22s %s" % (k, a, b))
        if k in (
            "sse-after-avx",
            "align32-fn",
            "weak-align",
            "missing",
            "bench-anchor",
        ):
            block += 1
    print("---")
    for k in sorted(kindn):
        print("%s %d" % (k, kindn[k]))
    print("total %d block %d" % (len(hits), block))
    return 1 if block else 0


if __name__ == "__main__":
    sys.exit(main())
