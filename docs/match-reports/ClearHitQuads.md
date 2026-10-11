# ClearHitQuads — MATCHED 20/20 (round 2, charlie)

> Renamed from `func_80036EA0` on 2026-10-11 (tools/rename.py). Address 0x80036ea0.

Unit `src/hit_tool.c`. Clears the 0x1DB0-byte buffer `gLiveQuads` with
`bzero` and resets two halfwords (0x6E to 0, 0x70 to -1) in each of the 80
0x78-byte records of `gObjectModels` (`ObjectModel`, declared in the unit).

```c
void ClearHitQuads(void) {
    u32 i;

    bzero(gLiveQuads, 0x1DB0);
    for (i = 0; i < 80; i++) {
        gObjectModels[i].unk6E = 0;
        gObjectModels[i].unk70 = -1;
    }
}
```

## Externs

`bzero` comes from Sony's `memory.h`, included by the unit.
`gLiveQuads` and `gObjectModels` are reached `lui`/`addiu`, so they are arrays
of unknown size, declared in the unit (the other unit that reaches them,
`object`, still has them only in asm).

## Levers

None needed: first try, as the twin of `ClearHitPoints`. The `u32` counter (`sltiu`) and indexed
`gObjectModels[i]` (strength-reduced to a record pointer) are the shapes
`docs/LEARNINGS.md` already lists.
