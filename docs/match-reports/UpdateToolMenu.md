# UpdateToolMenu — MATCHED 112/112 (round 9, echo)

> Renamed from `func_800345C8` on 2026-10-11 (tools/rename.py). Address 0x800345c8.

Unit `src/hit_tool.c`. A five-line menu step: sets the edit limit and
wrap count, runs the menu (`StepMenuCursor`), and on flag bit 5 acts on the
highlighted line (`gMenuLine`): lines 0 and 1 arm an edit and record the
line in `gToolKind`; lines 2 and 3 toggle `sToolPickMode` / `sToolInfoMode` and,
when `sToolPickMode` is set, pick an error code by `gToolKind`; line 4 sets
error 7. Flag bit 6 clears the error code.

```c
void UpdateToolMenu(void) {
    gEditMax = 1;
    gMenuLineCount = 5;
    StepMenuCursor();
    if (gPadTrig & 0x20) {
        switch (gMenuLine) {
            case 0:
                gToolKind = gMenuLine;
                gEditMax = 0x33;
                gMenuLineCount = 2;
                gMenuLine = 0;
                gToolMode = 2;
                sToolPickMode = 0;
                sToolInfoMode = 0;
                gEditValue = gToolRadius;
                break;
            case 1:
                gToolKind = gMenuLine;
                gMenuLine = 0;
                gEditValue = 0;
                gToolMode = 4;
                sToolPickMode = 0;
                sToolInfoMode = 0;
                break;
            case 2:
                sToolInfoMode = 0;
                sToolPickMode ^= 1;
            case 3:
                if (gMenuLine == 3) {
                    sToolPickMode = sToolInfoMode = sToolInfoMode ^ 1;
                }
                gSubStep = 0;
                if (sToolPickMode == 1) {
                    switch (gToolKind) {
                        case 0:
                            gToolMode = 3;
                            break;
                        case 1:
                            gToolMode = 5;
                            break;
                    }
                }
                break;
            case 4:
                gSubStep = 0;
                gToolMode = 7;
                break;
        }
    }
    if (gPadTrig & 0x40) {
        gToolMode = 0;
    }
}
```

## Levers

| C | words |
| --- | --- |
| `gToolKind = gMenuLine;` after `gMenuLine = 0;` | 97/112: the byte reloaded after the store (cc1 does not forward it, but the order is wrong) |
| a `line` local loaded at the case top, stored at the end (u8, s16, s32, u16) | 104/112: same code, `$v1`/`$a0` swapped |
| **`gToolKind = gMenuLine;` as each case's FIRST statement** | **112/112**: the scheduler sinks the `sb` to the end of the case, the `lbu` stays first |

Not a twin of DrawHitQuad (same length, different body).

### Proposed learning

- **A byte load at the top of a case whose store comes last, after stores
  that change the loaded global: the assignment was the case's first
  statement**; the scheduler sinks the store. A local carrying the value
  gives the same code with swapped registers. (UpdateToolMenu)
