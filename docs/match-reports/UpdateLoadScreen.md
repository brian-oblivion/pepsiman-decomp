# UpdateLoadScreen — MATCHED (89/89), round 7, runner bravo

> Renamed from `func_80016FC0` on 2026-10-10 (tools/rename.py). Address 0x80016fc0.

Twin of UpdateAct3LoadScreen: the same three sprites, then `LoadStage`
instead of `LoadAct3Stage`, and when `gSubStep` reaches 100 it also
clears `D_80095980` and calls `PlayStageMusic(gStage)`.

```c
void LoadStage(void);

extern s32 D_80095980;

void UpdateLoadScreen(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    func_8001B354(gFrameCount % 10 + 0xFB, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    LoadStage();
    if (gSubStep == 100) {
        gModeStep = 4;
        D_80095980 = 0;
        gSubStep = 0;
        gGameMode = 14;
        PlayStageMusic(gStage);
    }
}
```

Matched on the first build. The `lbu` of `gStage` that retail prints
above the stores is the scheduler's; the call stays last in the source.
