#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libetc.h"
#include "libcd.h"
#include "sound.h"
#include "libsnd.h"
#include "libmcrd.h"
#include "model.h"
#include "draw.h"
#include "spad.h"
#include "world_intro.h"

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

extern s32 D_800FA220;
extern s32 D_800FA28C;
void func_800F83E0(void);
void func_800F8898(void);
void func_800F88B0(void);

void func_800F01EC(void) {
    SVECTOR pos;
    SColor color;

    switch (gSubStep) {
        case 50:
            DefineSprite(0x1E, 0, 0x80, 0xF0, 0x15, 0, 0, 0, 0x1F3);
            DefineSprite(0x1F, 0, 0x80, 0xF0, 0x15, 0x80, 0, 0, 0x1F3);
            DefineSprite(0x20, 0, 0x40, 0xF0, 0x16, 0, 0, 0, 0x1F3);
            sLights.l[0].r = 0x80;
            sLights.l[0].g = 0x80;
            sLights.l[0].b = 0x80;
            sLights.l[1].r = 0x80;
            sLights.l[1].g = 0x80;
            sLights.l[1].b = 0x80;
            sLights.l[2].r = 0x80;
            sLights.l[2].g = 0x80;
            sLights.l[2].b = 0x80;
            sLights.l[0].vx = 0;
            sLights.l[0].vy = 100;
            sLights.l[0].vz = 100;
            GsSetFlatLight(0, &sLights.l[0]);
            sLights.l[1].vx = 0x56;
            sLights.l[1].vy = 100;
            sLights.l[1].vz = -50;
            GsSetFlatLight(1, &sLights.l[1]);
            sLights.l[2].vx = -0x56;
            sLights.l[2].vy = 100;
            sLights.l[2].vz = -50;
            GsSetFlatLight(2, &sLights.l[2]);
            gPlayer.action = 0x61;
            gPlayer.motion[0] = 3;
            gPlayer.z = -600;
            gPlayer.speed = 0;
            gPlayer.x = 0;
            gPlayer.y = 100;
            gPlayer.yaw = 0x800;
            sView.vpx = 500;
            sView.vpy = -100;
            sView.vpz = 0;
            sView.vrx = 0;
            sView.vry = 0;
            sView.vrz = 0;
            D_800FA220 = 0;
            gSubStep = 100;
            func_800F8898();
            break;
        case 100:
            func_800F83E0();
            color.r = -1;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -160;
            pos.vy = -120;
            DrawSprite(0x1E, &pos, (CVECTOR *)&color, 1, gCurOt);
            pos.vx += 0x80;
            DrawSprite(0x1F, &pos, (CVECTOR *)&color, 1, gCurOt);
            pos.vx += 0x80;
            DrawSprite(0x20, &pos, (CVECTOR *)&color, 1, gCurOt);
            if (D_800FA28C == 1) {
                D_800FA220++;
                gPlayer.z += 20;
                if (D_800FA220 >= 60) {
                    func_800F88B0();
                }
            }
            if (D_800FA28C == 0) {
                gModeStep = 5;
            }
            if (gPadTrig & PADstart) {
                gModeStep = 5;
            }
            break;
    }
}

void func_800F04EC(SVECTOR *pos, s32 count, s32 frames) {
    SColor color;
    s32 x;
    s32 n;
    s32 m;
    s32 d;
    s32 k;
    s32 t;
    s32 sec;
    /* MATCHING: two dead divisions keep the /10 and /100 constants in
     * prologue registers; the /100 one only after the first draw. */
    u32 dead;

    dead = (s16)count / 10;
    color.r = BLEND_ADD;
    color.g = color.b = color.cd = 0x80;
    x = pos->vx;
    pos->vx -= 8;
    DrawSprite(0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x2F, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x39, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = count;
    dead = (u32)frames / 100;
    count /= 10;
    pos->vx -= 8;
    DrawSprite(n % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    if (count != 0) {
        pos->vx -= 8;
        DrawSprite(count % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    if (n / 100 != 0) {
        pos->vx -= 8;
        DrawSprite(n / 100 % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    n = frames * 100 / 30;
    d = n;
    d /= 10;
    pos->vx = x - 8;
    pos->vy += 8;
    DrawSprite(n % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(d % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x38, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    sec = frames / 30;
    m = sec / 60;
    n = sec % 60;
    k = 1;
    d = n;
    d /= 10;
    pos->vx -= 8;
    DrawSprite(n % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(d % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = m;
    pos->vx -= 8;
    DrawSprite(0x38, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    for (; k < 11; k *= 10) {
        t = n / k;
        if (t != 0 || (k == 1 && n == 0)) {
            pos->vx -= 8;
            DrawSprite(t % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
    }
}

/** @brief Two words, copied as a pair (a unit-local view). */
typedef struct {
    s32 a; /**< first word */
    s32 b; /**< second word */
} Pair8;

/** @brief The game's save as the title reads it from the memory card. */
typedef struct {
    Pair8 times[4];   /**< gBestTimes */
    Pair8 cans[4];    /**< gBestCans */
    u8 pad40[8];      /**< not read here */
    Pair8 unk48;      /**< D_80095978 */
    u8 stereo;        /**< gStereo */
    u8 rumble;        /**< gRumbleEnabled */
    s8 sortMode;      /**< gTmdSortMode */
    s8 buttonConfig;  /**< gButtonConfig */
    s8 unk54;         /**< D_800957A0 */
    u8 gameCleared;   /**< gGameCleared */
    u8 allTargets;    /**< gAllTargetsBeaten */
    u8 unlocks;       /**< gUnlocks */
    s32 checksum;     /**< the complement of the byte sum of the 0x5C bytes, read as 0 */
} SaveData;

/* MATCHING: per-unit views, as in main.c. */
extern Pair8 gBestCans[];
extern Pair8 gBestTimes[];
extern Pair8 D_80095978;
extern u8 gRumbleEnabled;
extern s8 D_800957A0;
extern u8 gGameCleared;
extern s32 D_800FA240;
extern SaveData *D_800FA24C;
extern s32 D_800FA250;
extern s32 D_800FA25C;
extern long D_800FA270;
s32 IsScreenWipeActive(void);
extern char D_800F0000[]; /* "BISLPS-01762", the save file's name */

void func_800F0AD4(void) {
    s32 bad;
    u32 i;

    switch (gSubStep) {
        case 50:
            D_800FA25C = 0;
            gSubStep = 51;
            break;
        case 51:
            if (IsScreenWipeActive() == 0 && D_800FA25C == 0) {
                D_800FA25C = 1;
            }
            if (D_800FA25C != 0) {
                D_800FA25C++;
            }
            if (D_800FA25C == 8) {
                MemCardSync(0, 0, &D_800FA270);
                switch (D_800FA270) {
                    case 0:
                    case 3:
                        bad = 0;
                        break;
                    default:
                        bad = 1;
                        break;
                }
                if (bad == 0) {
                    MemCardReadFile(0, D_800F0000, (unsigned long *)0x8016D000, 0x200, 0x80);
                    MemCardSync(0, 0, &D_800FA270);
                    if (D_800FA270 == 0) {
                        D_800FA24C = (SaveData *)0x8016D000;
                        D_800FA240 = D_800FA24C->checksum;
                        D_800FA24C->checksum = 0;
                        D_800FA250 = 0;
                        for (i = 0; i < 0x5C; i++) {
                            D_800FA250 += ((u8 *)D_800FA24C)[i];
                        }
                        D_800FA250 = ~D_800FA250;
                        if (D_800FA250 == D_800FA240) {
                            gBestTimes[0].a = D_800FA24C->times[0].a;
                            gBestTimes[1].a = D_800FA24C->times[1].a;
                            gBestTimes[2].a = D_800FA24C->times[2].a;
                            gBestTimes[3].a = D_800FA24C->times[3].a;
                            gBestTimes[0].b = D_800FA24C->times[0].b;
                            gBestTimes[1].b = D_800FA24C->times[1].b;
                            gBestTimes[2].b = D_800FA24C->times[2].b;
                            gBestTimes[3].b = D_800FA24C->times[3].b;
                            gBestCans[0].a = D_800FA24C->cans[0].a;
                            gBestCans[1].a = D_800FA24C->cans[1].a;
                            gBestCans[2].a = D_800FA24C->cans[2].a;
                            gBestCans[3].a = D_800FA24C->cans[3].a;
                            gBestCans[0].b = D_800FA24C->cans[0].b;
                            gBestCans[1].b = D_800FA24C->cans[1].b;
                            gBestCans[2].b = D_800FA24C->cans[2].b;
                            gBestCans[3].b = D_800FA24C->cans[3].b;
                            D_80095978.a = D_800FA24C->unk48.a;
                            D_80095978.b = D_800FA24C->unk48.b;
                            gStereo = D_800FA24C->stereo;
                            gRumbleEnabled = D_800FA24C->rumble;
                            gTmdSortMode = D_800FA24C->sortMode;
                            gButtonConfig = D_800FA24C->buttonConfig;
                            D_800957A0 = D_800FA24C->unk54;
                            gGameCleared = D_800FA24C->gameCleared;
                            gAllTargetsBeaten = D_800FA24C->allTargets;
                            gUnlocks = D_800FA24C->unlocks;
                            if (gStereo != 0) {
                                SsSetStereo();
                            } else {
                                SsSetMono();
                            }
                            switch (gTmdSortMode) {
                                case 0:
                                case 2:
                                case 3:
                                    LoadTimList((unsigned long *)(*(s32 *)0x8019D004 + 0x8019D000));
                                    break;
                                case 1:
                                    LoadTimList((unsigned long *)(*(s32 *)0x8019D008 + 0x8019D000));
                                    break;
                            }
                        }
                    }
                }
                gModeStep = 40;
                gSubStep = 50;
            }
            break;
    }
}

void func_800F0E58(void) {
    s32 i;

    for (i = 0; i < 7; i++) {
        DefineSprite(i + 0x1E, 0, 0xA0, 0x10, 6, 0, (u8)(i * 16), 0, 0x1E0);
    }
    for (i = 0; i < 7; i++) {
        DefineSprite(i + 0x26, 0, 0x60, 0x10, 6, 0xA0, (u8)(i * 16), 0, 0x1E0);
    }
    DefineSprite(0x25, 0, 0x80, 0x10, 6, 0, 0x70, 0, 0x1E0);
    DefineSprite(0x2D, 0, 0x20, 0x10, 6, 0x80, 0x70, 0, 0x1E0);
    for (i = 0; i < 12; i++) {
        DefineSprite(i + 0x2E, 0, 8, 8, 6, (u8)(i * 8 + 0xA0), 0x70, 0, 0x1E0);
    }
    DefineSprite(0x3A, 0, 0x50, 0x10, 6, 0, 0x80, 0, 0x1E0);
    for (i = 0; i < 3; i++) {
        DefineSprite(i + 0x3B, 0, 0x80, 0x10, 6, 0, (u8)(i * 16 + 0x90), 0, 0x1E0);
    }
    DefineSprite(0x3E, 0, 0x30, 0x10, 6, 0x50, 0x80, 0, 0x1E0);
    DefineSprite(0x3F, 0, 0x40, 0x10, 6, 0, 0xC0, 0, 0x1E0);
    DefineSprite(0x40, 0, 0x60, 0x10, 6, 0x40, 0xC0, 0, 0x1E0);
    DefineSprite(0x41, 0, 0x90, 0x10, 6, 0, 0xD0, 0, 0x1E0);
    DefineSprite(0x42, 0, 0x50, 0x10, 6, 0, 0xE0, 0, 0x1E0);
    DefineSprite(0x43, 0, 0x50, 0x10, 6, 0x50, 0xE0, 0, 0x1E0);
    DefineSprite(0x44, 0, 0x50, 0x10, 6, 0, 0xF0, 0, 0x1E0);
    DefineSprite(0x45, 0, 0x50, 0x10, 6, 0x50, 0xF0, 0, 0x1E0);
    for (i = 0; i < 8; i++) {
        DefineSprite(i + 0x46, 0, 0x60, 0x10, 6, 0xA0, (u8)(i * 16 + 0x80), 0, 0x1E0);
    }
    DefineSprite(0x4E, 0, 0x10, 0x10, 6, 0x80, 0x80, 0, 0x1E0);
    DefineSprite(0x4F, 0, 0x10, 0x10, 6, 0x90, 0x80, 0, 0x1E0);
    DefineSprite(0x50, 0, 0x10, 0x10, 6, 0x80, 0x90, 0, 0x1E0);
    DefineSprite(0x51, 0, 0x10, 0x10, 6, 0x80, 0xA0, 0, 0x1E0);
    DefineSprite(0x52, 0, 0x10, 0x10, 6, 0x80, 0xB0, 0, 0x1E0);
    DefineSprite(0x53, 0, 0xA0, 0x20, 7, 0, 0x80, 0x30, 0x1E0);
    DefineSprite(0x54, 0, 0x80, 0x20, 7, 0, 0xA0, 0x30, 0x1E0);
    DefineSprite(0x55, 0, 0x90, 0x20, 7, 0, 0xC0, 0x30, 0x1E0);
    DefineSprite(0x56, 0, 0x88, 0x20, 7, 0, 0xE0, 0x30, 0x1E0);
    for (i = 0; i < 18; i++) {
        DefineSprite(i + 0x57, 0, 0xFF, 0xE, 5, 0, (u8)(i * 14), 0x10, 0x1E0);
    }
    DefineSprite(0x69, 0, 0x10, 0x10, 7, 0, 0, 0x20, 0x1E0);
    DefineSprite(0x6A, 0, 0x10, 0x10, 7, 0x10, 0, 0x20, 0x1E0);
    DefineSprite(0x6B, 0, 0x10, 0x10, 7, 0x20, 0, 0x20, 0x1E0);
}

extern GsCOORDINATE2 sAct3Coords[];
extern GsDOBJ2 sAct3Objs[];
extern VECTOR D_8009EA78[];
extern SVECTOR D_8009EB58[];

void func_800F1494(s32 i) {
    MATRIX m;

    sAct3Coords[i].coord.t[0] = D_8009EA78[i].vx;
    sAct3Coords[i].coord.t[1] = D_8009EA78[i].vy;
    sAct3Coords[i].coord.t[2] = D_8009EA78[i].vz;
    SetCoordRotation(&D_8009EB58[i], &sAct3Coords[i]);
    GsGetLs(sAct3Objs[i].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sAct3Objs[i].coord2, &m);
    GsSetLightMatrix(&m);
    SetTmdSortMode(0);
    GsSortObject4J(&sAct3Objs[i], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}

INCLUDE_RODATA("asm/ovl/ovl02/nonmatchings/ovl02", D_800F0000);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F1570);

extern CdlLOC D_800F96F8;
extern CdlLOC D_800F96FC;
extern s32 D_800F9748;
extern s32 D_800F974C;
void func_800F0AD4(void);
void func_800F1570(void);
void func_800F69B0(void);
void func_800F7A4C(void);

void func_800F6670(void) {
    switch (gModeStep) {
        case 0:
            gSubStep = 0;
            gCdLoadLoc[0] = (s32)&D_800F96F8;
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gModeStep = 1;
            gCdLoadSectors[0] = D_800F9748;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                gPlayer.rideMode = 0;
                gPlayer.flattened = 0;
                LoadTimList((unsigned long *)(*(s32 *)0x8019D000 + 0x8019D000));
                gSubStep = 50;
                if (gStageTableSet == -1) {
                    LoadTimList((unsigned long *)(*(s32 *)0x8019D004 + 0x8019D000));
                    gModeStep = 2;
                } else {
                    gModeStep = 40;
                }
            }
            break;
        case 2:
            DrawFrozenFrame();
            SetTmdSortMode(gTmdSortMode);
            func_800F01EC();
            break;
        case 5:
            PlayMovie("\\MOVIE0.STR;1", NULL, 0x31F, 0xF0, 1);
            gModeStep = 9;
            gSubStep = 50;
            break;
        case 6:
            PlayMovie("\\OP2_OK.STR;1", NULL, 0xAE, 0xF0, 1);
            gModeStep = 9;
            gSubStep = 50;
            break;
        case 7:
            PlayMovie("\\OP2_15.STR;1", NULL, 0x15E, 0xF0, 1);
            gModeStep = 9;
            gSubStep = 50;
            break;
        case 9:
            LoadTimList((unsigned long *)(*(s32 *)0x8019D000 + 0x8019D000));
            gModeStep = 10;
            gSubStep = 50;
            break;
        case 10:
            DrawFrozenFrame();
            SetTmdSortMode(gTmdSortMode);
            func_800F69B0();
            break;
        case 30:
            DrawFrozenFrame();
            DrawScreenWipe();
            func_800F0AD4();
            break;
        case 40:
            DrawFrozenFrame();
            SetTmdSortMode(gTmdSortMode);
            func_800F1570();
            break;
        case 50:
            gSubStep = 0;
            gCdLoadLoc[0] = (s32)&D_800F96FC;
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gModeStep = 51;
            gCdLoadSectors[0] = D_800F974C;
            break;
        case 51:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)(*(s32 *)0x8019D000 + 0x8019D000));
                if (gTmdSortMode == 1) {
                    LoadTimList((unsigned long *)(*(s32 *)0x8019D00C + 0x8019D000));
                } else {
                    LoadTimList((unsigned long *)(*(s32 *)0x8019D008 + 0x8019D000));
                }
                gSubStep = 0;
                gModeStep = 60;
            }
            break;
        case 60:
            func_800F7A4C();
            break;
    }
}

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F69B0);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F7A4C);

void func_800F80F8(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[1] = 0;
    c.coord.t[0] = gPlayer.x;
    c.coord.t[2] = gPlayer.z;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    size.vx = 100;
    size.vy = 50;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x40;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

void func_800F81B4(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[0] = 0x2B5C;
    c.coord.t[1] = 0;
    c.coord.t[2] = 0;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    size.vx = 800;
    size.vy = 50;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x60;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

void func_800F8264(void) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    SetTmdSortMode(1);
    o = ((DrawList *)gTodObjTable)->objs;
    o->coord2->coord.t[0] = gPlayer.x;
    o->coord2->coord.t[1] = gPlayer.y;
    o->coord2->coord.t[2] = gPlayer.z;
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = gPlayer.yaw;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; i++, o++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            SetSpadStack();
            GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
        /* MATCHING: as in world_intro.inc's model draw, three barriers rank
         * the object pointer above the counter (retail's s1 and s2). */
        __asm__("");
        __asm__("");
        __asm__("");
    }
}

extern s32 D_800FA288;

void func_800F83E0(void) {
    GsOT *ot;
    POLY_F4 *p;
    DR_TPAGE *t;

    ot = &gSubOt[gActiveBuff];
    switch (D_800FA28C) {
        case 1:
            break;
        case 0:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0;
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            addPrim(ot->org, p);
            p++;
            GsOUT_PACKET_P = (PACKET *)p;
            break;
        case 2:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            p->r0 = p->g0 = p->b0 = ~(D_800FA288 * 8);
            addPrim(ot->org, p);
            p++;
            t = (DR_TPAGE *)p;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(ot->org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            D_800FA288++;
            if (D_800FA288 >= 32) {
                D_800FA28C = 1;
            }
            break;
        case 3:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            p->r0 = p->g0 = p->b0 = D_800FA288 * 8;
            addPrim(ot->org, p);
            p++;
            t = (DR_TPAGE *)p;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(ot->org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            D_800FA288++;
            if (D_800FA288 >= 32) {
                D_800FA28C = 0;
            }
            break;
    }
}

void func_800F8708(SVECTOR *pos, s32 kind) {
    switch (kind) {
        case 1:
            DrawSprite(0x53, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 2:
            DrawSprite(0x54, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 3:
            DrawSprite(0x55, pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos->vx += 0x90;
            DrawSprite(0x56, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
    }
}

void func_800F8840(PlayerState *obj) {
    obj->motionResult[0] = StepAnimation(obj, 0, obj->animFilter);
    switch (obj->action) {
        case 0x60:
            break;
        case 0x61:
            if (obj->motionResult[0] == 0) {
                obj->curMotion[0] = 0xFF;
            }
            break;
    }
}

void func_800F8898(void) {
    D_800FA288 = 0;
    D_800FA28C = 2;
}

void func_800F88B0(void) {
    D_800FA288 = 0;
    D_800FA28C = 3;
}

void func_800F88C8(SVECTOR *rot) {
    MATRIX m;
    SVECTOR v;
    VECTOR out;

    v.vx = 300;
    v.vy = 0;
    v.vz = 0;
    m = GsIDMATRIX;
    RotMatrix(rot, &m);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    RotTrans(&v, &out, 0);
    sView.vrx = 0;
    sView.vry = -100;
    sView.vrz = 0;
    sView.vpx = out.vx;
    sView.vpy = out.vy - 100;
    sView.vpz = out.vz;
    GsSetRefView2(&sView);
}
