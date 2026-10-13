/* Stage 2-3's overlay: the act-3 chase of world 2, built like stage 1-3's
 * from chase.inc with its own objects. */

#define CHASE_LENGTH 10240000
#define CHASE_PLACEMENTS 126
#define CHASE_VAB 3
#define CHASE_REACH 550
#define CHASE_LIGHT_BLUE 0x40
#define CHASE_FAR_COLOR 0, 0, 32
/* MATCHING: the loader's jump table sits 4 bytes past an 8-byte boundary;
 * this toolchain aligns jump tables to 8, so the loader stays asm. */
#define CHASE_LOADER INCLUDE_ASM("asm/ovl/ovl08/nonmatchings/ovl08", LoadChaseStage);

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

/* The frame step's state. */
extern s32 D_800F8034;
extern s32 D_800F8038;
extern s32 D_800F803C;
extern POLY_F4 *D_800F8040;
extern DR_MODE *D_800F8044;
extern MATRIX D_800F8014;
extern s32 D_800F7A54;
extern s32 (*D_800F7A60[])(Act3Obj *o);
extern u8 D_800F7A94[];
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
s32 func_800F4758(void) {
    Act3Obj *o;
    s32 i;
    s32 v;
    PlayerState *p;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = D_800F7A94;
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
            D_800F803C = 0;
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
                        D_800F803C = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5: {
                    s32 t;

                    t = rsin(++D_800F803C * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(t / 4096) - 150;
                }
                    gPlayer.x += 50;
                    if (D_800F803C < 7) {
                        gView[3] = gRefView[3] + rand() % 100 - 50;
                        gView[4] = gRefView[4] + rand() % 100 - 50;
                        gView[5] = gRefView[5] + rand() % 100 - 50;
                        gView[0] = gRefView[0] - rand() % 200;
                    } else {
                        *(GsRVIEW2 *)gView = sView;
                    }
                    if (D_800F803C >= 23) {
                        gPlayer.motion[0] = 0x3D;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    v = rsin(++D_800F803C * 20480 / 360) * 400;
                    gPlayer.y = -ABS_GE(v / 4096);
                    gView[0] = -0xA8C;
                    gView[1] = -300;
                    gView[2] = 0;
                    gView[4] = gPlayer.y;
                    gPlayer.x += 50;
                    gView[3] = gPlayer.x;
                    gView[5] = gPlayer.z;
                    if (D_800F803C >= 36) {
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
                        D_800F803C = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (++D_800F803C >= 30) {
                        gPlayer.motion[0] = 0x3E;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800F803C = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (++D_800F803C >= 72) {
                        gPlayer.motion[0] = 0x3B;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800F803C = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    if (++D_800F803C >= 30) {
                        gView[0] = -0xA8C - (D_800F803C - 30) * 12;
                        gView[1] = (D_800F803C - 30) * 5 - 300;
                        gView[2] = 0;
                        gView[3] = gPlayer.x;
                        gView[4] = gPlayer.y - (D_800F803C - 30) * 5;
                        gView[5] = gPlayer.z;
                    }
                    if (D_800F803C >= 60) {
                        PlaySe(0x55);
                        D_800F803C = 30;
                        sSubStep = 30;
                    }
                    break;
                case 30:
                    if (gFrameCount & 1) {
                        D_800F803C++;
                    }
                    sCan.x += D_800F803C;
                    *(GsRVIEW2 *)gView = sView;
                    if (D_800F803C >= 60) {
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
            GsGetLs(sAct3Objs[0].coord2, &D_800F8014);
            GsSetLsMatrix(&D_800F8014);
            GsGetLw(sAct3Objs[0].coord2, &D_800F8014);
            GsSetLightMatrix(&D_800F8014);
            GsSortObject4(&sAct3Objs[0], &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
            sAct3Coords[1].coord.t[0] = sCan.x;
            sAct3Coords[1].coord.t[1] = sCan.y;
            sAct3Coords[1].coord.t[2] = sCan.z;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &D_800F8014);
            GsSetLsMatrix(&D_800F8014);
            GsGetLw(sAct3Objs[1].coord2, &D_800F8014);
            GsSetLightMatrix(&D_800F8014);
            GsSortObject4(&sAct3Objs[1], &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
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
            D_800F8034 = 0;
            D_800F8038 = 0;
            sHurtTimer = 0;
            gMissState = 0;
            gWorldIntroStep = 20;
            break;
        case 20:
            if (ClipF == 0) {
                DrawChaseHud();
            }
            if (D_800F8038 == 0) {
                gView[0] = 700;
                gView[1] = -200;
                gView[2] = 0;
                gView[3] = -500;
                gView[4] = 0;
                gView[5] = 0;
                /* MATCHING: one call per arm; cc1 cross-jumps them. */
                GsSetRefView2((GsRVIEW2 *)gView);
            } else {
                gView[0] = rcos((D_800F8038 << 12) / 360) * (700 - D_800F8038 * 3) / 4096;
                gView[1] = -200;
                gView[2] = rsin((D_800F8038 << 12) / 360) * (700 - D_800F8038 * 3) / 4096;
                gView[3] = -(rcos((D_800F8038 << 12) / 360) * 500 / 4096);
                gView[4] = 0;
                gView[5] = -(rsin((D_800F8038 << 12) / 360) * 500 / 4096);
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
                GsGetLs(sAct3Objs[0].coord2, &D_800F8014);
                GsSetLsMatrix(&D_800F8014);
                GsGetLw(sAct3Objs[0].coord2, &D_800F8014);
                GsSetLightMatrix(&D_800F8014);
                GsSortObject4(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
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
                GsGetLs(sAct3Objs[1].coord2, &D_800F8014);
                GsSetLsMatrix(&D_800F8014);
                GsGetLw(sAct3Objs[1].coord2, &D_800F8014);
                GsSetLightMatrix(&D_800F8014);
                GsSortObject4(&sAct3Objs[1], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            }
            i = 0;
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 40000);
            /* MATCHING: i cleared before the call, the base taken before the loop. */
            o = sObjs;
            for (; i < D_800F7A54; i++) {
                if (o[i].state != -1) {
                    D_800F7A60[(s16)o[i].kind](&o[i]);
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
                D_800F8034 = 1;
            }
            if (D_800F8034 != 0) {
                if (++D_800F8034 == 90) {
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
            if (gPlayer.x > 40000 && D_800F8038 == 0) {
                gMissState = 1;
                ClipF = 1;
                D_800F8038 = 1;
            }
            if (D_800F8038 != 0) {
                D_800F8038++;
                if (gPlayer.z > 15) {
                    gPlayer.z -= 15;
                } else if (gPlayer.z < -15) {
                    gPlayer.z += 15;
                } else {
                    gPlayer.z = 0;
                }
                if (D_800F8038 > 60) {
                    D_800F803C = 0;
                    gWorldIntroStep = 50;
                }
            }
            ResetSpadStack();
            break;
        case 50:
            if (D_800F803C > 160) {
                D_800F8040 = (POLY_F4 *)GsOUT_PACKET_P;
                setPolyF4(D_800F8040);
                setSemiTrans(D_800F8040, 1);
                D_800F8040->x0 = D_800F8040->x2 = -160;
                D_800F8040->y0 = D_800F8040->y1 = -120;
                D_800F8040->x1 = D_800F8040->x3 = 160;
                D_800F8040->y2 = D_800F8040->y3 = 120;
                D_800F8040->r0 = D_800F8040->g0 = D_800F8040->b0 = (D_800F803C - 160) * 4;
                addPrim(gSubOt[gActiveBuff].org, D_800F8040);
                D_800F8040++;
                GsOUT_PACKET_P = (PACKET *)D_800F8040;
                D_800F8044 = (DR_MODE *)D_800F8040;
                setlen(D_800F8044, 1);
                D_800F8044->code[0] = 0xE1000640;
                addPrim(gSubOt[gActiveBuff].org, D_800F8044);
                D_800F8044 = (DR_MODE *)((u8 *)D_800F8044 + 8);
                GsOUT_PACKET_P = (PACKET *)D_800F8044;
            }
            if (D_800F803C < 120) {
                gPlayer.x -= rsin(gPlayer.yaw) * 30 / 4096;
                gPlayer.z -= rcos(gPlayer.yaw) * 30 / 4096;
            }
            gWorldShift[1] = 0;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            if (D_800F803C == 120) {
                for (gLoopI = 0; gLoopI < 20; gLoopI++) {
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0xADD4, 0, 0, gLoopI * 2, rand() % 8,
                                          -(rand() % 30) - 10, rand() % 300 + 100, rand() % 20 - 10);
                }
            }
            if (D_800F803C == 90) {
                gRefView[0] = gView[0] + gPlayer.x;
                gRefView[1] = gView[1] + gPlayer.y;
                gRefView[2] = gView[2] + gPlayer.z;
                gRefView[3] = gView[3] - gView[0];
                gRefView[4] = gView[4] - gView[1];
                gRefView[5] = gView[5] - gView[2];
            }
            /* MATCHING: nested, so cc1 does not merge the two bounds. */
            if (D_800F803C < 150) {
                if (D_800F803C >= 120) {
                    if (D_800F803C == 120) {
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
                } else if (D_800F803C >= 90) {
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
            if (D_800F803C < 52) {
                gPlayer.yaw -= 11;
            }
            D_800F803C++;
            SetChaseLights();
            StepChaseMotion(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(0x1000);
            DrawChaseShadowCentre();
            if (D_800F803C < 150) {
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
            GsGetLs(sAct3Objs[0].coord2, &D_800F8014);
            GsSetLsMatrix(&D_800F8014);
            GsGetLw(sAct3Objs[0].coord2, &D_800F8014);
            GsSetLightMatrix(&D_800F8014);
            GsSortObject4(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            sAct3Coords[1].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[1].coord.t[1] = sCan.y + gWorldShift[1];
            sAct3Coords[1].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[1].vx = 0;
            sCanRot[1].vy = rsin((gFrameCount * 8 - 30) * 4096 / 360) * 10 / 4096 * 4096 / 360;
            sCanRot[1].vz = 0;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &D_800F8014);
            GsSetLsMatrix(&D_800F8014);
            GsGetLw(sAct3Objs[1].coord2, &D_800F8014);
            GsSetLightMatrix(&D_800F8014);
            GsSortObject4(&sAct3Objs[1], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 40000);
            DrawSky(0);
            if (D_800F803C >= 223) {
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

#include "chase_tail.inc"
