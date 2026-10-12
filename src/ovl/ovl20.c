/* The world 3 intro, an overlay main loads for GM_WORLD_INTRO and runs each
 * frame. It loads the intro's models, animations and sprites, then plays a
 * scripted scene of the player and a partner model under fifteen lines of
 * rising text until the script ends or START skips it, draws one black
 * frame and starts stage 3-1. Unlike the other world intros it plays no XA
 * track and shows no captions. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "libetc.h"
#include "rand.h"
#include "model.h"
#include "draw.h"
#include "sound.h"
#include "spad.h"
#include "world_intro.h"

/* MATCHING: model types these as its own records; here they are the
 * overlay's tables, as in main. */
void InitTodObjTable(s32 *list, u8 *slots, GsCOORDINATE2 *objs, u8 *data, s32 n);
/* MATCHING: main's s32 view, which keeps sound.h from declaring it. */
s32 LoadVabBank(s32 bank);

/* MATCHING: all s32 where model has s16: the view's words are passed
 * and stored whole. */
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

extern CdlLOC D_800F1368[];
extern s32 D_800F13B8[];
extern GsCOORDINATE2 D_800F1D70;
extern PlayerState D_800F1DC0;
extern u8 D_800F2198[];
extern s32 D_800F22D8;
extern s32 D_800F22E0[];
extern GsCOORDINATE2 D_800F22F0[];
extern GsDOBJ2 D_800F2930;
extern VECTOR D_800F2940;
extern SVECTOR D_800F2950;
extern u8 D_800F2958[];

/* The camera script's step. */
/* MATCHING: kept in the fog fade's word, reached as its own symbol. */
#define sSubStep gFogFade

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

u8 func_800F0068(PlayerState *obj, u16 index, s32 objTable, s32 tmdTable) {
    if (obj->motion[index] != obj->curMotion[index]) {
        obj->motionTime[index] = obj->motionStart[obj->motion[index]];
        obj->motionData[index] = (u8 *)gAnimations[obj->motion[index]];
        obj->curMotion[index] = obj->motion[index];
    }
    if (obj->motionTime[index] >=
        obj->motionLen[obj->curMotion[index]] + obj->motionStart[obj->curMotion[index]]) {
        return 0;
    }
    obj->motionTime[index]++;
    obj->motionData[index] = SetTodFrame(obj->motionTime[index], obj->motionData[index], objTable,
                                         (s32)gTodTmdIds, tmdTable, 0, 0);
    return 1;
}

void func_800F0160(s32 *pack) {
    s32 *dir;
    s32 i;
    s32 n;

    dir = (s32 *)(pack[0] + (s32)pack);
    D_800F22D8 = dir[0] + (s32)dir + 4;
    GsMapModelingData((unsigned long *)D_800F22D8);
    InitTodObjTable(D_800F22E0, D_800F2198, D_800F22F0, D_800F2958, 20);
    GsInitCoordinate2(WORLD, D_800F22F0);
    GsMapModelingData((unsigned long *)(dir[1] + (s32)dir + 4));
    GsLinkObject4((unsigned long)(dir[1] + (s32)dir + 12), &D_800F2930, 0);
    D_800F2930.coord2 = &D_800F1D70;
    D_800F2930.attribute = 0;
    GsInitCoordinate2(WORLD, &D_800F1D70);
    dir = (s32 *)(pack[1] + (s32)pack);
    n = dir[0] / 4;
    for (i = 0; i < n; i++) {
        gAnimations[i + 51] = (s32 *)(dir[i] + (s32)dir) + 1;
        gPlayer.motionLen[i + 51] = *gAnimations[i + 51];
        gAnimations[i + 51]++;
        gPlayer.motionStart[i + 51] = gAnimations[i + 51][1];
    }
    LoadTimList((unsigned long *)(pack[2] + (s32)pack));
    for (i = 0; i < 15; i++) {
        DefineSprite(SPR_STAGE + i, 0, 0xFF, 0x10, 5, 0, (u8)(i * 16), 0, 0x1FD);
    }
    DefineSprite(SPR_STAGE + 30, 0, 0xFF, 0x58, 6, 0, 0, 0x10, 0x1FD);
}

/* MATCHING: inline, so that the intro's frame step expands it and the
 * compiler writes this copy after it. */
inline void func_800F11FC(void) {
    MATRIX m;

    D_800F1D70.coord.t[0] = D_800F2940.vx;
    D_800F1D70.coord.t[1] = D_800F2940.vy;
    D_800F1D70.coord.t[2] = D_800F2940.vz;
    SetCoordRotation(&D_800F2950, &D_800F1D70);
    GsGetLs(D_800F2930.coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(D_800F2930.coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&D_800F2930, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}

/* Draws a model's objects at its position and yaw. It counts the player's
 * objects whichever list it is handed. */
static inline void DrawIntroModel(PlayerState *p, s32 *list) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    o = ((DrawList *)list)->objs;
    o->coord2->coord.t[0] = p->x;
    o->coord2->coord.t[1] = p->y;
    o->coord2->coord.t[2] = p->z;
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = p->yaw;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; o++, i++) {
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
        /* MATCHING: three barriers lengthen the loop so that global-alloc
         * ranks the object pointer above the counter (retail's s1 and s2). */
        __asm__("");
        __asm__("");
        __asm__("");
    }
}

/* Steps the player's first animation channel and draws the player. */
static inline void DrawIntroPlayer(void) {
    s8 r;

    gPlayer.motionResult[0] = r = func_800F0068(&gPlayer, 0, (s32)gTodObjTable, gModelTmd);
    switch (gPlayer.action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (r == 0) {
                gPlayer.curMotion[0] = MOT_OFF;
            }
            break;
    }
    DrawIntroModel(&gPlayer, gTodObjTable);
}

/* The same for the partner, from its own model list. */
static inline void DrawIntroPartner(void) {
    s8 r;

    D_800F1DC0.motionResult[0] = r = func_800F0068(&D_800F1DC0, 0, (s32)D_800F22E0, D_800F22D8);
    switch (D_800F1DC0.action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (r == 0) {
                D_800F1DC0.curMotion[0] = MOT_OFF;
            }
            break;
    }
    DrawIntroModel(&D_800F1DC0, D_800F22E0);
}

/* Draws the scrolling text: fifteen lines that rise with @p time, fading
 * in near the bottom of the screen and out near the top. */
void func_800F0364(s32 time) {
    SVECTOR pos;
    SColor color;
    s32 i;

    time /= 2;
    for (i = 0; i < 15; i++) {
        pos.vx = -0x80;
        pos.vy = i * 16 + 0x68 - time;
        color.r = BLEND_ADD;
        if (pos.vy > 0x68) {
            color.g = color.b = color.cd = 0;
        } else if (pos.vy > 0x58) {
            color.g = color.b = color.cd = (0x68 - pos.vy) * 8;
        } else if (pos.vy > 0x38) {
            color.g = color.b = color.cd = 0x80;
        } else if (pos.vy > 0x28) {
            color.g = color.b = color.cd = (pos.vy - 0x28) * 8;
        } else {
            color.g = color.b = color.cd = 0;
        }
        DrawSprite(SPR_STAGE + i, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
}

/* MATCHING: non-void with no return keeps step 2's delay slot a nop. */
s32 func_800F0480(void) {
    SVECTOR pos;
    POLY_F4 *p;

    switch (gWorldIntroStep) {
        case 0:
            gCdLoadLoc[0] = (s32)D_800F1368;
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gWorldIntroStep = 1;
            gCdLoadSectors[0] = D_800F13B8[0];
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                func_800F0160((s32 *)0x8019D000);
                gVabLoadStep = 0;
                gWorldIntroStep = 2;
            }
            break;
        case 2:
            if (LoadVabBank(4) == 1) {
                gLoading = 0;
                gWorldIntroStep = 3;
            }
            break;
        case 3:
            D_800F1DC0 = gPlayer;
            D_800F1DC0.motion[0] = 0x38;
            D_800F1DC0.curMotion[0] = MOT_OFF;
            D_800F1DC0.motion[1] = 0x38;
            D_800F1DC0.curMotion[1] = MOT_OFF;
            func_800F0068(&D_800F1DC0, 0, (s32)D_800F22E0, D_800F22D8);
            sLights.l[0].r = 0xFF;
            sLights.l[0].g = 0xFF;
            sLights.l[0].b = 0xFF;
            sLights.l[1].r = 0xFF;
            sLights.l[1].g = 0xFF;
            sLights.l[1].b = 0xFF;
            sLights.l[2].r = 0xFF;
            sLights.l[2].g = 0xFF;
            sLights.l[2].b = 0xFF;
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
            gPlayer.z = -0x1644;
            gPlayer.yaw = 0x800;
            gPlayer.motion[0] = 0x33;
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            D_800F1DC0.x = -100;
            D_800F1DC0.yaw = -0x200;
            D_800F1DC0.motion[0] = 0x39;
            D_800F1DC0.y = 0;
            D_800F1DC0.z = 100;
            D_800F1DC0.action = ACT_MODEL_LOOP;
            D_800F2950.vx = 0;
            D_800F2950.vy = 0;
            D_800F2950.vz = 0;
            D_800F2940.vx = -250;
            D_800F2940.vy = 0;
            D_800F2940.vz = 250;
            sView.vpx = 0;
            sView.vpy = -200;
            sView.vpz = -0x17D4;
            sView.vrx = 0;
            sView.vry = 0;
            sView.vrz = -0x1450;
            D_800D39D8.timer = 0;
            sSubStep = 0;
            gWorldIntroStep = 10;
            break;
        case 10:
            DrawFrozenFrame();
            switch (sSubStep) {
                case 0:
                    sView.vpx = 0;
                    sView.vpy = -100;
                    sView.vry = -100;
                    sView.vrx = 0;
                    gPlayer.z += 30;
                    sView.vpz = gPlayer.z + 200 + D_800D39D8.timer * 2;
                    sView.vrz = gPlayer.z;
                    GsSetRefView2(&sView);
                    if (++D_800D39D8.timer >= 50) {
                        D_800D39D8.timer = 0;
                        sSubStep = 3;
                    }
                    break;
                case 3:
                    gPlayer.z += 30;
                    if (D_800D39D8.timer < 90) {
                        sView.vpx = rsin((-(D_800D39D8.timer * 2) + 180) * 4096 / 360) * 300 / 4096;
                        sView.vpy = -100;
                        sView.vpz = gPlayer.z -
                                    rcos((-(D_800D39D8.timer * 2) + 180) * 4096 / 360) * 300 / 4096;
                        sView.vrx = 0;
                        sView.vry = -100;
                        sView.vrz = gPlayer.z;
                    } else {
                        sView.vpx = rsin(0) * 300 / 4096;
                        sView.vpy = -100;
                        sView.vpz = gPlayer.z - rcos(0) * 300 / 4096;
                        sView.vrx = 0;
                        sView.vry = -100;
                        sView.vrz = gPlayer.z;
                    }
                    D_800D39D8.timer++;
                    if (gPlayer.z >= 0) {
                        gView[0] = -250;
                        gView[1] = -200;
                        gView[2] = -150;
                        gView[3] = -50;
                        gView[4] = -100;
                        gView[5] = 50;
                        gPlayer.motion[0] = 0x34;
                        gPlayer.action = ACT_MODEL_ONCE;
                        gPlayer.yaw = 0x600;
                        D_800F1DC0.motion[0] = 0x3A;
                        D_800F1DC0.action = ACT_MODEL_ONCE;
                        D_800D39D8.timer = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    sView.vpx = Lerp(sView.vpx, gView[0], D_800D39D8.timer, 15);
                    sView.vpy = Lerp(sView.vpy, gView[1], D_800D39D8.timer, 15);
                    sView.vpz = Lerp(sView.vpz, gView[2], D_800D39D8.timer, 15);
                    sView.vrx = Lerp(sView.vrx, gView[3], D_800D39D8.timer, 15);
                    sView.vry = Lerp(sView.vry, gView[4], D_800D39D8.timer, 15);
                    sView.vrz = Lerp(sView.vrz, gView[5], D_800D39D8.timer, 15);
                    if (++D_800D39D8.timer == 15) {
                        gPlayer.motion[0] = 0x35;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800D39D8.timer = 0;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    if (D_800F1DC0.motionResult[0] == 0) {
                        D_800F1DC0.motion[0] = rand() % 2 + 0x3A;
                        D_800F1DC0.curMotion[0] = MOT_OFF;
                        D_800F1DC0.action = 0;
                    }
                    func_800F0364(D_800D39D8.timer);
                    if (++D_800D39D8.timer == 0x260) {
                        D_800D39D8.timer = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.motion[0] = 0x36;
                        gPlayer.curMotion[0] = MOT_OFF;
                        gPlayer.action = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.motion[0] = 0x37;
                        gPlayer.curMotion[0] = MOT_OFF;
                        gPlayer.action = 0;
                        D_800F1DC0.motion[0] = 0x3C;
                        D_800F1DC0.curMotion[0] = MOT_OFF;
                        D_800F1DC0.action = 0;
                        D_800D39D8.timer = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.yaw = 0x800;
                        gPlayer.motion[0] = 0x33;
                        gPlayer.curMotion[0] = MOT_OFF;
                        gPlayer.action = 1;
                    }
                    if (D_800F1DC0.motionResult[0] == 0) {
                        D_800F1DC0.motion[0] = 0x3D;
                        D_800F1DC0.curMotion[0] = MOT_OFF;
                        D_800F1DC0.action = 1;
                    }
                    if (gPlayer.action == 1) {
                        gPlayer.z += 30;
                        if (++D_800D39D8.timer == 90) {
                            gWorldIntroStep = 100;
                        }
                    }
                    break;
            }
            if (gPadTrig & PADstart) {
                gWorldIntroStep = 100;
            }
            DrawIntroPlayer();
            DrawIntroPartner();
            func_800F11FC();
            break;
        case 100:
            pos.vx = -0x80;
            pos.vy = -50;
            DrawSprite(SPR_STAGE + 30, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0;
            setXY4(p, -160, -120, 160, -120, -160, 120, 160, 120);
            addPrim(gSubOt[gActiveBuff].org, p);
            gWorldIntroStep++;
            p++;
            GsOUT_PACKET_P = (u8 *)p;
            break;
        case 101:
            gStage = STAGE_3_1;
            BeginLoadScreenModel();
            break;
    }
}
