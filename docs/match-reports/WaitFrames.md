# WaitFrames — MATCHED (25/25), round 1, runner charlie

> Renamed from `func_80014B8C` on 2026-10-10 (tools/rename.py). Address 0x80014b8c.

Runs `frames` frames: each calls `ReadPad(0)` then `VSync(0)`.

```c
void ReadPad(s32 arg);

void WaitFrames(s16 frames) {
    s16 i;

    for (i = 0; i < frames; i++) {
        ReadPad(0);
        VSync(0);
    }
}
```

First attempt. The `s16` parameter and counter come from retail's
`sll 16; sra 16` on `$a0` and on each increment. `VSync` comes from Sony's
`libetc.h`, now included. The prototype of `ReadPad` (same unit, still
`INCLUDE_ASM`) is local; it takes one argument (`$a0 = 0`), and its return
type is unknown (written `void`; the value is never used here).
