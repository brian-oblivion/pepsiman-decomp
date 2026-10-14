#include "common.h"
#include "sbss.h"
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

/** @brief What the hit tool places, gToolKind: the first two lines of its menu. */
enum ToolKind {
    TK_BALL = 0, /**< "BALL HIT": HitPoint spheres */
    TK_RIDE = 1  /**< "RIDE HIT": HitQuad boxes */
};

/** @brief The hit tool's modes, gToolMode. */
enum ToolMode {
    TM_MOVE = 0,       /**< moves the cursor and places */
    TM_MENU = 1,       /**< the menu */
    TM_BALL_SEL = 2,   /**< "HIT SEL": a ball hit's size and type */
    TM_PICK_POINT = 3, /**< DELETE or INFO on ball hits */
    TM_RIDE_START = 4, /**< starts a new ride hit, then back to TM_MOVE */
    TM_PICK_QUAD = 5,  /**< DELETE or INFO on ride hits */
    TM_VIEW_MOVE = 6,  /**< the view moves to a preset */
    TM_DATA = 7        /**< "DATA MODE": the memory card screen */
};

extern u8 gToolKind; /**< what is placed, a ToolKind */

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


extern u16 gToolRadius; /**< saved value of menu line 0 */
extern u8 gToolType;    /**< saved value of menu line 1 */


extern u8 gToolZoom;
extern u8 gToolViewStep;
extern s16 gToolStep;
extern u8 gToolViewPreset;
extern s16 sToolModeSaved; /**< gToolMode, kept while the view moves to a preset */
extern s32 gToolPointIndex;
extern s32 D_80095A5C;
extern s32 gToolQuadIndex;
extern s32 D_80095A40;
extern u8 sToolPickMode; /**< 1 while a record is being picked, for DELETE or INFO */
extern u8 sToolInfoMode; /**< 1 while the picked record's INFO is shown */

void ClearHitPoints(void);
void ClearHitQuads(void);


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
    DrawSprite(SPR_POINTER, &size, 0, 5, &gSubOt[gActiveBuff]);
}

/* The hit tool's status line and its point/quad info panel. */
const char sHitSyncFmt[] = "SYNC %3d";
const char sHitTrapFmt[] = " TRAP %2d";
const char sHitGridFmt[] = " GRID %2d";
const char sHitPosFmt[] = "\nPOS X %3d  Y %3d  Z %3d";
const char sHitItemText[] = "\nITEM : ";
const char sHitBallHitText[] = "BALL HIT";
const char sHitRideHitText[] = "RIDE HIT";
const char sHitInfoText[] = " <<< INFO";
const char sHitDeleteNowText[] = " <<< !!! DELETE NOW !!!";
const char sHitInfoTypeFmt[] = "TYPE:%d\n";
const char sHitInfoPosFmt[] = "POS:X %d  Y %d  Z %d\n";
const char sHitNoSupportText[] = "NO SUPORT\n";
const char sHitInfoNormHitText[] = "NORM HIT\n";
const char sHitInfoRideHitText[] = "RIDE HIT\n";
const char sHitHitSelText[] = "   HIT  SEL\n\n";
const char sHitSizeRangeFmt[] = "SIZE 0<-%3d->500\n";
const char sHitTypeFmt[] = "TYPE. %d ";

/** @brief Five rows of three words, copied from rodata as one block. */
typedef struct {
    s32 v[5][3]; /**< per-row x, y, z */
} Rows5;

/** The view presets the move goes to (x, y, z), scaled by the zoom. */
const Rows5 sViewPresets = {{
    {0, 0, -100},
    {100, 0, 0},
    {0, 0, 100},
    {-100, 0, 0},
    {0, -100, 0},
}};

/* The hit tool's memory-card data menu (PrintToolCardMenu). */
const char sHitDataModeText[] = "DATA MODE";
const char sHitModeSelectText[] = "  MODE SELECT\n\n";
const char sHitBufferSelectText[] = "BUFFER SELECT\n\n";
const char sHitCreateFileText[] = "  CREATE FILE";
const char sHitCardFormatText[] = "  CARD FORMAT";
const char sHitSaveToDataFmt[] = "SAVE TO DATA %d\n\n";
const char sHitOverwriteOkText[] = "OVER WRITE OK ?";
const char sHitDataWritingText[] = "DATA WRITING...";
const char sHitLoadFromDataFmt[] = "LOAD TO DATA %d\n\n";
const char sHitLoadOkText[] = "    LOAD OK ?";
const char sHitCantOpenDataFmt[] = "CAN NOT OPEN DATA %d";
const char sHitCompleteText[] = "   COMPLATE";
const char sHitErrorText[] = "    !!!! ERROR!!!!";
const char sHitNotEnoughMemoryText[] = "   NOT ENOUGH MEMORY";
const char sHitNoCardText[] = "NO CONNECT CARD";
const char sHitNoFileText[] = "NO FILE\n\n";
const char sHitCreateNewFileText[] = "CREATE NEW FILE ?";
const char sHitUnformattedText[] = "    UN FORMAT\n\n";
const char sHitFormatOkText[] = "   FORMAT OK ?";

/** Format of one numbered line of the data-file list. */
const char sHitDataNoFmt[] = "DATA %d\n";

/* The host (sim:) files the tool saves its working buffer and its three
 * hit-data slots to. */
const char sHitTmpFilePath[] = "sim:\\PS\\PEPSI\\DATA\\TOOL1\\TMP.TL1";
const char sHitDataFilePath0[] = "sim:\\PS\\PEPSI\\DATA\\TOOL1\\HITDATA0.T1D";
const char sHitDataFilePath1[] = "sim:\\PS\\PEPSI\\DATA\\TOOL1\\HITDATA1.T1D";
const char sHitDataFilePath2[] = "sim:\\PS\\PEPSI\\DATA\\TOOL1\\HITDATA2.T1D";

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
            sToolModeSaved = 0;
            gToolSection = 0;
            gToolKind = TK_BALL;
            gToolPointIndex = 0;
            D_80095A5C = 0;
            gToolQuadIndex = 0;
            D_80095A40 = 0;
            sToolPickMode = 0;
            sToolInfoMode = 0;
            gToolMode = TM_MOVE;
            gMenuLine = 0;
            gEditValue = 0;
            gRepeatButtonsOn = 1;
            gModeStep++;
            break;
        case 1:
            switch ((u16)gToolMode) {
                case TM_MOVE:
                    MoveToolCursor();
                    ToolPlace();
                    if (gToolKind == TK_RIDE && (gPadTrig & PADRdown)) {
                        gToolQuadStage = 0;
                    }
                    break;
                case TM_MENU:
                    UpdateToolMenu();
                    break;
                case TM_BALL_SEL:
                    EditToolValues();
                    break;
                case TM_PICK_POINT:
                    EditHitPoints();
                    break;
                case TM_RIDE_START:
                    gToolQuadStage = 0;
                    gToolMode = TM_MOVE;
                    break;
                case TM_PICK_QUAD:
                    EditHitQuads();
                    break;
                case TM_VIEW_MOVE:
                    break;
                case TM_DATA:
                    UpdateToolCard();
                    break;
            }
            PrintToolStatus();
            CycleToolStep();
            RefreshToolPreview();
            if ((u16)(gToolMode - TM_VIEW_MOVE) >= 2) { /* neither TM_VIEW_MOVE nor TM_DATA */
                SelectToolView();
            }
            UpdateToolView();
            if (gToolKind == TK_RIDE && gToolQuadStage == 1) {
                DrawHitQuad(&gToolNewQuad);
                SetBoxAround(&gToolNewQuad, (u8 *)&gPlayer.x, gToolQuadW, gToolQuadH);
            }
            if (sToolPickMode == 0) {
                SelectToolSection();
                DrawToolPin((VECTOR *)&gPlayer.x);
                DrawToolPole((VECTOR *)&gPlayer.x);
                if (gToolKind == TK_BALL) {
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
        gToolMode = TM_MENU;
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
                gToolMode = TM_BALL_SEL;
                sToolPickMode = 0;
                sToolInfoMode = 0;
                gEditValue = gToolRadius;
                break;
            case 1:
                gToolKind = gMenuLine;
                gMenuLine = 0;
                gEditValue = 0;
                gToolMode = TM_RIDE_START;
                sToolPickMode = 0;
                sToolInfoMode = 0;
                break;
            case 2:
                sToolInfoMode = 0;
                sToolPickMode ^= 1;
            case 3:
                if (gMenuLine == 3) {
                    sToolPickMode = sToolInfoMode = sToolInfoMode ^ 1;
                }
                gSubStep = 0;
                if (sToolPickMode == 1) {
                    switch (gToolKind) {
                        case TK_BALL:
                            gToolMode = TM_PICK_POINT;
                            break;
                        case TK_RIDE:
                            gToolMode = TM_PICK_QUAD;
                            break;
                    }
                }
                break;
            case 4:
                gSubStep = 0;
                gToolMode = TM_DATA;
                break;
        }
    }
    if (gPadTrig & PADRdown) {
        gToolMode = TM_MOVE;
    }
}

extern char sHitNewlines6[];      /**< "\n\n\n\n\n\n" */
extern char sHitRadiusFmt[];      /**< "R:%d\n" */
extern char sHitMenuDeleteText[]; /**< "DELETE\n" */
extern char sHitMenuInfoText[];   /**< "INFO\n" */
extern char sHitMenuDataText[];   /**< "DATA\n" */
extern char sHitNewlines5[];      /**< "\n\n\n\n\n" */

void PrintToolStatus(void) {
    FntPrint(sHitSyncFmt, gVSyncTime);
    FntPrint(sHitTrapFmt, gToolSection);
    FntPrint(sHitGridFmt, gToolStep);
    FntPrint(sHitPosFmt, gPlayer.x, gPlayer.y, gPlayer.z);
    FntPrint(sHitItemText);
    switch (gToolKind) {
        case TK_BALL:
            FntPrint(sHitBallHitText);
            break;
        case TK_RIDE:
            FntPrint(sHitRideHitText);
            break;
    }
    if (sToolInfoMode == 1) {
        FntPrint(sHitInfoText);
    } else if (sToolPickMode == 1) {
        FntPrint(sHitDeleteNowText);
    }
    FntPrint(sHitNewlines6);
    if (sToolInfoMode == 1) {
        switch (gToolKind) {
            case TK_BALL:
                FntPrint(sHitRadiusFmt, gToolPoint->radius);
                FntPrint(sHitInfoTypeFmt, gToolPoint->type);
                FntPrint(sHitInfoPosFmt, -gToolPoint->x, gToolPoint->y, -gToolPoint->z);
                break;
            case TK_RIDE:
                FntPrint(sHitNoSupportText);
                break;
        }
    }
    switch ((u16)gToolMode) {
        case TM_MENU:
            sToolPickMode = 0;
            sToolInfoMode = 0;
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
        case TM_BALL_SEL:
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
        case TM_DATA:
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
                gToolMode = TM_MOVE;
                sToolPickMode = 0;
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
                gToolMode = TM_MOVE;
                sToolPickMode = 0;
            }
            if (sToolInfoMode != 1 && (gPadTrig & PADRright)) {
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
                gToolMode = TM_MOVE;
                sToolPickMode = 0;
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
                gToolMode = TM_MOVE;
                sToolPickMode = 0;
            }
            if (sToolInfoMode != 1 && (gPadTrig & PADRright)) {
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
        sToolModeSaved = gToolMode;
        gToolMode = TM_VIEW_MOVE;
        gToolViewStep = 0;
    }
    if (flags & PADR2) {
        gToolViewPreset--;
        sToolModeSaved = gToolMode;
        gToolMode = TM_VIEW_MOVE;
        gToolViewStep = 0;
    }
    if (flags & PADRleft) {
        gToolZoom++;
        sToolModeSaved = gToolMode;
        gToolMode = TM_VIEW_MOVE;
        gToolViewStep = 0;
    }
    if (flags & PADRdown) {
        gToolZoom--;
        sToolModeSaved = gToolMode;
        gToolMode = TM_VIEW_MOVE;
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

extern s32 sViewLerpFrom[]; /**< where the view move started, x y z */
extern u8 sViewLerpStep;    /**< the view move's step, 0..30 */

/* MATCHING: a per-unit view; model defines it as (s16, s16, u16, u16)
 * returning s16. */
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

void UpdateToolView(void) {
    Rows5 rows;

    rows = sViewPresets;
    switch (gToolViewStep) {
        case 0:
            gToolViewStep = 1;
            sViewLerpStep = 0;
            sViewLerpFrom[0] = gRefView[0];
            sViewLerpFrom[1] = gRefView[1];
            sViewLerpFrom[2] = gRefView[2];
            break;
        case 1:
            gRefView[0] = Lerp(sViewLerpFrom[0], rows.v[(s8)gToolViewPreset][0] * (s8)gToolZoom,
                               sViewLerpStep, 30);
            gRefView[1] = Lerp(sViewLerpFrom[1], rows.v[(s8)gToolViewPreset][1] * (s8)gToolZoom,
                               sViewLerpStep, 30);
            gRefView[2] = Lerp(sViewLerpFrom[2], rows.v[(s8)gToolViewPreset][2] * (s8)gToolZoom,
                               sViewLerpStep, 30);
            if (++sViewLerpStep == 30) {
                gToolViewStep++;
            }
            break;
        case 2:
            gToolMode = sToolModeSaved;
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
    s32 err = CARD_OK;
    s32 r;

    switch (gSubStep) {
        case 0:
            OpenCardEvents();
            gToolCardStep = CS_CHECK;
            gMenuLine = 0;
            gEditValue = 0;
            gSubStep++;
            break;
        case 1:
            switch (gToolCardStep) {
                case CS_CHECK:
                    gCardNew = 0;
                    err = RunCardOp2(CARD_OP_READ);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_MODE_SELECT;
                    }
                    break;
                case CS_MODE_SELECT:
                    if (CheckCard() == -1 || (gPadTrig & PADRdown)) {
                        gToolCardStep = CS_EXIT;
                        break;
                    }
                    gMenuChoice = RunMenu(2);
                    switch (gMenuChoice) {
                        case 2:
                            if (WriteToolFile() == -1) {
                                gToolCardStep = CS_ERROR;
                            } else if (WriteHitDataFiles() == -1) {
                                gToolCardStep = CS_ERROR;
                            } else {
                                gToolCardStep = CS_COMPLETE;
                            }
                            break;
                        case 3:
                            if (ReadToolFile() == -1) {
                                gToolCardStep = CS_ERROR;
                            } else {
                                gMenuChoice = 1;
                                gToolCardStep = CS_SLOT_SELECT;
                            }
                            break;
                        case -1:
                            break;
                        default:
                            gMenuLine = 0;
                            gToolCardStep = CS_SLOT_SELECT;
                            break;
                    }
                    break;
                case CS_SLOT_SELECT:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = CS_MODE_SELECT;
                        break;
                    }
                    gToolSlot = RunMenu(3);
                    if (gToolSlot == -1) {
                        break;
                    }
                    switch (gMenuChoice) {
                        case 0:
                            gToolCardStep = CS_SAVE_CHECK;
                            break;
                        case 1:
                            gToolCardStep = CS_LOAD_CHECK;
                            break;
                    }
                    break;
                case CS_COMPLETE:
                    if (gPadTrig & (PADRdown | PADRright)) {
                        gToolCardStep = CS_MODE_SELECT;
                    }
                    break;
                /* MATCHING: before CS_SAVE_ASK, so its block merges into that one. */
                case CS_SAVE_CHECK:
                    gToolCardStep = CS_WRITE;
                    break;
                case CS_SAVE_ASK:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_WRITE;
                            break;
                        case 1:
                            gToolCardStep = CS_MODE_SELECT;
                            break;
                    }
                    break;
                case CS_WRITE:
                    StoreHitSlot(gToolSlot);
                    err = RunCardOp2(CARD_OP_REWRITE);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_COMPLETE;
                    }
                    break;
                case CS_LOAD_CHECK:
                    gToolCardStep = CS_LOAD_ASK;
                    gMenuLine = 0;
                    break;
                case CS_LOAD_ASK:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_LOAD;
                            break;
                        case 1:
                            gToolCardStep = CS_MODE_SELECT;
                            break;
                    }
                    break;
                case CS_NO_DATA:
                    if (gPadTrig & PADRright) {
                        gToolCardStep = CS_LOAD_CHECK;
                    }
                    break;
                case CS_LOAD:
                    LoadHitSlot(gToolSlot);
                    ApplyHitPoints();
                    ApplyHitQuads();
                    gToolCardStep = CS_COMPLETE;
                    break;
                case CS_ERROR:
                case CS_NO_CARD:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = CS_EXIT;
                    }
                    break;
                case CS_NO_FILE:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_CREATE;
                            break;
                        case 1:
                            gToolCardStep = CS_EXIT;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case CS_UNFORMATTED:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_FORMAT;
                            break;
                        case 1:
                            gToolCardStep = CS_EXIT;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case CS_CREATE:
                    err = RunCardOp2(CARD_OP_CREATE);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_CHECK;
                    }
                    break;
                case CS_FORMAT:
                    err = RunCardOp2(CARD_OP_FORMAT);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_CHECK;
                    }
                    break;
                case CS_EXIT:
                    gSubStep++;
                    break;
            }
            switch (err) {
                case CARD_PROBE_FAILED:
                case CARD_FAILED:
                    gToolCardStep = CS_ERROR;
                    break;
                case CARD_FULL:
                    gToolCardStep = CS_NO_MEMORY;
                    break;
                case CARD_ABSENT:
                    gToolCardStep = CS_NO_CARD;
                    break;
                case CARD_NO_FILE:
                    gToolCardStep = CS_NO_FILE;
                    break;
                case CARD_UNFORMATTED:
                    gToolCardStep = CS_UNFORMATTED;
                    break;
            }
            break;
        case 2:
            CloseCardEvents();
            gToolMode = TM_MOVE;
            break;
    }
}

extern char sHitNewlines3[]; /**< a line prefix, "\n\n\n" */
extern char sHitCheckText[]; /**< not yet known, "CHECK" */

/* The hit tool's small data: its strings, in address order. */
/* MATCHING: each declared above as an array of unknown size first, so the
 * code splits its address. */
char sHitNewlines6[] SDATA = "\n\n\n\n\n\n";
char sHitRadiusFmt[] SDATA = "R:%d\n";
char sHitColorWhite[] SDATA = "~c888";
char sHitColorBlue[] SDATA = "~c008";
char sHitMenuDeleteText[] SDATA = "DELETE\n";
char sHitMenuInfoText[] SDATA = "INFO\n";
char sHitMenuDataText[] SDATA = "DATA\n";
char sHitNewlines5[] SDATA = "\n\n\n\n\n";
char sHitNewlines3[] SDATA = "\n\n\n";
char sHitCheckText[] SDATA = "CHECK";
char sHitNewlines2[] SDATA = "\n\n";

void PrintToolCardMenu(void) {
    PrintTabs(5);
    FntPrint(sHitDataModeText);
    FntPrint(sHitNewlines3);
    switch (gToolCardStep) {
        case CS_CHECK:
            FntPrint(sHitNewlines3);
            PrintTabs(4);
            FntPrint(sHitCheckText);
            break;
        case CS_MODE_SELECT:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitModeSelectText);
            PrintSaveLoad();
            break;
        case CS_SLOT_SELECT:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitBufferSelectText);
            PrintToolSlots();
            break;
        case CS_CREATE:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCreateFileText);
            break;
        case CS_FORMAT:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCardFormatText);
            break;
        case CS_SAVE_ASK:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitSaveToDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sHitOverwriteOkText);
            PrintYesNo();
            break;
        case CS_WRITE:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitDataWritingText);
            break;
        case CS_LOAD_ASK:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitLoadFromDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sHitLoadOkText);
            PrintYesNo();
            break;
        case CS_NO_DATA:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitCantOpenDataFmt, gToolSlot + 1);
            break;
        case CS_COMPLETE:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCompleteText);
            break;
        case CS_ERROR:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitErrorText);
            break;
        case CS_NO_MEMORY:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitNotEnoughMemoryText);
            break;
        case CS_NO_CARD:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitNoCardText);
            break;
        case CS_NO_FILE:
            FntPrint(sHitNewlines3);
            PrintTabs(4);
            FntPrint(sHitNoFileText);
            PrintTabs(3);
            FntPrint(sHitCreateNewFileText);
            PrintYesNo();
            break;
        case CS_UNFORMATTED:
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

    fd = open((char *)sHitDataFilePath0, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x8018D200, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x80191944, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    fd = open((char *)sHitDataFilePath1, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x80193978, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x801980BC, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    fd = open((char *)sHitDataFilePath2, O_CREAT | O_WRONLY);
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
    DrawSprite(SPR_POINTER, &size, 0, 5, &gSubOt[gActiveBuff]);
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
    void *unk28;   /**< the owning model slot's coord */
} Live2C;

void *__builtin_memcpy(void *dst, const void *src, u32 n);

/* MATCHING: __builtin_memcpy, for a word-aligned 42-byte block move. */
void ApplyHitPoints(void) {
    s16 i;
    BankEntry *e = ((HitPointBank *)gHitPoints)->entries;
    HitPoint *rec;

    for (i = 0; i < (u32)gHitPointSections; i++) {
        gObjectModels[i].pointFirst = e->first;
        gObjectModels[i].pointCount = e->count;
        e++;
    }
    for (i = 0; i < 400; i++) {
        rec = GetHitPoint(0, i);
        __builtin_memcpy(&((Live2C *)gLivePoints)[i], rec, 0x2A);
        ((Live2C *)gLivePoints)[i].unk28 = gObjectModels[rec->section].coord;
    }
}

void ApplyHitQuads(void) {
    s16 i;
    s16 j;
    BankEntry *e = ((HitQuadBank *)gHitQuads)->entries;

    for (i = 0; i < (u32)gHitQuadSections; i++) {
        gObjectModels[i].quadFirst = e->first;
        gObjectModels[i].quadCount = e->count;
        e++;
    }
    for (i = 0; i < 80; i++) {
        for (j = gObjectModels[i].quadFirst;
             j < gObjectModels[i].quadCount + gObjectModels[i].quadFirst; j++) {
            GetHitQuad(0, j)->unk48 = gObjectModels[i].coord;
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
    color.r = BLEND_HALF;
    color.g = color.b = color.cd = 0x80;
    DrawBillboard(SPR_MARKER, &size, &color, 2, &gSubOt[gActiveBuff]);
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
    sToolModeSaved = 0;
    gToolSection = 0;
    gToolKind = TK_BALL;
    gToolPointIndex = 0;
    D_80095A5C = 0;
    gToolQuadIndex = 0;
    D_80095A40 = 0;
    gToolRadius = 5;
    sToolPickMode = 0;
    sToolInfoMode = 0;
}

void ClearHitPoints(void) {
    u32 i;

    bzero(gLivePoints, 0x44C0);
    for (i = 0; i < 80; i++) {
        gObjectModels[i].pointFirst = 0;
        gObjectModels[i].pointCount = -1;
    }
}

void ClearHitQuads(void) {
    u32 i;

    bzero(gLiveQuads, 0x1DB0);
    for (i = 0; i < 80; i++) {
        gObjectModels[i].quadFirst = 0;
        gObjectModels[i].quadCount = -1;
    }
}

void ToolPlace(void) {
    if (gPadTrig & PADRright) {
        switch (gToolKind) {
            case TK_BALL:
                InsertHitPoint();
                break;
            case TK_RIDE:
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
            gToolNewQuad.unk48 = gObjectModels[gToolSection].coord;
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
    sPreviewing = 1;
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
    LeaveQuadTop();
    sPreviewing = 0;
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
        gToolMode = TM_MOVE;
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

    fd = open((char *)sHitTmpFilePath, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}

s32 ReadToolFile(void) {
    s32 fd;
    s32 n;

    fd = open((char *)sHitTmpFilePath, O_RDONLY);
    n = read(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}
