# RunValueMenu — MATCHED 49/49 (round 8, bravo)

> Renamed from `func_80033B34` on 2026-10-11 (tools/rename.py). Address 0x80033b34.

Unit `src/editor.c`. Runs `StepMenuCursor` (the menu update); if the
highlighted line `gMenuLine` changed, loads the edited value `gEditValue`
and its limit `gEditMax` for the new line (line 0: limit 200, value from
`sTotals.unk12`; line 1: limit 3, value from `sTotals.unk14`), otherwise
saves the edited value back into the current line's slot. First build
matched.

## Shapes read from retail

- `D_80095B3A` / `D_80095B3C` are reached `lui $v0` / `lhu $v1,
  %lo(..)($v0)`, a cc1 split, so they are members of the `u8 []`-declared
  `gEditorState` block: `EditorState` gained `unk12` and `unk14` (offsets 0x12
  and 0x14, carved out of its leading padding; layout unchanged).
- The old line is read before the call into a saved register as `s16`
  (`sll`/`sra` after the call): an `s16` local.
- Two `switch`es on {0, 1}; the first switch's two arms share their
  stores through cross-jumping (`j .L80033B94`).

```c
void RunValueMenu(void) {
    s16 old;

    old = gMenuLine;
    StepMenuCursor();
    if (old != gMenuLine) {
        switch (gMenuLine) {
        case 0:
            gEditMax = 200;
            gEditValue = sTotals.unk12;
            break;
        case 1:
            gEditMax = 3;
            gEditValue = sTotals.unk14;
            break;
        }
    } else {
        switch (old) {
        case 0:
            sTotals.unk12 = gEditValue;
            break;
        case 1:
            sTotals.unk14 = gEditValue;
            break;
        }
    }
}
```
