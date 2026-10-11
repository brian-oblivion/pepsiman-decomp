# SpawnEffectWithParams — MATCHED 48/48 (round 6, delta)

> Renamed from `func_80040130` on 2026-10-10 (tools/rename.py). Address 0x80040130.

Unit `src/stage_flow.c`. Takes a record off the free list of the 128-entry
`gEffects` table (head `gEffectFree`, -1 when empty; `gEffectActive` the
record last taken), links the previous one behind it and fills it from the
nine arguments. Returns -1 when the list is empty, 0 otherwise.

```c
/* Slot: the unit's 0x14-byte view of gEffects (s16 unk0..unk10, u8
 * unk12, u8 next) */
#define sSlots ((Slot *)gEffects)

s32 SpawnEffectWithParams(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5,
                  s16 arg6, s16 arg7, s16 arg8) {
    u8 old;

    if ((s8)gEffectFree < 0) {
        return -1;
    }
    old = gEffectActive;
    gEffectActive = gEffectFree;
    gEffectFree = sSlots[gEffectActive].next;
    sSlots[gEffectActive].next = old;
    sSlots[gEffectActive].unk12 = arg0;
    sSlots[gEffectActive].unkC = arg1;
    sSlots[gEffectActive].unkE = arg2;
    sSlots[gEffectActive].unk10 = arg3;
    sSlots[gEffectActive].unk4 = arg5;
    sSlots[gEffectActive].unk6 = arg6;
    sSlots[gEffectActive].unk8 = arg7;
    sSlots[gEffectActive].unkA = arg8;
    sSlots[gEffectActive].unk0 = 0;
    sSlots[gEffectActive].unk2 = arg4;
    return 0;
}
```

## Levers

- **The `lb`/`lbu` pair on `gEffectFree`**: one `(s8)` read for the test, one
  plain read for the value (LEARNINGS, SpawnEffect).
- **Every store through `sSlots[gEffectActive]`.** Retail reloads
  `gEffectActive` after each byte store into the table and not after the
  halfword ones; the plain array form reproduces exactly that. A local
  pointer taken after the `unk12` store gives the same code.
- **`arg0` is `s32`** although it is only stored with `sb`. As `u8` the
  build gave it `$t0` and moved `arg1` and the stack arguments up one
  temporary each (34/48: 14 register-only diffs). LEARNINGS' "a `u8`
  parameter only stored with `sb` may be `s32`" (SpawnEffect), again.
- Writing `unk12` last, after the halfwords, moves a reload and breaks the
  structure (worse).

The unit-local `Slot` duplicates stage's view of the same table;
the head may want one shared typedef in `common.h` in place of
`NumberedSlot`.
