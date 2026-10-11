# UpdateEditor — MATCHED 520/520 (round 10, echo)

> Renamed from `func_8002D424` on 2026-10-11 (tools/rename.py). Address 0x8002d424.

Unit `src/editor.c`, its first function. The tool's main step, by
`gModeStep`: state 0 resets the tool buffer (`ResetGameState`), clears the
error code `gToolMode` and sets the tool up; state 1 marks the 127 entry
flags `gCanGroupOn[1..127]`, then by `gToolMode` runs the current mode's
editors (mode 0 picks by `sTotals.unk3` between the four placers, or with
`D_800958D8` set the four pickers, and on flag bit 5 loads the picked
EditorTrap or StageSprite record into the menu editor's state), the menus of modes
1..5, the data mode (`UpdateEditorData`) or the message screens; then moves
the camera and the view (`DrawPin`, `UpdatePickups`,
`DrawStage`, `UpdateCamera`, ...) and draws the caption
(`PrintEditorStatus`). Flag bit 11 leaves the tool (state 3); flag bit 8 steps
the move distance `sTotals.unk4` (50 to 100, 100 to 2, 1000 to 10, any
other to 50). About 20 builds.

## Shapes read from retail

- Frame 0x30 with no stack local: `s32 unused[4];`.
- `lhu` switch on the `s16` `gToolMode`: `switch ((u16)gToolMode)`
  (LEARNINGS).
- Retail's case bodies in the order 0, 1, 2, 3, 4, 5, 6, (7, 8 share 2..5's
  `& 0x40` tail), 9..11; mode 1's "clear `gSubStep` when line 2" jumps
  into the shared `gToolMode = 0` store.
- In the two record loaders the stores of the error code and the menu
  globals sit between the record reads; the source order that matched is
  retail's store order (`gToolMode = 4;` before `sTotals.unk1E = ...`;
  for the StageSprite one: unk2C, `gToolMode = 5`, unk2A, unk6D, unk6C,
  `gEditMax`, `D_800958D8`, `gMenuLineCount`, `gEditValue`, `gMenuLine`,
  `gSubStep`, unk6A).
- **`li a1, 100` in the delay slot of the `& 0x100` test, a `nop` left in
  the load delay below: `s32` with no return.** Void let reorg fill the
  slot with the following `lui $v0`; a live `$v0` blocks that and the
  next candidate is the constant. The forward-branch case of the LEARNINGS
  entry. (441 to 514/520.)
- **The flag loop `tbl[sTotals.unk18 + i].unk36 = -1` with its two loop
  copies in swapped registers (`move t0, s1; move a3, s0` in retail):**
  - indexing the cast table directly: copies swapped (516/520);
  - `r = (EditorTrap *)gTraps;` before the loop: registers right but
    `li a0, 1` hoisted above the count store (514/520); before the call:
    427; in the `for` init either way round: 514; `i = 1; r = ...; for
    (;...)`: 514; a second counter: 507; `i + unk18`: 515; `(u16)` index:
    192; `&tbl[unk18])[i]`: 186;
  - **`r = (EditorTrap *)gTraps;` as the first statement of the loop body:
    520/520.**

## Declarations

- `EditorState`: `u8 unk6E[2]` and `s32 unk70` appended (the stamp saved on
  leaving the tool; was `*(s32 *)(gEditorState + 0x70)` in the first build).
  `GameSave`: `unk354[0x70]` split into `unk354[0x6C]` + `s32 unk3C0`.
  Unit-local; whole oracle green.
- Prototypes in this `.c` for functions of other units without (or not
  in) a header: `ResetGameState(Trap *)`, `ShowRevealedCans`, `TestBoxes`,
  `ResetStageObjects`, `ResetStageOverlay`, `DrawSky`,
  `DrawStage(s16, s16, s16)`, `UpdateCamera(s16, s16, s16, s16)`, and
  `UpdatePickups(GsOT *)` with a `MATCHING:` note (object's stall body
  takes no parameter; the call passes `&gSubOt[gActiveBuff]`). Forward
  prototypes for the unit's own later functions.
- `extern u8 gCanGroupOn[];` (also declared in object.c), `extern s32
  gRetries;`, `extern u8 gToolExited;`.
- **declcheck hits left for the head:** MULTI `gCanGroupOn` (object.c
  and this unit; belongs in a shared header); LOCAL `ShowRevealedCans`,
  `ResetGameState` (belong in object.h), `ResetStageOverlay` (stage_flow.c's
  local prototype, now also used here; belongs in a header for
  stage_overlay).
- `config/typeviews-warnings.txt`: one more editor "control reaches
  end of non-void function" and one more "unused variable `unused'".

```c
                                    for (i = 1; i < gPlaceCount; i++) {
                                        r = (EditorTrap *)gTraps;
                                        r[sTotals.unk18 + i].unk36 = -1;
                                    }
```

(The full body is in `src/editor.c`.)

### Proposed learning

- **Two loop-invariant base copies in swapped registers: take the base
  into a pointer local as the first statement of the loop body**; before
  the loop it fixes the registers but lets the counter's init move up.
  (UpdateEditor)
