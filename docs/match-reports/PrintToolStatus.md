# PrintToolStatus — MATCHED 273/273 (round 9, echo)

> Renamed from `func_80034788` on 2026-10-11 (tools/rename.py). Address 0x80034788.

Unit `src/hit_tool.c`. Prints the tool's status panel (counter, current
entry, step, the game state's position, the dispatch states, the record
inserted last), then by error code `gToolMode` either a five-line menu
(1), a two-line value editor (2) with its frame drawn by `SortSemiQuad`,
or the help panel `PrintToolCardMenu` (7).

```c
extern s32 gVSyncTime; /**< printed first on the status panel */
extern char sHitSyncFmt[];
extern char sHitTrapFmt[];
extern char sHitGridFmt[];
extern char sHitPosFmt[];
extern char sHitItemText[];
extern char sHitBallHitText[];
extern char sHitRideHitText[];
extern char sHitInfoText[];
extern char sHitDeleteNowText[];
extern char sHitInfoTypeFmt[];
extern char sHitInfoPosFmt[];
extern char sHitNoSupportText[];
extern char sHitInfoNormHitText[];
extern char sHitInfoRideHitText[];
extern char sHitHitSelText[];
extern char sHitSizeRangeFmt[];
extern char sHitTypeFmt[];
extern char sHitNewlines6[];
extern char sHitRadiusFmt[];
extern char sHitMenuDeleteText[];
extern char sHitMenuInfoText[];
extern char sHitMenuDataText[];
extern char sHitNewlines5[];

void PrintToolStatus(void);
void DrawHitQuad(Obj48 *obj);
void EditHitPoints(void);
void EditHitQuads(void);
void SelectToolView(void);
void UpdateToolView(void);
void UpdateToolCard(void);
void DrawToolPin(VECTOR *pos);
void DrawToolPole(VECTOR *pos);
void DrawToolRadius(VECTOR *pos, u16 scale);
void ToolPlace(void);
s32 CycleToolStep(void);
void SelectToolSection(void);
void RefreshToolPreview(void);
void EditToolValues(void);

/* MATCHING: the bob gets its own statement, or cc1 adds -200 to pos->vy. */
void DrawPin(VECTOR *pos) {
    SVECTOR size;
    VECTOR world;
    SVECTOR screen;
    s32 bob;

    world.vx = pos->vx + gWorldShift[0];
    bob = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 10) >> 12) - 200;
    world.vy = pos->vy + bob;
    world.vz = pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    size.vx = screen.vx - 7;
    size.vy = screen.vy - 32;
    DrawSprite(0x15E, &size, 0, 5, &gSubOt[gActiveBuff]);
}
void PrintToolStatus(void) {
    FntPrint(sHitSyncFmt, gVSyncTime);
    FntPrint(sHitTrapFmt, gToolSection);
    FntPrint(sHitGridFmt, gToolStep);
    FntPrint(sHitPosFmt, sGamePos.unk348, sGamePos.unk34C, sGamePos.unk350);
    FntPrint(sHitItemText);
    switch (gToolKind) {
        case 0:
            FntPrint(sHitBallHitText);
            break;
        case 1:
            FntPrint(sHitRideHitText);
            break;
    }
    if (D_80095A28 == 1) {
        FntPrint(sHitInfoText);
    } else if (D_80095A58 == 1) {
        FntPrint(sHitDeleteNowText);
    }
    FntPrint(sHitNewlines6);
    if (D_80095A28 == 1) {
        switch (gToolKind) {
            case 0:
                FntPrint(sHitRadiusFmt, gToolPoint->unk14);
                FntPrint(sHitInfoTypeFmt, gToolPoint->unk18);
                FntPrint(sHitInfoPosFmt, -gToolPoint->unk1C, gToolPoint->unk20, -gToolPoint->unk24);
                break;
            case 1:
                FntPrint(sHitNoSupportText);
                break;
        }
    }
    switch ((u16)gToolMode) {
        case 1:
            D_80095A58 = 0;
            D_80095A28 = 0;
            if (gMenuLine == 0) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            PrintTabs(7);
            FntPrint(sHitInfoNormHitText);
            if (gMenuLine == 1) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            PrintTabs(7);
            FntPrint(sHitInfoRideHitText);
            if (gMenuLine == 2) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            PrintTabs(7);
            FntPrint(sHitMenuDeleteText);
            if (gMenuLine == 3) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            PrintTabs(7);
            FntPrint(sHitMenuInfoText);
            if (gMenuLine == 4) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            PrintTabs(7);
            FntPrint(sHitMenuDataText);
            SortSemiQuad(0x2821, 60, -68, 140, -68, 140, 12, 60, 12, 0);
            break;
        case 2:
            FntPrint(sHitNewlines5);
            PrintTabs(3);
            FntPrint(sHitHitSelText);
            PrintTabs(3);
            if (gMenuLine == 0) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            FntPrint(sHitSizeRangeFmt, (s16)gToolRadius * 10);
            PrintTabs(3);
            if (gMenuLine == 1) {
                FntPrint(sHitColorWhite);
            } else {
                FntPrint(sHitColorBlue);
            }
            FntPrint(sHitTypeFmt, gToolType);
            SortSemiQuad(0x2821, -80, -30, 80, -30, 80, 30, -80, 30, 0);
            break;
        case 7:
            PrintToolCardMenu();
            break;
    }
}
```

## Levers

First build matched. The highlighted-line markers are one `FntPrint` per
arm (`if (line == k) FntPrint(A); else FntPrint(B);`, LEARNINGS'
PrintToolSlots shape); cross-jumping leaves one call at the join.

## Declarations

`gVSyncTime` (`s32`) is declared in this `.c` and in `src/main.c`;
declcheck lists it MULTI. It belongs in `common.h` at merge.
