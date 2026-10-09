# func_8002CAE4 — MATCHED 16/16 (round 3, alpha)

Unit `src/code_1a098.c`. The two-endpoint sibling of `func_8002CAA4`:
`out[1] = func_80018D04(a, b, t, n)`, `out` the fifth (stack) argument.

```c
void func_8002CAE4(s16 a, s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(a, b, t, n);
}
```

Matched with the `s32` prototype of `func_80018D04` found for
`func_8002CAA4` (see that report). The `s16`/`u16` parameters give retail's
`sll`/`sra` and `andi` on entry.
