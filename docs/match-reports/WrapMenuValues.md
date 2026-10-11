# WrapMenuValues — MATCHED 44/44 (round 6, bravo)

> Renamed from `func_80033E98` on 2026-10-11 (tools/rename.py). Address 0x80033e98.

Unit `src/editor.c`. Wraps two menu values into range and returns 0:
the edited value `gEditValue` against its limit `gEditMax`, then the
highlighted line `gMenuLine` against the line count `gMenuLineCount`. A
value below 0 becomes limit - 1; one at or past the limit becomes 0.

```c
s32 WrapMenuValues(void) {
    /* MATCHING: retail reads the edited value signed (lh); common.h's
     * declaration is u16. */
    if ((s16)gEditValue < 0) {
        gEditValue = gEditMax - 1;
    }
    if ((s16)gEditValue >= gEditMax) {
        gEditValue = 0;
    }
    if (gMenuLine < 0) {
        gMenuLine = gMenuLineCount - 1;
    }
    if (gMenuLine >= gMenuLineCount) {
        gMenuLine = 0;
    }
    return 0;
}
```

First build. Retail loads `gEditValue` with `lh` (and tests it with
`bgez`/`slt`), but `include/common.h` declares it `u16`; the `(s16)` cast
folds into `lh` with no `sll`/`sra`. The reload after the store is cc1's
own (the stored value is not known sign-extended).

Not changed: `gEditValue`'s shared declaration. Its only `lh` loads are
the three here; the other retail loads (EditorEditRecordValues, EditorEditSpriteValues,
StepMenuCursor, RunValueMenu, all still asm in this unit) are `lhu`,
which an `s16` gives too for plain copies and halfword arithmetic, so it
may well be `s16`. That is the head's call; the cast is unit-local.

### Proposed learning

- **A global declared unsigned but loaded `lh` and tested signed: an
  `(s16)` cast at the test folds into the `lh`** with no shifts, when the
  shared declaration cannot change. (WrapMenuValues)
