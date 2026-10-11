# AdjustScreenPosition — MATCHED 72/72 (round 4, delta)

> Renamed from `func_800148B0` on 2026-10-10 (tools/rename.py). Address 0x800148b0.

Unit `src/main.c`. An interactive screen-position adjuster: after a
20-frame wait it loops reading the pad word `gPadHeld[0]`, moving x and y
by 8 per frame on four direction bits, clamping x to 0..0x2C0 and y to
0..0x110, and redefining the display buffers there, until bit 0x800 is
pressed. Then it reinitialises 3D and the game's graphics and waits five
frames.

```c
void AdjustScreenPosition(void) {
    s32 x;
    s32 y;
    s32 pad;

    x = 0;
    y = 0;
    WaitFrames(20);
    for (;;) {
        pad = gPadHeld[0];
        if (pad & 0x2000) {
            x += 8;
        }
        if (pad & 0x8000) {
            x -= 8;
        }
        if (pad & 0x1000) {
            y -= 8;
        }
        if (pad & 0x4000) {
            y += 8;
        }
        if (pad & 0x800) {
            break;
        }
        x = x < 0 ? 0 : x > 0x2C0 ? 0x2C0 : x;
        y = y < 0 ? 0 : y > 0x110 ? 0x110 : y;
        GsDefDispBuff(x, y, x, y);
        ReadPad(0);
        DrawSync(0);
        VSync(0);
        GsSwapDispBuff();
    }
    InitGraphics();
    GsInit3D();
    InitView();
    InitLights();
    WaitFrames(5);
}
```

First build. The clamps are LEARNINGS' nested-ternary clamp (one store at
the join); `GsDefDispBuff`'s `u_short` parameters give the `andi 0xFFFF`s.
`gPadHeld` keeps the `s32 [2]` declaration from `InitPads`: its
known size of 8 makes this load symbolic, `lui $v1` / `lw $v1`, as retail
has it. `InitView` is C in model, so its prototype went into
`include/common.h` (additive); `InitLights` is still asm there and is
prototyped in main.c. The unit's forward prototypes for its own functions
now sit together above this function.
