# ZeroBytes — MATCHED (round 1, bravo)

> Renamed from `func_80038990` on 2026-10-10 (tools/rename.py). Address 0x80038990.

9/9 words. Zeroes `n` bytes, a counted `for` with a post-incremented pointer:

```c
void ZeroBytes(u8 *p, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        *p++ = 0;
    }
}
```

The signed `slt` on the counter fixes `n` and `i` as `s32`.
