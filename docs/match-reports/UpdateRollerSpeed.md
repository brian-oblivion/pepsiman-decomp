# UpdateRollerSpeed — MATCHED 70/70 (round 5, delta)

> Renamed from `UpdateSpeedMode2` on 2026-10-11 (tools/rename.py). Address 0x80027bec.

> Renamed from `func_80027BEC` on 2026-10-11 (tools/rename.py). Address 0x80027bec.

Unit `src/player.c`. The twin of UpdateCoverSpeed for game-state modes
0x33..0x35 with 35 in place of 30: steers `gSpeedFixed` (1/16 units)
towards 35 from the count at 0x3A8 and pad bits 0x1000/0x4000 of
`gPad`, snapping to 560 (35 whole), and stores the whole part in
`D_8009EF20[0]`.

```c
void UpdateRollerSpeed(void) {
    s16 v;

    if (sGame.unk0 != 0 && (u32)(sGame.unk6 - 0x33) < 3) {
        v = sGame.unk3A8;
        if (v < 35) {
            gSpeedFixed += 16;
        } else if (gPad & 0x1000) {
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 35) {
                gSpeedFixed = 560;
            }
        } else if (gPad & 0x4000) {
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 35) {
                gSpeedFixed = 560;
            }
        } else if (v > 35) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        D_8009EF20[0] = (s16)gSpeedFixed >> 4;
    }
}
```

## Levers

- **A range test on a byte written as `x >= 0x33 && x < 0x36` stays two
  compares** in 2.8.1 (`sltiu 0x33; bnez; sltiu 0x36`). Retail's `addiu
  -0x33; sltiu 3` is the unsigned form `(u32)(x - 0x33) < 3`. (A `switch`
  with three shared cases would likely give the same range check; not
  tried.)
- The rest is UpdateCoverSpeed's body with the constants changed.

### Proposed learning

`addiu r, x, -K; sltiu r, r, N` on a value: the source tested
`(u32)(x - K) < N` (or a switch range); 2.8.1 does not fold `x >= K && x <
K + N` into it.
