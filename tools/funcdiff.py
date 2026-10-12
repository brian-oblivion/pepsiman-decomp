#!/usr/bin/env python3
"""Compare the built executable against retail over one function's byte range.

    python3 tools/funcdiff.py <func_name>          # score, plus differing words
    python3 tools/funcdiff.py <func_name> --context  # every word, matched too
    python3 tools/funcdiff.py <func_name> --ovl ovl18  # in that overlay

The function is looked for in the executable and in every overlay the build
covers (the Makefile's OVERLAYS); `--ovl` is needed only when a name exists in
more than one, as the world intros' func_800F00C0 does.

Exit status is the answer: 0 = byte-identical, 1 = differs, 2 = THE NUMBER
CANNOT BE TRUSTED (see the two guards below).

`./build-and-verify.sh` is the project's only oracle. This tool exists to say
*where* a function is still wrong once you already know it is wrong, and to
give a score you can watch move. It is not a substitute for the full check.

Addresses come from splat's own instruction comments, which carry the file
offset directly: `/* 39CD8 800494D8 E8FFBD27 */`, i.e. FILEOFS VRAM WORD. For
the executable file offset = vram - 0x80010000 + 0x800; an overlay has no
header, so its file offset = vram - 0x800F0000.
"""
import re
import subprocess
import sys
from pathlib import Path

import progress
import srcpath

ROOT = Path(__file__).resolve().parent.parent
NM = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"


class Target:
    """One image the build verifies: the executable, or one overlay.

    Everything below that names a file goes through one of these, so an
    overlay function is never scored against the executable's bytes (or the
    reverse) -- the same vram means different code in the two.
    """

    def __init__(self, name, retail, built, elf, vram_base, file_base,
                 asm_root, stale_globs, sources):
        self.name = name
        self.retail = ROOT / retail
        self.built = ROOT / built
        self.elf = ROOT / elf
        self.vram_base = vram_base
        self.file_base = file_base
        self.asm_root = ROOT / asm_root
        self.stale_globs = stale_globs
        self.sources = sources

    def vram(self, off):
        return off - self.file_base + self.vram_base

    def nm_find(self, fn):
        if self.name == "main":
            return srcpath.nm_find(fn)
        return sorted((self.asm_root / "nonmatchings").rglob(f"{fn}.s"))


def overlays():
    """The OVERLAYS the Makefile builds, read from the Makefile itself."""
    m = re.search(r"^OVERLAYS\s*:=(.*)$", (ROOT / "Makefile").read_text(), re.M)
    return m.group(1).split() if m else []


def targets():
    main = Target("main", "disk/SLPS_017.62", "build/SLPS_017.62",
                  "build/pepsiman.elf", 0x80010000, 0x800, "asm",
                  ("src/**/*.c", "include/**/*.h", "include/*.inc",
                   "asm/*.s", "asm/nonmatchings/**/*.s"),
                  [p for p in srcpath.src_files()
                   if "ovl" not in p.relative_to(srcpath.SRC).parts[:-1]])
    out = [main]
    for o in overlays():
        out.append(Target(o, f"disk/ovl/{o}.bin", f"build/ovl/{o}.bin",
                          f"build/ovl/{o}.elf", 0x800F0000, 0, f"asm/ovl/{o}",
                          (f"src/ovl/{o}.c", "include/**/*.h", "include/*.inc",
                           f"asm/ovl/{o}/**/*.s", f"config/ovl/gp.{o}.txt"),
                          [ROOT / f"src/ovl/{o}.c"]))
    return out

# /* FILEOFS VRAM WORD */
INSN_RE = re.compile(r"/\* ([0-9A-Fa-f]+) [0-9A-Fa-f]{8} [0-9A-Fa-f]{8} \*/")


def range_from_elf(t, name):
    """(start, end) file offsets from the linked ELF's symbol table, or None.

    THE ONLY SOURCE THAT WORKS FOR AN ALREADY-MATCHED FUNCTION. Once a function
    is real C its `asm/nonmatchings/` file stops being generated, so the two
    disassembly sources below cannot see it at all -- and funcdiff would answer
    "not found" for precisely the functions someone is trying to RE-verify.
    That matters: the head agent's protocol spot-checks every claimed match
    after a merge, and a tool that cannot score a match is useless for it.

    Size is the gap to the next symbol, which is how the linker lays functions
    out. It over-reads when the next symbol is not a function, so this is the
    fallback rather than the primary source.
    """
    if not (NM.exists() and t.elf.exists()):
        return None
    out = subprocess.run([str(NM), "-n", str(t.elf)],
                         capture_output=True, text=True).stdout
    syms = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in ("T", "t"):
            syms.append((int(parts[0], 16), parts[2]))
    for i, (addr, sym) in enumerate(syms):
        if sym != name:
            continue
        nxt = next((a for a, _ in syms[i + 1:] if a > addr), None)
        if nxt is None:
            return None
        return (addr - t.vram_base + t.file_base, nxt - t.vram_base + t.file_base)
    return None


def find_range(t, name):
    """(start, end) file offsets for `name` in target `t`, or None.

    Three sources, in descending order of precision:
      1. asm/nonmatchings/**/<name>.s -- one function per file, exact.
      2. the monolithic `asm/*.s` segments, where a function is delimited by
         `glabel <name>` ... `endlabel <name>`.
      3. the linked ELF's symbol table, which is the only one that still knows
         about a function after it has been MATCHED.
    """
    for p in t.nm_find(name):
        text = p.read_text()
        # DELIMIT BY glabel/endlabel, exactly as source 2 below does -- do NOT
        # min/max over the whole file.  A function that owns a jump table has
        # its rodata emitted into its OWN `.s` as a leading
        # `.section .rodata` block, and those data words carry the same
        # `/* fileofs vram word */` comment shape as instructions.  Because the
        # rodata slot sits at a much LOWER file offset than the text, a
        # min/max over the file returned a window spanning the whole image:
        # func_800513D0 measured `65533/65533 words match (file 0x1E28-0x41E1C)`
        # against a real size of 147 words.
        #
        # That was worse than a cosmetic wrong denominator.  It also DISABLED
        # THE DRIFT GUARD -- with a ~262KB window almost nothing is "outside
        # this range", so the out-of-range byte count can no longer fire.  And
        # for a near-miss it prints a precise-looking ratio in which 3 words
        # off and 30 words off are indistinguishable.
        m = re.search(rf"^glabel {re.escape(name)}$\n(.*?)^endlabel "
                      rf"{re.escape(name)}$", text, re.S | re.M)
        body = m.group(1) if m else text
        offs = [int(x, 16) for x in INSN_RE.findall(body)]
        if offs:
            return min(offs), max(offs) + 4

    for p in sorted(t.asm_root.glob("*.s")):
        text = p.read_text()
        m = re.search(rf"^glabel {re.escape(name)}$\n(.*?)^endlabel "
                      rf"{re.escape(name)}$", text, re.S | re.M)
        if not m:
            continue
        offs = [int(x, 16) for x in INSN_RE.findall(m.group(1))]
        if offs:
            return min(offs), max(offs) + 4

    return range_from_elf(t, name)


def staleness_check(t):
    """Warn if any build input is NEWER than the built executable.

    THE ORACLE TRAP THIS PROJECT WILL HIT MOST OFTEN, made mechanical instead
    of procedural. This tool reads the BUILT FILE. A failed compile or a failed
    LINK leaves the PREVIOUS build in place, so every number below comes from
    the last build that succeeded -- and when that build had the function as
    INCLUDE_ASM, the comparison is retail against retail and the score reads as
    a FULL MATCH with no diagnostic anywhere.

    The reason a written rule ("always check the build's exit status") is not
    enough: a stale number can be PLAUSIBLE AND SELF-CONSISTENT. It will often
    equal a real score you measured minutes earlier, so nothing about the
    number itself looks wrong.

    Comparing mtimes is a heuristic in one direction only -- it cannot catch a
    stale build whose sources were not touched -- but it never fires falsely,
    and it catches the sequence that actually recurs: edit, build fails, read
    the score.
    """
    built_rel = t.built.relative_to(ROOT)
    try:
        built_mtime = t.built.stat().st_mtime
    except OSError:
        return f"no {built_rel} — run ./build-and-verify.sh first."
    newest, newest_path = 0.0, None
    # The executable's patterns reach into src/ovl/ and asm/ovl/ too; an
    # overlay edit does not make the executable stale, so those are skipped.
    for pat in t.stale_globs:
        for p in ROOT.glob(pat):
            rel = p.relative_to(ROOT).parts
            if t.name == "main" and rel[1:2] == ("ovl",):
                continue
            try:
                m = p.stat().st_mtime
            except OSError:
                continue
            if m > newest:
                newest, newest_path = m, p
    if newest > built_mtime:
        return (f"STALE BUILD: {newest_path.relative_to(ROOT)} is NEWER than "
                f"{built_rel}.\n"
                f"         The build does not reflect your latest edit, which "
                f"means it almost certainly\n"
                f"         FAILED (compile or link). EVERY NUMBER ABOVE IS "
                f"FROM THE PREVIOUS BUILD.\n"
                f"         Re-run ./build-and-verify.sh and read its exit "
                f"status before believing anything here.")
    return None


def include_asm_check(t, name):
    """Warn if `name` is still INCLUDE_ASM, i.e. this is retail vs retail.

    A DIFFERENT TRAP IN KIND from staleness: the build SUCCEEDS, the output is
    FRESH, and the score is still a full match -- because an INCLUDE_ASM
    function contributes retail's own assembled bytes. Nothing fails and there
    is no message to grep for.

    It fires most often when spot-checking someone else's stall claim after a
    merge, because merging restores the INCLUDE_ASM. To re-measure a stall you
    have to re-enable its body first.
    """
    inc = re.compile(r"INCLUDE_ASM\([^,]+,\s*" + re.escape(name) + r"\s*\)")
    for c in t.sources:
        # A preserved body, a commented-out line, or the `#else` arm of an
        # `#if 1` a runner iterates under is not compiled and must not count
        # as a live INCLUDE_ASM (round 64: the regex form warned "still
        # INCLUDE_ASM" on a genuine match). progress.py owns the walk.
        code = progress.strip_dead_code(c.read_text())
        if inc.search(code):
            return (f"{name} is still INCLUDE_ASM in {c.name}.\n"
                    f"         The build therefore contains RETAIL'S OWN BYTES "
                    f"for it, so this compares\n"
                    f"         retail against retail and a full match means "
                    f"NOTHING. Re-enable the C\n"
                    f"         body before reading a score.")
    return None


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    name = sys.argv[1]
    want = None
    if "--ovl" in sys.argv:
        i = sys.argv.index("--ovl")
        if i + 1 >= len(sys.argv):
            sys.exit("--ovl needs an overlay name, e.g. --ovl ovl18")
        want = sys.argv[i + 1]

    pool = targets()
    if want:
        pool = [t for t in pool if t.name == want]
        if not pool:
            sys.exit(f"{want} is not an overlay the build covers "
                     f"(the Makefile's OVERLAYS: {' '.join(overlays()) or 'none'})")
    found = [(t, r) for t in pool for r in [find_range(t, name)] if r]
    if not found:
        where = want or "the executable or any overlay"
        sys.exit(f"function {name} not found in {where}, in asm/ or in the "
                 f"linked ELF.\n"
                 f"  Is it spelled right? Has `make extract` run? Has "
                 f"./build-and-verify.sh run at least once\n"
                 f"  (the ELF is the only source that knows about an "
                 f"already-matched function)?")
    if len(found) > 1:
        sys.exit(f"{name} exists in {', '.join(t.name for t, _ in found)}; "
                 f"pick one with --ovl <name>.")
    t, (start, end) = found[0]

    if not t.retail.exists():
        sys.exit(f"{t.retail.relative_to(ROOT)} missing — see README.md, 'Building it'")
    if not t.built.exists():
        sys.exit(f"{t.built.relative_to(ROOT)} missing — run ./build-and-verify.sh")

    retail = t.retail.read_bytes()
    built = t.built.read_bytes()

    stale_warning = staleness_check(t)
    inc_warning = include_asm_check(t, name)

    a, b = retail[start:end], built[start:end]
    outside = sum(1 for i in range(min(len(retail), len(built)))
                  if (i < start or i >= end) and retail[i] != built[i])

    n = len(a) // 4
    bad = 0
    for i in range(n):
        # Little-endian: print the word as the disassembler shows it, so a
        # value here can be grepped straight out of a .s comment.
        x, y = a[i * 4:i * 4 + 4], b[i * 4:i * 4 + 4]
        if x != y:
            bad += 1
        if x != y or "--context" in sys.argv:
            mark = "OK  " if x == y else "DIFF"
            print(f"{i:3d} off=0x{start + i * 4:06X} vram=0x{t.vram(start + i * 4):08X} "
                  f"{mark} retail={x.hex()} built={y.hex()}")

    where = "" if t.name == "main" else f"{t.name} "
    print(f"{name}: {n - bad}/{n} words match "
          f"({where}file 0x{start:X}-0x{end:X})")

    # Insertions / deletions at the OPCODE level, so Gate 3's check 3 (does the
    # permuter scaffold's signature AGREE with the real build) can be run at
    # all: round 58 found it had been inferred from "length exact, so 0/0",
    # and equal length is exactly where an insertion and a deletion cancel
    # (measured 3/3 and 14/14). Same skeleton as tools/sdkname.py: the
    # instruction with its immediate field zeroed, so a register or offset
    # change counts as a REPLACEMENT and only a genuinely added or dropped
    # instruction counts here. A pure register-identity residue reads 0/0.
    import difflib
    def skel(w):
        v = int.from_bytes(w, "little")
        op = v >> 26
        if op == 0:
            return v & 0xFFFFF83F
        if op in (2, 3):
            return v & 0xFC000000
        return v & 0xFFFF0000
    ra = [skel(a[i * 4:i * 4 + 4]) for i in range(len(a) // 4)]
    rb = [skel(b[i * 4:i * 4 + 4]) for i in range(len(b) // 4)]
    # Positional skeleton mismatches. If ZERO at equal length, no instruction
    # was inserted or dropped, whatever the sequence matcher says: on a loop
    # nest's repeating skeleton alphabet it can report N/N where nothing moved
    # (round 63 measured 26/26 and 22/22 on synthetic zero-insertion inputs).
    positional = sum(1 for x, y in zip(ra, rb) if x != y) if len(ra) == len(rb) else None
    ins = dele = 0
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, ra, rb, autojunk=False).get_opcodes():
        if tag == "insert":
            ins += j2 - j1
        elif tag == "delete":
            dele += i2 - i1
        elif tag == "replace":
            d = (j2 - j1) - (i2 - i1)
            ins += max(d, 0)
            dele += max(-d, 0)
    if positional == 0:
        ins = dele = 0
    note = ""
    if positional is not None and ins and ins == dele:
        note = (f"; NOTE equal length with {positional} positional skeleton diff(s): an N/N figure "
                f"here is a pointer to READ the diff, not a verdict (a repeating skeleton alphabet "
                f"can align falsely)")
    print(f"{name}: insertions {ins} / deletions {dele} (opcode-level, built vs retail; "
          f"positional skeleton diffs {positional if positional is not None else 'n/a'}; "
          f"Gate 3 check 3 compares these with the scaffold's --stack-diffs){note}")

    if outside:
        print(f"WARNING: the build differs OUTSIDE this range too ({outside} "
              f"bytes) — a size change may have\n"
              f"         shifted linked addresses, so this per-function read "
              f"is NOT trustworthy.\n"
              f"         ./build-and-verify.sh is the oracle.")
    # Print the warnings LAST as well as exiting non-zero: a full match is
    # exactly the case where the reader stops reading, and a false full match
    # is the failure these guard against.
    if inc_warning:
        print(f"WARNING: {inc_warning}")
    if stale_warning:
        print(f"WARNING: {stale_warning}")

    if stale_warning or inc_warning:
        sys.exit(2)
    sys.exit(1 if bad or outside else 0)


if __name__ == "__main__":
    main()
