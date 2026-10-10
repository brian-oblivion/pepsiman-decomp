# func_80041BAC — MATCHED 52/52 (round 10, delta)

Unit `src/code_31cec.c`. Movie start-up: records the stream length
(`D_80095AD8`) and slice height (`D_80095AC8`), copies the start position
into the unit's `CdlLOC` (`D_80095AB0`, minute/second/sector), clears a
480x480 VRAM area through the unit's `RECT` (`D_80095AE0`), sets the CD
mix (no cross-feed when the stereo flag `D_80095AEE` is set) and hands
the file name to func_80041534.

```c
void func_80041534(char *name, s16 arg1);

extern CdlLOC D_80095AB0;
extern s32 D_80095AD8;
extern RECT D_80095AE0;
extern s16 D_80095AC8;

void func_80041BAC(char *name, CdlLOC *loc, s32 arg2, s16 arg3, s16 arg4) {
    CdlATV atv;

    D_80095AD8 = arg2;
    D_80095AC8 = arg3;
    D_80095AB0.minute = loc->minute;
    D_80095AB0.second = loc->second;
    D_80095AB0.sector = loc->sector;
    setRECT(&D_80095AE0, 0, 0, 480, 480);
    ClearImage(&D_80095AE0, 0, 0, 0);
    DrawSync(0);
    /* MATCHING: D_80095AEE by its address (see below). */
    if (*(u8 *)0x80095AEE) {
        atv.val2 = 80;
        atv.val0 = 80;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = 80;
        atv.val0 = 80;
        atv.val3 = 80;
        atv.val1 = 80;
    }
    CdMix(&atv);
    func_80041534(name, arg4);
}
```

## Levers

| step | result |
| --- | --- |
| first write, `if (D_80095AEE)` (common.h's `extern u8`) | 51 words, the flag read `lbu %gp_rel` where retail has `lui $v0` / `lbu $v0, %lo` (one word short, everything after shifted) |
| **`if (*(u8 *)0x80095AEE)`** | **52/52, `OK: build matches retail`** |

- Retail reaches `D_80095AEE` through `lui` here but through `$gp` in
  func_80042968 and func_80042B80, later in the same unit: a source-file
  seam (the original file declared the flag without a size, or the unit
  holds two files). maspsx applies one gp list per unit, so the symbol
  cannot be both. A constant address has no decl for cc1 to call small
  data: cc1 splits it `%hi`/`%lo` itself and maspsx leaves it alone. The
  bytes are identical; only the relocation's symbol differs, and nothing
  checks that. If the head later splits the unit at the seam, this can go
  back to `D_80095AEE`.
- The rest came out on the first write: the field-by-field `CdlLOC` copy
  interleaves exactly as retail, the CdMix block is func_80042968's body
  with 80 (stores `val2, val0, val3, val1` in source order), and the
  stack fifth argument read `lhu` then re-extended for the call is an
  `s16` parameter.
- Declarations from retail: `D_80095AB0` and `D_80095AE0` are stored
  `%gp_rel` and passed `lui`/`addiu`, so complete objects of size at most 8
  (`CdlLOC`, `RECT`).

### Proposed learning

- **A global reached `lui` in one function of a unit and `$gp` in
  another: read the `lui` one through its literal address**, `*(u8
  *)0x80095AEE`, with a `MATCHING:` note. cc1 splits a constant address
  `%hi`/`%lo` and maspsx leaves it; no unit split is needed for a few
  accesses. (func_80041BAC)
