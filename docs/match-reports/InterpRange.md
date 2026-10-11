# InterpRange — MATCHED 16/16 (round 3, alpha)

> Renamed from `func_8002CAE4` on 2026-10-11 (tools/rename.py). Address 0x8002cae4.

Unit `src/code_1a098.c`. The two-endpoint sibling of `InterpFromZero`:
`out[1] = Lerp(a, b, t, n)`, `out` the fifth (stack) argument.

```c
void InterpRange(s16 a, s16 b, u16 t, u16 n, s32 *out) {
    out[1] = Lerp(a, b, t, n);
}
```

Matched with the `s32` prototype of `Lerp` found for
`InterpFromZero` (see that report). The `s16`/`u16` parameters give retail's
`sll`/`sra` and `andi` on entry.
