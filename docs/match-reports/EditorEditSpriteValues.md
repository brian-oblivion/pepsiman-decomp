# EditorEditSpriteValues — MATCHED 217/217 (round 10, echo)

> Renamed from `func_8003146C` on 2026-10-11 (tools/rename.py). Address 0x8003146c.

Unit `src/code_1dc24.c`. The StageSprite template's menu editor, the nine-line
sibling of EditorEditRecordValues: saves the highlighted line, runs
`StepMenuCursor`, then after a line change loads the new line's value
(`gEditValue`) and limit (`gEditMax`) from the tool block, otherwise
stores the edited value back. Steps the angle `sTotals.unk6A` on flag bits
3 and 2 of `gPad` (wrapping 0..359), turns `D_8009EEC0` to it when
`sTotals.unk6C` is set, then rebuilds the template `sSpriteTemplate` at the game
position, takes two heights from the kind's eight-byte table entry
(`GetQuad16`), and runs one of the two StageSprite handlers. Fifth build
matched.

## Shapes read from retail

- Two nine-case jump tables, `switch (gMenuLine)` after a change and
  `switch (line)` otherwise; EditorEditRecordValues's layout unchanged. Case 6 moves
  the low byte of `gEditValue` to `sSpriteFlags` (`$gp`).
- The angle is read `lh` for the call and compared after `sll`/`sra`: an
  `s16` field.
- `lbu 0x16($sp)` / `lbu 0x17($sp)` after `GetQuad16(.., $sp+0x10)`:
  bytes 6 and 7 of an eight-byte local copied whole.
- **`addiu -0x1F4` then `addu`, where the build gave `li 0xFE0C` and an
  `addu`:** written inline, `u16 + (u8 - 500)` stored to an `s16` is
  narrowed by fold to a halfword add, whose constant cc1 loads as an
  unsigned halfword. An `(s32)` cast does not stop it, nor does making the
  `u16` operand `s16`. Each offset in its own `s32` local does
  (`h0 = e.b[6] - 500; ... = sTotals.unk2E + h0;`). One local shared by the
  three uses was 164/217: it took `$a2` and the second arm's sum merged into
  the first's.
- `void` and `s32` with no return both match; kept `void`.

## Declarations

- `EditorState`: `u8 unk2E[2]` became `u16 unk2E`; `u8 unk34[0x34]` became
  `u16 unk34` + `u8 unk36[0x32]`; `unk6A` became `s16` (was `u16`; the
  only other user, EditorPlaceSprite, still matches).
- `StageSprite`: `u8 unk1C[0xA]` became `s16 unk1C`, `s16 unk1E`, `s32 unk20`,
  `u8 unk24[2]`.
- New unit-local `Bytes8` (`u8 b[8]`).
- `GetQuad16` declared in this `.c` as `(s32 i, Bytes8 *out)` with a
  `MATCHING:` note (object defines `(u16, Quad16 *)`; retail passes the
  index unextended). `DrawHeadingLine` (defined later in this unit)
  prototyped at the top.
- All unit-local; whole oracle green.

```c
void EditorEditSpriteValues(void) {
    s16 line;
    Bytes8 e;
    s32 h0;
    s32 h1;
    s32 h2;

    line = gMenuLine;
    StepMenuCursor();
    if (line != gMenuLine) {
        switch (gMenuLine) { /* nine cases: limit, then value */ }
    } else {
        switch (line) { /* nine cases: store back */ }
    }
    /* angle step on bits 3 and 2, DrawHeadingLine on sTotals.unk6C */
    sSpriteTemplate.unk4[0] = sGameSave.unk348[0];
    sSpriteTemplate.unk4[1] = sGameSave.unk348[1];
    sSpriteTemplate.unk4[2] = sGameSave.unk348[2];
    GetQuad16(sTotals.unk2C + 30, &e);
    if (sTotals.unk34 == 0) {
        h0 = e.b[6] - 500;
        sSpriteTemplate.unk1C = sTotals.unk2E + h0;
    } else {
        h1 = e.b[6] - 500;
        sSpriteTemplate.unk1C = -(sTotals.unk2E + h1);
    }
    h2 = e.b[7] - 500;
    sSpriteTemplate.unk1E = sTotals.unk2E + h2;
    sSpriteTemplate.unk20 = sSpriteTemplate.unk1C;
    if (sTotals.unk32 == 0) {
        DrawObjectSprite(sTotals.unk2C + 30, &sSpriteTemplate);
    } else {
        StepSprite(sTotals.unk2C + 30, &sSpriteTemplate);
    }
}
```

(The full body is in `src/editor.c`.)

### Proposed learning

- **`addiu -K` before an `addu` into a halfword store, where the build
  loads `li 0x10000-K`: the offset was its own `s32` local, one per use.**
  Inline, `u16 + (u8 - K)` stored to an `s16` is narrowed to a halfword add.
  (EditorEditSpriteValues)
