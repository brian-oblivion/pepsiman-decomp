# PlayMovieFile — MATCHED 268/268 (round 10, delta)

> Renamed from `func_80041534` on 2026-10-10 (tools/rename.py). Address 0x80041534.

Unit `src/sound.c`. The movie player's main loop, Sony's sample
`play` on the unit's globals. Looks the file up when a name is given
(`CdSearchFile` into `gMovieFile`, position copied to `gMovieLoc`), sets
up the decode environment (InitMovieDecoder), the stream (InitMovieStream,
callback OnMovieSliceDecoded) and the first frame (DecodeNextMovieFrame), switches the
display to 24-bit, then decodes frame after frame, flipping the display
to the finished frame area each time. It stops at the movie's end
(`gMovieEnded`) or, when `skip` is 1, on the pad's start bit
(`gPadTrig & 0x800`, returning 1): mutes the CD, pauses it, clears the
frame, copies the last frame into place, unhooks the decoder and the
ring, and restores a 16-bit display.

```c
void ReadPad(s32 arg);
void OnMovieSliceDecoded(void);
void InitMovieDecoder(DECENV *dec, s32 x0, s32 y0, s32 x1, s32 y1);
void InitMovieStream(CdlLOC *loc, void (*callback)());
s32 DecodeNextMovieFrame(DECENV *dec);
void WaitMovieFrameDecoded(DECENV *dec, s32 mode);

extern CdlLOC gMovieLoc;
extern s32 gMovieLastFrame;
extern RECT gMovieClearRect;
extern DECENV gMovieDec;
extern CdlFILE gMovieFile;
extern DRAWENV gMovieDrawEnv;
extern DISPENV gMovieDispEnv;

s32 PlayMovieFile(char *name, s16 skip) {
    CdlATV atv;
    s16 one;
    s32 id;
    s32 ret;

    ret = 0;
    if (*(s8 *)name != 0) {
        while (CdSearchFile(&gMovieFile, name) == 0) {
        }
        gMovieLoc.minute = gMovieFile.pos.minute;
        gMovieLoc.second = gMovieFile.pos.second;
        gMovieLoc.sector = gMovieFile.pos.sector;
    }
    InitMovieDecoder(&gMovieDec, 0, 0, 0, 240);
    InitMovieStream(&gMovieLoc, OnMovieSliceDecoded);
    DecodeNextMovieFrame(&gMovieDec);
    SetDefDispEnv(&gMovieDispEnv, 0, 0, 0, 0);
    SetDefDrawEnv(&gMovieDrawEnv, 0, 0, 0, 0);
    one = 1;
    gMovieDispEnv.disp.w = gMovieDispEnv.disp.w * 2 / 3;
    gMovieDispEnv.isrgb24 = one;
    PutDispEnv(&gMovieDispEnv);
    PutDrawEnv(&gMovieDrawEnv);
    SetDispMask(0);
    while (1) {
        DecDCTin(gMovieDec.vlcbuf[gMovieDec.vlcid], 3);
        DecDCTout((u_long *)gMovieDec.imgbuf, gMovieDec.slice.w * gMovieDec.slice.h / 2);
        DecodeNextMovieFrame(&gMovieDec);
        WaitMovieFrameDecoded(&gMovieDec, 0);
        VSync(0);
        id = gMovieDec.rectid == 0;
        SetDefDispEnv(&gMovieDispEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y,
                      gMovieDec.rect[id].w, gMovieDec.rect[id].h);
        SetDefDrawEnv(&gMovieDrawEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y,
                      gMovieDec.rect[id].w, gMovieDec.rect[id].h);
        gMovieDispEnv.disp.w = gMovieDispEnv.disp.w * 2 / 3;
        gMovieDispEnv.isrgb24 = 1;
        PutDispEnv(&gMovieDispEnv);
        PutDrawEnv(&gMovieDrawEnv);
        SetDispMask(1);
        if (gMovieEnded == one) {
            atv.val3 = 0;
            atv.val2 = 0;
            atv.val1 = 0;
            atv.val0 = 0;
            CdMix(&atv);
            VSync(0);
            while (CdControl(CdlPause, 0, 0) == 0) {
            }
            setRECT(&gMovieClearRect, 0, 0, 480, 480);
            ClearImage(&gMovieClearRect, 0, 0, 0);
            DrawSync(0);
            break;
        }
        ReadPad(0);
        if ((gPadTrig & 0x800) && skip == one) {
            atv.val3 = 0;
            atv.val2 = 0;
            atv.val1 = 0;
            atv.val0 = 0;
            CdMix(&atv);
            VSync(0);
            while (CdControl(CdlPause, 0, 0) == 0) {
            }
            setRECT(&gMovieClearRect, 0, 0, 480, 480);
            ClearImage(&gMovieClearRect, 0, 0, 0);
            DrawSync(0);
            ret = 1;
            break;
        }
    }
    MoveImage(&gMovieDec.rect[gMovieDec.rectid], gMovieDec.rect[id].x, gMovieDec.rect[id].y);
    DrawSync(0);
    DecDCToutCallback(0);
    StUnSetRing();
    while (CdControlB(CdlPause, 0, 0) == 0) {
    }
    ChangeClearPAD(0);
    /* MATCHING: retail keeps this load and drops the store. */
    gMovieDispEnv.disp.w = gMovieDispEnv.disp.w;
    gMovieDispEnv.isrgb24 = 0;
    SetDefDispEnv(&gMovieDispEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y, gMovieDec.rect[id].w,
                  gMovieDec.rect[id].h);
    SetDefDrawEnv(&gMovieDrawEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y, gMovieDec.rect[id].w,
                  gMovieDec.rect[id].h);
    return ret;
}
```

## Levers

| step | result |
| --- | --- |
| first write, `if ((s8)name[0] != 0)`, `s32 one = 1` stored as `isrgb24 = one` | 267 words: `lbu` for retail's `lb`, `isrgb24` stored straight from the saved register, `$s4`/`$s5` swapped, one load missing at the end |
| `*(s8 *)name` | the `lb` |
| **`one` declared `s16`** (or `u8`) | `li s5, 1; move v1, s5; sb v1`, and `$s4`/`$s5` as retail |
| **`gMovieDispEnv.disp.w = gMovieDispEnv.disp.w;` before `isrgb24 = 0`** | **268/268** |

- **`lb` on a `char *` argument: `*(s8 *)name`.** `(s8)name[0] != 0`
  folds to `lbu`, as the LEARNINGS entry on globals says.
- **A constant held in a saved register and copied (`move v1, sN`) before
  a byte store: the local is narrower than `s32`.** Truncating it to
  `u_char` goes through a fresh pseudo; an `s32` local is stored
  directly. The narrow type also flips which of `&gMovieDrawEnv` and `one`
  gets `$s4`. Companion of LEARNINGS' "a constant rebuilt ... narrower
  than s32".
- **A dead `lhu` of a field just before a store to its neighbour: a
  self-assignment**, `x.f = x.f;`. 2.8.1 drops the store and keeps the
  load. Removing it costs 38 words (230/268). Noted `MATCHING:` in the
  source.
- `WaitMovieFrameDecoded` is called with a second argument 0 (Sony's
  `strSync(&dec, 0)`); its definition gains an unused `s32 mode` and
  stays 48/48.
- `CdControlB`, `StUnSetRing`, `ChangeClearPAD` (`libapi.h`, now
  included) come from Sony's headers. The prototype of main's
  `ReadPad` is in this `.c` (declcheck MULTI with main.c; it
  belongs in a shared header, left for the head).

### Proposed learning

- **A dead load of a field kept in the output: a self-assignment `x.f =
  x.f;`.** (PlayMovieFile)
- **A constant from a saved register copied to a scratch register before
  a byte store: the local holding it is `s16`/`u8`.** (PlayMovieFile)
