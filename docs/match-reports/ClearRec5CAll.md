# ClearRec5CAll — MATCHED 9/9 (round 1, alpha)

> Renamed from `func_80033854` on 2026-10-11 (tools/rename.py). Address 0x80033854.

Unit `src/code_1dc24.c`. Fills byte 0 of 200 consecutive 0x5C-byte records
with -1.

```c
void ClearRec5CAll(Rec5C *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->unk0 = -1;
        recs++;
    }
}
```

## Levers

- **The counter is `u32`.** Retail counts up from 0 and tests with
  `sltiu`. An `s32` counter makes GCC 2.8.1 reverse the loop into a
  count-down with `bgez` (0/9, image drifts).
- **The stored byte is `s8` -1**: retail materialises `addiu $a1,$zero,-1`;
  a `u8` 0xFF gives `addiu ...,0xFF`.

First matched as `s8 *p` stepping 0x5C; retyped to the unit's `Rec5C`
(0x5C bytes, a signed byte at 0) with `ClearRec5CUpper`, still 9/9.
