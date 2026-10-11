# PrintToolSlots — MATCHED 38/38 (round 4, charlie)

> Renamed from `func_80037280` on 2026-10-11 (tools/rename.py). Address 0x80037280.

Unit `src/hit_tool.c`. Prints a three-line debug menu with `FntPrint`: a
title, then for each line a spacer (`PrintTabs(4)`), a marker string
that depends on whether the line is the highlighted one (`D_8009574A`), and
the line's number from the format at `sHitDataNoFmt`.

```c
void PrintToolSlots(void) {
    s16 i;

    FntPrint(sHitNewlines2);
    for (i = 0; i < 3; i++) {
        PrintTabs(4);
        if (D_8009574A == i) {
            FntPrint(sHitColorWhite);
        } else {
            FntPrint(sHitColorBlue);
        }
        FntPrint(sHitDataNoFmt, i + 1);
    }
}
```

## Externs

- `D_8009574A`: `lui $v1` / `lh $v1, %lo` (same register), a plain `s16`
  scalar outside `$gp`.
- `sHitNewlines2`, `sHitColorWhite`, `sHitColorBlue`: `.sdata` strings reached
  `lui`/`addiu`, declared `char []`. Declaring the two markers `char [8]`
  (cc1's single-insn `la`) scored 1/38 to 4/38: the address is computed
  before the compare and overwritten, and `sHitDataNoFmt`'s address is hoisted
  into `$s2`.
- `PrintTabs` is defined in `main.c`; its prototype went into
  `include/common.h` (additive, same type as the definition) because
  declcheck rejects a local prototype of another unit's function.

## Levers

| C | result |
| --- | --- |
| `for`, `FntPrint(D_8009574A == i ? A : B)` | 1/38 + drift: the shared `%hi` of the two markers hoisted out of the loop into `$s1` (extra saved register, 2 words long) |
| same as `do`/`while` | 1/38, same hoist |
| same with a `goto` loop (no loop notes, so no invariant motion) | 32/38: right length, the shared `%hi` in `$v0` instead of `$a0` |
| `char *s` set in `if`/`else`, one call, `goto` loop | 32/38, same |
| **`if (...) FntPrint(A); else FntPrint(B);`**, `goto` or `for` loop | **38/38** |

With one call per arm, cross-jumping merges the two calls after each arm
has built its argument directly in `$a0`, so no shared `%hi` pseudo exists
to hoist or to land in `$v0`.

### Proposed learning

A call at a join whose argument is built `lui $a0` / `addiu $a0` separately
in each arm (two symbols sharing a `%hi`): the source called the function in
each arm, `if (c) f(A); else f(B);`, and cross-jumping merged the calls. A
ternary or a pointer local makes one `%hi` pseudo shared by both arms, which
lands in `$v0` or is hoisted out of an enclosing loop. (PrintToolSlots)
