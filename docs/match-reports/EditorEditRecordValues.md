# EditorEditRecordValues — MATCHED 141/141 (round 9, alpha)

> Renamed from `func_80030B6C` on 2026-10-11 (tools/rename.py). Address 0x80030b6c.

Unit `src/code_1dc24.c`. The tool's menu editor: saves the highlighted line
(`gMenuLine`) and the edited value (`gEditValue`), runs
`StepMenuCursor`, clears `gSubStep` when line 1's value changed, then
either loads the new line's value and limit (`gEditMax`) from the tool
block after a line change, or stores the edited value back into the line's
field. Finally steps the angle `sTotals.unk20` up on flag bit 3 and down on
bit 2 of `gPad`, wrapping at 0..359. Second build matched.

## Shapes read from retail

- Two jump tables of five cases: `switch (gMenuLine)` after a change,
  `switch (line)` (the saved `s16`) otherwise. Cases 3 and 4 move bytes
  (`lbu` of the low byte of `gEditValue`, `sb`).
- `sTotals.unk20` reached through `lui`/`addiu` of `gEditorState` and
  `0x20($v1)`: the struct lvalue through the unit's `sTotals` macro, as
  the learning predicts.
- **The first build differed only in the last two `if`s: retail keeps
  `nop` in both branch delay slots where the build filled them.** `s32`
  with no `return` closed it (LEARNINGS, "Control flow and frames").

## Declarations

- `EditorState.unk20[6]` split into `s16 unk20` (the angle) and
  `u8 unk22[4]`. Unit-local type; whole oracle green.
- New `-Wall` warning "control reaches end of non-void function" in
  editor, deliberate; not yet in `config/typeviews-warnings.txt`
  (left for the head, the file is shared).

```c
s32 EditorEditRecordValues(void) {
    s16 line;
    s16 val;

    /* MATCHING: non-void with no return keeps the last two branches' delay
     * slots nops. */
    line = gMenuLine;
    val = gEditValue;
    StepMenuCursor();
    if (gMenuLine == 1 && val != (s16)gEditValue) {
        gSubStep = 0;
    }
    if (line != gMenuLine) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 200;
                gEditValue = sTotals.unk18;
                break;
            case 1:
                gEditMax = gSectionCount;
                gEditValue = sTotals.unk1C;
                break;
            case 2:
                gEditMax = 200;
                gEditValue = sTotals.unk1E;
                break;
            case 3:
                gEditMax = 0x100;
                gEditValue = sTotals.unk68;
                break;
            case 4:
                gEditMax = 0x100;
                gEditValue = sTotals.unk69;
                break;
        }
    } else {
        switch (line) {
            case 0:
                sTotals.unk18 = gEditValue;
                break;
            case 1:
                sTotals.unk1C = gEditValue;
                break;
            case 2:
                sTotals.unk1E = gEditValue;
                break;
            case 3:
                sTotals.unk68 = gEditValue;
                break;
            case 4:
                sTotals.unk69 = gEditValue;
                break;
        }
    }
    if (gPad & 8) {
        if (++sTotals.unk20 >= 360) {
            sTotals.unk20 = 0;
        }
    }
    if (gPad & 4) {
        if (--sTotals.unk20 < 0) {
            sTotals.unk20 = 359;
        }
    }
}
```
