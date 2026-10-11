# EditorRunSprites — MATCHED 41/41 (round 6, bravo)

> Renamed from `func_80031A48` on 2026-10-11 (tools/rename.py). Address 0x80031a48.

Unit `src/editor.c`. Walks the 100 StageSprite records at `gStageSprites`;
for each used one (`unk0 != -1`) whose `unk2C` equals the halfword at 0x2A
of `gEditorState` (`sTotals.unk2A`), calls `StepSprite(r->unk0, r)` when
bit 7 of byte `unk26` is set, else `DrawObjectSprite(r->unk0, r)`.

```c
void EditorRunSprites(void) {
    u32 i;
    StageSprite *r;

    for (i = 0; i < 100; i++) {
        r = &gStageSprites[i];
        if (r->unk0 != -1 && sTotals.unk2A == r->unk2C) {
            if (r->unk26 & 0x80) {
                StepSprite(r->unk0, r);
            } else {
                DrawObjectSprite(r->unk0, r);
            }
        }
    }
}
```

First build, using ApplyRecordsToEntries's shape (`r = &tbl[i];` in the body, a
`u32` counter tested with `sltiu`). The duplicated `addiu $s0, 0x3C` in
the `j`'s delay slot is cc1's own.

## Declarations

- Unit-local `StageSprite` gained `u8 unk26` and `s16 unk2C` split out of its
  pad; unit-local `EditorState` gained `u16 unk2A` past its old end (now 0x2C
  bytes; only reached through the `sTotals` macro). Whole image OK.
- `StepSprite` and `DrawObjectSprite` (defined in object, still asm;
  no other C declares them) are prototyped in editor.c as
  `(s16 id, StageSprite *r)`, since `StageSprite` is a editor-local type.
