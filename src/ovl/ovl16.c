/* Stage 5-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

/* This overlay's data, in address order: the engine's (engine_data.inc)
 * around its own. */
#define ENGINE_SPEEDS_PAD 0x1324
#define ENGINE_HINT_FILE

#include "engine_data.inc" /* sFiles1BootLoc to sFiles1LastSectors */

/* The goal's three lights, lit one by one as the goal sequence counts down;
 * CycleGoalLights draws a lit one's other pair of frames. */
s8 sGoalLightLit[3] = {0};

#include "engine_data.inc" /* sBurstDirs to sType1FSpeeds */

#include "engine_data.inc" /* sFiles2BootLoc on, sHintBurstSpins on */

/* The stage-hint file's data: the hint being watched, the hints and their
 * count. Nothing in this overlay reads them. */
s32 sHintIndex = 0;
StageHint sHints[12] = {
    {16, 30, 2},  {27, 30, 2},  {30, 30, 2},  {45, 30, 2},  {58, 30, 2},  {129, 30, 1},
    {208, 30, 2}, {260, 30, 2}, {267, 30, 2}, {271, 30, 2}, {276, 30, 2}, {-1},
};
s32 sHintCount = 12;

/* MATCHING: the word retail's link padded the file's data with, whatever
 * its buffer held. */
s32 sHintPad = 0x2E22ED1C;

#include "engine_data.inc" /* sWalkPose to sGoalTraps */

/* The view a camera shot (a fall's, the goal's) moves the camera to. */
s32 sShotView[8] = {0};

/* 400 zeroed bytes nothing in this overlay reads. */
s32 sUnusedWords[100] = {0};

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

void ResetSpawnedTraps(void) {
    u32 i;
    StageTrap *t;
    ModelSlot *a;
    ModelSlot *b;
    ObjectModel *m;

    if (gSkipLandRumble == 1 || sSpawnedTrapsReady == 0) {
        bzero((u8 *)sSpawnedTraps, 200 * sizeof(StageTrap));
        for (i = 0; i < 200; i++) {
            sSpawnedTraps[i].switchNo = -1;
            sSpawnedTraps[i].model = -1;
            sSpawnedTraps[i].state = 0;
            sSpawnedTraps[i].timer = 0;
        }
        for (i = 0; i < 10; i++) {
            D_800D39D8[i].model = -1;
            D_800D39D8[i].state = 0;
            D_800D39D8[i].timer = 0;
        }
        for (i = 0; i < 2; i++) {
            sGoalTraps[i].model = -1;
            sGoalTraps[i].state = 0;
            sGoalTraps[i].timer = 0;
        }
        sSpawnedTrapsReady = 1;
        gSlotTable = (s32)sSpawnedTraps;
        gSlotTableSize = 200 * sizeof(StageTrap);
        m = gObjectModels;
        t = (StageTrap *)gTraps;
        a = (ModelSlot *)&m[t[194].model];
        b = (ModelSlot *)&m[t[195].model];
        b->scale.vy = a->scale.vy = b->scale.vx = a->scale.vx = 0x1000;
    }
}

#include "engine_code.inc" /* StepCheckpoint */

#include "engine/step_flung_piece.inc"

#include "engine/spawn_miss_debris.inc"

#include "engine/step_miss_debris.inc"

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {
    s16 dir;

    dir = 1;
    if (mode == 2) {
        dir = -1;
    }
    if (t->state == 0) {
        if (LatchProximity(t, 2000) != 0) {
            PlaySe(82);
        }
        SnapToFloor(t);
        if (t->timer < 150 && speed != 0) {
            t->timer++;
            if (mode == 0 || mode == 2) {
                t->rotY = GetPathDirection(t) + dir * 2048;
            }
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
        }
        if (mode == 1 && DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 250000) {
            t->state++;
        }
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x39);
    }
}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->state++;
        case 1:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 2:
            if (t->homeRotZ == 0) {
                PolarToXZ((s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            } else {
                PolarToXZ(-(s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            }
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (LatchProximity(t, 1500) != 0) {
                PlaySe(0x84);
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x39);
            }
            if (dir == 1 && arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->state--;
                t->homeRotZ ^= 1;
            }
            break;
    }
}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->state++;
        case 1:
            if ((s8)arg != 1) {
                if ((s8)arg == 2) {
                    if (t->homeRotZ != 0) {
                        PolarToXZ(-10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
                    } else {
                        PolarToXZ(10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
                    }
                }
            } else if (t->homeRotZ == 0) {
                PolarToXZ(-10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
            } else {
                PolarToXZ(10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
            }
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (++t->timer >= 26) {
                t->timer = 0;
                t->state++;
                PlaySe(0x83);
            }
            break;
        case 2:
            t->timer++;
            switch (arg2) {
                case 0:
                    t->homeRotZ ^= 1;
                    t->timer = 0;
                    t->state--;
                    break;
                case 1:
                    if (t->timer >= 16) {
                        t->homeRotZ ^= 1;
                        t->timer = 0;
                        t->state--;
                    }
                    break;
                case 2:
                    if (t->homeRotZ != 0 || t->timer >= 31) {
                        t->homeRotZ ^= 1;
                        t->timer = 0;
                        t->state--;
                    }
                    break;
            }
            break;
        default:
            return;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x36);
    }
}

#include "engine/step_trap_type27.inc"

#include "engine/knock_trap_away.inc"

#include "engine/tumble_trap_away.inc"

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

#include "engine/spin_trap_away.inc"

#include "engine_code.inc" /* TipTrapAway */

#include "engine/burst_raised_trap.inc"

#include "engine/topple_trap_on_approach.inc"

#include "engine/scatter_trap_dust.inc"

#include "engine/swap_trap_model_on_approach.inc"

void StepMissSequence(void) {
    s32 unused[18];
    StageTrap *m;

    m = &D_800AC6E0;
    if (D_800D3638 == 1) {
    top:
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = 0;
                gPlayer.motion[0] = 4;
                StopSpeed();
                EndJump();
                gMissState = -1;
                D_800D39D8[0].homeRotZ = 0;
                gFellOff = 1;
                m->timer = 0;
                D_800D39D8[0].homeX = gPlayer.x;
                D_800D39D8[0].homeY = gPlayer.y;
                D_800D39D8[0].homeZ = gPlayer.z;
                gRefView[0] = gView[0];
                gRefView[1] = gView[1];
                gRefView[2] = gView[2];
                m->state++;
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, D_8007AD68[gStage].x, m->timer, 20);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, D_8007AD68[gStage].z, m->timer, 20);
                gRefView[0] -= gPlayer.x + gWorldShift[0];
                gRefView[1] = gView[1];
                gRefView[2] -= gPlayer.z + gWorldShift[2];
                gRefView[3] = gView[3];
                gRefView[4] = gView[4];
                gRefView[5] = gView[5];
                GsSetRefView2((GsRVIEW2 *)gRefView);
                if (++m->timer == 10) {
                    gPlayer.motion[0] = 11;
                    gPlayer.action = 0;
                    gPlayer.jumpPower = 40;
                }
                if (m->timer >= 21) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                if (gPlayer.y >= gPlayer.groundY) {
                    gPlayer.rideMode = (gPlayer.rideMode & 0xF0) | 4;
                    SpawnMissDebris();
                    gSpeedFixed = 800;
                    D_800D3638 = 2;
                    gMissState = 0;
                    gFellOff = 0;
                    gPlayer.control = 1;
                }
                break;
        }
        return;
        /* MATCHING: unreachable; its label splits the cse block at the switch. */
        for (;;) {
            if (m->state != 0) {
                goto top;
            }
        }
    }
}

/* This stage's goal walk, defined below. */
s8 func_800F60CC(StageTrap *t);

void StepGoalSequence(void) {
    StageTrap *t;
    StageTrap *u;

    t = sGoalTraps;
    u = &sGoalTraps[1];
    if (D_800D37A8 == 1) {
        ResetRumble();
        DrawFrozenFrame();
        switch (t->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = 0;
                gPlayer.motion[0] = 4;
                StopSpeed();
                gMissState = -2;
                ClipF = 1;
                t->timer = 0;
                u->homeX = gPlayer.x;
                u->homeY = gPlayer.y;
                u->homeZ = gPlayer.z;
                t->state++;
            case 1:
                if (func_800F60CC(u)) {
                    t->state++;
                    StartActResults();
                }
                break;
            case 2:
                if (IsResultsActive() == 0) {
                    t->state++;
                }
                break;
            case 3:
                t->state++;
                break;
            case 4:
                if (gInEditor != 1) {
                    if (gStageTableSet != 3) {
                        t->timer = 0;
                        t->state++;
                    } else {
                        gGameMode = 6;
                        gModeStep = 0;
                        ClipF = 0;
                    }
                }
                break;
            case 5:
                if (++t->timer >= 9) {
                    gStage = GetNextStage(13);
                    BeginAct3LoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

void func_800F8B4C(DirEnt16 *dir, u16 first, u16 count);

void CycleGoalLights(s8 arg) {
    u16 periods[8] = {30, 30, 30, 30, 20, 20, 10, 10};
    s8 lights[8] = {0, 0, 0, 0, 1, 1, 2, 2};
    s16 i;

    if (gSectionEdges[-1] - gSection < 20) {
        for (i = 0; i < 8; i++) {
            if (gFrameCount % periods[i] == 0) {
                func_800F8B4C((DirEnt16 *)0x8018D000, i * 4 + sGoalLightLit[lights[i]] * 2, 1);
            } else if (gFrameCount % periods[i] == periods[i] >> 1) {
                func_800F8B4C((DirEnt16 *)0x8018D000, i * 4 + 1 + sGoalLightLit[lights[i]] * 2, 1);
            }
        }
    }
}

void func_800F8BD4(void);

/* MATCHING: an unused 32-byte local gives retail's frame. */
void func_800F5E80(void) {
    s32 unused[8];
    s32 v;

    if (*(s8 *)&gFellOff == 0) {
        gCourse.rot.vy = UpdateCamera(gPlayer.x, gPlayer.y, gPlayer.z, gFloorY[0]) + 0x800;
    }
    gPlayer.editorY = gPlayer.groundY;
    v = FindFloorNear(&gPlayer.x, gFloorY, gFloorIndex);
    if (v != -1) {
        gFloorIndex = v;
        gStageParams = *(s32 *)&gFloorY[4];
    }
    gPlayer.groundY = gFloorY[0];
    SetSpadStack();
    UpdatePlayer();
    UpdateStageLights();
    if (*(s8 *)&gFellOff == 0) {
        PushPlayerFromWalls();
    }
    DrawModeObject();
    DrawPlayerShadow();
    TestBoxes();
    PairMarks();
    ResetCollision();
    UpdateStageEvents();
    UpdateStageTraps();
    UpdateStageSprites();
    ReactToHits(&gPlayer);
    if (*(s8 *)&gPlayLive == 1) {
        UpdatePickups(&gSubOt[gActiveBuff]);
    }
    LeaveQuadTop();
    UpdatePlayerAction(&gPlayer);
    ResetSpadStack();
    DrawTodObjects();
    UnusedHook2();
    PopInNextCan();
    if (*(s8 *)&gPlayLive == 1) {
        DrawStage(-gPlayer.x, 0, -gPlayer.z);
        DrawSky(gStageParams);
    }
    UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
    func_800F8BD4();
}

extern Vec3 D_8007AE1C[];
extern Vec3 D_8007AED0[];

/* MATCHING: frame counts as s16 locals set at the top keep saved registers. */
s8 func_800F60CC(StageTrap *t) {
    s8 ret = 0;
    s16 n = 20;
    s16 m = 40;

    switch (t->state) {
        case 0:
            gPlayer.x = Lerp(t->homeX, D_8007AE1C[gStage].x, t->timer, n);
            gPlayer.z = Lerp(t->homeZ, D_8007AE1C[gStage].z, t->timer, n);
            if (++t->timer > n) {
                t->homeX = gPlayer.x;
                t->homeY = gPlayer.y;
                t->homeZ = gPlayer.z;
                t->timer = 0;
                t->state++;
                gPlayer.action = 0x17;
                SetSoundFade(4);
            }
            break;
        case 1:
            gPlayer.x = Lerp(gPlayer.x, D_8007AED0[gStage].x, t->timer, m);
            gPlayer.z = Lerp(gPlayer.z, D_8007AED0[gStage].z, t->timer, m);
            if (++t->timer < (s16)(m / 3)) {
                ScatterDust((VECTOR *)&gPlayer.x);
            }
            if (t->timer > m) {
                gPlayer.action = 0;
                gPlayer.motion[0] = 0x13;
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            if (++t->timer >= 16) {
                gPlayer.action = 0x19;
                gFellOff = 1;
                sShotView[0] = rsin(gCourse.rot.vy + 0x8E3) * 200 >> 12;
                sShotView[2] = rcos(gCourse.rot.vy + 0x8E3) * 200 >> 12;
                sShotView[1] = gPlayer.y - 80;
                sShotView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                sShotView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                sShotView[4] = gPlayer.y - 130;
                t->homeRotZ = 0;
                gRefView[0] = rsin(gCourse.rot.vy + 0x8E3) * 30 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x8E3) * 30 >> 12;
                gRefView[1] = gPlayer.y - 100;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[4] = gPlayer.y - 130;
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            gView[0] = Lerp(gRefView[0], sShotView[0], t->timer, 20);
            gView[1] = Lerp(gRefView[1], sShotView[1], t->timer, 20);
            gView[2] = Lerp(gRefView[2], sShotView[2], t->timer, 20);
            gView[3] = Lerp(gRefView[3], sShotView[3], t->timer, 20);
            gView[4] = Lerp(gRefView[4], sShotView[4], t->timer, 20);
            gView[5] = Lerp(gRefView[5], sShotView[5], t->timer, 20);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (t->timer < 20) {
                t->timer++;
            }
            if (t->timer == 15) {
                PlaySe(0x22);
            }
            if (++t->homeRotZ > 20) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 4:
            if (++t->timer > 20) {
                gPlayer.curMotion[0] = 0xFF;
                gPlayer.action = 0x19;
                gFellOff = 1;
                sShotView[0] = rsin(gCourse.rot.vy + 0x71D) * 200 >> 12;
                sShotView[2] = rcos(gCourse.rot.vy + 0x71D) * 200 >> 12;
                sShotView[1] = gPlayer.y - 200;
                sShotView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                sShotView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                sShotView[4] = gPlayer.y - 130;
                gRefView[0] = rsin(gCourse.rot.vy + 0x71D) * 30 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x71D) * 30 >> 12;
                gRefView[1] = gPlayer.y - 150;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[4] = gPlayer.y - 130;
                t->homeRotZ = 0;
                t->timer = 0;
                t->state++;
            }
            break;
        case 5:
            gView[0] = Lerp(gRefView[0], sShotView[0], t->timer, 20);
            gView[1] = Lerp(gRefView[1], sShotView[1], t->timer, 20);
            gView[2] = Lerp(gRefView[2], sShotView[2], t->timer, 20);
            gView[3] = Lerp(gRefView[3], sShotView[3], t->timer, 20);
            gView[4] = Lerp(gRefView[4], sShotView[4], t->timer, 20);
            gView[5] = Lerp(gRefView[5], sShotView[5], t->timer, 20);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (t->timer < 20) {
                t->timer++;
            }
            if (t->timer == 15) {
                PlaySe(0x22);
            }
            if (++t->homeRotZ > 20) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 6:
            if (++t->timer > 20) {
                gPlayer.curMotion[0] = 0xFF;
                gPlayer.action = 0x19;
                gFellOff = 1;
                sShotView[0] = rsin(gCourse.rot.vy + 0x800) * 200 >> 12;
                sShotView[2] = rcos(gCourse.rot.vy + 0x800) * 200 >> 12;
                sShotView[1] = gPlayer.y - 150;
                sShotView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                sShotView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                sShotView[4] = gPlayer.y - 150;
                gRefView[0] = rsin(gCourse.rot.vy + 0x800) * 30 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x800) * 30 >> 12;
                gRefView[1] = gPlayer.y - 150;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[4] = gPlayer.y - 150;
                t->homeRotZ = 0;
                t->timer = 0;
                t->state++;
            }
            break;
        case 7:
            gView[0] = Lerp(gRefView[0], sShotView[0], t->timer, 20);
            gView[1] = Lerp(gRefView[1], sShotView[1], t->timer, 20);
            gView[2] = Lerp(gRefView[2], sShotView[2], t->timer, 20);
            gView[3] = Lerp(gRefView[3], sShotView[3], t->timer, 20);
            gView[4] = Lerp(gRefView[4], sShotView[4], t->timer, 20);
            gView[5] = Lerp(gRefView[5], sShotView[5], t->timer, 20);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (t->timer < 20) {
                t->timer++;
            }
            if (t->timer == 15) {
                PlaySe(0x22);
            }
            if (++t->homeRotZ > 60) {
                t->timer = 0;
                t->state++;
                StartScreenWipe();
                gFellOff = 1;
                sShotView[0] = rsin(gCourse.rot.vy + 0x800) * 300 >> 12;
                sShotView[2] = rcos(gCourse.rot.vy + 0x800) * 300 >> 12;
                sShotView[1] = gPlayer.y - 250;
                sShotView[3] = rsin(gCourse.rot.vy + 0x800) * 1500 >> 12;
                sShotView[5] = rcos(gCourse.rot.vy + 0x800) * 1500 >> 12;
                sShotView[4] = gPlayer.y - 400;
                gRefView[0] = rsin(gCourse.rot.vy) * 500 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy) * 500 >> 12;
                gRefView[1] = gPlayer.y - 250;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 1500 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 1500 >> 12;
                gRefView[4] = gPlayer.y - 200;
                t->homeRotZ = 0;
            }
            break;
        case 8:
            t->timer++;
            if (IsScreenWipeActive() == 0) {
                gView[0] = Lerp(gRefView[0], sShotView[0], 0, 90);
                gView[1] = Lerp(gRefView[1], sShotView[1], 0, 90);
                gView[2] = Lerp(gRefView[2], sShotView[2], 0, 90);
                gView[3] = Lerp(gRefView[3], sShotView[3], 0, 90);
                gView[4] = Lerp(gRefView[4], sShotView[4], 0, 90);
                gView[5] = Lerp(gRefView[5], sShotView[5], 0, 90);
                GsSetRefView2((GsRVIEW2 *)gView);
                t->timer = 0;
                t->state++;
            }
            break;
        case 9:
            gView[0] = Lerp(gRefView[0], sShotView[0], t->timer, 90);
            gView[1] = Lerp(gRefView[1], sShotView[1], t->timer, 90);
            gView[2] = Lerp(gRefView[2], sShotView[2], t->timer, 90);
            gView[3] = Lerp(gRefView[3], sShotView[3], t->timer, 90);
            gView[4] = Lerp(gRefView[4], sShotView[4], t->timer, 90);
            gView[5] = Lerp(gRefView[5], sShotView[5], t->timer, 90);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (++t->timer > 90) {
                t->timer = 0;
                sGoalLightsSet = 1;
                t->state++;
            }
            break;
        case 10:
            if (++t->timer == 15) {
                sGoalLightLit[2] = 1;
                PlaySe(0x71);
            }
            if (t->timer == 45) {
                sGoalLightLit[1] = 1;
                PlaySe(0x71);
            }
            if (t->timer == 75) {
                sGoalLightLit[0] = 1;
                PlaySe(0x71);
            }
            if (t->timer == 80) {
                PlaySe(0x33);
            }
            if (t->timer > 120) {
                ret = 1;
            }
            break;
    }
    return ret;
}

void func_800F6FA0(void) {
    s16 v;

    if (gPlayer.control != 0 && (u32)(gPlayer.motion[0] - MOT_RIDE) < 5) {
        v = gPlayer.speed;
        if (v < 100) {
            gSpeedFixed += 16;
        } else if (gPad & 0x1000) { /* PADLup */
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 100) {
                gSpeedFixed = 0x640;
            }
        } else if (gPad & 0x4000) { /* PADLdown */
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 100) {
                gSpeedFixed = 0x640;
            }
        } else if (v > 100) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        gPlayer.speed = (s16)gSpeedFixed >> 4;
    }
}

/* MATCHING: player's, declared only where defined. */
void OnJumpStart(PlayerState *g);

void func_800F70A8(void) {
    s16 n;

    if (gPlayer.control != 0) {
        if ((gGamePadTrig & 0x20) &&
            (gPlayer.y >= gPlayer.groundY || (gPlayer.onRideQuad == 1 && gPlayer.onQuadTop == 1)) &&
            gPlayer.jumpState == JUMP_NONE) {
            gPlayer.airborne = 1;
            gPlayer.onQuadTop = 0;
            gPlayer.jumpState = JUMP_RISING;
            gPlayer.jumpTime = 0;
            gPlayer.peakY = 0;
            OnJumpStart(&gPlayer);
            gPlayer.motion[0] = MOT_RIDE_JUMP;
            gPlayer.jumpPower = 0x23;
            gPlayer.jumpStartY = gPlayer.y;
            ScatterDust((VECTOR *)&gPlayer.x);
            PlaySe(0x17);
        }
        if (gPlayer.jumpState == JUMP_RISING) {
            n = ++gPlayer.jumpTime;
            if (n < 3) {
                gPlayer.jumpPower = 0x21;
            } else if ((u16)(n - 4) < 3) {
                gPlayer.jumpPower = 0x1A;
            } else if (n >= 9) {
                gPlayer.jumpPower = 0x1C;
                gPlayer.jumpState = JUMP_RELEASED;
                gPlayer.motion[0] = MOT_RIDE_JUMP;
                gPlayer.jumpHeight = 2;
            } else if (gGamePad & 0x20) {
                gPlayer.jumpPower = 0x1C;
            } else {
                gPlayer.jumpState = JUMP_RELEASED;
                switch (n) {
                    case 3:
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.jumpHeight = 0;
                        break;
                    case 7:
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.jumpHeight = 1;
                        break;
                    default:
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.jumpHeight = 2;
                        break;
                }
            }
        }
        if (gPlayer.jumpState == JUMP_RELEASED) {
            gPlayer.jumpState = JUMP_NONE;
        }
    }
}

extern s16 sSteerDrift;
/* MATCHING: player's, declared only where defined. */
void ApplySteer(u8 mode, u16 a, s16 b);
void RecentreSteer(void);

void func_800F729C(void) {
    if (gPad & 0x8000) { /* PADLleft */
        ApplySteer(3, 16, 15);
        sSteerDrift -= 3;
        gPlayer.action = 0x35;
    } else if (gPad & 0x2000) { /* PADLright */
        ApplySteer(2, 16, 15);
        sSteerDrift += 3;
        gPlayer.action = 0x37;
    } else {
        RecentreSteer();
        gPlayer.action = 0x33;
        if (sSteerDrift != 0) {
            if (sSteerDrift < 0) {
                sSteerDrift++;
            } else {
                sSteerDrift--;
            }
        }
    }
    if (gPadType == 0) {
        RecentreSteer();
    }
    sSteerDrift = sSteerDrift < -50 ? -50 : sSteerDrift > 50 ? 50 : sSteerDrift;
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F74FC, StartSpriteWalk, 0x3C);

#include "engine/run_trap_type04.inc"

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F7968, RunTrapType0A, 0x58);
FUNC_ALIAS(func_800F79A0, RunTrapType0A, 0x90);

#include "engine/run_trap_type0b.inc"

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {}

void StepTrapType12(StageTrap *t, u16 speed, s16 arg) {}

void StepTrapType1F(StageTrap *t, u16 arg) {}

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapFFAct68(StageTrap *t) {}

void StepTrapAct21(StageTrap *t) {}

void StepTrapAct22(StageTrap *t) {}

void StepTrapAct23(StageTrap *t) {}

void StepTrapAct24(StageTrap *t) {}

void StepTrapAct25(StageTrap *t, s32 arg) {
    u8 unused[0x78];

    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
    }
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(func_800F82E0, StepTrapAct25, 0x78);

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void RunTrapAhead(StageTrap *t, s16 speed) {}

void RunTrapAheadNonstop(StageTrap *t, s16 speed) {}

void RunTrapAlongPath(StageTrap *t, u16 speed, s8 turn) {}

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

#include "engine/draw_trap_warning.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType2A(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F8454, RunTrapType13, 0x38);
FUNC_ALIAS(func_800F84A8, RunTrapType13, 0x8C);

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1D(StageTrap *t, u16 speed) {}

void RunTrapAheadAtHeight(StageTrap *t, u16 speed) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F87E4, RunTrapType2C, 0x24);

#include "engine_code.inc" /* BurstTrapStacked */

void SlideTrapAway(StageTrap *t, s32 arg) {}

#include "engine/tumble_trap_act08.inc"

#include "engine/tumble_trap_act09.inc"

#include "engine/tumble_trap_act0b.inc"

#include "engine/knock_trap_away_act0c.inc"

void StepDashBoard(StageTrap *t) {}

#include "engine/tumble_trap_act12.inc"

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepSmokingTumbleTrap(StageTrap *t) {}

void StepTrapAct1D(StageTrap *t) {}

void StepSmokingTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

#include "engine/draw_trap_while_linked.inc"

void UpdateStageEventHook(void) {
    StageTrap *m;

    m = &D_800D39D8[1];
    if (gSwitches[192].state == SWITCH_ON) {
        m->state = 0;
        gPlayer.motion[0] = 11;
        gPlayer.jumpPower = 60;
        gPlayer.action = 0;
        gSwitches[192].state = SWITCH_OFF;
        gPlayer.dropRide = 1;
        m->timer = 0;
        m->state = 1;
        gMissState = 0;
        gPlayer.hittable = 0;
        gPlayer.invulnTimer = 30;
        gPlayer.rideMode &= 0xF0;
        PlaySe(0x38);
    }
}

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

void func_800F8B4C(DirEnt16 *dir, u16 first, u16 count) {
    DirEnt16 *e;
    u16 i;

    e = dir;
    e += first;
    for (i = first; i < first + count; i++) {
        LoadTim((u8 *)dir + e->offset);
        e++;
        DrawSync(0);
    }
}

void func_800F8BD4(void) {
    RECT r;

    r.x = gFrameCount / 2 % 12 * 16;
    r.y = 0x1F3;
    r.w = 16;
    r.h = 1;
    MoveImage(&r, 0, 0x1E0);
}

#include "engine/reveal_start_cans.inc"

#include "engine/emit_trap_particles.inc"

s32 func_800F8D34(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&sFiles2Ovl16Loc[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl16Sectors[1];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gVabLoadStep = 0;
                gSubStep++;
            }
            break;
        case 2:
            if (LoadVabBank(6) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl16Loc[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl16Sectors[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl16Loc[3];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&sFiles2Ovl16Loc[4];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl16Sectors[3];
            gCdLoadSectors[1] = sFiles2Ovl16Sectors[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl16Loc[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl16Sectors[5];
            gSubStep++;
            break;
        case 8:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadSpritePack((void *)0x8016D000, SPR_STAGE);
                gSubStep++;
            }
            break;
        case 9:
            InitEntryModels();
            gSubStep++;
            break;
        case 10:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl16Loc[6];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl16Sectors[6];
            gSubStep++;
            break;
        case 11:
            if (gCdLoadState == CD_LOAD_DONE) {
                SetHitBanks((void *)0x8016D000, (void *)0x80171744);
                RestoreStageObjects();
                LoadStageSprites();
                gSubStep++;
            }
            break;
        case 12:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl16Loc[7];
            gCdLoadDest[0] = 0x8018D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl16Sectors[7];
            gSubStep++;
            break;
        case 13:
            if (gCdLoadState == CD_LOAD_DONE) {
                ResetStageObjects();
                StartStageOverlay();
                SetBootReady(1);
                ret = 1;
            }
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}

/* Resets the player, the camera and the course to the stage's start. */
void func_800F90F4(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    gCameraYaw = 0;
    sCamHeight = 0;
    sViewSpeed = 100;
    gViewerAngle = 0xD8;
    gFellOff = 0;
    st = gStageStart;
    /* MATCHING: the pointer and the 1 in locals order the first stores. */
    one = 1;
    gPlayLive = one;
    yaw = -st->yaw;
    ((SVECTOR *)sCamOffset)->vx = 0;
    gView[1] = 0;
    ((SVECTOR *)sCamOffset)->vy = 0;
    gPlayer.motion[0] = 2;
    gPlayer.control = 1;
    gPlayer.hittable = 1;
    gCourse.rot.vy = yaw;
    gCourse.rot.vz = yaw;
    gPlayer.startYaw = gPlayer.yaw = yaw;
    gFloorIndex = 0;
    gCourse.x = gPlayer.x = -st->x;
    gSteerYaw = yaw;
    gCourse.y = gPlayer.y = -st->y;
    gSteerYawHalf = yaw;
    gCourse.z = gPlayer.z = -st->z;
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gView[1] = (s16)(gCourse.y - 500);
    RevealStartCans();
}

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 func_800F922C(s16 id, StageTrap *src, StageTrap *dst) {
    u16 ret = 1;
    u8 models[80] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
                     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79};

    if (id < 80) {
        if (id >= 0) {
            dst->homeX = src->x;
            dst->homeY = src->y;
            dst->homeZ = src->z;
            dst->homeRotX = src->rotX;
            dst->homeRotY = src->rotY;
            dst->homeRotZ = src->rotZ;
            ResetObj34(dst);
            dst->model = models[id];
            ret = 1;
        }
    }
    return ret;
}

/** @brief The shape of each of this stage's own sprites, as one block. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} StageShapes;

/** @brief The shapes as this overlay's rodata holds them: the link
 *         padded the block to a word. */
typedef struct {
    StageShapes shapes; /**< the shapes */
    u8 pad[3];          /**< MATCHING: the link's padding, whatever its buffer held */
} StageShapesRodata;

extern const StageShapesRodata sStageSpriteShapes;

void func_800F936C(void) {
    StageShapes shapes;

    shapes = sStageSpriteShapes.shapes;
    *(StageShapes *)sSpriteShapes = shapes;
}

/* This stage's sprite shapes. */
const StageShapesRodata sStageSpriteShapes = {
    {{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    }},
    {35, 208, 15},
};
