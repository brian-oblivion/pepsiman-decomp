# GetStageVabBank — MATCHED 8/8 (round 12, alpha)

> Renamed from `func_80041178` on 2026-10-10 (tools/rename.py). Address 0x80041178.

Unit `src/code_31958.c`. The twin of func_80041158 on the halfword table
`D_8007B10C`. main.c passes the current stage (`gStage`) and hands the
result to LoadVabBank.

Matched on the first build.

```c
s32 GetStageVabBank(u16 stage) {
    return D_8007B10C[stage];
}
```

## Notes

- main.c had its own prototype `s32 GetStageVabBank(u8 stage)`. The body's
  `andi 0xFFFF` needs a `u16` parameter; main.c's call builds identically
  with `u16`, so the prototype moved to `include/common.h` as
  `s32 GetStageVabBank(u16 stage)` and main.c's copy was dropped (declcheck
  LOCAL/TYPE). The `s32` return keeps main.c's view; the `lhu` is unchanged.
