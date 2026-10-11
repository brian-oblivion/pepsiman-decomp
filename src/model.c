#include "common.h"
#include "libetc.h"
#include "rand.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "model.h"
#include "draw.h"
#include "object.h"
#include "spad.h"

/** @brief A 16-byte entry of a pack's directory; the first entry's count is
 *         the number of entries. */
typedef struct {
    s32 offset;   /**< byte offset of the entry's data from the pack start */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< number of entries (read from the first one) */
} PackEntry;

/** @brief The objects a TOD animation drives: an array of GsDOBJ2, each
 *         found by its TOD object id, filled from the front. */
typedef struct {
    GsDOBJ2 *top; /**< the object array */
    s32 count;    /**< objects in use, free ones included */
    s32 capacity; /**< objects in the array */
} TodObjTable;

/* MATCHING: reading the TimInfo through the global each time, not a
 * pointer local, picks retail's registers. */
#define sTim ((TimInfo *)gTimInfo)

/** @brief A position as three words. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3i;

extern s8 gViewerReplay;  /**< set to 1 when the level is applied */
extern s16 D_80095918;    /**< cleared when the viewer starts */
extern char gMotionFmt[]; /**< the motion-number format */
/* MATCHING: a per-unit view; object defines it on its own floor
 * types, and this unit passes gFloorData's word as the data. */
s32 QueryFloor(void *out, s32 index, Vec3i *pos, s32 data);
/* MATCHING: this unit passes the coordinates unextended, so it saw s32
 * parameters (stage defines them as s16). */
void UpdateSection(s32 *index, s32 x, s32 y);
extern CdlLOC gCdTrackLoc;

/** @brief The three flat lights. */
typedef struct {
    GsF_LIGHT l[3]; /**< lights 0 to 2 */
} FlatLights;

/* MATCHING: a struct lvalue keeps the base in one register; common.h
 * declares the table as words. */
#define sLights ((*(FlatLights *)gFlatLights).l)


void OrbitViewerCamera(void);
s32 DrawTodObjects(void);
void SelectViewerMotion(void);
s8 ReadTimHeader(u16 *tim);
u8 *SetTodPacket(u8 *data, s32 objTable, s32 tmdIds, s32 tmdTable, s32 mode, s8 filter);
u8 *SetTodFrame(u32 time, u8 *data, s32 objTable, s32 tmdIds, s32 tmdTable, s32 mode, s8 filter);

void StopCdTrack(void) {
    if (gCdTrackPlaying != 0) {
        CdControlF(CdlPause, 0);
        gCdTrackPlaying = 0;
    }
}

s32 SendCdTrackCommand(u8 com) {
    CdIntToPos(gCdTrackStart, &gCdTrackLoc);
    while (CdControl(com, (u_char *)&gCdTrackLoc, 0) == 0) {
    }
    gCdTrackPos = gCdTrackStart;
    gCdTrackPlays++;
    return 0;
}

void OnCdTrackReady(u8 mode) {
    if (mode == 4) {
        SendCdTrackCommand(CdlPlay);
    }
}

s8 ReadTimHeader(u16 *tim) {
    u16 *p;
    u16 *start;
    s32 len;
    s32 c;

    p = tim;
    if (*p != 0x10) {
        return -1;
    }
    p += 2;
    sTim->mode = *p & 7;
    /* MATCHING: the masked flag through a local keeps cc1's store-flag
     * code from turning the test into a shift. */
    c = *p & 8;
    sTim->hasClut = c != 0;
    if (sTim->hasClut) {
        p += 2;
        start = p;
        len = *p;
        p += 2;
        sTim->clutRect.x = *p++;
        sTim->clutRect.y = *p++;
        sTim->clutRect.w = *p++;
        sTim->clutRect.h = *p++;
        sTim->clut = (u32 *)p;
        p = (u16 *)((u8 *)start + len);
    } else {
        p += 2;
    }
    p += 2;
    sTim->pixRect.x = *p++;
    sTim->pixRect.y = *p++;
    sTim->pixRect.w = *p++;
    sTim->pixRect.h = *p++;
    sTim->pixel = (u32 *)p;
    switch (sTim->mode) {
        case TIM_4BIT:
            sTim->perUnit = 4;
            /* MATCHING: a multiply, not a shift, loads the width with lh. */
            sTim->width = sTim->pixRect.w * 4;
            break;
        case TIM_8BIT:
            sTim->perUnit = 2;
            sTim->width = sTim->pixRect.w * 2;
            break;
        default:
            sTim->width = sTim->pixRect.w;
            break;
    }
    return 0;
}

s32 LoadTim(void *data) {
    RECT rect;
    RECT clut;
    TimInfo *info;
    u16 w;
    u16 h;

    if (ReadTimHeader(data) == -1) {
        return -1;
    }
    w = sTim->pixRect.w;
    h = sTim->pixRect.h;
    rect.x = sTim->pixRect.x;
    rect.y = sTim->pixRect.y;
    rect.w = sTim->pixRect.w;
    rect.h = sTim->pixRect.h;
    LoadImage(&rect, sTim->pixel);
    info = sTim;
    if (info->hasClut == 1) {
        clut.x = info->clutRect.x;
        clut.y = info->clutRect.y;
        clut.w = info->clutRect.w;
        clut.h = info->clutRect.h;
        LoadImage(&clut, info->clut);
    }
    return (s16)w * (s16)h * sTim->perUnit + (s32)sTim->pixel;
}

/* MATCHING: s32 with no value returned keeps the return register live,
 * so cc1 leaves the two forward branch delay slots empty. */
s32 LoadTimAt(s16 px, s16 py, s16 cx, s16 cy, u16 *tim) {
    RECT rect;
    RECT clut;
    TimInfo *info;
    TimInfo *clutInfo;

    if (ReadTimHeader(tim) == -1) {
        return;
    }
    info = (TimInfo *)gTimInfo;
    if (px != -1) {
        rect.x = px;
        rect.y = py;
        rect.w = info->pixRect.w;
        rect.h = info->pixRect.h;
        LoadImage(&rect, info->pixel);
    }
    if (cx != -1) {
        clutInfo = (TimInfo *)gTimInfo;
        if (clutInfo->hasClut == 1) {
            clut.x = cx;
            clut.y = cy;
            clut.w = clutInfo->clutRect.w;
            clut.h = clutInfo->clutRect.h;
            LoadImage(&clut, clutInfo->clut);
        }
    }
}

void LoadTimPack(PackEntry *pack) {
    PackEntry *e;
    u16 i;
    u16 n;

    e = pack;
    n = pack->count;
    for (i = 0; i < n; i++) {
        LoadTim((u8 *)pack + e->offset);
        e++;
        DrawSync(0);
    }
}

/* MATCHING: the red channel through its own local moves the colour's copy
 * first in the prologue, as retail. */
void SortSemiQuad(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri) {
    POLY_F4 poly;
    POLY_F4 *p;
    s32 r;

    p = &poly;
    SetPolyF4(p);
    SetSemiTrans(p, 1);
    r = col & 0x1F;
    p->r0 = r << 3;
    p->g0 = ((col >> 5) & 0x1F) << 3;
    p->b0 = ((col >> 10) & 0x1F) << 3;
    p->x0 = x0;
    p->y0 = y0;
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x3;
    p->y2 = y3;
    p->x3 = x2;
    p->y3 = y2;
    GsSortPoly(p, &gSubOt[gActiveBuff], pri);
}

s32 Random(void) {
    return rand();
}

void UpdateMotionViewer(void) {
    /* MATCHING: an unused local gives retail's 0x20-byte frame. */
    s32 unused[2];

    switch (gModeStep) {
        case 0:
            gClearR = 0x10;
            gClearG = 0x20;
            gClearB = 0x80;
            gRefView[0] = 0;
            gRefView[1] = 0;
            gRefView[2] = -1000;
            gRefView[3] = 0;
            gRefView[4] = 0;
            gRefView[5] = 0;
            gViewerAngle = -90;
            gPlayer.yaw = 0;
            gPlayer.lean = 0;
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gWorldShift[0] = 0;
            D_80095918 = 0;
            gWorldShift[2] = 0;
            gTodCoords[0].coord.t[0] = 0;
            gTodCoords[0].coord.t[1] = 0;
            gTodCoords[0].coord.t[2] = 0;
            gBodyRot[0] = 0;
            gBodyRot[1] = gPlayer.yaw;
            gBodyRot[2] = gPlayer.lean;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            gPlayer.motion[0] = MOT_DEFAULT;
            StepAnimation(&gPlayer, 0, 0);
            gModeStep++;
            break;
        case 1:
            if (gViewerReplay == 1) {
                gViewerReplay = StepAnimation(&gPlayer, 0, 0);
            } else {
                gPlayer.motion[0] = MOT_DEFAULT;
            }
            DrawTodObjects();
            SelectViewerMotion();
            OrbitViewerCamera();
            if (gPad & PADLright) {
                gTodCoords[0].coord.t[0]++;
            }
            if (gPad & PADLleft) {
                gTodCoords[0].coord.t[0]--;
            }
            if (gPad & PADLdown) {
                gTodCoords[0].coord.t[1]++;
            }
            if (gPad & PADLup) {
                gTodCoords[0].coord.t[1]--;
            }
            if (gPad & PADL2) {
                gTodCoords[0].coord.t[2]--;
            }
            if (gPad & PADR2) {
                gTodCoords[0].coord.t[2]++;
            }
            break;
    }
    FntPrint(gMotionFmt, gViewerMotion);
}

void OrbitViewerCamera(void) {
    gRefView[0] = rcos((gViewerAngle << 12) / 360) * 400 / 4096;
    gRefView[1] = -100;
    gRefView[2] = rsin((gViewerAngle << 12) / 360) * 400 / 4096;
    gRefView[6] = 0;
    gRefView[3] = 0;
    gRefView[4] = -100;
    gRefView[5] = 0;
    if (gPad & PADL1) {
        gViewerAngle += 2;
    }
    if (gPad & PADR1) {
        gViewerAngle -= 2;
    }
}

extern u8 sTodTmdIds[]; /**< SetTodFrame's TMD id table */

u8 StepAnimation(PlayerState *obj, u16 index, u8 arg) {
    u8 ret;
    s32 seq;

    ret = 1;
    if (obj->motion[0] == MOT_NONE) {
        /* MATCHING: keeps this return from cross-jumping into a later one. */
        __asm__("");
        return 0;
    }
    if (obj->motion[index] != obj->curMotion[index]) {
        obj->motionTime[index] = obj->motionStart[obj->motion[index]];
        obj->motionData[index] = (u8 *)gAnimations[obj->motion[index]];
        obj->curMotion[index] = obj->motion[index];
    }
    seq = obj->curMotion[index];
    if (obj->motionTime[index] >= obj->motionLen[seq] + obj->motionStart[seq]) {
        if (seq < 2) {
            return 0;
        }
        if (seq < 5) {
            ret = 2;
            obj->motionTime[index] = obj->motionStart[seq];
            obj->motionData[index] = (u8 *)gAnimations[obj->curMotion[index]];
        } else if ((s16)seq == 26) {
            ret = 2;
            /* MATCHING: lengthens &obj->motionStart[seq]'s live range (global-alloc). */
            __asm__("");
            __asm__("");
            __asm__("");
            obj->motionTime[index] = obj->motionStart[seq];
            obj->motionData[index] = (u8 *)gAnimations[obj->curMotion[index]];
        } else {
            return 0;
        }
    } else {
        obj->motionTime[index]++;
    }
    obj->motionData[index] = SetTodFrame(obj->motionTime[index], obj->motionData[index],
                                         (s32)gTodObjTable, (s32)sTodTmdIds, gModelTmd, 0, arg);
    return ret;
}

void InitLights(void) {
    GsFOGPARAM fog;

    sLights[0].vx = 0;
    sLights[0].vy = 100;
    sLights[0].vz = 100;
    GsSetFlatLight(0, &sLights[0]);
    sLights[1].vx = 86;
    sLights[1].vy = 100;
    sLights[1].vz = -50;
    GsSetFlatLight(1, &sLights[1]);
    sLights[2].vx = -86;
    sLights[2].vy = 100;
    sLights[2].vz = -50;
    sLights[0].r = 0xD0;
    sLights[0].g = 0xD0;
    sLights[0].b = 0xD0;
    sLights[1].r = 0xD0;
    sLights[1].g = 0xD0;
    sLights[1].b = 0xD0;
    sLights[2].r = 0xD0;
    sLights[2].g = 0xD0;
    sLights[2].b = 0xD0;
    GsSetFlatLight(2, &sLights[2]);
    GsSetAmbient(0x400, 0x400, 0x400);
    GsSetLightMode(0);
    fog.dqa = -0x3200;
    fog.dqb = 0x1400000;
    fog.rfc = gClearR;
    fog.gfc = gClearG;
    fog.bfc = gClearB;
    GsSetFogParam(&fog);
}

/** @brief The model list as the model draw walks it. */
typedef struct {
    GsDOBJ2 *objs; /**< the models */
    s32 count;     /**< how many */
} DrawList;

/* MATCHING: per-unit view; main.c reads the same slot as a Model70. */
extern s32 gAltModel[]; /**< main's Model70, read as words: [2] is its TMD */

s32 DrawTodObjects(void) {
    GsDOBJ2 save;
    MATRIX m;
    GsDOBJ2 *o;
    GsDOBJ2 *q;
    s32 i;

    if (gPlayer.hittable == 0 && (gFrameCount & 3) < 2) {
        if (gPlayer.action == ACT_RUN || gPlayer.action == ACT_RIDE_STRAIGHT) {
            return;
        }
    }
    o = (*(DrawList *)gTodObjTable).objs;
    for (i = 0; i < (*(DrawList *)gTodObjTable).count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id == -1) {
            continue;
        }
        if (o->tmd != NULL) {
            save = *o;
            q = o;
            if (o->id == 10 && gPlayer.altModel == 1) {
                o->tmd = (unsigned long *)gAltModel[2];
            }
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            SetSpadStack();
            if (gPlayer.falling == 0) {
                GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            } else {
                GsSortObject4J(o, &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
            }
            ResetSpadStack();
            /* MATCHING: the loop pointer is reloaded from the copy. */
            o = q;
            *o = save;
        }
        /* MATCHING: a barrier keeps the tmd branch's label off the loop test. */
        __asm__("");
    }
}

s32 FindFloor(Vec3i *pos) {
    Vec3i p;
    s32 out[4];
    u32 i;
    s32 found;
    u8 *blob;
    u8 *e;
    s32 verts;
    s16 *v;
    s32 dx;
    s32 dz;
    s32 d;

    p.x = pos->x;
    p.y = pos->y;
    p.z = pos->z;
    found = -1;
    for (i = 0; i < gStageObjCount; i++) {
        blob = (u8 *)gFloorData;
        /* MATCHING: integer sums, offset first, and the vertex base in
         * its own statement give retail's operand and load order. */
        e = (u8 *)(*(s32 *)(blob + i * 8 + 8) + (s32)blob);
        verts = *(s32 *)(blob + 4) + (s32)blob;
        v = (s16 *)(*(u16 *)(e + 4) * 8 + verts);
        dx = v[0] - p.x;
        dz = v[2] - p.z;
        d = dx * dx + dz * dz;
        if ((d < 0 ? -d : d) > 100000000) {
            continue;
        }
        if (QueryFloor(out, i, &p, gFloorData) != 0x7FFF) {
            found = i;
            break;
        }
    }
    return found;
}

/** @brief A sky band's four corner colours. */
typedef struct {
    CVECTOR c[4]; /**< top and bottom of the upper band, then of the lower */
} SkyColors;

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern SkyColors *gSkyColors;

void DrawSky(u32 arg) {
    /* MATCHING: volatile keeps every store in source order and in place. */
    volatile POLY_G4 *p;
    u32 *tag;
    s32 set;
    s32 dx;
    s32 dz;
    s32 y;

    dx = gView[3] - gView[0];
    dz = gView[5] - gView[2];
    /* MATCHING: two steps keep the shift and the mask in one register. */
    set = arg >> 10;
    set &= 3;
    y = gView[4] - gView[1];
    p = (volatile POLY_G4 *)GsOUT_PACKET_P;
    y = rsin(ratan2(y, SquareRoot0(dx * dx + dz * dz))) * 240 >> 12;
    if (gFogFade != 100 && gStage % 3 != 2) {
        p->r0 = p->r1 = gSkyColors[set].c[0].r * (gFogFade << 11) / 0x10000;
        p->g0 = p->g1 = gSkyColors[set].c[0].g * (gFogFade << 11) / 0x10000;
        p->b0 = p->b1 = gSkyColors[set].c[0].b * (gFogFade << 11) / 0x10000;
        p->r2 = p->r3 = gSkyColors[set].c[1].r * (gFogFade << 11) / 0x10000;
        p->g2 = p->g3 = gSkyColors[set].c[1].g * (gFogFade << 11) / 0x10000;
        p->b2 = p->b3 = gSkyColors[set].c[1].b * (gFogFade << 11) / 0x10000;
    } else {
        *(u32 *)&p->r0 = *(u32 *)&p->r1 = *(u32 *)&gSkyColors[set].c[0];
        *(u32 *)&p->r2 = *(u32 *)&p->r3 = *(u32 *)&gSkyColors[set].c[1];
    }
    p->x0 = -160;
    p->y0 = -160;
    p->x1 = 159;
    p->y1 = -160;
    p->x2 = -160;
    p->y2 = 8 - y;
    p->x3 = 159;
    p->y3 = 8 - y;
    setcode(p, 0x38);
    tag = (u32 *)gOt[gActiveBuff].org + 0xFFF;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x08000000;
    *tag = (u32)p & 0xFFFFFF;
    p++;
    if (gFogFade != 100 && gStage % 3 != 2) {
        p->r0 = p->r1 = gSkyColors[set].c[2].r * (gFogFade << 11) / 0x10000;
        p->g0 = p->g1 = gSkyColors[set].c[2].g * (gFogFade << 11) / 0x10000;
        p->b0 = p->b1 = gSkyColors[set].c[2].b * (gFogFade << 11) / 0x10000;
        p->r2 = p->r3 = gSkyColors[set].c[3].r * (gFogFade << 11) / 0x10000;
        p->g2 = p->g3 = gSkyColors[set].c[3].g * (gFogFade << 11) / 0x10000;
        p->b2 = p->b3 = gSkyColors[set].c[3].b * (gFogFade << 11) / 0x10000;
    } else {
        *(u32 *)&p->r0 = *(u32 *)&p->r1 = *(u32 *)&gSkyColors[set].c[2];
        *(u32 *)&p->r2 = *(u32 *)&p->r3 = *(u32 *)&gSkyColors[set].c[3];
    }
    p->x0 = -160;
    p->y0 = 8 - y;
    p->x1 = 159;
    p->y1 = 8 - y;
    p->x2 = -160;
    p->y2 = 119;
    p->x3 = 159;
    p->y3 = 119;
    setcode(p, 0x38);
    tag = (u32 *)gOt[gActiveBuff].org + 0xFFF;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x08000000;
    *tag = (u32)p & 0xFFFFFF;
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void SetCoordRotation(SVECTOR *rot, GsCOORDINATE2 *coord) {
    MATRIX m;
    SVECTOR r;

    m = GsIDMATRIX;
    m.t[0] = coord->coord.t[0];
    m.t[1] = coord->coord.t[1];
    m.t[2] = coord->coord.t[2];
    r = *rot;
    RotMatrixYXZ(&r, &m);
    coord->coord = m;
    coord->flg = 0;
}

void SelectViewerMotion(void) {
    s32 flags = gPadTrig;

    if (flags & PADRup) {
        gViewerMotion++;
    }
    if (flags & PADRleft) {
        gViewerMotion--;
    }
    if (gViewerMotion < 0) {
        gViewerMotion = 0;
    }
    if (gViewerMotion >= 0x79) {
        gViewerMotion = 0x78;
    }
    if (flags & PADRright) {
        gPlayer.curMotion[0] = MOT_OFF;
        gViewerReplay = 1;
        gPlayer.motion[0] = gViewerMotion;
    }
}

/** @brief Empty; called with the player when a jump starts. */
void OnJumpStart(void) {}

/** @brief Empty; called with the player when a jump lands. */
void OnJumpLand(void) {}

void InitView(void) {
    GsSetProjection(250);
    gRefView[0] = 0;
    gRefView[1] = 0;
    gRefView[2] = 4000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
    gRefView[6] = 0;
    gRefView[7] = 0;
    GsSetRefView2((GsRVIEW2 *)gRefView);
}

s16 Lerp(s16 from, s16 to, u16 step, u16 steps) {
    s32 v;

    if (step == steps) {
        return to;
    }
    /* MATCHING: one local carried through compound steps keeps every
     * stage of the arithmetic in one register. */
    v = (to - from) << 16;
    v /= steps;
    v *= step;
    v /= 0x10000;
    return from + v;
}

s32 FindFloorNear(void *pos, void *arg, s32 cur) {
    Vec3i p;

    p.x = ((Vec3i *)pos)->x;
    p.y = ((Vec3i *)pos)->y;
    p.z = ((Vec3i *)pos)->z;
    UpdateSection(&cur, p.x, p.z);
    if (QueryFloor(arg, cur, &p, gFloorData) == 0x7FFF) {
        return -1;
    }
    return cur;
}

/** @brief Empty; called with 0 when the boot files are read and 1 once their
 *         models are set up. */
void SetBootReady(void) {}

GsDOBJ2 *FindTodObj(TodObjTable *list, s32 key);
GsDOBJ2 *CreateTodObj(TodObjTable *list, s32 key);
GsDOBJ2 *RemoveTodObj(TodObjTable *list, s32 key);
s32 *FindTmdById(s32 *table, s32 *keys, s32 key);

u8 *SetTodPacket(u8 *data, s32 objTable, s32 tmdIds, s32 tmdTable, s32 mode, s8 filter) {
    s16 mask[3];
    s32 rot[3];
    s32 unused[6];
    GsDOBJ2 dummy;
    MATRIX dummyM;
    GsCOORD2PARAM dummyP;
    s16 k;
    s16 j;
    u32 w;
    u32 bits;
    s16 sign;
    s32 id;
    u32 type;
    GsDOBJ2 *obj;
    u32 flags;
    u32 words;
    u8 *p;
    s16 h;
    GsCOORDINATE2 *c;
    MATRIX *m;
    GsCOORD2PARAM *prm;
    s32 a;
    s32 v;
    GsDOBJ2 *s;

    w = *(u32 *)data;
    p = data + 4;
    bits = w & 0x7000;
    sign = w & 0x8000;
    id = w & 0xFFF;
    if (sign == 0 || id == 1) {
        type = (w >> 16) & 0xF;
        flags = (w >> 20) & 0xF;
        words = w >> 24;
    } else {
        type = 1;
        /* MATCHING: loop depth 3 weights flags' refs (global-alloc order). */
        do {
            do {
                flags = 2;
            } while (0);
        } while (0);
        k = 0;
        words = 0;
        for (; k < 3; k++) {
            mask[k] = bits & (0x4000 >> k);
            if (mask[k] != 0) {
                words++;
            }
        }
        if (words != 0) {
            words = 1;
        } else {
            words = 2;
        }
    }
    if (filter & 0x7F) {
        switch (id) {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                if ((filter & 0x7F) == 1) {
                    return data + words * 4;
                }
                break;
            case 1:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
                if ((filter & 0x7F) == 2) {
                    return data + words * 4;
                }
                break;
        }
    }
    obj = FindTodObj((TodObjTable *)objTable, id);
    if (obj == NULL) {
        obj = &dummy;
        m = &dummyM;
        prm = &dummyP;
    } else {
        c = obj->coord2;
        prm = c->param;
        m = &c->coord;
        c->flg = 0;
    }
    switch (type) {
        case 1:
            if (flags & PADL2) {
                break;
            }
            if (flags & PADR2) {
                if (sign != 0) {
                    if (id == type) {
                        for (k = 0; k < 3; k++) {
                            rot[k] = (h = ((s16 *)p)[k]) & 0xFFFF;
                            if (h & 0x8000) {
                                rot[k] |= 0xFFFF0000;
                            }
                            rot[k] <<= 12;
                        }
                        p += 8;
                    } else if ((id != 2 && id != 10) || !(filter & 0x80)) {
                        if (words == 2) {
                            for (k = 0; k < 3; k++) {
                                rot[k] = (h = ((s16 *)p)[k - 1]) & 0xFFFF;
                                if (h & 0x8000) {
                                    rot[k] |= 0xFFFF0000;
                                }
                                rot[k] <<= 12;
                            }
                            p += 4;
                        } else {
                            j = 0;
                            for (k = 0; k < 3; k++) {
                                if (mask[k] != 0) {
                                    rot[k] = 0;
                                } else {
                                    rot[k] = (h = ((s16 *)p)[j - 1]) & 0xFFFF;
                                    if (h & 0x8000) {
                                        rot[k] |= 0xFFFF0000;
                                    }
                                    j++;
                                    rot[k] <<= 12;
                                }
                            }
                        }
                    } else {
                        a = ratan2(gTodCoords[0].coord.t[0] - gPlayer.lookX,
                                   gTodCoords[0].coord.t[2] - gPlayer.lookZ);
                        v = gBodyYaw[0];
                        a -= v;
                        a &= 0xFFF;
                        if (id == 2) {
                            if (a < 0x800) {
                                if (a > 0x2AA) {
                                    a = 0x2AA;
                                }
                            } else if (a < 0xD55) {
                                a = 0xD55;
                            }
                            v = a;
                            if (v > 0x800) {
                                v -= 0x1000;
                            }
                            if (gLookYawNarrow < v) {
                                gLookYawNarrow += 8;
                            } else if (v < gLookYawNarrow) {
                                gLookYawNarrow -= 8;
                            }
                            v = gLookYawNarrow;
                            gPlayer.lookYaw = v;
                        } else {
                            if (a < 0x800) {
                                if (a > 0x38E) {
                                    a = 0x38E;
                                }
                            } else if (a < 0xC71) {
                                a = 0xC71;
                            }
                            v = a;
                            if (v > 0x800) {
                                v -= 0x1000;
                            }
                            if (gLookYawWide < v) {
                                gLookYawWide += 50;
                            } else if (v < gLookYawWide) {
                                gLookYawWide -= 50;
                            }
                            v = gLookYawWide;
                        }
                        rot[1] = v * 360;
                    }
                    prm->rotate.vx = rot[0] / 360;
                    prm->rotate.vy = rot[1] / 360;
                    prm->rotate.vz = rot[2] / 360;
                } else {
                    prm->rotate.vx = ((s32 *)p)[0] / 360;
                    prm->rotate.vy = ((s32 *)p)[1] / 360;
                    prm->rotate.vz = ((s32 *)p)[2] / 360;
                    p += 12;
                }
                obj->coord2->flg = 0;
                RotMatrix(&prm->rotate, m);
            }
            if (flags & PADL1) {
                prm->scale.vx = ((s16 *)p)[0];
                prm->scale.vy = ((s16 *)p)[1];
                prm->scale.vz = ((s16 *)p)[2];
                p += 8;
            } else {
                if (gPlayer.flattened == 0) {
                    prm->scale.vx = 0x1000;
                    prm->scale.vy = 0x1000;
                    prm->scale.vz = 0x1000;
                } else {
                    prm->scale.vx = 0x1000;
                    prm->scale.vy = 0x1000;
                    prm->scale.vz = 0;
                }
                if ((gPlayer.rideMode & RIDE_MASK) == RIDE_COVER && (u32)id < 11 && (u32)id >= 2) {
                    prm->scale.vx = 0;
                    prm->scale.vy = 0;
                    prm->scale.vz = 0;
                }
            }
            if (!(flags & PADR2)) {
                RotMatrix(&prm->rotate, m);
            }
            ScaleMatrix(m, &prm->scale);
            if (flags & PADR1) {
                prm->trans.vx = ((s32 *)p)[0] / 5;
                prm->trans.vy = ((s32 *)p)[1] / 5;
                prm->trans.vz = ((s32 *)p)[2] / 5;
                TransMatrix(m, &prm->trans);
            }
            break;
        case 2:
            if (tmdTable != 0) {
                GsLinkObject4((u_long)FindTmdById((s32 *)tmdTable, (s32 *)tmdIds, *(u16 *)p), obj, 0);
            }
            break;
        case 3:
            if (mode != 2) {
                if (*(s32 *)p == 0 || *(s32 *)p == 0xFFFF) {
                    obj->coord2->super = NULL;
                } else {
                    s = FindTodObj((TodObjTable *)objTable, *(s32 *)p);
                    obj->coord2->super = s->coord2;
                }
            }
            break;
        case 8:
            if (objTable != 0) {
                switch (flags) {
                    case 0:
                        CreateTodObj((TodObjTable *)objTable, id);
                        break;
                    case 1:
                        RemoveTodObj((TodObjTable *)objTable, id);
                        break;
                }
            }
            break;
    }
    return data + words * 4;
}

u8 *SetTodFrame(u32 time, u8 *data, s32 objTable, s32 tmdIds, s32 tmdTable, s32 mode, s8 filter) {
    u32 i;
    u32 n;

    n = *(u16 *)(data + 2);
    if (time < *(u32 *)(data + 4)) {
        return data;
    }
    data += 8;
    for (i = 0; i < n; i++) {
        data = SetTodPacket(data, objTable, tmdIds, tmdTable, mode, filter);
    }
    return data;
}

void InitTodObjTable(TodObjTable *table, GsDOBJ2 *objs, GsCOORDINATE2 *coords,
                     GsCOORD2PARAM *params, s32 n) {
    s32 i;

    table->count = 0;
    table->top = objs;
    table->capacity = n;
    for (i = 0; i < n; i++) {
        objs->attribute = GsDOFF;
        objs->id = -1;
        objs->coord2 = coords;
        coords->param = params;
        objs->tmd = 0;
        objs++;
        coords++;
        params++;
    }
}

GsDOBJ2 *FindTodObj(TodObjTable *list, s32 key) {
    GsDOBJ2 *s;
    s32 i;

    s = list->top;
    for (i = 0; i < list->count; i++) {
        if (key == s->id) {
            break;
        }
        s++;
    }
    if (i == list->count) {
        return NULL;
    }
    return s;
}

GsDOBJ2 *CreateTodObj(TodObjTable *list, s32 key) {
    GsDOBJ2 *s;
    s32 i;

    s = list->top;
    for (i = 0; i < list->count; i++) {
        if (s->id == -1) {
            break;
        }
        s++;
    }
    if (i < list->count) {
        s->id = key;
        s->attribute = 0;
        GsInitCoordinate2(NULL, s->coord2);
        s->tmd = 0;
        return s;
    }
    if (i < list->capacity) {
        s = &list->top[list->count];
        list->count++;
        s->id = key;
        s->attribute = 0;
        GsInitCoordinate2(NULL, s->coord2);
        s->tmd = 0;
        return s;
    }
    return NULL;
}

/* The key is never compared: the search stops at the first free object, not
 * at the one a kill packet names, so no live object is ever freed. */
GsDOBJ2 *RemoveTodObj(TodObjTable *list, s32 key) {
    GsDOBJ2 *s;
    s32 i;

    s = list->top;
    for (i = 0; i < list->count; i++) {
        if (s->id == -1) {
            break;
        }
        s++;
    }
    if (i < list->count) {
        s->id = -1;
        if (i == list->count - 1) {
            do {
                list->count--;
                s--;
            } while (s->id == -1);
        }
        return s;
    }
    return NULL;
}

s32 *FindTmdById(s32 *table, s32 *keys, s32 key) {
    s32 n;

    table++;
    n = *table++;
    while (n > 0) {
        if (key == *keys) {
            break;
        }
        table += 7;
        n--;
        keys++;
    }
    if (n == 0) {
        return NULL;
    }
    return table;
}

INCLUDE_RODATA("asm/nonmatchings/model", gTargetTimes);
