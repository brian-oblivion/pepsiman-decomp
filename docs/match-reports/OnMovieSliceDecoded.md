# OnMovieSliceDecoded — MATCHED 66/66 (round 10, delta)

> Renamed from `func_80041964` on 2026-10-10 (tools/rename.py). Address 0x80041964.

Unit `src/sound.c`. The `DecDCToutCallback` handler of the movie
player (PlayMovieFile installs it through InitMovieStream): Sony's sample
`out_callback` on the unit's decode environment `gMovieDec` (a
`DECENV`). It services a pending stream interrupt (`D_800E1F1C`, libcd's
`StCdIntrFlag`), sends the decoded slice to VRAM, steps the slice right,
and either asks for the next slice or, at the frame's right edge, marks
the frame done, flips the frame area and rewinds the slice to that
area's corner, offset by the movie's left margin (`gMovieOffsetX`) and
vertically centred for its height (`gMovieDisplayHeight`).

```c
extern s32 D_800E1F1C;
extern DECENV gMovieDec;
extern s16 gMovieOffsetX;
extern s16 gMovieDisplayHeight;

void OnMovieSliceDecoded(void) {
    if (D_800E1F1C) {
        StCdInterrupt();
        D_800E1F1C = 0;
    }
    LoadImage(&gMovieDec.slice, (u_long *)gMovieDec.imgbuf);
    gMovieDec.slice.x += gMovieDec.slice.w;
    if (gMovieDec.slice.x <
        gMovieDec.rect[gMovieDec.rectid].x + gMovieDec.rect[gMovieDec.rectid].w) {
        DecDCTout((u_long *)gMovieDec.imgbuf, gMovieDec.slice.w * gMovieDec.slice.h / 2);
    } else {
        gMovieDec.isdone = 1;
        gMovieDec.rectid = gMovieDec.rectid == 0;
        gMovieDec.slice.x = gMovieDec.rect[gMovieDec.rectid].x + gMovieOffsetX;
        gMovieDec.slice.y = gMovieDec.rect[gMovieDec.rectid].y + (240 - gMovieDisplayHeight) / 2;
    }
}
```

## Levers

| step | result |
| --- | --- |
| Sony's `out_callback` transcribed onto the unit's `DECENV`, with the two offsets InitMovieDecoder applies | **66/66 on the first build** |

- Retail builds `&gMovieDec.slice` (splat's `D_800E04D4`) and derives the
  struct base from it (`addiu $s0, $a0, -0x24`): cc1's CSE of the first
  member address against the base, not a separate pointer.
- `gMovieDec` is reached `lui`/`addiu` (48 bytes), `D_800E1F1C`
  `lui $v0` / `lw $v0` (a plain scalar). `StCdInterrupt` comes from
  `libcd.h`; a local prototype fails declcheck.
- The `rectid` store lands before the `isdone` store although `isdone` is
  written first; the scheduler did that, no reordering in the source.
