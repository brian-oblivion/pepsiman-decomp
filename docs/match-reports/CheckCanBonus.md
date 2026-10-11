# CheckCanBonus — MATCHED 44/44 (round 3, delta)

> Renamed from `func_80028448` on 2026-10-11 (tools/rename.py). Address 0x80028448.

Unit `src/code_13068.c`. Every tenth tick of the halfword counter
`gCans` (non-zero and a multiple of 10) it bumps the counter
`gBonusLevel` by one through `AddBonusLevel(1)`; before that it plays
`PlaySe(34)` unless the counter already sits at its cap (2 when
`D_800958AC` is 1, else 3) or the byte `D_8009EF4D` is 1.

```c
/* MATCHING: `one` is the 1 retail keeps in $a0 for both compares. */
void CheckCanBonus(void) {
    s32 one;

    if (gCans != 0 && gCans % 10 == 0) {
        one = 1;
        if (D_800958AC == one) {
            if (gBonusLevel == 2) {
                goto skip;
            }
        } else if (gBonusLevel == 3) {
            goto skip;
        }
        if (D_8009EF4D[0] != one) {
            PlaySe(34);
        }
    skip:
        AddBonusLevel(1);
    }
}
```

## Shared header (additive)

`extern u16 gCans;` moved into `include/common.h`'s small-object
block (one line added) and removed from `src/code_1a098.c` (one line), as
`tools/declcheck.py` requires once two units use it. Every retail access in
all units is `lhu`/`sh` with a `lui` in the destination or `$at`: a `u16`
scalar. Whole image OK.

## Levers

- **Retail's per-arm reload of `gBonusLevel` with one shared `beq`** comes
  from two compares, one per arm of the `D_800958AC` test, that jump to the
  same place: cross-jumping merges the `beq`s. A ternary (`gBonusLevel !=
  (D_800958AC == 1 ? 2 : 3)`, 17/44) and an if/else `max` temp (19/44)
  both load `gBonusLevel` once. The `goto` form gave 36/44.
- **One register for the constant 1 across a join.** Retail loads `li
  a0,1` once and compares both `D_800958AC` and the `D_8009EF4D` byte
  with `$a0`, though the second compare's block has two predecessors.
  With literal `1`s the build rematerialises it (`li v0,1` twice; 36/44,
  37/44 as `s32`). A local `s32 one = 1;` used in both compares: 44/44,
  as `void` (no non-void trick needed). The `1` passed to `AddBonusLevel`
  stays a literal; retail reloads it in the call's delay slot.
- Negatives: the ternary inside the `&&` with `one` (23/44, void or s32).

### Proposed learning

A constant held in one register across compares in different basic blocks
(the second block a join) is a local variable in the source; literal
constants are rematerialised per block on 2.8.1.
