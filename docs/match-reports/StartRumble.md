# StartRumble — MATCHED (29/29) after dropping maspsx's --nop-at-expansion (round 4, delta)

> Renamed from `func_80015450` on 2026-10-10 (tools/rename.py). Address 0x80015450.

**Resolved by the head after round 4:** the Makefile no longer passes
`--nop-at-expansion` to maspsx. The whole image verified unchanged without it,
and the body below is live and matches 29/29. Original title: StartRumble — STALL: 4 words long, 17/29, first diff at word 16 (a `nop` after `lbu v0,5(a0)`) — round 4, runner delta

Unit `src/main.c`. Points `gRumblePattern` at a table of 10-byte `RumbleStep`
records, steps to record `index`, stores `index` in `gRumbleIndex`, and
unpacks the record's fields into six halfword globals. `ResetRumble` is
the same thing for record 0 of `gRumblePatterns`.

**The C is right; the pinned maspsx flag `--nop-at-expansion` blocks it.**
Compiled through the Makefile's pipeline with that one flag dropped, the
body below is byte-exact (29/29, relocations masked). With the flag, maspsx
puts a `nop` between each of the four byte/halfword loads and the `lui $at`
of the store-to-symbol that uses the loaded register, four words retail
does not have. The function is parked under `NON_MATCHING`.

```c
#if 0
typedef struct {
    u16 unk0;
    u16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u16 unk8;
} RumbleStep;

extern RumbleStep *gRumblePattern;
extern u16 gRumbleIndex;
extern u16 gRumbleWait;
extern u16 gRumbleTime;
extern u16 gRumbleSmall;
extern u16 gRumbleLarge;
extern u16 gRumbleNext;
extern u16 gRumbleRepeat;

void StartRumble(RumbleStep *tbl, u16 index) {
    u8 *rec;

    gRumblePattern = tbl;
    tbl += index;
    gRumbleIndex = index;
    gRumbleWait = tbl->unk0;
    gRumbleTime = tbl->unk2;
    rec = (u8 *)tbl;
    gRumbleSmall = rec[5];
    gRumbleLarge = rec[4];
    gRumbleNext = *(u16 *)(rec + 8);
    gRumbleRepeat = rec[6];
}
#endif
```

## How the shape was found

- `tbl += index;` after the store of `tbl`: retail overwrites `$a0` with
  the record address (`addu a0,a0,v0`) once `gRumblePattern` is stored. A
  separate `hdr = &tbl[index]` keeps both and puts the sum in `$v0`.
- All-member reads (`tbl->unk5` ...) let cc1 reorder: it hoists `lhu 8`
  above the store of `gRumbleLarge` and swaps two stores. A member read from
  a varying address and a store to a fixed scalar are treated as
  non-conflicting (2.8.1 has the "fixed scalar vs varying struct" alias
  rule). Retail keeps every load of fields 4, 5, 8 and 6 directly below
  the previous store, so those were not member reads. Reading every field
  through a byte pointer pins all of them, including the first two, which
  retail does hoist above the store of `gRumbleIndex`. Members for `unk0` and
  `unk2`, a byte pointer for the rest, gives retail's order exactly.

## The toolchain evidence

Posted to the broadcast; not acted on (CLAUDE.md hard rule 6).

- Retail, all of `asm/`: 15 sites of `load $r` / `lui $at` / `s{b,h,w} $r,
  %lo(sym)($at)` with no `nop` between (this function x4, `UpdateRumble`
  x5, `code_29f54` `LoadStageData` x2 and `LoadAct3Data` x2, `code_1a098`
  `func_8002B220` and `func_80030DA0`), and 0 sites of the same with the
  `nop`. The flag's comment in `tools/maspsx/maspsx.py` counts 40 sites
  with the nop, which is lsddecomp's retail, not this one.
- Every current C unit run through maspsx with and without the flag: the
  only difference anywhere is this function's four nops. Dropping the flag
  would not move any matched code, as far as the C that exists today goes.
- Reproducer: cpp | cc1 (Makefile flags) | maspsx with `MASPSX_FLAGS` minus
  `--nop-at-expansion` | as, then compare `StartRumble` to retail with
  relocated fields masked: 29/29.

`gRumblePatterns` was not retyped: this function takes the table as a
parameter, so the retype the brief allowed is not needed for it.

### Proposed learning

- A record unpacked into globals with each load sitting right under the
  previous store: the loads were not member accesses. 2.8.1 hoists a
  member read from a pointer over stores to fixed scalars; a read through
  a byte pointer (`*(u16 *)(rec + 8)`, `rec[5]`) stays put.
- `--nop-at-expansion` contradicts Pepsiman's retail (15 sites without the
  nop, 0 with), pending the head's whole-image check.
