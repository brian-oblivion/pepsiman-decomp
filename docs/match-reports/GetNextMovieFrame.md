# GetNextMovieFrame — MATCHED 80/80 (round 10, delta)

> Renamed from `func_80041A6C` on 2026-10-10 (tools/rename.py). Address 0x80041a6c.

Unit `src/sound.c`. Sony's sample `strNext` on the movie player's
globals: polls `StGetNext` for the next frame with a timeout counter
(`gMovieTimeout`, volatile), returns NULL on timeout. Records the frame
number (`gMovieFrameNo`), raises the end flag `gMovieEnded` once the frame
number reaches the movie length `gMovieLastFrame`, clears VRAM when the frame
size changes (`gMovieWidth`/`gMovieHeight` hold the last width and height),
then sizes both frame areas (width * 3 / 2, 24-bit) and the slice height.
Returns the frame address `gMovieAddr`.

```c
extern volatile s32 gMovieTimeout; /* MATCHING note in the unit */
extern s32 gMovieEnded;
extern u_long gMovieFrameNo;
extern StHEADER *gMovieHeader;
extern u_long *gMovieAddr;
extern s32 gMovieLastFrame;
extern RECT gMovieClearRect;
extern s32 gMovieWidth;
extern s32 gMovieHeight;

u_long *GetNextMovieFrame(DECENV *dec) {
    gMovieTimeout = 0x800000;
    while (StGetNext(&gMovieAddr, (u_long **)&gMovieHeader)) {
        if (--gMovieTimeout == 0) {
            return NULL;
        }
    }
    gMovieFrameNo = gMovieHeader->frameCount;
    if (gMovieHeader->frameCount >= gMovieLastFrame) {
        gMovieEnded = 1;
    }
    if (gMovieWidth != gMovieHeader->width || gMovieHeight != gMovieHeader->height) {
        setRECT(&gMovieClearRect, 0, 0, 480, 480);
        ClearImage(&gMovieClearRect, 0, 0, 0);
        DrawSync(0);
        gMovieWidth = gMovieHeader->width;
        gMovieHeight = gMovieHeader->height;
    }
    dec->rect[0].w = dec->rect[1].w = gMovieWidth * 3 / 2;
    dec->slice.h = dec->rect[0].h = dec->rect[1].h = gMovieHeight;
    return gMovieAddr;
}
```

## Levers

| step | result |
| --- | --- |
| `if ((gMovieFrameNo = gMovieHeader->frameCount) >= gMovieLastFrame)` | 79 words: the compare done in place on the loaded value, sector pointer in `$a0` |
| `gMovieFrameNo = ...; if (gMovieFrameNo >= gMovieLastFrame)` | the same |
| **`gMovieFrameNo = gMovieHeader->frameCount; if (gMovieHeader->frameCount >= gMovieLastFrame)`** | **80/80** |

- **A loaded value copied (`move v1, v0`) and compared in the copy while
  the original is stored: the source read the member twice**, once for
  the store and once for the test; CSE merges the loads but keeps two
  pseudos. Comparing the stored global (or an assignment expression)
  compares in place.
- The two-headed chained stores (`rect[1].w`, `rect[0].w`, then
  `rect[1].h`, `rect[0].h`, `slice.h`) are right-to-left chained
  assignment, as LEARNINGS says.
- `StGetNext` and `StHEADER` come from `libcd.h` (the struct is under
  `#ifndef _LIBDS_H_`); no local typedef or prototype is needed. The
  `width`/`height` reads are `lhu` against `lw` of the s32 globals.

### Proposed learning

- **`lw x; move y, x; sltu y, y, z` with `x` stored afterwards: the
  member was read twice, `G = p->f; if (p->f >= L)`.** Testing `G` or
  `(G = p->f) >= L` compares in place. (GetNextMovieFrame)
