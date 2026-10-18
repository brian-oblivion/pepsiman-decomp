#!/usr/bin/env python3
"""Render every sprite the game defines, from the textures on your disc.

    .venv/bin/python3 tools/sprites.py "Pepsiman (Japan) (Track 1).bin"
    .venv/bin/python3 tools/sprites.py disc.bin --only stage --only ovl03

Writes build/sprites/index.html and one PNG per sprite. The pictures are
Sony's and the game's: they stay in build/, which is gitignored, and are
never committed.

A sprite is a rectangle of VRAM, not a file: DefineSprite (src/draw.c) takes
a texture page, a u/v corner, a size and a CLUT position. Every TIM on the
disc carries the VRAM position LoadImage puts it at, so a sprite is found by
loading the TIMs a screen loads and reading its rectangle back. Two kinds of
definition are covered:

  * DefineSprite calls in the C. Each unit is run through the project's cpp
    (macros and .inc files expanded) and every call's arguments evaluated,
    inside the `for` loops around it, against the unit's enums. A call whose
    arguments depend on runtime state (LoadSpritePack, LoadSpritePackRange)
    is listed as unresolved.
  * Stage sprite packs. A stage's file Loc[5] (include/disc_files.inc) is a
    directory of TIMs that LoadSpritePack registers as SPR_STAGE + n, each
    with its own CLUT; they are decoded from the file directly.

Which TIMs a screen has in VRAM is not tracked frame by frame. A unit's
sprites are looked up in its own files first (the overlay's directory and
name prefix on the disc; CDDATA/0 for the executable), then anywhere on the
disc; each picture says which file its pixels came from. Missing texels are
drawn magenta.
"""
import argparse
import base64
import html
import re
import struct
import subprocess
import sys
import zlib
from pathlib import Path

import extract_exe
import extract_overlays as eo
from m2ctx import CPP, CPP_ARGS

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build" / "sprites"
DISC_FILES = ROOT / "include" / "disc_files.inc"
SPR_STAGE = 0x1E

# DefineSprite(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v,
#              u16 clutX, s32 clutY): what each argument is narrowed to.
PARAMS = ("id", "mode", "w", "h", "page", "u", "v", "clutX", "clutY")
PARAM_BITS = {"id": 16, "mode": 8, "clutX": 16}


# ---------------------------------------------------------------- disc

def tims(data):
    """Every TIM in `data`: a list of (clut region or None, pixel region),
    each region (x, y, w, h, data, offset) in VRAM halfwords."""
    out = []
    i = 0
    while i + 20 <= len(data):
        if data[i:i + 4] == b"\x10\0\0\0" and not struct.unpack_from("<I", data, i + 4)[0] & ~0xB:
            flags = struct.unpack_from("<I", data, i + 4)[0]
            j = i + 8
            try:
                clut = None
                if flags & 8:
                    n, x, y, w, h = struct.unpack_from("<I4H", data, j)
                    if n != 12 + w * h * 2 or j + n > len(data):
                        raise ValueError
                    clut = (x, y, w, h, data, j + 12)
                    j += n
                n, x, y, w, h = struct.unpack_from("<I4H", data, j)
                if n != 12 + w * h * 2 or x >= 1024 or y >= 512 or w == 0 or j + n > len(data):
                    raise ValueError
                out.append((clut, (x, y, w, h, data, j + 12)))
                i = j + n
                continue
            except (ValueError, struct.error):
                pass
        i += 4
    return out


def regions(file_tims, name):
    """A file's VRAM regions, latest load first (a later LoadImage wins),
    each tagged with the file: (x, y, w, h, data, offset, file)."""
    out = []
    for clut, pix in file_tims:
        if clut:
            out.append(clut + (name,))
        out.append(pix + (name,))
    return out[::-1]


def disc_tables():
    """include/disc_files.inc's CdlLOC arrays: {"Ovl03Loc": [lba, ...]}."""
    text = DISC_FILES.read_text()
    out = {}
    for name, body in re.findall(r"CdlLOC DISC_FILE\((\w+)\)\[\d+\] = \{(.*?)\};", text, re.S):
        out[name] = [(int(m) * 60 + int(s)) * 75 + int(f) - 150
                     for m, s, f in re.findall(r"CD_LOC\((\d+), (\d+), (\d+)\)", body)]
    return out


class Vram:
    """VRAM as tiers of regions: a halfword comes from the first tier that
    has it."""

    def __init__(self, tiers):
        self.tiers = tiers

    def view(self, x0, y0, x1, y1):
        """The regions that touch [x0, x1) x [y0, y1) only."""
        return Vram([[r for r in rs if r[0] < x1 and x0 < r[0] + r[2] and r[1] < y1 and y0 < r[1] + r[3]]
                     for rs in self.tiers])

    def get(self, x, y):
        """The halfword at (x, y) and its file, or (None, None)."""
        for rs in self.tiers:
            for rx, ry, rw, rh, data, off, f in rs:
                if rx <= x < rx + rw and ry <= y < ry + rh:
                    return struct.unpack_from("<H", data, off + ((y - ry) * rw + x - rx) * 2)[0], f
        return None, None


MISSING = b"\xff\x00\xff\xff"


def render(vram, tpage, u, v, w, h, clut):
    """RGBA rows of a sprite, and the files its texels and CLUT came from."""
    bx, by, depth = (tpage & 15) * 64, (tpage >> 4 & 1) * 256, tpage >> 7 & 3
    cx, cy = (clut & 63) * 16, clut >> 6 & 511
    per = (4, 2, 1, 1)[depth]
    texels = vram.view(bx, by, bx + 256 // per, by + 256)
    palette = {}
    files = set()
    rows = []
    for yy in range(h):
        row = bytearray()
        for xx in range(w):
            tu, tv = (u + xx) & 0xFF, (v + yy) & 0xFF
            c, f = texels.get(bx + tu // per, by + tv)
            if c is not None and depth < 2:
                files.add(f)
                bits = 4 << depth
                index = c >> (tu % per * bits) & ((1 << bits) - 1)
                if index not in palette:
                    palette[index] = vram.get(cx + index, cy)
                c, f = palette[index]
            if c is None:
                row += MISSING
                continue
            files.add(f)
            row += bytes(((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 0 if c == 0 else 255))
        rows.append(bytes(row))
    return rows, files


def png(rows, w, h):
    """An RGBA PNG of `rows`."""
    raw = b"".join(b"\0" + r for r in rows)

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


# ---------------------------------------------------------------- the C

def preprocess(path):
    """The unit through the project's cpp, as m2ctx.py runs it."""
    res = subprocess.run([str(CPP)] + CPP_ARGS + [str(path)], cwd=ROOT,
                         capture_output=True, text=True)
    if res.returncode:
        sys.exit(f"cpp failed on {path}:\n{res.stderr}")
    return res.stdout


def split_lines(text):
    """cpp output without its line markers, and each kept line's source."""
    out, where = [], []
    cur, line = "?", 0
    for raw in text.split("\n"):
        m = re.match(r'# (\d+) "([^"]*)"', raw)
        if m:
            line, cur = int(m.group(1)), m.group(2)
            continue
        out.append(raw)
        where.append((cur, line))
        line += 1
    return "\n".join(out), where


class Unknown(Exception):
    """An argument that is not a constant expression, and why."""


CASTS = {"u8": (8, False), "s8": (8, True), "u16": (16, False), "s16": (16, True),
         "u32": (32, False), "s32": (32, True), "char": (8, False), "short": (16, True),
         "int": (32, True), "long": (32, True)}


def narrow(v, bits, signed):
    v &= (1 << bits) - 1
    return v - (1 << bits) if signed and v >> (bits - 1) else v


class Evaluator:
    """Evaluates a C constant expression over enums and loop variables."""

    OPS = {"+": lambda a, b: a + b, "-": lambda a, b: a - b, "*": lambda a, b: a * b,
           "<<": lambda a, b: a << b, ">>": lambda a, b: a >> b, "&": lambda a, b: a & b,
           "|": lambda a, b: a | b, "^": lambda a, b: a ^ b, "<": lambda a, b: int(a < b),
           ">": lambda a, b: int(a > b), "<=": lambda a, b: int(a <= b),
           ">=": lambda a, b: int(a >= b), "==": lambda a, b: int(a == b),
           "!=": lambda a, b: int(a != b), "&&": lambda a, b: int(bool(a and b)),
           "||": lambda a, b: int(bool(a or b)),
           "/": lambda a, b: int(a / b) if b else Evaluator.fail("division by 0"),
           "%": lambda a, b: a - int(a / b) * b if b else Evaluator.fail("modulo by 0")}

    def __init__(self, names):
        from pycparser import c_parser
        self.parser = c_parser.CParser()
        self.names = names
        self.cache = {}

    @staticmethod
    def fail(why):
        raise Unknown(why)

    def parse(self, expr):
        if expr not in self.cache:
            src = "typedef int " + ", ".join(k for k in CASTS if k not in ("char", "short", "int", "long"))
            try:
                ast = self.parser.parse(f"{src}; int x_ = ({expr});")
            except Exception as e:  # pycparser raises its own ParseError
                raise Unknown(f"cannot parse {expr!r}: {e}")
            self.cache[expr] = ast.ext[-1].init
        return self.cache[expr]

    def eval(self, expr, env):
        return self.node(self.parse(expr), env)

    def node(self, n, env):
        kind = type(n).__name__
        if kind == "Constant":
            s = n.value.rstrip("uUlL")
            if n.type == "char":
                return ord(s.strip("'"))
            return int(s, 0) if not re.fullmatch(r"0[0-7]+", s) else int(s, 8)
        if kind == "ID":
            if n.name in env:
                return env[n.name]
            if n.name in self.names:
                return self.names[n.name]
            raise Unknown(f"{n.name} is not a constant")
        if kind == "BinaryOp":
            return self.OPS[n.op](self.node(n.left, env), self.node(n.right, env))
        if kind == "UnaryOp":
            v = self.node(n.expr, env)
            ops = {"-": lambda: -v, "+": lambda: v, "~": lambda: ~v, "!": lambda: int(not v)}
            if n.op not in ops:
                raise Unknown(f"operator {n.op}")
            return ops[n.op]()
        if kind == "TernaryOp":
            return self.node(n.iftrue if self.node(n.cond, env) else n.iffalse, env)
        if kind == "Cast":
            v = self.node(n.expr, env)
            decl = n.to_type.type
            names = getattr(decl.type, "names", None) if type(decl).__name__ == "TypeDecl" else None
            if not names or names[-1] not in CASTS:
                raise Unknown("a cast to a pointer or struct")
            unsigned = "unsigned" in names
            bits, signed = CASTS[names[-1]]
            return narrow(v, bits, signed and not unsigned)
        raise Unknown(f"{kind} is not a constant expression")


def enums(text, ev):
    """Every enumerator in the unit, with its value."""
    for body in re.findall(r"\benum\s*\w*\s*\{([^{}]*)\}", text):
        nxt = 0
        for item in body.split(","):
            item = item.strip()
            if not item:
                continue
            name, _, expr = item.partition("=")
            name = name.strip()
            try:
                nxt = ev.eval(expr, {}) if expr.strip() else nxt
            except Unknown:
                continue
            ev.names[name] = nxt
            nxt += 1


def blank_strings(text):
    """`text` with string and char literals blanked, so braces in them do
    not count."""
    return re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: m.group(0)[0] + " " * (len(m.group(0)) - 2) + m.group(0)[-1], text)


def matching(text, i):
    """The index just past the parenthesis that closes text[i] == '('."""
    depth = 0
    for j in range(i, len(text)):
        depth += text[j] == "("
        depth -= text[j] == ")"
        if depth == 0:
            return j + 1
    raise ValueError("unbalanced parentheses")


def split_args(s):
    """A call's arguments, split at its top-level commas."""
    out, depth, cur = [], 0, ""
    for c in s:
        if c == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
            continue
        depth += c in "(["
        depth -= c in ")]"
        cur += c
    return out + [cur.strip()]


LOOP = re.compile(r"^\s*(\w+)\s*=\s*(.+?)\s*;\s*\1\s*(<|<=|!=)\s*(.+?)\s*;\s*(?:\1\s*\+\+|\+\+\s*\1|\1\s*\+=\s*(.+?))\s*$")


def definitions(path):
    """Every DefineSprite call in a unit: dicts with the call's source line,
    function, and either `defs` (the evaluated argument tuples) or `why`."""
    text, where = split_lines(preprocess(path))
    text = blank_strings(text)
    starts = [0]
    for line in text.split("\n"):
        starts.append(starts[-1] + len(line) + 1)

    def origin(pos):
        lo, hi = 0, len(starts) - 1
        while lo + 1 < hi:
            mid = (lo + hi) // 2
            lo, hi = (mid, hi) if starts[mid] <= pos else (lo, mid)
        return where[lo]

    ev = Evaluator({})
    enums(text, ev)
    calls = []
    depth = 0
    loops = []  # (brace depth of the body, statement-only, header text)
    func = "?"
    last_stmt = 0
    i = 0
    token = re.compile(r"[{};]|\bfor\s*\(|\bDefineSprite\s*\(")
    while True:
        m = token.search(text, i)
        if not m:
            break
        t, i = m.group(0), m.end()
        if t == "{":
            if depth == 0:
                head = re.search(r"(\w+)\s*\([^;{}]*\)\s*$", text[last_stmt:m.start()])
                func = head.group(1) if head else "?"
            depth += 1
        elif t == "}":
            depth -= 1
            loops = [lp for lp in loops if lp[0] <= depth]
            last_stmt = i
        elif t == ";":
            loops = [lp for lp in loops if not (lp[1] and lp[0] == depth)]
            last_stmt = i
        elif t.startswith("for"):
            end = matching(text, m.end() - 1)
            header = text[m.end():end - 1]
            rest = text[end:].lstrip()
            stmt = not rest.startswith("{")
            loops.append((depth + (0 if stmt else 1), stmt, header))
            i = end
        elif depth > 0:
            end = matching(text, m.end() - 1)
            args = split_args(text[m.end():end - 1])
            src, line = origin(m.start())
            if Path(src).is_absolute() and Path(src).is_relative_to(ROOT):
                src = Path(src).relative_to(ROOT).as_posix()
            call = {"file": src, "line": line, "func": func}
            try:
                call["defs"] = evaluate(ev, args, [lp[2] for lp in loops])
            except Unknown as e:
                call["why"] = str(e)
            calls.append(call)
            i = end
    return calls, ev.names


def evaluate(ev, args, headers):
    """The argument tuples of one call, over its loops (outermost first)."""
    if len(args) != len(PARAMS):
        raise Unknown(f"{len(args)} arguments")
    envs = [{}]
    for h in headers:
        m = LOOP.match(h)
        if not m:
            raise Unknown(f"loop header {h.strip()!r}")
        var, lo, op, hi, step = m.groups()
        nxt = []
        for env in envs:
            k, stop = ev.eval(lo, env), ev.eval(hi, env)
            inc = ev.eval(step, env) if step else 1
            while (k < stop if op == "<" else k <= stop if op == "<=" else k != stop):
                nxt.append(dict(env, **{var: k}))
                k += inc
                if len(nxt) > 4096:
                    raise Unknown("loop too long")
        envs = nxt
    out = []
    for env in envs:
        vals = {}
        for name, a in zip(PARAMS, args):
            v = ev.eval(a, env)
            bits = PARAM_BITS.get(name)
            vals[name] = v & ((1 << bits) - 1) if bits else v
        out.append(vals)
    return out


# ---------------------------------------------------------------- output

def sprite_name(sid, names):
    """The SPR_ enumerator at or below `sid`, as NAME or NAME + n."""
    best = None
    for n, v in names.items():
        if n.startswith("SPR_") and v <= sid and (best is None or v > best[1]):
            best = (n, v)
    if not best:
        return ""
    return best[0] if best[1] == sid else f"{best[0]} + {sid - best[1]}"


def tpage_of(page, mode):
    """DefineSprite's texture page word."""
    return (page & 0x7F) | mode << 7


def clut_of(x, y):
    """DefineSprite's CLUT word."""
    return ((y << 6) | (x >> 4)) & 0xFFFF


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("image", help="disc image (.bin raw or .iso), data track")
    ap.add_argument("--out", type=Path, default=OUT)
    ap.add_argument("--only", action="append", default=[],
                    help="a unit (stage, ovl02), or a stage's pack (ovl03-pack; ovl03 selects both); repeatable")
    args = ap.parse_args()

    disc, _ = extract_exe.open_disc(args.image)
    files = eo.all_files(disc)
    by_lba = {lba: name for name, (lba, _) in files.items()}
    file_tims = {}

    def tims_of(name):
        if name not in file_tims:
            file_tims[name] = tims(disc.read(*files[name]))
        return file_tims[name]

    every = [n for n in sorted(files, key=lambda n: files[n][0]) if n.startswith("CDDATA/")]

    args.out.mkdir(parents=True, exist_ok=True)
    for old in args.out.glob("*.png"):
        old.unlink()
    sheets = []  # (key, note, [card])
    seen = {}

    def want(key):
        return not args.only or any(key == o or key.startswith(o + "-") for o in args.only)

    # DefineSprite calls in the C.
    units = sorted(ROOT.glob("src/*.c")) + sorted(ROOT.glob("src/ovl/*.c"))
    for path in units:
        key = path.stem
        if not want(key):
            continue
        calls, names = definitions(path)
        calls = [c for c in calls if c["file"] != "?"]
        if not calls:
            continue
        if path.parent.name == "ovl" and key[3:].isdigit() and int(key[3:]) in eo.OVERLAYS:
            ovl_file = eo.OVERLAYS[int(key[3:])][0]
            d, stem = ovl_file.rsplit("/", 1)
            dir_files = [n for n in every if n.startswith(d + "/")]
            own = [[n for n in dir_files if n.rsplit("/", 1)[1][:2] == stem[:2]], dir_files,
                   [n for n in every if n.startswith("CDDATA/0/")]]
        else:
            own = [[n for n in every if n.startswith("CDDATA/0/")]]
        vram = tiered(tims_of, own, every)
        cards = []
        for c in calls:
            where = f"{c['file']}:{c['line']} {c['func']}"
            if "why" in c:
                cards.append({"title": "unresolved", "where": where, "note": c["why"]})
                continue
            for d in c["defs"]:
                cards.append(card(vram, d, names, where, args.out, key, seen))
        sheets.append((key, "DefineSprite calls in " + path.relative_to(ROOT).as_posix(), cards))
        print(f"{key}: {sum(len(c.get('defs', [])) for c in calls)} sprites, "
              f"{sum('why' in c for c in calls)} unresolved calls")

    # Stage sprite packs.
    common = None
    for name, lbas in disc_tables().items():
        m = re.fullmatch(r"Ovl(\d+)Loc", name)
        if not m or len(lbas) <= 5 or lbas[5] not in by_lba:
            continue
        ovl = int(m.group(1))
        key = f"ovl{ovl:02}-pack"
        if not want(key):
            continue
        fname = by_lba[lbas[5]]
        data = disc.read(*files[fname])
        entries = pack_entries(data)
        if entries is None:
            continue
        if common is None:
            common = Evaluator({})
            enums(split_lines(preprocess(ROOT / "src" / "stage.c"))[0], common)
        cards = []
        for n, off in enumerate(entries):
            t = tims(data[off:])
            if not t:
                cards.append({"title": f"{SPR_STAGE + n:#x}", "where": f"{fname} entry {n}",
                              "note": "no TIM at its offset"})
                continue
            clut, pix = t[0]
            x, y, w, h = pix[:4]
            depth = struct.unpack_from("<I", data, off + 4)[0] & 3
            per = (4, 2, 1, 1)[depth]
            tp = (y & 0x100) >> 4 | (x & 0x3FF) >> 6 | depth << 7
            d = {"id": SPR_STAGE + n, "mode": depth, "w": (w * per) & 0xFF, "h": h & 0xFF,
                 "page": tp & 0x7F, "u": ((x % 64) * per) & 0xFF, "v": y % 256,
                 "clutX": clut[0] if clut else 0, "clutY": clut[1] if clut else 0}
            vram = Vram([regions(t[:1], fname)])
            cards.append(card(vram, d, common.names, f"{fname} entry {n}", args.out, key, seen))
        stage = f"{(ovl - 3) // 3 + 1}-{(ovl - 3) % 3 + 1}"
        sheets.append((key, f"Stage {stage}'s sprite pack: LoadSpritePack registers "
                            f"{fname} (Ovl{ovl:02}Loc[5]) as SPR_STAGE + n", cards))
        print(f"{key}: {len(cards)} sprites from {fname}")

    write_index(args.out, sheets)
    print(f"wrote {(args.out / 'index.html').relative_to(ROOT)}")
    return 0


def tiered(tims_of, own, every):
    """The unit's own file groups, then the whole disc, as Vram tiers."""
    out, seen = [], set()
    for group in own + [every]:
        rs = []
        for n in group:
            if n not in seen:
                seen.add(n)
                rs += regions(tims_of(n), n)
        out.append(rs)
    return Vram(out)


def pack_entries(data):
    """A sprite pack's TIM offsets, or None if `data` is not one."""
    if len(data) < 16 or data[8:12] != b"TIM\0":
        return None
    count = struct.unpack_from("<H", data, 14)[0]
    out = []
    for n in range(count):
        off, _, tag = struct.unpack_from("<II4s", data, n * 16)
        if tag != b"TIM\0" or off >= len(data):
            return None
        out.append(off)
    return out


def card(vram, d, names, where, out_dir, key, seen):
    """One sprite's entry on a sheet; renders it to out_dir. `seen` counts
    the ids already written per sheet, to name a redefinition's file."""
    tpage = tpage_of(d["page"], d["mode"])
    clut = clut_of(d["clutX"], d["clutY"])
    sid = d["id"]
    info = {"title": f"{sid:#x}", "name": sprite_name(sid, names), "where": where,
            "size": f"{d['w']}x{d['h']}"}
    if d["mode"] >= 2:
        info["note"] = "a 16-bit page: the game draws only frozen frames this way, which are not on the disc"
        return info
    if d["w"] == 0 or d["h"] == 0:
        info["note"] = "empty rectangle"
        return info
    rows, used = render(vram, tpage, d["u"], d["v"], d["w"], d["h"], clut)
    n = seen[key, sid] = seen.get((key, sid), -1) + 1
    fname = f"{key}_{sid:03x}" + (f"_{n}" if n else "") + ".png"
    data = png(rows, d["w"], d["h"])
    (out_dir / fname).write_bytes(data)
    info["img"] = fname
    info["src"] = "data:image/png;base64," + base64.b64encode(data).decode()
    info["from"] = ", ".join(sorted(f for f in used if f)) or "nothing: all texels missing"
    if used and all(r[k + 3] == 0 for r in rows for k in range(0, len(r), 4)):
        info["note"] = "every texel transparent: probably not the file this sprite uses"
    return info


def write_index(out_dir, sheets):
    """build/sprites/index.html: every sheet, images embedded."""
    parts = ["<!doctype html><meta charset=utf-8><title>Pepsiman sprites</title><style>",
             "body{background:#24242e;color:#ddd;font:13px system-ui,sans-serif;margin:16px}",
             "h2{margin:28px 0 4px}p.n{color:#999;margin:0 0 10px}",
             ".g{display:flex;flex-wrap:wrap;gap:10px}",
             ".c{background:#30303c;border-radius:6px;padding:8px;width:min(300px,100%)}",
             ".c img{image-rendering:pixelated;max-width:100%;height:auto;background:",
             "repeating-conic-gradient(#3a3a48 0 25%,#44445a 0 50%) 0 0/16px 16px}",
             ".t{font-weight:600}.s{color:#aaa;font-size:12px;word-break:break-word}",
             "nav{display:flex;flex-wrap:wrap;gap:4px 12px}nav a{color:#9cf}</style>",
             "<h1>Pepsiman sprites</h1><p class=n>Rendered by tools/sprites.py from your "
             "own disc. Not for the repository.</p><nav>"]
    parts += [f"<a href='#{html.escape(k)}'>{html.escape(k)}</a>" for k, _, _ in sheets]
    parts.append("</nav>")
    for key, note, cards in sheets:
        parts.append(f"<h2 id='{html.escape(key)}'>{html.escape(key)}</h2>"
                     f"<p class=n>{html.escape(note)}</p><div class=g>")
        for c in cards:
            parts.append("<div class=c>")
            if "img" in c:
                w, h = (int(v) for v in c["size"].split("x"))
                scale = max(1, min(4, 128 // max(w, h, 1)))
                parts.append(f"<img src='{c['src']}' title='{c['img']}' width={w * scale} height={h * scale}>")
            parts.append(f"<div class=t>{html.escape(c['title'])} {html.escape(c.get('name', ''))}</div>")
            for k in ("size", "where", "from", "note"):
                if c.get(k):
                    parts.append(f"<div class=s>{html.escape(k)}: {html.escape(c[k])}</div>")
            parts.append("</div>")
        parts.append("</div>")
    (out_dir / "index.html").write_text("".join(parts))


if __name__ == "__main__":
    sys.exit(main())
