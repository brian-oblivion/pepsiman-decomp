# ResetGameState — MATCHED 89/89 (round 8, alpha)

> Renamed from `func_8002D2C0` on 2026-10-11 (tools/rename.py). Address 0x8002d2c0.

Unit `src/object.c`. The full game-state reset: clears the
0x30000-byte block at `recs` and marks its 200 Trap records free
(`unk34`, `unk36` = -1), clears and rebuilds the block header at
0x801FD000 (BuildEmptyBlock), clears and resets the SwitchBox, Trap and StageSprite
tables (editor's resets), and resets the progress block at
`D_80095B28` and three globals. Matched on the first build.

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
    sProgress.unk0 = 0x38;
    sProgress.unk1 = 1;
    sProgress.unk68 = 0;
    sProgress.unk69 = 0;
    sProgress.unk4 = 50;
    sProgress.unk8 = 0;
    sProgress.unkC = 0;
    sProgress.unk12[8] = 0;
    sProgress.unk12[9] = 0;
    sProgress.unk12[10] = 0;
    sProgress.unk12[0] = 0;
    sProgress.unk12[1] = 0;
    sProgress.unk12[2] = 0;
    sProgress.unk12[3] = 0;
    sProgress.unk12[4] = 0;
    sProgress.unk12[6] = 0;
    sProgress.unk12[5] = 0;
    sProgress.unk12[7] = 0;
    sProgress.unk12[11] = 0;
    sProgress.unk12[12] = 0;
    sProgress.unk12[13] = 0;
    sProgress.unk12[14] = 600;
    sProgress.unk12[15] = 0;
    sProgress.unk12[16] = 0;
    sProgress.unk12[17] = 0;
    sProgress.unk6A = 0;
    sProgress.unk6C = 0;
    sProgress.unk6D = 0;
    gCurBlock = 0;
    D_800958D8 = 0;
    D_800959D8 = 0;
    sProgress.unk2 = gStage;
}
```

The progress block is a unit-local `Progress6E` view through
`#define sProgress (*(Progress6E *)D_80095B28)` (retail stores the first
byte through `%lo(D_80095B28)`, then builds the base and stores the rest
off it: a struct lvalue). Stores are in retail's order; the copy of
`gStage` comes last, its load hoisted by the scheduler.

## Declarations (need the head)

New in this unit: `extern StageSprite gStageSprites[]`, `extern u8
D_80095B28[]`, `gCurBlock` (s32), `D_800958D8` and `D_800959D8` (u8;
the last reached `$gp`, already in config/gp/object.txt), and
prototypes of editor's BuildEmptyBlock, ClearSwitches, ResetTraps
and ClearStageSprites, under one `MATCHING:` per-unit-view note (StageSprite is
local to each unit).

declcheck now reports LOCAL on editor.c's own declarations of
gStageSprites, D_80095B28, BuildEmptyBlock and ClearStageSprites (plus
DrawObjectSprite and StepSprite from StepSprite). The lasting fix is
one shared header for the two units: StageSprite (the union of both views),
those two externs and the prototypes in object.h, with the local
copies deleted from both .c files. editor was bravo's this round, so
this is left to the merge.
