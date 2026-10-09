# func_8003E1FC — MATCHED (40/40), round 3, runner bravo

Unit `src/code_29f54.c`. Leaf. Writes to `out` the foot of the perpendicular
from the origin to the line through (x0, y0) and (x1, y1), relative to
(x0, y0): `t * d / |d|^2` with `d = p1 - p0` and `t = -(p0 . d)`.

```c
void func_8003E1FC(s16 *out, s16 x0, s16 y0, s16 x1, s16 y1) {
    s32 dx;
    s32 dy;
    s32 t;
    s32 d;

    dx = x1 - x0;
    dy = y1 - y0;
    t = -(x0 * dx + y0 * dy);
    d = dx * dx + dy * dy;
    out[0] = t * dx / d;
    out[1] = t * dy / d;
}
```

The fifth argument is the `lh 0x10($sp)`. The divides carry no
divide-by-zero trap in retail and none in the build (the pinned flags). No
callers in `asm/` reach it by `jal`, so the names are read from the
arithmetic only. First try.
