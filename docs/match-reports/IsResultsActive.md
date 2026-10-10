# IsResultsActive — MATCHED (3/3), round 3, runner bravo

> Renamed from `func_8003E544` on 2026-10-10 (tools/rename.py). Address 0x8003e544.

Unit `src/code_29f54.c`. Getter: returns the word `gResultsActive`.

```c
s32 IsResultsActive(void) {
    return gResultsActive;
}
```

No `jal` reaches it in `asm/`; `lw $v0, %gp_rel(gResultsActive)` before `jr $ra`
makes it a getter. `gResultsActive` (gp-relative everywhere) is a plain
`extern s32` in the unit. First try.
