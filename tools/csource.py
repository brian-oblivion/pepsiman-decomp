"""Reading the game's C for the tools: a unit through the project's cpp,
its enums, and the calls to one function with their arguments evaluated.

    from csource import calls, loop_envs
    for call in calls(Path("src/stage.c"), "DefineSprite"):
        ...

calls() finds every call to a function in a unit (macros and .inc files
expanded, as the compiler sees them), with the source file and line it
came from, the function it is in and the `for` loops around it. Arguments
are evaluated by Evaluator, over the unit's enums and the loop variables;
an argument that depends on runtime state raises Unknown.
"""
import re
import subprocess
import sys
from pathlib import Path

from m2ctx import CPP, CPP_ARGS

ROOT = Path(__file__).resolve().parent.parent


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


def calls(path, name):
    """Every call to `name` in a unit, and the unit's Evaluator (its enums
    loaded). Each call is a dict: file, line, func, args (the argument
    texts) and loops (the headers of the `for` loops around it, outermost
    first)."""
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
    out = []
    depth = 0
    loops = []  # (brace depth of the body, statement-only, header text)
    func = "?"
    last_stmt = 0
    i = 0
    token = re.compile(r"[{};]|\bfor\s*\(|\b" + re.escape(name) + r"\s*\(")
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
            stmt = not text[end:].lstrip().startswith("{")
            loops.append((depth + (0 if stmt else 1), stmt, header))
            i = end
        elif depth > 0:
            end = matching(text, m.end() - 1)
            src, line = origin(m.start())
            if Path(src).is_absolute() and Path(src).is_relative_to(ROOT):
                src = Path(src).relative_to(ROOT).as_posix()
            out.append({"file": src, "line": line, "func": func,
                        "args": split_args(text[m.end():end - 1]),
                        "loops": [lp[2] for lp in loops]})
            i = end
    return out, ev


def loop_envs(ev, headers):
    """The loop variables' values at each pass through `headers` (the loops
    around a call, outermost first): a list of {variable: value}."""
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
    return envs
