#!/usr/bin/env python3
"""Pull the game's code overlays out of a PlayStation disc image into disk/ovl/.

    python3 tools/extract_overlays.py "Pepsiman (Japan) (Track 1).bin"
    python3 tools/extract_overlays.py disc.iso --list

LoadOverlay(id) (src/draw_tmd.c) reads one file into 0x800F0000: the disc
position in sOverlay<id>Loc[0] (a CdlLOC kept as a word) and its length in
sOverlay<id>Sectors[0]. Ids 0 and 2 come from main.c's mode switch, 3..17 from
LoadStageOverlay (one per stage), 18..22 from the world intros (world + 18).
Id 1 is never loaded. Each overlay is written to disk/ovl/ovl<id>.bin at its
ISO 9660 file length, and checked against check-overlays.sha1.

The disc paths below are what those CdlLOCs point at. They are not trusted:
every one is checked against the table in disk/SLPS_017.62, so a different
revision of the disc fails here instead of extracting the wrong files.
"""
import argparse
import hashlib
import struct
import sys
from pathlib import Path

import extract_exe

ROOT = Path(__file__).resolve().parent.parent
EXE = ROOT / "disk" / "SLPS_017.62"
OUT_DIR = ROOT / "disk" / "ovl"
SHA1_FILE = ROOT / "check-overlays.sha1"

# id: (disc path, vram of sOverlay<id>Loc). The Loc/Sectors names are in
# config/symbols.slps01762.pepsiman.txt; Sectors is Loc + 0x50 for 0..22 and
# Loc + 0x24 for the stages, whose tables also hold the stage's asset files.
OVERLAYS = {
    0: ("CDDATA/1/1000", 0x800761AC, 0x800761FC),
    2: ("CDDATA/1/1001", 0x800761B0, 0x80076200),
    3: ("CDDATA/2/2000", 0x8007624C, 0x80076270),
    4: ("CDDATA/3/3000", 0x80076294, 0x800762B8),
    5: ("CDDATA/4/4000", 0x800762DC, 0x80076300),
    6: ("CDDATA/5/5000", 0x80076324, 0x80076348),
    7: ("CDDATA/6/6000", 0x8007636C, 0x80076390),
    8: ("CDDATA/7/7000", 0x800763B4, 0x800763D8),
    9: ("CDDATA/8/8000", 0x800763FC, 0x80076420),
    10: ("CDDATA/9/9000", 0x80076444, 0x80076468),
    11: ("CDDATA/A/A000", 0x8007648C, 0x800764B0),
    12: ("CDDATA/B/B000", 0x800764D4, 0x800764F8),
    13: ("CDDATA/C/C000", 0x8007651C, 0x80076540),
    14: ("CDDATA/D/D000", 0x80076564, 0x80076588),
    15: ("CDDATA/E/E000", 0x800765AC, 0x800765D0),
    16: ("CDDATA/F/F000", 0x800765F4, 0x80076618),
    17: ("CDDATA/G/G000", 0x8007663C, 0x80076660),
    18: ("CDDATA/1/1100", 0x800761D4, 0x80076224),
    19: ("CDDATA/1/1200", 0x800761DC, 0x8007622C),
    20: ("CDDATA/1/1300", 0x800761E4, 0x80076234),
    21: ("CDDATA/1/1400", 0x800761EC, 0x8007623C),
    22: ("CDDATA/1/1500", 0x800761F4, 0x80076244),
}


def exe_word(exe, vram):
    off = vram - 0x80010000 + 0x800
    return struct.unpack("<I", exe[off:off + 4])[0]


def cdloc_to_lba(loc):
    """CdlLOC {minute, second, sector, track} in BCD -> logical block."""
    bcd = lambda b: (b >> 4) * 10 + (b & 0xF)
    m, s, f = bcd(loc & 0xFF), bcd(loc >> 8 & 0xFF), bcd(loc >> 16 & 0xFF)
    return (m * 60 + s) * 75 + f - 150


def walk(disc, lba, length, prefix, out):
    """Every file under a directory: {"DIR/NAME": (lba, size)}."""
    data = disc.read(lba, length)
    i = 0
    while i < len(data):
        rec_len = data[i]
        if rec_len == 0:
            i = (i // 2048 + 1) * 2048
            continue
        rec = data[i:i + rec_len]
        i += rec_len
        raw = rec[33:33 + rec[32]]
        if raw in (b"\0", b"\1"):  # "." and ".."
            continue
        name = raw.decode("ascii", "replace").split(";")[0].rstrip(".")
        rlba = struct.unpack("<I", rec[2:6])[0]
        size = struct.unpack("<I", rec[10:14])[0]
        if rec[25] & 2:
            walk(disc, rlba, size, prefix + name + "/", out)
        else:
            out[prefix + name] = (rlba, size)
    return out


def all_files(disc):
    pvd = disc.sector(16)
    root = pvd[156:156 + 34]
    return walk(disc, struct.unpack("<I", root[2:6])[0],
                struct.unpack("<I", root[10:14])[0], "", {})


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("image", help="disc image (.bin raw or .iso), data track")
    ap.add_argument("--list", action="store_true",
                    help="list every file on the disc and exit")
    args = ap.parse_args()

    disc, _ = extract_exe.open_disc(args.image)
    files = all_files(disc)
    if args.list:
        for name, (lba, size) in sorted(files.items(), key=lambda f: f[1][0]):
            print(f"  {name:<20} lba={lba:<8} {size:>10} bytes")
        return 0

    if not EXE.exists():
        sys.exit(f"{EXE.relative_to(ROOT)} not found; run tools/extract_exe.py first.")
    exe = EXE.read_bytes()

    expected = {}
    for line in SHA1_FILE.read_text().splitlines():
        digest, path = line.split()
        expected[Path(path).name] = digest

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    bad = 0
    for oid, (path, loc_vram, sectors_vram) in OVERLAYS.items():
        lba, size = files[path]
        want_lba = cdloc_to_lba(exe_word(exe, loc_vram))
        sectors = exe_word(exe, sectors_vram)
        if lba != want_lba or (size + 2047) // 2048 != sectors:
            print(f"ovl{oid:02}: {path} is at lba {lba} ({size} bytes), but the "
                  f"executable loads lba {want_lba} ({sectors} sectors)",
                  file=sys.stderr)
            bad += 1
            continue
        data = disc.read(lba, size)
        name = f"ovl{oid:02}.bin"
        (OUT_DIR / name).write_bytes(data)
        digest = hashlib.sha1(data).hexdigest()
        ok = expected.get(name) == digest
        bad += not ok
        print(f"ovl{oid:02}  {path:<14} {size:>6} bytes  "
              f"{'ok' if ok else 'SHA1 MISMATCH ' + digest}")
    if bad:
        print(f"\n{bad} overlay(s) do not match check-overlays.sha1.",
              file=sys.stderr)
        return 1
    print(f"wrote {len(OVERLAYS)} overlays to {OUT_DIR.relative_to(ROOT)}/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
