# SelectToolSection — MATCHED 34/34 (round 2, charlie)

> Renamed from `func_8003708C` on 2026-10-11 (tools/rename.py). Address 0x8003708c.

Unit `src/hit_tool.c`. Steps the index `gToolSection` up when bit 3 of the
flag word `gPadTrig` is set and down when bit 2 is, then clamps it to
`0 .. gSectionCount - 1`.

```c
void SelectToolSection(void) {
    s32 flags = gPadTrig;

    if (flags & 8) {
        gToolSection++;
    }
    if (flags & 4) {
        gToolSection--;
    }
    gToolSection = gToolSection < 0 ? 0 : gToolSection > gSectionCount - 1 ? gSectionCount - 1 : gToolSection;
}
```

## Externs

`gToolSection` is `%gp_rel` (in `config/gp-symbols.txt`), loaded `lh` for the
compare: `extern s16`. `gSectionCount` is reached `lui`/`lh` and is not in the
list: a plain `extern s16`. Both declared in the unit.

## Levers

| C | result |
| --- | --- |
| clamp as `if (x < 0) x = 0; else if (x > n - 1) x = n - 1;` | not matching: a store in each branch, `bgez` skeleton, early `jr` |
| **one assignment of a nested ternary** | **34/34** |

Retail stores `gToolSection` once, at a join, in every path including the
unchanged one (it reloads the value `lhu` into `$a0` for that path): the
source assigned the whole clamp expression.

### Proposed learning

A clamp whose single store sits at a join that every path reaches (the
"unchanged" path copying a reloaded value): one assignment of a nested
ternary, `x = x < lo ? lo : x > hi ? hi : x;`. An `if`/`else if` clamp
stores in each branch.
