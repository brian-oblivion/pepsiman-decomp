# InitGridTest — MATCHED 51/51 (round 5, delta)

> Renamed from `func_800229A8` on 2026-10-11 (tools/rename.py). Address 0x800229a8.

Unit `src/player.c`. Lays out a 21-by-16 grid of points 16 pixels
apart centred on a 320x240 screen (x from -160, y from -120, z 0) into
`sGridPoints`, row by row, using two globals as the loop counters
(`gLoopJ` rows, `gLoopI` columns). Then resets the first six words
of `gRefView` (the view record model hands to `GsSetRefView2`) with
1000 in the third, and clears `sGridAngle`.

```c
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} GridPoint;

extern s32 gLoopJ;
extern s32 gLoopI;      /* moved to common.h */
extern s32 sGridAngle;
extern GridPoint sGridPoints[];
extern s32 gRefView[];    /* common.h */

void InitGridTest(void) {
    for (gLoopJ = 0; gLoopJ < 16; gLoopJ++) {
        for (gLoopI = 0; gLoopI < 21; gLoopI++) {
            sGridPoints[gLoopJ * 21 + gLoopI].x = gLoopI * 16 - 160;
            sGridPoints[gLoopJ * 21 + gLoopI].y = gLoopJ * 16 - 120;
            sGridPoints[gLoopJ * 21 + gLoopI].z = 0;
        }
    }
    gRefView[0] = 0;
    sGridAngle = 0;
    gRefView[1] = 0;
    gRefView[2] = 1000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
}
```

## Levers

- **A flat index, not a 2-D array.** As `GridPoint sGridPoints[][21]`,
  cc1 scales the row by 252 (`sll 6; subu; sll 2`) and adds the column
  separately, 8/51 and one word short. Retail computes `row * 21` once per
  row (`sll 2; addu; sll 2; addu`), adds the column and scales the sum by
  12: the source indexed a flat table with `row * 21 + col`. 51/51.
- Store order of the tail is source order, as the round-3 learning says;
  `sGridAngle` (gp) sits between the first two `gRefView` stores.

## Declarations

- `gLoopI` was a local extern in `src/main.c` (SetPadMainMode also uses
  it as a loop counter). Now two units use it, so it moved to
  `include/common.h` (additive line) and the local extern in `main.c` was
  deleted (main.c is not one of this round's units).

### Proposed learning

A row-major table walked by two counters whose row offset is built as
`row * N` once per row and then scaled together with the column by the
element size was indexed flat, `tbl[row * N + col]`; a 2-D array scales
the row by `N * size` instead.
