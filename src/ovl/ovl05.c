/* Stage 1-3's overlay: the act-3 chase, where the player runs toward the
 * camera, jumps and steps sideways, and the stage's own objects come at
 * it. */

#define CHASE_LENGTH 7680000
#define CHASE_PLACEMENTS 103
#define CHASE_VAB 2
#define CHASE_REACH 120
#define CHASE_LIGHT_BLUE 0x80
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0

#include "chase.inc"
#include "rand.h"
#include "spad.h"
#include "prim.h"

/* Starts a jump on a face button, unless the play is frozen. */
#ifdef NON_MATCHING
void StartChaseJump(void) {
    if (ClipF == 0 && (gGamePadTrig & (PADRleft | PADRdown | PADRright))) {
        sJumpSpeed = -40;
        gPlayer.slideState = 1;
        gPlayer.unk38C = 0;
        gPlayer.curMotion[0] = MOT_OFF;
        PlaySe(0x17);
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl05/nonmatchings/ovl05", StartChaseJump);
#endif

#include "chase_head.inc"

#include "chase_pieces.inc"

/* The frame step's state. */
extern s32 D_800F80E0;
extern s32 D_800F80E4;
extern s32 D_800F80E8;
extern POLY_F4 *D_800F80EC;
extern DR_MODE *D_800F80F0;
extern MATRIX D_800F80C0;
extern VECTOR D_800F8100;
extern s32 D_800F8110;
extern s32 D_800F7B00;
extern s32 (*D_800F7B0C[])(Act3Obj *o);
extern u8 D_800F7B40[];
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
s32 SpawnEffectWithParams(s32 kind, s16 x, s16 y, s16 z, s16 delay, s16 param0, s16 param1,
                          s16 param2, s16 param3);

#define ABS(x) ((x) < 0 ? -(x) : (x))
/* MATCHING: the step's sub-step, kept in the fog fade's word. */
#define sSubStep gFogFade

/* The stage's frame step: the opening scene, the chase, the escape at the
 * run's end, and the results. */
#ifdef NON_MATCHING
void func_800F4794(void) {
    Act3Obj *o;
    s32 i;
    s32 v;
    s32 r;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = D_800F7B40;
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].flg = 0;
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
            D_800F80E8 = 0;
            sSubStep = 0;
            sHurtTimer = 0;
            gMissState = 1;
            gWorldIntroStep = 1;
            break;
        case 1:
            DrawFrozenFrame();
            switch (sSubStep) {
                case 0:
                    if (++D_800F80E8 >= 30) {
                        D_800F8110 = 1;
                        D_800F80E8 = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    gView[0] = -0x157C;
                    gView[1] = -250;
                    sCan.lidZ += 30;
                    gView[2] = sCan.lidZ - (D_800F80E8 * 5 - 700);
                    gView[4] = sCan.lidY;
                    gView[5] = sCan.lidZ;
                    gView[3] = sCan.lidX;
                    if (D_800F80E8 % 10 == 0) {
                        PlaySe(0x54);
                    }
                    if (++D_800F80E8 >= 30) {
                        sCan.lidZ = -2000;
                        D_800F80E8 = 0;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    gView[0] = -0x157C;
                    gView[1] = -200;
                    gView[3] = -0x157C;
                    gView[4] = -150;
                    gView[5] = D_800F80E8 * 30 - 500;
                    gView[2] = D_800F80E8 * 30 - 1000;
                    if (D_800F80E8 == 10) {
                        PlaySe(0x58);
                    }
                    if (++D_800F80E8 >= 25) {
                        sCan.lidZ = -500;
                        D_800F80E8 = 0;
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
                    if (++D_800F80E8 >= 10) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x38);
                        PlaySe(0x34);
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200 - 100, -150 - rand() % 50,
                                        gPlayer.z - 100 + rand() % 200, rand() % 4);
                        }
                        gPlayer.motion[0] = 0x3C;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800F80E4 = 30;
                        D_800F80E8 = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (D_800F80E4 != 0) {
                        D_800F80E4 -= 5;
                    }
                    sCan.lidZ += D_800F80E4;
                    gPlayer.x -= 6;
                    gPlayer.y -= 0x38;
                    gPlayer.z += 60;
                    if (++D_800F80E8 >= 7) {
                        gPlayer.x = -0x1518;
                        gPlayer.y = -1200;
                        gPlayer.z = 1000;
                        D_800F80E8 = 0;
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
                    if (++D_800F80E8 >= 10) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x39);
                        for (gLoopI = 0; gLoopI < 8; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + rand() % 200 - 100, gPlayer.y + rand() % 200 - 100,
                                        gPlayer.z - 100 + rand() % 200, rand() % 4);
                        }
                        D_800F80E8 = 0;
                        sSubStep = 30;
                        D_800F80E4 = sCanRot[0].vx;
                    }
                    break;
                case 30:
                    gPlayer.x += 60;
                    gPlayer.z -= 60;
                    gPlayer.y += 10;
                    sCanRot[0].vx = D_800F80E4 + rand() % 113 - 0x38;
                    if (++D_800F80E8 >= 30) {
                        D_800F80E8 = 0;
                        sSubStep = 35;
                    }
                    break;
                case 35:
                    sCanRot[0].vx += 0x16;
                    sCanRot[0].vz -= 0x2D;
                    sCan.x += 10;
                    sCan.z -= 10;
                    sCan.y = rsin((D_800F80E8 << 12) / 360) * 500 / 4096 - 0x514;
                    if (++D_800F80E8 >= 25) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 10;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        PlaySe(0x3B);
                        D_800F80E8 = 0;
                        sSubStep = 40;
                    }
                    break;
                case 40:
                    sCanRot[0].vz -= 0x2D;
                    sCan.x += 10;
                    sCan.z -= 10;
                    sCan.y = -(rsin(D_800F80E8 * 20480 / 360) * 300 / 4096) - 0x44C;
                    if (++D_800F80E8 >= 30) {
                        sCan.x = -0x1068;
                        sCan.z = 0;
                        sCan.y = 0;
                        sCanRot[0].vx = 0x800;
                        D_800F8110 = 0;
                        D_800F80E8 = 18;
                        sCanRot[0].vy = 0;
                        sCanRot[0].vz = 0;
                        sSubStep = 45;
                    }
                    break;
                case 45:
                    if (D_800F80E8 >= 36 && D_800F80E8 < 40) {
                        gView[0] = -3000;
                        gView[3] = -4000;
                        gView[4] = gView[1] = rand() % 50 - 25;
                        gView[1] -= 300;
                        gView[5] = gView[2] = rand() % 50 - 25;
                    } else {
                        gView[0] = -3000;
                        gView[1] = -300;
                        gView[2] = 0;
                        gView[3] = -4000;
                        gView[4] = 0;
                        gView[5] = 0;
                    }
                    sCan.y = -(ABS(rsin(D_800F80E8 * 20480 / 360)) * 500) / 4096;
                    sCan.x += 30;
                    sCanRot[0].vz -= 0x71;
                    if (++D_800F80E8 == 36) {
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
                    if (D_800F80E8 >= 60) {
                        gWorldIntroStep = 10;
                    }
                    break;
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            SetChaseLights();
            gPlayer.motionResult[0] = StepAnimation(&gPlayer, 0, 0);
            if (!(gFrameCount & 7) && gPlayer.motion[0] == 0x33) {
                PlaySe(0x13);
            }
            switch (gPlayer.action) {
                case ACT_MODEL_ONCE:
                    break;
                case ACT_MODEL_LOOP:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.curMotion[0] = MOT_OFF;
                    }
                    break;
            }
            UpdateEffects((s32)&gSubOt[gActiveBuff], 0, 0, 0);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayer();
            DrawChaseShadow();
            sAct3Coords[0].coord.t[0] = sCan.x;
            sAct3Coords[0].coord.t[1] = sCan.y;
            sAct3Coords[0].coord.t[2] = sCan.z;
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &D_800F80C0);
            if (D_800F8110 != 0) {
                D_800F8100.vx = 0x1800;
                D_800F8100.vy = 0x1800;
                D_800F8100.vz = 0x1800;
                ScaleMatrix(&D_800F80C0, &D_800F8100);
            }
            GsSetLsMatrix(&D_800F80C0);
            GsGetLw(sAct3Objs[0].coord2, &D_800F80C0);
            GsSetLightMatrix(&D_800F80C0);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            sAct3Coords[1].coord.t[0] = sCan.lidX;
            sAct3Coords[1].coord.t[1] = sCan.lidY;
            sAct3Coords[1].coord.t[2] = sCan.lidZ;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &D_800F80C0);
            GsSetLsMatrix(&D_800F80C0);
            GsGetLw(sAct3Objs[1].coord2, &D_800F80C0);
            GsSetLightMatrix(&D_800F80C0);
            GsSortObject4(&sAct3Objs[1], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            DrawRepeatingStage(0, 0, 0, 30000);
            DrawSky(0);
            if (gPadTrig & PADstart) {
                gWorldIntroStep = 10;
            }
            break;
        case 10:
            PlayStageMusic(gStage);
            for (gLoopI = 10; gLoopI < 50; gLoopI++) {
                sPieceCoords[gLoopI].flg = 0;
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
            D_800F80E0 = 0;
            D_800F80E4 = 0;
            sHurtTimer = 0;
            gMissState = 0;
            gWorldIntroStep = 20;
            break;
        case 20:
            if (ClipF == 0) {
                DrawChaseHud();
            }
            if (D_800F80E4 == 0) {
                gView[0] = 700;
                gView[1] = -200;
                gView[2] = 0;
                gView[3] = -500;
                gView[4] = 0;
                gView[5] = 0;
            } else {
                gView[0] = rcos((D_800F80E4 << 12) / 360) * (700 - D_800F80E4 * 3) / 4096;
                gView[1] = -200;
                gView[2] = rsin((D_800F80E4 << 12) / 360) * (700 - D_800F80E4 * 3) / 4096;
                gView[3] = -(rcos((D_800F80E4 << 12) / 360) * 500 / 4096);
                gView[4] = 0;
                gView[5] = -(rsin((D_800F80E4 << 12) / 360) * 500 / 4096);
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            StepChasePlayer();
            SetChaseLights();
            gWorldShift[1] = 0;
            gPlayer.x += gPlayer.speed;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            SetSpadStack();
            gPlayer.motionResult[0] = StepAnimation(&gPlayer, 0, 0);
            if (!(gFrameCount & 7) && gPlayer.motion[0] == 0x33) {
                PlaySe(0x13);
            }
            switch (gPlayer.action) {
                case ACT_MODEL_ONCE:
                    break;
                case ACT_MODEL_LOOP:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.curMotion[0] = MOT_OFF;
                    }
                    break;
            }
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            ResetSpadStack();
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(sChaseLive);
            DrawChaseShadowCentre();
            v = sCan.x;
            sCan.x = v + 30;
            if (v + 650 < gPlayer.x) {
                sCan.x = gPlayer.x - 620;
            }
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[0].coord.t[1] =
                sCan.y + gWorldShift[1] - ABS(rsin(gFrameCount * 40960 / 360) * 100 / 4096);
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            if (gFrameCount % 36 == 0 || gFrameCount % 36 == 18) {
                PlaySe(0x39);
            }
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            if (sCan.x + 120 < gPlayer.x + 500) {
                SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
                GsGetLs(sAct3Objs[0].coord2, &D_800F80C0);
                GsSetLsMatrix(&D_800F80C0);
                GsGetLw(sAct3Objs[0].coord2, &D_800F80C0);
                GsSetLightMatrix(&D_800F80C0);
                SetTmdSortMode(0);
                GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            }
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
            for (i = 0, o = sObjs; i < D_800F7B00; i++, o++) {
                if (o->state != -1) {
                    D_800F7B0C[o->kind](o);
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
                D_800F80E0 = 1;
            }
            if (D_800F80E0 != 0) {
                if (++D_800F80E0 == 90) {
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
            if (gPlayer.x > 30000 && D_800F80E4 == 0) {
                gMissState = 1;
                ClipF = 1;
                D_800F80E4 = 1;
            }
            if (D_800F80E4 != 0) {
                D_800F80E4++;
                if (gPlayer.z > 15) {
                    gPlayer.z -= 15;
                } else if (gPlayer.z < -15) {
                    gPlayer.z += 15;
                } else {
                    gPlayer.z = 0;
                }
                if (D_800F80E4 > 60) {
                    D_800F80E8 = 0;
                    gWorldIntroStep = 50;
                }
            }
            break;
        case 50:
            if (D_800F80E8 > 160) {
                D_800F80EC = (POLY_F4 *)GsOUT_PACKET_P;
                setPolyF4(D_800F80EC);
                setSemiTrans(D_800F80EC, 1);
                setXY4(D_800F80EC, -160, -120, 160, -120, -160, 120, 160, 120);
                setRGB0(D_800F80EC, (D_800F80E8 - 160) * 4, (D_800F80E8 - 160) * 4,
                        (D_800F80E8 - 160) * 4);
                addPrim(gSubOt[gActiveBuff].org, D_800F80EC);
                D_800F80EC++;
                GsOUT_PACKET_P = (PACKET *)D_800F80EC;
                D_800F80F0 = (DR_MODE *)D_800F80EC;
                setlen(D_800F80F0, 1);
                D_800F80F0->code[0] = 0xE1000640;
                addPrim(gSubOt[gActiveBuff].org, D_800F80F0);
                D_800F80F0 = (DR_MODE *)((u8 *)D_800F80F0 + 8);
                GsOUT_PACKET_P = (PACKET *)D_800F80F0;
            }
            if (D_800F80E8 < 120) {
                if (gFrameCount % 36 == 0 || gFrameCount % 36 == 18) {
                    PlaySe(0x39);
                }
                gPlayer.x -= rsin(gPlayer.yaw) * 30 / 4096;
                gPlayer.z -= rcos(gPlayer.yaw) * 30 / 4096;
            }
            gWorldShift[1] = 0;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            if (D_800F80E8 == 120) {
                for (gLoopI = 0; gLoopI < 20; gLoopI++) {
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0x86C4, 0, 0, gLoopI * 2, rand() % 8,
                                          -(rand() % 30) - 10, rand() % 300 + 100, rand() % 20 - 10);
                }
            }
            if (D_800F80E8 == 90) {
                gRefView[0] = gView[0] + gPlayer.x;
                gRefView[1] = gView[1] + gPlayer.y;
                gRefView[2] = gView[2] + gPlayer.z;
                gRefView[3] = gView[3] - gView[0];
                gRefView[4] = gView[4] - gView[1];
                gRefView[5] = gView[5] - gView[2];
            }
            if (D_800F80E8 < 150 && D_800F80E8 >= 120) {
                if (D_800F80E8 == 120) {
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
            } else if (D_800F80E8 >= 90) {
                gView[0] = gRefView[0] - gPlayer.x;
                gView[1] = gRefView[1] - gPlayer.y;
                gView[2] = gRefView[2] - gPlayer.z;
                gView[3] = gRefView[3] + gView[0];
                gView[4] = gRefView[4] + gView[1];
                gView[5] = gRefView[5] + gView[2];
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            if (D_800F80E8 < 52) {
                gPlayer.yaw -= 11;
            }
            D_800F80E8++;
            SetChaseLights();
            gPlayer.motionResult[0] = StepAnimation(&gPlayer, 0, 0);
            if (!(gFrameCount & 7) && gPlayer.motion[0] == 0x33) {
                PlaySe(0x13);
            }
            switch (gPlayer.action) {
                case ACT_MODEL_ONCE:
                    break;
                case ACT_MODEL_LOOP:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.curMotion[0] = MOT_OFF;
                    }
                    break;
            }
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(0x1000);
            DrawChaseShadowCentre();
            if (D_800F80E8 < 150) {
                sCan.x += 30;
                SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            }
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[0].coord.t[1] =
                sCan.y + gWorldShift[1] - ABS(rsin(gFrameCount * 40960 / 360) * 100 / 4096);
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &D_800F80C0);
            GsSetLsMatrix(&D_800F80C0);
            GsGetLw(sAct3Objs[0].coord2, &D_800F80C0);
            GsSetLightMatrix(&D_800F80C0);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
            DrawSky(0);
            if (D_800F80E8 >= 223) {
                ClipF = 0;
                StopCdTrack();
                if (gStageTableSet >= 0) {
                    if (gStageTableSet < 3) {
                        StartWorldResults();
                        gWorldIntroStep = 60;
                    } else if (gStageTableSet == 3) {
                        gGameMode = 6;
                        gModeStep = 0;
                    }
                }
            }
            break;
        case 60:
            UpdateWorldResults();
            if (IsResultsActive() == 0 && gStageTableSet >= 0) {
                if (gStageTableSet < 2) {
                    gStage = STAGE_2_1;
                    gGameMode = 0x2C;
                } else if (gStageTableSet < 4) {
                    gGameMode = 6;
                    gModeStep = 0;
                }
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl05/nonmatchings/ovl05", func_800F4794);
#endif


#include "chase_tail.inc"
