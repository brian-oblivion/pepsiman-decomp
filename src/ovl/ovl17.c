/* Stage 5-3's overlay: the act-3 chase of world 5, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 12800000
#define CHASE_PLACEMENTS 90
#define CHASE_VAB 6
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

/* A roller from behind the player, at 60 a frame. */
s32 func_800F0BB8(Act3Obj *o) {
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
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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
s32 func_800F0E80(Act3Obj *o) {
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
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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

/* A roller from behind the player, at 40 a frame. */
s32 func_800F1220(Act3Obj *o) {
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
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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

/* A roller from behind the player, at 80 a frame. */
s32 func_800F14E8(Act3Obj *o) {
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
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 40 < gPlayer.x && gPlayer.x < o->x + 40 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 35 < gPlayer.y && gPlayer.y < o->y + 215) {
                    PlaySe(0x39);
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

/* The frame step's state. */
extern s32 D_800F4B08;
extern s32 D_800F4B0C;
extern s32 D_800F4B10;
extern POLY_F4 *D_800F4B14;
extern DR_MODE *D_800F4B18;
extern MATRIX D_800F4AE8;
extern s32 D_800F454C;
extern s32 (*D_800F4558[])(Act3Obj *o);
extern u8 D_800F4568[];
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
s32 func_800F17B0(void) {
    Act3Obj *o;
    s32 i;

    switch (gWorldIntroStep) {
        case 0:
            gSkyColors = D_800F4568;
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
            gPlayer.motion[0] = 3;
            gPlayer.y = 0;
            gPlayer.yaw = 0;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sCan.x = -0x1658;
            sCan.y = -700;
            sCan.z = 600;
            sCanRot[0].vx = -0xE3;
            sCanRot[0].vy = -0x400;
            sCanRot[0].vz = 0;
            ClipF = 1;
            D_800F4B10 = 0;
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
                        PlaySe(0x13);
                    }
                    gPlayer.z -= 20;
                    if (++D_800F4B10 >= 30) {
                        gView[0] = -0x1658;
                        gView[1] = -300;
                        gView[2] = -300;
                        gView[4] = -100;
                        gView[5] = 1000;
                        gView[3] = -0x1658;
                        D_800F4B10 = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    if (!(gFrameCount & 7)) {
                        PlaySe(0x13);
                    }
                    gPlayer.z -= 30;
                    if (++D_800F4B10 >= 20) {
                        PlaySe(0x21);
                        gPlayer.motion[0] = 0x10;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800F4B10 = 30;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    D_800F4B10 -= 2;
                    gPlayer.z -= D_800F4B10;
                    if (!(gFrameCount & 1)) {
                        SpawnEffect(4, gPlayer.x + rand() % 100 - 50, gPlayer.y + rand() % 50 - 25,
                                    gPlayer.z + rand() % 100 - 50, 0);
                    }
                    if (D_800F4B10 == 0) {
                        D_800F4B10 = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (++D_800F4B10 == 40) {
                        gView[0] = -0x15E0;
                        gView[1] = 100;
                        gView[2] = -100;
                        gView[3] = -0x1658;
                        gView[4] = -500;
                        gView[5] = 600;
                        gPlayer.motion[0] = 0x13;
                        gPlayer.action = ACT_MODEL_LOOP;
                        D_800F4B10 = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (++D_800F4B10 == 30) {
                        sSubStep = 25;
                        D_800F4B10 = 0;
                        D_800F4B0C = sCanRot[0].vy;
                    }
                    break;
                case 25:
                    sCanRot[0].vy = D_800F4B0C + rand() % 113 - 0x38;
                    if (++D_800F4B10 == 60) {
                        gView[1] = -700;
                        gView[4] = 0;
                        sCan.y = -700;
                        gPlayer.motion[0] = 0x3F;
                        gPlayer.action = ACT_MODEL_ONCE;
                        sCanRot[0].vy = D_800F4B0C;
                        D_800F4B10 = 0;
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
                    if (++D_800F4B10 == 30) {
                        gPlayer.yaw = -0x400;
                        gPlayer.motion[0] = 2;
                        gPlayer.action = ACT_MODEL_LOOP;
                        D_800F4B10 = 0;
                        sSubStep = 35;
                    }
                    break;
                case 35:
                    gView[1] += 4;
                    if (!(gFrameCount & 7)) {
                        PlaySe(0x13);
                    }
                    gPlayer.x += 30;
                    if (++D_800F4B10 == 20) {
                        D_800F4B10 = 0;
                        sSubStep = 40;
                    }
                    break;
                case 40:
                    sCan.y += 50;
                    sCanRot[0].vy += 0x22;
                    if (++D_800F4B10 == 14) {
                        if (gRumbleEnabled) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 200;
                            gRumbleSmall[1] = 200;
                        }
                        sView = *(GsRVIEW2 *)gView;
                        PlaySe(0x39);
                        D_800F4B10 = 0;
                        sSubStep = 45;
                    }
                    break;
                case 45:
                    sCan.x += 30;
                    sCan.y -= 30;
                    sCanRot[0].vx += 0x38;
                    sCanRot[0].vy += 0x22;
                    if (D_800F4B10 < 4) {
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
                    if (++D_800F4B10 == 30) {
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
            GsGetLs(sAct3Objs[0].coord2, &D_800F4AE8);
            GsSetLsMatrix(&D_800F4AE8);
            GsGetLw(sAct3Objs[0].coord2, &D_800F4AE8);
            GsSetLightMatrix(&D_800F4AE8);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
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
            D_800F4B08 = 0;
            D_800F4B0C = 0;
            sHurtTimer = 0;
            gMissState = 0;
            gWorldIntroStep = 20;
            break;
        case 20:
            if (ClipF == 0) {
                DrawChaseHud();
            }
            if (D_800F4B0C == 0) {
                gView[0] = 700;
                gView[1] = -200;
                gView[2] = 0;
                gView[3] = -500;
                gView[4] = 0;
                gView[5] = 0;
                /* MATCHING: one call per arm; cc1 cross-jumps them. */
                GsSetRefView2((GsRVIEW2 *)gView);
            } else {
                gView[0] = rcos((D_800F4B0C << 12) / 360) * (700 - D_800F4B0C * 3) / 4096;
                gView[1] = -200;
                gView[2] = rsin((D_800F4B0C << 12) / 360) * (700 - D_800F4B0C * 3) / 4096;
                gView[3] = -(rcos((D_800F4B0C << 12) / 360) * 500 / 4096);
                gView[4] = 0;
                gView[5] = -(rsin((D_800F4B0C << 12) / 360) * 500 / 4096);
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
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            if (gPlayer.x + 500 > sCan.x + 120) {
                SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
                GsGetLs(sAct3Objs[0].coord2, &D_800F4AE8);
                GsSetLsMatrix(&D_800F4AE8);
                GsGetLw(sAct3Objs[0].coord2, &D_800F4AE8);
                GsSetLightMatrix(&D_800F4AE8);
                SetTmdSortMode(0);
                GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            }
            i = 0;
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 50000);
            /* MATCHING: i cleared before the call, the base taken before the loop. */
            o = sObjs;
            for (; i < D_800F454C; i++) {
                if (o[i].state != -1) {
                    D_800F4558[(s16)o[i].kind](&o[i]);
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
                D_800F4B08 = 1;
            }
            if (D_800F4B08 != 0) {
                if (++D_800F4B08 == 90) {
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
            if (gPlayer.x > 50000 && D_800F4B0C == 0) {
                gMissState = 1;
                ClipF = 1;
                D_800F4B0C = 1;
            }
            if (D_800F4B0C != 0) {
                D_800F4B0C++;
                if (gPlayer.z > 15) {
                    gPlayer.z -= 15;
                } else if (gPlayer.z < -15) {
                    gPlayer.z += 15;
                } else {
                    gPlayer.z = 0;
                }
                if (D_800F4B0C > 60) {
                    D_800F4B10 = 0;
                    gWorldIntroStep = 50;
                }
            }
            ResetSpadStack();
            break;
        case 50:
            if (D_800F4B10 > 160) {
                D_800F4B14 = (POLY_F4 *)GsOUT_PACKET_P;
                setPolyF4(D_800F4B14);
                setSemiTrans(D_800F4B14, 1);
                D_800F4B14->x0 = D_800F4B14->x2 = -160;
                D_800F4B14->y0 = D_800F4B14->y1 = -120;
                D_800F4B14->x1 = D_800F4B14->x3 = 160;
                D_800F4B14->y2 = D_800F4B14->y3 = 120;
                D_800F4B14->r0 = D_800F4B14->g0 = D_800F4B14->b0 = (D_800F4B10 - 160) * 4;
                addPrim(gSubOt[gActiveBuff].org, D_800F4B14);
                D_800F4B14++;
                GsOUT_PACKET_P = (PACKET *)D_800F4B14;
                D_800F4B18 = (DR_MODE *)D_800F4B14;
                setlen(D_800F4B18, 1);
                D_800F4B18->code[0] = 0xE1000640;
                addPrim(gSubOt[gActiveBuff].org, D_800F4B18);
                D_800F4B18 = (DR_MODE *)((u8 *)D_800F4B18 + 8);
                GsOUT_PACKET_P = (PACKET *)D_800F4B18;
            }
            if (D_800F4B10 < 120) {
                if (gFrameCount % 36 == 0 || gFrameCount % 36 == 18) {
                    PlaySe(0x39);
                }
                gPlayer.x -= rsin(gPlayer.yaw) * 30 / 4096;
                gPlayer.z -= rcos(gPlayer.yaw) * 30 / 4096;
            }
            gWorldShift[1] = 0;
            gWorldShift[0] = -gPlayer.x;
            gWorldShift[2] = -gPlayer.z;
            if (D_800F4B10 == 120) {
                for (gLoopI = 0; gLoopI < 20; gLoopI++) {
                    SpawnEffectWithParams(EFFECT_FLYING_PIECE, 0xD4E4, 0, 0, gLoopI * 2, 0,
                                          -(rand() % 30) - 10, rand() % 300 + 100, rand() % 20 - 10);
                }
            }
            if (D_800F4B10 == 90) {
                gRefView[0] = gView[0] + gPlayer.x;
                gRefView[1] = gView[1] + gPlayer.y;
                gRefView[2] = gView[2] + gPlayer.z;
                gRefView[3] = gView[3] - gView[0];
                gRefView[4] = gView[4] - gView[1];
                gRefView[5] = gView[5] - gView[2];
            }
            /* MATCHING: nested, so cc1 does not merge the two bounds. */
            if (D_800F4B10 < 150) {
                if (D_800F4B10 >= 120) {
                    if (D_800F4B10 == 120) {
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
                } else if (D_800F4B10 >= 90) {
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
            if (D_800F4B10 < 52) {
                gPlayer.yaw -= 11;
            }
            D_800F4B10++;
            SetChaseLights();
            StepChaseMotion(&gPlayer);
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            SetTmdSortMode(gTmdSortMode);
            DrawChasePlayerScaled(0x1000);
            DrawChaseShadowCentre();
            if (D_800F4B10 < 150) {
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
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &D_800F4AE8);
            GsSetLsMatrix(&D_800F4AE8);
            GsGetLw(sAct3Objs[0].coord2, &D_800F4AE8);
            GsSetLightMatrix(&D_800F4AE8);
            SetTmdSortMode(4);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 50000);
            DrawSky(0);
            if (D_800F4B10 >= 223) {
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

#include "chase_tail.inc"
