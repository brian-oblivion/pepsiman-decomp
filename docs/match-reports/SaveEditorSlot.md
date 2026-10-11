# SaveEditorSlot — MATCHED 177/177 (round 9, alpha)

> Renamed from `func_80032964` on 2026-10-11 (tools/rename.py). Address 0x80032964.

Unit `src/code_1dc24.c`. Saves the five areas LoadEditorSlot restores into
save slot `a` of the tool buffer `buf`: the 0x5C-byte record table, the
Obj48 table, the Rec3C table, the block header area at `0x801FD000` and the
tool state block. First build matched, written as LoadEditorSlot's mirror.

## Shapes read from retail

- The same two levers as LoadEditorSlot (an `s16` parameter; byte
  arithmetic for the 0x8294 and 0xB174 offsets, built whole with `ori`).
  The block-area copy tests the alignment of the destination this time.

## Declarations

- CHANGED existing declaration (unit-local): `SaveEditorSlot`'s prototype
  in code_1dc24.c, `s32 a` to `s16 a`; its callers are unchanged.

```c
void SaveEditorSlot(s16 a, u8 *buf) {
    ((SaveSlot *)buf)[a].unk3AB4 = *(Rec5CsCopy *)D_800CF080;
    ((SaveSlot *)buf)[a].unk274 = *(Obj48sCopy *)D_800A9008;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(Rec3CsCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294) = *(Rec3CsCopy *)D_800A7898;
    *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174) = *(BlockAreaCopy *)0x801FD000;
    *(TotalsCopy *)&((SaveSlot *)buf)[a].unk200 = *(TotalsCopy *)D_80095B28;
}
```
