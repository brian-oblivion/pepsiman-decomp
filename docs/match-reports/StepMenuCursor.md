# StepMenuCursor — MATCHED 84/84 (round 8, bravo)

> Renamed from `func_800330D4` on 2026-10-11 (tools/rename.py). Address 0x800330d4.

Unit `src/code_1dc24.c`. The menu update: steps the edited value
`D_80095748` (pad bits 0x2000 up, 0x8000 down) or the highlighted line
`D_8009574A` (0x4000 up, 0x1000 down) from the pad word `gPadTrig`, else
from `D_80095950` (value) and `D_80095958` (line), then wraps both with
`WrapMenuValues`. Returns 0. Fourth build matched.

## Shapes read from retail

- **The call is written in every branch, not once after the chain.** With one
  call after an if/else-if chain (`else return 0;`), the eight step blocks
  jump to one label and cross-jumping merges the identical `D_80095748`
  blocks of the two pad groups (69 words, 15 short). With `f()` in each
  branch, the calls are tail-merged back into one, and only the
  `D_8009574A` blocks share code (their store, and the whole `--` block),
  exactly as retail.
- A final `else { return 0; }` before `return 0;` gives the early
  `beqz ..., epilogue` with `v0 = 0` in its slot; without it the two
  returns share one `move v0, zero` after the epilogue's `lw ra`.
- `D_80095950` / `D_80095958` are `s32` scalars (`lui $v1` / `lw $v1`),
  declared in the unit (code_24748 and main also read them).
- Returns 0 explicitly on both paths, so the function is `s32`;
  `include/common.h`'s prototype changed from `void` to `s32` (its callers
  discard the result; the whole image still matches).

```c
extern s32 D_80095950;
extern s32 D_80095958;
s32 WrapMenuValues(void);

s32 StepMenuCursor(void) {
    if (gPadTrig & 0x2000) {
        D_80095748++;
        WrapMenuValues();
    } else if (gPadTrig & 0x8000) {
        D_80095748--;
        WrapMenuValues();
    } else if (gPadTrig & 0x4000) {
        D_8009574A++;
        WrapMenuValues();
    } else if (gPadTrig & 0x1000) {
        D_8009574A--;
        WrapMenuValues();
    } else if (D_80095950 & 0x2000) {
        D_80095748++;
        WrapMenuValues();
    } else if (D_80095950 & 0x8000) {
        D_80095748--;
        WrapMenuValues();
    } else if (D_80095958 & 0x4000) {
        D_8009574A++;
        WrapMenuValues();
    } else if (D_80095958 & 0x1000) {
        D_8009574A--;
        WrapMenuValues();
    } else {
        return 0;
    }
    return 0;
}
```

### Proposed learning

- **Identical step blocks that retail keeps apart, with one call after the
  chain merging them in the build: the call was written in each branch.**
  cross-jumping later folds the calls into one. (StepMenuCursor)
