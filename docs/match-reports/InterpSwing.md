# InterpSwing — MATCHED 68/68 (round 8, alpha)

> Renamed from `func_8002C994` on 2026-10-11 (tools/rename.py). Address 0x8002c994.

Unit `src/object.c`. Interpolates three angles by `t`/`n` with
Lerp and converts each from degrees to 4096ths of a turn:
out[1] = `r` times the sine of 0..180 degrees, out[0] = -60..60 degrees,
out[2] = 0..20 degrees. Matched on the first build.

```c
void InterpSwing(s16 r, u16 t, u16 n, s32 *out) {
    out[1] = rsin((s16)Lerp(0, 180, t, n) * 4096 / 360) * r;
    out[0] = (s16)Lerp(-60, 60, t, n) * 4096 / 360;
    out[2] = (s16)Lerp(0, 20, t, n) * 4096 / 360;
}
```

Retail narrows each result (`sll 16; sra 4`, the `(s16)` and the `* 4096`
in one pair) where InterpFromZero/CAE4/CB24 store it unextended: this
unit's prototype is `s32` for those, so the cast is written here.
