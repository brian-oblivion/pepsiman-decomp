/* Stage 4-3's overlay: the act-3 chase of world 4, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 10240000

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
        gPlayer.slideStateTime = 0;
        gPlayer.curMotion[0] = MOT_OFF;
        PlaySe(SE_JUMP);
    }
}

#include "chase_head.inc"

/* Piece model 0, which comes from behind the player, rolling, and hurts it
 * as it passes. */
s32 StepChaseRollingLog(Act3Obj *o) {
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
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 140 < gPlayer.z &&
                    gPlayer.z < o->z + 140 && o->y - 25 < gPlayer.y && gPlayer.y < o->y + 205) {
                    PlaySe(SE_CRASH);
                    gPlayer.action = ACT_TRIP_HARD_START;
                    EndJump();
                    if (gPlayer.slideState < 2 && sHurtTimer == 0) {
                        sHurtTimer = 60;
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        gPlayer.slideState = 2;
                        gPlayer.slideStateTime = 0;
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
s32 StepChaseBouncingLog(Act3Obj *o) {
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
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 140 < gPlayer.z &&
                    gPlayer.z < o->z + 140 && o->y - 25 < gPlayer.y && gPlayer.y < o->y + 205) {
                    PlaySe(SE_CRASH);
                    gPlayer.action = ACT_TRIP_HARD_START;
                    EndJump();
                    if (gPlayer.slideState < 2 && sHurtTimer == 0) {
                        sHurtTimer = 60;
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        gPlayer.slideState = 2;
                        gPlayer.slideStateTime = 0;
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
void DrawWalker(s32 x, s32 y, s32 z, s32 n) {
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
    GsSortObject4(&sPieceObjs[11], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
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
    GsSortObject4(&sPieceObjs[12], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
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
    GsSortObject4(&sPieceObjs[13], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
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
    GsSortObject4(&sPieceObjs[14], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
}

/* The frame step's state. */
extern s32 sWalkerShown; /* nonzero while the opening scene shows the walker */

/* The stage's own models. */
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

/* MATCHING: tested `>= 0`, cc1 folds it to an abs that keeps the minuend
 * computed first. */
#define ABS_GE(x) ((x) >= 0 ? (x) : -(x))

void SetChaseLights(void);
void PlaceChaseObjects(void);

/* Steps the player's first animation channel, with a footstep while it
 * runs. */
static inline void StepChaseMotion(PlayerState *q) {
    PlayerState *p;
    u8 m;

    q->motionResult[0] = StepAnimation(q, 0, 0);
    /* MATCHING: copied past the call, the address and its %hi share one
     * saved register. */
    p = q;
    /* MATCHING: the barrier keeps the result's store above the reads. */
    __asm__("");
    m = p->motion[0];
    if (!(gFrameCount & 7) && m == 0x33) {
        PlaySe(SE_STEP);
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

/* The chase's own step: the player's, the lights, the run forward and the
 * world shifted to follow it, on the scratchpad stack. */
static inline void StepChaseRun(void) {
    PlayerState *p;

    StepChasePlayer();
    /* MATCHING: taken after the call, so reorg can hoist the next arm's lui. */
    p = &gPlayer;
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
/* MATCHING: a plain block; a do-while(0) adds a loop level to the refs and
 * reorders the saved registers. */
#define DRAW_CANS()                                                                    \
    {                                                                                  \
        s32 t;                                                                         \
        s32 t2;                                                                        \
        s32 t3;                                                                        \
                                                                                       \
        sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];                           \
        t = rsin(gFrameCount * 40960 / 360) * 100;                                     \
        sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);        \
        sCanRot[0].vx = ANGLE_DEG(180);                                                \
        sCanRot[0].vy = 0;                                                             \
        sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];                           \
        sCanRot[0].vz = -(gFrameCount * 61440 / 360);                                  \
        if (gPlayer.x + 500 > sCan.x + 120) {                                          \
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                            \
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);                               \
            GsSetLsMatrix(&sChaseMatrix);                                              \
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);                               \
            GsSetLightMatrix(&sChaseMatrix);                                           \
            SetTmdSortMode(4);                                                         \
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0)); \
        }                                                                              \
        sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0] - 100;                     \
        t2 = rsin(((gFrameCount * 10 + 45) << 12) / 360) * 100;                        \
        sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t2 / 4096);       \
        sCanRot[0].vx = ANGLE_DEG(180);                                                \
        sCanRot[0].vy = 0;                                                             \
        sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];                           \
        sCanRot[0].vz = -(gFrameCount * 40960 / 360);                                  \
        if (gPlayer.x + 500 > sCan.x + 120) {                                          \
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                            \
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);                               \
            GsSetLsMatrix(&sChaseMatrix);                                              \
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);                               \
            GsSetLightMatrix(&sChaseMatrix);                                           \
            SetTmdSortMode(4);                                                         \
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0)); \
        }                                                                              \
        sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0] - 200;                     \
        t3 = rsin(((gFrameCount * 10 + 90) << 12) / 360) * 100;                        \
        sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t3 / 4096);       \
        sCanRot[0].vx = ANGLE_DEG(180);                                                \
        sCanRot[0].vy = 0;                                                             \
        sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];                           \
        sCanRot[0].vz = -(gFrameCount * 20480 / 360);                                  \
        if (gPlayer.x + 500 > sCan.x + 120) {                                          \
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);                            \
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);                               \
            GsSetLsMatrix(&sChaseMatrix);                                              \
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);                               \
            GsSetLightMatrix(&sChaseMatrix);                                           \
            SetTmdSortMode(4);                                                         \
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0)); \
        }                                                                              \
    }

/* Draws the still can at an offset from its place. */
static inline void DrawStillCan(SVECTOR *rot, GsCOORDINATE2 *coord, s32 dx, s32 dy) {
    coord->coord.t[0] = sCan.x + dx;
    coord->coord.t[1] = sCan.y + dy;
    coord->coord.t[2] = sCan.z;
    SetCoordRotation(rot, coord);
    GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
    GsSetLsMatrix(&sChaseMatrix);
    GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
    GsSetLightMatrix(&sChaseMatrix);
    SetTmdSortMode(4);
    GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
}

/* MATCHING: a view of a routine model.c declares only where defined. */
void DrawSky(u32 arg);

/* The stage's frame step: the opening scene, the chase, the escape at the
 * run's end, and the results. */
/* MATCHING: non-void with no return keeps the tail's delay slots nops. */
s32 StepChaseFrame(void) {
    Act3Obj *o;
    s32 i;
    s32 v;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = (u8 *)sChaseSky;
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
            gPlayer.motion[0] = MOT_STAND;
            gPlayer.x = -5000;
            gPlayer.y = 0;
            gPlayer.z = -1500;
            gPlayer.yaw = 0;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sCan.x = -0xDAC;
            sCan.y = 0;
            sCan.z = 0;
            sCanRot[0].vx = ANGLE_DEG(180);
            sCanRot[0].vy = 0;
            sCanRot[0].vz = 0;
            sWalker.walkX = -8000;
            ClipF = 1;
            sSceneTimer = 0;
            sSubStep = 0;
            sHurtTimer = 0;
            sWalkerShown = 1;
            gMissState = 1;
            sWalker.walkY = 0;
            sWalker.walkZ = 0;
            gWorldIntroStep = 1;
            break;
        case 1:
            DrawFrozenFrame();
            switch (sSubStep) {
                case 0:
                    if (++sSceneTimer >= 30) {
                        sSceneTimer = 0;
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
                    if (++sSceneTimer >= 30) {
                        sWalker.walkX = -8000;
                        sWalker.walkY = 0;
                        sWalker.walkZ = 0;
                        sSceneTimer = 0;
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
                    if (++sSceneTimer >= 60) {
                        sWalker.walkX = -0x1964;
                        sWalker.walkY = 0;
                        sWalker.walkZ = -1500;
                        sSceneTimer = 0;
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
                        gPlayer.yaw = -ANGLE_DEG(90);
                        gPlayer.motion[0] = MOT_RIDE_LEAN_B_OUT;
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
                    sSceneTimer++;
                    sWalker.walkX += 30;
                    gPlayer.x += 50;
                    gView[1] -= 300;
                    gView[4] -= 140;
                    gPlayer.y = -(rsin(sSceneTimer * 20480 / 360) * 400 / 4096);
                    if (sSceneTimer >= 20) {
                        gPlayer.yaw = -ANGLE_DEG(90);
                        gPlayer.motion[0] = 0x3C;
                        gPlayer.action = ACT_MODEL_ONCE;
                        gPlayer.x = -0xF8C;
                        sWalkerShown = 0;
                        sSceneTimer = 20;
                        gPlayer.z = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    gPlayer.y = -(rsin(++sSceneTimer * 20480 / 360) * 400 / 4096) - 150;
                    gView[0] = -0xAF0;
                    gView[1] = -200;
                    gView[2] = 0;
                    gView[4] = gPlayer.y;
                    gPlayer.x += 30;
                    gView[3] = gPlayer.x;
                    gView[5] = gPlayer.z;
                    if (sSceneTimer >= 36) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(SE_CRASH);
                        sView = *(GsRVIEW2 *)gView;
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.action = ACT_MODEL_ONCE;
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200, -150 - rand() % 100,
                                        gPlayer.z - (100 - rand() % 200), rand() % 4);
                        }
                        sSceneTimer = 0;
                        sSubStep = 30;
                    }
                    break;
                case 30: {
                    s32 t;

                    t = rsin(++sSceneTimer * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(t / 4096) - 150;
                }
                    gPlayer.x += 50;
                    if (sSceneTimer < 7) {
                        gView[3] = gRefView[3] + rand() % 100 - 50;
                        gView[4] = gRefView[4] + rand() % 100 - 50;
                        gView[5] = gRefView[5] + rand() % 100 - 50;
                        gView[0] = gRefView[0] - rand() % 200;
                    } else {
                        *(GsRVIEW2 *)gView = sView;
                    }
                    if (sSceneTimer >= 23) {
                        gPlayer.motion[0] = 0x3D;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSubStep = 35;
                    }
                    break;
                case 35:
                    v = rsin(++sSceneTimer * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(v / 4096);
                    gView[0] = -0x4B0;
                    gView[1] = -300;
                    gView[2] = 0;
                    gView[4] = gPlayer.y;
                    gPlayer.x += 50;
                    gView[3] = gPlayer.x;
                    gView[5] = gPlayer.z;
                    if (sSceneTimer >= 36) {
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
                        sSceneTimer = 0;
                        sSubStep = 40;
                    }
                    break;
                case 40:
                    if (++sSceneTimer >= 30) {
                        gPlayer.motion[0] = 0x3E;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSceneTimer = 0;
                        sSubStep = 45;
                    }
                    break;
                case 45:
                    if (++sSceneTimer >= 72) {
                        gPlayer.motion[0] = 0x3B;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSceneTimer = 0;
                        sSubStep = 50;
                    }
                    break;
                case 50:
                    if (++sSceneTimer >= 30) {
                        gView[0] = -0x4B0 - (sSceneTimer - 30) * 12;
                        gView[1] = (sSceneTimer - 30) * 5 - 300;
                        gView[2] = 0;
                        gView[3] = gPlayer.x;
                        gView[4] = gPlayer.y - (sSceneTimer - 30) * 5;
                        gView[5] = gPlayer.z;
                    }
                    if (sSceneTimer >= 60) {
                        sSceneTimer = 0;
                        sSubStep = 55;
                    }
                    break;
                case 55:
                    if (gFrameCount & 3) {
                        sSceneTimer++;
                    }
                    *(GsRVIEW2 *)gView = sView;
                    sCan.x += sSceneTimer;
                    sCanRot[0].vz -= (sSceneTimer / 3 << 12) / 360;
                    if (sSceneTimer >= 30) {
                        gWorldIntroStep = 10;
                    }
                    break;
            }
            SetSpadStack();
            GsSetRefView2((GsRVIEW2 *)gView);
            SetChaseLights();
            StepChaseMotion(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], 0, 0, 0);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayer();
            DrawChaseShadow();
            if (sWalkerShown != 0) {
                DrawWalker(sWalker.walkX - 100, sWalker.walkY, sWalker.walkZ, 0);
                DrawWalker(sWalker.walkX - 200, sWalker.walkY, sWalker.walkZ - 150, 1);
                DrawWalker(sWalker.walkX - 300, sWalker.walkY, sWalker.walkZ + 150, 2);
            } else {
                DrawStillCan(&sCanRot[0], &sAct3Coords[0], 0, 0);
                DrawStillCan(&sCanRot[0], &sAct3Coords[0], -100, -100);
                DrawStillCan(&sCanRot[0], &sAct3Coords[0], -200, 0);
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
            gPlayer.yaw = -ANGLE_DEG(90);
            gPlayer.speed = 30;
            gPlayer.motion[0] = MOT_RIDE;
            gPlayer.action = ACT_MODEL_LOOP;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.control = 1;
            gPlayer.slideState = 0;
            gPlayer.slideStateTime = 0;
            gPlayer.curMotion[0] = MOT_OFF;
            sChaseLive = 0x1000;
            sCan.x = -0x974;
            sCan.y = -140;
            sCan.z = 0;
            sCanRot[0].vx = 0;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = 0;
            ClipF = 0;
            sCaughtTimer = 0;
            sEscapeTimer = 0;
            sHurtTimer = 0;
            gMissState = 0;
            gWorldIntroStep = 20;
            break;
        case 20:
            if (ClipF == 0) {
                DrawChaseHud();
            }
            if (sEscapeTimer == 0) {
                gView[0] = 700;
                gView[1] = -200;
                gView[2] = 0;
                gView[3] = -500;
                gView[4] = 0;
                gView[5] = 0;
                /* MATCHING: one call per arm; cc1 cross-jumps them. */
                GsSetRefView2((GsRVIEW2 *)gView);
            } else {
                gView[0] = rcos((sEscapeTimer << 12) / 360) * (700 - sEscapeTimer * 3) / 4096;
                gView[1] = -200;
                gView[2] = rsin((sEscapeTimer << 12) / 360) * (700 - sEscapeTimer * 3) / 4096;
                gView[3] = -(rcos((sEscapeTimer << 12) / 360) * 500 / 4096);
                gView[4] = 0;
                gView[5] = -(rsin((sEscapeTimer << 12) / 360) * 500 / 4096);
                GsSetRefView2((GsRVIEW2 *)gView);
            }
            StepChaseRun();
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
            for (; i < sObjCount; i++) {
                if (o[i].state != -1) {
                    sPieceSteps[(s16)o[i].kind](&o[i]);
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
                gPlayer.slideStateTime = 0;
                sCaughtTimer = 1;
            }
            if (sCaughtTimer != 0) {
                if (++sCaughtTimer == 90) {
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
            if (gPlayer.x > 40000 && sEscapeTimer == 0) {
                gMissState = 1;
                ClipF = 1;
                sEscapeTimer = 1;
            }
            if (sEscapeTimer != 0) {
                sEscapeTimer++;
                if (gPlayer.z > 15) {
                    gPlayer.z -= 15;
                } else if (gPlayer.z < -15) {
                    gPlayer.z += 15;
                } else {
                    gPlayer.z = 0;
                }
                if (sEscapeTimer > 60) {
                    sSceneTimer = 0;
                    gWorldIntroStep = 50;
                }
            }
            ResetSpadStack();
            break;
        case 50:
            if (sSceneTimer > 160) {
                sFadePoly = (POLY_F4 *)GsOUT_PACKET_P;
                setPolyF4(sFadePoly);
                setSemiTrans(sFadePoly, 1);
                sFadePoly->x0 = sFadePoly->x2 = -160;
                sFadePoly->y0 = sFadePoly->y1 = -120;
                sFadePoly->x1 = sFadePoly->x3 = 160;
                sFadePoly->y2 = sFadePoly->y3 = 120;
                sFadePoly->r0 = sFadePoly->g0 = sFadePoly->b0 = (sSceneTimer - 160) * 4;
                addPrim(gSubOt[gActiveBuff].org, sFadePoly);
                sFadePoly++;
                GsOUT_PACKET_P = (PACKET *)sFadePoly;
                sFadeMode = (DR_MODE *)sFadePoly;
                setlen(sFadeMode, 1);
                sFadeMode->code[0] = 0xE1000640;
                addPrim(gSubOt[gActiveBuff].org, sFadeMode);
                sFadeMode = (DR_MODE *)&sFadeMode->code[1];
                GsOUT_PACKET_P = (PACKET *)sFadeMode;
            }
            if (sSceneTimer < 120) {
                gPlayer.x -= rsin(gPlayer.yaw) * 30 / 4096;
                gPlayer.z -= rcos(gPlayer.yaw) * 30 / 4096;
            }
            gWorldShift[1] = 0;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            if (sSceneTimer == 120) {
                for (gLoopI = 0; gLoopI < 20; gLoopI++) {
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0xADD4, 0, 0, gLoopI * 2, 0,
                                          -(rand() % 30) - 10, rand() % 300 + 100, rand() % 20 - 10);
                }
            }
            if (sSceneTimer == 90) {
                gRefView[0] = gView[0] + gPlayer.x;
                gRefView[1] = gView[1] + gPlayer.y;
                gRefView[2] = gView[2] + gPlayer.z;
                gRefView[3] = gView[3] - gView[0];
                gRefView[4] = gView[4] - gView[1];
                gRefView[5] = gView[5] - gView[2];
            }
            /* MATCHING: nested, so cc1 does not merge the two bounds. */
            if (sSceneTimer < 150) {
                if (sSceneTimer >= 120) {
                    if (sSceneTimer == 120) {
                        PlaySe(SE_EXPLOSION);
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
                } else if (sSceneTimer >= 90) {
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
            if (sSceneTimer < 52) {
                gPlayer.yaw -= 11;
            }
            sSceneTimer++;
            SetChaseLights();
            StepChaseMotion(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(0x1000);
            DrawChaseShadowCentre();
            if (sSceneTimer < 150) {
                sCan.x += 30;
                SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            }
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            DRAW_CANS();
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 40000);
            DrawSky(0);
            if (sSceneTimer >= 223) {
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

/** @brief The stage's objects, in run order: what PlaceChaseObjects puts
 * in the object slots. */
Act3Place sPlacements[] = {
    /*   0 */ {0, {0, 0}, 0, 0, 122},
    /*   1 */ {0, {0, 0}, 1500, 0, -122},
    /*   2 */ {0, {0, 0}, 3000, 0, 122},
    /*   3 */ {0, {0, 0}, 4500, 0, -122},
    /*   4 */ {0, {0, 0}, 6000, 0, 0},
    /*   5 */ {0, {0, 0}, 7500, 0, 122},
    /*   6 */ {0, {0, 0}, 9000, 0, -122},
    /*   7 */ {0, {0, 0}, 11000, 0, 122},
    /*   8 */ {1, {0, 0}, 11000, 0, -122},
    /*   9 */ {0, {0, 0}, 12000, 0, -122},
    /*  10 */ {1, {0, 0}, 12000, 0, 122},
    /*  11 */ {0, {0, 0}, 13000, 0, 0},
    /*  12 */ {1, {0, 0}, 13000, 0, 0},
    /*  13 */ {0, {0, 0}, 14500, 0, 122},
    /*  14 */ {1, {0, 0}, 14500, 0, -122},
    /*  15 */ {0, {0, 0}, 16000, 0, -122},
    /*  16 */ {1, {0, 0}, 16000, 0, 122},
    /*  17 */ {0, {0, 0}, 17500, 0, 122},
    /*  18 */ {0, {0, 0}, 17500, 0, -122},
    /*  19 */ {0, {0, 0}, 19000, 0, 122},
    /*  20 */ {0, {0, 0}, 19000, 0, -122},
    /*  21 */ {0, {0, 0}, 20000, 0, 122},
    /*  22 */ {0, {0, 0}, 20000, 0, -122},
    /*  23 */ {1, {0, 0}, 21000, 0, 122},
    /*  24 */ {1, {0, 0}, 21000, 0, -122},
    /*  25 */ {1, {0, 0}, 22000, 0, -122},
    /*  26 */ {1, {0, 0}, 22500, 0, 122},
    /*  27 */ {0, {0, 0}, 23500, 0, -122},
    /*  28 */ {0, {0, 0}, 23500, 0, 122},
    /*  29 */ {1, {0, 0}, 23700, 0, 122},
    /*  30 */ {0, {0, 0}, 25000, 0, 122},
    /*  31 */ {1, {0, 0}, 25200, 0, 122},
    /*  32 */ {0, {0, 0}, 26500, 0, -122},
    /*  33 */ {1, {0, 0}, 26600, 0, -122},
    /*  34 */ {0, {0, 0}, 28000, 0, 122},
    /*  35 */ {1, {0, 0}, 28100, 0, -122},
    /*  36 */ {0, {0, 0}, 29000, 0, 0},
    /*  37 */ {0, {0, 0}, 30000, 0, 122},
    /*  38 */ {0, {0, 0}, 30000, 0, -122},
    /*  39 */ {0, {0, 0}, 31500, 0, 122},
    /*  40 */ {1, {0, 0}, 31700, 0, -122},
    /*  41 */ {1, {0, 0}, 31700, 0, 122},
    /*  42 */ {0, {0, 0}, 33000, 0, -122},
    /*  43 */ {1, {0, 0}, 33200, 0, 0},
    /*  44 */ {0, {0, 0}, 34000, 0, 122},
    /*  45 */ {1, {0, 0}, 34100, 0, 0},
    /*  46 */ {0, {0, 0}, 36000, 0, -122},
    /*  47 */ {0, {0, 0}, 36000, 0, 122},
    /*  48 */ {1, {0, 0}, 36000, 0, 122},
    /*  49 */ {0, {0, 0}, 37000, 0, -122},
    /*  50 */ {0, {0, 0}, 37000, 0, 122},
    /*  51 */ {0, {0, 0}, 37000, 0, -122},
    /*  52 */ {1, {0, 0}, 37000, 0, -122},
};

#define CHASE_PIECE_STEPS StepChaseRollingLog, StepChaseBouncingLog
#define CHASE_SKY \
    {0x6B, 0xBC, 0xEB, 0}, {0xFF, 0xFF, 0xFF, 0}, {0xFF, 0xFF, 0xFF, 0}, {0xFF, 0xFF, 0xFF, 0}

#include "chase_data.inc"

s32 sWalkerShown;
