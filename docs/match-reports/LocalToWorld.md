# LocalToWorld — MATCHED 42/42 (round 6, alpha)

> Renamed from `func_80029930` on 2026-10-11 (tools/rename.py). Address 0x80029930.

Unit `src/object.c`. Converts a local position (three words) to world
space in the coordinate system the byte after them indexes in the
`GsCOORDINATE2` table `gTodCoords` (stride 0x50 = sizeof(GsCOORDINATE2)),
writing the result to `out`. Unlike UpdatePlacedPos it does not restore the
light-source matrix afterwards.

```c
extern GsCOORDINATE2 gTodCoords[];

void LocalToWorld(LocalPos *lp, VECTOR *out) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = lp->x;
    v.vy = lp->y;
    v.vz = lp->z;
    GsGetLws(&gTodCoords[lp->coord], &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    out->vx = t.vx;
    out->vy = t.vy;
    out->vz = t.vz;
}
```

First build, from UpdatePlacedPos's shape. `LocalPos` is a new unit-local
view (`s8` index: retail loads it `lb`). The table is reached `lui`/`addiu`
then indexed, so `[]`. One more NEW `unused variable 'unused'` warning in
`tools/typeviews.py --warnings`, to baseline.
