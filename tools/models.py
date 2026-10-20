#!/usr/bin/env python3
"""Render every stage's trap and object models, from the TMDs on your disc,
and say which trap records use each and which functions run them.

    .venv/bin/python3 tools/models.py "Pepsiman (Japan) (Track 1).bin"
    .venv/bin/python3 tools/models.py disc.bin --only ovl03 --only ovl13

Writes build/models/index.html, one PNG per model (four views of it) and
build/models/models.json (every trap record, its model, type, act and call
tree). The models and textures are Sony's and the game's: they stay in
build/, which is gitignored, and are never committed.

A stage's loader (src/main.c, gSubStep 8 to 14; stage3.inc's LoadStageFiles
for world 3) reads its file table (include/disc_files.inc) like this:

  * Loc[4] to RAM_OBJECT_MODELS, one TMD; InitEntryModels (src/object.c)
    makes gObjectModels[i] its object i, and a trap's `model` indexes
    gObjectModels. So a trap's model is object `model` of Loc[4].
  * Loc[6] (Loc[7] in world 3) holds the saved tables RestoreStageObjects
    copies: 200 Trap records of 0x48 bytes at 0x6778 (at 0 in world 3).
    A record's model is the s16 at 0x36 (-1: free), its switch the s16 at
    0x34, its type and act the bytes at 0x42 and 0x43.
  * Loc[1] and Loc[2] are its TIM list and TIM pack, Loc[5] its sprite
    pack; with the boot files' TIMs they are the VRAM the models sample.

Which function runs a record is read from the C. The engine stages run
StepStageTrap (src/ovl/engine_code.inc), a switch on the type that calls a
dispatcher, which switches on the act and calls a step routine. Each
overlay unit is run through cpp and its functions parsed with pycparser;
the walk from StepStageTrap follows only the cases and `if`s the record's
type, act and stage (gStage) select, and every call that passes the trap
on, to a few levels. PlaySe calls met on the way are listed. Stage code
that reaches a record by its index (sTraps[11]) is listed too.

The chase stages (x-3) are read differently: their objects are each
overlay's sPlacements list, a kind picks the step from the overlay's step
table, and each step draws a piece model (object n - 10 of the act-3 pack's
part 3, Loc[2], for its sPieceObjs[n]).

The renders are orthographic, from four sides at 30 degrees above,
flat-shaded with a fixed light, textured from VRAM (magenta where no TIM
covers a texel). Not pretty, but recognisable.
"""
import argparse
import html
import json
import math
import re
import struct
import sys
from array import array
from pathlib import Path

import extract_exe
import extract_overlays as eo
from csource import Evaluator, Unknown, enums, preprocess, split_lines
from sprites import disc_tables, png, tims

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build" / "models"

TRAP_TABLE = 0x6778    # the Trap records' offset in Loc[6]
TRAP_SIZE = 0x48
TRAP_COUNT = 200
VIEW = 112             # each view's size in pixels; a model's PNG is 2x2 views
DEPTH = 3              # how far below StepStageTrap the call walk goes
MISSING = (255, 0, 255)
STAGE3 = (9, 10)       # world 3's overlays: tables in Loc[7], no type dispatch
# The chase stages (x-3) have no Trap records: their gTraps holds chase.inc's
# 20-byte Act3Obj, placed from the overlay's own list (chase_stage()).


def stage_name(ovl):
    return f"{(ovl - 3) // 3 + 1}-{(ovl - 3) % 3 + 1}"


# ---------------------------------------------------------------- TMD

def tmd_objects(data):
    """A TMD's objects: each a dict of verts [(x, y, z)] and prims, each
    prim a dict: v (vertex indices), uv ([(u, v)] or None), tsb, cba and
    rgb (one or one per vertex)."""
    ident, flags, nobj = struct.unpack_from("<III", data, 0)
    if ident != 0x41:
        raise ValueError("not a TMD")
    base = 12
    out = []
    for n in range(nobj):
        vt, nv, _nt, _nn, pt, np_, _scale = struct.unpack_from("<IIIIIIi", data, base + n * 28)
        if not flags & 1:
            vt, pt = vt + base, pt + base
        verts = [struct.unpack_from("<hhh", data, vt + i * 8) for i in range(nv)]
        prims = []
        p = pt
        for _ in range(np_):
            olen, ilen, flag, mode = struct.unpack_from("<BBBB", data, p)
            prim = prim_of(data, p + 4, ilen * 4, flag, mode)
            if prim and all(i < nv for i in prim["v"]):
                prims.append(prim)
            p += 4 + ilen * 4
        out.append({"verts": verts, "prims": prims})
    return out


def prim_of(data, p, size, flag, mode):
    """One TMD primitive's packet, decoded; None for a line or sprite."""
    if mode >> 5 != 1:
        return None
    quad, tme, iip = bool(mode & 8), bool(mode & 4), bool(mode & 0x10)
    lit, grd = not flag & 1, bool(flag & 4)
    n = 4 if quad else 3
    end = p + size
    uv, tsb, cba = None, 0, 0
    if tme:
        uv = []
        for k in range(n):
            u, v, w = struct.unpack_from("<BBH", data, p + 4 * k)
            uv.append((u, v))
            if k == 0:
                cba = w
            elif k == 1:
                tsb = w
        p += 4 * n
    rgb = [(128, 128, 128)]
    if not tme or not lit:
        k = n if (grd or (iip and not lit)) else 1
        rgb = [struct.unpack_from("<BBB", data, p + 4 * i) for i in range(k)]
        p += 4 * k
    idx = struct.unpack_from(f"<{(end - p) // 2}H", data, p)
    if lit and iip:
        v = list(idx[1:2 * n:2])
    elif lit:
        v = list(idx[1:1 + n])
    else:
        v = list(idx[:n])
    if len(v) != n:
        return None
    return {"v": v, "uv": uv, "tsb": tsb, "cba": cba, "rgb": rgb}


# ---------------------------------------------------------------- VRAM

class Vram:
    """1024x512 halfwords, filled by loading TIMs in order; `have` marks
    what a TIM covered."""

    def __init__(self):
        self.px = array("H", bytes(1024 * 512 * 2))
        self.have = bytearray(1024 * 512)

    def load(self, data):
        for clut, pix in tims(data):
            for x, y, w, h, d, off in ([clut] if clut else []) + [pix]:
                for yy in range(h):
                    if y + yy >= 512:
                        break
                    row = array("H", d[off + yy * w * 2:off + (yy + 1) * w * 2])
                    ww = min(w, 1024 - x)
                    o = (y + yy) * 1024 + x
                    self.px[o:o + ww] = row[:ww]
                    self.have[o:o + ww] = b"\1" * ww

    def texel(self, tsb, cba, u, v, cache):
        """RGB of a texel, None when transparent (0)."""
        key = (tsb, cba, u, v)
        if key in cache:
            return cache[key]
        bx, by, depth = (tsb & 15) * 64, (tsb >> 4 & 1) * 256, tsb >> 7 & 3
        if depth == 0:
            o = (by + v) * 1024 + bx + (u >> 2)
            c = self.px[o] >> (u & 3) * 4 & 15
            hv = self.have[o]
            o = (cba >> 6 & 511) * 1024 + (cba & 63) * 16 + c
        elif depth == 1:
            o = (by + v) * 1024 + bx + (u >> 1)
            c = self.px[o] >> (u & 1) * 8 & 255
            hv = self.have[o]
            o = (cba >> 6 & 511) * 1024 + (cba & 63) * 16 + c
        else:
            o = (by + v) * 1024 + bx + u
            hv = 1
        if o >= len(self.px) or not (hv and self.have[o]):
            out = MISSING
        else:
            c = self.px[o]
            out = None if c == 0 else ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3)
        cache[key] = out
        return out


# ---------------------------------------------------------------- render

LIGHT = (-0.45, -0.75, -0.5)   # towards the light: from above (y is down)
_l = math.sqrt(sum(c * c for c in LIGHT))
LIGHT = tuple(c / _l for c in LIGHT)


def view(obj, vram, yaw, pitch, size, cache):
    """One orthographic view of an object: rows of RGBA bytes."""
    cy, sy = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
    cp, sp = math.cos(math.radians(pitch)), math.sin(math.radians(pitch))

    def rot(x, y, z):
        x, z = x * cy - z * sy, x * sy + z * cy
        y, z = y * cp - z * sp, y * sp + z * cp
        return x, y, z

    pts = [rot(*v) for v in obj["verts"]]
    used = sorted({i for p in obj["prims"] for i in p["v"]})
    img = bytearray(size * size * 4)
    if not used:
        return img
    xs = [pts[i][0] for i in used]
    ys = [pts[i][1] for i in used]
    span = max(max(xs) - min(xs), max(ys) - min(ys), 1)
    k = (size - 6) / span
    ox, oy = (size - k * (max(xs) + min(xs))) / 2, (size - k * (max(ys) + min(ys))) / 2
    scr = [(x * k + ox, y * k + oy, z) for x, y, z in pts]
    zbuf = [1e30] * (size * size)
    for p in obj["prims"]:
        tris = [(0, 1, 2), (1, 3, 2)] if len(p["v"]) == 4 else [(0, 1, 2)]
        a, b, c = (pts[p["v"][i]] for i in (0, 1, 2))
        nx = (b[1] - a[1]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[1] - a[1])
        ny = (b[2] - a[2]) * (c[0] - a[0]) - (b[0] - a[0]) * (c[2] - a[2])
        nz = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
        ln = math.sqrt(nx * nx + ny * ny + nz * nz) or 1
        rl, ry, rz = LIGHT
        shade = 0.45 + 0.55 * abs(nx * rl + ny * ry + nz * rz) / ln
        for t in tris:
            tri(img, zbuf, size, [scr[p["v"][i]] for i in t], p, t, shade, vram, cache)
    return img


def tri(img, zbuf, size, s, p, t, shade, vram, cache):
    """Rasterises one triangle into img/zbuf."""
    (x0, y0, z0), (x1, y1, z1), (x2, y2, z2) = s
    area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)
    if abs(area) < 1e-9:
        return
    uv = [p["uv"][i] for i in t] if p["uv"] else None
    rgb = p["rgb"]
    cols = [rgb[i] if len(rgb) > 1 else rgb[0] for i in t]
    flat = cols[0] if len(set(cols)) == 1 else None
    minx, maxx = max(0, int(min(x0, x1, x2))), min(size - 1, int(max(x0, x1, x2)) + 1)
    miny, maxy = max(0, int(min(y0, y1, y2))), min(size - 1, int(max(y0, y1, y2)) + 1)
    inv = 1 / area
    for py in range(miny, maxy + 1):
        fy = py + 0.5
        for px in range(minx, maxx + 1):
            fx = px + 0.5
            w0 = ((x1 - fx) * (y2 - fy) - (x2 - fx) * (y1 - fy)) * inv
            w1 = ((x2 - fx) * (y0 - fy) - (x0 - fx) * (y2 - fy)) * inv
            w2 = 1 - w0 - w1
            if w0 < -1e-6 or w1 < -1e-6 or w2 < -1e-6:
                continue
            z = w0 * z0 + w1 * z1 + w2 * z2
            o = py * size + px
            if z >= zbuf[o]:
                continue
            if flat:
                r, g, b = flat
            else:
                r = w0 * cols[0][0] + w1 * cols[1][0] + w2 * cols[2][0]
                g = w0 * cols[0][1] + w1 * cols[1][1] + w2 * cols[2][1]
                b = w0 * cols[0][2] + w1 * cols[1][2] + w2 * cols[2][2]
            if uv:
                u = int(w0 * uv[0][0] + w1 * uv[1][0] + w2 * uv[2][0] + 0.5) & 255
                v = int(w0 * uv[0][1] + w1 * uv[1][1] + w2 * uv[2][1] + 0.5) & 255
                tx = vram.texel(p["tsb"], p["cba"], u, v, cache)
                if tx is None:
                    continue
                if tx is MISSING:
                    r, g, b = tx
                else:
                    r, g, b = tx[0] * r / 128, tx[1] * g / 128, tx[2] * b / 128
            zbuf[o] = z
            o *= 4
            img[o] = min(255, int(r * shade))
            img[o + 1] = min(255, int(g * shade))
            img[o + 2] = min(255, int(b * shade))
            img[o + 3] = 255


def picture(obj, vram, size=VIEW):
    """Four views of an object, 2x2, as PNG bytes."""
    cache = {}
    views = [view(obj, vram, yaw, -30, size, cache) for yaw in (30, 120, 210, 300)]
    w = size * 2
    rows = []
    for half in (0, 1):
        for y in range(size):
            row = b""
            for vi in (half * 2, half * 2 + 1):
                row += bytes(views[vi][y * size * 4:(y + 1) * size * 4])
            rows.append(row)
    return png(rows, w, w)


# ---------------------------------------------------------------- the C

def functions(text):
    """Every function definition in preprocessed text: {name: (params,
    body, offset of its body)}."""
    out = {}
    depth, last, i = 0, 0, 0
    token = re.compile(r"[{};]")
    start = None
    while True:
        m = token.search(text, i)
        if not m:
            break
        t, i = m.group(0), m.end()
        if t == "{":
            if depth == 0:
                head = re.search(r"(\w+)\s*\(([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*)\)\s*$",
                                 text[last:m.start()])
                start = (head.group(1), head.group(2), m.start()) if head else None
            depth += 1
        elif t == "}":
            depth -= 1
            if depth == 0:
                if start and start[0] not in ("if", "while", "for", "switch"):
                    out[start[0]] = (start[1], text[start[2]:i], start[2])
                start = None
                last = i
        elif t == ";" and depth == 0:
            last = i
    return out


def typedef_names(text):
    """The typedef names a unit declares, for pycparser."""
    out = set()
    for m in re.finditer(r"\btypedef\b", text):
        depth, j = 0, m.end()
        while j < len(text):
            c = text[j]
            depth += c in "{("
            depth -= c in "})"
            if c == ";" and depth == 0:
                break
            j += 1
        decl = text[m.end():j]
        fp = re.search(r"\(\s*\*\s*(\w+)\s*\)", decl)
        name = fp.group(1) if fp else (re.findall(r"(\w+)\s*(?:\[[^\]]*\])*\s*$", decl) or [None])[0]
        if name:
            out.add(name)
    return out


def strip_gnu(text):
    """Body text with GNU asm statements and attributes removed."""
    out, i = [], 0
    for m in re.finditer(r"\b(?:__asm__|asm|__attribute__)\b(?:\s+(?:volatile|__volatile__))?\s*\(", text):
        if m.start() < i:
            continue
        depth, j = 0, m.end() - 1
        while j < len(text):
            depth += text[j] == "("
            depth -= text[j] == ")"
            j += 1
            if depth == 0:
                break
        out.append(text[i:m.start()])
        out.append("0" if not m.group(0).startswith("__attribute__") else "")
        i = j
    out.append(text[i:])
    s = "".join(out)
    return re.sub(r"\b(?:__inline__|__inline|inline|__extension__|volatile|register)\b", "", s)


class TrapEval(Evaluator):
    """Evaluator that also reads the trap's fields from the environment."""

    def node(self, n, env):
        kind = type(n).__name__
        if kind == "StructRef" and type(n.name).__name__ == "ID":
            key = n.name.name + n.type + n.field.name
            if key in env:
                return env[key]
            raise Unknown(key)
        return super().node(n, env)


class Unit:
    """An overlay unit's functions, walkable for one trap record."""

    def __init__(self, path):
        from pycparser import c_ast, c_generator, c_parser
        self.ast, self.gen = c_ast, c_generator.CGenerator()
        text, where = split_lines(preprocess(path))
        self.text, self.where = text, where
        self.starts = [0]
        for line in text.split("\n"):
            self.starts.append(self.starts[-1] + len(line) + 1)
        self.funcs = functions(text)
        self.ev = TrapEval({})
        enums(text, self.ev)
        self.preamble = "".join(f"typedef int {n};" for n in sorted(typedef_names(text)))
        self.parser = c_parser.CParser()
        self.parsed = {}

    def origin(self, pos):
        lo, hi = 0, len(self.starts) - 1
        while lo + 1 < hi:
            mid = (lo + hi) // 2
            lo, hi = (mid, hi) if self.starts[mid] <= pos else (lo, mid)
        f, line = self.where[lo]
        p = Path(f)
        if p.is_absolute() and p.is_relative_to(ROOT):
            f = p.relative_to(ROOT).as_posix()
        return f, line

    def func(self, name):
        """(param names, body AST) of a function, or None if unparsable."""
        if name not in self.parsed:
            params, body, _ = self.funcs[name]
            try:
                ast = self.parser.parse(f"{self.preamble} void {name}({strip_gnu(params)}) "
                                        f"{strip_gnu(body)}")
                fd = ast.ext[-1]
                ps = fd.decl.type.args.params if fd.decl.type.args else []
                names = [getattr(p, "name", None) for p in ps]
                self.parsed[name] = (names, fd.body)
            except Exception:
                self.parsed[name] = None
        return self.parsed[name]

    def walk(self, name, env, depth=0, stack=()):
        """The call tree of `name` under env: {name, args, calls, se}."""
        node = {"name": name, "children": [], "se": []}
        f = self.func(name)
        if f is None:
            node["note"] = "not parsed"
            return node
        trap = env.get("@trap")
        calls = []
        self.stmt(f[1], env, calls)
        for callee, args, cenv in calls:
            if callee.startswith("PlaySe"):
                se = f"{callee}({', '.join(args)})"
                if se not in node["se"]:
                    node["se"].append(se)
                continue
            if callee not in self.funcs or callee in stack or not trap:
                continue
            if not any(re.search(rf"\b{re.escape(trap)}\b", a) for a in args):
                continue
            if any(c["name"] == callee for c in node["children"]):
                continue
            child = {"name": callee, "args": args, "children": [], "se": []}
            if depth < DEPTH:
                child = dict(self.walk(callee, self.bind(callee, args, cenv, trap), depth + 1,
                                       stack + (name,)), args=args)
            node["children"].append(child)
        return node

    def bind(self, callee, args, env, trap):
        """The callee's environment: the trap's fields under its own
        parameter name, and each constant argument."""
        f = self.func(callee)
        new = {"gStage": env["gStage"]}
        if f is None:
            return new
        for pname, a in zip(f[0], args):
            if not pname:
                continue
            if re.fullmatch(rf"(?:\(\s*\w+\s*\*\s*\)\s*)?{re.escape(trap)}", a):
                new["@trap"] = pname
                for fld in ("type", "act"):
                    new[pname + "->" + fld] = env[trap + "->" + fld]
                continue
            try:
                new[pname] = self.ev.eval(a, self.env_names(env))
            except Unknown:
                pass
        return new

    @staticmethod
    def env_names(env):
        return {k: v for k, v in env.items() if not k.startswith("@")}

    def value(self, expr, env):
        try:
            return self.ev.node(expr, self.env_names(env))
        except (Unknown, KeyError, ValueError, AttributeError, TypeError):
            return None

    def exprs(self, n, env, calls):
        """Collects the calls in an expression tree."""
        a = self.ast
        if n is None:
            return
        if isinstance(n, a.FuncCall) and isinstance(n.name, a.ID):
            args = [self.gen.visit(x) for x in (n.args.exprs if n.args else [])]
            calls.append((n.name.name, args, env))
        for _, c in n.children():
            self.exprs(c, env, calls)

    def stmt(self, s, env, calls):
        """Walks a statement; returns "break", "return" or None."""
        a = self.ast
        if s is None:
            return None
        if isinstance(s, a.Compound):
            for item in s.block_items or []:
                r = self.stmt(item, env, calls)
                if r:
                    return r
            return None
        if isinstance(s, a.If):
            self.exprs(s.cond, env, calls)
            v = self.value(s.cond, env)
            if v is not None:
                return self.stmt(s.iftrue if v else s.iffalse, env, calls)
            r1 = self.stmt(s.iftrue, env, calls)
            r2 = self.stmt(s.iffalse, env, calls)
            return r1 if r1 == r2 else None
        if isinstance(s, a.Switch):
            self.exprs(s.cond, env, calls)
            v = self.value(s.cond, env)
            items = s.stmt.block_items or [] if isinstance(s.stmt, a.Compound) else [s.stmt]
            labels = [i for i in items if isinstance(i, (a.Case, a.Default))]
            if v is None:
                for c in labels:
                    for item in c.stmts or []:
                        if self.stmt(item, env, calls):
                            break
                return None
            start = None
            for k, c in enumerate(labels):
                if isinstance(c, a.Case) and self.value(c.expr, env) == v:
                    start = k
                    break
            if start is None:
                start = next((k for k, c in enumerate(labels) if isinstance(c, a.Default)), None)
            if start is None:
                return None
            for c in labels[start:]:
                for item in c.stmts or []:
                    r = self.stmt(item, env, calls)
                    if r == "break":
                        return None
                    if r:
                        return r
            return None
        if isinstance(s, (a.Case, a.Default)):
            for item in s.stmts or []:
                r = self.stmt(item, env, calls)
                if r:
                    return r
            return None
        if isinstance(s, a.Break):
            return "break"
        if isinstance(s, a.Return):
            self.exprs(s.expr, env, calls)
            return "return"
        if isinstance(s, (a.For, a.While, a.DoWhile)):
            for part in ("init", "cond", "next"):
                self.exprs(getattr(s, part, None), env, calls)
            self.stmt(s.stmt, env, calls)
            return None
        if isinstance(s, a.Label):
            return self.stmt(s.stmt, env, calls)
        self.exprs(s, env, calls)
        return None

    def run(self, trap_type, act, stage):
        """The call tree StepStageTrap runs for a record."""
        env = {"@trap": "t", "t->type": trap_type, "t->act": act, "gStage": stage}
        return self.walk("StepStageTrap", env)

    def index_refs(self):
        """{record index: [(array, function)]}: stage code that reaches a
        placed trap by a constant index."""
        out = {}
        for name, (_, body, _) in self.funcs.items():
            for m in re.finditer(r"\b(sTraps|gTraps)\s*\)?\s*\[\s*(\d+)\s*\]", body):
                ref = (m.group(1), name)
                lst = out.setdefault(int(m.group(2)), [])
                if ref not in lst:
                    lst.append(ref)
        return out

    def where_is(self, name):
        return self.origin(self.funcs[name][2]) if name in self.funcs else ("?", 0)


def flat_names(tree):
    """Every function in a call tree below its root, depth-first."""
    out = []
    for c in tree["children"]:
        out.append(c["name"])
        out += flat_names(c)
    return out


def all_se(tree):
    out = list(tree["se"])
    for c in tree["children"]:
        out += [s for s in all_se(c) if s not in out]
    return out


# ---------------------------------------------------------------- records

def trap_records(data, off):
    """The live Trap records of a saved table."""
    out = []
    for i in range(TRAP_COUNT):
        r = data[off + i * TRAP_SIZE:off + (i + 1) * TRAP_SIZE]
        if len(r) < TRAP_SIZE:
            break
        x, y, z = struct.unpack_from("<iii", r, 0)
        rx, ry, rz = struct.unpack_from("<hhh", r, 0x18)
        switch, model = struct.unpack_from("<hh", r, 0x34)
        if model == -1:
            continue
        out.append({"index": i, "model": model, "switch": switch, "type": r[0x42], "act": r[0x43],
                    "pos": [x, y, z], "rot": [rx, ry, rz]})
    return out


# ---------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("image", help="disc image (.bin raw or .iso), data track")
    ap.add_argument("--out", type=Path, default=OUT)
    ap.add_argument("--only", action="append", default=[], help="a stage overlay (ovl03); repeatable")
    args = ap.parse_args()

    disc, _ = extract_exe.open_disc(args.image)
    files = eo.all_files(disc)
    by_lba = {lba: name for name, (lba, _) in files.items()}

    def read(name):
        return disc.read(*files[name])

    args.out.mkdir(parents=True, exist_ok=True)
    for old in args.out.glob("*.png"):
        old.unlink()
    boot = [n for n in sorted(files, key=lambda n: files[n][0]) if n.startswith("CDDATA/0/")]
    stages = []
    for name, lbas in sorted(disc_tables().items()):
        m = re.fullmatch(r"Ovl(\d+)Loc", name)
        if not m:
            continue
        ovl = int(m.group(1))
        key = f"ovl{ovl:02}"
        if args.only and key not in args.only:
            continue
        if (ovl - 3) % 3 == 2:
            if len(lbas) > 2 and lbas[2] in by_lba:
                print(f"{key} (stage {stage_name(ovl)}, chase): ", end="", flush=True)
                st = chase_stage(key, ovl, [by_lba.get(x) for x in lbas], read, boot, args.out)
                stages.append(st)
                print(f"{len(st['models'])} pieces, {len(st['records'])} objects")
            continue
        table = 7 if ovl in STAGE3 else 6
        if len(lbas) <= table or lbas[4] not in by_lba or lbas[table] not in by_lba:
            continue
        tmd = read(by_lba[lbas[4]])
        if tmd[:4] != b"\x41\0\0\0":
            continue
        print(f"{key} (stage {stage_name(ovl)}): ", end="", flush=True)
        vram = Vram()
        for n in boot + [by_lba[lbas[k]] for k in (1, 2, 5) if lbas[k] in by_lba]:
            vram.load(read(n))
        objs = tmd_objects(tmd)
        recs = trap_records(read(by_lba[lbas[table]]), 0 if ovl in STAGE3 else TRAP_TABLE)
        unit_path = ROOT / "src" / "ovl" / f"{key}.c"
        unit = Unit(unit_path)
        engine = "StepStageTrap" in unit.funcs
        refs = unit.index_refs()
        for r in recs:
            if engine:
                tree = unit.run(r["type"], r["act"], ovl - 3)
                r["tree"] = tree
                r["funcs"] = flat_names(tree)
                r["se"] = all_se(tree)
            else:
                r["funcs"], r["se"] = [], []
            r["refs"] = [f"{f} ({arr}[{r['index']}])" for arr, f in refs.get(r["index"], [])]
        models = render_models(objs, vram, {r["model"] for r in recs}, key, args.out)
        stages.append({"key": key, "ovl": ovl, "stage": stage_name(ovl), "engine": engine,
                       "tmd": by_lba[lbas[4]], "table": by_lba[lbas[table]],
                       "models": models, "records": recs,
                       "where": {f: "%s:%d" % unit.where_is(f)
                                 for r in recs for f in r["funcs"] + [x.split(" ")[0] for x in r["refs"]]}})
        print(f"{len(objs)} models, {len(recs)} traps")

    (args.out / "models.json").write_text(json.dumps(stages, indent=1))
    write_index(args.out, stages)
    index = (args.out / "index.html").resolve()
    print(f"wrote {index.relative_to(ROOT) if index.is_relative_to(ROOT) else index}")
    return 0


def render_models(objs, vram, used, key, out_dir):
    """Renders each object to out_dir; the stage's model list."""
    models = []
    for k, obj in enumerate(objs):
        fname = f"{key}_model{k:02}.png"
        info = {"index": k, "prims": len(obj["prims"]), "verts": len(obj["verts"]), "used": k in used}
        if obj["prims"]:
            (out_dir / fname).write_bytes(picture(obj, vram))
            info["img"] = fname
        models.append(info)
    return models


def chase_stage(key, ovl, locs, read, boot, out_dir):
    """A chase stage (x-3): its objects are the overlay's own placement
    list (sPlacements, kind and position), each kind's step the overlay's
    step table, and each step draws piece model n - 10 of its
    sPieceObjs[n], object n - 10 of part 3 of the act-3 pack LoadAct3Data
    reads (Loc[2]). Textures: the boot TIMs and Loc[1]."""
    pack = read(locs[2])
    offs = struct.unpack_from("<5I", pack, 0)
    pieces = tmd_objects(pack[offs[3]:])
    vram = Vram()
    for n in boot + [locs[1]]:
        vram.load(read(n))
    unit = Unit(ROOT / "src" / "ovl" / f"{key}.c")
    text = unit.text
    steps = []
    m = re.search(r"\{\s*(StepChasePiece\w+(?:\s*,\s*StepChasePiece\w+)*)\s*,?\s*\}", text)
    if m:
        steps = [x.strip() for x in m.group(1).split(",")]
    places = []
    m = re.search(r"\bsPlacements\s*\[\s*\d*\s*\]\s*=\s*\{", text)
    if m:
        body = text[m.end():text.index("};", m.end())]
        places = [tuple(int(v, 0) for v in p) for p in re.findall(
            r"\{\s*(-?\w+)\s*,\s*\{[^}]*\}\s*,\s*(-?\w+)\s*,\s*(-?\w+)\s*,\s*(-?\w+)\s*\}", body)]
    piece_of, se_of = {}, {}
    for name in set(steps):
        if name not in unit.funcs:
            continue
        idx = sorted({int(n) - 10 for n in re.findall(r"sPieceObjs\s*\[\s*(\d+)\s*\]", unit.funcs[name][1])})
        piece_of[name] = idx
        tree = unit.walk(name, {"@trap": "o", "gStage": ovl - 3})
        se_of[name] = all_se(tree)
    recs = []
    for i, (kind, x, y, z) in enumerate(places):
        step = steps[kind] if kind < len(steps) else None
        idx = piece_of.get(step, [])
        recs.append({"index": i, "model": idx[0] if idx else -1, "switch": -1, "type": kind, "act": 0,
                     "pos": [x, y, z], "rot": [0, 0, 0], "funcs": [step] if step else [],
                     "se": se_of.get(step, []), "refs": []})
    return {"key": key, "ovl": ovl, "stage": stage_name(ovl), "engine": True, "chase": True,
            "tmd": f"{locs[2]} part 3", "table": "sPlacements", "records": recs,
            "models": render_models(pieces, vram, {r["model"] for r in recs}, key, out_dir),
            "where": {f: "%s:%d" % unit.where_is(f) for f in set(steps) if f in unit.funcs}}


def write_index(out_dir, stages):
    """build/models/index.html: functions to pictures, then each stage."""
    def img(st, model, w=112):
        m = st["models"][model] if 0 <= model < len(st["models"]) else None
        if not m or "img" not in m:
            return f"<span class=s>model {model}: none</span>"
        return f"<img src='{m['img']}' width={w} title='{m['img']}' loading=lazy>"

    byfunc = {}
    for st in stages:
        for r in st["records"]:
            for f in dict.fromkeys(r["funcs"] + [x.split(" ")[0] for x in r["refs"]]):
                byfunc.setdefault(f, []).append((st, r))
    parts = ["<!doctype html><meta charset=utf-8><title>Pepsiman trap models</title><style>",
             "body{background:#24242e;color:#ddd;font:13px system-ui,sans-serif;margin:16px}",
             "h2{margin:28px 0 4px}h3{margin:18px 0 4px;font-family:monospace}p.n{color:#999;margin:0 0 10px}",
             ".g{display:flex;flex-wrap:wrap;gap:8px}",
             ".c{background:#30303c;border-radius:6px;padding:6px;width:min(240px,100%)}",
             ".c img{max-width:100%;height:auto;background:#1a1a22}",
             ".t{font-weight:600}.s{color:#aaa;font-size:12px;word-break:break-word}",
             "a{color:#9cf}nav{display:flex;flex-wrap:wrap;gap:4px 12px}code{font-size:12px}</style>",
             "<h1>Pepsiman trap models</h1><p class=n>Rendered by tools/models.py from your own "
             "disc. Not for the repository. Four views per model, 30&deg; above, from four sides.</p>",
             "<h2>Stages</h2><nav>"]
    parts += [f"<a href='#{s['key']}'>{s['key']} ({s['stage']})</a>" for s in stages]
    parts.append("</nav><h2>Functions</h2><p class=n>Every function the walk from StepStageTrap "
                 "reaches for a record, and stage code that names a record by index.</p><nav>")
    parts += [f"<a href='#f-{html.escape(f)}'>{html.escape(f)}</a>" for f in sorted(byfunc)]
    parts.append("</nav>")
    for f in sorted(byfunc):
        where = sorted({st["where"][f] for st, _ in byfunc[f] if f in st["where"]})
        parts.append(f"<h3 id='f-{html.escape(f)}'>{html.escape(f)}</h3>"
                     f"<p class=n>{html.escape(', '.join(where[:3]))}</p><div class=g>")
        seen = set()
        for st, r in byfunc[f]:
            k = (st["key"], r["model"], r["type"], r["act"])
            if k in seen:
                continue
            seen.add(k)
            n = sum(1 for s2, r2 in byfunc[f] if (s2["key"], r2["model"], r2["type"], r2["act"]) == k)
            parts.append(f"<div class=c>{img(st, r['model'])}<div class=t>"
                         f"<a href='#{st['key']}-r{r['index']}'>{st['key']} ({st['stage']}) "
                         f"#{r['index']}</a></div><div class=s>model {r['model']}, "
                         + (f"kind {r['type']}" if st.get("chase") else
                            f"type {r['type']:#x}, act {r['act']:#x}") + (f", {n} records" if n > 1 else "") + "</div></div>")
        parts.append("</div>")
    for st in stages:
        parts.append(f"<h2 id='{st['key']}'>{st['key']}: stage {st['stage']}</h2><p class=n>"
                     f"models {html.escape(st['tmd'])}, traps {html.escape(st['table'])}"
                     + ("" if st["engine"] else "; no StepStageTrap: this stage runs its traps "
                        "from its own code") + "</p><div class=g>")
        for r in st["records"]:
            chain = tree_text(r["tree"]) if "tree" in r else ""
            parts.append(f"<div class=c id='{st['key']}-r{r['index']}'>{img(st, r['model'])}"
                         f"<div class=t>#{r['index']} model {r['model']}</div>"
                         + (f"<div class=s>kind {r['type']}</div>" if st.get("chase") else
                          f"<div class=s>type {r['type']:#x} act {r['act']:#x}, switch {r['switch']}</div>"))
            if chain:
                parts.append(f"<div class=s><code>{chain}</code></div>")
            elif st.get("chase") and r["funcs"]:
                parts.append(f"<div class=s><code>&rarr; <a href='#f-{r['funcs'][0]}'>{r['funcs'][0]}</a>"
                             "</code></div>")
            if r["se"]:
                parts.append(f"<div class=s>sounds: {html.escape(', '.join(r['se']))}</div>")
            if r["refs"]:
                parts.append(f"<div class=s>named by: {html.escape(', '.join(r['refs']))}</div>")
            parts.append("</div>")
        parts.append("</div><h3>All models</h3><div class=g>")
        for m in st["models"]:
            parts.append(f"<div class=c>{img(st, m['index'], 160)}<div class=s>model {m['index']}"
                         f"{'' if m['used'] else ' (no trap record)'}, {m['prims']} polygons</div></div>")
        parts.append("</div>")
    (out_dir / "index.html").write_text("".join(parts))


def tree_text(tree, depth=0):
    """A call tree as nested html."""
    out = []
    for c in tree["children"]:
        out.append("&nbsp;" * 2 * depth + "&rarr; " + f"<a href='#f-{html.escape(c['name'])}'>"
                   f"{html.escape(c['name'])}</a>({html.escape(', '.join(c.get('args', [])))})<br>")
        out.append(tree_text(c, depth + 1))
    return "".join(out)


if __name__ == "__main__":
    sys.exit(main())
