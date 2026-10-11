# UpdateSpeedMode3 — MATCHED 68/68 (round 5, delta)

> Renamed from `func_80027D04` on 2026-10-11 (tools/rename.py). Address 0x80027d04.

Unit `src/code_13068.c`. In game-state mode 0x33 (flag byte set), steers
`gSpeedFixed`, a value in 1/16 units whose whole part goes to
`D_8009EF20[0]`: below 30 in the count at 0x3A8 it rises by 16; otherwise
pad bit 0x1000 adds 32 and bit 0x4000 subtracts 32, each snapping to 480
(30 whole) once it passes 30; with neither, it drifts 16 towards 30 (down
above 30, up at exactly 30).

```c
extern u16 gSpeedFixed;    /* unit */
extern s32 gPad;    /* unit */
extern s16 D_8009EF20[];  /* common.h */

void UpdateSpeedMode3(void) {
    s16 v;

    if (sGame.unk0 != 0 && sGame.unk6 == 0x33) {
        v = sGame.unk3A8;
        if (v < 30) {
            gSpeedFixed += 16;
        } else if (gPad & 0x1000) {
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 30) {
                gSpeedFixed = 480;
            }
        } else if (gPad & 0x4000) {
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 30) {
                gSpeedFixed = 480;
            }
        } else if (v > 30) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        D_8009EF20[0] = (s16)gSpeedFixed >> 4;
    }
}
```

## Levers

- None needed: the straightforward if/else-if chain matched on the first
  build. `(s16)` of the `u16` global gives retail's `sll 16; sra 20`, and
  cross-jumping shares the stores of 480, -16 and +16 at one `sh`.
