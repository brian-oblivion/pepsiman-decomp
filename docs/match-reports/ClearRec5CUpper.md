# ClearRec5CUpper — MATCHED 10/10 (round 1, alpha)

> Renamed from `func_80033878` on 2026-10-11 (tools/rename.py). Address 0x80033878.

Unit `src/code_1dc24.c`. Sets byte 0 of records 100..199 of a 200-entry
table of 0x5C-byte records (`Rec5C`, declared in the unit) to -1: the second
half of what `ClearRec5CAll` does to the whole table.

```c
void ClearRec5CUpper(Rec5C *recs) {
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
- u32 counter, as in `ClearRec5CAll`.

This commit also retypes `ClearRec5CAll` from `s8 *` to `Rec5C *` (still
9/9), since both walk the same table.
