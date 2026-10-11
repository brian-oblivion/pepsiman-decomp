# ClearSwitchBoxUpper — MATCHED 10/10 (round 1, alpha)

> Renamed from `ClearRec5CUpper` on 2026-10-11 (tools/rename.py). Address 0x80033878.

> Renamed from `func_80033878` on 2026-10-11 (tools/rename.py). Address 0x80033878.

Unit `src/editor.c`. Sets byte 0 of records 100..199 of a 200-entry
table of 0x5C-byte records (`SwitchBox`, declared in the unit) to -1: the second
half of what `ClearSwitches` does to the whole table.

```c
void ClearSwitchBoxUpper(SwitchBox *recs) {
    u32 i;

    recs += 100;
    for (i = 100; i < 200; i++) {
        recs->unk0 = -1;
        recs++;
    }
}
```

## Levers

- **Advance the pointer before the loop, in the source.** `recs[i].unk0 = -1`
  with `i` from 100 is 7/10: GCC strength-reduces it to the same loop but
  emits the `addiu $a0,$a0,0x23F0` after the two constants. Writing
  `recs += 100;` as its own statement puts it first, as retail has it.
- u32 counter, as in `ClearSwitches`.

This commit also retypes `ClearSwitches` from `s8 *` to `SwitchBox *` (still
9/9), since both walk the same table.
