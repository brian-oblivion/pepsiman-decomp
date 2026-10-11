# InitView — MATCHED (20/20), round 3, runner charlie

> Renamed from `func_80018CB4` on 2026-10-10 (tools/rename.py). Address 0x80018cb4.

Unit `src/model.c`. Sets the projection distance to 250 and a
reference view at (0, 0, 4000) looking at the origin, then applies it with
`GsSetRefView2`.

```c
extern s32 gRefView[];   /* common.h */

void InitView(void) {
    GsSetProjection(250);
    gRefView[0] = 0;
    gRefView[1] = 0;
    gRefView[2] = 4000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
    gRefView[6] = 0;
    gRefView[7] = 0;
    GsSetRefView2((GsRVIEW2 *)gRefView);
}
```

## Attempts (1)

1. As above: 20/20 first build. `gRefView` is already `s32[]` in
   `common.h` (15 functions in 5 units reach it); indexing it gives retail's
   `lui $v0` / `addiu $a0` base with the first store at `%lo(sym)($v0)` and
   the rest at `off($a0)`. It is a `GsRVIEW2` (vpx..vrz, rz, super); the
   cast keeps the shared declaration unchanged.
