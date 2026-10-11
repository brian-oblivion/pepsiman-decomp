# SteerMode2 — MATCHED 123/123 (round 7, delta)

> Renamed from `func_80027A00` on 2026-10-11 (tools/rename.py). Address 0x80027a00.

Unit `src/player.c`. The twin of SteerMode3 for another mode:
pad bits 0x8000/0x2000 call `ApplySteer(3|2, 16, 5)` and nudge the
lateral speed `sSteerDrift` by -/+4, otherwise byte 5 becomes 0x58 and the
speed decays by 2; on stages 3 and 10 the value 0x34C is pinned below its
cap (0x5A and 0x50 under 0x3C0). The speed is clamped to -10..10 and the
tail is SteerMode3's (push sideways by a tenth of it, clamp the
heading to the camera yaw +/- 0x71).

```c
void SteerMode2(void) {
    s16 step;
    s32 v;
    s32 x;
    GameState *g;
    s32 hi;

    if (sGame.unk0 != 0) {
        if (gPad & 0x8000) {
            ApplySteer(3, 16, 5);
            sSteerDrift -= 4;
        } else if (gPad & 0x2000) {
            ApplySteer(2, 16, 5);
            sSteerDrift += 4;
        } else {
            sGame.unk5 = 0x58;
            if (sSteerDrift != 0) {
                if (sSteerDrift < 0) {
                    sSteerDrift += 2;
                } else {
                    sSteerDrift -= 2;
                }
            }
        }
        switch (gStage) {
            case 3:
                sGame.unk34C = sGame.unk3C0 - 0x5A;
                break;
            case 10:
                sGame.unk34C = sGame.unk3C0 - 0x50;
                break;
        }
        sSteerDrift = sSteerDrift < -10 ? -10 : sSteerDrift > 10 ? 10 : sSteerDrift;
        step = sSteerDrift / 10;
        pushBack(90, step);
        /* MATCHING: as in SteerMode3. */
        __asm__("");
        g = &sGame;
        x = g->unk380;
        v = gCourse->vy - 0x71;
        if (x >= v) {
            hi = gCourse->vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->unk380 = v;
    }
}
```

The `pushBack` inline helper (PushPlayer's body) moved above this
function so both users follow it; see SteerMode3's report for it.

## Path

- SteerMode3's matched shape with this function's constants and a
  two-case `switch` on `gStage`: 123/123 and oracle OK on the first
  build.

## Levers

- Everything from SteerMode3 (inline helper, `__asm__("")` barrier,
  `g = &sGame`, the if/else clamp) carries over unchanged, which is some
  evidence the shared tail was one construct in the original source.
