# AddBonusLevel — MATCHED 36/36 (round 3, delta)

> Renamed from `func_80028260` on 2026-10-11 (tools/rename.py). Address 0x80028260.

Unit `src/code_13068.c`. Adds `n` to the counter `gBonusLevel`, caps it at 2
when `D_800958AC` is 1 and at 3 otherwise, floors it at 0, zeroes it when
the byte `D_8009EF4D` is 1, and returns it.

```c
s32 AddBonusLevel(s32 n) {
    gBonusLevel += n;
    if (D_800958AC == 1) {
        if (gBonusLevel >= 2) {
            gBonusLevel = 2;
        }
    } else if (gBonusLevel >= 3) {
        gBonusLevel = 3;
    }
    if (gBonusLevel <= 0) {
        gBonusLevel = 0;
    }
    if (D_8009EF4D[0] == 1) {
        gBonusLevel = 0;
    }
    return gBonusLevel;
}
```

Matched on the first build.

## Declarations

- `gBonusLevel`, `D_800958AC`: every load is `lui $X` / `lw $X` (the `%hi` in
  the destination) and every store `lui $at`, so plain `s32` scalars,
  declared in the unit (no other unit in `src/` uses them yet; not in
  `config/gp-symbols.txt`).
- `D_8009EF4D`: `lui $v0` in a branch delay slot, then `lb $v1,
  %lo(D_8009EF4D)($v0)`: a cc1 split, so an array of unknown size; `lb` makes
  it `s8`. Declared `extern s8 D_8009EF4D[];` in the unit. It is likely a
  byte inside the record common.h declares as `D_8009EF48[]` (`u8`), the
  same way `D_8009EF4A` is split out; no retype proposed.
