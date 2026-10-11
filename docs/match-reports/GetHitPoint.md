# GetHitPoint — MATCHED 13/13 (round 2, charlie)

> Renamed from `func_80036A50` on 2026-10-11 (tools/rename.py). Address 0x80036a50.

Unit `src/hit_tool.c`. Returns a pointer to record `sub` of entry `idx` in
the bank `gHitPoints` points at: 80 eight-byte entries (`BankEntry`, the
first word the index of the entry's first record), then 0x2C-byte records
from byte 0x280 (`Bank2C`, `Rec2C`, declared in the unit).

```c
Rec2C *GetHitPoint(s32 idx, s32 sub) {
    Bank2C *bank = (Bank2C *)gHitPoints;
    Rec2C *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}
```

## Externs

`gHitPoints` is `extern s32` in `include/common.h` (2 units) and `%gp_rel`
in retail; the function casts it rather than retyping the shared
declaration.

## Levers

| C | result |
| --- | --- |
| byte arithmetic, `base + 0x280 + (((s32 *)base)[idx * 2] + sub) * 0x2C` | 9/13: `addu` operands swapped, `+0x280` folded into the last add |
| struct, `&bank->recs[bank->entries[idx].first + sub]` | 10/13: operands right, `+0x280` still last |
| **struct, records base in its own local `recs = bank->recs`** | **13/13** |

Retail computes `base + 0x280` into a register of its own before the
multiply; a separate local for the records' base is what reproduces it.

### Proposed learning

An `addiu base, K` early and kept apart from the index arithmetic, with the
final `addu` adding the scaled index to it: the source took the address of
an array member into its own local (`T *recs = p->recs;`) before indexing.
Indexing `p->recs[i]` directly folds `K` into the last add.
