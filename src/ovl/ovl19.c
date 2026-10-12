/* The world 2 intro, an overlay main loads for GM_WORLD_INTRO and runs each
 * frame. It loads the intro's models, animations and sprites, starts its XA
 * track, then plays a scripted scene of the player and a partner model with
 * captions until the script ends or START skips it, and starts stage 2-1. */

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

/** @brief The caption table: each caption's first frame, and the next one's. */
typedef struct {
    s32 t[8]; /**< frame numbers */
} CaptionTimes;

extern CdlLOC sIntroPackLoc[];
extern s32 sIntroPackSectors[];
extern CdlLOC sXaLoc;
extern s32 sIntroFrame;
extern s32 sScriptPaused;
extern GsCOORDINATE2 sPropCoord;
extern PlayerState sPartner;
extern u8 sPartnerSlots[];
extern s32 sPartnerTmd;
extern s32 sPartnerObjs[];
extern GsCOORDINATE2 sPartnerCoords[];
extern GsDOBJ2 sProp;
extern VECTOR sPropPos;
extern SVECTOR sPropRot;
extern u8 sPartnerData[];

/* The camera script's step. */
/* MATCHING: kept in the fog fade's word, reached as its own symbol. */
#define sSubStep gFogFade

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

const CaptionTimes sCaptionTimes = {{0xED, 0x10B, 0x144, 0x1A4, 0x219, 0x294, 0x33C, 0x3AB}};

u8 StepIntroMotion(PlayerState *obj, u16 index, s32 objTable, s32 tmdTable) {
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

void LoadIntroPack(s32 *pack) {
    s32 *dir;
    s32 i;
    s32 n;

    dir = (s32 *)(pack[0] + (s32)pack);
    sPartnerTmd = dir[0] + (s32)dir + 4;
    GsMapModelingData((unsigned long *)sPartnerTmd);
    InitTodObjTable(sPartnerObjs, sPartnerSlots, sPartnerCoords, sPartnerData, 20);
    GsInitCoordinate2(WORLD, sPartnerCoords);
    GsMapModelingData((unsigned long *)(dir[1] + (s32)dir + 4));
    GsLinkObject4((unsigned long)(dir[1] + (s32)dir + 12), &sProp, 0);
    sProp.coord2 = &sPropCoord;
    sProp.attribute = 0;
    GsInitCoordinate2(WORLD, &sPropCoord);
    dir = (s32 *)(pack[1] + (s32)pack);
    n = dir[0] / 4;
    for (i = 0; i < n; i++) {
        gAnimations[i + 51] = (s32 *)(dir[i] + (s32)dir) + 1;
        gPlayer.motionLen[i + 51] = *gAnimations[i + 51];
        gAnimations[i + 51]++;
        gPlayer.motionStart[i + 51] = gAnimations[i + 51][1];
    }
    LoadTimList((unsigned long *)(pack[2] + (s32)pack));
    for (i = 0; i < 7; i++) {
        DefineSprite(SPR_STAGE + i, 0, 0xFF, 0x20, 5, 0, (u8)(i * 32), 0, 0x1FE);
    }
    DefineSprite(SPR_STAGE + 30, 0, 0xFF, 0x58, 6, 0, 0, 0x10, 0x1FD);
}

/* MATCHING: inline, so that the intro's frame step expands it and the
 * compiler writes this copy after it. */
inline void DrawCaptions(s32 time) {
    SVECTOR pos;
    SColor color;
    CaptionTimes times;
    s32 i;

    times = sCaptionTimes;
    for (i = 0; i < 7; i++) {
        if (times.t[i] < time && time < times.t[i + 1]) {
            pos.vx = -0x80;
            pos.vy = 0x48;
            color.r = BLEND_OPAQUE;
            color.cd = 0x80;
            color.b = 0x80;
            color.g = 0x80;
            DrawSprite(SPR_STAGE + i, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
    }
}

/* MATCHING: inline, as above. */
inline void DrawIntroProp(void) {
    MATRIX m;

    sPropCoord.coord.t[0] = sPropPos.vx;
    sPropCoord.coord.t[1] = sPropPos.vy;
    sPropCoord.coord.t[2] = sPropPos.vz;
    SetCoordRotation(&sPropRot, &sPropCoord);
    GsGetLs(sProp.coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sProp.coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sProp, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
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

    gPlayer.motionResult[0] = r = StepIntroMotion(&gPlayer, 0, (s32)gTodObjTable, gModelTmd);
    switch (gPlayer.action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (r == 0) {
                gPlayer.curMotion[0] = MOT_OFF;
            }
            break;
    }
    SetTmdSortMode(0);
    DrawIntroModel(&gPlayer, gTodObjTable);
}

/* The same for the partner, from its own model list. */
static inline void DrawIntroPartner(void) {
    s8 r;

    sPartner.motionResult[0] = r = StepIntroMotion(&sPartner, 0, (s32)sPartnerObjs, sPartnerTmd);
    switch (sPartner.action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (r == 0) {
                sPartner.curMotion[0] = MOT_OFF;
            }
            break;
    }
    SetTmdSortMode(4);
    DrawIntroModel(&sPartner, sPartnerObjs);
}

void StepWorldIntro(void) {
    /* MATCHING: never used; its slot puts the inline functions' frames at
     * sp+0x20. */
    SVECTOR unused;

    switch (gWorldIntroStep) {
        case 0:
            gCdLoadLoc[0] = (s32)sIntroPackLoc;
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gWorldIntroStep = 1;
            gCdLoadSectors[0] = sIntroPackSectors[0];
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadIntroPack((s32 *)0x8019D000);
                gVabLoadStep = 0;
                gWorldIntroStep = 2;
            }
            break;
        case 2:
            if (LoadVabBank(3) == 1) {
                gLoading = 0;
                CdControlF(CdlSeekL, (u_char *)&sXaLoc);
                gWorldIntroStep = 3;
            }
            break;
        case 3:
            CdSync(0, 0);
            PlayXa(1);
            sIntroFrame = 0;
            gWorldIntroStep = 4;
            break;
        case 4:
            sPartner = gPlayer;
            sPartner.motion[0] = 0x38;
            sPartner.curMotion[0] = MOT_OFF;
            sPartner.motion[1] = 0x38;
            sPartner.curMotion[1] = MOT_OFF;
            StepIntroMotion(&sPartner, 0, (s32)sPartnerObjs, sPartnerTmd);
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
            gPlayer.z = -0x1644;
            gPlayer.yaw = 0x800;
            gPlayer.motion[0] = 0x33;
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.curMotion[0] = MOT_OFF;
            gPlayer.action = ACT_MODEL_LOOP;
            sPartner.x = -100;
            sPartner.yaw = -0x200;
            sPartner.motion[0] = 0x39;
            sPartner.y = 0;
            sPartner.z = 100;
            sPartner.action = ACT_MODEL_LOOP;
            sPropRot.vx = 0;
            sPropRot.vy = 0x400;
            sPropRot.vz = 0;
            sPropPos.vx = -250;
            sPropPos.vy = 0;
            sPropPos.vz = 400;
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
            DrawCaptions(sIntroFrame);
            if (sScriptPaused == 0) {
                switch (sSubStep) {
                    case 0:
                        gPlayer.z += 30;
                        sView.vpx = rsin(0x800) * (D_800D39D8.timer * 2 + 200) / 4096 + gPlayer.x;
                        sView.vpy = -100 - (D_800D39D8.timer >> 1);
                        sView.vpz = rcos(0x800) * (D_800D39D8.timer * 2 + 200) / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        /* MATCHING: `/ 2` where vpy has `>> 1`; the timer is then reloaded
                         * for the increment. */
                        sView.vry = -100 - (D_800D39D8.timer / 2);
                        sView.vrz = gPlayer.z;
                        if (++D_800D39D8.timer >= 100) {
                            D_800D39D8.timer = 0;
                            sSubStep = 3;
                        }
                        break;
                    case 3:
                        gPlayer.z += 30;
                        sView.vpx = rsin(0x800) * 400 / 4096 + gPlayer.x;
                        sView.vpy = -150;
                        sView.vpz = rcos(0x800) * 400 / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        sView.vry = -150;
                        sView.vrz = gPlayer.z;
                        D_800D39D8.timer++;
                        if (gPlayer.z >= -0xE1) {
                            gPlayer.motion[0] = 0x34;
                            gPlayer.action = ACT_MODEL_ONCE;
                            sPartner.motion[0] = 0x3A;
                            sPartner.action = ACT_MODEL_ONCE;
                            D_800D39D8.timer = 30;
                            sSubStep = 5;
                        }
                        break;
                    case 5:
                        gPlayer.z += D_800D39D8.timer;
                        gPlayer.yaw -= 0x22;
                        sView.vpx = rsin(0x800) * 400 / 4096 + gPlayer.x;
                        sView.vpy = -150;
                        sView.vpz = rcos(0x800) * 400 / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        sView.vry = -150;
                        sView.vrz = gPlayer.z;
                        if ((D_800D39D8.timer -= 2) == 0) {
                            gPlayer.yaw = 0x600;
                            gPlayer.motion[0] = 0x35;
                            gPlayer.action = ACT_MODEL_ONCE;
                            D_800D39D8.timer = 0;
                            sSubStep = 10;
                        }
                        break;
                    case 10:
                        if (sPartner.motionResult[0] == 0) {
                            sPartner.motion[0] = rand() % 2 + 0x3A;
                            sPartner.curMotion[0] = MOT_OFF;
                            sPartner.action = 0;
                        }
                        if (sIntroFrame == 324) {
                            sView.vpx = 110;
                            sView.vpy = -200;
                            sView.vpz = 0x154;
                            sView.vrx = -60;
                            sView.vry = -80;
                            sView.vrz = 40;
                        }
                        if (sIntroFrame == 939) {
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
                            sPartner.motion[0] = 0x3C;
                            sPartner.curMotion[0] = MOT_OFF;
                            sPartner.action = 0;
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
                        if (sPartner.motionResult[0] == 0) {
                            sPartner.motion[0] = 0x3D;
                            sPartner.curMotion[0] = MOT_OFF;
                            sPartner.action = 1;
                        }
                        if (gPlayer.action == 1) {
                            gPlayer.z += 30;
                            if (++D_800D39D8.timer == 90) {
                                StopXa();
                                gStage = STAGE_2_1;
                                BeginLoadScreenModel();
                            }
                        }
                        sView.vpx = rsin(0x800) * 400 / 4096 + gPlayer.x;
                        sView.vpy = -150;
                        sView.vpz = rcos(0x800) * 400 / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        sView.vry = -150;
                        sView.vrz = gPlayer.z;
                        break;
                }
                GsSetRefView2(&sView);
            }
            if (gPadTrig & PADstart) {
                StopXa();
                gStage = STAGE_2_1;
                BeginLoadScreenModel();
            }
            DrawIntroPlayer();
            DrawIntroPartner();
            DrawIntroProp();
            break;
    }
    sIntroFrame++;
}
