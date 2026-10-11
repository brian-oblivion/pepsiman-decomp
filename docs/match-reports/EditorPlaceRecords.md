# EditorPlaceRecords — MATCHED 122/122 (round 8, bravo)

> Renamed from `func_80030984` on 2026-10-11 (tools/rename.py). Address 0x80030984.

Unit `src/editor.c`. Counts the live records (`unk36 != -1`) of the
200-entry 0x48-byte table at `gTraps` into `sTotals.unk1A`; on flag bit
5, places each of the `gPlaceCount` waiting records of `gPlaceQueue` at slot
`sTotals.unk18` (stamping position copy, entry, `gSection`, two bytes of
the tool block, and `unk38` = `sTotals.unk1C` for the first, -1 for the
rest), with error 8 in `gToolMode` when the table is full; then clears
`gSubStep` and a set `gEditorInfo`. Fourth build matched.

## Shapes read from retail

- **One counter register (`$t0`) for both loops, tested `sltiu` in the
  first and `slt` against a `u16` global in the second: one `u32` counter,
  compared `(s32)i < gPlaceCount` in the second loop.** Two counters move the
  registers (112/122); one `s32` counter gives `slti` in the first loop
  (121/122).
- The slot count is read once into a `u16` local (`andi 0xFFFF` for the
  `== 200` test, `(s16)` for the index) and written back after each
  placement.
- The first record's `unk38` is stored through the table base
  (`sh ..., 0x38($t4)`), not the walking pointer: `gPlaceQueue[0].unk38`.
- The placement is a 0x48-byte struct copy (four-word loop plus two words),
  then `unk34` stored into the placed slot.

## Declarations

- `EditorState` gained `unk18`, `unk1A`, `unk1C`, and `unk68` / `unk69` past
  its old end (the block is longer than 0x28; its name is now historical).
- A unit-local `EditorTrap` view of the 0x48-byte record (object.h's `Trap`
  is the same record; the head may union them).
- `gSection` (declared in stage_flow.c) and `gSubStep` (declared in
  main.c) moved to `include/common.h` with their types unchanged, since a
  second unit now uses each (declcheck MULTI/LOCAL). This deletes one line in
  each of stage_flow.c and main.c.

```c
void EditorPlaceRecords(void) {
    u32 i;
    u16 n;
    EditorTrap *recs;
    EditorTrap *w;

    recs = (EditorTrap *)gTraps;
    n = sTotals.unk18;
    sTotals.unk1A = 0;
    for (i = 0; i < 200; i++) {
        if (recs[i].unk36 != -1) {
            sTotals.unk1A++;
        }
    }
    if (gPadTrig & 0x20) {
        /* MATCHING: one counter for both loops, unsigned (sltiu) in the
         * first and compared signed (slt) here. */
        for (i = 0; (s32)i < gPlaceCount; i++) {
            w = &gPlaceQueue[i];
            if (n == 200) {
                gToolMode = 8;
                return;
            }
            w->unkC[0] = w->unk0[0];
            w->unkC[1] = w->unk0[1];
            w->unkC[2] = w->unk0[2];
            w->unk40 = 0;
            w->unk24 = 0;
            w->unk26 = 0;
            w->unk28 = gCurBlock;
            w->unk3C = gSection;
            w->unk42 = sTotals.unk68;
            w->unk43 = sTotals.unk69;
            if (i == 0) {
                /* MATCHING: stored through the table base, not w. */
                gPlaceQueue[0].unk38 = sTotals.unk1C;
            } else {
                w->unk38 = -1;
            }
            ((EditorTrap *)gTraps)[(s16)n] = *w;
            ((EditorTrap *)gTraps)[(s16)n].unk34 = sTotals.unk1E;
            n++;
            sTotals.unk18 = n;
        }
        gSubStep = 0;
        if (gEditorInfo == 1) {
            gEditorInfo = 0;
        }
    }
}
```
