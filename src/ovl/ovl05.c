/* Stage 1-3's overlay: the act-3 chase, where the player runs toward the
 * camera, jumps and steps sideways, and the stage's own objects come at
 * it. */

#define CHASE_LENGTH 7680000
#define CHASE_REACH 120

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

/* The can's scale in the opening scene, and whether it applies. */
extern s32 sCanUnused[3];
extern VECTOR sCanScale;
extern s32 sCanScaled;
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
s32 func_800F4794(void) {
    Act3Obj *o;
    s32 i;
    s32 v;
    s32 r;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = (u8 *)sChaseSky;
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].super = NULL;
            }
            gView[0] = -0x157C;
            gView[1] = -200;
            gView[2] = -400;
            gView[3] = -0x157C;
            gView[4] = -140;
            gView[5] = 0;
            GsSetRefView2((GsRVIEW2 *)gView);
            gPlayer.yaw = -0x800;
            gPlayer.motion[0] = 0x13;
            gPlayer.x = -0x157C;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sCan.x = -0x15E0;
            sCan.y = -0x514;
            sCan.z = 0x76C;
            sCanRot[0].vx = -0x238;
            sCanRot[0].vy = -0x5C7;
            sCanRot[0].vz = 0;
            sCan.lidZ = -0x9C4;
            sCan.lidX = -0x157C;
            sCan.lidY = 0;
            sCanRot[1].vy = 0x800;
            sCanRot[1].vx = 0;
            sCanRot[1].vz = 0;
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
                    if (++sSceneTimer >= 30) {
                        sCanScaled = 1;
                        sSceneTimer = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    gView[0] = -0x157C;
                    gView[1] = -250;
                    sCan.lidZ += 30;
                    /* MATCHING: the offset kept in a local, so cc1 does not fold the 30 in. */
                    gView[2] = sCan.lidZ - (r = sSceneTimer * 5 - 700);
                    gView[3] = sCan.lidX;
                    gView[4] = sCan.lidY;
                    gView[5] = sCan.lidZ;
                    if (sSceneTimer % 10 == 0) {
                        PlaySe(0x54);
                    }
                    if (++sSceneTimer >= 30) {
                        sCan.lidZ = -2000;
                        sSceneTimer = 0;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    gView[0] = -0x157C;
                    gView[1] = -200;
                    gView[2] = sSceneTimer * 30 - 1000;
                    gView[3] = -0x157C;
                    gView[4] = -150;
                    gView[5] = sSceneTimer * 30 - 500;
                    if (sSceneTimer == 10) {
                        PlaySe(0x58);
                    }
                    if (++sSceneTimer >= 25) {
                        sCan.lidZ = -500;
                        sSceneTimer = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    gView[0] = -0x15AE;
                    gView[1] = -500;
                    gView[2] = 500;
                    gView[3] = -0x157C;
                    gView[4] = -150;
                    gView[5] = 0;
                    sCan.lidZ += 30;
                    if (++sSceneTimer >= 10) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x38);
                        PlaySe(0x34);
                        /* MATCHING: z less (100 less the roll) takes z - 100 first. */
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200 - 100, -150 - rand() % 50,
                                        gPlayer.z - (100 - rand() % 200), rand() % 4);
                        }
                        gPlayer.motion[0] = 0x3C;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sEscapeTimer = 30;
                        sSceneTimer = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (sEscapeTimer != 0) {
                        sEscapeTimer -= 5;
                    }
                    sCan.lidZ += sEscapeTimer;
                    gPlayer.x -= 6;
                    gPlayer.y -= 0x38;
                    gPlayer.z += 60;
                    if (++sSceneTimer >= 7) {
                        gPlayer.x = -0x1518;
                        gPlayer.y = -1200;
                        gPlayer.z = 1000;
                        sSceneTimer = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    gView[0] = -5000;
                    gView[1] = -1000;
                    gView[2] = 1000;
                    gView[3] = -0x170C;
                    gView[4] = -0x514;
                    gView[5] = 2000;
                    gPlayer.z += 60;
                    if (++sSceneTimer >= 10) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x39);
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200 - 100, gPlayer.y + rand() % 200 - 100,
                                        gPlayer.z - (100 - rand() % 200), rand() % 4);
                        }
                        sSceneTimer = 0;
                        sSubStep = 30;
                        sEscapeTimer = sCanRot[0].vx;
                    }
                    break;
                case 30:
                    gPlayer.x += 60;
                    gPlayer.y += 10;
                    gPlayer.z -= 60;
                    sCanRot[0].vx = sEscapeTimer + rand() % 113 - 0x38;
                    if (++sSceneTimer >= 30) {
                        sSceneTimer = 0;
                        sSubStep = 35;
                    }
                    break;
                case 35:
                    sCanRot[0].vx += 0x16;
                    sCanRot[0].vz -= 0x2D;
                    sCan.x += 10;
                    sCan.z -= 10;
                    sCan.y = rsin((sSceneTimer << 12) / 360) * 500 / 4096 - 0x514;
                    if (++sSceneTimer >= 25) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 10;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x3B);
                        sSceneTimer = 0;
                        sSubStep = 40;
                    }
                    break;
                case 40:
                    sCanRot[0].vz -= 0x2D;
                    sCan.x += 10;
                    sCan.z -= 10;
                    sCan.y = -(rsin(sSceneTimer * 20480 / 360) * 300 / 4096) - 0x44C;
                    if (++sSceneTimer >= 30) {
                        sCan.x = -0x1068;
                        sCan.z = 0;
                        sCan.y = 0;
                        sCanRot[0].vx = 0x800;
                        sCanScaled = 0;
                        sSceneTimer = 18;
                        sCanRot[0].vy = 0;
                        sCanRot[0].vz = 0;
                        sSubStep = 45;
                    }
                    break;
                case 45:
                    if (sSceneTimer >= 36 && sSceneTimer < 40) {
                        gView[0] = -3000;
                        gView[3] = -4000;
                        gView[1] = gView[4] = rand() % 50 - 25;
                        gView[2] = gView[5] = rand() % 50 - 25;
                        gView[1] -= 300;
                    } else {
                        gView[0] = -3000;
                        gView[1] = -300;
                        gView[2] = 0;
                        gView[3] = -4000;
                        gView[4] = 0;
                        gView[5] = 0;
                    }
                    v = rsin(sSceneTimer * 20480 / 360);
                    sCan.y = -(ABS_GE(v) * 500) / 4096;
                    sCan.x += 30;
                    sCanRot[0].vz -= 0x71;
                    if (++sSceneTimer == 36) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 10;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x39);
                        for (gLoopI = 0; gLoopI < 16; gLoopI++) {
                            SpawnEffect(4, sCan.x + rand() % 100 - 50, -(rand() % 50),
                                        rand() % 500 - 250, rand() % 4);
                        }
                    }
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
            if (sCanScaled != 0) {
                sCanScale.vx = 0x1800;
                sCanScale.vy = 0x1800;
                sCanScale.vz = 0x1800;
                ScaleMatrix(&sChaseMatrix, &sCanScale);
            }
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            sAct3Coords[1].coord.t[0] = sCan.lidX;
            sAct3Coords[1].coord.t[1] = sCan.lidY;
            sAct3Coords[1].coord.t[2] = sCan.lidZ;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[1].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            GsSortObject4(&sAct3Objs[1], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            DrawRepeatingStage(0, 0, 0, 30000);
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
            if (sCan.x + 620 < gPlayer.x) {
                sCan.x = gPlayer.x - 620;
            }
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
            sCanRot[0].vx = 0x800;
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
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
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
            if (gPlayer.x > 30000 && sEscapeTimer == 0) {
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
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0x86C4, 0, 0, gLoopI * 2, rand() % 8,
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
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            {
                s32 t;

                t = rsin(gFrameCount * 40960 / 360) * 100;
                sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);
            }
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLsMatrix(&sChaseMatrix);
            GsGetLw(sAct3Objs[0].coord2, &sChaseMatrix);
            GsSetLightMatrix(&sChaseMatrix);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gOt[gActiveBuff], 2, getScratchAddr(0));
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
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
                        gStage = STAGE_2_1;
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
    /*   0 */ {1, {0, 0}, 700, 0, 230},
    /*   1 */ {6, {0, 0}, 1460, 0, -220},
    /*   2 */ {6, {0, 0}, 1500, 0, -180},
    /*   3 */ {6, {0, 0}, 1560, 0, -235},
    /*   4 */ {5, {0, 0}, 2150, 0, 200},
    /*   5 */ {5, {0, 0}, 2240, 0, 150},
    /*   6 */ {1, {0, 0}, 2250, 0, -220},
    /*   7 */ {6, {0, 0}, 3100, 0, -250},
    /*   8 */ {6, {0, 0}, 3160, 0, -210},
    /*   9 */ {6, {0, 0}, 3210, 0, -220},
    /*  10 */ {5, {0, 0}, 4000, 0, 210},
    /*  11 */ {5, {0, 0}, 4060, 0, 110},
    /*  12 */ {5, {0, 0}, 4120, 0, 250},
    /*  13 */ {7, {0, 0}, 4500, 0, -225},
    /*  14 */ {1, {0, 0}, 4900, 0, -210},
    /*  15 */ {0, {0, 0}, 4980, 0, 20},
    /*  16 */ {6, {0, 0}, 5350, 0, 240},
    /*  17 */ {6, {0, 0}, 5410, 0, 190},
    /*  18 */ {0, {0, 0}, 5800, 0, -20},
    /*  19 */ {2, {0, 0}, 6400, 0, 205},
    /*  20 */ {3, {0, 0}, 6430, 0, -200},
    /*  21 */ {3, {0, 0}, 6510, 0, -105},
    /*  22 */ {5, {0, 0}, 7290, 0, -100},
    /*  23 */ {5, {0, 0}, 7420, 0, -210},
    /*  24 */ {5, {0, 0}, 7500, 0, -155},
    /*  25 */ {6, {0, 0}, 7900, 0, 195},
    /*  26 */ {6, {0, 0}, 7960, 0, 60},
    /*  27 */ {6, {0, 0}, 8010, 0, 240},
    /*  28 */ {3, {0, 0}, 8500, 0, 195},
    /*  29 */ {3, {0, 0}, 8600, 0, 95},
    /*  30 */ {1, {0, 0}, 8950, 0, 30},
    /*  31 */ {0, {0, 0}, 9050, 0, -120},
    /*  32 */ {4, {0, 0}, 9200, 0, 80},
    /*  33 */ {3, {0, 0}, 9700, 0, 60},
    /*  34 */ {5, {0, 0}, 10250, 0, 110},
    /*  35 */ {5, {0, 0}, 10430, 0, 125},
    /*  36 */ {2, {0, 0}, 11000, 0, 200},
    /*  37 */ {5, {0, 0}, 11220, 0, 205},
    /*  38 */ {5, {0, 0}, 11280, 0, 50},
    /*  39 */ {1, {0, 0}, 11840, 0, -230},
    /*  40 */ {6, {0, 0}, 11900, 0, -75},
    /*  41 */ {6, {0, 0}, 12000, 0, -230},
    /*  42 */ {6, {0, 0}, 12000, 0, 155},
    /*  43 */ {2, {0, 0}, 12500, 0, -170},
    /*  44 */ {5, {0, 0}, 13080, 0, -100},
    /*  45 */ {5, {0, 0}, 13200, 0, -180},
    /*  46 */ {3, {0, 0}, 13320, 0, -95},
    /*  47 */ {6, {0, 0}, 13900, 0, -150},
    /*  48 */ {6, {0, 0}, 14210, 0, -120},
    /*  49 */ {6, {0, 0}, 14230, 0, -90},
    /*  50 */ {1, {0, 0}, 14400, 0, -150},
    /*  51 */ {0, {0, 0}, 14810, 0, -100},
    /*  52 */ {0, {0, 0}, 15300, 0, -80},
    /*  53 */ {3, {0, 0}, 15450, 0, -60},
    /*  54 */ {3, {0, 0}, 15560, 0, -140},
    /*  55 */ {5, {0, 0}, 16190, 0, 210},
    /*  56 */ {5, {0, 0}, 16250, 0, 120},
    /*  57 */ {1, {0, 0}, 16650, 0, -210},
    /*  58 */ {6, {0, 0}, 16820, 0, 212},
    /*  59 */ {6, {0, 0}, 16860, 0, 160},
    /*  60 */ {6, {0, 0}, 16960, 0, 230},
    /*  61 */ {1, {0, 0}, 17050, 0, 190},
    /*  62 */ {2, {0, 0}, 17600, 0, -200},
    /*  63 */ {3, {0, 0}, 17750, 0, 205},
    /*  64 */ {3, {0, 0}, 17860, 0, 110},
    /*  65 */ {4, {0, 0}, 18650, 0, -30},
    /*  66 */ {6, {0, 0}, 18900, 0, -190},
    /*  67 */ {6, {0, 0}, 18960, 0, -120},
    /*  68 */ {6, {0, 0}, 18600, 0, -240},
    /*  69 */ {5, {0, 0}, 19380, 0, 230},
    /*  70 */ {5, {0, 0}, 19460, 0, 194},
    /*  71 */ {1, {0, 0}, 20250, 0, 210},
    /*  72 */ {3, {0, 0}, 20750, 0, -110},
    /*  73 */ {7, {0, 0}, 20840, 0, -205},
    /*  74 */ {5, {0, 0}, 21550, 0, -200},
    /*  75 */ {2, {0, 0}, 21650, 0, 195},
    /*  76 */ {6, {0, 0}, 21270, 0, 230},
    /*  77 */ {6, {0, 0}, 21300, 0, 150},
    /*  78 */ {6, {0, 0}, 21350, 0, 200},
    /*  79 */ {0, {0, 0}, 21950, 0, 30},
    /*  80 */ {7, {0, 0}, 22800, 0, -250},
    /*  81 */ {7, {0, 0}, 22890, 0, -110},
    /*  82 */ {0, {0, 0}, 23400, 0, -30},
    /*  83 */ {3, {0, 0}, 23850, 0, 225},
    /*  84 */ {3, {0, 0}, 23950, 0, 195},
    /*  85 */ {1, {0, 0}, 24200, 0, -200},
    /*  86 */ {0, {0, 0}, 24400, 0, -105},
    /*  87 */ {7, {0, 0}, 25450, 0, 200},
    /*  88 */ {7, {0, 0}, 25560, 0, 95},
    /*  89 */ {1, {0, 0}, 26300, 0, -210},
    /*  90 */ {6, {0, 0}, 26920, 0, 220},
    /*  91 */ {6, {0, 0}, 26970, 0, 240},
    /*  92 */ {6, {0, 0}, 27010, 0, 140},
    /*  93 */ {4, {0, 0}, 27400, 0, -55},
    /*  94 */ {7, {0, 0}, 27950, 0, -210},
    /*  95 */ {7, {0, 0}, 28080, 0, -105},
    /*  96 */ {3, {0, 0}, 28550, 0, 210},
    /*  97 */ {3, {0, 0}, 28650, 0, 120},
    /*  98 */ {1, {0, 0}, 28700, 0, 85},
    /*  99 */ {2, {0, 0}, 29200, 0, -195},
    /* 100 */ {3, {0, 0}, 29280, 0, -50},
    /* 101 */ {3, {0, 0}, 29310, 0, 50},
    /* 102 */ {1, {0, 0}, 29500, 0, 200},
};

#define CHASE_PIECE_STEPS                                                                    \
    StepChasePiece0, StepChasePiece1, StepChasePiece2, StepChasePiece3, StepChasePiece4,     \
        StepChasePiece5, StepChasePiece6, StepChasePiece7, StepChasePiece8, StepChasePiece9, \
        StepChasePiece10, StepChasePiece11, StepChasePiece12
#define CHASE_SKY \
    {0x6B, 0xBC, 0xEB, 0}, {0xFF, 0xFF, 0xFF, 0}, {0xFF, 0xFF, 0xFF, 0}, {0xFF, 0xFF, 0xFF, 0}

#include "chase_data.inc"

s32 sCanUnused[3]; /* never read or written */
VECTOR sCanScale;
s32 sCanScaled;
