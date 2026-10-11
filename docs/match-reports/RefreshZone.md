# RefreshZone — MATCHED 17/17 (round 3, alpha)

> Renamed from `func_8002C438` on 2026-10-11 (tools/rename.py). Address 0x8002c438.

Unit `src/code_1a098.c`. Calls the lookup `FindFloorNear` with the object,
the table `sFloorHit` and the object's word at 0x28, and stores the result
back unless it is -1. First build matched.

```c
void RefreshZone(Obj2C *p) {
    s32 v;

    v = FindFloorNear(p, sFloorHit, p->unk28);
    if (v != -1) {
        p->unk28 = v;
    }
}
```

`sFloorHit` is reached `lui $a1` / `addiu $a1,$a1,%lo` straight into the
argument register; `extern u8 sFloorHit[];` (unknown size, cc1's split)
gives exactly that here, because the address is wanted in `$a1` itself.
`Obj2C` and the prototype of `FindFloorNear` (defined in `model`) are
declared in the unit.
