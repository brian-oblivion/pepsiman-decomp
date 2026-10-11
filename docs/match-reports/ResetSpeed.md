# ResetSpeed — MATCHED 7/7 (round 2, alpha)

> Renamed from `func_800287C0` on 2026-10-11 (tools/rename.py). Address 0x800287c0.

Unit `src/code_13068.c`. Clears `D_8009EF20[0]` and sets `gSpeedFixed` to
0x280 (640).

```c
void ResetSpeed(void) {
    D_8009EF20[0] = 0;
    gSpeedFixed = 0x280;
}
```

First try. `D_8009EF20` is common.h's `s16[]` (split `lui $v0`/`%lo`);
`gSpeedFixed` is a plain `extern u16` in the unit (stored `lui $at`, loaded
`lhu` elsewhere in the unit).
