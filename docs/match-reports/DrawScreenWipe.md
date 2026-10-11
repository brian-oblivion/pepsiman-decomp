# DrawScreenWipe — MATCHED (107/107), round 8, runner charlie

> Renamed from `func_8003C2E8` on 2026-10-10 (tools/rename.py). Address 0x8003c2e8.

Unit `src/stage.c`. While `gWipeActive` is set, adds one full-screen
`POLY_FT4` (320x240, centred) to the current ordering table each frame,
stepping through eight 128x96 texture cells (frame counter `gWipeFrame / 3`:
column `n % 2`, row `n / 2 % 2`, texture page `n / 4`, CLUT row `448 + n`).
At frame 24 it clears the flag. An eight-cell full-screen animation.

```c
void DrawScreenWipe(void) {
    POLY_FT4 *p;
    s32 n;

    if (gWipeActive != 0) {
        if (gWipeFrame >= 24) {
            gWipeActive = 0;
            return;
        }
        n = gWipeFrame / 3;
        p = (POLY_FT4 *)GsOUT_PACKET_P;
        setPolyFT4(p);
        p->r0 = p->g0 = p->b0 = 0x80;
        p->x0 = p->x2 = -160;
        p->x1 = p->x3 = 160;
        p->y0 = p->y1 = -120;
        p->y2 = p->y3 = 120;
        p->u0 = p->u2 = (n % 2) << 7;
        p->v0 = p->v1 = (n / 2 % 2) * 96;
        p->clut = getClut(720, n + 448);
        p->u1 = p->u3 = p->u0 + 0x7F;
        p->v2 = p->v3 = p->v0 + 0x5F;
        p->tpage = getTPage(0, 0, n / 4 * 64 + 704, 256);
        addPrim(gSubOt[gActiveBuff].org, p);
        gWipeFrame++;
        p++;
        GsOUT_PACKET_P = (u8 *)p;
    }
}```

## Levers

- Matched on the first build. Spelled with libgpu's macros (`setPolyFT4`,
  `getClut`, `getTPage`, `addPrim`) and chained assignments for the paired
  fields: retail stores each pair second-member first (`x2` before `x0`,
  `b0`/`g0`/`r0`), which is `a = b = v` storing right to left.
- The counter's increment sits between `addPrim` and the primitive-buffer
  bump in source; cc1 schedules it inside the `addPrim` sequence.
