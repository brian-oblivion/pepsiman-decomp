# GetStageTimeLimit — MATCHED 33/33 (round 12, alpha)

> Renamed from `func_80041198` on 2026-10-10 (tools/rename.py). Address 0x80041198.

Unit `src/stage_overlay.c`. Looks up word `k` of entry `mode` in one of two
per-stage tables of three words (`gTimeLimitsAlt` when `gStageTableSet` is 1,
`gTimeLimits` otherwise) and returns it times 30. stage_flow has the same
lookup written inline (UpdateStagePlay's neighbourhood).

```c
s32 GetStageTimeLimit(u8 mode, s32 k) {
    if (gStageTableSet != 1) {
        return gTimeLimits[mode][k] * 30;
    }
    return gTimeLimitsAlt[mode][k] * 30;
}
```

## Notes

- First build, written `== 1` first, was 30/33: the right blocks with the
  tables swapped and `beq` for `bne`. Retail's fall-through block is the
  `== 1` one, yet the source tests `!= 1` first: cc1 inverts the test and
  lays the second return out first.
- `sllv v0, v1, v0` for `mode * 2` in one arm: cse took the shift count from
  `$v0`, which holds the 1 just compared.
- The two table externs were declared in stage_flow.c; with a second unit
  using them unchanged they moved to `include/common.h` (declcheck MULTI).
