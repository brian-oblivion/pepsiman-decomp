# RestoreEditorState — MATCHED 38/38 (round 6, bravo)

> Renamed from `func_800336F8` on 2026-10-11 (tools/rename.py). Address 0x800336f8.

Unit `src/code_1dc24.c`. The inverse of SaveEditorState: runs
`LoadEditorSlot(a, 0x8016D000)`, then copies the three words at 0x38 of the
tool state block `D_80095B28` back to 0x348 of `D_8009EB78`, the halfword
at 0x44 to `D_800A7680[0].vy`, and the six words at 0x48 to `gRefView[0..5]`.

```c
void RestoreEditorState(s16 a) {
    LoadEditorSlot(a, (u8 *)0x8016D000);
    sGameSave.unk348[0] = sToolSave.unk38[0];
    sGameSave.unk348[1] = sToolSave.unk38[1];
    sGameSave.unk348[2] = sToolSave.unk38[2];
    D_800A7680[0].vy = sToolSave.unk44;
    gRefView[0] = sToolSave.unk48[0];
    /* ... [1] to [5] the same way ... */
    gRefView[5] = sToolSave.unk48[5];
}
```

(`ToolSave`/`GameSave` as in SaveEditorState's report.)

## Levers

Same as SaveEditorState: struct copies give block moves (0/38 with a
4 + 20 split, one word long with a 24-byte copy, which shifted the whole
image); separate word assignments give retail's batches of three and five
registers and the `%lo(gRefView)` store of the first word. 38/38.
