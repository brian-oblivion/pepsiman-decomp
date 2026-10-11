# LoadEditorSlot — MATCHED 175/175 (round 9, alpha)

> Renamed from `func_80032C28` on 2026-10-11 (tools/rename.py). Address 0x80032c28.

Unit `src/editor.c`. Restores save slot `a` of the tool buffer
`buf` (0xB774-byte slots): five block moves into the 0x5C-byte record
table `gSwitches`, the EditorTrap table `gTraps`, the StageSprite table
`gStageSprites`, the block header area at `0x801FD000` and the tool state
block `gEditorState`. Fourth build matched.

## Shapes read from retail

- **`move a3, a0` at entry, the first index extended in place in `a0`
  and every later one from `a3` with its own `sll`/`sra`: an `s16`
  parameter.** `s32 a` with `(s16)a` at each use extends into another
  register and drops the copy (44/175). Both callers pass an `s16` or a
  constant and are unchanged.
- The slot layout, read from the copies: the slot data starts 0x200 bytes
  into the existing `SaveSlot` view (its tag byte `unk200` is the tool
  block's first byte), so the 0x800-byte block area at 0xB174 runs 0x200
  bytes past the view's 0xB774 size. It is reached by byte arithmetic.
- **An offset above 0x7FFF built whole (`ori v1, 0x8294; addu`): byte
  arithmetic on the slot pointer**, `(u8 *)&slot + 0x8294`. A member
  access (`slot.unk8294`) splits it into `li 0x8000` and `0x294` offsets
  on the start and end pointers (76/175).
- The block-area copy tests the alignment (`lwl`/`lwr` path): a
  `u8 b[0x800]` wrapper. The other four are `s32 w[N/4]` wrappers.

## Declarations

- New unit-local copy wrappers `EditorStateCopy`, `TrapTable`, `SwitchTable`,
  `SpriteTable`, `BlockAreaCopy`; `SaveSlot` gains `unk203[0x71]` and the
  three table members (size unchanged, 0xB774).
- `extern s32 gSwitches[];` in the `.c` (MATCHING note; object has
  its SwitchBox view).
- CHANGED existing declaration (unit-local): `LoadEditorSlot`'s prototype
  in editor.c, `s32 a` to `s16 a`.

```c
void LoadEditorSlot(s16 a, u8 *buf) {
    *(SwitchTable *)gSwitches = ((SaveSlot *)buf)[a].unk3AB4;
    *(TrapTable *)gTraps = ((SaveSlot *)buf)[a].unk274;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(SpriteTable *)gStageSprites = *(SpriteTable *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294);
    *(BlockAreaCopy *)0x801FD000 = *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174);
    *(EditorStateCopy *)gEditorState = *(EditorStateCopy *)&((SaveSlot *)buf)[a].unk200;
}
```

### Proposed learning

- An offset above 0x7FFF added whole (`ori; addu`) before a block move's
  start and end pointers: byte arithmetic on the base pointer; a member
  access splits it into `0x8000` plus the remainder. (LoadEditorSlot)
- A parameter copied to another register at entry, extended in place for
  the first use and again from the copy for every later one: an `s16`
  parameter. (LoadEditorSlot)
