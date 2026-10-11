# ClearStageSprites — MATCHED 9/9 (round 1, alpha)

> Renamed from `ClearStageSpriteAll` on 2026-10-11 (tools/rename.py). Address 0x8003390c.

> Renamed from `ClearRec3CAll` on 2026-10-11 (tools/rename.py). Address 0x8003390c.

> Renamed from `func_8003390C` on 2026-10-11 (tools/rename.py). Address 0x8003390c.

Unit `src/editor.c`. Sets the signed halfword at 0 of all 100 records of
a table of 0x3C-byte records (`StageSprite`, declared in the unit) to -1. The
halfword sibling of `ClearSwitches`; first build matched.

```c
void ClearStageSprites(StageSprite *recs) {
    u32 i;

    for (i = 0; i < 100; i++) {
        recs->unk0 = -1;
        recs++;
    }
}
```

Levers: the u32 counter and the signed field, as in `ClearSwitches`.
