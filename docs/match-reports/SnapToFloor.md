# SnapToFloor — MATCHED 39/39 (round 6, alpha)

> Renamed from `func_8002C0EC` on 2026-10-11 (tools/rename.py). Address 0x8002c0ec.

Unit `src/object.c`. RefreshZone with more work on success: calls
the lookup `FindFloorNear(p, D_80096738, p->unk28)`; unless it returns -1,
stores the result in `unk28`, copies the first halfword the lookup wrote
into both y words of the object (current `unk4`, stored `unk10`), and sets
`unk2C`/`unk30` to `ratan2` angles of the next three halfwords. Returns the
lookup's result.

```c
extern s16 D_80096738[];

s32 SnapToFloor(Obj34 *p) {
    s32 v;

    v = FindFloorNear(p, D_80096738, p->unk28);
    if (v != -1) {
        p->unk28 = v;
        p->unk4 = p->unk10 = D_80096738[0];
        p->unk2C = ratan2(-D_80096738[3], D_80096738[2]);
        p->unk30 = ratan2(-D_80096738[1], D_80096738[2]);
    }
    return v;
}
```

First build. The record's fields at 0x4, 0x10, 0x2C and 0x30 are those of
the unit's existing `Obj34` (current/stored position, the two words a reset
zeroes), so `Obj34`'s `u8 unk24[8]` was split into `unk24[4]` and
`s32 unk28`. `D_80096738`, declared `u8[]` in this unit only, became
`s16[]` (retail reads it with `lh` at 0, 2, 4, 6). The store at 0x10
before 0x4 is the chained assignment (stores right to left).
