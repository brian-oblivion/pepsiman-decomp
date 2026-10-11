# ResetTraps — MATCHED 14/14 (round 3, alpha)

> Renamed from `ResetTrapAll` on 2026-10-11 (tools/rename.py). Address 0x800338a0.

> Renamed from `ResetRec48All` on 2026-10-11 (tools/rename.py). Address 0x800338a0.

> Renamed from `func_800338A0` on 2026-10-11 (tools/rename.py). Address 0x800338a0.

Unit `src/editor.c`. Resets all 200 records of a `Trap` table: the
halfwords at 0x36, 0x38 and 0x34 to -1 and the byte at 0x41 to 1. First
build matched.

```c
void ResetTraps(Trap *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->unk36 = -1;
        recs->unk38 = -1;
        recs->unk34 = -1;
        recs->unk41 = 1;
        recs++;
    }
}
```

The shape of `ClearSwitches` (a `u32` counter, `sltiu`) and the walking
pointer of `ResetTrapUpper`; the stores come out in source order. `Trap`
gained `unk38` (s16), and, for the unattempted `ClearObjectHits`, `unk24`
(s16) and `unk40` (u8); the record size stays 0x48 and the whole image
stays OK.
