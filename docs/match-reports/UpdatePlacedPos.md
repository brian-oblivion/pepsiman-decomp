# UpdatePlacedPos — MATCHED 38/38 (round 6, alpha)

> Renamed from `func_80029898` on 2026-10-11 (tools/rename.py). Address 0x80029898.

Unit `src/code_1a098.c`. Converts a record's local position (words at
0x1C/0x20/0x24, in the coordinate system at 0x28) to world space with
`GsGetLws` / `GsSetLsMatrix` / `RotTrans` and stores it in the record's
first three words, then restores the light-source matrix. Its caller
`UpdateObjectEntry` walks a table of 0x2C-byte records (`gLivePoints`), hence
the new unit-local view `Placed2C`.

```c
void UpdatePlacedPos(Placed2C *p) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = p->lx;
    v.vy = p->ly;
    v.vz = p->lz;
    GsGetLws(p->coord, &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    p->pos.vx = t.vx;
    p->pos.vy = t.vy;
    p->pos.vz = t.vz;
    GsSetLsMatrix(&world);
}
```

First build. The body is code_13068's TransformByCoord with the record as
source and destination, including its `s32 unused[2]` (flag at sp+0x70,
frame 0x88). The word fields copied into the SVECTOR come out as `lhu`
loads, so s32 members are right. The unit now includes `libgpu.h` and
`libgs.h`. `tools/typeviews.py --warnings` reports one NEW warning,
`unused variable 'unused'`, the known cost of that lever (to baseline).
