# InitMovieDecoder — MATCHED 39/39 (round 4, echo)

> Renamed from `func_80041C7C` on 2026-10-10 (tools/rename.py). Address 0x80041c7c.

Unit `src/sound.c`. Initialises the movie decode state (Sony movie
sample's `strInit` part): the two VLC buffers at 0x8015D000 and
0x80171000, the image buffer at 0x80195000, indices and done flag 0, two
480x240 VRAM frame areas, and the first 24-pixel-wide decode slice,
offset by `gMovieOffsetX` across and centred vertically for a movie
`gMovieDisplayHeight` lines tall. Called from PlayMovieFile.

```c
extern s16 gMovieOffsetX;
extern s16 gMovieDisplayHeight;
/* DECENV: see DecodeNextMovieFrame.md */

void InitMovieDecoder(DECENV *dec, s32 x0, s32 y0, s32 x1, s32 y1) {
    dec->vlcbuf[0] = (u_long *)0x8015D000;
    dec->vlcbuf[1] = (u_long *)0x80171000;
    dec->vlcid = 0;
    dec->imgbuf = (u_short *)0x80195000;
    dec->rectid = 0;
    dec->isdone = 0;
    setRECT(&dec->rect[0], x0, y0, 480, 240);
    setRECT(&dec->rect[1], x1, y1, 480, 240);
    setRECT(&dec->slice, x0 + gMovieOffsetX, y0 + (240 - gMovieDisplayHeight) / 2, 24, gMovieDisplayHeight);
}
```

## Levers

- First build matched once the `DECENV` layout from DecodeNextMovieFrame was
  in place; libgpu's `setRECT` gives the x, y, w, h store order.
- `gMovieDisplayHeight` is read `lh` for the arithmetic and `lhu` for the plain
  halfword store: `s16`. `gMovieOffsetX` is only ever `lhu`, so its sign is
  not measured; `s16` chosen to match its neighbour.
