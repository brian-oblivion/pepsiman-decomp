# SaveEditorSlot — MATCHED 177/177 (round 9, alpha)

> Renamed from `func_80032964` on 2026-10-11 (tools/rename.py). Address 0x80032964.

Unit `src/editor.c`. Saves the five areas LoadEditorSlot restores into
save slot `a` of the tool buffer `buf`: the 0x5C-byte record table, the
EditorTrap table, the StageSprite table, the block header area at `0x801FD000` and the
tool state block. First build matched, written as LoadEditorSlot's mirror.

## Shapes read from retail

- The same two levers as LoadEditorSlot (an `s16` parameter; byte
  arithmetic for the 0x8294 and 0xB174 offsets, built whole with `ori`).
  The block-area copy tests the alignment of the destination this time.

## Declarations

- CHANGED existing declaration (unit-local): `SaveEditorSlot`'s prototype
  in editor.c, `s32 a` to `s16 a`; its callers are unchanged.

```c
void SaveEditorSlot(s16 a, u8 *buf) {
    ((SaveSlot *)buf)[a].unk3AB4 = *(SwitchTable *)gSwitches;
    ((SaveSlot *)buf)[a].unk274 = *(TrapTable *)gTraps;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(SpriteTable *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294) = *(SpriteTable *)gStageSprites;
    *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174) = *(BlockAreaCopy *)0x801FD000;
    *(EditorStateCopy *)&((SaveSlot *)buf)[a].unk200 = *(EditorStateCopy *)gEditorState;
}
```
