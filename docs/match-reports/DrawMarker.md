# DrawMarker — MATCHED 52/52 (round 4, bravo)

> Renamed from `func_8002964C` on 2026-10-10 (tools/rename.py). Address 0x8002964c.

Unit `src/collision.c`. Sets the local-screen matrix to a world
coordinate at `pos`, then draws sprite 0x15D through DrawBillboard at
twice `scale`, in grey (0, 0x80, 0x80, 0x80), into the current OT
`&gSubOt[gActiveBuff]`.

```c
extern GsOT gSubOt[];
extern s32 gActiveBuff;
void DrawBillboard(s32 id, SVECTOR *size, CVECTOR *color, s32 mode, GsOT *ot);

void DrawMarker(VECTOR *pos, u16 scale) {
    SVECTOR size;
    CVECTOR color;
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = pos->vx;
    coord.coord.t[1] = pos->vy;
    coord.coord.t[2] = pos->vz;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
    size.vx = size.vy = scale * 2;
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    DrawBillboard(0x15D, &size, &color, 2, &gSubOt[gActiveBuff]);
}
```

First try. Notes for the next unit that calls DrawBillboard (many in
stage do, with the same frame):

- The frame is `SVECTOR` at 0x18, `CVECTOR` at 0x20, `GsCOORDINATE2` at
  0x28, `MATRIX` at 0x78, in declaration order above the 5th-argument slot.
- Chained assignments store right to left: `size.vx = size.vy = v` gives
  `sh 0x1A` then `sh 0x18`, and `color.g = color.b = color.cd` gives
  `0x23, 0x22, 0x21`, as retail.
- `gActiveBuff` is a scalar (`lui $v1` / `lw $v1, %lo($v1)`, same register,
  adjacent); `gSubOt` is the `GsOT` array (20-byte stride). Both are
  declared in this unit only; `DrawBillboard`'s prototype (draw,
  still asm) is this unit's view: `void`, five arguments.
