#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_7d74.h"
#include "spad.h"

/** @brief Eight bytes, copied together as one unaligned block. */
typedef struct {
    u8 b[8]; /**< the bytes */
} Bytes8;

extern s32 gWipeFrame;
extern s32 gTallyCans;
extern s32 gResultsActive;
extern s32 gResultsTimer;
extern s32 gWipeActive;
extern s32 gResultsStep;
extern s8 gResultsSliding[8];
extern s8 gResultsSlide[8];
extern u8 D_800956B0[];
extern u8 D_800956B8[];

/** @brief An 8-byte boundary record: a point and the direction its sign
 *         test projects onto. */
typedef struct {
    s16 x;  /**< the point's x */
    s16 y;  /**< the point's y */
    s16 dx; /**< the direction's x */
    s16 dy; /**< the direction's y */
} Edge;

extern Edge *gBoundaryEdges;

extern GsDOBJ2 gStageObjs[];
extern u32 gPropObjCount;
extern GsDOBJ2 gPropObjs[];
extern GsCOORDINATE2 gStageCoord;

extern s32 gCpuMarkTimes[];
extern u8 D_800AC848[];
extern u8 D_800A7888[];
extern u8 D_800A76E8[];

/** @brief A run of entries in a draw list: where it starts and how long. */
typedef struct {
    s16 start; /**< the first entry's index */
    s16 count; /**< the number of entries */
} Run;

/* MATCHING: a per-unit view; code_1a098 types the same list as Zone4. */
extern Run *gStageRuns;
extern Run *gPropRuns;
extern s8 *gPropRunObjs;

extern DR_STP D_800DFA90;
extern DR_ENV D_800DFA10;
extern DR_STP D_800DFAA0;
extern DR_ENV D_800DFA50;

void SetLetterbox(s32 clip) {
    DRAWENV env;
    /* MATCHING: retail's frame has 8 more bytes above the environment. */
    s32 unused[2];

    if (clip) {
        GetDrawEnv(&env);
        env.clip.x = 0;
        env.clip.y = D_800E474C * 240 + 32;
        env.clip.w = 319;
        env.clip.h = 175;
        SetDrawStp(&D_800DFA90, 1);
        addPrim(gOt[gActiveBuff].org + 0xFFF, &D_800DFA90);
        SetDrawEnv(&D_800DFA10, &env);
        addPrim(gOt[gActiveBuff].org + 0xFFF, &D_800DFA10);
        env.clip.x = 0;
        env.clip.y = D_800E474C * 240;
        env.clip.w = 319;
        env.clip.h = 239;
        SetDrawStp(&D_800DFAA0, 1);
        addPrim(gSubOt[gActiveBuff].org, &D_800DFAA0);
        SetDrawEnv(&D_800DFA50, &env);
        addPrim(gSubOt[gActiveBuff].org, &D_800DFA50);
    }
}

/* MATCHING: per-unit views; this loader stores the stage pointers plainly. */
extern Edge *gSectionEdges;
extern s32 D_80095840;
extern s32 D_80095790;
extern s8 *D_80095808;
extern s32 gSkyColors;
extern s32 D_80095948;
extern s32 D_8009594C;

void LoadStageData(unsigned long *hdr) {
    u32 i;
    GsDOBJ2 *obj;
    unsigned long *tmd;

    tmd = (unsigned long *)(hdr[0] + (s32)hdr);
    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
    tmd = (unsigned long *)(hdr[1] + (s32)hdr);
    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
    gSection = 0;
    D_800957F4 = 0;
    gFloorData = hdr[2] + (s32)hdr;
    gSectionEdges = (Edge *)(hdr[3] + (s32)hdr + 4);
    gBoundaryEdges = (Edge *)(hdr[4] + (s32)hdr + 4);
    D_80095948 = *(s32 *)(hdr[5] + (s32)hdr);
    gStageRuns = (Run *)(hdr[5] + (s32)hdr + 4);
    gStageRunObjs = (s8 *)(hdr[5] + (s32)hdr + D_80095948 * 4 + 4);
    D_8009594C = *(s32 *)(hdr[6] + (s32)hdr);
    gPropRuns = (Run *)(hdr[6] + (s32)hdr + 4);
    D_80095840 = hdr[7] + (s32)hdr;
    D_80095808 = (s8 *)(hdr[8] + (s32)hdr);
    gFogColors = (u8 *)(hdr[9] + (s32)hdr);
    D_80095790 = hdr[10] + (s32)hdr;
    gSkyColors = hdr[11] + (s32)hdr;
    gPropRunObjs = (s8 *)(hdr[6] + (s32)hdr + D_8009594C * 4 + 4);
}

extern GsDOBJ2 D_800CEF00[];
extern GsCOORDINATE2 D_800CEF40[];
extern GsDOBJ2 D_80096008[];
extern GsCOORDINATE2 D_800CDF20[];

/* MATCHING: D_80096008 is the piece table's true start (undefined_syms.*.txt). */

/** @brief The head of the game state as the model loader writes it. */
typedef struct {
    u8 unk0[0x20];   /**< not yet known */
    s32 unk20[100];  /**< first word of each loaded entry */
    s32 unk1B0[100]; /**< third word of each loaded entry */
} SeqHead;

#define sSeqHead (*(SeqHead *)gPlayer)

/**
 * @brief Maps the stage TMD and links its objects to the stage object table.
 * @param tmd the file, at its leading word
 */
static __inline__ void linkStage(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
}

/**
 * @brief Maps the props TMD and links its objects to the props object table.
 * @param tmd the file, at its leading word
 */
static __inline__ void linkProps(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
}

void LoadAct3Data(s32 *pack) {
    s32 i;
    s32 n;
    u8 *t;

    linkStage((unsigned long *)(pack[0] + (s32)pack));
    linkProps((unsigned long *)(pack[1] + (s32)pack));
    GsMapModelingData((unsigned long *)(pack[2] + (s32)pack + 4));
    n = *(s32 *)(pack[2] + (s32)pack + 8);
    for (i = 0; i < n; i++) {
        GsLinkObject4((unsigned long)(pack[2] + (s32)pack + 12), &D_800CEF00[i], i);
        D_800CEF00[i].coord2 = &D_800CEF40[i];
        D_800CEF00[i].attribute = 0;
    }
    GsMapModelingData((unsigned long *)(pack[3] + (s32)pack + 4));
    n = *(s32 *)(pack[3] + (s32)pack + 8);
    for (i = 0; i < n; i++) {
        GsLinkObject4((unsigned long)(pack[3] + (s32)pack + 12), &D_80096008[i + 10], i);
        D_80096008[i + 10].coord2 = &D_800CDF20[i];
        D_80096008[i + 10].attribute = 0;
    }
    t = (u8 *)(pack[4] + (s32)pack);
    n = *(s32 *)t / 4;
    for (i = 0; i < n; i++) {
        gAnimations[i + 51] = (s32 *)(((s32 *)t)[i] + (s32)t) + 1;
        sSeqHead.unk20[i + 51] = *gAnimations[i + 51];
        gAnimations[i + 51]++;
        sSeqHead.unk1B0[i + 51] = gAnimations[i + 51][1];
    }
    DefineSprite(0x1E, 0, 0xFF, 0x40, 0x1C, 0, 0, 0, 0x1FE);
    DefineSprite(0x1F, 0, 0xFF, 0x40, 0x1C, 0, 0x40, 0, 0x1FE);
    DefineSprite(0x20, 0, 0xFF, 0x40, 0x1C, 0, 0x80, 0, 0x1FE);
    DefineSprite(0x21, 0, 0xFF, 0x40, 0x1C, 0, 0xC0, 0, 0x1FE);
}

void DrawStage(s16 x, s16 y, s16 z) {
    s32 i;
    s32 k;
    SVECTOR rot;
    MATRIX ls;

    gStageCoord.coord.t[0] = x;
    gStageCoord.coord.t[1] = y;
    gStageCoord.coord.t[2] = z;
    rot.vx = rot.vy = rot.vz = 0;
    SetCoordRotation(&rot, &gStageCoord);
    GsGetLs(&gStageCoord, &ls);
    GsSetLsMatrix(&ls);
    SetSpadStack();
    for (i = 0; i < gStageRuns[gSection].count; i++) {
        k = gStageRunObjs[gStageRuns[gSection].start + i];
        if ((u32)k < gStageObjCount) {
            GsSortObject4J(&gStageObjs[k], gCurOt, 2, (u_long *)0x1F800000);
        }
    }
    for (i = 0; i < gPropRuns[gSection].count; i++) {
        k = gPropRunObjs[gPropRuns[gSection].start + i];
        if ((u32)k < gPropObjCount && k != -1) {
            GsSortObject4J(&gPropObjs[k], gCurOt, 2, (u_long *)0x1F800000);
        }
    }
    ResetSpadStack();
}

void DrawRepeatingStage(s32 pos, s32 unused, s32 z, s32 range) {
    s32 i;
    s32 k;
    s32 d;
    s32 r;
    SVECTOR rot;
    MATRIX ls;

    for (i = -1; i < 2; i++) {
        k = (((pos + 10000000) / 5000 + i) & 1) + 1;
        d = pos - i * 5000;
        if (d >= 0) {
            if (d >= 5000) {
                continue;
            }
            k = 0;
        }
        if (d < -range) {
            if (d < -(range + 5000)) {
                continue;
            }
            k = 3;
        }
        if (pos >= 0) {
            r = pos % 5000;
        } else {
            r = (pos + 10000000) % 5000;
        }
        gStageCoord.coord.t[0] = r + (i - 1) * 5000;
        gStageCoord.coord.t[1] = 0;
        gStageCoord.coord.t[2] = z;
        rot.vx = rot.vy = rot.vz = 0;
        SetCoordRotation(&rot, &gStageCoord);
        GsGetLs(&gStageCoord, &ls);
        GsSetLsMatrix(&ls);
        GsSortObject4J(&gStageObjs[k], gCurOt, 2, (u_long *)0x1F800000);
        GsSortObject4J(&gPropObjs[k], gCurOt, 2, (u_long *)0x1F800000);
    }
}

void UpdateSection(s32 *index, s16 x, s16 y) {
    s32 i;
    s32 j;
    s32 d;
    Edge *e;
    Edge *f;

    /* MATCHING: integer address sums put the scaled index first; pointer
     * arithmetic puts the base first and changes the register choice. */
    i = *index;
    e = (Edge *)(i * 8 + (s32)gBoundaryEdges) + 1;
    d = e->dx * (x - e->x) + e->dy * (y - e->y);
    if (d >= 0) {
        *index = i + 1;
    }
    j = *index;
    f = (Edge *)(j * 8 + (s32)gBoundaryEdges);
    d = -f->dx * (x - f->x) + -f->dy * (y - f->y);
    if (d >= 0) {
        *index = j - 1;
    }
}

/**
 * @brief Draws one glyph of the time display and steps the pen left.
 * @param pos the pen position
 * @param id  the glyph's sprite id
 */
static __inline__ void putDigit(SVECTOR *pos, u16 id) {
    pos->vx -= 6;
    DrawSprite(id, pos, NULL, 0, &gSubOt[gActiveBuff]);
}

void DrawHudTime(SVECTOR *pos, s32 frames) {
    s32 n;
    s32 m;
    s32 d;
    s32 k;
    s32 t;
    s32 sec;

    n = frames * 100 / 30;
    /* MATCHING: d copied from n, then divided; d = n / 10 gives d a
     * different register. */
    d = n;
    d /= 10;
    putDigit(pos, n % 10 + 0x10F);
    putDigit(pos, d % 10 + 0x10F);
    putDigit(pos, 0x119);
    sec = frames / 30;
    m = sec / 60;
    n = sec % 60;
    k = 1;
    d = n;
    d /= 10;
    putDigit(pos, n % 10 + 0x10F);
    putDigit(pos, d % 10 + 0x10F);
    /* MATCHING: the minutes move into n between the digit and the colon. */
    n = m;
    putDigit(pos, 0x119);
    for (; k < 11; k *= 10) {
        t = n / k;
        if (t != 0 || (k == 1 && n == 0)) {
            putDigit(pos, t % 10 + 0x10F);
        }
    }
}

void DefineHudSprites(void) {
    s32 i;

    DefineSprite(0xFA, 0, 0x40, 0x40, 0x1E, 0, 0, 0, 0x1F9);
    /* MATCHING: the u ternary tests i >= 6 first; i < 6 swaps the two
     * strength-reduced registers. */
    for (i = 0; i < 10; i++) {
        DefineSprite(i + 0xFB, 0, 0x20, 0x20, 0x1E, (u8)(i >= 6 ? i * 32 - 0x80 : i * 32 + 0x40),
                     i < 6 ? 0 : 0x20, 0x10, 0x1F9);
    }
    for (i = 0; i < 10; i++) {
        DefineSprite(i + 0x105, 0, 0x10, 0x10, 0x1E, (u8)(i * 16), 0x40, 0x20, 0x1F9);
    }
    for (i = 0; i < 11; i++) {
        DefineSprite(i + 0x10F, 0, 8, 8, 0x1E, (u8)(i * 8), 0x58, 0x30, 0x1F9);
    }
    for (i = 0; i < 8; i++) {
        DefineSprite(i + 0x11A, 0, 0x20, 0x20, 0x1E, (u8)(i * 32), 0x60, 0x40, 0x1F9);
    }
    DefineSprite(0x12D, 0, 0x20, 0x20, 0x1E, 0xA0, 0x40, 0x50, 0x1F9);
    DefineSprite(0x133, 0, 0x58, 0x28, 0x1E, 0, 0xC0, 0x60, 0x1F9);
    DefineSprite(0x134, 0, 0x20, 0x20, 0x1E, 0xC0, 0x20, 0x70, 0x1F9);
    DefineSprite(0x135, 0, 0x20, 0x30, 0x1E, 0xE0, 0x20, 0x90, 0x1F9);
    DefineSprite(0x136, 0, 0x90, 0x18, 0x1E, 0x58, 0xC0, 0xB0, 0x1F9);
    DefineSprite(0x122, 0, 0x18, 8, 0x1E, 0x58, 0x58, 0x30, 0x1F9);
    for (i = 0; i < 8; i++) {
        DefineSprite(i + 0x123, 0, 0x20, 0x20, 0x1E, (u8)(i * 32), 0x80, 0x80, 0x1F9);
    }
    for (i = 0; i < 2; i++) {
        DefineSprite(i + 0x12B, 0, 0x20, 0x20, 0x1E, (u8)(i * 32), 0xA0, 0x80, 0x1F9);
    }
    DefineSprite(0x12E, 0, 0x60, 0x10, 0x1E, 0x60, 0xE0, 0xA0, 0x1F9);
    for (i = 0; i < 4; i++) {
        DefineSprite(i + 0x12F, 0, 8, 0x10, 0x1E, (u8)(i * 8 - 0x40), 0xE0, 0xA0, 0x1F9);
    }
    for (i = 0; i < 12; i++) {
        DefineSprite(i + 0x137, 0, 0x10, 0x10, 0x1E, (u8)(i * 16 + 0x40), 0xB0, 0x2D0, 0x1CA);
    }
    for (i = 0; i < 12; i++) {
        DefineSprite(i + 0x143, 0, 0x10, 0x10, 0x1E, (u8)(i * 16 + 0x40), 0xA0, 0x2D0, 0x1CA);
    }
    DefineSprite(0x14F, 0, 0x50, 0x10, 0x1B, 0x80, 0xC0, 0x2D0, 0x1C9);
    DefineSprite(0x150, 0, 0x50, 0x10, 0x1B, 0x80, 0xD0, 0x2D0, 0x1C9);
    DefineSprite(0x151, 0, 0x80, 0x20, 0x1B, 0x80, 0xE0, 0x2D0, 0x1C9);
    DefineSprite(0x152, 0, 0x80, 0x20, 0x1C, 0x80, 0xC0, 0x2D0, 0x1C9);
    for (i = 0; i < 4; i++) {
        DefineSprite(i + 0x153, 0, 0x20, 0x20, 0x1C, (u8)(i * 32 - 0x80), 0xE0, 0x2D0, 0x1C9);
    }
    for (i = 0; i < 4; i++) {
        DefineSprite(i + 0x157, 0, 0x80, 0x10, 0x1C, 0, (u8)(i * 16 - 0x40), 0x2D0, 0x1C9);
    }
    DefineSprite(0x15B, 0, 0x68, 0x10, 0x1E, 0x80, 0xF0, 0xD0, 0x1F9);
    DefineSprite(0x15C, 0, 0x80, 0x10, 0x1E, 0, 0xF0, 0xC0, 0x1F9);
    DefineSprite(0x15D, 0, 0x10, 0x10, 0x0F, 0, 0xC0, 0x3C0, 0xFF);
    DefineSprite(0x15E, 0, 0x20, 0x20, 0x0F, 0, 0xD0, 0x3D0, 0xFF);
    DefineSprite(0x15F, 0, 0x80, 0x20, 0x0F, 0x20, 0xC0, 0x3E0, 0xFF);
}

extern u32 D_80095A84;

/** @brief A CVECTOR whose first byte is signed (-1 means "no tint"). */
typedef struct {
    s8 r;  /**< red, or -1 */
    u8 g;  /**< green */
    u8 b;  /**< blue */
    u8 cd; /**< code byte */
} SColor;

/* MATCHING: code_a0bc takes a Sprite2D *, a type local to that unit; this
 * unit passes the same eight halfwords as an array. */
void DrawSpriteTransformed(u16 id, s16 *quad, CVECTOR *color, u16 otz, GsOT *ot);
s32 SpawnEffect(s32 kind, s16 x, s16 y, s16 z, s16 delay);

void DrawHud(void) {
    s32 n;
    s32 k;
    s32 q;
    s32 digits;
    SVECTOR pos;
    s16 quad[8];
    SColor color;

    n = gCans;
    digits = 0;
    for (k = 1; k < 101; k *= 10) {
        if (n / k == 0) {
            break;
        }
        digits++;
    }
    if (n == 0) {
        pos.vx = -120;
        pos.vy = -96;
        DrawSprite(0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    }
    k = 1;
    pos.vx = (digits - 1) * 12 - 120;
    pos.vy = -96;
    for (; k < 101; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + 0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    quad[0] = quad[2] = -0x90;
    quad[1] = quad[3] = -0x68;
    quad[4] = quad[5] = 0x800;
    quad[6] = 0;
    quad[7] = 0;
    DrawSpriteTransformed(0xFA, quad, NULL, 0, &gSubOt[gActiveBuff]);
    quad[0] = quad[2] = -0x40;
    quad[1] = quad[3] = -0x58;
    quad[7] = 1;
    quad[6] = 0;
    D_80095A84 += D_8009EF20[0];
    switch (gBonusLevel) {
        case 1:
            quad[4] = quad[5] = 0x800;
            if ((gFrameCount >> 1) & 1) {
                DrawSpriteTransformed(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &gSubOt[gActiveBuff]);
            }
            break;
        case 2:
            quad[4] = quad[5] = 0xB50;
            DrawSpriteTransformed(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 0:
            break;
        default:
            quad[4] = quad[5] = 0x1000;
            DrawSpriteTransformed(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &gSubOt[gActiveBuff]);
            break;
    }
    if (gTimeLeft == 0x14A) {
        PlaySe(0x28);
        SpawnEffect(10, 0, 0, 0, 0);
    }
    n = gTimeLeft;
    n /= 30;
    color.r = -1;
    if (n < 11) {
        color.g = 0x80;
        color.b = color.cd = 0;
    } else {
        color.g = color.b = color.cd = 0x80;
    }
    if (n >= 5 || ((gFrameCount >> 1) & 1)) {
        /* MATCHING: a digit count whose result is never used; the
         * divisions stay. */
        for (k = 1; k < 101; k *= 10) {
            if (n / k == 0) {
                break;
            }
        }
        if (n == 0) {
            pos.vx = 4;
            pos.vy = -0x68;
            DrawSprite(0x123, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        k = 1;
        pos.vx = 4;
        pos.vy = -0x68;
        for (; k < 101; k *= 10) {
            q = n / k;
            if (q != 0) {
                DrawSprite(q % 10 + 0x123, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                pos.vx -= 18;
            }
        }
    }
    color.r = 1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = (gSection * 11 << 15) / ((s32 *)gSectionEdges)[-1] / 4096 + 0x34;
    pos.vy = -0x69;
    DrawSprite(((gFrameCount >> 2) & 3) + 0x12F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    k = 1;
    pos.vx = 0x34;
    pos.vy = -0x64;
    DrawSprite(0x12E, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x34;
    pos.vy = -0x54;
    DrawSprite(0x122, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x90;
    pos.vy = -0x54;
    DrawHudTime(&pos, gElapsed);
    n = gRetries;
    pos.vx = 0x68;
    pos.vy = 0x4C;
    for (; k < 10001; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + 0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = 0x38;
    DrawSprite(0x135, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
}

/**
 * @brief Absolute value.
 * @param v the value
 * @return |v|
 */
static __inline__ s32 iabs(s32 v) {
    return v >= 0 ? v : -v;
}

void LineCircleIntersect(s16 *out, s16 x0, s16 y0, s16 x1, s16 y1) {
    s32 swapped;
    s32 t;
    s32 m;
    s32 b;
    s32 c;
    s32 a;
    s32 r0;
    s32 r1;
    s32 h;
    s32 nb;
    s32 sq;

    swapped = 0;
    if (iabs(x1 - x0) < iabs(y1 - y0)) {
        t = x0;
        x0 = y0;
        y0 = -t;
        t = x1;
        /* MATCHING: the negation goes through m (free until the slope). */
        m = -t;
        x1 = y1;
        y1 = m;
        swapped = 1;
    }
    m = ((y1 - y0) << 16) / (x1 - x0);
    a = (y1 - y0) * (y1 - y0) * 4 / ((x1 - x0) * (x1 - x0));
    c = y0 - ((m * x0) >> 16);
    b = (m * c) >> 15;
    a += 4;
    /* MATCHING: one local carries the discriminant and then the half
     * leading coefficient, b * b computed into it first. */
    h = b * b;
    h -= a * (c * c - 250000);
    if (h >= 0) {
        sq = SquareRoot0(h);
        h = a >> 1;
        nb = -b;
        r1 = (nb + sq) / h;
        r0 = (nb - sq) / h;
        if (iabs(x0 - r1) <= iabs(x0 - r0)) {
            if (swapped) {
                out[1] = r1;
                out[0] = -(((m * r1) >> 16) + c);
            } else {
                out[0] = r1;
                out[1] = ((m * r1) >> 16) + c;
            }
        } else {
            if (swapped) {
                out[1] = r0;
                out[0] = -(((m * r0) >> 16) + c);
            } else {
                out[0] = r0;
                out[1] = ((m * r0) >> 16) + c;
            }
        }
    }
}

extern s32 D_80095828;
extern s16 D_80095994;
extern s16 D_80095996;
extern s16 D_80095890;
extern s16 D_800957DE;

#define sView (*(GsRVIEW2 *)gView)

/**
 * @brief Steps a boundary index across the edges either side of (x, y).
 * @param index the index to step
 * @param tbl   the edge table
 * @param x     the point's x
 * @param y     the point's y
 */
static __inline__ void stepEdge(s32 *index, Edge *tbl, s16 x, s16 y) {
    s32 i;
    s32 j;
    s32 d;
    Edge *e;
    Edge *f;

    i = *index;
    e = (Edge *)(i * 8 + (s32)tbl) + 1;
    d = e->dx * (x - e->x) + e->dy * (y - e->y);
    if (d >= 0) {
        *index = i + 1;
    }
    j = *index;
    f = (Edge *)(j * 8 + (s32)tbl);
    d = -f->dx * (x - f->x) + -f->dy * (y - f->y);
    if (d >= 0) {
        *index = j - 1;
    }
}

s16 UpdateCamera(s16 x, s16 target, s16 y, s16 height) {
    Edge *tbl;
    Edge *e;
    /* MATCHING: s16, extended only at the return; as s32 it takes a's
     * register and a gets another. */
    s16 angle;
    s32 b;
    s32 a;
    s32 old;
    s32 v;
    s32 off;
    /* MATCHING: retail's frame has 8 more bytes below the saved registers. */
    s32 unused[2];

    tbl = gSectionEdges;
    stepEdge(&gSection, tbl, x, y);
    e = (Edge *)(gSection * 8 + (s32)tbl);
    angle = a = ratan2(e[1].x - e[0].x, e[1].y - e[0].y);
    a &= 0xFFF;
    b = gCameraYaw & 0xFFF;
    if (iabs(a - b) > 0x800) {
        if (a < 0x800) {
            a += 0x1000;
        }
        if (b < 0x800) {
            b += 0x1000;
        }
    }
    /* MATCHING: the distance re-derived in each test; a local for it gets
     * a register retail gives to a. */
    if (iabs(a - b) > 50) {
        if (D_80095828 < 20) {
            D_80095828 += 2;
        }
    } else if (iabs(a - b) > 30) {
        D_80095828 = 10;
    } else if (iabs(a - b) > 5) {
        D_80095828 = 5;
    } else if (a == b) {
        D_80095828 = 0;
    } else {
        D_80095828 = 1;
    }
    if (a - b > 0) {
        gCameraYaw += D_80095828;
    } else {
        gCameraYaw -= D_80095828;
    }
    D_80095994 = rsin((s16)gCameraYaw + 0x800) * 400 / 4096;
    D_80095996 = rcos((s16)gCameraYaw + 0x800) * 400 / 4096;
    sView.vpz = D_80095996;
    sView.vpx = D_80095994;
    D_800957DE = sView.vpy;
    if (D_80095808[gSection] * 10 < D_80095890) {
        D_80095890 -= 10;
    }
    if (D_80095890 < D_80095808[gSection] * 10) {
        D_80095890 += 10;
    }
    old = D_800957DE;
    v = height + D_80095890;
    sView.vpy = v;
    if (v - old > 200) {
        sView.vpy = old + 50;
    } else {
        /* MATCHING: the rest of the clamp reads old through the dead a
         * (permuter find); it moves a's register for the whole function. */
        a = old;
        if (v - a > 50) {
            sView.vpy = a + 20;
        } else if (v - a > 20) {
            sView.vpy = a + 10;
        }
    }
    if (sView.vpy - D_800957DE < -200) {
        sView.vpy = D_800957DE - 50;
    } else if (sView.vpy - D_800957DE < -50) {
        sView.vpy = D_800957DE - 20;
    } else if (sView.vpy - D_800957DE < -20) {
        sView.vpy = D_800957DE - 10;
    }
    /* MATCHING: a fresh local, so the sum stays in one register; -height
     * first loads height first. */
    off = (-height + target) / 2 - 150;
    sView.vrx = 0;
    sView.vry = height + off;
    sView.vrz = 0;
    sView.rz = 0;
    sView.super = NULL;
    GsSetRefView2L(&sView);
    return angle;
}

void DrawCpuMeter(void) {
    POLY_F4 *p;
    s32 i;

    for (i = 0; i < gCpuMarkCount; i++) {
        if (gCpuMarkTimes[i] != 0xFF) {
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = -152;
            p->y0 = p->y1 = (gCpuMarkTimes[i] >> 1) - 120;
            if (i != 0) {
                p->y3 = (gCpuMarkTimes[i - 1] >> 1) - 120;
            } else {
                p->y3 = -120;
            }
            p->y2 = p->y3;
            p->r0 = D_800AC848[i];
            p->g0 = D_800A7888[i];
            p->b0 = D_800A76E8[i];
            addPrim(gCurOt->org, p);
            p++;
            GsOUT_PACKET_P = (u8 *)p;
        }
    }
    p = (POLY_F4 *)GsOUT_PACKET_P;
    setPolyF4(p);
    p->x0 = p->x2 = -160;
    p->x1 = p->x3 = -144;
    p->y0 = p->y1 = 8;
    p->y2 = p->y3 = 9;
    setRGB0(p, 0xFF, 0xFF, 0xFF);
    gCpuMarkCount = 0;
    addPrim(gCurOt->org, p);
    p++;
    GsOUT_PACKET_P = (u8 *)p;
}

void DrawFrozenFrame(void) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, &color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, &color, 0, &gSubOt[gActiveBuff]);
}

void DrawFrozenFrameFaded(u8 level) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    /* MATCHING: an s8 store gives li -1; a u_char one gives li 0xFF. */
    *(s8 *)&color.r = -1;
    color.g = color.b = color.cd = level;
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, &color, 0xFFF, &gOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, &color, 0xFFF, &gOt[gActiveBuff]);
}

void DrawScreenWipe(void) {
    POLY_FT4 *p;
    s32 n;

    if (gWipeActive != 0) {
        if (gWipeFrame >= 24) {
            gWipeActive = 0;
            return;
        }
        n = gWipeFrame / 3;
        p = (POLY_FT4 *)GsOUT_PACKET_P;
        setPolyFT4(p);
        p->r0 = p->g0 = p->b0 = 0x80;
        p->x0 = p->x2 = -160;
        p->x1 = p->x3 = 160;
        p->y0 = p->y1 = -120;
        p->y2 = p->y3 = 120;
        p->u0 = p->u2 = (n % 2) << 7;
        p->v0 = p->v1 = (n / 2 % 2) * 96;
        p->clut = getClut(720, n + 448);
        p->u1 = p->u3 = p->u0 + 0x7F;
        p->v2 = p->v3 = p->v0 + 0x5F;
        p->tpage = getTPage(0, 0, n / 4 * 64 + 704, 256);
        addPrim(gSubOt[gActiveBuff].org, p);
        gWipeFrame++;
        p++;
        GsOUT_PACKET_P = (u8 *)p;
    }
}

void DrawResultNumber(SVECTOR *pos, s32 n) {
    CVECTOR color;
    CVECTOR shadow;
    u16 d;
    s32 t;
    /* MATCHING: dead x keeps both magic constants in prologue registers. */
    u32 x;

    x = (s16)n / 10;
    color.r = 1;
    shadow.r = 2;
    shadow.g = shadow.b = shadow.cd = 0x80;
    color.g = color.b = color.cd = 0x80;
    pos->vx -= 16;
    DrawSprite(0x137, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(0x143, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(0x137, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(0x143, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(0x138, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(0x144, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(0x142, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(0x14E, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    t = n / 10;
    x = (u32)n / 100;
    d = n % 10;
    DrawSprite(d + 0x137, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(d + 0x143, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    if (t != 0) {
        pos->vx -= 12;
        d = t % 10;
        DrawSprite(d + 0x137, pos, &color, 0, &gSubOt[gActiveBuff]);
        DrawSprite(d + 0x143, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    }
    if (n / 100 != 0) {
        pos->vx -= 12;
        d = n / 100 % 10;
        DrawSprite(d + 0x137, pos, &color, 0, &gSubOt[gActiveBuff]);
        DrawSprite(d + 0x143, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    }
}

void DrawResultTime(SVECTOR *pos, s32 frames) {
    s32 n;
    s32 m;
    s32 d;
    s32 k;
    s32 t;
    s32 sec;
    SColor color;
    /* MATCHING: retail fills a second colour at sp+0x20 it never passes. */
    SColor shadow;

    color.r = -1;
    shadow.r = 2;
    shadow.g = shadow.b = shadow.cd = 0x80;
    color.g = color.b = color.cd = 0x80;
    n = frames * 100 / 30;
    /* MATCHING: d copied from n, then divided (as in DrawHudTime). */
    d = n;
    d /= 10;
    pos->vx -= 16;
    DrawSprite(n % 10 + 0x137, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(d % 10 + 0x137, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(0x141, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    sec = frames / 30;
    m = sec / 60;
    n = sec % 60;
    k = 1;
    d = n;
    d /= 10;
    pos->vx -= 12;
    DrawSprite(n % 10 + 0x137, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(d % 10 + 0x137, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = m;
    pos->vx -= 12;
    DrawSprite(0x141, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    for (; k < 11; k *= 10) {
        t = n / k;
        if (t != 0 || (k == 1 && n == 0)) {
            pos->vx -= 12;
            DrawSprite(t % 10 + 0x137, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
    }
}

/* MATCHING: 8-byte arrays (rank % 3 indexes them); as scalars indexed
 * through & the stores in UpdateActResults schedule the address first. */
extern s32 gActCans[2];
extern s32 gActTimes[2];
/* MATCHING: per-unit views; main.c reads the same pairs as Pair8. */
extern s32 gBestCans[][2];
extern s32 gBestTimes[][2];
extern s32 gTargetTimes[][2];

void DrawResultNumber(SVECTOR *pos, s32 n);

void UpdateActResults(void) {
    s32 i;
    s32 k;
    s32 q;
    s32 n;
    s32 place;
    s32 rank;
    SVECTOR pos;
    s16 quad[8];
    SColor color;
    s16 tile[4];

    if (gResultsActive == 0) {
        return;
    }
    place = gStage;
    if (place >= 0) {
        if (place < 6) {
            rank = place;
        } else if (place < 15) {
            if (place >= 9) {
                rank = place - 3;
            }
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    switch (gResultsStep) {
        case 0:
            switch (gResultsTimer) {
                case 0:
                    gResultsSliding[0] = 1;
                    break;
                case 24:
                    gResultsSlide[1] = -24;
                    break;
                case 40:
                    gResultsSliding[1] = 1;
                    break;
                case 64:
                    gResultsSliding[2] = 1;
                    break;
                case 68:
                    gResultsSliding[3] = 1;
                    break;
                case 80:
                    gResultsStep = 10;
                    break;
            }
            gResultsTimer++;
            break;
        case 10:
            if (gTallyCans < gCans) {
                if (gTallyCans % 10 == 0) {
                    PlaySe(0x2A);
                }
                gTallyCans++;
                if (gTallyCans != 0 && gTallyCans % 25 == 0) {
                    gRetries++;
                    PlaySe(0x22);
                }
            }
            if (gTallyCans == gCans) {
                if (gTallyCans == 100) {
                    PlaySe(0x2F);
                    tile[1] = 0x1FB;
                    tile[2] = 0x10;
                    tile[3] = 1;
                    gResultsSlide[6] = -9;
                    tile[0] = 0;
                    gResultsTimer = 0;
                    gResultsStep = 11;
                } else if (gBestCans[rank / 3][rank % 3] < gTallyCans) {
                    PlaySe(0x25);
                    gResultsSlide[7] = -9;
                    gResultsTimer = 0;
                    gResultsStep = 15;
                } else {
                    gResultsTimer = 0;
                    gResultsStep = 20;
                }
            }
            break;
        case 11:
            if (gResultsSlide[6] == 0) {
                if (gResultsTimer < 36) {
                    tile[1] = 0x1FB;
                    tile[2] = 0x10;
                    tile[3] = 1;
                    tile[0] = gResultsTimer % 14 * 16;
                }
                if (++gResultsTimer == 60) {
                    gRetries++;
                    PlaySe(0x22);
                    gResultsSlide[6] = 1;
                }
            } else if (++gResultsSlide[6] == 9) {
                if (gTallyCans > gBestCans[rank / 3][rank % 3]) {
                    PlaySe(0x25);
                    gResultsSlide[7] = -9;
                    gResultsTimer = 0;
                    gResultsStep = 15;
                } else {
                    gResultsTimer = 0;
                    gResultsStep = 20;
                }
            }
            break;
        case 15:
            tile[3] = 1;
            tile[1] = 0x1FA;
            tile[2] = 0x10;
            tile[0] = gResultsTimer % 14 * 16;
            if (gResultsSlide[7] == 0) {
                if (++gResultsTimer == 60) {
                    gBestCans[rank / 3][rank % 3] = gTallyCans;
                    gResultsSlide[7] = 1;
                }
            } else if (++gResultsSlide[7] == 9) {
                gResultsTimer = 0;
                gResultsStep = 20;
            }
            break;
        case 20:
            switch (gResultsTimer) {
                case 15:
                    gResultsSliding[4] = 1;
                    break;
                case 19:
                    gResultsSliding[5] = 1;
                    break;
                case 31:
                    gResultsStep = 30;
                    break;
            }
            gResultsTimer++;
            break;
        case 30:
            if (gElapsed < gBestTimes[rank / 3][rank % 3]) {
                PlaySe(0x25);
                gResultsSlide[7] = -9;
                gResultsTimer = 0;
                gResultsStep = 35;
            } else {
                gResultsTimer = 0;
                gResultsStep = 50;
            }
            break;
        case 35:
            tile[3] = 1;
            tile[1] = 0x1FA;
            tile[2] = 0x10;
            tile[0] = gResultsTimer % 14 * 16;
            if (gResultsSlide[7] == 0) {
                if (++gResultsTimer == 60) {
                    gBestTimes[rank / 3][rank % 3] = gElapsed;
                    gResultsSlide[7] = 1;
                }
            } else if (++gResultsSlide[7] == 9) {
                gResultsTimer = 0;
                gResultsStep = 50;
            }
            break;
        case 50:
            if (++gResultsTimer == 60) {
                n = 0;
                for (i = 0; i < 4; i++) {
                    if (gBestTimes[i][0] >= gTargetTimes[i][0]) {
                        n = 1;
                    }
                    if (gBestTimes[i][1] >= gTargetTimes[i][1]) {
                        n = 1;
                    }
                }
                if (n == 0) {
                    D_800957D5 = 1;
                }
                n = 0;
                for (i = 0; i < 4; i++) {
                    n += gBestCans[i][0] + gBestCans[i][1];
                }
                if (n == 800) {
                    gUnlocks |= 4;
                }
                gResultsActive = 0;
                gActCans[rank % 3] = gTallyCans;
                gActTimes[rank % 3] = gElapsed;
            }
            break;
    }
    for (i = 0; i < 6; i++) {
        if (gResultsSliding[i] != 0) {
            if (gResultsSlide[i] != 0) {
                gResultsSlide[i]++;
            } else {
                gResultsSliding[i] = 0;
            }
        }
    }
    k = 1;
    n = gRetries;
    pos.vx = 0x68;
    pos.vy = -0x44;
    for (; k < 10001; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + 0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = -0x58;
    DrawSprite(0x135, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    color.r = -1;
    color.g = color.b = 0x80;
    color.cd = 0;
    pos.vy = -16;
    pos.vx = gResultsSlide[6] * 32 - 0x34;
    DrawSprite(0x15B, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -16;
    pos.vx = gResultsSlide[7] * 32 - 0x40;
    DrawSprite(0x15C, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -0x28;
    pos.vy = gResultsSlide[0] * 4 - 0x48;
    DrawSprite(rank % 3 + 0x14F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    if (gResultsSlide[1] != -25) {
        quad[1] = -0x20;
        quad[3] = -0x40;
        quad[4] = quad[5] = (-gResultsSlide[1] << 8) + 0x1000;
        quad[0] = 0;
        quad[2] = 0;
        quad[6] = 0;
        quad[7] = 1;
        DrawSpriteTransformed(0x151, quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    pos.vy = 0;
    pos.vx = gResultsSlide[2] * 32 - 0x80;
    DrawSprite(0x157, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[2] * 32 + 0x80;
    DrawResultNumber(&pos, gTallyCans);
    pos.vy = 0x18;
    pos.vx = gResultsSlide[3] * 32 - 0x80;
    DrawSprite(0x158, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[3] * 32 + 0x80;
    DrawResultNumber(&pos, gBestCans[rank / 3][rank % 3]);
    pos.vy = 0x38;
    pos.vx = gResultsSlide[4] * 32 - 0x80;
    DrawSprite(0x159, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[4] * 32 + 0x80;
    DrawResultTime(&pos, gElapsed);
    pos.vy = 0x50;
    pos.vx = gResultsSlide[5] * 32 - 0x80;
    DrawSprite(0x15A, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[5] * 32 + 0x80;
    DrawResultTime(&pos, gBestTimes[rank / 3][rank % 3]);
}

extern s32 gWorldCans;
extern s32 gWorldTime;

void UpdateWorldResults(void) {
    s32 i;
    s32 k;
    s32 rank;
    SVECTOR pos;
    SColor color;

    if (gResultsActive == 0) {
        return;
    }
    k = gStage;
    if (k >= 0) {
        if (k < 6) {
            rank = k;
        } else if (k < 15) {
            if (k >= 9) {
                rank = k - 3;
            }
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    switch (gResultsStep) {
        case 0:
            switch (gResultsTimer) {
                case 0:
                    gResultsSliding[0] = 1;
                    break;
                case 60:
                    gResultsSliding[1] = 1;
                    break;
                case 90:
                    gResultsSliding[2] = 1;
                    break;
                case 94:
                    gResultsSliding[3] = 1;
                    break;
                case 120:
                    gResultsSliding[4] = 1;
                    break;
                case 150:
                    gResultsSliding[5] = 1;
                    break;
                case 154:
                    gResultsSliding[6] = 1;
                    break;
                case 300:
                    gResultsStep = 10;
                    break;
            }
            gResultsTimer++;
            break;
        case 10:
            gResultsActive = 0;
            break;
    }
    for (i = 0; i < 7; i++) {
        if (gResultsSliding[i] != 0) {
            if (gResultsSlide[i] != 0) {
                gResultsSlide[i]++;
            } else {
                gResultsSliding[i] = 0;
            }
        }
    }
    pos.vx = -80;
    pos.vy = gResultsSlide[0] * 4 - 88;
    DrawSprite(0x152, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 48;
    DrawSprite(rank / 3 + 0x153, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -48;
    pos.vx = gResultsSlide[1] * 32 - 40;
    DrawSprite(0x14F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -24;
    pos.vx = gResultsSlide[2] * 32 - 128;
    DrawSprite(0x157, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[2] * 32 + 128;
    DrawResultNumber(&pos, gActCans[0]);
    pos.vy = 0;
    pos.vx = gResultsSlide[3] * 32 - 128;
    DrawSprite(0x159, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[3] * 32 + 128;
    DrawResultTime(&pos, gActTimes[0]);
    pos.vy = 24;
    pos.vx = gResultsSlide[4] * 32 - 40;
    DrawSprite(0x150, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = 48;
    pos.vx = gResultsSlide[5] * 32 - 128;
    DrawSprite(0x157, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[5] * 32 + 128;
    DrawResultNumber(&pos, gWorldCans);
    pos.vy = 72;
    pos.vx = gResultsSlide[6] * 32 - 128;
    DrawSprite(0x159, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[6] * 32 + 128;
    DrawResultTime(&pos, gWorldTime);
}

/** @brief Sixteen signed bytes, copied together as one unaligned block. */
typedef struct {
    s8 b[4][4]; /**< four rows of four */
} Bytes16;

extern Bytes16 D_80011EB0;

void RemapButtons(void) {
    Bytes16 tbl;
    s32 k;
    u32 a;
    u32 b;

    /* MATCHING: the row index in a local, indexed per use; a row pointer
     * moves the address add above the first andi. */
    tbl = D_80011EB0;
    a = gPadTrig;
    b = gPad;
    k = gButtonConfig;
    gGamePadTrig = a;
    gGamePad = b;
    gGamePadTrig = a & ~0xE0;
    gGamePad = b & ~0xE0;
    gGamePadTrig |= ((a & 0x20) >> 5) << (tbl.b[k][0] + 5);
    gGamePadTrig |= ((a & 0x40) >> 6) << (tbl.b[k][1] + 5);
    gGamePadTrig |= ((a & 0x80) >> 7) << (tbl.b[k][2] + 5);
    gGamePad |= ((b & 0x20) >> 5) << (tbl.b[k][0] + 5);
    gGamePad |= ((b & 0x40) >> 6) << (tbl.b[k][1] + 5);
    gGamePad |= ((b & 0x80) >> 7) << (tbl.b[k][2] + 5);
}

void LinkStageTmd(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
}

void LinkPropsTmd(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
}

void LoadTimList(unsigned long *p) {
    RECT rect;
    GsIMAGE tim;

    while (*p == 0x10) {
        GsGetTimInfo(p + 1, &tim);
        rect.x = tim.cx;
        rect.y = tim.cy;
        rect.w = tim.cw;
        rect.h = tim.ch;
        LoadImage(&rect, (u_long *)tim.clut);
        rect.x = tim.px;
        rect.y = tim.py;
        rect.w = tim.pw;
        rect.h = tim.ph;
        LoadImage(&rect, (u_long *)tim.pixel);
        p += 2;
        p += *p >> 2;
        p += *p >> 2;
    }
}

void NearestPointToOrigin(s16 *out, s16 x0, s16 y0, s16 x1, s16 y1) {
    s32 dx;
    s32 dy;
    s32 t;
    s32 d;

    dx = x1 - x0;
    dy = y1 - y0;
    t = -(x0 * dx + y0 * dy);
    d = dx * dx + dy * dy;
    out[0] = t * dx / d;
    out[1] = t * dy / d;
}

void MarkCpuTime(s32 a, s32 b, s32 c) {
    if (gCpuMarkCount < 16) {
        gCpuMarkTimes[gCpuMarkCount] = VSync(1);
        D_800AC848[gCpuMarkCount] = a;
        D_800A7888[gCpuMarkCount] = b;
        D_800A76E8[gCpuMarkCount] = c;
        gCpuMarkCount++;
    }
}

void func_8003E360(s32 level) {
    s32 i;
    s32 v;
    s32 base;

    struct {
        SVECTOR a;
        SVECTOR b;
        CVECTOR c;
    } st;

    if (level > 0x80) {
        level = 0x80;
    }
    for (i = 8; i >= 0; i--) {
        base = i * 3 + 0x40;
        v = base - level;
        if (v < 0) {
            v = 0;
        }
        st.a.vy = st.a.pad = st.a.vx = st.a.vz = 0;
        st.b.vx = st.b.vy = v * 32 + 0x1000;
        st.b.vz = (v << 12) / 360 * 3;
        st.b.pad = 1;
        st.c.r = 1;
        st.c.g = st.c.b = st.c.cd = 0x10;
    }
}

void StartScreenWipe(void) {
    gWipeFrame = 0;
    gWipeActive = 1;
    PlaySe(0x19);
}

s32 IsScreenWipeActive(void) {
    return gWipeActive;
}

void StartActResults(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)D_800956B0;
    gResultsStep = 0;
    gResultsTimer = 0;
    gResultsActive = 1;
    gTallyCans = 0;
    for (i = 0; i < 8; i++) {
        gResultsSlide[i] = buf.b[i];
        gResultsSliding[i] = 0;
    }
}

void StartWorldResults(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)D_800956B8;
    gResultsStep = 0;
    gResultsTimer = 0;
    gResultsActive = 1;
    gTallyCans = 0;
    for (i = 0; i < 8; i++) {
        gResultsSlide[i] = buf.b[i];
        gResultsSliding[i] = 0;
    }
}

s32 IsResultsActive(void) {
    return gResultsActive;
}

/* MATCHING: inlined, so each call rebuilds the two struct addresses. */
/**
 * @brief Sets the GS local-screen matrix to a pure translation (x, y, z);
 *        the sprite steps below expand it in place.
 * @param x translation x
 * @param y translation y
 * @param z translation z
 */
static __inline__ void setLs(s16 x, s16 y, s16 z) {
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = x;
    coord.coord.t[1] = y;
    coord.coord.t[2] = z;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
}

s32 StepEffect1(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        /* MATCHING: retail shifts the scaled sine with srl. */
        size.vx = (u32)(rcos((p->frame * 3 << 13) / 360) * 25) >> 9;
        size.vy = (u32)(rsin((p->frame << 15) / 360) * 25) >> 8;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        DrawStandingBillboard(0x11F, &size, &color, 2, ot);
        return ++p->frame == 15;
    }
    p->delay--;
    return 0;
}

s32 StepEffect5(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 c[2];
    POLY_F4 *f;
    DR_TPAGE *t;

    c[0] = p->x + (p->z >> 1) + 2;
    c[1] = p->y + 4;
    f = (POLY_F4 *)GsOUT_PACKET_P;
    setPolyF4(f);
    setSemiTrans(f, 1);
    f->x0 = p->x;
    f->x1 = p->x + p->z;
    f->x2 = f->x0 + 4;
    f->x3 = f->x1 + 4;
    f->y0 = f->y1 = p->y;
    f->y2 = f->y3 = p->y + 8;
    f->x0 = c[0] + ((c[0] - f->x0) * ((p->frame << 9) + 0x1000) >> 12);
    f->x1 = c[0] + ((c[0] - f->x1) * ((p->frame << 9) + 0x1000) >> 12);
    f->x2 = c[0] + ((c[0] - f->x2) * ((p->frame << 9) + 0x1000) >> 12);
    f->x3 = c[0] + ((c[0] - f->x3) * ((p->frame << 9) + 0x1000) >> 12);
    f->y0 = c[1] + ((c[1] - f->y0) * ((p->frame << 10) + 0x1000) >> 12);
    f->y1 = c[1] + ((c[1] - f->y1) * ((p->frame << 10) + 0x1000) >> 12);
    f->y2 = c[1] + ((c[1] - f->y2) * ((p->frame << 10) + 0x1000) >> 12);
    f->y3 = c[1] + ((c[1] - f->y3) * ((p->frame << 10) + 0x1000) >> 12);
    f->r0 = f->g0 = f->b0 = ~(p->frame << 5);
    addPrim(ot->org, f);
    f++;
    GsOUT_PACKET_P = (u8 *)f;
    t = (DR_TPAGE *)GsOUT_PACKET_P;
    setDrawTPage(t, 1, 1, getTPage(0, 1, 0, 0));
    addPrim(ot->org, t);
    t++;
    GsOUT_PACKET_P = (u8 *)t;
    return ++p->frame == 8;
}

s32 StepEffect7(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];

    if (p->delay == 0) {
        if (p->frame == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->x, (long *)q, NULL, NULL);
            p->x = q[0];
            p->y = q[1];
        }
        q[0] = q[2] = p->x + ((-136 - p->x) * p->frame >> 3);
        q[1] = q[3] = p->y + ((-88 - p->y) * p->frame >> 3);
        q[4] = q[5] = 0x1000 - (p->frame << 8);
        q[6] = p->frame << 8;
        q[7] = 1;
        color.r = 0;
        color.g = color.b = color.cd = ~(p->frame << 4);
        DrawSpriteTransformed(0xFA, q, &color, 0, &gSubOt[gActiveBuff]);
        if (++p->frame == 8) {
            gCans++;
            CheckCanBonus();
            return 1;
        }
        return 0;
    }
    p->delay--;
    return 0;
}

s32 StepEffect8(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];
    s32 i;

    if (p->delay == 0) {
        if (p->frame == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->x, (long *)q, NULL, NULL);
            p->x = q[0];
            p->y = q[1];
        }
        for (i = 0; i < 5; i++) {
            q[0] = p->x + ((-136 - p->x) * p->frame >> 3);
            q[1] = p->y + ((-88 - p->y) * p->frame >> 3);
            q[0] = q[2] =
                q[0] + rsin(((p->frame * 20 + i * 72) << 12) / 360) * (64 - p->frame * 8) / 4096;
            q[1] = q[3] =
                q[1] + rcos(((p->frame * 20 + i * 72) << 12) / 360) * (64 - p->frame * 8) / 4096;
            q[4] = q[5] = 0x1000 - (p->frame << 8);
            q[6] = -(((p->frame * 20 + i * 72 + 90) << 12) / 360);
            q[7] = 1;
            color.r = 0;
            color.g = color.b = color.cd = ~(p->frame << 4);
            DrawSpriteTransformed(0xFA, q, &color, 0, &gSubOt[gActiveBuff]);
        }
        if (++p->frame == 8) {
            gCans += 5;
            CheckCanBonus();
            return 1;
        }
        return 0;
    }
    p->delay--;
    return 0;
}

s32 StepEffect10(Effect *p) {
    SVECTOR pos;
    SColor color;

    if (p->delay == 0) {
        if (p->frame < 16) {
            pos.vx = -72 - (16 - p->frame) * 15;
            pos.vy = -12;
            color.r = -1;
            color.g = color.b = color.cd = 0x80;
            DrawSprite(0x136, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        } else if (p->frame < 32) {
            pos.vx = -72;
            pos.vy = -12;
            color.r = -1;
            color.g = 0x80;
            color.b = color.cd = (8 - (p->frame - 16) % 8) * 16;
            DrawSprite(0x136, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        } else {
            pos.vx = (p->frame - 32) * 15 - 72;
            pos.vy = -12;
            color.r = -1;
            color.g = 0x80;
            color.b = color.cd = 0;
            DrawSprite(0x136, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        return ++p->frame == 48;
    }
    p->delay--;
    return 0;
}

extern GsCOORDINATE2 D_800CDC00[];
extern SVECTOR D_800CEBE0[];
extern GsDOBJ2 D_800960A8[];

/* MATCHING: D_800CDC00 is the coordinate table's true start (undefined_syms.*.txt). */
s32 StepEffect11(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    MATRIX m;

    if (p->delay == 0) {
        GsInitCoordinate2(WORLD, &D_800CDC00[p->param0 + 10]);
        D_800CDC00[p->param0 + 10].coord.t[0] = p->x + x;
        D_800CDC00[p->param0 + 10].coord.t[1] =
            p->y - rsin(p->frame * 3 * 8192 / 360) * p->param2 / 4096 + y;
        D_800CDC00[p->param0 + 10].coord.t[2] = p->z + z;
        D_800CDC00[p->param0 + 10].flg = 0;
        D_800CEBE0[p->param0 + 10].vx = (p->frame * p->param3 << 12) / 360;
        D_800CEBE0[p->param0 + 10].vy = 0;
        D_800CEBE0[p->param0 + 10].vz = 0;
        SetCoordRotation(&D_800CEBE0[p->param0 + 10], &D_800CDC00[p->param0 + 10]);
        GsGetLs(&D_800CDC00[p->param0 + 10], &m);
        GsSetLsMatrix(&m);
        GsGetLw(&D_800CDC00[p->param0 + 10], &m);
        GsSetLightMatrix(&m);
        if (p->frame % 5 == 0) {
            SpawnEffect(4, p->x, p->y - rsin(p->frame * 3 * 8192 / 360) * p->param2 / 4096, p->z, 0);
        }
        p->x += p->param1;
        GsSortObject4(&D_800960A8[p->param0], ot, 2, (u_long *)0x1F800000);
        return ++p->frame > 29;
    }
    p->delay--;
    return 0;
}

s32 StepEffect12(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame * 10 + 100;
        size.vy = p->frame * 10 + 100;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->frame << 2);
        DrawBillboard(0x12D, &size, &color, 2, ot);
        if (p->param1 != 0) {
            p->param1--;
        }
        p->x += rsin(p->param0) * p->param1 / 4096;
        p->z += rcos(p->param0) * p->param1 / 4096;
        p->y -= p->param2;
        return ++p->frame == 32;
    }
    p->delay--;
    return 0;
}

/** @brief One frame of an effect; returns nonzero when it ends. */
typedef s32 (*EffectStep)(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);

extern EffectStep gEffectSteps[];

void UpdateEffects(s32 unused, s16 x, s16 y, s16 z) {
    u8 i;
    u8 prev;
    u8 next;
    s32 d;
    s32 done;
    Effect *slot;

    prev = 0xFF;
    if ((s8)gEffectActive < 0) {
        return;
    }
    i = gEffectActive;
    do {
        slot = &gEffects[i];
        d = x + slot->x;
        next = slot->next;
        if (d * (d * 2) > 0x77A0F) {
            done = gEffectSteps[slot->kind](slot, &gOt[gActiveBuff], x, y, z);
        } else {
            done = gEffectSteps[slot->kind](slot, &gSubOt[gActiveBuff], x, y, z);
        }
        if (done) {
            if (prev == 0xFF) {
                gEffectActive = gEffects[i].next;
            } else {
                gEffects[prev].next = gEffects[i].next;
            }
            gEffects[i].next = gEffectFree;
            gEffectFree = i;
        } else {
            prev = i;
        }
        if ((s8)next < 0) {
            break;
        }
        i = next;
    } while (1);
}

s32 SpawnEffect(s32 kind, s16 x, s16 y, s16 z, s16 delay) {
    u8 prev;
    Effect *slot;

    if ((s8)gEffectFree < 0) {
        return -1;
    }
    prev = gEffectActive;
    gEffectActive = gEffectFree;
    gEffectFree = gEffects[gEffectActive].next;
    gEffects[gEffectActive].next = prev;
    gEffects[gEffectActive].kind = kind;
    slot = &gEffects[gEffectActive];
    slot->x = x;
    slot->y = y;
    slot->z = z;
    slot->frame = 0;
    slot->delay = delay;
    return 0;
}

void SetLsTranslation(s16 x, s16 y, s16 z) {
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = x;
    coord.coord.t[1] = y;
    coord.coord.t[2] = z;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
}

s32 StepEffect0(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vy = size.vx = 0x96;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        DrawBillboard((u16)(p->frame + 0x11A), &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

s32 StepEffect2(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame + 0x14;
        size.vy = p->frame + 0x14;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->frame << 2);
        DrawBillboard(0x11F, &size, &color, 2, ot);
        p->y -= 4;
        p->z += p->frame >> 2;
        return ++p->frame == 0x20;
    }
    p->delay--;
    return 0;
}

s32 StepEffect3(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vy = size.vx = 0x32;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->frame << 4);
        DrawBillboard(0x11A, &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

s32 StepEffect4(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame * 10 + 100;
        size.vy = p->frame * 10 + 100;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->frame << 4);
        DrawBillboard(0x12D, &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

s32 StepEffect6(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame * 10 + 100;
        size.vy = p->frame * 10 + 100;
        color.r = 2;
        color.g = color.b = color.cd = 0x80 - (p->frame << 4);
        DrawBillboard(0x12D, &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

s32 StepEffect9(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y + p->param0 * p->frame, z + p->z);
        size.vy = size.vx = 100;
        color.r = 0;
        color.g = color.b = color.cd = 0x80;
        DrawBillboard(0x134, &size, &color, 2, ot);
        return ++p->frame * p->param0 >= p->param1;
    }
    p->delay--;
    return 0;
}
