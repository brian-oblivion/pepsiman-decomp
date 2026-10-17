/* Stage 2-3's overlay: the act-3 chase of world 2, built like stage 1-3's
 * from chase.inc with its own objects. */

#define CHASE_LENGTH 10240000
#define CHASE_REACH 550

#include "chase.inc"
#include "rand.h"
#include "spad.h"
#include "prim.h"

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

#include "chase_pieces.inc"

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
    s32 v;
    PlayerState *p;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = (u8 *)sChaseSky;
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].super = NULL;
            }
            gView[0] = -0x9C4;
            gView[1] = -400;
            gView[2] = 0;
            gView[3] = -0x1388;
            gView[4] = -0x7D0;
            gView[5] = 0;
            GsSetRefView2((GsRVIEW2 *)gView);
            gPlayer.yaw = -0x400;
            gPlayer.motion[0] = 0x3C;
            gPlayer.x = -0x1388;
            gPlayer.y = -0x7D0;
            gPlayer.z = 0;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sCan.x = -0x14B4;
            sCan.y = 0;
            sCan.z = 0;
            sCanRot[0].vx = 0;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = 0;
            sCanRot[1].vx = 0;
            sCanRot[1].vy = 0;
            sCanRot[1].vz = 0;
            PlaySe(0x1B);
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
                    p = &gPlayer;
                    p->y += 50;
                    gView[4] = p->y;
                    gView[3] = p->x;
                    gView[5] = p->z;
                    gView[0] -= 40;
                    gView[1] -= 10;
                    if (p->y >= -150) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x39);
                        gPlayer.motion[0] = 0x39;
                        gPlayer.action = ACT_MODEL_ONCE;
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200, -250 - rand() % 100,
                                        gPlayer.z - (100 - rand() % 200), rand() % 4);
                        }
                        sView = *(GsRVIEW2 *)gView;
                        sSceneTimer = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5: {
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
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    v = rsin(++sSceneTimer * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(v / 4096);
                    gView[0] = -0xA8C;
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
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (++sSceneTimer >= 30) {
                        gPlayer.motion[0] = 0x3E;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSceneTimer = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (++sSceneTimer >= 72) {
                        gPlayer.motion[0] = 0x3B;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSceneTimer = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    if (++sSceneTimer >= 30) {
                        gView[0] = -0xA8C - (sSceneTimer - 30) * 12;
                        gView[1] = (sSceneTimer - 30) * 5 - 300;
                        gView[2] = 0;
                        gView[3] = gPlayer.x;
                        gView[4] = gPlayer.y - (sSceneTimer - 30) * 5;
                        gView[5] = gPlayer.z;
                    }
                    if (sSceneTimer >= 60) {
                        PlaySe(0x55);
                        sSceneTimer = 30;
                        sSubStep = 30;
                    }
                    break;
                case 30:
                    if (gFrameCount & 1) {
                        sSceneTimer++;
                    }
                    sCan.x += sSceneTimer;
                    *(GsRVIEW2 *)gView = sView;
                    if (sSceneTimer >= 60) {
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
            GsSortObject4(&sAct3Objs[0], &gOt[gActiveBuff], 2, getScratchAddr(0));
            sAct3Coords[1].coord.t[0] = sCan.x;
            sAct3Coords[1].coord.t[1] = sCan.y;
            sAct3Coords[1].coord.t[2] = sCan.z;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[1].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            GsSortObject4(&sAct3Objs[1], &gOt[gActiveBuff], 2, getScratchAddr(0));
            DrawRepeatingStage(0, 0, 0, 40000);
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
            sCan.x = -0xB22;
            sCan.y = 0;
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
            if (sCan.x + 1050 < gPlayer.x) {
                sCan.x = gPlayer.x - 1050;
            }
            SpawnEffect(4, sCan.x + 550, -50 - rand() % 50, rand() % 500 - 250, 0);
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1];
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vx = 0;
            ResetSpadStack();
            sCanRot[0].vy = rsin(gFrameCount * 32768 / 360) * 15 / 4096 * 4096 / 360;
            SetSpadStack();
            sCanRot[0].vz = 0;
            if (gPlayer.x + 500 > sCan.x + 550) {
                SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
                GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
                GsSetLsMatrix(&sChaseMatrix);
                GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
                GsSetLightMatrix(&sChaseMatrix);
                GsSortObject4(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            }
            sAct3Coords[1].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[1].coord.t[1] = sCan.y + gWorldShift[1];
            sAct3Coords[1].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[1].vx = 0;
            /* MATCHING: the barrier lengthens both bases' lives, so the coordinates'
             * base outranks the rotations' for the lower saved register. */
            __asm__("");
            ResetSpadStack();
            sCanRot[1].vy = rsin((gFrameCount * 8 - 30) * 4096 / 360) * 10 / 4096 * 4096 / 360;
            SetSpadStack();
            sCanRot[1].vz = 0;
            if (gPlayer.x + 500 > sCan.x + 550) {
                SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
                GsGetLs(sAct3Objs[1].coord2, &sChaseMatrix);
                GsSetLsMatrix(&sChaseMatrix);
                GsGetLw(sAct3Objs[1].coord2, &sChaseMatrix);
                GsSetLightMatrix(&sChaseMatrix);
                GsSortObject4(&sAct3Objs[1], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            }
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
            if (gPlayer.x < sCan.x + 550 && (s8)gPlayer.slideState != 3) {
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
                sFadeMode = (DR_MODE *)((u8 *)sFadeMode + 8);
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
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0xADD4, 0, 0, gLoopI * 2, rand() % 8,
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
                SpawnEffect(4, sCan.x + 550, -50 - rand() % 50, rand() % 500 - 250, 0);
            }
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1];
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vx = 0;
            sCanRot[0].vy = rsin(gFrameCount * 32768 / 360) * 15 / 4096 * 4096 / 360;
            sCanRot[0].vz = 0;
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            GsSortObject4(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            sAct3Coords[1].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[1].coord.t[1] = sCan.y + gWorldShift[1];
            sAct3Coords[1].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[1].vx = 0;
            sCanRot[1].vy = rsin((gFrameCount * 8 - 30) * 4096 / 360) * 10 / 4096 * 4096 / 360;
            sCanRot[1].vz = 0;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[1].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            GsSortObject4(&sAct3Objs[1], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
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
                        gStage = STAGE_4_1;
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
    /*   0 */ {5, {0, 0}, 1500, 0, -250},
    /*   1 */ {5, {0, 0}, 1550, 0, -150},
    /*   2 */ {1, {0, 0}, 1700, 0, 180},
    /*   3 */ {3, {0, 0}, 1780, 0, 80},
    /*   4 */ {5, {0, 0}, 1820, 0, 190},
    /*   5 */ {0, {0, 0}, 2350, 0, -190},
    /*   6 */ {5, {0, 0}, 2450, 0, -60},
    /*   7 */ {8, {0, 0}, 2500, -250, -280},
    /*   8 */ {5, {0, 0}, 2550, 0, -150},
    /*   9 */ {1, {0, 0}, 3150, 0, -100},
    /*  10 */ {4, {0, 0}, 3250, 0, -180},
    /*  11 */ {5, {0, 0}, 3600, 0, 150},
    /*  12 */ {5, {0, 0}, 3650, 0, 40},
    /*  13 */ {7, {0, 0}, 3750, 0, 200},
    /*  14 */ {5, {0, 0}, 4400, 0, -150},
    /*  15 */ {5, {0, 0}, 4450, 0, -50},
    /*  16 */ {7, {0, 0}, 4600, 0, -205},
    /*  17 */ {5, {0, 0}, 5450, 0, 70},
    /*  18 */ {5, {0, 0}, 5550, 0, 150},
    /*  19 */ {4, {0, 0}, 6500, 0, -150},
    /*  20 */ {1, {0, 0}, 6800, 0, 160},
    /*  21 */ {4, {0, 0}, 7100, 0, -5},
    /*  22 */ {3, {0, 0}, 7500, 0, 190},
    /*  23 */ {9, {0, 0}, 7500, -250, 280},
    /*  24 */ {3, {0, 0}, 7550, 0, 100},
    /*  25 */ {1, {0, 0}, 8150, 0, 180},
    /*  26 */ {5, {0, 0}, 8400, 0, -50},
    /*  27 */ {5, {0, 0}, 8500, 0, -190},
    /*  28 */ {7, {0, 0}, 8550, 0, -110},
    /*  29 */ {1, {0, 0}, 9100, 0, 200},
    /*  30 */ {4, {0, 0}, 9300, 0, -160},
    /*  31 */ {7, {0, 0}, 9750, 0, 190},
    /*  32 */ {5, {0, 0}, 10050, 0, 50},
    /*  33 */ {1, {0, 0}, 10750, 0, 175},
    /*  34 */ {4, {0, 0}, 11200, 0, 85},
    /*  35 */ {7, {0, 0}, 11750, 0, 125},
    /*  36 */ {5, {0, 0}, 11850, 0, 20},
    /*  37 */ {0, {0, 0}, 12350, 0, 45},
    /*  38 */ {10, {0, 0}, 12500, -350, -280},
    /*  39 */ {3, {0, 0}, 12550, 0, 190},
    /*  40 */ {4, {0, 0}, 12800, 0, 100},
    /*  41 */ {5, {0, 0}, 13050, 0, 105},
    /*  42 */ {5, {0, 0}, 13150, 0, 190},
    /*  43 */ {4, {0, 0}, 13350, 0, -110},
    /*  44 */ {3, {0, 0}, 13500, 0, -5},
    /*  45 */ {3, {0, 0}, 13530, 0, 95},
    /*  46 */ {4, {0, 0}, 14000, 0, 105},
    /*  47 */ {1, {0, 0}, 14600, 0, -175},
    /*  48 */ {4, {0, 0}, 14790, 0, 10},
    /*  49 */ {7, {0, 0}, 14760, 0, 150},
    /*  50 */ {7, {0, 0}, 14900, 0, 50},
    /*  51 */ {7, {0, 0}, 15750, 0, 175},
    /*  52 */ {7, {0, 0}, 15850, 0, 75},
    /*  53 */ {7, {0, 0}, 16450, 0, 96},
    /*  54 */ {7, {0, 0}, 16500, 0, 120},
    /*  55 */ {1, {0, 0}, 17300, 0, -190},
    /*  56 */ {1, {0, 0}, 17500, 0, 190},
    /*  57 */ {0, {0, 0}, 17550, 0, 65},
    /*  58 */ {9, {0, 0}, 17500, -250, 280},
    /*  59 */ {1, {0, 0}, 18000, 0, 200},
    /*  60 */ {4, {0, 0}, 18050, 0, 75},
    /*  61 */ {3, {0, 0}, 18880, 0, -5},
    /*  62 */ {3, {0, 0}, 18920, 0, -100},
    /*  63 */ {7, {0, 0}, 18980, 0, -200},
    /*  64 */ {4, {0, 0}, 19900, 0, 150},
    /*  65 */ {5, {0, 0}, 21050, 0, 0},
    /*  66 */ {5, {0, 0}, 21150, 0, -100},
    /*  67 */ {4, {0, 0}, 21450, 0, -190},
    /*  68 */ {10, {0, 0}, 22500, -350, -280},
    /*  69 */ {7, {0, 0}, 22500, 0, -195},
    /*  70 */ {7, {0, 0}, 22550, 0, -100},
    /*  71 */ {3, {0, 0}, 22800, 0, -50},
    /*  72 */ {3, {0, 0}, 22850, 0, 210},
    /*  73 */ {0, {0, 0}, 23000, 0, -45},
    /*  74 */ {0, {0, 0}, 23200, 0, -100},
    /*  75 */ {7, {0, 0}, 23850, 0, -180},
    /*  76 */ {7, {0, 0}, 24100, 0, -35},
    /*  77 */ {1, {0, 0}, 24500, 0, -150},
    /*  78 */ {4, {0, 0}, 24800, 0, -50},
    /*  79 */ {3, {0, 0}, 25480, 0, 200},
    /*  80 */ {5, {0, 0}, 25550, 0, -130},
    /*  81 */ {5, {0, 0}, 25650, 0, -10},
    /*  82 */ {5, {0, 0}, 25800, 0, -155},
    /*  83 */ {5, {0, 0}, 25900, 0, -95},
    /*  84 */ {5, {0, 0}, 26050, 0, -5},
    /*  85 */ {7, {0, 0}, 26550, 0, -180},
    /*  86 */ {7, {0, 0}, 27160, 0, -30},
    /*  87 */ {5, {0, 0}, 27350, 0, 180},
    /*  88 */ {9, {0, 0}, 27500, -250, 280},
    /*  89 */ {1, {0, 0}, 28550, 0, -190},
    /*  90 */ {3, {0, 0}, 28850, 0, 5},
    /*  91 */ {1, {0, 0}, 29350, 0, 195},
    /*  92 */ {4, {0, 0}, 29500, 0, -150},
    /*  93 */ {3, {0, 0}, 30450, 0, -195},
    /*  94 */ {3, {0, 0}, 30550, 0, -110},
    /*  95 */ {1, {0, 0}, 31200, 0, 195},
    /*  96 */ {4, {0, 0}, 31350, 0, 80},
    /*  97 */ {5, {0, 0}, 31900, 0, 25},
    /*  98 */ {7, {0, 0}, 31980, 0, 175},
    /*  99 */ {1, {0, 0}, 32450, 0, -180},
    /* 100 */ {8, {0, 0}, 32500, -250, -280},
    /* 101 */ {7, {0, 0}, 33050, 0, -200},
    /* 102 */ {7, {0, 0}, 33125, 0, -100},
    /* 103 */ {1, {0, 0}, 33650, 0, 190},
    /* 104 */ {4, {0, 0}, 33750, 0, 55},
    /* 105 */ {3, {0, 0}, 34550, 0, -100},
    /* 106 */ {3, {0, 0}, 34650, 0, 10},
    /* 107 */ {7, {0, 0}, 35750, 0, -200},
    /* 108 */ {7, {0, 0}, 35900, 0, -125},
    /* 109 */ {7, {0, 0}, 36150, 0, 0},
    /* 110 */ {3, {0, 0}, 36650, 0, 210},
    /* 111 */ {3, {0, 0}, 36700, 0, 25},
    /* 112 */ {3, {0, 0}, 36750, 0, 125},
    /* 113 */ {1, {0, 0}, 37400, 0, 200},
    /* 114 */ {11, {0, 0}, 37500, -350, 280},
    /* 115 */ {7, {0, 0}, 37850, 0, -10},
    /* 116 */ {4, {0, 0}, 37950, 0, -210},
    /* 117 */ {4, {0, 0}, 38050, 0, -125},
    /* 118 */ {1, {0, 0}, 38650, 0, -100},
    /* 119 */ {3, {0, 0}, 38750, 0, 150},
    /* 120 */ {3, {0, 0}, 38850, 0, 190},
    /* 121 */ {5, {0, 0}, 38950, 0, -125},
    /* 122 */ {1, {0, 0}, 39250, 0, 195},
    /* 123 */ {5, {0, 0}, 39250, 0, 90},
    /* 124 */ {5, {0, 0}, 39350, 0, 125},
    /* 125 */ {1, {0, 0}, 39350, 0, -195},
};

#define CHASE_PIECE_STEPS                                                                    \
    StepChasePiece0, StepChasePiece1, StepChasePiece2, StepChasePiece3, StepChasePiece4,     \
        StepChasePiece5, StepChasePiece6, StepChasePiece7, StepChasePiece8, StepChasePiece9, \
        StepChasePiece10, StepChasePiece11, StepChasePiece12
#define CHASE_SKY {0, 0, 0x20, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}

#include "chase_data.inc"
