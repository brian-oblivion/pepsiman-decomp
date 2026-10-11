# ClearHitPoints — MATCHED 20/20 (round 2, charlie)

> Renamed from `func_80036E50` on 2026-10-11 (tools/rename.py). Address 0x80036e50.

Unit `src/hit_tool.c`. Clears the 0x44C0-byte buffer `gLivePoints` with
`bzero` and resets two halfwords (0x72 to 0, 0x74 to -1) in each of the 80
0x78-byte records of `gObjectModels` (`ObjectModel`, declared in the unit).

```c
void ClearHitPoints(void) {
    u32 i;

    bzero(gLivePoints, 0x44C0);
    for (i = 0; i < 80; i++) {
        gObjectModels[i].unk72 = 0;
        gObjectModels[i].unk74 = -1;
    }
}
```

## Externs

`bzero` comes from Sony's `memory.h`, now included by the unit.
`gLivePoints` and `gObjectModels` are reached `lui`/`addiu`, so they are arrays
of unknown size, declared in the unit (the other unit that reaches them,
`object`, still has them only in asm).

## Levers

None needed: first try. The `u32` counter (`sltiu`) and indexed
`gObjectModels[i]` (strength-reduced to a record pointer) are the shapes
`docs/LEARNINGS.md` already lists.
