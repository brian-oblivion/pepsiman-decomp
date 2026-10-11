# InterpOvershoot — MATCHED 64/64 (round 8, alpha)

> Renamed from `func_8002CB24` on 2026-10-11 (tools/rename.py). Address 0x8002cb24.

Unit `src/code_1a098.c`. The third of the interpolation wrappers after
InterpFromZero and InterpRange: while `t` is under 80 percent of `n`
(compared in doubles, each scaled to per-4096 by `* 4096 / 100`) it
interpolates from 0 towards `2b`, after that from `4b` towards `b`.
Matched on the first build.

```c
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

void InterpOvershoot(s16 b, u16 t, u16 n, s32 *out) {
    if ((double)(t * 4096 / 100) < (double)(n * 4096 / 100) * 0.8) {
        out[1] = Lerp(0, b * 2, t, n);
    } else {
        out[1] = Lerp(b * 4, b, t, n);
    }
}
```

## Change to an existing declaration (this unit's .c only)

The unit's view of Lerp (code_13068, `s16 (s16, s16, u16, u16)`)
was `s32 (s16 a, s16 b, u16 t, u16 n)`. Retail passes `b * 2` as
`sll 16; sra 15` and `b * 4` as `sll 2` of the extended `b`, with no
truncation to 16 bits, so `a` and `b` are now `s32` in the view as well.
InterpFromZero and InterpRange still match (16/16 each). The MATCHING
note on the prototype says so.

The two calls are written separately; cross-jumping merges them into the
one `jal` retail has.
