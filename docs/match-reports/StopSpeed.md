# StopSpeed — MATCHED 6/6 (round 2, alpha)

> Renamed from `func_800287DC` on 2026-10-11 (tools/rename.py). Address 0x800287dc.

Unit `src/player.c`. Clears `D_8009EF20[0]` and `gSpeedFixed`.

```c
void StopSpeed(void) {
    D_8009EF20[0] = 0;
    gSpeedFixed = 0;
}
```

First try; declarations as in ResetSpeed.
