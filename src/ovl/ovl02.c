#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libetc.h"
#include "libcd.h"
#include "sound.h"
#include "libsnd.h"
#include "libmcrd.h"
#include "libpad.h"
#include "rand.h"
#include "model.h"
#include "draw.h"
#include "spad.h"
#include "world_intro.h"

/* MATCHING: non-static inline, emitted at the file's end in this order. */
inline void func_800F80F8(void);
inline void func_800F81B4(void);
inline void func_800F8264(void);
inline void func_800F83E0(void);
inline void func_800F8708(SVECTOR *pos, s32 kind);
inline void func_800F8840(PlayerState *obj);
inline void func_800F8898(void);
inline void func_800F88B0(void);
inline void func_800F88C8(SVECTOR *rot);

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

extern s32 D_800FA220;
extern s32 D_800FA28C;

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
    Pair8 times[4];  /**< gBestTimes */
    Pair8 cans[4];   /**< gBestCans */
    u8 pad40[8];     /**< not read here */
    Pair8 unk48;     /**< copied to the pair after gBestTimes and gBestCans */
    u8 stereo;       /**< gStereo */
    u8 rumble;       /**< gRumbleEnabled */
    s8 sortMode;     /**< gTmdSortMode */
    s8 buttonConfig; /**< gButtonConfig */
    s8 unk54;        /**< a signed byte the boot code sets to -1 */
    u8 gameCleared;  /**< gGameCleared */
    u8 allTargets;   /**< gAllTargetsBeaten */
    u8 unlocks;      /**< gUnlocks */
    s32 checksum;    /**< the complement of the byte sum of the 0x5C bytes, read as 0 */
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
                            /* MATCHING: one call per arm, which cross-jumping merges. */
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

extern SVECTOR D_800FA230;
extern s32 D_800FA228;
extern s32 D_800FA238;
extern s32 D_800FA23C;
extern s32 D_800FA244;
extern s32 D_800FA248;
extern s32 D_800FA254;
extern s32 D_800FA258;
extern s8 D_800FA260[];
extern s8 D_800FA268;
extern s32 D_800FA26C;
extern s32 D_800FA274;
extern s32 D_80095984;
extern s32 D_80095998;
extern u8 D_800F9460[];
extern s8 D_800F9660[][4];
extern Pair8 gActTimes;
extern Pair8 gActCans;
extern Pair8 D_800958C0;
extern u16 gRumbleTime;
extern u16 gRumbleSmall[];

#ifdef NON_MATCHING
/* MATCHING: 0.999; step 5000's entry loop keeps gSubOt's address in a saved
 * register, where the original rebuilds it every pass (step 6000 keeps it). */
void func_800F1570(void) {
    SVECTOR pos;
    SVECTOR p2;
    SVECTOR unused;
    SColor col;
    SColor norm;
    SColor hi;
    u32 i;
    s32 t;
    s32 d;

    switch (gSubStep) {
        case 50:
            D_800FA258 = 0;
            func_800F0E58();
            GsInitCoordinate2(&sAct3Coords[0], NULL);
            GsInitCoordinate2(&sAct3Coords[1], NULL);
            GsInitCoordinate2(&sAct3Coords[2], NULL);
            GsMapModelingData((unsigned long *)(*(s32 *)0x8019D018 + 0x8019D004));
            GsMapModelingData((unsigned long *)(*(s32 *)0x8019D01C + 0x8019D004));
            GsMapModelingData((unsigned long *)(*(s32 *)0x8019D020 + 0x8019D004));
            GsLinkObject4(*(s32 *)0x8019D018 + 0x8019D00C, &sAct3Objs[0], 0);
            GsLinkObject4(*(s32 *)0x8019D01C + 0x8019D00C, &sAct3Objs[1], 0);
            GsLinkObject4(*(s32 *)0x8019D020 + 0x8019D00C, &sAct3Objs[2], 0);
            sAct3Objs[0].coord2 = &sAct3Coords[0];
            sAct3Objs[0].attribute = 0;
            sAct3Objs[1].coord2 = &sAct3Coords[1];
            sAct3Objs[1].attribute = 0;
            sAct3Objs[2].coord2 = &sAct3Coords[2];
            sAct3Objs[2].attribute = 0;
            D_8009EA78[0].vx = D_8009EA78[1].vx = D_8009EA78[2].vx = 0;
            D_8009EA78[0].vy = D_8009EA78[1].vy = D_8009EA78[2].vy = 0;
            D_8009EA78[0].vz = D_8009EA78[1].vz = D_8009EA78[2].vz = 0;
            D_8009EB58[0].vx = D_8009EB58[1].vx = D_8009EB58[2].vx = 0;
            D_8009EB58[0].vy = D_8009EB58[1].vy = D_8009EB58[2].vy = 0;
            D_8009EB58[0].vz = D_8009EB58[1].vz = D_8009EB58[2].vz = 0;
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
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.yaw = 0;
            D_800FA230.vx = 0;
            D_800FA230.vy = 0;
            D_800FA230.vz = 0;
            switch (gStageTableSet) {
                case -1:
                case 0:
                case 1:
                    gSubStep = 100;
                    break;
                case 2:
                case 3:
                    D_800FA258 = 2;
                    gSubStep = 1000;
                    break;
            }
            break;
        case 100:
            D_800FA258 = 0;
            D_800FA268 = 0;
            D_800FA260[D_800FA268] = 0;
            D_800FA268++;
            if (D_800957A0 != -1) {
                D_800FA260[D_800FA268] = 1;
                D_800FA268++;
            }
            /* MATCHING: retail tests the flags with lb, an s8 view. */
            if (*(s8 *)&gGameCleared != 0) {
                D_800FA260[D_800FA268] = 2;
                D_800FA268++;
            }
            if (*(s8 *)&gAllTargetsBeaten != 0) {
                D_800FA260[D_800FA268] = 3;
                D_800FA268++;
            }
            D_800FA260[D_800FA268] = 4;
            D_800FA268++;
            D_800FA260[D_800FA268] = 5;
            D_800FA268++;
            D_800FA260[D_800FA268] = 6;
            D_800FA268++;
            D_800FA274 = 0;
            D_800FA244 = 0;
            gSubStep = 101;
            break;
        case 101:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            D_800FA244++;
            for (i = 0; (s32)i < D_800FA268; i++) {
                t = D_800FA244 - i * 2;
                if (t > 8) {
                    t = 8;
                }
                pos.vx = t * 32 - 0x150;
                /* MATCHING: the cast keeps i * 24 - 12 one induction value;
                 * without it the -12 moves onto the count. */
                pos.vy = (s16)(i * 24 - 12) - D_800FA268 * 12;
                DrawSprite(D_800FA260[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (D_800FA244 == D_800FA268 * 2 + 8) {
                gSubStep = 102;
            }
            break;
        case 102:
            norm.r = -1;
            norm.g = norm.b = norm.cd = 0x80;
            hi.r = -1;
            hi.g = hi.b = 0x80;
            hi.cd = 0;
            for (i = 0; (s32)i < D_800FA268; i++) {
                if (i == D_800FA274) {
                    col = hi;
                } else {
                    col = norm;
                }
                pos.vx = -0x50;
                pos.vy = (s16)(i * 24 - 12) - D_800FA268 * 12;
                DrawSprite(D_800FA260[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            p2.vx = -0x50;
            p2.vy = D_800FA274 * 24 - D_800FA268 * 12 - 0x15;
            func_800F8708(&p2, 1);
            if (gPadTrig & PADLup) {
                PlaySe(0xB);
                if (D_800FA274 == 0) {
                    D_800FA274 = D_800FA268 - 1;
                } else {
                    D_800FA274--;
                }
            } else if (gPadTrig & PADLdown) {
                PlaySe(0xB);
                if (D_800FA274 == D_800FA268 - 1) {
                    D_800FA274 = 0;
                } else {
                    D_800FA274++;
                }
            } else if (gPadTrig & PADRright) {
                PlaySe(0xC);
                switch (D_800FA260[D_800FA274]) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        D_800FA244 = 0;
                        gSubStep = 103;
                        break;
                    case 5:
                        D_800FA228 = 0;
                        gSubStep = 5000;
                        break;
                    case 6:
                        D_800FA228 = 0;
                        gSubStep = 6000;
                        break;
                }
            }
            break;
        case 103:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            D_800FA244++;
            for (i = 0; (s32)i < D_800FA268; i++) {
                t = D_800FA244 - i * 2;
                if (t < 0) {
                    t = 0;
                }
                pos.vx = t * 32 - 0x50;
                pos.vy = (s16)(i * 24 - 12) - D_800FA268 * 12;
                DrawSprite(D_800FA260[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (D_800FA244 == D_800FA268 * 2 + 12) {
                switch (D_800FA260[D_800FA274]) {
                    case 0:
                        gStageTableSet = 0;
                        gRetries = 3;
                        gStage = 0;
                        gGameMode = 0x2C;
                        break;
                    case 1:
                        gRetries = 3;
                        if (D_800957A0 >= 100) {
                            gStageTableSet = 1;
                            gStage = D_800957A0 - 100;
                        } else {
                            gStageTableSet = 0;
                            gStage = D_800957A0;
                        }
                        gActTimes.a = D_800958C0.a;
                        gActTimes.b = D_800958C0.b;
                        gActCans.a = D_80095978.a;
                        gActCans.b = D_80095978.b;
                        switch ((u8)(gStage % 3)) {
                            case 0:
                                BeginLoadScreenModel();
                                break;
                            case 1:
                                BeginLoadScreen();
                                break;
                            case 2:
                                BeginAct3LoadScreen();
                                break;
                        }
                        break;
                    case 2:
                        gSubStep = 1000;
                        break;
                    case 3:
                        gStageTableSet = 1;
                        gRetries = 3;
                        gStage = 0;
                        gGameMode = 0x2C;
                        break;
                    case 4:
                        D_800FA23C = 0;
                        gSubStep = 2000;
                        break;
                }
            }
            break;
        case 1000:
            D_800FA258 = 2;
            D_800FA244 = 0;
            D_800FA248 = 0;
            gSubStep++;
            break;
        case 1001:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++D_800FA244;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x150;
            pos.vy = -0x58;
            DrawSprite(0x20, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x130;
            pos.vy = -0x38;
            DrawSprite(D_80095998 + 0x26, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            /* MATCHING: the bob in a block-local with an (s16) cast; a shared
             * local takes the division's register, no cast folds the + 8. */
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x58);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 4;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x18;
            DrawSprite(0x25, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 6;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 8;
            DrawSprite(0x2A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            func_800F04EC(&p2, gBestCans[D_80095998].a, gBestTimes[D_80095998].a);
            t = D_800FA244 - 8;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x28;
            DrawSprite(0x2B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            func_800F04EC(&p2, gBestCans[D_80095998].b, gBestTimes[D_80095998].b);
            t = D_800FA244 - 10;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x48;
            DrawSprite(0x2C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 == 20) {
                gSubStep = 1002;
            }
            break;
        case 1002:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            norm.r = -1;
            norm.g = norm.b = norm.cd = 0x80;
            hi.r = -1;
            hi.g = hi.b = 0x80;
            hi.cd = 0;
            pos.vx = -0x50;
            pos.vy = -0x58;
            DrawSprite(0x20, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx = -0x30;
            pos.vy = -0x38;
            DrawSprite(D_80095998 + 0x26, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x58);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 < 16) {
                D_800FA244++;
            }
            if (D_80095984 == 0) {
                col = hi;
            } else {
                col = norm;
            }
            t = D_800FA244;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x18;
            DrawSprite(0x25, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_80095984 == 1) {
                col = hi;
            } else {
                col = norm;
            }
            t = D_800FA244 - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 8;
            DrawSprite(0x2A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            func_800F04EC(&p2, gBestCans[D_80095998].a, gBestTimes[D_80095998].a);
            if (D_80095984 == 2) {
                col = hi;
            } else {
                col = norm;
            }
            t = D_800FA244 - 4;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x28;
            DrawSprite(0x2B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            func_800F04EC(&p2, gBestCans[D_80095998].b, gBestTimes[D_80095998].b);
            if (D_80095984 == 3) {
                col = hi;
            } else {
                col = norm;
            }
            t = D_800FA244 - 6;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x48;
            DrawSprite(0x2C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 >= 16) {
                p2.vx = -0x8E;
                p2.vy = D_80095984 * 32 - 0x21;
                func_800F8708(&p2, 2);
            }
            if (gPadTrig & PADLleft) {
                PlaySe(0xB);
                D_800FA244 = 0;
                if (D_80095998 == 0) {
                    D_80095998 = 3;
                } else {
                    D_80095998--;
                }
            } else if (gPadTrig & PADLright) {
                PlaySe(0xB);
                D_800FA244 = 0;
                if (D_80095998 == 3) {
                    D_80095998 = 0;
                } else {
                    D_80095998++;
                }
            } else if (gPadTrig & PADLup) {
                if (D_800FA244 >= 16) {
                    PlaySe(0xB);
                    if (D_80095984 == 0) {
                        D_80095984 = 3;
                    } else {
                        D_80095984--;
                    }
                }
            } else if (gPadTrig & PADLdown) {
                if (D_800FA244 >= 16) {
                    PlaySe(0xB);
                    if (D_80095984 == 3) {
                        D_80095984 = 0;
                    } else {
                        D_80095984++;
                    }
                }
            } else if (gPadTrig & PADRdown) {
                if (D_800FA244 >= 16) {
                    PlaySe(0xE);
                    D_800FA248 = 1;
                    D_800FA244 = 0;
                    gSubStep = 1003;
                }
            } else if (gPadTrig & PADRright) {
                if (D_800FA244 >= 16) {
                    PlaySe(0xC);
                    D_800FA244 = 0;
                    gSubStep = 1003;
                }
            }
            break;
        case 1003:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++D_800FA244;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x50;
            pos.vy = -0x58;
            DrawSprite(0x20, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 2;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x30;
            pos.vy = -0x38;
            DrawSprite(D_80095998 + 0x26, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x58);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 4;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = -0x18;
            DrawSprite(0x25, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 6;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = 8;
            DrawSprite(0x2A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            func_800F04EC(&p2, gBestCans[D_80095998].a, gBestTimes[D_80095998].a);
            t = D_800FA244 - 8;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = 0x28;
            DrawSprite(0x2B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            func_800F04EC(&p2, gBestCans[D_80095998].b, gBestTimes[D_80095998].b);
            t = D_800FA244 - 10;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = 0x48;
            DrawSprite(0x2C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 == 24) {
                if (D_800FA248 == 1) {
                    gSubStep = 100;
                } else {
                    gRetries = 99;
                    switch (D_80095998) {
                        case 0:
                            gStage = 0;
                            break;
                        case 1:
                            gStage = 3;
                            break;
                        case 2:
                            gStage = 9;
                            break;
                        case 3:
                            gStage = 12;
                            break;
                    }
                    switch (D_80095984) {
                        case 0:
                            gStageTableSet = 2;
                            gGameMode = 0x26;
                            break;
                        case 1:
                            gStageTableSet = 3;
                            BeginLoadScreenModel();
                            break;
                        case 2:
                            gStageTableSet = 3;
                            gStage++;
                            BeginLoadScreen();
                            break;
                        case 3:
                            gStageTableSet = 3;
                            gStage += 2;
                            BeginAct3LoadScreen();
                            break;
                    }
                }
            }
            break;
        case 2000:
            D_800FA258 = 1;
            D_800FA244 = 0;
            gSubStep = 2001;
            break;
        case 2001:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++D_800FA244;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x150;
            pos.vy = -0x58;
            DrawSprite(0x22, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x38;
            DrawSprite(0x3A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0x98;
            if (gStereo != 0) {
                DrawSprite(0x46, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            } else {
                DrawSprite(0x47, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (PadInfoMode(0, 2, 0) != 0) {
                t = D_800FA244 - 4;
                if (t > 8) {
                    t = 8;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = -0x18;
                DrawSprite(0x3B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                if (gRumbleEnabled != 0) {
                    DrawSprite(0x48, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    DrawSprite(0x49, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
            }
            if (*(s8 *)&gUnlocks != 1) {
                t = D_800FA244 - 6;
                if (t > 8) {
                    t = 8;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = 8;
                DrawSprite(0x3C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                switch (gTmdSortMode) {
                    case 0:
                        DrawSprite(0x4A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 1:
                        DrawSprite(0x4B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 2:
                        DrawSprite(0x4C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 3:
                        DrawSprite(0x4D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                }
            }
            t = D_800FA244 - 8;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x140;
            pos.vy = 0x28;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 10;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x118;
            pos.vy = 0x48;
            DrawSprite(0x3E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 == 20) {
                gSubStep = 2002;
            }
            break;
        case 2002:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            norm.r = -1;
            norm.g = norm.b = norm.cd = 0x80;
            hi.r = -1;
            hi.g = hi.b = 0x80;
            hi.cd = 0;
            pos.vx = -0x50;
            pos.vy = -0x58;
            DrawSprite(0x22, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = 8;
            if (D_800FA23C == 0) {
                col = hi;
            } else {
                col = norm;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x38;
            DrawSprite(0x3A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0x98;
            if (gStereo != 0) {
                DrawSprite(0x46, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            } else {
                DrawSprite(0x47, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (D_800FA23C == 0) {
                p2.vy = pos.vy;
                {
                    s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                    p2.vx = pos.vx - (b >= 0 ? b : -b);
                }
                DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                {
                    s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                    p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x50);
                }
                DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if ((D_800FA254 = PadInfoMode(0, 2, 0) != 0)) {
                if (D_800FA23C == 1) {
                    col = hi;
                } else {
                    col = norm;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = -0x18;
                DrawSprite(0x3B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                if (gRumbleEnabled != 0) {
                    DrawSprite(0x48, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    DrawSprite(0x49, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
                if (D_800FA23C == 1) {
                    p2.vy = pos.vy;
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx - (b >= 0 ? b : -b);
                    }
                    DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x50);
                    }
                    DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
            } else if (D_800FA23C == 1) {
                D_800FA23C = 0;
            }
            if (*(s8 *)&gUnlocks != 1) {
                if (D_800FA23C == 2) {
                    col = hi;
                } else {
                    col = norm;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = 8;
                DrawSprite(0x3C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                switch (gTmdSortMode) {
                    case 0:
                        DrawSprite(0x4A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 1:
                        DrawSprite(0x4B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 2:
                        DrawSprite(0x4C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 3:
                        DrawSprite(0x4D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                }
                if (D_800FA23C == 2) {
                    D_800FA258 = 3;
                    p2.vy = pos.vy;
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx - (b >= 0 ? b : -b);
                    }
                    DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x50);
                    }
                    DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    D_800FA258 = 1;
                }
            }
            if (D_800FA23C == 3) {
                col = hi;
            } else {
                col = norm;
            }
            pos.vx = t * 32 - 0x140;
            pos.vy = 0x28;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA23C == 4) {
                col = hi;
            } else {
                col = norm;
            }
            pos.vx = t * 32 - 0x118;
            pos.vy = 0x48;
            DrawSprite(0x3E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            d = D_800FA23C;
            p2.vx = d >= 3 ? -0x40 : -0x8E;
            p2.vy = d * 32 - 0x41;
            /* MATCHING: keeps the arguments below the stores. */
            __asm__("");
            func_800F8708(&p2, 2);
            if (gPadTrig & PADLup) {
                PlaySe(0xB);
                do {
                    i = 0;
                    if (D_800FA23C == 0) {
                        D_800FA23C = 4;
                    } else {
                        D_800FA23C--;
                    }
                    if (D_800FA23C == 1 && D_800FA254 == 0) {
                        i = 1;
                    }
                    if (D_800FA23C == 2 && *(s8 *)&gUnlocks == 1) {
                        i = 1;
                    }
                } while (i != 0);
            } else if (gPadTrig & PADLdown) {
                PlaySe(0xB);
                do {
                    i = 0;
                    if (D_800FA23C == 4) {
                        D_800FA23C = 0;
                    } else {
                        D_800FA23C++;
                    }
                    if (D_800FA23C == 1 && D_800FA254 == 0) {
                        i = 1;
                    }
                    if (D_800FA23C == 2 && *(s8 *)&gUnlocks == 1) {
                        i = 1;
                    }
                } while (i != 0);
            } else if (gPadTrig & PADLleft) {
                PlaySe(0xB);
                switch (D_800FA23C) {
                    case 0:
                        gStereo ^= 1;
                        if (gStereo != 0) {
                            SsSetStereo();
                        } else {
                            SsSetMono();
                        }
                        break;
                    case 1:
                        gRumbleEnabled ^= 1;
                        if (gRumbleEnabled != 0) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 0xFF;
                            gRumbleSmall[1] = 0xFF;
                        }
                        break;
                    case 2:
                        do {
                            if (gTmdSortMode == 0) {
                                gTmdSortMode = 3;
                            } else {
                                gTmdSortMode--;
                            }
                        } while (!((*(s8 *)&gUnlocks >> gTmdSortMode) & 1));
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
                        break;
                }
            } else if (gPadTrig & PADLright) {
                PlaySe(0xB);
                switch (D_800FA23C) {
                    case 0:
                        gStereo ^= 1;
                        if (gStereo != 0) {
                            SsSetStereo();
                        } else {
                            SsSetMono();
                        }
                        break;
                    case 1:
                        gRumbleEnabled ^= 1;
                        if (gRumbleEnabled != 0) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 0xFF;
                            gRumbleSmall[1] = 0xFF;
                        }
                        break;
                    case 2:
                        do {
                            if (gTmdSortMode == 3) {
                                gTmdSortMode = 0;
                            } else {
                                gTmdSortMode++;
                            }
                        } while (!((*(s8 *)&gUnlocks >> gTmdSortMode) & 1));
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
                        break;
                }
            } else if (gPadTrig & PADRdown) {
                PlaySe(0xE);
                D_800FA244 = 0;
                if (D_800FA23C != 4) {
                    D_800FA23C = 4;
                }
                gSubStep = 2003;
            } else if (gPadTrig & PADRright) {
                switch (D_800FA23C) {
                    case 3:
                        PlaySe(0xC);
                        D_800FA244 = 0;
                        gSubStep = 2003;
                        break;
                    case 4:
                        PlaySe(0xE);
                        D_800FA244 = 0;
                        gSubStep = 2003;
                        break;
                }
            }
            break;
        case 2003:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++D_800FA244;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x50;
            pos.vy = -0x58;
            DrawSprite(0x22, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 2;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = -0x38;
            DrawSprite(0x3A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0x98;
            if (gStereo != 0) {
                DrawSprite(0x46, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            } else {
                DrawSprite(0x47, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (PadInfoMode(0, 2, 0) != 0) {
                t = D_800FA244 - 4;
                if (t < 0) {
                    t = 0;
                }
                pos.vx = t * 32 - 0x80;
                pos.vy = -0x18;
                DrawSprite(0x3B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                if (gRumbleEnabled != 0) {
                    DrawSprite(0x48, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    DrawSprite(0x49, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
            }
            if (*(s8 *)&gUnlocks != 1) {
                t = D_800FA244 - 6;
                if (t < 0) {
                    t = 0;
                }
                pos.vx = t * 32 - 0x80;
                pos.vy = 8;
                DrawSprite(0x3C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                switch (gTmdSortMode) {
                    case 0:
                        DrawSprite(0x4A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 1:
                        DrawSprite(0x4B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 2:
                        DrawSprite(0x4C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 3:
                        DrawSprite(0x4D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                }
            }
            t = D_800FA244 - 8;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x40;
            pos.vy = 0x28;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 10;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x18;
            pos.vy = 0x48;
            DrawSprite(0x3E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 == 24) {
                if (D_800FA23C == 4) {
                    gSubStep = 100;
                } else {
                    gSubStep = 3000;
                }
            }
            break;
        case 3000:
            D_800FA244 = 0;
            gSubStep++;
            break;
        case 3001:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++D_800FA244;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x140;
            pos.vy = -0x58;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x128;
            pos.vy = -0x38;
            DrawSprite(gButtonConfig + 0x42, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 4;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x160;
            pos.vy = -0x18;
            DrawSprite(0x3F, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(D_800F9660[gButtonConfig][0] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 6;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x160;
            pos.vy = 8;
            DrawSprite(0x40, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(D_800F9660[gButtonConfig][1] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 == 18) {
                gSubStep = 3002;
            }
            break;
        case 3002:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = 8;
            pos.vx = t * 32 - 0x140;
            pos.vy = -0x58;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx = t * 32 - 0x128;
            pos.vy = -0x38;
            DrawSprite(gButtonConfig + 0x42, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x48);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx = t * 32 - 0x160;
            pos.vy = -0x18;
            DrawSprite(0x3F, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(D_800F9660[gButtonConfig][0] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx = t * 32 - 0x160;
            pos.vy = t;
            DrawSprite(0x40, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(D_800F9660[gButtonConfig][1] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            if (gPadTrig & PADLleft) {
                PlaySe(0xB);
                if (gButtonConfig == 0) {
                    gButtonConfig = 3;
                } else {
                    gButtonConfig--;
                }
            } else if (gPadTrig & PADLright) {
                PlaySe(0xB);
                if (gButtonConfig == 3) {
                    gButtonConfig = 0;
                } else {
                    gButtonConfig++;
                }
            } else if (gPadTrig & PADRright) {
                PlaySe(0xE);
                D_800FA244 = 0;
                gSubStep = 3003;
            } else if (gPadTrig & PADRdown) {
                PlaySe(0xE);
                D_800FA244 = 0;
                gSubStep = 3003;
            }
            break;
        case 3003:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++D_800FA244;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x40;
            pos.vy = -0x58;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 2;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x28;
            pos.vy = -0x38;
            DrawSprite(gButtonConfig + 0x42, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 4;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x60;
            pos.vy = -0x18;
            DrawSprite(0x3F, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(D_800F9660[gButtonConfig][0] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            t = D_800FA244 - 6;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x60;
            pos.vy = 8;
            DrawSprite(0x40, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(D_800F9660[gButtonConfig][1] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            if (D_800FA244 == 22) {
                gSubStep = 2000;
            }
            break;
        case 5000:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            for (i = 0; (s32)i < D_800FA268; i++) {
                pos.vx = -0x50;
                pos.vy = (s16)(i * 24 - 12) - D_800FA268 * 12;
                DrawSprite(D_800FA260[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            p2.vx = -0x88;
            p2.vy = 0x4F;
            func_800F8708(&p2, 3);
            pos.vx = -0x80;
            pos.vy = 0x58;
            switch (D_800FA228) {
                case 0:
                    DrawSprite(0x5A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        D_800FA228 = 10;
                    }
                    break;
                case 5:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    MemCardExist(0);
                    D_800FA228 = 6;
                    break;
                case 6:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &D_800FA270) == 1) {
                        switch (D_800FA270) {
                            case 0:
                                D_800FA228 = 12;
                                break;
                            case 1:
                                D_800FA238 = 0x59;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 2:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 3:
                                D_800FA228 = 10;
                                break;
                        }
                    }
                    break;
                case 10:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    MemCardAccept(0);
                    D_800FA228 = 11;
                    break;
                case 11:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &D_800FA270) == 1) {
                        switch (D_800FA270) {
                            case 0:
                            case 3:
                                D_800FA228 = 12;
                                break;
                            case 1:
                                D_800FA238 = 0x59;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 2:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 4:
                                D_800FA238 = 0x58;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                        }
                    }
                    break;
                case 12:
                    MemCardReadFile(0, D_800F0000, (unsigned long *)0x8016D000, 0x200, 0x80);
                    D_800FA228 = 13;
                    break;
                case 13:
                    DrawSprite(0x5D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &D_800FA270) == 1) {
                        switch (D_800FA270) {
                            case 0:
                                D_800FA228 = 20;
                                break;
                            case 1:
                                D_800FA238 = 0x59;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 2:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 3:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 4:
                                break;
                            case 5:
                                D_800FA238 = 0x58;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                        }
                    }
                    break;
                case 20:
                    D_800FA24C = (SaveData *)0x8016D000;
                    D_800FA240 = D_800FA24C->checksum;
                    D_800FA24C->checksum = 0;
                    D_800FA250 = 0;
                    for (i = 0; i < 0x5C; i++) {
                        D_800FA250 += ((u8 *)D_800FA24C)[i];
                    }
                    D_800FA250 = ~D_800FA250;
                    if (D_800FA250 != D_800FA240) {
                        D_800FA238 = 0x67;
                        D_800FA26C = 0;
                        D_800FA228 = 1000;
                        break;
                    }
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
                    D_800FA238 = 0x66;
                    D_800FA26C = 0;
                    D_800FA228 = 2000;
                    break;
                case 1000:
                    col.g = 0x80;
                    col.b = col.cd = 0;
                    DrawSprite(D_800FA238, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 30) {
                        gSubStep = 102;
                    }
                    break;
                case 2000:
                    DrawSprite(D_800FA238, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 30) {
                        gSubStep = 100;
                    }
                    break;
            }
            break;
        case 6000:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            for (i = 0; (s32)i < D_800FA268; i++) {
                pos.vx = -0x50;
                pos.vy = (s16)(i * 24 - 12) - D_800FA268 * 12;
                DrawSprite(D_800FA260[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            p2.vx = -0x88;
            p2.vy = 0x4F;
            func_800F8708(&p2, 3);
            pos.vx = -0x80;
            pos.vy = 0x58;
            switch (D_800FA228) {
                case 0:
                    DrawSprite(0x5B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        D_800FA26C = 0;
                        D_800FA228 = 10;
                    }
                    break;
                case 5:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 4) {
                        MemCardExist(0);
                        D_800FA228 = 6;
                    }
                    break;
                case 6:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &D_800FA270) == 1) {
                        switch (D_800FA270) {
                            case 0:
                                switch (MemCardCreateFile(0, D_800F0000, 1)) {
                                    case 0:
                                        D_800FA228 = 12;
                                        break;
                                    case 6:
                                        D_800FA228 = 13;
                                        break;
                                    case 1:
                                        D_800FA238 = 0x59;
                                        D_800FA26C = 0;
                                        D_800FA228 = 1000;
                                        break;
                                    case 2:
                                        D_800FA238 = 0x68;
                                        D_800FA26C = 0;
                                        D_800FA228 = 1000;
                                        break;
                                    case 7:
                                        D_800FA238 = 0x65;
                                        D_800FA26C = 0;
                                        D_800FA228 = 1000;
                                        break;
                                    case 3:
                                    case 5:
                                        break;
                                    case 4:
                                        D_800FA26C = 0;
                                        D_800FA228 = 500;
                                        break;
                                }
                                break;
                            case 3:
                                D_800FA228 = 10;
                                break;
                            case 1:
                                D_800FA238 = 0x59;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 2:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                        }
                    }
                    break;
                case 10:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    MemCardAccept(0);
                    D_800FA228 = 11;
                    break;
                case 11:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &D_800FA270) == 1) {
                        switch (D_800FA270) {
                            case 0:
                            case 3:
                                switch (MemCardCreateFile(0, D_800F0000, 1)) {
                                    case 0:
                                        D_800FA228 = 12;
                                        break;
                                    case 6:
                                        D_800FA228 = 13;
                                        break;
                                    case 1:
                                        D_800FA238 = 0x59;
                                        D_800FA26C = 0;
                                        D_800FA228 = 1000;
                                        break;
                                    case 2:
                                        D_800FA238 = 0x68;
                                        D_800FA26C = 0;
                                        D_800FA228 = 1000;
                                        break;
                                    case 7:
                                        D_800FA238 = 0x65;
                                        D_800FA26C = 0;
                                        D_800FA228 = 1000;
                                        break;
                                    case 3:
                                    case 5:
                                        break;
                                    case 4:
                                        D_800FA26C = 0;
                                        D_800FA228 = 500;
                                        break;
                                }
                                break;
                            case 1:
                                D_800FA238 = 0x59;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 2:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 4:
                                D_800FA26C = 0;
                                D_800FA228 = 500;
                                break;
                        }
                    }
                    break;
                case 12:
                    for (i = 0; (s32)i < 0x200; i++) {
                        ((u8 *)0x8016D000)[i] = D_800F9460[i];
                    }
                    D_800FA24C = (SaveData *)0x8016D200;
                    D_800FA24C->times[0].a = gBestTimes[0].a;
                    D_800FA24C->times[1].a = gBestTimes[1].a;
                    D_800FA24C->times[2].a = gBestTimes[2].a;
                    D_800FA24C->times[3].a = gBestTimes[3].a;
                    D_800FA24C->times[0].b = gBestTimes[0].b;
                    D_800FA24C->times[1].b = gBestTimes[1].b;
                    D_800FA24C->times[2].b = gBestTimes[2].b;
                    D_800FA24C->times[3].b = gBestTimes[3].b;
                    D_800FA24C->cans[0].a = gBestCans[0].a;
                    D_800FA24C->cans[1].a = gBestCans[1].a;
                    D_800FA24C->cans[2].a = gBestCans[2].a;
                    D_800FA24C->cans[3].a = gBestCans[3].a;
                    D_800FA24C->cans[0].b = gBestCans[0].b;
                    D_800FA24C->cans[1].b = gBestCans[1].b;
                    D_800FA24C->cans[2].b = gBestCans[2].b;
                    D_800FA24C->cans[3].b = gBestCans[3].b;
                    D_800FA24C->unk48.a = D_80095978.a;
                    D_800FA24C->unk48.b = D_80095978.b;
                    D_800FA24C->stereo = gStereo;
                    D_800FA24C->rumble = gRumbleEnabled;
                    D_800FA24C->sortMode = gTmdSortMode;
                    D_800FA24C->buttonConfig = gButtonConfig;
                    D_800FA24C->unk54 = D_800957A0;
                    D_800FA24C->gameCleared = gGameCleared;
                    D_800FA24C->allTargets = gAllTargetsBeaten;
                    D_800FA24C->unlocks = gUnlocks;
                    D_800FA24C->checksum = 0;
                    D_800FA250 = 0;
                    for (i = 0; i < 0x5C; i++) {
                        D_800FA250 += ((u8 *)D_800FA24C)[i];
                    }
                    D_800FA250 = ~D_800FA250;
                    D_800FA24C->checksum = D_800FA250;
                    MemCardWriteFile(0, D_800F0000, (unsigned long *)0x8016D000, 0, 0x280);
                    D_800FA228 = 15;
                    break;
                case 13:
                    DrawSprite(0x5C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        D_800FA24C = (SaveData *)0x8016D000;
                        D_800FA24C->times[0].a = gBestTimes[0].a;
                        D_800FA24C->times[1].a = gBestTimes[1].a;
                        D_800FA24C->times[2].a = gBestTimes[2].a;
                        D_800FA24C->times[3].a = gBestTimes[3].a;
                        D_800FA24C->times[0].b = gBestTimes[0].b;
                        D_800FA24C->times[1].b = gBestTimes[1].b;
                        D_800FA24C->times[2].b = gBestTimes[2].b;
                        D_800FA24C->times[3].b = gBestTimes[3].b;
                        D_800FA24C->cans[0].a = gBestCans[0].a;
                        D_800FA24C->cans[1].a = gBestCans[1].a;
                        D_800FA24C->cans[2].a = gBestCans[2].a;
                        D_800FA24C->cans[3].a = gBestCans[3].a;
                        D_800FA24C->cans[0].b = gBestCans[0].b;
                        D_800FA24C->cans[1].b = gBestCans[1].b;
                        D_800FA24C->cans[2].b = gBestCans[2].b;
                        D_800FA24C->cans[3].b = gBestCans[3].b;
                        D_800FA24C->unk48.a = D_80095978.a;
                        D_800FA24C->unk48.b = D_80095978.b;
                        D_800FA24C->stereo = gStereo;
                        D_800FA24C->rumble = gRumbleEnabled;
                        D_800FA24C->sortMode = gTmdSortMode;
                        D_800FA24C->buttonConfig = gButtonConfig;
                        D_800FA24C->unk54 = D_800957A0;
                        D_800FA24C->gameCleared = gGameCleared;
                        D_800FA24C->allTargets = gAllTargetsBeaten;
                        D_800FA24C->unlocks = gUnlocks;
                        D_800FA24C->checksum = 0;
                        D_800FA250 = 0;
                        for (i = 0; i < 0x5C; i++) {
                            D_800FA250 += ((u8 *)D_800FA24C)[i];
                        }
                        D_800FA250 = ~D_800FA250;
                        D_800FA24C->checksum = D_800FA250;
                        MemCardWriteFile(0, D_800F0000, (unsigned long *)0x8016D000, 0x200, 0x80);
                        D_800FA228 = 15;
                    }
                    break;
                case 15:
                    DrawSprite(0x5E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &D_800FA270) == 1) {
                        switch (D_800FA270) {
                            case 0:
                                D_800FA238 = 0x66;
                                D_800FA26C = 0;
                                D_800FA228 = 2000;
                                break;
                            case 1:
                                D_800FA238 = 0x59;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 2:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 3:
                                D_800FA238 = 0x68;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                            case 4:
                                break;
                            case 5:
                                D_800FA238 = 0x58;
                                D_800FA26C = 0;
                                D_800FA228 = 1000;
                                break;
                        }
                    }
                    break;
                case 500:
                    col.g = 0x80;
                    col.b = col.cd = 0;
                    DrawSprite(0x61, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 30) {
                        D_800FA228 = 501;
                    }
                    break;
                case 501:
                    DrawSprite(0x62, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        D_800FA26C = 0;
                        D_800FA228 = 502;
                    }
                    break;
                case 502:
                    DrawSprite(0x63, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 8) {
                        D_800FA228 = 503;
                    }
                    break;
                case 503:
                    DrawSprite(0x63, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    switch (MemCardFormat(0)) {
                        case 0:
                            D_800FA26C = 0;
                            D_800FA228 = 10;
                            break;
                        case 1:
                            D_800FA238 = 0x59;
                            D_800FA26C = 0;
                            D_800FA228 = 1000;
                            break;
                        case 2:
                            D_800FA238 = 0x68;
                            D_800FA26C = 0;
                            D_800FA228 = 1000;
                            break;
                    }
                    break;
                case 504:
                    DrawSprite(0x66, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    D_800FA26C++;
                    break;
                case 1000:
                    col.g = 0x80;
                    col.b = col.cd = 0;
                    DrawSprite(D_800FA238, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 30) {
                        gSubStep = 102;
                    }
                    break;
                case 2000:
                    DrawSprite(D_800FA238, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++D_800FA26C == 30) {
                        gSubStep = 100;
                    }
                    break;
            }
            break;
    }
    D_800FA230.vy += 11;
    func_800F88C8(&D_800FA230);
    StepStepper(&gPlayer);
    switch (D_800FA258) {
        case 0:
            func_800F1494(2);
            break;
        case 1:
            func_800F1494(1);
            break;
        case 2:
            func_800F1494(0);
            break;
        case 3:
            DrawStepper(&gPlayer);
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F1570);
#endif

extern CdlLOC D_800F96F8;
extern CdlLOC D_800F96FC;
extern s32 D_800F9748;
extern s32 D_800F974C;
void func_800F0AD4(void);
void func_800F1570(void);
void func_800F69B0(void);
s32 func_800F7A4C(void);

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

inline void func_800F80F8(void) {
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
    size.vy = size.vx / 2;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x40;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

inline void func_800F81B4(void) {
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

extern s32 D_800FA278;
extern s32 D_800FA280;
extern s32 D_800FA284;

#ifdef NON_MATCHING
void func_800F69B0(void) {
    SVECTOR pos;
    SColor color;
    MATRIX m;
    POLY_F4 *p;
    DR_TPAGE *t;
    s32 *dir;
    s32 *tbl;
    s32 d;

    switch (gSubStep) {
        case 50:
            DefineSprite(0x1E, 1, 0xFF, 0x60, 0x17, 0, 0, 0, 0x1E1);
            DefineSprite(0x1F, 0, 0xFF, 0x10, 0x17, 0, 0x80, 0x40, 0x1E0);
            DefineSprite(0x20, 0, 0xFF, 0x10, 0x17, 0, 0x90, 0x40, 0x1E0);
            DefineSprite(0x21, 0, 0xFF, 0x20, 0x17, 0, 0xA0, 0x40, 0x1E0);
            DefineSprite(0x22, 0, 0xFF, 0x20, 0x17, 0, 0xC0, 0x40, 0x1E0);
            tbl = (s32 *)0x8019D010;
            dir = (s32 *)0x8019D000;
            for (gLoopI = 0; gLoopI < 2; gLoopI++) {
                gAnimations[gLoopI + 51] = (s32 *)(tbl[gLoopI] + (s32)dir) + 1;
                gPlayer.motionLen[gLoopI + 51] = *gAnimations[gLoopI + 51];
                gAnimations[gLoopI + 51]++;
                gPlayer.motionStart[gLoopI + 51] = gAnimations[gLoopI + 51][1];
            }
            GsSetProjection(250);
            SetFogNearFar(2000, 6000, 250);
            SetFarColor(0, 0, 0);
            GsMapModelingData((unsigned long *)(*(s32 *)0x8019D00C + 0x8019D004));
            GsLinkObject4(*(s32 *)0x8019D00C + 0x8019D00C, &sAct3Objs[0], 0);
            sAct3Objs[0].coord2 = &sAct3Coords[0];
            sAct3Objs[0].attribute = 0x200;
            D_8009EB58[0].vx = 0;
            D_8009EB58[0].vy = 0;
            D_8009EB58[0].vz = 0;
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
            sView.vpx = 0;
            sView.vpy = -150;
            sView.vpz = 0;
            sView.vrx = -1000;
            sView.vry = -150;
            sView.vrz = 0;
            gPlayer.x = -10000;
            gPlayer.yaw = -0x400;
            gPlayer.action = 0x61;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.motion[0] = 3;
            PlayCdTrack(7, 0);
            gSubStep = 100;
            break;
        case 100:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            gLoopI = ((gPlayer.x >= 0 ? gPlayer.x : -gPlayer.x) - 1000) / 20;
            if (gLoopI < 0) {
                gLoopI = 0;
            }
            if (gLoopI >= 256) {
                gLoopI = 255;
            }
            setXY4(p, -160, -120, 160, -120, -160, 120, 160, 120);
            p->r0 = p->g0 = p->b0 = gLoopI;
            addPrim(gOt[gActiveBuff].org, p);
            p++;
            GsOUT_PACKET_P = (PACKET *)p;
            t = (DR_TPAGE *)GsOUT_PACKET_P;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(gOt[gActiveBuff].org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            gPlayer.x += 50;
            if (gPlayer.x >= -250) {
                D_800FA280 = 0;
                gSubStep = 110;
            }
            break;
        case 110:
            gPlayer.x += 50;
            sView.vpx = gPlayer.x + rsin((-D_800FA280 * 12 + 90) * 4096 / 360) * 250 / 4096;
            sView.vpy = -150;
            sView.vpz = rcos((-D_800FA280 * 12 + 90) * 4096 / 360) * 150 / 4096;
            sView.vry = -150;
            sView.vrz = 0;
            sView.vrx = gPlayer.x;
            if (++D_800FA280 == 15) {
                D_800FA280 = 250;
                gSubStep = 120;
            }
            break;
        case 120:
            sView.vpy = -150;
            sView.vpz = 0;
            sView.vry = -150;
            sView.vrz = 0;
            gPlayer.x += 50;
            sView.vpx = gPlayer.x - D_800FA280;
            sView.vrx = gPlayer.x;
            if ((D_800FA280 += 5) == 500) {
                D_800FA278 = 0;
                gSubStep = 130;
            }
            break;
        case 130:
            gPlayer.x += 50;
            if (D_800FA278 == 0) {
                if ((D_800FA280 -= 5) == 300) {
                    D_800FA278 = 1;
                }
            } else {
                if ((D_800FA280 += 5) == 500) {
                    D_800FA278 = 0;
                }
            }
            sView.vpy = -150;
            sView.vpz = 0;
            sView.vry = -150;
            sView.vrz = 0;
            sView.vpx = gPlayer.x - D_800FA280;
            sView.vrx = gPlayer.x;
            if (gPlayer.x == 11000) {
                D_800FA280 = 0;
                PlaySe(0x23);
                gPlayer.action = 0x60;
                gPlayer.motion[0] = 0x33;
                gSubStep = 140;
            }
            break;
        case 140:
            if (D_800FA280 < 5) {
                d = D_800FA280 * 50 - 500;
                sView.vpx = gPlayer.x + d;
                sView.vrx = gPlayer.x;
                sView.vry = sView.vpy = rand() % 50 - 175;
                sView.vrz = sView.vpz = rand() % 50 - 25;
            } else if (D_800FA280 < 10) {
                d = (D_800FA280 - 5) * 50 + 250;
                sView.vpx = gPlayer.x - d;
                sView.vrx = gPlayer.x;
                sView.vry = sView.vpy = rand() % 50 - 175;
                sView.vrz = sView.vpz = rand() % 50 - 25;
            } else {
                sView.vry = sView.vpy = -150;
                sView.vrz = sView.vpz = 0;
                sView.vpx = gPlayer.x - 500;
                sView.vrx = gPlayer.x;
            }
            if (++D_800FA280 == 31) {
                gSubStep = 1000;
            }
            break;
        case 1000:
            gPlayer.x = 11000;
            gPlayer.yaw = -0x400;
            gPlayer.action = 0x61;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.motion[0] = 0x34;
            sView.vpx = 0x2904;
            sView.vrx = 11000;
            sView.vpy = sView.vry = -150;
            sView.vpz = sView.vrz = 0;
            D_800FA280 = 0;
            D_800FA284 = 0;
            D_800FA278 = 0;
            gSubStep = 1001;
            PlaySe(0x50);
            break;
        case 1001:
            if (gPadTrig & PADstart) {
                if (D_800FA278 == 0) {
                    MemCardAccept(0);
                    PlaySe(0x22);
                    gPlayer.yaw = 0x400;
                    gPlayer.action = 0x60;
                    gPlayer.motion[0] = 0xF;
                    D_800FA284 = 1;
                    D_800FA278 = 1;
                }
            }
            if (D_800FA284 != 0) {
                if (++D_800FA284 == 36) {
                    StopCdTrack();
                    gSubStep = 50;
                    gModeStep = 30;
                    StartScreenWipe();
                }
            } else {
                if (++D_800FA280 == 450) {
                    StopCdTrack();
                    gSubStep = 50;
                    gModeStep = 2;
                }
            }
            break;
    }
    GsSetRefView2(&sView);
    if (gPlayer.motion[0] == 3 && !(gFrameCount & 7)) {
        PlaySe(0x13);
    }
    if (gSubStep >= 1000) {
        if (D_800FA278 == 0 || (gFrameCount & 1)) {
            color.r = 1;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -128;
            pos.vy = 16;
            DrawSprite(0x1F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
            color.r = 2;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -128;
            pos.vy = 16;
            DrawSprite(0x20, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        color.r = -1;
        color.g = color.b = color.cd = 0x80;
        pos.vx = -128;
        pos.vy = 0x48;
        DrawSprite(0x21, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        color.r = 2;
        color.g = color.b = color.cd = 0x80;
        pos.vx = -128;
        pos.vy = 0x48;
        DrawSprite(0x22, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    StepStepper(&gPlayer);
    DrawStepper(&gPlayer);
    func_800F80F8();
    sAct3Coords[0].coord.t[0] = 11000;
    sAct3Coords[0].coord.t[1] = -234;
    sAct3Coords[0].coord.t[2] = 0;
    m = GsIDMATRIX;
    m.t[0] = sAct3Coords[0].coord.t[0];
    m.t[1] = sAct3Coords[0].coord.t[1];
    m.t[2] = sAct3Coords[0].coord.t[2];
    RotMatrixYXZ(&D_8009EB58[0], &m);
    sAct3Coords[0].coord = m;
    sAct3Coords[0].flg = 0;
    GsGetLs(sAct3Objs[0].coord2, &m);
    GsSetLsMatrix(&m);
    pos.vx = 0x1FE;
    pos.vy = 0xC0;
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    DrawBillboard(0x1E, &pos, (CVECTOR *)&color, 2, &gOt[gActiveBuff]);
    sAct3Coords[0].coord.t[0] = 0;
    sAct3Coords[0].coord.t[1] = 0;
    sAct3Coords[0].coord.t[2] = 0;
    m = GsIDMATRIX;
    m.t[0] = sAct3Coords[0].coord.t[0];
    m.t[1] = sAct3Coords[0].coord.t[1];
    m.t[2] = sAct3Coords[0].coord.t[2];
    RotMatrixYXZ(&D_8009EB58[0], &m);
    sAct3Coords[0].coord = m;
    sAct3Coords[0].flg = 0;
    GsGetLs(sAct3Objs[0].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sAct3Objs[0].coord2, &m);
    GsSetLightMatrix(&m);
    if (gPlayer.x != -10000) {
        GsSortObject4J(&sAct3Objs[0], &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
    }
    func_800F81B4();
    if ((gPadTrig & PADstart) && gSubStep < 1000) {
        gSubStep = 1000;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F69B0);
#endif

extern CdlLOC D_800F9BC8;
extern SVECTOR D_800F9BD0[];
extern s32 D_800FA27C;

#ifdef NON_MATCHING
s32 func_800F7A4C(void) {
    SVECTOR pos;
    SColor color;
    MATRIX m;

    switch (gSubStep) {
        case 0:
            DefineSprite(0x1E, 1, 0xFF, 0x20, 5, 0, 0, 0, 0x1E0);
            DefineSprite(0x1F, 1, 0xFF, 0x20, 5, 0, 0x20, 0, 0x1E0);
            GsInitCoordinate2(&sAct3Coords[0], NULL);
            GsMapModelingData((unsigned long *)(*(s32 *)0x8019D004 + 0x8019D004));
            GsLinkObject4(*(s32 *)0x8019D004 + 0x8019D00C, &sAct3Objs[0], 0);
            sAct3Objs[0].coord2 = &sAct3Coords[0];
            D_8009EA78[0].vx = 0;
            sAct3Objs[0].attribute = 0;
            D_8009EA78[0].vy = -1000;
            D_8009EA78[0].vz = 0;
            D_8009EB58[0].vx = 0;
            D_8009EB58[0].vy = -0x2CC;
            D_8009EB58[0].vz = 0;
            sView.vrx = 0;
            sView.vry = -240;
            sView.vrz = 0;
            sView.vpx = 0x226;
            sView.vpy = -240;
            sView.vpz = 0x226;
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
            D_800FA27C = 0;
            CdControlF(CdlSeekL, (u_char *)&D_800F9BC8);
            CdSync(0, NULL);
            PlayXa(5);
            gSubStep++;
            break;
        case 1:
            if (++D_800FA27C < 10) {
                D_8009EA78[0].vy += 100;
            } else if (D_800FA27C < 35) {
                D_8009EA78[0].vy = 0;
                D_8009EB58[0].vx = D_800F9BD0[D_800FA27C - 10].vx;
                D_8009EB58[0].vy = D_800F9BD0[D_800FA27C - 10].vy;
                D_8009EB58[0].vz = D_800F9BD0[D_800FA27C - 10].vz;
            }
            if ((u32)(D_800FA27C - 31) < 63) {
                color.r = 1;
                color.g = color.b = color.cd = (D_800FA27C - 30) * 2;
                pos.vx = -128;
                pos.vy = -16;
                DrawSprite(0x1E, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                color.r = 2;
                pos.vx = -128;
                pos.vy = -16;
                color.g = color.b = color.cd = (D_800FA27C - 30) * 2;
                DrawSprite(0x1F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
            }
            if (D_800FA27C >= 94) {
                color.r = 1;
                color.g = color.b = color.cd = 0x80;
                pos.vx = -128;
                pos.vy = -16;
                DrawSprite(0x1E, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                color.r = 2;
                color.g = color.b = color.cd = 0x80;
                pos.vx = -128;
                pos.vy = -16;
                DrawSprite(0x1F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
            }
            sAct3Coords[0].coord.t[0] = D_8009EA78[0].vx;
            sAct3Coords[0].coord.t[2] = D_8009EA78[0].vz;
            sAct3Coords[0].coord.t[1] = D_8009EA78[0].vy;
            m = GsIDMATRIX;
            m.t[0] = sAct3Coords[0].coord.t[0];
            m.t[1] = sAct3Coords[0].coord.t[1];
            m.t[2] = sAct3Coords[0].coord.t[2];
            RotMatrixYXZ(&D_8009EB58[0], &m);
            sAct3Coords[0].coord = m;
            sAct3Coords[0].flg = 0;
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[0].coord2, &m);
            GsSetLightMatrix(&m);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            sAct3Coords[0].flg = 0;
            sAct3Coords[0].coord.m[1][0] *= -1;
            sAct3Coords[0].coord.m[1][1] *= -1;
            sAct3Coords[0].coord.m[1][2] *= -1;
            sAct3Coords[0].coord.t[1] = -sAct3Coords[0].coord.t[1];
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            SetTmdSortMode(5);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            if (D_800FA27C == 150) {
                if (gStageTableSet == 0) {
                    D_800957A0 = gStage;
                    D_800958C0.a = gActTimes.a;
                    D_800958C0.b = gActTimes.b;
                    D_80095978.a = gActCans.a;
                    D_80095978.b = gActCans.b;
                } else if (gStageTableSet == 1) {
                    D_800957A0 = gStage + 100;
                    D_800958C0.a = gActTimes.a;
                    D_800958C0.b = gActTimes.b;
                    D_80095978.a = gActCans.a;
                    D_80095978.b = gActCans.b;
                }
                StopXa();
                gModeStep = 0;
                gSubStep = 50;
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F7A4C);
#endif

inline void func_800F8264(void) {
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

inline void func_800F83E0(void) {
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

inline void func_800F8708(SVECTOR *pos, s32 kind) {
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

inline void func_800F8840(PlayerState *obj) {
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

inline void func_800F8898(void) {
    D_800FA288 = 0;
    D_800FA28C = 2;
}

inline void func_800F88B0(void) {
    D_800FA288 = 0;
    D_800FA28C = 3;
}

inline void func_800F88C8(SVECTOR *rot) {
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
