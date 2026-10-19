#!/usr/bin/env python3
"""Render every sound effect the game plays, from the sound banks on your disc.

    .venv/bin/python3 tools/sounds.py "Pepsiman (Japan) (Track 1).bin"
    .venv/bin/python3 tools/sounds.py disc.bin --only 0x35 --only 0x7c

Writes build/sounds/index.html, one WAV per effect and bank, and
build/sounds/sounds.json with what each one is made of. Nothing plays by
itself: the page's players load nothing until clicked. The sounds are the
game's: they stay in build/, which is gitignored, and are never committed.

PlaySe(id) (src/sound.c) plays SEQ `id` of the sound pack (boot file 4,
CDDATA/0/0004), a short MIDI-like sequence, on one of two open VAB banks:
bank 0 (boot file 6), open for good, for every id below 70, and from 70 on
whichever sSeVabBanks names, 0 or the swappable bank LoadVabBank last
loaded (boot file 6 + n for bank n). A stage loads its world's bank
(sStageVabBanks); an overlay names its own in its LoadVabBank calls. An
effect that plays from the swappable bank is rendered once per bank its
callers can have loaded.

The rendering is an approximation of the console: the SEQ's notes,
programs, pitch bends and volume controllers, played from the VAB's
ADPCM samples with each tone's key range, root note and ADSR envelope as
the SPU runs it, mixed to mono without reverb.
"""
import argparse
import html
import json
import re
import struct
import sys
import wave
from pathlib import Path

import extract_exe
import extract_overlays as eo
from csource import Unknown, calls, preprocess, split_lines
from sprites import disc_tables

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build" / "sounds"
RATE = 44100           # the SPU's rate: a pitch of 0x1000 plays a sample at this rate
OUT_RATE = 22050       # the WAVs': every other SPU sample, averaged
MAX_SECONDS = 6.0      # an effect still sounding after this is cut off
SE_BANK_FROM = 70      # PlaySe: ids below this always play from bank 0
ADPCM_FILTERS = ((0, 0), (60, 0), (115, -52), (98, -55), (122, -60))


# ---------------------------------------------------------------- the disc

def pack_entries(data, tag):
    """A pack's entries (offset, size), checked against their 4-byte tag."""
    count = struct.unpack_from("<H", data, 14)[0]
    out = []
    for n in range(count):
        off, size, t = struct.unpack_from("<II4s", data, n * 16)
        if t != tag:
            raise ValueError(f"entry {n} is {t!r}, not {tag!r}")
        out.append((off, size))
    return out


def decode_vag(data):
    """A VAG's samples (ADPCM, 28 to a 16-byte block), and its loop start
    in samples, or None when it does not loop."""
    out = []
    s1 = s2 = 0
    loop = None
    for i in range(0, len(data) - 15, 16):
        shift, filt = data[i] & 15, min(data[i] >> 4, 4)
        flags = data[i + 1]
        if flags & 4:
            loop = len(out)
        f0, f1 = ADPCM_FILTERS[filt]
        for b in data[i + 2:i + 16]:
            for nib in (b & 15, b >> 4):
                s = (nib << 12) - ((nib & 8) << 13)  # sign-extended to the top of 16 bits
                s = (s >> shift) + (s1 * f0 + s2 * f1 + 32) // 64
                s = max(-32768, min(32767, s))
                out.append(s)
                s2, s1 = s1, s
        if flags & 1:
            if not flags & 2:
                loop = None
            break
    return out, loop


class Vab:
    """A VAB bank: programs of tones, each tone a key range of a sample."""

    def __init__(self, data):
        hdr_off, hdr_size, _, _, body_off = struct.unpack_from("<5I", data, 0)
        vh = data[hdr_off:hdr_off + hdr_size]
        if vh[:4] != b"pBAV":
            raise ValueError("not a VAB header")
        self.ps, self.ts, self.vs = struct.unpack_from("<3H", vh, 18)
        self.mvol = vh[24]
        progs = [struct.unpack_from("<BB", vh, 32 + p * 16) for p in range(128)]
        self.prog_vol = {p: progs[p][1] for p in range(128)}
        tone_base = 32 + 128 * 16
        self.tones = {}  # program -> [tone]
        block = 0
        for p in range(128):
            n = progs[p][0]
            if n == 0:
                continue
            for k in range(n):
                t = struct.unpack_from("<16B2H2h", vh, tone_base + block * 512 + k * 32)
                self.tones.setdefault(p, []).append({
                    "vol": t[2], "center": t[4], "shift": t[5], "min": t[6], "max": t[7],
                    "pbmin": t[12], "pbmax": t[13], "adsr1": t[16], "adsr2": t[17], "vag": t[19]})
            block += 1
            if block >= self.ps:
                break
        sizes = struct.unpack_from("<256H", vh, tone_base + self.ps * 512)
        body = data[body_off:]
        self.vags = {}
        off = 0
        for v in range(1, self.vs + 1):
            size = sizes[v] * 8
            self.vags[v] = decode_vag(body[off:off + size])
            off += size


def parse_seq(data):
    """A SEQ's events in time order: (seconds, kind, channel, a, b)."""
    if data[:4] != b"pQES":
        raise ValueError("not a SEQ")
    res = struct.unpack_from(">H", data, 8)[0]
    tempo = int.from_bytes(data[10:13], "big")
    i = 15
    t = 0.0
    status = 0
    out = []

    def vlq():
        nonlocal i
        v = 0
        while True:
            b = data[i]
            i += 1
            v = v << 7 | (b & 0x7F)
            if not b & 0x80:
                return v

    while i < len(data):
        t += vlq() * tempo / 1e6 / res
        if i >= len(data):
            break
        if data[i] & 0x80:
            status = data[i]
            i += 1
        kind, ch = status >> 4, status & 15
        if status == 0xFF:
            meta, n = data[i], data[i + 1]
            body = data[i + 2:i + 2 + n]
            i += 2 + n
            if meta == 0x51:
                tempo = int.from_bytes(body, "big")
            elif meta == 0x2F:
                out.append((t, "end", 0, 0, 0))
                break
            continue
        if kind in (0x8, 0x9, 0xB, 0xE):
            a, b = data[i], data[i + 1]
            i += 2
        elif kind in (0xC, 0xD):
            a, b = data[i], 0
            i += 1
        else:
            raise ValueError(f"status {status:#x} at {i}")
        if kind == 0x9 and b == 0:
            kind = 0x8
        out.append((t, {0x8: "off", 0x9: "on", 0xB: "cc", 0xC: "prog", 0xD: "press", 0xE: "bend"}[kind],
                    ch, a, b))
    return out


# ---------------------------------------------------------------- the SPU

def rate_step(rate, decreasing, exponential, level):
    """One envelope tick of SPU rate `rate` (0..127): (cycles to wait, step)."""
    shift, st = rate >> 2, rate & 3
    step = (-8 + st) if decreasing else (7 - st)
    if shift < 11:
        step <<= 11 - shift
        cycles = 1
    else:
        cycles = 1 << (shift - 11)
    if exponential and not decreasing and level > 0x6000:
        cycles *= 4
    if exponential and decreasing:
        step = step * level >> 15
    return cycles, step


class Envelope:
    """The SPU's ADSR envelope of one voice, a tick per SPU sample."""

    def __init__(self, adsr1, adsr2):
        self.attack = (adsr1 >> 8) & 0x7F, bool(adsr1 >> 15)
        self.decay = ((adsr1 >> 4) & 15) << 2
        self.sustain_level = min(0x7FFF, ((adsr1 & 15) + 1) * 0x800)
        self.sustain = (adsr2 >> 6) & 0x7F, bool(adsr2 >> 15), bool(adsr2 & 0x4000)
        self.release = (adsr2 & 31) << 2, bool(adsr2 & 0x20)
        self.phase = "attack"
        self.level = 0
        self.wait = 0

    def key_off(self):
        self.phase = "release"
        self.wait = 0

    def tick(self):
        if self.wait > 0:
            self.wait -= 1
            return self.level
        if self.phase == "attack":
            rate, exp = self.attack
            cycles, step = rate_step(rate, False, exp, self.level)
            self.level = min(0x7FFF, self.level + step)
            if self.level >= 0x7FFF:
                self.phase = "decay"
        elif self.phase == "decay":
            cycles, step = rate_step(self.decay, True, True, self.level)
            self.level = max(0, self.level + step)
            if self.level <= self.sustain_level:
                self.phase = "sustain"
        elif self.phase == "sustain":
            rate, exp, dec = self.sustain
            cycles, step = rate_step(rate, dec, exp, self.level)
            self.level = max(0, min(0x7FFF, self.level + step))
        else:
            rate, exp = self.release
            cycles, step = rate_step(rate, True, exp, self.level)
            self.level = max(0, self.level + step)
        self.wait = cycles - 1
        return self.level


class Voice:
    """One tone of one note, sounding."""

    def __init__(self, tone, samples, loop, key, vel, gain, bend_semis):
        self.tone, self.samples, self.loop = tone, samples, loop
        self.key, self.gain = key, gain * vel / 127 * tone["vol"] / 127
        self.env = Envelope(tone["adsr1"], tone["adsr2"])
        self.pos = 0.0
        self.set_bend(bend_semis)
        self.done = False

    def set_bend(self, semis):
        notes = self.key - self.tone["center"] + self.tone["shift"] / 128 + semis
        self.step = 2 ** (notes / 12)

    def next(self):
        level = self.env.tick()
        n = len(self.samples)
        if self.pos >= n:
            if self.loop is None:
                self.done = True
                return 0
            self.pos = self.loop + (self.pos - self.loop) % max(1, n - self.loop)
        p = int(self.pos)
        q = p + 1 if p + 1 < n else (self.loop if self.loop is not None else p)
        frac = self.pos - p
        s = self.samples[p] + (self.samples[q] - self.samples[p]) * frac  # the SPU's is Gaussian
        self.pos += self.step
        if self.env.phase == "release" and level == 0:
            self.done = True
        return s * level / 0x7FFF * self.gain


def render(events, vab):
    """The mono samples (at OUT_RATE) of a SEQ played on `vab`, and what
    played: programs, samples, keys, whether it was cut off."""
    chan = {c: {"prog": 0, "vol": 127, "expr": 127, "bend": 0.0} for c in range(16)}
    voices = {}  # (channel, key) -> [Voice]
    sounding = []
    out = []
    used = {"programs": set(), "samples": set(), "keys": [], "missing": set()}
    t_end = min(events[-1][0] if events else 0, MAX_SECONDS) if events else 0
    n = 0
    ev = 0
    acc = 0.0
    limit = int(MAX_SECONDS * RATE)
    while n < limit:
        now = n / RATE
        while ev < len(events) and events[ev][0] <= now:
            t, kind, c, a, b = events[ev]
            ev += 1
            st = chan[c]
            if kind == "prog":
                st["prog"] = a
            elif kind == "cc" and a == 7:
                st["vol"] = b
            elif kind == "cc" and a == 11:
                st["expr"] = b
            elif kind == "bend":
                value = (b << 7 | a) - 8192
                st["bend"] = value / 8192
                for (vc, _), vs in voices.items():
                    if vc == c:
                        for v in vs:
                            span = v.tone["pbmax"] if value >= 0 else v.tone["pbmin"]
                            v.set_bend(st["bend"] * span)
            elif kind == "on":
                tones = [tn for tn in vab.tones.get(st["prog"], []) if tn["min"] <= a <= tn["max"]]
                if not tones:
                    used["missing"].add(st["prog"])
                used["programs"].add(st["prog"])
                used["keys"].append(a)
                gain = vab.mvol / 127 * vab.prog_vol.get(st["prog"], 127) / 127 * st["vol"] / 127 * st["expr"] / 127
                new = []
                for tn in tones:
                    if tn["vag"] not in vab.vags:
                        continue
                    smp, loop = vab.vags[tn["vag"]]
                    used["samples"].add(tn["vag"])
                    new.append(Voice(tn, smp, loop, a, b, gain, 0.0))
                voices.setdefault((c, a), []).extend(new)
                sounding.extend(new)
            elif kind == "off":
                for v in voices.pop((c, a), []):
                    v.env.key_off()
        if ev >= len(events) and not sounding:
            break
        mix = 0.0
        for v in sounding:
            mix += v.next()
        if any(v.done for v in sounding):
            sounding = [v for v in sounding if not v.done]
        acc += mix
        if n & 1:
            out.append(acc / 2)
            acc = 0.0
        n += 1
    used["cut"] = n >= limit
    used["seq_seconds"] = round(t_end, 3)
    return out, used


def write_wav(path, samples, peak=0.9):
    """A 16-bit mono WAV of `samples`, scaled to `peak` of full scale (the
    game's own level is kept in sounds.json)."""
    top = max((abs(x) for x in samples), default=0)
    k = peak * 32767 / top if top else 1
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(OUT_RATE)
        w.writeframes(b"".join(struct.pack("<h", int(x * k)) for x in samples))


# ---------------------------------------------------------------- the C

def int_table(path, name):
    """The initializer of a global integer array in a unit, as a list."""
    text = split_lines(preprocess(path))[0]
    m = re.search(r"\b" + re.escape(name) + r"\s*\[\s*\w*\s*\]\s*=\s*\{([^}]*)\}", text)
    if not m:
        raise SystemExit(f"{name} not found in {path}")
    return [int(v, 0) for v in m.group(1).replace("\n", " ").split(",") if v.strip()]


def unit_banks():
    """The swappable banks each unit's code runs with: the overlay's own
    LoadVabBank calls (an overlay and its _tail unit together); the
    executable's units run with any stage's."""
    stage_banks = sorted(set(int_table(ROOT / "src" / "stage_overlay.c", "sStageVabBanks")) - {0})
    by_ovl = {}
    for path in sorted(ROOT.glob("src/ovl/*.c")):
        ovl = path.stem.split("_")[0]
        found, ev = calls(path, "LoadVabBank")
        for c in found:
            try:
                by_ovl.setdefault(ovl, set()).add(ev.eval(c["args"][0], {}))
            except Unknown:
                pass  # the sound test's chosen bank
    return stage_banks, by_ovl


def play_sites():
    """Every PlaySe call with a constant id: {id: [site]}, and the names."""
    stage_banks, by_ovl = unit_banks()
    sites, names = {}, {}
    units = sorted(ROOT.glob("src/*.c")) + sorted(ROOT.glob("src/ovl/*.c"))
    for path in units:
        if path.name == "sound.c":
            continue
        found, ev = calls(path, "PlaySe")
        names.update({n: v for n, v in ev.names.items() if n.startswith("SE_")})
        ovl = path.stem.split("_")[0] if path.parent.name == "ovl" else None
        banks = sorted(by_ovl.get(ovl, set())) if ovl else stage_banks
        for c in found:
            try:
                sid = ev.eval(c["args"][0], {})
            except Unknown:
                continue  # the sound test's chosen id
            sites.setdefault(sid, []).append({
                "unit": path.stem, "where": f"{c['file']}:{c['line']}", "func": c["func"],
                "arg": c["args"][0], "banks": banks})
    return sites, {v: n for n, v in names.items()}


# ---------------------------------------------------------------- output

def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("image", help="disc image (.bin raw or .iso), data track")
    ap.add_argument("--out", type=Path, default=OUT)
    ap.add_argument("--only", action="append", default=[], help="an effect id (0x35, 53); repeatable")
    args = ap.parse_args()
    only = {int(o, 0) for o in args.only}

    disc, _ = extract_exe.open_disc(args.image)
    files = eo.all_files(disc)
    by_lba = {lba: name for name, (lba, _) in files.items()}
    boot = [by_lba[lba] for lba in disc_tables()["BootLoc"] if lba in by_lba]
    pack = disc.read(*files[boot[4]])
    seqs = pack_entries(pack, b"SEQ\0")
    vabs = {}

    def vab(bank):
        if bank not in vabs:
            vabs[bank] = Vab(disc.read(*files[boot[6 + bank]]))
        return vabs[bank]

    se_banks = int_table(ROOT / "src" / "sound.c", "sSeVabBanks")
    sites, names = play_sites()
    args.out.mkdir(parents=True, exist_ok=True)
    for old in args.out.glob("*.wav"):
        old.unlink()
    sounds = []
    for sid in sorted(sites):
        if only and sid not in only:
            continue
        swappable = sid >= SE_BANK_FROM and se_banks[sid - SE_BANK_FROM] == 1
        banks = sorted({b for s in sites[sid] for b in s["banks"]}) if swappable else [0]
        off, size = seqs[sid]
        events = parse_seq(pack[off:off + size])
        entry = {"id": sid, "hex": f"{sid:#04x}", "name": names.get(sid, ""), "sites": sites[sid],
                 "notes": sum(e[1] == "on" for e in events), "renders": []}
        for bank in banks:
            samples, used = render(events, vab(bank))
            fname = f"se_{sid:03x}_bank{bank}.wav"
            write_wav(args.out / fname, samples)
            peak = max((abs(s) for s in samples), default=0)
            entry["renders"].append({
                "bank": bank, "file": fname, "seconds": round(len(samples) / OUT_RATE, 3),
                "level": round(peak / 32768, 3), "programs": sorted(used["programs"]),
                "samples": sorted(used["samples"]), "keys": used["keys"],
                "missing_programs": sorted(used["missing"]), "cut_off": used["cut"]})
        sounds.append(entry)
        print(f"{sid:#04x} {entry['name'] or '-':<14} banks {banks} notes {entry['notes']} "
              f"{', '.join(str(r['seconds']) + 's' for r in entry['renders'])}")
    (args.out / "sounds.json").write_text(json.dumps(sounds, indent=1))
    write_index(args.out, sounds)
    index = (args.out / "index.html").resolve()
    print(f"wrote {index.relative_to(ROOT) if index.is_relative_to(ROOT) else index}")
    return 0


def write_index(out_dir, sounds):
    """build/sounds/index.html: a row per effect, a silent player per bank."""
    e = html.escape
    parts = ["<!doctype html><meta charset=utf-8><title>Pepsiman sounds</title><style>",
             "body{background:#24242e;color:#ddd;font:13px system-ui,sans-serif;margin:16px}",
             "table{border-collapse:collapse;width:100%}td,th{border-top:1px solid #3a3a48;",
             "padding:6px;vertical-align:top;text-align:left}th{color:#999;font-weight:500}",
             ".s{color:#aaa;font-size:12px}audio{height:28px;max-width:100%}</style>",
             "<h1>Pepsiman sounds</h1><p class=s>Rendered by tools/sounds.py from your own disc; "
             "approximate (no reverb). Not for the repository. Nothing plays until clicked.</p>",
             "<table><tr><th>id</th><th>name</th><th>sound</th><th>played at</th></tr>"]
    for s in sounds:
        players = "".join(
            f"<div>bank {r['bank']}: <audio controls preload=none src='{e(r['file'])}'></audio> "
            f"<span class=s>{r['seconds']}s, programs {r['programs']}, samples {r['samples']}"
            f"{', cut off' if r['cut_off'] else ''}</span></div>" for r in s["renders"])
        where = "<br>".join(f"{e(x['where'])} {e(x['func'])}" for x in s["sites"])
        parts.append(f"<tr><td>{s['hex']}</td><td>{e(s['name'])}</td><td>{players}</td>"
                     f"<td class=s>{where}</td></tr>")
    parts.append("</table>")
    (out_dir / "index.html").write_text("".join(parts))


if __name__ == "__main__":
    sys.exit(main())
