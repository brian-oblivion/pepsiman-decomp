# UpdateToolMenu — MATCHED 112/112 (round 9, echo)

> Renamed from `func_800345C8` on 2026-10-11 (tools/rename.py). Address 0x800345c8.

Unit `src/code_24748.c`. A five-line menu step: sets the edit limit and
wrap count, runs the menu (`func_800330D4`), and on flag bit 5 acts on the
highlighted line (`D_8009574A`): lines 0 and 1 arm an edit and record the
line in `gToolKind`; lines 2 and 3 toggle `D_80095A58` / `D_80095A28` and,
when `D_80095A58` is set, pick an error code by `gToolKind`; line 4 sets
error 7. Flag bit 6 clears the error code.

```c
void UpdateToolMenu(void) {
    D_800958B0 = 1;
    D_800958B2 = 5;
    func_800330D4();
    if (gPadTrig & 0x20) {
        switch (D_8009574A) {
            case 0:
                gToolKind = D_8009574A;
                D_800958B0 = 0x33;
                D_800958B2 = 2;
                D_8009574A = 0;
                gToolMode = 2;
                D_80095A58 = 0;
                D_80095A28 = 0;
                D_80095748 = gToolRadius;
                break;
            case 1:
                gToolKind = D_8009574A;
                D_8009574A = 0;
                D_80095748 = 0;
                gToolMode = 4;
                D_80095A58 = 0;
                D_80095A28 = 0;
                break;
            case 2:
                D_80095A28 = 0;
                D_80095A58 ^= 1;
            case 3:
                if (D_8009574A == 3) {
                    D_80095A58 = D_80095A28 = D_80095A28 ^ 1;
                }
                gSubStep = 0;
                if (D_80095A58 == 1) {
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
| `gToolKind = D_8009574A;` after `D_8009574A = 0;` | 97/112: the byte reloaded after the store (cc1 does not forward it, but the order is wrong) |
| a `line` local loaded at the case top, stored at the end (u8, s16, s32, u16) | 104/112: same code, `$v1`/`$a0` swapped |
| **`gToolKind = D_8009574A;` as each case's FIRST statement** | **112/112**: the scheduler sinks the `sb` to the end of the case, the `lbu` stays first |

Not a twin of DrawHitQuad (same length, different body).

### Proposed learning

- **A byte load at the top of a case whose store comes last, after stores
  that change the loaded global: the assignment was the case's first
  statement**; the scheduler sinks the store. A local carrying the value
  gives the same code with swapped registers. (UpdateToolMenu)
