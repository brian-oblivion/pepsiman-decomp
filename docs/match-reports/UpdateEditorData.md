# UpdateEditorData — MATCHED 346/346 (round 10, echo)

> Renamed from `func_80031EF4` on 2026-10-11 (tools/rename.py). Address 0x80031ef4.

Unit `src/editor.c`. The tool's data-mode state machine, the sibling of
hit_tool's UpdateToolCard (round 9 stall): by `gSubStep`, state 0 opens
the card (`OpenCardEvents`), state 2 closes it (`CloseCardEvents`); state 1
steps the screen `gToolCardStep` through a 19-entry jump table (yes/no menus
through `RunMenu`, the card calls `RunCardOp1(mode)` and
`CheckCard`, the host tool file `WriteEditorFile` / `ReadEditorFile`, the
tool-buffer save slots `CheckEditorSlot` / `SaveEditorState` / `RestoreEditorState`,
the game-position reset of screen 10), then maps a card error (-2..4,
second jump table) to a message screen 11..15. Second build matched.

## Shapes read from retail

- Written from UpdateToolCard's preserved body with this function's calls,
  its screen numbers and its case 10 (the same game-position reset as
  func_8002F4FC's case 6). Case bodies in retail's block order: 0, 1, 2,
  10, 3, 4, 8, 5, 6, 7, 9, 11/13, 14, 15, 16, 17, 18.
- `CheckEditorSlot`'s and `RunMenu`'s results re-extended with
  `sll`/`sra`: an `(s16)` cast on the first, an `s16 r` for the second.
- Error table bound check with the index `sll` in its delay slot: `s32`
  with no return (LEARNINGS); new baselined warning.
- `blez` in cases 14/15: `case -1: break;` (UpdateToolCard's lever).
- **Cases 4 and 6, UpdateToolCard's residue: retail's case 4 branches
  `bnez r` into case 6's `beq r, 1` test.** The first build had case 4 a
  switch and case 6 an if/else-if (the round 9 stall's best): case 4 kept
  its own tail, 338/346. **Both as `switch (r) { case 0: ...; case 1:
  ...; }` on an `s16 r` matched**: case 4's `[li 1; beq r, 1; j end]`
  tail cross-jumps into case 6's, and the `case 1` store is `sh $v1` (cse
  stores `r` itself). The round 9 stall declared `r` as `s32`; this is the
  difference worth trying there.

## Declarations

- Prototypes in this `.c`: `RunCardOp1(s16)` (card, no header),
  `CheckCard` as `s32 (void)` (card returns `s16`; the result is
  compared unextended, as in hit_tool's view), and the unit's own
  `WriteEditorSlotFiles`, `CheckEditorSlot`, `SaveEditorState`, `RestoreEditorState`,
  `WriteEditorFile`, `ReadEditorFile`, defined later in the file.
- `config/typeviews-warnings.txt`: one more editor "control reaches
  end of non-void function".

```c
                case 4:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 8;
                            break;
                        case 1:
                            gToolCardStep = 1;
                            break;
                    }
                    break;
                /* ... */
                case 6:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 9;
                            break;
                        case 1:
                            gToolCardStep = 1;
                            break;
                    }
                    break;
```

(The full body is in `src/editor.c`.)

### Proposed learning

- **`bnez r` from one menu case into another case's `beq r, 1` test: both
  cases `switch (r) { case 0: ...; case 1: ...; }` on an `s16 r`**;
  cross-jumping merges the first case's tail into the second's.
  (UpdateEditorData; likely UpdateToolCard's open residue)
