# func_8002CAA4 — MATCHED 16/16 (round 3, alpha)

Unit `src/code_1a098.c`. Stores into `out[1]` the interpolation
`func_80018D04(0, b, t, n)` (`code_7d74`: from `a` towards `b` by `t`/`n`,
returning a sign-extended halfword).

```c
void func_8002CAA4(s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(0, b, t, n);
}
```

## The lever: the callee's prototype returns s32

With `s16 func_80018D04(...)` the caller re-extends the result (`sll`/`sra
16` after the `jal`, 11/16 and two words long). Retail stores `$v0`
untouched, so the unit prototypes it `s32` (with a `MATCHING:` note). The
callee does sign-extend its result, so the view is safe.

### Proposed learning

A call's result stored with no `sll`/`sra` although the callee returns a
halfword: prototype the callee `s32` in the calling unit (2.8.1 trusts a
promoted `s16` return no more than it is declared).
