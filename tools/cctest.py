#!/usr/bin/env python3
"""Score candidate compilers against retail, function by function.

Each unit is compiled with -DNON_MATCHING through the project's own pipeline
(cpp, maspsx and as from the Makefile) with only cc1, -G and the maspsx
--aspsx-version swapped, and each chosen function is compared word for word
with the retail executable, relocated fields masked. Nothing in build/ is
touched: objects go to build/cctest/. This is how the compiler was chosen
(GCC 2.8.1, -G8, aspsx 2.56: see the commit that switched to it); rerun it
when a near-miss looks like the toolchain rather than the C.

    python3 tools/cctest.py --fetch 2.6.3 2.7.2 2.8.0   # into tools/cc-candidates/
    python3 tools/cctest.py                             # pinned + every candidate
    python3 tools/cctest.py --cc pinned 2.8.0 -G -G4 --aspsx 2.79 code_7d74:func_80018D04

`pinned` is the Makefile's cc1. A score is `same/retail words`; `*` marks a
compiled length that differs from retail, which usually means the C, not the
compiler, is the wrong shape. Scores only mean something for C that was
written as a match attempt: a body tuned on one compiler can lose a word or
two on another (func_800382DC needed `u16 sum` on 2.8.1).
"""
import argparse, os, re, struct, subprocess, sys, tarfile, tempfile, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CANDIDATES = ROOT / 'tools/cc-candidates'
OUT = ROOT / 'build/cctest'
EXE = ROOT / 'disk/SLPS_017.62'
OLD_GCC = 'https://github.com/decompals/old-gcc/releases/download/0.17/gcc-{}-psx.tar.gz'

# The probes the switch was decided on: three plain functions, then a branchy
# zero-return, a compiler-split %hi/%lo loop, a jump table and a div.
DEFAULT_TARGETS = [
    'code_27bc8:func_800382DC', 'main:func_80015328', 'code_31cec:func_80042958',
    'code_7d74:func_80019874', 'code_308ec:func_800400EC',
    'code_31958:func_8004121C', 'code_7d74:func_80018D04',
]


def makefile_vars():
    """The Makefile's simple `:=`/`+=` assignments, $(VAR) expanded."""
    v = {}
    for line in (ROOT / 'Makefile').read_text().splitlines():
        m = re.match(r'^([A-Z0-9_]+)\s*(:=|\+=)\s*(.*?)\s*$', line)
        if not m:
            continue
        val = re.sub(r'\$\((\w+)\)', lambda x: v.get(x.group(1), ''), m.group(3))
        v[m.group(1)] = (v.get(m.group(1), '') + ' ' + val).strip() if m.group(2) == '+=' else val
    return v


def fetch(version):
    dest = CANDIDATES / version
    if (dest / 'cc1').exists():
        print(f'{version}: already in {dest.relative_to(ROOT)}')
        return
    with tempfile.TemporaryDirectory() as tmp:
        tgz = Path(tmp) / 'gcc.tar.gz'
        urllib.request.urlretrieve(OLD_GCC.format(version), tgz)
        dest.mkdir(parents=True, exist_ok=True)
        with tarfile.open(tgz) as t:
            t.extractall(dest, filter='data')
    for f in dest.iterdir():
        f.chmod(0o755)
    print(f'{version}: fetched into {dest.relative_to(ROOT)}')


def retail_words(mk, fn):
    """Retail's words for fn, its length from the linked image's next text
    symbol (asm/ drops a function's .s once it is C, so not from there)."""
    elf = ROOT / 'build/pepsiman.elf'
    if not elf.exists():
        sys.exit('no build/pepsiman.elf -- run ./build-and-verify.sh first')
    nm = subprocess.run([ROOT / mk['NM'], '-n', elf], capture_output=True, text=True).stdout
    text = [int(a, 16) for a, t in re.findall(r'^([0-9a-f]{8}) ([Tt]) ', nm, re.M)]
    start = int(fn[5:], 16)
    end = min(a for a in text if a > start)
    off = start - 0x80010000 + 0x800
    return struct.unpack_from(f'<{(end - start) // 4}I', EXE.read_bytes(), off)


def compile_unit(mk, cc1, unit, gflag, aspsx, obj):
    ccf = re.sub(r'-G\d+', gflag, mk['CC_FLAGS'])
    mxf = re.sub(r'--aspsx-version=\S+', f'--aspsx-version={aspsx}', mk['MASPSX_FLAGS'])
    cmd = (f"{mk['CPP']} {mk['CPP_FLAGS']} -DNON_MATCHING src/{unit}.c | {cc1} {ccf} | "
           f"{mk['MASPSX']} {mxf} | {mk['AS']} {mk['AS_FLAGS']} -o {obj}")
    r = subprocess.run(cmd, shell=True, cwd=ROOT, capture_output=True, text=True)
    return r.returncode == 0 and obj.exists(), r.stderr.strip()[-300:]


def built_words(mk, obj, fn):
    """(words, {index: mask}) for fn in obj, or (None, None) if absent."""
    d = subprocess.run([ROOT / mk['OBJDUMP'], '-dr', obj], capture_output=True, text=True).stdout
    m = re.search(rf'^[0-9a-f]+ <{fn}>:\n(.*?)(?=\n\n|\Z)', d, re.M | re.S)
    if not m:
        return None, None
    words, masks, base = [], {}, None
    for line in m.group(1).splitlines():
        a = re.match(r'^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s', line)
        if a:
            base = int(a.group(1), 16) if base is None else base
            words.append(int(a.group(2), 16))
            continue
        r = re.match(r'^\s*([0-9a-f]+):\s+R_MIPS_(\w+)', line)
        if r:
            masks[(int(r.group(1), 16) - base) // 4] = 0x03FFFFFF if r.group(2) == '26' else 0xFFFF
    return words, masks


def score(retail, words, masks):
    same = sum((words[i] & ~masks.get(i, 0)) == (retail[i] & ~masks.get(i, 0))
               for i in range(min(len(retail), len(words))))
    return f"{same}/{len(retail)}{'' if len(words) == len(retail) else '*'}"


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('targets', nargs='*', metavar='unit:func', help='default: the switch probes')
    ap.add_argument('--fetch', nargs='+', metavar='VER', help='download candidates and exit')
    ap.add_argument('--cc', nargs='+', metavar='VER', help='"pinned" and/or candidate versions')
    ap.add_argument('-G', dest='gflag', help='replaces the Makefile -G (e.g. -G4)')
    ap.add_argument('--aspsx', help='replaces the Makefile --aspsx-version')
    args = ap.parse_args()
    if args.fetch:
        for v in args.fetch:
            fetch(v)
        return

    mk = makefile_vars()
    gflag = args.gflag or re.search(r'-G\d+', mk['CC_FLAGS']).group(0)
    aspsx = args.aspsx or re.search(r'--aspsx-version=(\S+)', mk['MASPSX_FLAGS']).group(1)
    found = sorted(p.name for p in CANDIDATES.glob('*') if (p / 'cc1').exists()) if CANDIDATES.exists() else []
    vers = args.cc or ['pinned'] + found
    cc1 = {v: (ROOT / mk['CC1'] if v == 'pinned' else CANDIDATES / v / 'cc1') for v in vers}
    for v, p in cc1.items():
        if not p.exists():
            sys.exit(f'{v}: no {p.relative_to(ROOT)} -- run tools/cctest.py --fetch {v}')
    targets = [t.split(':') for t in (args.targets or DEFAULT_TARGETS)]

    OUT.mkdir(parents=True, exist_ok=True)
    print(f'{gflag}, aspsx {aspsx}')
    print(f'{"function":16}' + ''.join(f'{v:>10}' for v in vers))
    for unit, fn in targets:
        retail = retail_words(mk, fn)
        cells = []
        for v in vers:
            obj = OUT / f'{unit}.{v}{gflag}.{aspsx}.o'
            ok, err = compile_unit(mk, cc1[v], unit, gflag, aspsx, obj)
            if not ok:
                print(f'# {unit} on {v}: {err}', file=sys.stderr)
                cells.append('ERR')
                continue
            words, masks = built_words(mk, obj, fn)
            cells.append('absent' if words is None else score(retail, words, masks))
        print(f'{fn:16}' + ''.join(f'{c:>10}' for c in cells))
    print('(* = compiled length differs from retail)')


if __name__ == '__main__':
    main()
