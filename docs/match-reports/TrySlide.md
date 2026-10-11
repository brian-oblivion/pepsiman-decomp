# TrySlide — MATCHED 42/42 (round 3, delta)

> Renamed from `func_80028508` on 2026-10-11 (tools/rename.py). Address 0x80028508.

Unit `src/code_13068.c`. When the game-state record is live (`unk0`), its
mode byte `unk6` is outside 5..7, the `unk38F`/`unk398` gate is open, bit
0x1000 of `gPad` is clear and bit 0x40 of `gGamePadTrig` is set, it
plays `PlaySe(22)` and sets `unk6` to 5.

```c
/* MATCHING: non-void with no return keeps the second bnez's delay slot a nop. */
s32 TrySlide(void) {
    if (sGame.unk0 != 0 && (sGame.unk6 < 5 || sGame.unk6 >= 8) &&
        (sGame.unk38F != 1 || sGame.unk398 < 6) && !(gPad & 0x1000) && (gGamePadTrig & 0x40)) {
        PlaySe(22);
        sGame.unk6 = 5;
    }
}
```

## Levers

- **Non-void with no `return`** (ClearFlags128, UpdateSpeedBand). As `void`
  the build was 40/42: reorg filled the `bnez` after the `sltiu` range test
  with `addiu $v0,$zero,1` from the target and put a `nop` after it; retail
  keeps the slot a `nop`. `s32` keeps `$v0` live and blocks the fill. Adds a
  "control reaches end of non-void function" warning, NEW for code_13068 in
  `tools/typeviews.py --warnings`; the head baselines it.
- The 5..7 range test (`addiu -5` / `sltiu 3`) comes from either an `&&`
  spelling or a sequence of early `return`s; both gave the same code. The
  one-condition form was kept since an `s32` function with bare `return;`s
  reads worse.
- `gPad`, `gGamePadTrig`: adjacent `lui $v0` / `lw $v0`, so plain `s32`
  scalars, declared in the unit.
- `sGame` keeps the record's base in `$s0` (`addiu $s0,$v0,%lo`), as in
  ResetPlayerAction.
