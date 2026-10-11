#include "common.h"
#include "libetc.h"
#include "memory.h"
#include "libapi.h"
#include "sys/file.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "draw.h"
#include "model.h"
#include "player.h"
#include "object.h"

/** @brief One of the 80 entries heading a record bank: where the entry's
 *         records start. */
typedef struct {
    s32 first; /**< index in the bank's records of the section's first one */
    s32 count; /**< records the section has */
} BankEntry;

/** @brief A point the tool places: a position, a radius and a type. */
typedef struct {
    u8 unk0[0x14]; /**< not yet known */
    s32 radius;    /**< ten times the menu's line-0 value */
    u8 type;       /**< the menu's line-1 value */
    u8 pad19[3];   /**< not yet known */
    s32 x;         /**< the point's x, negated */
    s32 y;         /**< the point's y */
    s32 z;         /**< the point's z, negated */
    s16 section;   /**< the section the point belongs to */
    u8 pad2A[2];   /**< not yet known */
} HitPoint;

/** @brief Three halfwords copied as one 6-byte unit. */
typedef struct {
    s16 x; /**< not yet known */
    s16 y; /**< not yet known */
    s16 z; /**< not yet known */
} Pt6;

/** @brief A collision quad the tool places around a section's owner. */
typedef struct {
    Pt6 pts[4];     /**< copied from the dispatch's object */
    u8 unk18[0x30]; /**< not yet known */
    void *unk48;    /**< the owning ObjectModel's unk10 */
} HitQuad;

/** @brief The point bank: one entry per section, then the points. */
typedef struct {
    BankEntry entries[80]; /**< per-entry start indices */
    HitPoint recs[1];      /**< the records; real count unknown */
} HitPointBank;

/** @brief The quad bank: one entry per section, then the quads. */
typedef struct {
    BankEntry entries[80]; /**< per-entry start indices */
    HitQuad recs[1];       /**< the records; real count unknown */
} HitQuadBank;

HitPoint *GetHitPoint(s32 idx, s32 sub);
HitQuad *GetHitQuad(s32 idx, s32 sub);


extern u8 gToolKind; /**< state of that dispatch: 0 or 1 */

extern s16 gToolSection; /**< current index, clamped to the entry count */

void InsertHitPoint(void);
void PlaceHitQuad(void);

/** @brief Three words, of which the drawing reads the low halves. */
typedef struct {
    s32 x; /**< not yet known */
    s32 y; /**< not yet known */
    s32 z; /**< not yet known */
} Vec3W;

/** @brief An object reset when the two-state dispatch enters state 1. */
typedef struct {
    Pt6 pts[4];     /**< copied into a new second-bank record */
    Vec3W unk18[4]; /**< corner offsets drawn around the owner */
    void *unk48;    /**< points into the current entry's record */
} EditorTrap;

/** @brief What an EditorTrap's unk48 points at, as the drawing reads it. */
typedef struct {
    u8 pad0[0x18]; /**< not reached here */
    VECTOR pos;    /**< the owner's position */
} ObjOwner;

extern EditorTrap gToolNewQuad; /**< reset by the dispatch's state 0 */
extern HitQuad *gToolQuad;      /**< the second-bank record inserted last */
extern HitPoint *gToolPoint;    /**< the first-bank record inserted last */
extern u8 gToolQuadStage;       /**< state of the second dispatch: 0 or 1 */
extern s16 gToolQuadW;          /**< set to 100 on entering state 1 */
extern s16 gToolQuadH;          /**< set to 100 on entering state 1 */

void InsertHitQuad(void);
/* MATCHING: a per-unit view; object defines it as (s16 (*)[3], VECTOR *,
 * s16, s16), a box's corners around a position. */
void SetBoxAround(EditorTrap *obj, u8 *p, s32 a, s32 b);

extern char sHitNewlines2[];  /**< the menu's title, "\n\n" */
extern char sHitColorWhite[]; /**< marker of the highlighted line, "~c888" */
extern char sHitColorBlue[];  /**< marker of the other lines, "~c008" */
extern char sHitDataNoFmt[];  /**< format of one numbered line */


extern u16 gToolRadius; /**< saved value of menu line 0 */
extern u8 gToolType;    /**< saved value of menu line 1 */


extern u8 gToolZoom;
extern u8 gToolViewStep;
extern s16 gToolStep;
extern u8 gToolViewPreset;
extern s16 D_80095A22;
extern s32 gToolPointIndex;
extern s32 D_80095A5C;
extern s32 gToolQuadIndex;
extern s32 D_80095A40;
extern u8 D_80095A58;
extern u8 D_80095A28;

void ClearHitPoints(void);
void ClearHitQuads(void);

extern u8 D_80095774; /**< set while the reset below runs */

extern char sHitTmpFilePath[]; /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL1\\TMP.TL1" */
extern char sHitDataFilePath0[]; /**< path of the first hit-data file, HITDATA0.T1D */
extern char sHitDataFilePath1[]; /**< path of the second hit-data file, HITDATA1.T1D */
extern char sHitDataFilePath2[]; /**< path of the third hit-data file, HITDATA2.T1D */

void MoveToolCursor(void);
void UpdateToolMenu(void);
void PrintToolStatus(void);
void DrawHitQuad(EditorTrap *obj);
void EditHitPoints(void);
void EditHitQuads(void);
void SelectToolView(void);
void UpdateToolView(void);
s32 UpdateToolCard(void);
void PrintToolCardMenu(void);
void StoreHitSlot(s16 n);
s32 WriteHitDataFiles(void);
void LoadHitSlot(s16 n);
void DrawToolPin(VECTOR *pos);
void DrawToolPole(VECTOR *pos);
void DrawToolRadius(VECTOR *pos, u16 scale);
void ToolPlace(void);
s32 CycleToolStep(void);
void SelectToolSection(void);
void RefreshToolPreview(void);
void EditToolValues(void);
void PrintToolSlots(void);
s32 WriteToolFile(void);
s32 ReadToolFile(void);

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

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitSyncFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitTrapFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitGridFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitPosFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitItemText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitBallHitText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitRideHitText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitInfoText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDeleteNowText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitInfoTypeFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitInfoPosFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitNoSupportText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitInfoNormHitText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitInfoRideHitText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitHitSelText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitSizeRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitTypeFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", D_800115DC);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDataModeText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitModeSelectText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitBufferSelectText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitCreateFileText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitCardFormatText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitSaveToDataFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitOverwriteOkText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDataWritingText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitLoadFromDataFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitLoadOkText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitCantOpenDataFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitCompleteText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitErrorText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitNotEnoughMemoryText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitNoCardText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitNoFileText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitCreateNewFileText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitUnformattedText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitFormatOkText);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDataNoFmt);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitTmpFilePath);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDataFilePath0);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDataFilePath1);

INCLUDE_RODATA("asm/nonmatchings/hit_tool", sHitDataFilePath2);

void UpdateTool(void) {
    switch (gModeStep) {
        case 0:
            gWorldShift[0] = 0;
            gWorldShift[2] = 0;
            gSectionCount = 80;
            bzero((u8 *)0x8016D000, 0x10000);
            *(s32 *)0x8016D000 = 80;
            gHitPointSections = 80;
            gHitPoints = 0x8016D004;
            ClearHitPoints();
            bzero((u8 *)0x8017D000, 0x10000);
            *(s32 *)0x8017D000 = 80;
            gHitQuadSections = 80;
            gHitQuads = 0x8017D004;
            ClearHitQuads();
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gToolZoom = 10;
            gToolViewStep = 2;
            gToolStep = 10;
            gToolRadius = 5;
            gToolViewPreset = 0;
            D_80095A22 = 0;
            gToolSection = 0;
            gToolKind = 0;
            gToolPointIndex = 0;
            D_80095A5C = 0;
            gToolQuadIndex = 0;
            D_80095A40 = 0;
            D_80095A58 = 0;
            D_80095A28 = 0;
            gToolMode = 0;
            gMenuLine = 0;
            gEditValue = 0;
            gRepeatButtonsOn = 1;
            gModeStep++;
            break;
        case 1:
            switch ((u16)gToolMode) {
                case 0:
                    MoveToolCursor();
                    ToolPlace();
                    if (gToolKind == 1 && (gPadTrig & PADRdown)) {
                        gToolQuadStage = 0;
                    }
                    break;
                case 1:
                    UpdateToolMenu();
                    break;
                case 2:
                    EditToolValues();
                    break;
                case 3:
                    EditHitPoints();
                    break;
                case 4:
                    gToolQuadStage = 0;
                    gToolMode = 0;
                    break;
                case 5:
                    EditHitQuads();
                    break;
                case 6:
                    break;
                case 7:
                    UpdateToolCard();
                    break;
            }
            PrintToolStatus();
            CycleToolStep();
            RefreshToolPreview();
            if ((u16)(gToolMode - 6) >= 2) {
                SelectToolView();
            }
            UpdateToolView();
            if (gToolKind == 1 && gToolQuadStage == 1) {
                DrawHitQuad(&gToolNewQuad);
                SetBoxAround(&gToolNewQuad, (u8 *)&gPlayer.x, gToolQuadW, gToolQuadH);
            }
            if (D_80095A58 == 0) {
                SelectToolSection();
                DrawToolPin((VECTOR *)&gPlayer.x);
                DrawToolPole((VECTOR *)&gPlayer.x);
                if (gToolKind == 0) {
                    DrawToolRadius((VECTOR *)&gPlayer.x, (s16)gToolRadius * 10);
                }
            }
            break;
    }
}

void MoveToolCursor(void) {
    s32 flags;

    if (gToolQuadStage == 1) {
        flags = gPadTrig;
        if (flags & PADLup) {
            gToolQuadH += gToolStep;
        }
        if (flags & PADLdown) {
            gToolQuadH -= gToolStep;
        }
        if (flags & PADLright) {
            gToolQuadW += gToolStep;
        }
        if (flags & PADLleft) {
            gToolQuadW -= gToolStep;
        }
        gToolQuadW = gToolQuadW < 0 ? 0 : gToolQuadW > 1000 ? 1000 : gToolQuadW;
        gToolQuadH = gToolQuadH < 0 ? 0 : gToolQuadH > 1000 ? 1000 : gToolQuadH;
    } else {
        if (gPadTrig & PADLup) {
            gPlayer.z -= gToolStep;
        }
        if (gPadTrig & PADLdown) {
            gPlayer.z += gToolStep;
        }
        if (gPadTrig & PADLright) {
            gPlayer.x -= gToolStep;
        }
        if (gPadTrig & PADLleft) {
            gPlayer.x += gToolStep;
        }
    }
    if (gStickTrigY & PADLup) {
        gPlayer.y -= gToolStep;
    }
    if (gStickTrigY & PADLdown) {
        gPlayer.y += gToolStep;
    }
    if (gPadTrig & PADRup) {
        gToolQuadStage = 0;
        gToolMode = 1;
    }
}

void UpdateToolMenu(void) {
    gEditMax = 1;
    gMenuLineCount = 5;
    StepMenuCursor();
    if (gPadTrig & PADRright) {
        switch (gMenuLine) {
            case 0:
                gToolKind = gMenuLine;
                gEditMax = 0x33;
                gMenuLineCount = 2;
                gMenuLine = 0;
                gToolMode = 2;
                D_80095A58 = 0;
                D_80095A28 = 0;
                gEditValue = gToolRadius;
                break;
            case 1:
                gToolKind = gMenuLine;
                gMenuLine = 0;
                gEditValue = 0;
                gToolMode = 4;
                D_80095A58 = 0;
                D_80095A28 = 0;
                break;
            case 2:
                D_80095A28 = 0;
                D_80095A58 ^= 1;
            case 3:
                if (gMenuLine == 3) {
                    D_80095A58 = D_80095A28 = D_80095A28 ^ 1;
                }
                gSubStep = 0;
                if (D_80095A58 == 1) {
                    switch (gToolKind) {
                        case 0:
                            gToolMode = 3;
                            break;
                        case 1:
                            gToolMode = 5;
                            break;
                    }
                }
                break;
            case 4:
                gSubStep = 0;
                gToolMode = 7;
                break;
        }
    }
    if (gPadTrig & PADRdown) {
        gToolMode = 0;
    }
}

extern char sHitSyncFmt[];         /**< "SYNC %3d" */
extern char sHitTrapFmt[];         /**< " TRAP %2d" */
extern char sHitGridFmt[];         /**< " GRID %2d" */
extern char sHitPosFmt[];          /**< "\nPOS X %3d  Y %3d  Z %3d" */
extern char sHitItemText[];        /**< "\nITEM : " */
extern char sHitBallHitText[];     /**< "BALL HIT" */
extern char sHitRideHitText[];     /**< "RIDE HIT" */
extern char sHitInfoText[];        /**< " <<< INFO" */
extern char sHitDeleteNowText[];   /**< " <<< !!! DELETE NOW !!!" */
extern char sHitInfoTypeFmt[];     /**< "TYPE:%d\n" */
extern char sHitInfoPosFmt[];      /**< "POS:X %d  Y %d  Z %d\n" */
extern char sHitNoSupportText[];   /**< "NO SUPORT\n" */
extern char sHitInfoNormHitText[]; /**< "NORM HIT\n" */
extern char sHitInfoRideHitText[]; /**< "RIDE HIT\n" */
extern char sHitHitSelText[];      /**< "   HIT  SEL\n\n" */
extern char sHitSizeRangeFmt[];    /**< "SIZE 0<-%3d->500\n" */
extern char sHitTypeFmt[];         /**< "TYPE. %d " */
extern char sHitNewlines6[];       /**< "\n\n\n\n\n\n" */
extern char sHitRadiusFmt[];       /**< "R:%d\n" */
extern char sHitMenuDeleteText[];  /**< "DELETE\n" */
extern char sHitMenuInfoText[];    /**< "INFO\n" */
extern char sHitMenuDataText[];    /**< "DATA\n" */
extern char sHitNewlines5[];       /**< "\n\n\n\n\n" */

void PrintToolStatus(void) {
    FntPrint(sHitSyncFmt, gVSyncTime);
    FntPrint(sHitTrapFmt, gToolSection);
    FntPrint(sHitGridFmt, gToolStep);
    FntPrint(sHitPosFmt, gPlayer.x, gPlayer.y, gPlayer.z);
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
                FntPrint(sHitRadiusFmt, gToolPoint->radius);
                FntPrint(sHitInfoTypeFmt, gToolPoint->type);
                FntPrint(sHitInfoPosFmt, -gToolPoint->x, gToolPoint->y, -gToolPoint->z);
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

/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void InsertHitQuad(void) {
    HitQuadBank *bank;
    BankEntry *e;
    HitQuad *recs;
    HitQuad *src;
    HitQuad *dst;
    HitQuad *rec;
    s32 k;
    s32 n;

    bank = (HitQuadBank *)gHitQuads;
    e = bank->entries;
    e += gToolSection;
    recs = (HitQuad *)&bank->entries[gSectionCount];
    src = &recs[98];
    dst = &recs[99];
    for (k = e->first + e->count; k < 100; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (HitQuadBank *)gHitQuads;
    e = bank->entries;
    e += gToolSection;
    rec = GetHitQuad(gToolSection, e->count);
    gToolQuad = rec;
    for (k = 0; k < 4; k++) {
        rec->pts[k].x = gToolNewQuad.pts[k].x;
        rec->pts[k].y = gToolNewQuad.pts[k].y;
        rec->pts[k].z = gToolNewQuad.pts[k].z;
    }
    gToolQuad->unk48 = gToolNewQuad.unk48;
    e = (BankEntry *)gHitQuads;
    e += gToolSection;
    e->count++;
    n = gSectionCount;
    k = gToolSection + 1;
    goto test;
    do {
        e++;
        e->first++;
        k++;
    test:;
    } while (k < n);
    gToolQuadIndex++;
    ApplyHitQuads();
}

void DrawHitQuad(EditorTrap *obj) {
    VECTOR tmp;
    Pt6 scr[4];
    VECTOR world;
    Pt6 pts[4];
    s32 unused[6];
    s16 i;

    PlaceCollisionQuad(obj);
    world.vx = ((ObjOwner *)obj->unk48)->pos.vx + gWorldShift[0];
    world.vy = ((ObjOwner *)obj->unk48)->pos.vy;
    world.vz = ((ObjOwner *)obj->unk48)->pos.vz + gWorldShift[2];
    for (i = 0; i < 4; i++) {
        pts[i].x = obj->unk18[i].x;
        pts[i].y = obj->unk18[i].y;
        pts[i].z = obj->unk18[i].z;
    }
    tmp.vx = world.vx + pts[0].x;
    tmp.vy = pts[0].y;
    tmp.vz = world.vz + pts[0].z;
    ProjectToScreen(&tmp, (SVECTOR *)&scr[0]);
    tmp.vx = world.vx + pts[1].x;
    tmp.vy = pts[1].y;
    tmp.vz = world.vz + pts[1].z;
    ProjectToScreen(&tmp, (SVECTOR *)&scr[1]);
    tmp.vx = world.vx + pts[3].x;
    tmp.vy = pts[3].y;
    tmp.vz = world.vz + pts[3].z;
    ProjectToScreen(&tmp, (SVECTOR *)&scr[2]);
    tmp.vx = world.vx + pts[2].x;
    tmp.vy = pts[2].y;
    tmp.vz = world.vz + pts[2].z;
    ProjectToScreen(&tmp, (SVECTOR *)&scr[3]);
    SortSemiQuad(0x3E0, scr[0].x, scr[0].y, scr[1].x, scr[1].y, scr[2].x, scr[2].y, scr[3].x,
                 scr[3].y, 0x32);
}

/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void InsertHitPoint(void) {
    HitPointBank *bank;
    BankEntry *e;
    HitPoint *recs;
    HitPoint *src;
    HitPoint *dst;
    HitPoint *rec;
    s32 k;
    s32 n;

    bank = (HitPointBank *)gHitPoints;
    e = bank->entries;
    e += gToolSection;
    recs = (HitPoint *)&bank->entries[gSectionCount];
    src = &recs[398];
    dst = &recs[399];
    for (k = e->first + e->count; k < 400; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (HitPointBank *)gHitPoints;
    e = bank->entries;
    e += gToolSection;
    rec = GetHitPoint(gToolSection, e->count);
    rec->x = -gPlayer.x;
    rec->y = gPlayer.y;
    gToolPoint = rec;
    rec->z = -gPlayer.z;
    rec->radius = (s16)gToolRadius * 10;
    rec->section = gToolSection;
    rec->type = gToolType;
    e = (BankEntry *)gHitPoints;
    e += gToolSection;
    e->count++;
    n = gSectionCount;
    k = gToolSection + 1;
    goto test;
    do {
        e++;
        e->first++;
        k++;
    test:;
    } while (k < n);
    gToolPointIndex++;
    ApplyHitPoints();
}

void EditHitPoints(void) {
    VECTOR pos;
    HitPointBank *bank;
    BankEntry *e;
    HitPoint *src;
    HitPoint *dst;
    HitPoint *rec;
    s32 k;
    s32 n;

    switch (gSubStep) {
        case 0:
            gToolPointIndex = 0;
            gSubStep = 1;
            break;
        case 1:
            bank = (HitPointBank *)gHitPoints;
            e = bank->entries;
            e += gToolSection;
            if (e->count == 0) {
                gToolMode = 0;
                D_80095A58 = 0;
            }
            if (gPadTrig & PADR1) {
                gToolPointIndex++;
            }
            if (gPadTrig & PADL1) {
                gToolPointIndex--;
            }
            e = bank->entries;
            e += gToolSection;
            gToolPointIndex = gToolPointIndex<0 ? 0 : gToolPointIndex>(u32)(e->count - 1)
                                  ? e->count - 1
                                  : gToolPointIndex;
            rec = GetHitPoint(gToolSection, gToolPointIndex);
            pos.vx = -rec->x;
            pos.vy = rec->y;
            gToolPoint = rec;
            pos.vz = -rec->z;
            if (gPadTrig & PADRdown) {
                gToolMode = 0;
                D_80095A58 = 0;
            }
            if (D_80095A28 != 1 && (gPadTrig & PADRright)) {
                bank = (HitPointBank *)gHitPoints;
                e = bank->entries;
                e += gToolSection;
                dst = (HitPoint *)&bank->entries[gSectionCount];
                src = &dst[e->first + gToolPointIndex + 1];
                dst += e->first + gToolPointIndex;
                for (k = e->first + gToolPointIndex; k < 399; k++) {
                    *dst = *src;
                    dst++;
                    /* MATCHING: the empty do/while gives the source pointer
                     * $a1 and the block move's end $a3. */
                    do {
                        src++;
                    } while (0);
                }
                n = gSectionCount;
                /* MATCHING: a byte-pointer store keeps the index load below it. */
                *(s32 *)((u8 *)e + 4) -= 1;
                k = gToolSection + 1;
                for (; k < n; k++) {
                    e++;
                    e->first--;
                }
                ApplyHitPoints();
            }
            DrawToolPin(&pos);
            DrawToolPole(&pos);
            break;
    }
}

void EditHitQuads(void) {
    VECTOR pos;
    HitQuadBank *bank;
    BankEntry *e;
    HitQuad *src;
    HitQuad *dst;
    HitQuad *rec;
    s32 k;
    s32 n;

    switch (gSubStep) {
        case 0:
            gToolQuadIndex = 0;
            gSubStep = 1;
            break;
        case 1:
            bank = (HitQuadBank *)gHitQuads;
            e = bank->entries;
            e += gToolSection;
            if (e->count == 0) {
                gToolMode = 0;
                D_80095A58 = 0;
            }
            if (gPadTrig & PADR1) {
                gToolQuadIndex++;
            }
            if (gPadTrig & PADL1) {
                gToolQuadIndex--;
            }
            e = bank->entries;
            e += gToolSection;
            gToolQuadIndex = gToolQuadIndex<0 ? 0 : gToolQuadIndex>(u32)(e->count - 1)
                                 ? e->count - 1
                                 : gToolQuadIndex;
            rec = GetHitQuad(gToolSection, gToolQuadIndex);
            pos.vx = -rec->pts[0].x;
            pos.vy = rec->pts[0].y;
            gToolQuad = rec;
            pos.vz = -rec->pts[0].z;
            if (gPadTrig & PADRdown) {
                gToolMode = 0;
                D_80095A58 = 0;
            }
            if (D_80095A28 != 1 && (gPadTrig & PADRright)) {
                bank = (HitQuadBank *)gHitQuads;
                e = bank->entries;
                e += gToolSection;
                dst = (HitQuad *)&bank->entries[gSectionCount];
                src = &dst[e->first + gToolQuadIndex + 1];
                dst += e->first + gToolQuadIndex;
                for (k = e->first + gToolQuadIndex; k < 99; k++) {
                    *dst = *src;
                    dst++;
                    /* MATCHING: the empty do/while gives the source pointer
                     * $a1 and the block move's end $a3. */
                    do {
                        src++;
                    } while (0);
                }
                n = gSectionCount;
                /* MATCHING: a byte-pointer store keeps the index load below it. */
                *(s32 *)((u8 *)e + 4) -= 1;
                k = gToolSection + 1;
                for (; k < n; k++) {
                    e++;
                    e->first--;
                }
                ApplyHitQuads();
            }
            DrawToolPin(&pos);
            DrawToolPole(&pos);
            break;
    }
}

/* MATCHING: the first clamp is a preloaded local with nested ifs; a nested
 * ternary like the second one moves the value through another register. */
void SelectToolView(void) {
    s32 flags = gPadTrig;
    u8 v;

    if (flags & PADL2) {
        gToolViewPreset++;
        D_80095A22 = gToolMode;
        gToolMode = 6;
        gToolViewStep = 0;
    }
    if (flags & PADR2) {
        gToolViewPreset--;
        D_80095A22 = gToolMode;
        gToolMode = 6;
        gToolViewStep = 0;
    }
    if (flags & PADRleft) {
        gToolZoom++;
        D_80095A22 = gToolMode;
        gToolMode = 6;
        gToolViewStep = 0;
    }
    if (flags & PADRdown) {
        gToolZoom--;
        D_80095A22 = gToolMode;
        gToolMode = 6;
        gToolViewStep = 0;
    }
    v = gToolZoom;
    if ((s8)gToolZoom > 0) {
        if ((s8)gToolZoom > 20) {
            v = 20;
        }
    } else {
        v = 1;
    }
    gToolZoom = v;
    gToolViewPreset = (s8)gToolViewPreset < 0 ? 4 : (s8)gToolViewPreset > 4 ? 0 : gToolViewPreset;
}

/** @brief Five rows of three words, copied from rodata as one block. */
typedef struct {
    s32 v[5][3]; /**< per-row x, y, z */
} Rows5;

extern Rows5 D_800115DC; /**< the rows the step below scales */
extern s32 D_800CEBA0[]; /**< start of the interpolation */
extern u8 D_80095A20;    /**< interpolation step, 0..30 */

/* MATCHING: a per-unit view; model defines it as (s16, s16, u16, u16)
 * returning s16. */
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

void UpdateToolView(void) {
    Rows5 rows;

    rows = D_800115DC;
    switch (gToolViewStep) {
        case 0:
            gToolViewStep = 1;
            D_80095A20 = 0;
            D_800CEBA0[0] = gRefView[0];
            D_800CEBA0[1] = gRefView[1];
            D_800CEBA0[2] = gRefView[2];
            break;
        case 1:
            gRefView[0] =
                Lerp(D_800CEBA0[0], rows.v[(s8)gToolViewPreset][0] * (s8)gToolZoom, D_80095A20, 30);
            gRefView[1] =
                Lerp(D_800CEBA0[1], rows.v[(s8)gToolViewPreset][1] * (s8)gToolZoom, D_80095A20, 30);
            gRefView[2] =
                Lerp(D_800CEBA0[2], rows.v[(s8)gToolViewPreset][2] * (s8)gToolZoom, D_80095A20, 30);
            if (++D_80095A20 == 30) {
                gToolViewStep++;
            }
            break;
        case 2:
            gToolMode = D_80095A22;
            gToolViewStep++;
            break;
        case 3:
            gRefView[0] = rows.v[(s8)gToolViewPreset][0] * (s8)gToolZoom;
            gRefView[1] = rows.v[(s8)gToolViewPreset][1] * (s8)gToolZoom;
            gRefView[2] = rows.v[(s8)gToolViewPreset][2] * (s8)gToolZoom;
            gRefView[3] = 0;
            gRefView[4] = 0;
            gRefView[5] = 0;
            break;
    }
    gRefView[6] = 0;
}

/* MATCHING: a per-unit view; card defines it returning s16, and
 * UpdateToolCard compares the result with -1 unextended. */
s32 CheckCard(void);

/* MATCHING: s32 with no return keeps the error table's index shift in its
 * bound check's delay slot. */
s32 UpdateToolCard(void) {
    s32 err = 0;
    s32 r;

    switch (gSubStep) {
        case 0:
            OpenCardEvents();
            gToolCardStep = 0;
            gMenuLine = 0;
            gEditValue = 0;
            gSubStep++;
            break;
        case 1:
            switch (gToolCardStep) {
                case 0:
                    D_80095A14 = 0;
                    err = RunCardOp2(0);
                    if (err == 0) {
                        gToolCardStep = 1;
                    }
                    break;
                case 1:
                    if (CheckCard() == -1 || (gPadTrig & PADRdown)) {
                        gToolCardStep = 18;
                        break;
                    }
                    gMenuChoice = RunMenu(2);
                    switch (gMenuChoice) {
                        case 2:
                            if (WriteToolFile() == -1) {
                                gToolCardStep = 11;
                            } else if (WriteHitDataFiles() == -1) {
                                gToolCardStep = 11;
                            } else {
                                gToolCardStep = 10;
                            }
                            break;
                        case 3:
                            if (ReadToolFile() == -1) {
                                gToolCardStep = 11;
                            } else {
                                gMenuChoice = 1;
                                gToolCardStep = 2;
                            }
                            break;
                        case -1:
                            break;
                        default:
                            gMenuLine = 0;
                            gToolCardStep = 2;
                            break;
                    }
                    break;
                case 2:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = 1;
                        break;
                    }
                    gToolSlot = RunMenu(3);
                    if (gToolSlot == -1) {
                        break;
                    }
                    switch (gMenuChoice) {
                        case 0:
                            gToolCardStep = 3;
                            break;
                        case 1:
                            gToolCardStep = 5;
                            break;
                    }
                    break;
                case 10:
                    if (gPadTrig & (PADRdown | PADRright)) {
                        gToolCardStep = 1;
                    }
                    break;
                /* MATCHING: before case 4, so its block merges into 4's. */
                case 3:
                    gToolCardStep = 8;
                    break;
                case 4:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 8;
                            break;
                        case 1:
                            gToolCardStep = 1;
                            break;
                    }
                    break;
                case 8:
                    StoreHitSlot(gToolSlot);
                    err = RunCardOp2(1);
                    if (err == 0) {
                        gToolCardStep = 10;
                    }
                    break;
                case 5:
                    gToolCardStep = 6;
                    gMenuLine = 0;
                    break;
                case 6:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 9;
                            break;
                        case 1:
                            gToolCardStep = 1;
                            break;
                    }
                    break;
                case 7:
                    if (gPadTrig & PADRright) {
                        gToolCardStep = 5;
                    }
                    break;
                case 9:
                    LoadHitSlot(gToolSlot);
                    ApplyHitPoints();
                    ApplyHitQuads();
                    gToolCardStep = 10;
                    break;
                case 11:
                case 13:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = 18;
                    }
                    break;
                case 14:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 16;
                            break;
                        case 1:
                            gToolCardStep = 18;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case 15:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 17;
                            break;
                        case 1:
                            gToolCardStep = 18;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case 16:
                    err = RunCardOp2(2);
                    if (err == 0) {
                        gToolCardStep = 0;
                    }
                    break;
                case 17:
                    err = RunCardOp2(3);
                    if (err == 0) {
                        gToolCardStep = 0;
                    }
                    break;
                case 18:
                    gSubStep++;
                    break;
            }
            switch (err) {
                case -2:
                case -1:
                    gToolCardStep = 11;
                    break;
                case 1:
                    gToolCardStep = 12;
                    break;
                case 2:
                    gToolCardStep = 13;
                    break;
                case 3:
                    gToolCardStep = 14;
                    break;
                case 4:
                    gToolCardStep = 15;
                    break;
            }
            break;
        case 2:
            CloseCardEvents();
            gToolMode = 0;
            break;
    }
}

extern char sHitNewlines3[];           /**< a line prefix, "\n\n\n" */
extern char sHitCheckText[];           /**< not yet known, "CHECK" */
extern char sHitDataModeText[];        /**< "DATA MODE" */
extern char sHitModeSelectText[];      /**< "  MODE SELECT\n\n" */
extern char sHitBufferSelectText[];    /**< "BUFFER SELECT\n\n" */
extern char sHitCreateFileText[];      /**< "  CREATE FILE" */
extern char sHitCardFormatText[];      /**< "  CARD FORMAT" */
extern char sHitSaveToDataFmt[];       /**< "SAVE TO DATA %d\n\n" */
extern char sHitOverwriteOkText[];     /**< "OVER WRITE OK ?" */
extern char sHitDataWritingText[];     /**< "DATA WRITING..." */
extern char sHitLoadFromDataFmt[];     /**< "LOAD TO DATA %d\n\n" */
extern char sHitLoadOkText[];          /**< "    LOAD OK ?" */
extern char sHitCantOpenDataFmt[];     /**< "CAN NOT OPEN DATA %d" */
extern char sHitCompleteText[];        /**< "   COMPLATE" */
extern char sHitErrorText[];           /**< "    !!!! ERROR!!!!" */
extern char sHitNotEnoughMemoryText[]; /**< "   NOT ENOUGH MEMORY" */
extern char sHitNoCardText[];          /**< "NO CONNECT CARD" */
extern char sHitNoFileText[];          /**< "NO FILE\n\n" */
extern char sHitCreateNewFileText[];   /**< "CREATE NEW FILE ?" */
extern char sHitUnformattedText[];     /**< "    UN FORMAT\n\n" */
extern char sHitFormatOkText[];        /**< "   FORMAT OK ?" */

void PrintToolCardMenu(void) {
    PrintTabs(5);
    FntPrint(sHitDataModeText);
    FntPrint(sHitNewlines3);
    switch (gToolCardStep) {
        case 0:
            FntPrint(sHitNewlines3);
            PrintTabs(4);
            FntPrint(sHitCheckText);
            break;
        case 1:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitModeSelectText);
            PrintSaveLoad();
            break;
        case 2:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitBufferSelectText);
            PrintToolSlots();
            break;
        case 16:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCreateFileText);
            break;
        case 17:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCardFormatText);
            break;
        case 4:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitSaveToDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sHitOverwriteOkText);
            PrintYesNo();
            break;
        case 8:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitDataWritingText);
            break;
        case 6:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitLoadFromDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sHitLoadOkText);
            PrintYesNo();
            break;
        case 7:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitCantOpenDataFmt, gToolSlot + 1);
            break;
        case 10:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCompleteText);
            break;
        case 11:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitErrorText);
            break;
        case 12:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitNotEnoughMemoryText);
            break;
        case 13:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitNoCardText);
            break;
        case 14:
            FntPrint(sHitNewlines3);
            PrintTabs(4);
            FntPrint(sHitNoFileText);
            PrintTabs(3);
            FntPrint(sHitCreateNewFileText);
            PrintYesNo();
            break;
        case 15:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitUnformattedText);
            PrintTabs(3);
            FntPrint(sHitFormatOkText);
            PrintYesNo();
            break;
    }
    SortSemiQuad(0x421, -100, -60, 100, -60, 100, 60, -100, 60, 0);
}

/** @brief The first block of one hit-data slot. */
typedef struct {
    u8 b[0x4744]; /**< not yet known */
} HitBlockA;

/** @brief The second block of one hit-data slot. */
typedef struct {
    u8 b[0x2034]; /**< not yet known */
} HitBlockB;

/** @brief One hit-data slot: the two blocks a HITDATA file holds. */
typedef struct {
    HitBlockA a; /**< the first block */
    HitBlockB b; /**< the second block */
} HitSlot;

/** @brief The tool buffer: a header, then the three hit-data slots. */
typedef struct {
    u8 hdr[0x200];    /**< not yet known */
    HitSlot slots[3]; /**< one per HITDATA file */
} ToolBuf;

void StoreHitSlot(s16 n) {
    ToolBuf *tool = (ToolBuf *)0x8018D000;

    tool->slots[n].a = *(HitBlockA *)0x8016D000;
    (&tool->slots[n])->b = *(HitBlockB *)0x8017D000;
}

s32 WriteHitDataFiles(void) {
    s32 fd;

    fd = open(sHitDataFilePath0, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x8018D200, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x80191944, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    fd = open(sHitDataFilePath1, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x80193978, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x801980BC, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    fd = open(sHitDataFilePath2, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x8019A0F0, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x8019E834, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    return 0;
fail:
    close(fd);
    return -1;
}

/* MATCHING: -pos->vx + ... loads the parameter's word before the global's. */
void DrawToolPin(VECTOR *pos) {
    SVECTOR size;
    VECTOR world;
    SVECTOR screen;
    s32 bob;

    world.vx = -pos->vx + gWorldShift[0];
    bob = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 10) >> 12) - 200;
    world.vy = pos->vy + bob;
    world.vz = -pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    size.vx = screen.vx - 7;
    size.vy = screen.vy - 32;
    DrawSprite(0x15E, &size, 0, 5, &gSubOt[gActiveBuff]);
}

/* MATCHING: an s16 colour local keeps 0xFF loaded after the sine. */
void DrawToolPole(VECTOR *pos) {
    s32 unused[2];
    VECTOR world;
    SVECTOR screen;
    GsLINE line;
    s16 g;

    g = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = -pos->vx + gWorldShift[0];
    world.vy = pos->vy;
    world.vz = -pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vy = pos->vy - 200;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
}

/** @brief A record of the live buffer the first bank is copied into. */
typedef struct {
    u8 unk0[0x28]; /**< copied from the bank record */
    void *unk28;   /**< the owning record's unk10 */
} Live2C;

void *__builtin_memcpy(void *dst, const void *src, u32 n);

/* MATCHING: __builtin_memcpy, for a word-aligned 42-byte block move. */
void ApplyHitPoints(void) {
    s16 i;
    BankEntry *e = ((HitPointBank *)gHitPoints)->entries;
    HitPoint *rec;

    for (i = 0; i < (u32)gHitPointSections; i++) {
        gObjectModels[i].unk72 = e->first;
        gObjectModels[i].unk74 = e->count;
        e++;
    }
    for (i = 0; i < 400; i++) {
        rec = GetHitPoint(0, i);
        __builtin_memcpy(&((Live2C *)gLivePoints)[i], rec, 0x2A);
        ((Live2C *)gLivePoints)[i].unk28 = gObjectModels[rec->section].unk10;
    }
}

void ApplyHitQuads(void) {
    s16 i;
    s16 j;
    BankEntry *e = ((HitQuadBank *)gHitQuads)->entries;

    for (i = 0; i < (u32)gHitQuadSections; i++) {
        gObjectModels[i].unk6E = e->first;
        gObjectModels[i].unk70 = e->count;
        e++;
    }
    for (i = 0; i < 80; i++) {
        for (j = gObjectModels[i].unk6E; j < gObjectModels[i].unk70 + gObjectModels[i].unk6E; j++) {
            GetHitQuad(0, j)->unk48 = gObjectModels[i].unk10;
        }
    }
    for (i = 0; i < 100; i++) {
        ((HitQuad *)gLiveQuads)[i] = *GetHitQuad(0, i);
    }
}

HitPoint *GetHitPoint(s32 idx, s32 sub) {
    HitPointBank *bank = (HitPointBank *)gHitPoints;
    HitPoint *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}

HitQuad *GetHitQuad(s32 idx, s32 sub) {
    HitQuadBank *bank = (HitQuadBank *)gHitQuads;
    HitQuad *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}

/* MATCHING: the chained assignment stores its rightmost target first. */
void DrawToolRadius(VECTOR *pos, u16 scale) {
    SVECTOR size;
    CVECTOR color;
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = -pos->vx;
    coord.coord.t[1] = pos->vy;
    coord.coord.t[2] = -pos->vz;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
    size.vx = size.vy = scale * 2;
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    DrawBillboard(0x15D, &size, &color, 2, &gSubOt[gActiveBuff]);
}

/* MATCHING: (&slot)->b puts the second address sum offset-first. */
void LoadHitSlot(s16 n) {
    ToolBuf *tool = (ToolBuf *)0x8018D000;

    *(HitBlockA *)0x8016D000 = tool->slots[n].a;
    *(HitBlockB *)0x8017D000 = (&tool->slots[n])->b;
}

void ResetTool(void) {
    gWorldShift[0] = 0;
    gWorldShift[2] = 0;
    gSectionCount = 80;
    bzero((u8 *)0x8016D000, 0x10000);
    *(s32 *)0x8016D000 = 80;
    gHitPointSections = 80;
    gHitPoints = 0x8016D004;
    ClearHitPoints();
    bzero((u8 *)0x8017D000, 0x10000);
    *(s32 *)0x8017D000 = 80;
    gHitQuadSections = 80;
    gHitQuads = 0x8017D004;
    ClearHitQuads();
    gPlayer.x = 0;
    gPlayer.y = 0;
    gPlayer.z = 0;
    gToolZoom = 10;
    gToolViewStep = 2;
    gToolStep = 10;
    gToolViewPreset = 0;
    D_80095A22 = 0;
    gToolSection = 0;
    gToolKind = 0;
    gToolPointIndex = 0;
    D_80095A5C = 0;
    gToolQuadIndex = 0;
    D_80095A40 = 0;
    gToolRadius = 5;
    D_80095A58 = 0;
    D_80095A28 = 0;
}

void ClearHitPoints(void) {
    u32 i;

    bzero(gLivePoints, 0x44C0);
    for (i = 0; i < 80; i++) {
        gObjectModels[i].unk72 = 0;
        gObjectModels[i].unk74 = -1;
    }
}

void ClearHitQuads(void) {
    u32 i;

    bzero(gLiveQuads, 0x1DB0);
    for (i = 0; i < 80; i++) {
        gObjectModels[i].unk6E = 0;
        gObjectModels[i].unk70 = -1;
    }
}

void ToolPlace(void) {
    if (gPadTrig & PADRright) {
        switch (gToolKind) {
            case 0:
                InsertHitPoint();
                break;
            case 1:
                PlaceHitQuad();
                break;
        }
    }
}

void PlaceHitQuad(void) {
    switch (gToolQuadStage) {
        case 0:
            gToolQuadW = 100;
            gToolQuadH = 100;
            gToolNewQuad.unk48 = gObjectModels[gToolSection].unk10;
            SetBoxAround(&gToolNewQuad, (u8 *)&gPlayer.x, 100, 100);
            gToolQuadStage = 1;
            break;
        case 1:
            InsertHitQuad();
            gToolQuadStage = 0;
            break;
    }
}

/* MATCHING: s32 with no return; the jump table lands 4 bytes late. */
s32 CycleToolStep(void) {
    if (gPadTrig & PADselect) {
        switch (gToolStep) {
            case 10:
                gToolStep = 20;
                break;
            case 20:
                gToolStep = 30;
                break;
            case 30:
                gToolStep = 40;
                break;
            case 40:
                gToolStep = 50;
                break;
            case 50:
                gToolStep = 10;
                break;
        }
    }
}

void SelectToolSection(void) {
    s32 flags = gPadTrig;

    if (flags & PADR1) {
        gToolSection++;
    }
    if (flags & PADL1) {
        gToolSection--;
    }
    gToolSection = gToolSection < 0                   ? 0
                   : gToolSection > gSectionCount - 1 ? gSectionCount - 1
                                                      : gToolSection;
}

void RefreshToolPreview(void) {
    D_80095774 = 1;
    ResetCollision();
    gTraps[0] = 0;
    gTraps[1] = 0;
    gTraps[2] = 0;
    ((s16 *)gTraps)[12] = 0;
    ((s16 *)gTraps)[13] = 0;
    ((s16 *)gTraps)[14] = 0;
    gTraps[11] = 0;
    gTraps[12] = 0;
    UpdateObjectEntry(&gObjectModels[gToolSection], (Trap *)gTraps);
    ReactToHits(&gPlayer);
    func_80029838();
    D_80095774 = 0;
}

/* MATCHING: the unused 8 bytes give the 0x20-byte frame. */
void EditToolValues(void) {
    s32 unused[2];
    s16 line = gMenuLine;

    StepMenuCursor();
    if (line != gMenuLine) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 0x33;
                gEditValue = gToolRadius;
                break;
            case 1:
                gEditMax = 0x100;
                gEditValue = gToolType;
                break;
        }
    } else {
        switch (line) {
            case 0:
                gToolRadius = gEditValue;
                break;
            case 1:
                gToolType = gEditValue;
                break;
        }
    }
    if (gPadTrig & PADRdown) {
        gToolMode = 0;
    }
}

/* MATCHING: one call per arm; a ternary argument shares one %hi register. */
void PrintToolSlots(void) {
    s16 i;

    FntPrint(sHitNewlines2);
    for (i = 0; i < 3; i++) {
        PrintTabs(4);
        if (gMenuLine == i) {
            FntPrint(sHitColorWhite);
        } else {
            FntPrint(sHitColorBlue);
        }
        FntPrint(sHitDataNoFmt, i + 1);
    }
}

s32 WriteToolFile(void) {
    s32 fd;
    s32 n;

    fd = open(sHitTmpFilePath, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}

s32 ReadToolFile(void) {
    s32 fd;
    s32 n;

    fd = open(sHitTmpFilePath, O_RDONLY);
    n = read(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}
