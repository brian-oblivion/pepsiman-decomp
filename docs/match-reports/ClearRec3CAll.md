# ClearRec3CAll — MATCHED 9/9 (round 1, alpha)

> Renamed from `func_8003390C` on 2026-10-11 (tools/rename.py). Address 0x8003390c.

Unit `src/code_1dc24.c`. Sets the signed halfword at 0 of all 100 records of
a table of 0x3C-byte records (`Rec3C`, declared in the unit) to -1. The
halfword sibling of `ClearRec5CAll`; first build matched.

```c
void ClearRec3CAll(Rec3C *recs) {
    u32 i;

    for (i = 0; i < 100; i++) {
        recs->unk0 = -1;
        recs++;
    }
}
```

Levers: the u32 counter and the signed field, as in `ClearRec5CAll`.
