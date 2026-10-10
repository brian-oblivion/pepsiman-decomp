# PlayMovie — MATCHED 52/52 (round 10, delta)

> Renamed from `func_80041BAC` on 2026-10-10 (tools/rename.py). Address 0x80041bac.

Unit `src/code_31cec.c`. Movie start-up: records the stream length
(`gMovieLastFrame`) and slice height (`gMovieDisplayHeight`), copies the start position
into the unit's `CdlLOC` (`gMovieLoc`, minute/second/sector), clears a
480x480 VRAM area through the unit's `RECT` (`gMovieClearRect`), sets the CD
mix (no cross-feed when the stereo flag `D_80095AEE` is set) and hands
the file name to PlayMovieFile.

```c
void PlayMovieFile(char *name, s16 arg1);

extern CdlLOC gMovieLoc;
extern s32 gMovieLastFrame;
extern RECT gMovieClearRect;
extern s16 gMovieDisplayHeight;

void PlayMovie(char *name, CdlLOC *loc, s32 arg2, s16 arg3, s16 arg4) {
    CdlATV atv;

    gMovieLastFrame = arg2;
    gMovieDisplayHeight = arg3;
    gMovieLoc.minute = loc->minute;
    gMovieLoc.second = loc->second;
    gMovieLoc.sector = loc->sector;
    setRECT(&gMovieClearRect, 0, 0, 480, 480);
    ClearImage(&gMovieClearRect, 0, 0, 0);
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
    PlayMovieFile(name, arg4);
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
- Declarations from retail: `gMovieLoc` and `gMovieClearRect` are stored
  `%gp_rel` and passed `lui`/`addiu`, so complete objects of size at most 8
  (`CdlLOC`, `RECT`).

### Proposed learning

- **A global reached `lui` in one function of a unit and `$gp` in
  another: read the `lui` one through its literal address**, `*(u8
  *)0x80095AEE`, with a `MATCHING:` note. cc1 splits a constant address
  `%hi`/`%lo` and maspsx leaves it; no unit split is needed for a few
  accesses. (PlayMovie)
