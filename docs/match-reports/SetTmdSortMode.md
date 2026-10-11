# SetTmdSortMode — MATCHED (56/56), round 6, runner charlie

> Renamed from `func_80020CF8` on 2026-10-10 (tools/rename.py). Address 0x80020cf8.

Unit `src/draw.c`. Selects one of five pairs of handlers for row 3 and
row 7 column 0 of the handler table `gTmdSortFuncs` by `mode` (0 and 1 share
the first pair); any other mode leaves them.

```c
s32 SetTmdSortMode(s32 mode) {
    switch (mode) {
        case 0:
        case 1:
            gTmdSortFuncs[3][0] = SortTmdLitEnvMapGT3;
            gTmdSortFuncs[7][0] = SortTmdLitEnvMapGT4;
            break;
        case 2:
            gTmdSortFuncs[3][0] = SortTmdWireGT3;
            gTmdSortFuncs[7][0] = SortTmdWireGT4;
            break;
        case 3:
            gTmdSortFuncs[3][0] = SortTmdEnvMapGT3;
            gTmdSortFuncs[7][0] = SortTmdEnvMapGT4;
            break;
        case 4:
            gTmdSortFuncs[3][0] = SortTmdLitGT3;
            gTmdSortFuncs[7][0] = SortTmdLitGT4;
            break;
        case 5:
            gTmdSortFuncs[3][0] = SortTmdLitSemiGT3;
            gTmdSortFuncs[7][0] = SortTmdLitSemiGT4;
            break;
    }
}
```

## Attempts (3)

1. `void`, `s32 mode`: 53/56. The bound check's `beqz` delay slot held the
   jump table's `lui $v0` instead of retail's `sll $v1, $a0, 2`.
2. `u32 mode`: no change.
3. `s32` return with no `return` statement: 56/56. With `$v0` live on the
   branch-taken path (the return value), reorg cannot put `lui $v0` in the
   slot and takes the next safe instruction, the index shift.

Triggers `-Wall`'s "control reaches end of non-void function" (NEW in
`tools/typeviews.py --warnings`; to be baselined).

### Proposed learning

- **A jump table whose bound-check delay slot holds the index `sll`, not
  the table's `lui $v0`: the function is non-void with no `return`.** The
  live `$v0` keeps the `lui` out of the slot. Same mechanism as
  ClearFlags128's loop slot. (SetTmdSortMode)
