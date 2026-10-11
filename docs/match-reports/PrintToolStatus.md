# PrintToolStatus — MATCHED 273/273 (round 9, echo)

> Renamed from `func_80034788` on 2026-10-11 (tools/rename.py). Address 0x80034788.

Unit `src/hit_tool.c`. Prints the tool's status panel (counter, current
entry, step, the game state's position, the dispatch states, the record
inserted last), then by error code `gToolMode` either a five-line menu
(1), a two-line value editor (2) with its frame drawn by `SortSemiQuad`,
or the help panel `PrintToolCardMenu` (7).

```c
extern s32 gVSyncTime; /**< printed first on the status panel */
extern char D_800114DC[];
extern char D_800114E8[];
extern char D_800114F4[];
extern char D_80011500[];
extern char D_8001151C[];
extern char D_80011528[];
extern char D_80011534[];
extern char D_80011540[];
extern char D_8001154C[];
extern char D_80011564[];
extern char D_80011570[];
extern char D_80011588[];
extern char D_80011594[];
extern char D_800115A0[];
extern char D_800115AC[];
extern char D_800115BC[];
extern char D_800115D0[];
extern char D_80095658[];
extern char D_80095660[];
extern char D_80095678[];
extern char D_80095680[];
extern char D_80095688[];
extern char D_80095690[];

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
    FntPrint(D_800114DC, gVSyncTime);
    FntPrint(D_800114E8, gToolSection);
    FntPrint(D_800114F4, gToolStep);
    FntPrint(D_80011500, sGamePos.unk348, sGamePos.unk34C, sGamePos.unk350);
    FntPrint(D_8001151C);
    switch (gToolKind) {
        case 0:
            FntPrint(D_80011528);
            break;
        case 1:
            FntPrint(D_80011534);
            break;
    }
    if (D_80095A28 == 1) {
        FntPrint(D_80011540);
    } else if (D_80095A58 == 1) {
        FntPrint(D_8001154C);
    }
    FntPrint(D_80095658);
    if (D_80095A28 == 1) {
        switch (gToolKind) {
            case 0:
                FntPrint(D_80095660, gToolPoint->unk14);
                FntPrint(D_80011564, gToolPoint->unk18);
                FntPrint(D_80011570, -gToolPoint->unk1C, gToolPoint->unk20, -gToolPoint->unk24);
                break;
            case 1:
                FntPrint(D_80011588);
                break;
        }
    }
    switch ((u16)gToolMode) {
        case 1:
            D_80095A58 = 0;
            D_80095A28 = 0;
            if (D_8009574A == 0) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            PrintTabs(7);
            FntPrint(D_80011594);
            if (D_8009574A == 1) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            PrintTabs(7);
            FntPrint(D_800115A0);
            if (D_8009574A == 2) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            PrintTabs(7);
            FntPrint(D_80095678);
            if (D_8009574A == 3) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            PrintTabs(7);
            FntPrint(D_80095680);
            if (D_8009574A == 4) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            PrintTabs(7);
            FntPrint(D_80095688);
            SortSemiQuad(0x2821, 60, -68, 140, -68, 140, 12, 60, 12, 0);
            break;
        case 2:
            FntPrint(D_80095690);
            PrintTabs(3);
            FntPrint(D_800115AC);
            PrintTabs(3);
            if (D_8009574A == 0) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            FntPrint(D_800115BC, (s16)gToolRadius * 10);
            PrintTabs(3);
            if (D_8009574A == 1) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            FntPrint(D_800115D0, gToolType);
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
