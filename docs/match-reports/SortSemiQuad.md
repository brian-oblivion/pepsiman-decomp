# SortSemiQuad — MATCHED (72/72), round 7, runner alpha

> Renamed from `func_800179F8` on 2026-10-10 (tools/rename.py). Address 0x800179f8.

Unit `src/code_7d74.c`. Draws a semi-transparent flat quad: a stack
POLY_F4 set from a 15-bit colour and four corners, sorted into
`gSubOt[gActiveBuff]`. It is code_13068's SortQuad15 plus one
`SetSemiTrans(p, 1)`, including that function's corner order (the
arguments named x2/y2 land in x3/y3 and vice versa) and its red-channel
lever.

```c
/* code_a0bc.h: extern GsOT gSubOt[]; common.h: extern s32 gActiveBuff; */

void SortSemiQuad(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri) {
    POLY_F4 poly;
    POLY_F4 *p;
    s32 r;

    p = &poly;
    SetPolyF4(p);
    SetSemiTrans(p, 1);
    r = col & 0x1F;
    p->r0 = r << 3;
    p->g0 = ((col >> 5) & 0x1F) << 3;
    p->b0 = ((col >> 10) & 0x1F) << 3;
    p->x0 = x0;
    p->y0 = y0;
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x3;
    p->y2 = y3;
    p->x3 = x2;
    p->y3 = y2;
    GsSortPoly(p, &gSubOt[gActiveBuff], pri);
}
```

The unit now includes `code_a0bc.h` for `gSubOt`.

## Attempts (2)

1. SortQuad15's body plus `SetSemiTrans`, the red channel inline:
   64/72, only the prologue's argument copies out of order (`col` copied
   last instead of first).
2. `r = col & 0x1F;` as its own statement (SortQuad15's lever): 72/72.

### Proposed learning

- Confirms SortQuad15's lever on a second function: naming the first
  use of a parameter (`r = col & 0x1F;`) moves its prologue copy first.
  (SortSemiQuad)
