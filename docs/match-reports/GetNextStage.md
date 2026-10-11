# GetNextStage — MATCHED 8/8 (round 12, alpha)

> Renamed from `func_80041158` on 2026-10-11 (tools/rename.py). Address 0x80041158.

Unit `src/stage_overlay.c`. Returns entry `i` of the halfword table
`sNextStage`. No caller in C yet.

Matched on the first build.

```c
extern u16 sNextStage[];

u16 GetNextStage(u16 i) {
    return sNextStage[i];
}
```

## Notes

- `andi a0, 0xFFFF` at entry is the callee re-extending a `u16` parameter.
- The table is reached `lui`/`addiu` with the scaled index added afterwards:
  an array of unknown size.
