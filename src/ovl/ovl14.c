/* Stage 4-3's overlay: the act-3 chase of world 4, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 10240000
#define CHASE_PLACEMENTS 53
#define CHASE_VAB 5
#define CHASE_LIGHT_BLUE 0x80
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0

#include "chase.inc"
#include "rand.h"
#include "spad.h"
#include "prim.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

/* Starts a jump on a face button, unless the play is frozen. */
void StartChaseJump(void) {
    if (ClipF == 0 && (gGamePadTrig & (PADRleft | PADRdown | PADRright))) {
        s32 state;

        sJumpSpeed = -40;
        /* MATCHING: the 1 held in a local is loaded ahead of gPlayer's address. */
        state = 1;
        gPlayer.slideState = state;
        gPlayer.unk38C = 0;
        gPlayer.curMotion[0] = MOT_OFF;
        PlaySe(0x17);
    }
}

#include "chase_head.inc"

/* Piece model 0, which comes from behind the player, rolling, and hurts it
 * as it passes. */
s32 func_800F0BE0(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -25;
                o->x = x;
                o->state = 1;
                break;
            case 1:
                o->timer++;
                o->x += 60;
                GsInitCoordinate2(WORLD, &sPieceCoords[10]);
                sPieceCoords[10].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[10].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[10].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[10].flg = 0;
                sPieceRots[10].vx = 0;
                sPieceRots[10].vy = 0;
                sPieceRots[10].vz = o->timer * 40960 / 360;
                SetCoordRotation(&sPieceRots[10], &sPieceCoords[10]);
                GsGetLs(&sPieceCoords[10], &m);
                GsSetLsMatrix(&m);
                GsGetLw(&sPieceCoords[10], &m);
                GsSetLightMatrix(&m);
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 140 < gPlayer.z &&
                    gPlayer.z < o->z + 140 && o->y - 25 < gPlayer.y && gPlayer.y < o->y + 205) {
                    PlaySe(0x38);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2 && sHurtTimer == 0) {
                        sHurtTimer = 60;
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        gPlayer.slideState = 2;
                        gPlayer.unk38C = 0;
                        gPlayer.curMotion[0] = MOT_OFF;
                    }
                }
                if (o->x >= gPlayer.x + 500) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same, bouncing as it rolls. */
s32 func_800F0EA8(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -25;
                o->x = x;
                o->state = 1;
                break;
            case 1:
                o->timer++;
                o->x += 60;
                o->y = -(ABS(rcos(o->timer * 20480 / 360)) * 300 / 4096) - 25;
                GsInitCoordinate2(WORLD, &sPieceCoords[10]);
                sPieceCoords[10].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[10].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[10].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[10].flg = 0;
                sPieceRots[10].vx = 0;
                sPieceRots[10].vy = 0;
                sPieceRots[10].vz = o->timer * 40960 / 360;
                SetCoordRotation(&sPieceRots[10], &sPieceCoords[10]);
                GsGetLs(&sPieceCoords[10], &m);
                GsSetLsMatrix(&m);
                GsGetLw(&sPieceCoords[10], &m);
                GsSetLightMatrix(&m);
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 140 < gPlayer.z &&
                    gPlayer.z < o->z + 140 && o->y - 25 < gPlayer.y && gPlayer.y < o->y + 205) {
                    PlaySe(0x38);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2 && sHurtTimer == 0) {
                        sHurtTimer = 60;
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        gPlayer.slideState = 2;
                        gPlayer.unk38C = 0;
                        gPlayer.curMotion[0] = MOT_OFF;
                    }
                }
                if (o->x >= gPlayer.x + 500) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* Draws the four-part model at (@p x, @p y, @p z), its parts swinging with
 * the frame count offset by @p n, and throws up dust under it. */
void func_800F1248(s32 x, s32 y, s32 z, s32 n) {
    MATRIX m;
    SVECTOR rot;
    SVECTOR pos;
    s32 t;
    s32 a;
    s32 b;

    pos.vx = x;
    pos.vy = y;
    pos.vz = z;
    t = gFrameCount + n * 10;
    SpawnEffect(4, pos.vx + rand() % 100 - 50, rand() % 20 - 20, pos.vz + rand() % 100 - 50, 0);
    rot.vx = 0;
    rot.vy = 0;
    sPieceCoords[11].coord.t[0] = pos.vx;
    sPieceCoords[11].coord.t[1] = pos.vy;
    sPieceCoords[11].coord.t[2] = pos.vz;
    a = -t * 122880 / 360;
    rot.vz = rsin(a) / 20;
    SetCoordRotation(&rot, &sPieceCoords[11]);
    GsGetLs(sPieceObjs[11].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[11].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[11], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
    sPieceCoords[12].coord.t[0] = 100;
    sPieceCoords[12].coord.t[1] = -100;
    sPieceCoords[12].coord.t[2] = 0;
    rot.vx = 0;
    rot.vy = 0;
    b = t * 122880 / 360;
    rot.vz = rsin(b) / 10;
    SetCoordRotation(&rot, &sPieceCoords[12]);
    GsGetLs(sPieceObjs[12].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[12].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[12], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
    sPieceCoords[13].coord.t[0] = 33;
    sPieceCoords[13].coord.t[1] = -75;
    sPieceCoords[13].coord.t[2] = 0;
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = rsin(a) / 8;
    SetCoordRotation(&rot, &sPieceCoords[13]);
    GsGetLs(sPieceObjs[13].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[13].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[13], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
    sPieceCoords[14].coord.t[0] = -80;
    sPieceCoords[14].coord.t[1] = -88;
    sPieceCoords[14].coord.t[2] = 0;
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = rsin(b) / 5;
    SetCoordRotation(&rot, &sPieceCoords[14]);
    GsGetLs(sPieceObjs[14].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[14].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[14], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}

/* The frame step's state. */
extern s32 D_800F580C;
extern s32 D_800F5810;
extern s32 D_800F5814;
extern POLY_F4 *D_800F5818;
extern DR_MODE *D_800F581C;
extern s32 D_800F5820; /* nonzero while the opening scene shows the walker */
extern MATRIX D_800F57EC;
extern s32 D_800F5258;
extern s32 (*D_800F5264[])(Act3Obj *o);
extern u8 D_800F526C[];
extern GsCOORDINATE2 sAct3Coords[];
extern GsDOBJ2 sAct3Objs[];
/* MATCHING: model's view of the sky colours, as bytes. */
extern u8 *gSkyColors;

/** @brief Stage 4-3's view of the can record, which also places the walker
 *         its opening scene shows. */
typedef struct {
    ChaseCan can; /**< the can, as every chase has it */
    s32 pad[5];   /**< not yet known */
    s32 walkX;    /**< the walker's x */
    s32 walkY;    /**< the walker's y */
    s32 walkZ;    /**< the walker's z */
} ChaseCanWalker;

#define sWalker (*(ChaseCanWalker *)&sCan)

/* MATCHING: views of routines their units declare only where defined. */
void PlayStageMusic(u8 idx);
void DrawRepeatingStage(s32 pos, s32 unused, s32 z, s32 range);
void StartWorldResults(void);
void UpdateWorldResults(void);
s32 IsResultsActive(void);
void UpdateEffects(s32 unused, s16 x, s16 y, s16 z);
/* MATCHING: this unit passes it words. */
s32 SpawnEffectWithParams(s32 kind, s32 x, s32 y, s32 z, s32 delay, s32 param0, s32 param1,
                          s32 param2, s32 param3);

/* MATCHING: the step's sub-step, kept in the fog fade's word. */
#define sSubStep gFogFade

#ifdef NON_MATCHING
/* MATCHING: tested `>= 0`, cc1 folds it to an abs that keeps the minuend
 * computed first. */
#define ABS_GE(x) ((x) >= 0 ? (x) : -(x))

/* Steps the player's first animation channel, with a footstep while it
 * runs. */
static inline void StepChaseMotion(PlayerState *p) {
    u8 m;

    p->motionResult[0] = StepAnimation(p, 0, 0);
    /* MATCHING: the barrier keeps the result's store above the reads. */
    __asm__("");
    m = p->motion[0];
    if (!(gFrameCount & 7) && m == 0x33) {
        PlaySe(0x13);
    }
    switch (p->action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (p->motionResult[0] == 0) {
                p->curMotion[0] = MOT_OFF;
            }
            break;
    }
}

/* The same, after setting the scene's lights. */
static inline void StepChaseMotionLit(PlayerState *p) {
    PlayerState *q;

    SetChaseLights();
    /* MATCHING: a copy past the call puts the address in one saved register. */
    q = p;
    StepChaseMotion(q);
}

/* The chase's own step: the player's, the lights, the run forward and the
 * world shifted to follow it, on the scratchpad stack. */
static inline void StepChaseRun(PlayerState *q) {
    PlayerState *p;

    StepChasePlayer();
    p = q;
    SetChaseLights();
    gWorldShift[1] = 0;
    p->x += p->speed;
    gWorldShift[0] = -p->x;
    gWorldShift[2] = -p->z;
    SetSpadStack();
    StepChaseMotion(p);
}

/* Draws the can three times along the run, each rolling and bouncing at
 * its own phase. */
#define DRAW_CANS()                                                                       \
    do {                                                                                  \
        s32 t;                                                                            \
        s32 t2;                                                                           \
        s32 t3;                                                                           \
                                                                                          \
        sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];                              \
        t = rsin(gFrameCount * 40960 / 360) * 100;                                        \
        sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);           \
        sCanRot[0].vx = 0x800;                                                            \
        sCanRot[0].vy = 0;                                                                \
        sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];                              \
        sCanRot[0].vz = -(gFrameCount * 61440 / 360);                                     \
        if (gPlayer.x + 500 > sCan.x + 120) {                                             \
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                               \
            GsGetLs(sAct3Objs[0].coord2, &D_800F57EC);                                    \
            GsSetLsMatrix(&D_800F57EC);                                                   \
            GsGetLw(sAct3Objs[0].coord2, &D_800F57EC);                                    \
            GsSetLightMatrix(&D_800F57EC);                                                \
            SetTmdSortMode(4);                                                            \
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000); \
        }                                                                                 \
        sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0] - 100;                        \
        t2 = rsin(((gFrameCount * 10 + 45) << 12) / 360) * 100;                           \
        sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t2 / 4096);          \
        sCanRot[0].vx = 0x800;                                                            \
        sCanRot[0].vy = 0;                                                                \
        sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];                              \
        sCanRot[0].vz = -(gFrameCount * 40960 / 360);                                     \
        if (gPlayer.x + 500 > sCan.x + 120) {                                             \
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                               \
            GsGetLs(sAct3Objs[0].coord2, &D_800F57EC);                                    \
            GsSetLsMatrix(&D_800F57EC);                                                   \
            GsGetLw(sAct3Objs[0].coord2, &D_800F57EC);                                    \
            GsSetLightMatrix(&D_800F57EC);                                                \
            SetTmdSortMode(4);                                                            \
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000); \
        }                                                                                 \
        sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0] - 200;                        \
        t3 = rsin(((gFrameCount * 10 + 90) << 12) / 360) * 100;                           \
        sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t3 / 4096);          \
        sCanRot[0].vx = 0x800;                                                            \
        sCanRot[0].vy = 0;                                                                \
        sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];                              \
        sCanRot[0].vz = -(gFrameCount * 20480 / 360);                                     \
        if (gPlayer.x + 500 > sCan.x + 120) {                                             \
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                               \
            GsGetLs(sAct3Objs[0].coord2, &D_800F57EC);                                    \
            GsSetLsMatrix(&D_800F57EC);                                                   \
            GsGetLw(sAct3Objs[0].coord2, &D_800F57EC);                                    \
            GsSetLightMatrix(&D_800F57EC);                                                \
            SetTmdSortMode(4);                                                            \
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000); \
        }                                                                                 \
    } while (0)

/* Draws the still can at an offset from its place. */
#define DRAW_STILL_CAN(dx, dy)                                                        \
    do {                                                                              \
        sAct3Coords[0].coord.t[0] = sCan.x + (dx);                                    \
        sAct3Coords[0].coord.t[1] = sCan.y + (dy);                                    \
        sAct3Coords[0].coord.t[2] = sCan.z;                                           \
        SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                               \
        GsGetLs(sAct3Objs[0].coord2, &D_800F57EC);                                    \
        GsSetLsMatrix(&D_800F57EC);                                                   \
        GsGetLw(sAct3Objs[0].coord2, &D_800F57EC);                                    \
        GsSetLightMatrix(&D_800F57EC);                                                \
        SetTmdSortMode(4);                                                            \
        GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000); \
    } while (0)

/* The stage's frame step: the opening scene, the chase, the escape at the
 * run's end, and the results. */
/* MATCHING: non-void with no return keeps the tail's delay slots nops. */
s32 func_800F1654(void) {
    Act3Obj *o;
    s32 i;
    s32 v;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = D_800F526C;
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].super = NULL;
            }
            sPieceCoords[11].super = NULL;
            sPieceCoords[12].super = &sPieceCoords[11];
            sPieceCoords[13].super = &sPieceCoords[11];
            sPieceCoords[14].super = &sPieceCoords[11];
            gView[0] = -5000;
            gView[1] = -200;
            gView[2] = -1100;
            gView[3] = -5000;
            gView[4] = -140;
            gView[5] = -1500;
            GsSetRefView2((GsRVIEW2 *)gView);
            gPlayer.motion[0] = 0x13;
            gPlayer.x = -5000;
            gPlayer.y = 0;
            gPlayer.z = -1500;
            gPlayer.yaw = 0;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sCan.x = -0xDAC;
            sCan.y = 0;
            sCan.z = 0;
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = 0;
            sWalker.walkX = -8000;
            ClipF = 1;
            D_800F5814 = 0;
            sSubStep = 0;
            sHurtTimer = 0;
            D_800F5820 = 1;
            gMissState = 1;
            sWalker.walkY = 0;
            sWalker.walkZ = 0;
            gWorldIntroStep = 1;
            break;
        case 1:
            DrawFrozenFrame();
            switch (sSubStep) {
                case 0:
                    if (++D_800F5814 >= 30) {
                        D_800F5814 = 0;
                        sView = *(GsRVIEW2 *)gView;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    if (gFrameCount % 15 == 0) {
                        PlaySe(0x75);
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 100;
                            gRumbleSmall[1] = 100;
                        }
                    }
                    gView[0] = gView[3] = rand() % 20 - 10;
                    gView[1] = gView[4] = rand() % 20 - 10;
                    gView[0] -= 5000;
                    gView[3] -= 5000;
                    gView[1] -= 200;
                    gView[4] -= 140;
                    if (++D_800F5814 >= 30) {
                        sWalker.walkX = -8000;
                        sWalker.walkY = 0;
                        sWalker.walkZ = 0;
                        D_800F5814 = 0;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    if (gFrameCount % 15 == 0) {
                        PlaySe(0x75);
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 100;
                            gRumbleSmall[1] = 100;
                        }
                    }
                    sWalker.walkX += 30;
                    gView[0] = sWalker.walkX + 200;
                    gView[3] = sWalker.walkX;
                    gView[1] = gView[4] = rand() % 20 - 10;
                    gView[2] = gView[5] = rand() % 20 - 10;
                    gView[1] -= 200;
                    gView[4] -= 100;
                    if (++D_800F5814 >= 60) {
                        sWalker.walkX = -0x1964;
                        sWalker.walkY = 0;
                        sWalker.walkZ = -1500;
                        D_800F5814 = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (gFrameCount % 15 == 0) {
                        PlaySe(0x75);
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 100;
                            gRumbleSmall[1] = 100;
                        }
                    }
                    sWalker.walkX += 50;
                    gView[0] = gView[3] = rand() % 20 - 10;
                    gView[1] = gView[4] = rand() % 20 - 10;
                    gView[2] = -800;
                    gView[5] = -1500;
                    gView[0] -= 5000;
                    gView[3] -= 5000;
                    gView[1] -= 300;
                    gView[4] -= 140;
                    if (sWalker.walkX >= -5000) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        gPlayer.yaw = -0x400;
                        gPlayer.motion[0] = 0x37;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (gFrameCount % 15 == 0) {
                        PlaySe(0x75);
                    }
                    gView[0] = gView[3] = rand() % 20 - 10;
                    gView[1] = gView[4] = rand() % 20 - 10;
                    gView[2] = -800;
                    gView[5] = -1500;
                    gView[0] -= 5000;
                    gView[3] -= 5000;
                    D_800F5814++;
                    sWalker.walkX += 30;
                    gPlayer.x += 50;
                    gView[1] -= 300;
                    gView[4] -= 140;
                    gPlayer.y = -(rsin(D_800F5814 * 20480 / 360) * 400 / 4096);
                    if (D_800F5814 >= 20) {
                        gPlayer.yaw = -0x400;
                        gPlayer.motion[0] = 0x3C;
                        gPlayer.action = ACT_MODEL_ONCE;
                        gPlayer.x = -0xF8C;
                        D_800F5820 = 0;
                        D_800F5814 = 20;
                        gPlayer.z = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    gPlayer.y = -(rsin(++D_800F5814 * 20480 / 360) * 400 / 4096) - 150;
                    gView[0] = -0xAF0;
                    gView[1] = -200;
                    gView[2] = 0;
                    gView[4] = gPlayer.y;
                    gPlayer.x += 30;
                    gView[3] = gPlayer.x;
                    gView[5] = gPlayer.z;
                    if (D_800F5814 >= 36) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x38);
                        sView = *(GsRVIEW2 *)gView;
                        gPlayer.motion[0] = 0x39;
                        gPlayer.action = ACT_MODEL_ONCE;
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200, -150 - rand() % 100,
                                        gPlayer.z - (100 - rand() % 200), rand() % 4);
                        }
                        D_800F5814 = 0;
                        sSubStep = 30;
                    }
                    break;
                case 30: {
                    s32 t;

                    t = rsin(++D_800F5814 * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(t / 4096) - 150;
                }
                    gPlayer.x += 50;
                    if (D_800F5814 < 7) {
                        gView[3] = gRefView[3] + rand() % 100 - 50;
                        gView[4] = gRefView[4] + rand() % 100 - 50;
                        gView[5] = gRefView[5] + rand() % 100 - 50;
                        gView[0] = gRefView[0] - rand() % 200;
                    } else {
                        *(GsRVIEW2 *)gView = sView;
                    }
                    if (D_800F5814 >= 23) {
                        gPlayer.motion[0] = 0x3D;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSubStep = 35;
                    }
                    break;
                case 35:
                    v = rsin(++D_800F5814 * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(v / 4096);
                    gView[0] = -0x4B0;
                    gView[1] = -300;
                    gView[2] = 0;
                    gView[4] = gPlayer.y;
                    gPlayer.x += 50;
                    gView[3] = gPlayer.x;
                    gView[5] = gPlayer.z;
                    if (D_800F5814 >= 36) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 100;
                            gRumbleSmall[1] = 100;
                        }
                        PlaySe(0x1C);
                        gPlayer.motion[0] = 0x3A;
                        gPlayer.action = ACT_MODEL_ONCE;
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200 - 100, -(rand() % 50),
                                        gPlayer.z - (100 - rand() % 200), rand() % 4);
                        }
                        D_800F5814 = 0;
                        sSubStep = 40;
                    }
                    break;
                case 40:
                    if (++D_800F5814 >= 30) {
                        gPlayer.motion[0] = 0x3E;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800F5814 = 0;
                        sSubStep = 45;
                    }
                    break;
                case 45:
                    if (++D_800F5814 >= 72) {
                        gPlayer.motion[0] = 0x3B;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800F5814 = 0;
                        sSubStep = 50;
                    }
                    break;
                case 50:
                    if (++D_800F5814 >= 30) {
                        gView[0] = -0x4B0 - (D_800F5814 - 30) * 12;
                        gView[1] = (D_800F5814 - 30) * 5 - 300;
                        gView[2] = 0;
                        gView[3] = gPlayer.x;
                        gView[4] = gPlayer.y - (D_800F5814 - 30) * 5;
                        gView[5] = gPlayer.z;
                    }
                    if (D_800F5814 >= 60) {
                        D_800F5814 = 0;
                        sSubStep = 55;
                    }
                    break;
                case 55:
                    if (gFrameCount & 3) {
                        D_800F5814++;
                    }
                    *(GsRVIEW2 *)gView = sView;
                    sCan.x += D_800F5814;
                    sCanRot[0].vz -= (D_800F5814 / 3 << 12) / 360;
                    if (D_800F5814 >= 30) {
                        gWorldIntroStep = 10;
                    }
                    break;
            }
            SetSpadStack();
            GsSetRefView2((GsRVIEW2 *)gView);
            StepChaseMotionLit(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], 0, 0, 0);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayer();
            DrawChaseShadow();
            if (D_800F5820 != 0) {
                func_800F1248(sWalker.walkX - 100, sWalker.walkY, sWalker.walkZ, 0);
                func_800F1248(sWalker.walkX - 200, sWalker.walkY, sWalker.walkZ - 150, 1);
                func_800F1248(sWalker.walkX - 300, sWalker.walkY, sWalker.walkZ + 150, 2);
            } else {
                DRAW_STILL_CAN(0, 0);
                DRAW_STILL_CAN(-100, -100);
                DRAW_STILL_CAN(-200, 0);
            }
            DrawRepeatingStage(0, 0, 0, 40000);
            DrawSky(0);
            ResetSpadStack();
            if (gPadTrig & PADstart) {
                gWorldIntroStep = 10;
            }
            break;
        case 10:
            PlayStageMusic(gStage);
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].super = NULL;
            }
            PlaceChaseObjects();
            gView[0] = 700;
            gView[1] = -200;
            gView[2] = 0;
            gView[3] = -500;
            gView[4] = 0;
            gView[5] = 0;
            GsSetRefView2((GsRVIEW2 *)gView);
            gPlayer.x = -1800;
            gPlayer.yaw = -0x400;
            gPlayer.speed = 30;
            gPlayer.motion[0] = 0x33;
            gPlayer.action = ACT_MODEL_LOOP;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.control = 1;
            gPlayer.slideState = 0;
            gPlayer.unk38C = 0;
            gPlayer.curMotion[0] = MOT_OFF;
            sChaseLive = 0x1000;
            sCan.x = -0x974;
            sCan.y = -140;
            sCan.z = 0;
            sCanRot[0].vx = 0;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = 0;
            ClipF = 0;
            D_800F580C = 0;
            D_800F5810 = 0;
            sHurtTimer = 0;
            gMissState = 0;
            gWorldIntroStep = 20;
            break;
        case 20:
            if (ClipF == 0) {
                DrawChaseHud();
            }
            if (D_800F5810 == 0) {
                gView[0] = 700;
                gView[1] = -200;
                gView[2] = 0;
                gView[3] = -500;
                gView[4] = 0;
                gView[5] = 0;
                /* MATCHING: one call per arm; cc1 cross-jumps them. */
                GsSetRefView2((GsRVIEW2 *)gView);
            } else {
                gView[0] = rcos((D_800F5810 << 12) / 360) * (700 - D_800F5810 * 3) / 4096;
                gView[1] = -200;
                gView[2] = rsin((D_800F5810 << 12) / 360) * (700 - D_800F5810 * 3) / 4096;
                gView[3] = -(rcos((D_800F5810 << 12) / 360) * 500 / 4096);
                gView[4] = 0;
                gView[5] = -(rsin((D_800F5810 << 12) / 360) * 500 / 4096);
                GsSetRefView2((GsRVIEW2 *)gView);
            }
            StepChaseRun(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(sChaseLive);
            DrawChaseShadowCentre();
            sCan.x += 30;
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            DRAW_CANS();
            i = 0;
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 40000);
            /* MATCHING: i cleared before the call, the base taken before the loop. */
            o = sObjs;
            for (; i < D_800F5258; i++) {
                if (o[i].state != -1) {
                    D_800F5264[(s16)o[i].kind](&o[i]);
                }
            }
            DrawSky(0);
            if (gPlayer.x < sCan.x + 120 && (s8)gPlayer.slideState != 3) {
                gMissState = 1;
                sHurtTimer = 0;
                if (gRumbleEnabled) {
                    gRumbleTime = 30;
                    gRumbleSmall[0] = 100;
                    gRumbleSmall[1] = 100;
                }
                gPlayer.curMotion[0] = MOT_OFF;
                gPlayer.slideState = 3;
                gPlayer.unk38C = 0;
                D_800F580C = 1;
            }
            if (D_800F580C != 0) {
                if (++D_800F580C == 90) {
                    if (gRetries == 0) {
                        gGameMode = 6;
                        gModeStep = 0xFFFF;
                    } else {
                        gRetries--;
                        StopCdTrack();
                        gWorldIntroStep = 10;
                    }
                }
            }
            if (gPlayer.x > 40000 && D_800F5810 == 0) {
                gMissState = 1;
                ClipF = 1;
                D_800F5810 = 1;
            }
            if (D_800F5810 != 0) {
                D_800F5810++;
                if (gPlayer.z > 15) {
                    gPlayer.z -= 15;
                } else if (gPlayer.z < -15) {
                    gPlayer.z += 15;
                } else {
                    gPlayer.z = 0;
                }
                if (D_800F5810 > 60) {
                    D_800F5814 = 0;
                    gWorldIntroStep = 50;
                }
            }
            ResetSpadStack();
            break;
        case 50:
            if (D_800F5814 > 160) {
                D_800F5818 = (POLY_F4 *)GsOUT_PACKET_P;
                setPolyF4(D_800F5818);
                setSemiTrans(D_800F5818, 1);
                D_800F5818->x0 = D_800F5818->x2 = -160;
                D_800F5818->y0 = D_800F5818->y1 = -120;
                D_800F5818->x1 = D_800F5818->x3 = 160;
                D_800F5818->y2 = D_800F5818->y3 = 120;
                D_800F5818->r0 = D_800F5818->g0 = D_800F5818->b0 = (D_800F5814 - 160) * 4;
                addPrim(gSubOt[gActiveBuff].org, D_800F5818);
                D_800F5818++;
                GsOUT_PACKET_P = (PACKET *)D_800F5818;
                D_800F581C = (DR_MODE *)D_800F5818;
                setlen(D_800F581C, 1);
                D_800F581C->code[0] = 0xE1000640;
                addPrim(gSubOt[gActiveBuff].org, D_800F581C);
                D_800F581C = (DR_MODE *)((u8 *)D_800F581C + 8);
                GsOUT_PACKET_P = (PACKET *)D_800F581C;
            }
            if (D_800F5814 < 120) {
                gPlayer.x -= rsin(gPlayer.yaw) * 30 / 4096;
                gPlayer.z -= rcos(gPlayer.yaw) * 30 / 4096;
            }
            gWorldShift[1] = 0;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            if (D_800F5814 == 120) {
                for (gLoopI = 0; gLoopI < 20; gLoopI++) {
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0xADD4, 0, 0, gLoopI * 2, 0,
                                          -(rand() % 30) - 10, rand() % 300 + 100, rand() % 20 - 10);
                }
            }
            if (D_800F5814 == 90) {
                gRefView[0] = gView[0] + gPlayer.x;
                gRefView[1] = gView[1] + gPlayer.y;
                gRefView[2] = gView[2] + gPlayer.z;
                gRefView[3] = gView[3] - gView[0];
                gRefView[4] = gView[4] - gView[1];
                gRefView[5] = gView[5] - gView[2];
            }
            /* MATCHING: nested, so cc1 does not merge the two bounds. */
            if (D_800F5814 < 150) {
                if (D_800F5814 >= 120) {
                    if (D_800F5814 == 120) {
                        PlaySe(0x3E);
                        if (gRumbleEnabled) {
                            gRumbleTime = 30;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                    }
                    gView[0] = gRefView[0] - gPlayer.x + rand() % 20 - 10;
                    gView[1] = gRefView[1] - gPlayer.y + rand() % 20 - 10;
                    gView[2] = gRefView[2] - gPlayer.z + rand() % 20 - 10;
                    gView[3] = gRefView[3] + gView[0] + rand() % 20 - 10;
                    gView[4] = gRefView[4] + gView[1] + rand() % 20 - 10;
                    gView[5] = gRefView[5] + gView[2] + rand() % 20 - 10;
                } else if (D_800F5814 >= 90) {
                    gView[0] = gRefView[0] - gPlayer.x;
                    gView[1] = gRefView[1] - gPlayer.y;
                    gView[2] = gRefView[2] - gPlayer.z;
                    gView[3] = gRefView[3] + gView[0];
                    gView[4] = gRefView[4] + gView[1];
                    gView[5] = gRefView[5] + gView[2];
                }
            } else {
                gView[0] = gRefView[0] - gPlayer.x;
                gView[1] = gRefView[1] - gPlayer.y;
                gView[2] = gRefView[2] - gPlayer.z;
                gView[3] = gRefView[3] + gView[0];
                gView[4] = gRefView[4] + gView[1];
                gView[5] = gRefView[5] + gView[2];
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            if (D_800F5814 < 52) {
                gPlayer.yaw -= 11;
            }
            D_800F5814++;
            /* MATCHING: here the address is taken after the call, not copied. */
            SetChaseLights();
            StepChaseMotion(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(0x1000);
            DrawChaseShadowCentre();
            if (D_800F5814 < 150) {
                sCan.x += 30;
                SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            }
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            DRAW_CANS();
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 40000);
            DrawSky(0);
            if (D_800F5814 >= 223) {
                ClipF = 0;
                StopCdTrack();
                switch (gStageTableSet) {
                    case 0:
                    case 1:
                    case 2:
                        StartWorldResults();
                        gWorldIntroStep = 60;
                        break;
                    case 3:
                        gGameMode = 6;
                        gModeStep = 0;
                        break;
                }
            }
            break;
        case 60:
            UpdateWorldResults();
            if (IsResultsActive() == 0) {
                switch (gStageTableSet) {
                    case 0:
                    case 1:
                        gStage = STAGE_5_1;
                        gGameMode = 0x2C;
                        break;
                    case 2:
                    case 3:
                        gGameMode = 6;
                        gModeStep = 0;
                        break;
                }
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", func_800F1654);
#endif

#include "chase_tail.inc"
