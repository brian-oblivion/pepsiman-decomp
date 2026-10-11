# SaveEditorState — MATCHED 38/38 (round 6, bravo)

> Renamed from `func_800335E8` on 2026-10-11 (tools/rename.py). Address 0x800335e8.

Unit `src/editor.c`. Saves three pieces of game state into the tool
state block `D_80095B28`: three words at 0x348 of `gPlayer` into 0x38,
the halfword `D_800A7680[0].vy` into 0x44, and the first six words of
`gRefView` into 0x48. Then it runs `SaveEditorSlot(a, 0x8016D000)`.
RestoreEditorState is its inverse.

```c
typedef struct {
    u8 unk0[0x38];
    s32 unk38[3];
    s16 unk44;
    u8 unk46[2];
    s32 unk48[6];
} ToolSave;
#define sToolSave (*(ToolSave *)D_80095B28)

typedef struct {
    u8 unk0[0x348];
    s32 unk348[3];
} GameSave;
#define sGameSave (*(GameSave *)gPlayer)

void SaveEditorState(s16 a) {
    sToolSave.unk38[0] = sGameSave.unk348[0];
    sToolSave.unk38[1] = sGameSave.unk348[1];
    sToolSave.unk38[2] = sGameSave.unk348[2];
    sToolSave.unk44 = D_800A7680[0].vy;
    sToolSave.unk48[0] = gRefView[0];
    /* ... [1] to [5] the same way ... */
    sToolSave.unk48[5] = gRefView[5];
    SaveEditorSlot(a, (u8 *)0x8016D000);
}
```

## Levers

- **Word-by-word assignments, not struct copies.** A 12-byte and a
  24-byte struct copy (8/38) come out as cc1's block move: fresh pseudos
  (`a2`/`a3`/`t0` where retail has `a0`/`a1`/`a2`), at most four
  registers per batch, and the whole 24-byte destination reached through
  one `addiu` base. Retail's first `gRefView` word goes through
  `%lo(gRefView)` and the other five through the base, in one batch of
  five registers: per-word assignments, which the scheduler groups. A
  4 + 20 split (s32 then a 20-byte struct) still batched 4 + 1 (3/38).
  Six (and three) separate assignments: 38/38 at once.

## Declarations

The halfword is `D_800A7680[0].vy` (splat names the address `D_800A7682`;
player, object and editor reach it `lui` + `%lo`, `lh`/`lhu`).
At first it was declared `extern s16 D_800A7682[];` in `include/common.h`;
with MoveGamePos, which reaches `D_800A7680` as a base with `vy` at +2,
that line was dropped again and `extern SVECTOR D_800A7680[];` moved from
player.c into `include/player.h` (both units now include it).
Same bytes either way. `ToolSave` and `GameSave` are unit-local views of
`D_80095B28` and `gPlayer`.

### Proposed learning

- **Several words copied with the destination's first word reached by
  `%lo(sym)` and the rest through an `addiu` base, in one batch of more than
  four registers: separate word assignments.** cc1's block move for a struct
  copy uses at most four registers per batch and reaches every word through
  the base. (SaveEditorState, RestoreEditorState)
