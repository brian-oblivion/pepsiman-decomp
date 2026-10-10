# func_8002F8FC — MATCHED 95/95 (round 8, bravo)

Unit `src/code_1dc24.c`. Fills the six camera words `D_800DB2A0[0..5]`:
the sine and cosine of the first rotation's yaw times 500, the game
height `sGameSave.unk348[1]` plus `sTotals.unkC` plus the sine of the
view's orbit angle `D_80095914` (degrees, `* 4096 / 360`) times 500, and
the sine and cosine of the yaw minus 0x800 times 900, all `/ 4096`.
Second build matched.

## Shapes read from retail

- **`x / 4096 + (a + b)`**: retail sums the two globals into one register
  before adding the sine term, and computes the sine term first. Written
  `a + b + sin...`, cc1 loads the globals before the multiply (77/95).
- `D_80095B34` is reached `lui $v0` / `lw $a1, %lo(..)($v0)`, a cc1 split:
  a member of the `D_80095B28` block. `Totals28` gained `s32 unkC`
  (layout unchanged).
- `D_80095914` is an `s16` scalar (`lui $a0` / `lh $a0`). It was declared
  in `src/code_7d74.c`; with a second unit using it, the declaration moved
  to `include/common.h` (declcheck MULTI/LOCAL). This is an edit to
  code_7d74.c outside the round's unit, a one-line deletion.

```c
void func_8002F8FC(void) {
    SVECTOR *rot;

    rot = D_800A7680;
    D_800DB2A0[0] = rsin(rot->vy) * 500 / 4096;
    D_800DB2A0[1] = rsin(D_80095914 * 4096 / 360) * 500 / 4096 +
                    (sGameSave.unk348[1] + sTotals.unkC);
    D_800DB2A0[2] = rcos(rot->vy) * 500 / 4096;
    D_800DB2A0[3] = rsin(rot->vy - 0x800) * 900 / 4096;
    D_800DB2A0[4] = sGameSave.unk348[1];
    D_800DB2A0[5] = rcos(rot->vy - 0x800) * 900 / 4096;
}
```
