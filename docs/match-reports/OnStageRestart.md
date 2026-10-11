# OnStageRestart — MATCHED 16/16 (round 6, delta)

> Renamed from `func_80041118` on 2026-10-11 (tools/rename.py). Address 0x80041118.

Unit `src/stage_flow.c`. Calls the overlay routine `func_800FA5D0` when the
byte argument is 0; 1 and above do nothing. Same shape as OnCanCollected.

```c
void func_800FA5D0(void);

void OnStageRestart(u8 arg0) {
    s32 mode = arg0;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA5D0();
            }
        }
    }
}
```

## Levers

- First build. The `== 1` / `slti 2` / `bnez` chain is LEARNINGS' "nested
  `if`s on an `s32` copy" (OnCanCollected), with the `andi 0xFF` from a `u8`
  parameter.
