# UpdateGridTest — MATCHED 313/313 (round 10, bravo)

> Renamed from `func_80022A74` on 2026-10-11 (tools/rename.py). Address 0x80022a74.

Unit `src/player.c`. The debug grid view: pad bits nudge
`D_800959A8` and the first two words of `gRefView`. The function
projects the 21-by-16 grid `D_800DE5E0` (InitGridTest fills it) through
a coordinate system per point, keeps each point's screen position in
`D_800DE0A0` and its depth in the grid point, and draws a white dot
there. It then fills the 20-by-15 cells with a checkerboard of flat
quads, white and grey, sorted by depth, and copies a few values out for
display.

```c
extern DVECTOR D_800DE0A0[];
extern s32 D_800DED50[];
extern s32 D_800957F8;
extern s32 D_80095800;
extern s32 D_80095804;
extern s32 D_8009580C;

/* MATCHING: the parity mask through y, the first loop's local, keeps 0xFF out of the preheader. */
void UpdateGridTest(void) {
    DVECTOR sxy;
    SVECTOR v;
    MATRIX ls;
    GsCOORDINATE2 coord;
    GsLINE line;
    POLY_F4 poly;
    long p;
    long flag;
    s32 x;
    s32 y;
    POLY_F4 *pp;

    if (gPadTrig & 0x10) {
        D_800959A8 = 0;
    }
    if (gPad & 4) {
        D_800959A8 += 2;
    }
    if (gPad & 8) {
        D_800959A8 -= 2;
    }
    if (gPadTrig & 0x800) {
        gGameMode = 3;
    }
    if (gPad & 0x8000) {
        gRefView[0]++;
    }
    if (gPad & 0x2000) {
        gRefView[0]--;
    }
    if (gPad & 0x1000) {
        gRefView[1]++;
    }
    if (gPad & 0x4000) {
        gRefView[1]--;
    }
    line.attribute = 0;
    line.b = line.g = line.r = 0xFF;
    for (gLoopJ = 0; gLoopJ < 16; gLoopJ++) {
        for (gLoopI = 0; gLoopI < 21; gLoopI++) {
            x = D_800DE5E0[gLoopJ * 21 + gLoopI].x;
            y = D_800DE5E0[gLoopJ * 21 + gLoopI].y;
            GsInitCoordinate2(WORLD, &coord);
            coord.coord.t[2] = 0;
            coord.flg = 0;
            coord.coord.t[0] = x;
            coord.coord.t[1] = y;
            GsGetLs(&coord, &ls);
            GsSetLsMatrix(&ls);
            v.vx = v.vy = v.vz = 0;
            D_800DE5E0[gLoopJ * 21 + gLoopI].z = RotTransPers(&v, (long *)&sxy, &p, &flag);
            D_800DE0A0[gLoopJ * 21 + gLoopI].vx = line.x0 = line.x1 = sxy.vx;
            D_800DE0A0[gLoopJ * 21 + gLoopI].vy = line.y0 = line.y1 = sxy.vy;
            GsSortLine(&line, &gOt[gActiveBuff], 0);
        }
    }
    pp = &poly;
    for (gLoopJ = 0; gLoopJ < 15; gLoopJ++) {
        for (gLoopI = 0; gLoopI < 20; gLoopI++) {
            SetPolyF4(pp);
            pp->x0 = D_800DE0A0[gLoopJ * 21 + gLoopI].vx;
            pp->y0 = D_800DE0A0[gLoopJ * 21 + gLoopI].vy;
            pp->x1 = D_800DE0A0[gLoopJ * 21 + (gLoopI + 1)].vx;
            pp->y1 = D_800DE0A0[gLoopJ * 21 + (gLoopI + 1)].vy;
            pp->x3 = D_800DE0A0[(gLoopJ + 1) * 21 + (gLoopI + 1)].vx;
            pp->y3 = D_800DE0A0[(gLoopJ + 1) * 21 + (gLoopI + 1)].vy;
            pp->x2 = D_800DE0A0[(gLoopJ + 1) * 21 + gLoopI].vx;
            pp->y2 = D_800DE0A0[(gLoopJ + 1) * 21 + gLoopI].vy;
            y = 1;
            if (gLoopJ & y) {
                if (gLoopI & y) {
                    pp->r0 = 0x70;
                    pp->g0 = 0x70;
                    pp->b0 = 0x80;
                } else {
                    pp->r0 = 0xFF;
                    pp->g0 = 0xFF;
                    pp->b0 = 0xFF;
                }
            } else if (gLoopI & 1) {
                pp->r0 = 0xFF;
                pp->g0 = 0xFF;
                pp->b0 = 0xFF;
            } else {
                pp->r0 = 0x70;
                pp->g0 = 0x70;
                pp->b0 = 0x80;
            }
            GsSortPoly(pp, &gOt[gActiveBuff], D_800DE5E0[gLoopJ * 21 + gLoopI].z >> 6);
        }
    }
    D_800957F8 = D_800959A8;
    D_80095800 = D_800DED50[0];
    D_80095804 = D_800DE0A0[0].vx;
    D_8009580C = D_800DE0A0[0].vy;
}
```

## Levers

Builds: 182 -> 190 -> 190 -> 189 -> 190 -> 181 -> 307 (index order),
then 307 on four colour/branch reshapes, then the permuter, then 311 ->
313.

- Locals in declaration order give retail's frame (sxy 0x10, v 0x18, ls
  0x20, coord 0x40, line 0x90, poly 0xA0, p 0xB8, flag 0xBC). The first
  loop matched at once with chained stores (`line.b = line.g = line.r`,
  `D_800DE0A0[i].vx = line.x0 = line.x1 = sxy.vx`: stores right to left).
- `POLY_F4 *pp = &poly;` for the second loop (`$s0`), as SortQuad15.
- **The column+1 index needs `r * 21 + (c + 1)`.** It gives retail's
  `r*21`, `+ 1`, `+ c` in that operand order. `r * 21 + 1 + c` folds
  into `r*21 + (c+1)` with the operands swapped; `r * 21 + c + 1` shares
  `r*21 + c` with the first index; `c + 1 + r * 21` swaps the last
  `addu`; `(&D[r*21 + 1])[c]` scales twice.
- **The last word: where loop.c hoists the 0xFF.** Retail keeps `li s1,
  0xFF` in the outer loop's body (beside `&D_800DE5E0`) and moves `li
  s2, 0x70` out of both loops. Every plain spelling moved both out (307).
  `setRGB0`, an if/else-if arm order, one white and one grey block
  through `goto`, and a ternary condition all scored the same or worse
  (single blocks stop the hoisting altogether). A permuter run
  (37652 iterations, killed after the in-tree check; best 20 from base
  80, `--stack-diffs`, Gate 3 checks 1-3 run: scaffold reordering 1,
  in-tree insertions 1 / deletions 1 for the same `li`) found `if
  (gLoopJ & (y = 1)) { if (gLoopI & y)`. In-tree that matched
  the hoisting (311). The statement form `y = 1;` before the `if`
  matches too. A fresh local in place of `y` (`bit`) does not (309): the
  reuse of the first loop's local is what changes loop.c's budget.

### Proposed learning

A loop invariant (a `li` of a colour) that retail hoists one loop level
less than the build does: route a mask through a local the function
already uses elsewhere (`y = 1; if (r & y) ...`). An extra set of a
long-lived pseudo in the inner loop changes loop.c's move budget; a new
local does not. A permuter find; it may be what the source did, or
something with the same effect. (UpdateGridTest)
