/* Stage 5-3's overlay: the act-3 chase of world 5, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 12800000

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

/* A roller from behind the player, at 60 a frame. */
s32 StepChasePiece0(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -35;
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
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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
s32 StepChasePiece1(Act3Obj *o) {
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
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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

/* A roller from behind the player, at 40 a frame. */
s32 StepChasePiece2(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -35;
                o->x = x;
                o->state = 1;
                break;
            case 1:
                o->timer++;
                o->x += 40;
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
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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

/* A roller from behind the player, at 80 a frame. */
s32 StepChasePiece3(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -35;
                o->x = x;
                o->state = 1;
                break;
            case 1:
                o->timer++;
                o->x += 80;
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
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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

/* The stage's own models. */
extern GsCOORDINATE2 sAct3Coords[];
extern GsDOBJ2 sAct3Objs[];
/* MATCHING: model's view of the sky colours, as bytes. */
extern u8 *gSkyColors;

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

/* MATCHING: a view of a routine model.c declares only where defined. */
void DrawSky(u32 arg);

/* The stage's frame step: the opening scene, the chase, the escape at the
 * run's end, and the results. */
/* MATCHING: non-void with no return keeps the tail's delay slots nops. */
s32 StepChaseFrame(void) {
    Act3Obj *o;
    s32 i;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = (u8 *)sChaseSky;
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].super = NULL;
            }
            gView[0] = -0x12C0;
            gView[1] = -700;
            gView[2] = -500;
            gView[3] = -0x1770;
            gView[4] = -100;
            gView[5] = 1000;
            GsSetRefView2((GsRVIEW2 *)gView);
            /* MATCHING: x first; sched still stores z first, x's li ahead of z's. */
            gPlayer.x = -0x1658;
            gPlayer.z = 0x6A4;
            gPlayer.motion[0] = MOT_RUN;
            gPlayer.y = 0;
            gPlayer.yaw = 0;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sCan.x = -0x1658;
            sCan.y = -700;
            sCan.z = 600;
            sCanRot[0].vx = -0xE3;
            sCanRot[0].vy = -ANGLE_DEG(90);
            sCanRot[0].vz = 0;
            ClipF = 1;
            sSceneTimer = 0;
            sSubStep = 0;
            sHurtTimer = 0;
            gMissState = 1;
            gWorldIntroStep = 1;
            break;
        case 1:
            DrawFrozenFrame();
            switch (sSubStep) {
                case 0:
                    if (!(gFrameCount & 7)) {
                        PlaySe(SE_STEP);
                    }
                    gPlayer.z -= 20;
                    if (++sSceneTimer >= 30) {
                        gView[0] = -0x1658;
                        gView[1] = -300;
                        gView[2] = -300;
                        gView[4] = -100;
                        gView[5] = 1000;
                        gView[3] = -0x1658;
                        sSceneTimer = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    if (!(gFrameCount & 7)) {
                        PlaySe(SE_STEP);
                    }
                    gPlayer.z -= 30;
                    if (++sSceneTimer >= 20) {
                        PlaySe(0x21);
                        gPlayer.motion[0] = MOT_STOP;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSceneTimer = 30;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    sSceneTimer -= 2;
                    gPlayer.z -= sSceneTimer;
                    if (!(gFrameCount & 1)) {
                        SpawnEffect(4, gPlayer.x + rand() % 100 - 50, gPlayer.y + rand() % 50 - 25,
                                    gPlayer.z + rand() % 100 - 50, 0);
                    }
                    if (sSceneTimer == 0) {
                        sSceneTimer = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (++sSceneTimer == 40) {
                        gView[0] = -0x15E0;
                        gView[1] = 100;
                        gView[2] = -100;
                        gView[3] = -0x1658;
                        gView[4] = -500;
                        gView[5] = 600;
                        gPlayer.motion[0] = MOT_STAND;
                        gPlayer.action = ACT_MODEL_LOOP;
                        sSceneTimer = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (++sSceneTimer == 30) {
                        sSubStep = 25;
                        sSceneTimer = 0;
                        sEscapeTimer = sCanRot[0].vy;
                    }
                    break;
                case 25:
                    sCanRot[0].vy = sEscapeTimer + rand() % 113 - 0x38;
                    if (++sSceneTimer == 60) {
                        gView[1] = -700;
                        gView[4] = 0;
                        sCan.y = -700;
                        gPlayer.motion[0] = 0x3F;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sCanRot[0].vy = sEscapeTimer;
                        sSceneTimer = 0;
                        sSubStep = 30;
                        gView[0] = gPlayer.x;
                        gView[2] = gPlayer.z - 50;
                        gView[3] = gPlayer.x;
                        gView[5] = gPlayer.z;
                        sCan.x = gPlayer.x;
                        sCan.z = gPlayer.z;
                    }
                    break;
                case 30:
                    gPlayer.yaw = 0;
                    gView[1] += 4;
                    if (++sSceneTimer == 30) {
                        gPlayer.yaw = -ANGLE_DEG(90);
                        gPlayer.motion[0] = MOT_RUN_SLOW;
                        gPlayer.action = ACT_MODEL_LOOP;
                        sSceneTimer = 0;
                        sSubStep = 35;
                    }
                    break;
                case 35:
                    gView[1] += 4;
                    if (!(gFrameCount & 7)) {
                        PlaySe(SE_STEP);
                    }
                    gPlayer.x += 30;
                    if (++sSceneTimer == 20) {
                        sSceneTimer = 0;
                        sSubStep = 40;
                    }
                    break;
                case 40:
                    sCan.y += 50;
                    sCanRot[0].vy += 0x22;
                    if (++sSceneTimer == 14) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        sView = *(GsRVIEW2 *)gView;
                        PlaySe(0x39);
                        sSceneTimer = 0;
                        sSubStep = 45;
                    }
                    break;
                case 45:
                    sCan.x += 30;
                    sCan.y -= 30;
                    sCanRot[0].vx += 0x38;
                    sCanRot[0].vy += 0x22;
                    if (sSceneTimer < 4) {
                        gView[0] = gView[3] = rand() % 50 - 25;
                        gView[2] = gView[5] = rand() % 50 - 25;
                        gView[0] += gRefView[0];
                        gView[3] += gRefView[3];
                        gView[1] = gRefView[1];
                        gView[4] = gRefView[4];
                        gView[2] += gRefView[2];
                        gView[5] += gRefView[5];
                    } else {
                        *(GsRVIEW2 *)gView = sView;
                    }
                    if (++sSceneTimer == 30) {
                        gWorldIntroStep = 10;
                    }
                    break;
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            SetChaseLights();
            StepChaseMotion(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], 0, 0, 0);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayer();
            DrawChaseShadow();
            sAct3Coords[0].coord.t[0] = sCan.x;
            sAct3Coords[0].coord.t[1] = sCan.y;
            sAct3Coords[0].coord.t[2] = sCan.z;
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gOt[gActiveBuff], 2, getScratchAddr(0));
            DrawRepeatingStage(0, 0, 0, 50000);
            DrawSky(0);
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
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            {
                s32 t;

                t = rsin(gFrameCount * 40960 / 360) * 100;
                sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);
            }
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            if (gFrameCount % 36 == 0 || gFrameCount % 36 == 18) {
                PlaySe(0x39);
            }
            sCanRot[0].vx = ANGLE_DEG(180);
            sCanRot[0].vy = 0;
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            if (gPlayer.x + 500 > sCan.x + 120) {
                SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
                GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
                GsSetLsMatrix(&sChaseMatrix);
                GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
                GsSetLightMatrix(&sChaseMatrix);
                SetTmdSortMode(0);
                GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            }
            i = 0;
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 50000);
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
            if (gPlayer.x > 50000 && sEscapeTimer == 0) {
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
                if (gFrameCount % 36 == 0 || gFrameCount % 36 == 18) {
                    PlaySe(0x39);
                }
                gPlayer.x -= rsin(gPlayer.yaw) * 30 / 4096;
                gPlayer.z -= rcos(gPlayer.yaw) * 30 / 4096;
            }
            gWorldShift[1] = 0;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            if (sSceneTimer == 120) {
                for (gLoopI = 0; gLoopI < 20; gLoopI++) {
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0xD4E4, 0, 0, gLoopI * 2, 0,
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
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            {
                s32 t;

                t = rsin(gFrameCount * 40960 / 360) * 100;
                sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);
            }
            sCanRot[0].vx = ANGLE_DEG(180);
            sCanRot[0].vy = 0;
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            SetTmdSortMode(4);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 50000);
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
                        gStage = 15;
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
    /*   0 */ {2, {0, 0}, 0, 0, 188},
    /*   1 */ {2, {0, 0}, 200, 0, -188},
    /*   2 */ {2, {0, 0}, 2000, 0, -188},
    /*   3 */ {2, {0, 0}, 2100, 0, 62},
    /*   4 */ {2, {0, 0}, 4000, 0, -62},
    /*   5 */ {2, {0, 0}, 4150, 0, 188},
    /*   6 */ {2, {0, 0}, 6000, 0, -62},
    /*   7 */ {2, {0, 0}, 6200, 0, 62},
    /*   8 */ {2, {0, 0}, 8000, 0, 188},
    /*   9 */ {2, {0, 0}, 8100, 0, -188},
    /*  10 */ {2, {0, 0}, 9500, 0, 62},
    /*  11 */ {2, {0, 0}, 9520, 0, -62},
    /*  12 */ {2, {0, 0}, 10500, 0, -188},
    /*  13 */ {2, {0, 0}, 10530, 0, -62},
    /*  14 */ {1, {0, 0}, 11800, 0, 188},
    /*  15 */ {2, {0, 0}, 12000, 0, 62},
    /*  16 */ {2, {0, 0}, 13060, 0, 188},
    /*  17 */ {1, {0, 0}, 13400, 0, -62},
    /*  18 */ {2, {0, 0}, 14000, 0, -62},
    /*  19 */ {2, {0, 0}, 14054, 0, -188},
    /*  20 */ {2, {0, 0}, 14089, 0, 188},
    /*  21 */ {2, {0, 0}, 15000, 0, 188},
    /*  22 */ {2, {0, 0}, 15054, 0, -188},
    /*  23 */ {2, {0, 0}, 16089, 0, 62},
    /*  24 */ {1, {0, 0}, 17000, 0, -188},
    /*  25 */ {1, {0, 0}, 17100, 0, 62},
    /*  26 */ {2, {0, 0}, 18500, 0, 188},
    /*  27 */ {0, {0, 0}, 18600, 0, -188},
    /*  28 */ {0, {0, 0}, 19550, 0, 62},
    /*  29 */ {0, {0, 0}, 19600, 0, -62},
    /*  30 */ {1, {0, 0}, 19600, 0, 188},
    /*  31 */ {0, {0, 0}, 20000, 0, 188},
    /*  32 */ {0, {0, 0}, 22500, 0, -62},
    /*  33 */ {0, {0, 0}, 22600, 0, 188},
    /*  34 */ {1, {0, 0}, 22600, 0, 62},
    /*  35 */ {1, {0, 0}, 22700, 0, -188},
    /*  36 */ {2, {0, 0}, 24000, 0, 188},
    /*  37 */ {2, {0, 0}, 24100, 0, 62},
    /*  38 */ {0, {0, 0}, 25150, 0, -62},
    /*  39 */ {0, {0, 0}, 25200, 0, -188},
    /*  40 */ {1, {0, 0}, 25150, 0, -62},
    /*  41 */ {1, {0, 0}, 25200, 0, -188},
    /*  42 */ {2, {0, 0}, 26000, 0, -62},
    /*  43 */ {2, {0, 0}, 26100, 0, -188},
    /*  44 */ {0, {0, 0}, 27700, 0, 62},
    /*  45 */ {0, {0, 0}, 27050, 0, 188},
    /*  46 */ {1, {0, 0}, 28100, 0, -188},
    /*  47 */ {0, {0, 0}, 29000, 0, 62},
    /*  48 */ {0, {0, 0}, 30500, 0, 188},
    /*  49 */ {1, {0, 0}, 30600, 0, -188},
    /*  50 */ {1, {0, 0}, 32000, 0, -125},
    /*  51 */ {0, {0, 0}, 32100, 0, 188},
    /*  52 */ {0, {0, 0}, 32150, 0, 62},
    /*  53 */ {0, {0, 0}, 33100, 0, -188},
    /*  54 */ {0, {0, 0}, 33100, 0, -62},
    /*  55 */ {1, {0, 0}, 33000, 0, 188},
    /*  56 */ {0, {0, 0}, 34500, 0, 188},
    /*  57 */ {0, {0, 0}, 34530, 0, 62},
    /*  58 */ {0, {0, 0}, 35700, 0, -188},
    /*  59 */ {0, {0, 0}, 35750, 0, -62},
    /*  60 */ {1, {0, 0}, 35900, 0, 0},
    /*  61 */ {0, {0, 0}, 36500, 0, -62},
    /*  62 */ {0, {0, 0}, 36550, 0, 62},
    /*  63 */ {1, {0, 0}, 37100, 0, 188},
    /*  64 */ {1, {0, 0}, 37150, 0, -188},
    /*  65 */ {0, {0, 0}, 38000, 0, -62},
    /*  66 */ {0, {0, 0}, 38100, 0, 62},
    /*  67 */ {1, {0, 0}, 38600, 0, -62},
    /*  68 */ {3, {0, 0}, 40000, 0, -188},
    /*  69 */ {3, {0, 0}, 40100, 0, 188},
    /*  70 */ {1, {0, 0}, 49950, 0, 62},
    /*  71 */ {3, {0, 0}, 40050, 0, 188},
    /*  72 */ {3, {0, 0}, 40150, 0, -63},
    /*  73 */ {3, {0, 0}, 40500, 0, 64},
    /*  74 */ {3, {0, 0}, 41050, 0, 188},
    /*  75 */ {3, {0, 0}, 40950, 0, -188},
    /*  76 */ {1, {0, 0}, 41100, 0, 64},
    /*  77 */ {1, {0, 0}, 41200, 0, -64},
    /*  78 */ {3, {0, 0}, 42500, 0, 188},
    /*  79 */ {3, {0, 0}, 42600, 0, 64},
    /*  80 */ {3, {0, 0}, 42650, 0, -64},
    /*  81 */ {3, {0, 0}, 43400, 0, 188},
    /*  82 */ {3, {0, 0}, 43500, 0, -188},
    /*  83 */ {3, {0, 0}, 43500, 0, -64},
    /*  84 */ {3, {0, 0}, 44950, 0, 64},
    /*  85 */ {3, {0, 0}, 46000, 0, 64},
    /*  86 */ {3, {0, 0}, 46050, 0, -64},
    /*  87 */ {1, {0, 0}, 46050, 0, -64},
    /*  88 */ {3, {0, 0}, 47020, 0, -188},
    /*  89 */ {3, {0, 0}, 47050, 0, 188},
};

#define CHASE_PIECE_STEPS StepChasePiece0, StepChasePiece1, StepChasePiece2, StepChasePiece3
#define CHASE_SKY \
    {0x6B, 0xBC, 0xEB, 0}, {0xFF, 0xFF, 0xFF, 0}, {0xFF, 0xFF, 0xFF, 0}, {0xFF, 0xFF, 0xFF, 0}

#include "chase_data.inc"
