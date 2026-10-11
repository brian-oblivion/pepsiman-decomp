# StepMenuCursor — MATCHED 84/84 (round 8, bravo)

> Renamed from `func_800330D4` on 2026-10-11 (tools/rename.py). Address 0x800330d4.

Unit `src/editor.c`. The menu update: steps the edited value
`gEditValue` (pad bits 0x2000 up, 0x8000 down) or the highlighted line
`gMenuLine` (0x4000 up, 0x1000 down) from the pad word `gPadTrig`, else
from `gStickTrigX` (value) and `gStickTrigY` (line), then wraps both with
`WrapMenuValues`. Returns 0. Fourth build matched.

## Shapes read from retail

- **The call is written in every branch, not once after the chain.** With one
  call after an if/else-if chain (`else return 0;`), the eight step blocks
  jump to one label and cross-jumping merges the identical `gEditValue`
  blocks of the two pad groups (69 words, 15 short). With `f()` in each
  branch, the calls are tail-merged back into one, and only the
  `gMenuLine` blocks share code (their store, and the whole `--` block),
  exactly as retail.
- A final `else { return 0; }` before `return 0;` gives the early
  `beqz ..., epilogue` with `v0 = 0` in its slot; without it the two
  returns share one `move v0, zero` after the epilogue's `lw ra`.
- `gStickTrigX` / `gStickTrigY` are `s32` scalars (`lui $v1` / `lw $v1`),
  declared in the unit (hit_tool and main also read them).
- Returns 0 explicitly on both paths, so the function is `s32`;
  `include/common.h`'s prototype changed from `void` to `s32` (its callers
  discard the result; the whole image still matches).

```c
extern s32 gStickTrigX;
extern s32 gStickTrigY;
s32 WrapMenuValues(void);

s32 StepMenuCursor(void) {
    if (gPadTrig & 0x2000) {
        gEditValue++;
        WrapMenuValues();
    } else if (gPadTrig & 0x8000) {
        gEditValue--;
        WrapMenuValues();
    } else if (gPadTrig & 0x4000) {
        gMenuLine++;
        WrapMenuValues();
    } else if (gPadTrig & 0x1000) {
        gMenuLine--;
        WrapMenuValues();
    } else if (gStickTrigX & 0x2000) {
        gEditValue++;
        WrapMenuValues();
    } else if (gStickTrigX & 0x8000) {
        gEditValue--;
        WrapMenuValues();
    } else if (gStickTrigY & 0x4000) {
        gMenuLine++;
        WrapMenuValues();
    } else if (gStickTrigY & 0x1000) {
        gMenuLine--;
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
