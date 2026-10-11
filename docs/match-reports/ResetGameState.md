# ResetGameState — MATCHED 89/89 (round 8, alpha)

> Renamed from `func_8002D2C0` on 2026-10-11 (tools/rename.py). Address 0x8002d2c0.

Unit `src/object.c`. The full game-state reset: clears the
0x30000-byte block at `recs` and marks its 200 Trap records free
(`unk34`, `unk36` = -1), clears and rebuilds the block header at
0x801FD000 (BuildEmptyBlock), clears and resets the SwitchBox, Trap and StageSprite
tables (editor's resets), and resets the progress block at
`gEditorState` and three globals. Matched on the first build.

```c
void ResetGameState(Trap *recs) {
    u32 i;

    bzero((u8 *)recs, 0x30000);
    for (i = 0; i < 200; i++) {
        recs->unk34 = -1;
        recs->unk36 = -1;
        recs++;
    }
    bzero((u8 *)0x801FD000, 0x800);
    BuildEmptyBlock((u8 *)0x801FD000);
    bzero((u8 *)gSwitches, sizeof(SwitchBox) * 200);
    ClearSwitches(gSwitches);
    bzero((u8 *)sRecs48, sizeof(Trap) * 200);
    ResetTraps(sRecs48);
    bzero((u8 *)gStageSprites, sizeof(StageSprite) * 100);
    ClearStageSprites(gStageSprites);
    sEditorReset.unk0 = 0x38;
    sEditorReset.unk1 = 1;
    sEditorReset.unk68 = 0;
    sEditorReset.unk69 = 0;
    sEditorReset.unk4 = 50;
    sEditorReset.unk8 = 0;
    sEditorReset.unkC = 0;
    sEditorReset.unk12[8] = 0;
    sEditorReset.unk12[9] = 0;
    sEditorReset.unk12[10] = 0;
    sEditorReset.unk12[0] = 0;
    sEditorReset.unk12[1] = 0;
    sEditorReset.unk12[2] = 0;
    sEditorReset.unk12[3] = 0;
    sEditorReset.unk12[4] = 0;
    sEditorReset.unk12[6] = 0;
    sEditorReset.unk12[5] = 0;
    sEditorReset.unk12[7] = 0;
    sEditorReset.unk12[11] = 0;
    sEditorReset.unk12[12] = 0;
    sEditorReset.unk12[13] = 0;
    sEditorReset.unk12[14] = 600;
    sEditorReset.unk12[15] = 0;
    sEditorReset.unk12[16] = 0;
    sEditorReset.unk12[17] = 0;
    sEditorReset.unk6A = 0;
    sEditorReset.unk6C = 0;
    sEditorReset.unk6D = 0;
    gCurBlock = 0;
    gEditorPicking = 0;
    gEditorInfo = 0;
    sEditorReset.unk2 = gStage;
}
```

The editor state block is a unit-local `EditorReset` view through
`#define sEditorReset (*(EditorReset *)gEditorState)` (retail stores the first
byte through `%lo(gEditorState)`, then builds the base and stores the rest
off it: a struct lvalue). Stores are in retail's order; the copy of
`gStage` comes last, its load hoisted by the scheduler.

## Declarations (need the head)

New in this unit: `extern StageSprite gStageSprites[]`, `extern u8
gEditorState[]`, `gCurBlock` (s32), `gEditorPicking` and `gEditorInfo` (u8;
the last reached `$gp`, already in config/gp/object.txt), and
prototypes of editor's BuildEmptyBlock, ClearSwitches, ResetTraps
and ClearStageSprites, under one `MATCHING:` per-unit-view note (StageSprite is
local to each unit).

declcheck now reports LOCAL on editor.c's own declarations of
gStageSprites, gEditorState, BuildEmptyBlock and ClearStageSprites (plus
DrawObjectSprite and StepSprite from StepSprite). The lasting fix is
one shared header for the two units: StageSprite (the union of both views),
those two externs and the prototypes in object.h, with the local
copies deleted from both .c files. editor was bravo's this round, so
this is left to the merge.
