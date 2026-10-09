# func_80027BEC — MATCHED 70/70 (round 5, delta)

Unit `src/code_13068.c`. The twin of func_80027D04 for game-state modes
0x33..0x35 with 35 in place of 30: steers `D_800957D2` (1/16 units)
towards 35 from the count at 0x3A8 and pad bits 0x1000/0x4000 of
`D_80095964`, snapping to 560 (35 whole), and stores the whole part in
`D_8009EF20[0]`.

```c
void func_80027BEC(void) {
    s16 v;

    if (sGame.unk0 != 0 && (u32)(sGame.unk6 - 0x33) < 3) {
        v = sGame.unk3A8;
        if (v < 35) {
            D_800957D2 += 16;
        } else if (D_80095964 & 0x1000) {
            D_800957D2 += 32;
            if ((s16)D_800957D2 >> 4 > 35) {
                D_800957D2 = 560;
            }
        } else if (D_80095964 & 0x4000) {
            D_800957D2 -= 32;
            if ((s16)D_800957D2 >> 4 < 35) {
                D_800957D2 = 560;
            }
        } else if (v > 35) {
            D_800957D2 -= 16;
        } else {
            D_800957D2 += 16;
        }
        D_8009EF20[0] = (s16)D_800957D2 >> 4;
    }
}
```

## Levers

- **A range test on a byte written as `x >= 0x33 && x < 0x36` stays two
  compares** in 2.8.1 (`sltiu 0x33; bnez; sltiu 0x36`). Retail's `addiu
  -0x33; sltiu 3` is the unsigned form `(u32)(x - 0x33) < 3`. (A `switch`
  with three shared cases would likely give the same range check; not
  tried.)
- The rest is func_80027D04's body with the constants changed.

### Proposed learning

`addiu r, x, -K; sltiu r, r, N` on a value: the source tested
`(u32)(x - K) < N` (or a switch range); 2.8.1 does not fold `x >= K && x <
K + N` into it.
