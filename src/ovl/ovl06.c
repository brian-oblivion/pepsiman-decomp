/* Stage 2-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

/* This overlay's data, in address order: the engine's (engine_data.inc)
 * around its own. */
#define ENGINE_SPEEDS_PAD 0

#include "engine_data.inc" /* sFiles1BootLoc to sFiles1LastSectors */

#include "engine_data.inc" /* sBurstDirs to sType1FSpeeds */

#include "engine_data.inc" /* sFiles2BootLoc to sFiles3LastSectors */

#include "engine_data.inc" /* sWalkPose to sGoalTraps */

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

void ResetSpawnedTraps(void) {
    u32 i;
    StageTrap *t;
    ModelSlot *a;
    ModelSlot *b;
    ModelSlot *m;

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
        m = (ModelSlot *)gObjectModels;
        t = (StageTrap *)gTraps;
        b = &m[t[195].model];
        a = &m[t[194].model];
        b->scale.vy = a->scale.vy = b->scale.vx = a->scale.vx = 0x1000;
    }
}

#include "engine_code.inc" /* StepCheckpoint */

#include "engine/step_flung_piece.inc"

#include "engine/spawn_miss_debris.inc"

#include "engine/step_miss_debris.inc"

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

#include "engine/step_charging_trap.inc"

#include "engine/step_rolling_trap.inc"

#include "engine/run_trap_ahead_at_height.inc"

#include "engine/knock_trap_away.inc"

#include "engine/tumble_trap_away.inc"

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/step_smoking_tumble_trap.inc"

#include "engine/step_smoking_trap.inc"

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine/slide_trap_away.inc"

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine/step_dash_board.inc"

#include "engine_code.inc" /* StepFlipPanel */

#include "engine/spin_trap_away.inc"

#include "engine_code.inc" /* TipTrapAway */

#include "engine/step_spray_trap.inc"

#include "engine/burst_raised_trap.inc"

#include "engine/turn_trap_on_approach.inc"

#include "engine/open_gate_on_approach.inc"

#include "engine/swing_gate_continuously.inc"

#include "engine/topple_trap_on_approach.inc"

#include "engine/scatter_trap_dust.inc"

#include "engine/shift_trap_on_approach.inc"

void StepHoppingTrap(StageTrap *t, u8 act) {
    VECTOR v;
    s32 off;
    s32 amp;
    s16 r;
    s16 ang;
    s32 d;
    s32 e;

    switch (act) {
        case 0:
            off = 0x400;
            amp = 125;
            break;
        case 1:
            off = 0xC00;
            amp = 250;
            break;
        case 2:
            off = 0x400;
            break;
        case 3:
            off = 0xC00;
            break;
        case 4:
            off = 0;
            break;
        case 5:
            off = 0x800;
            break;
    }
    if (t->state == 0) {
        t->timer++;
        d = rsin((t->timer * 20 << 12) / 360) * 25 >> 12;
        t->y = t->homeY - (d >= 0 ? d : -d);
        switch (act) {
            case 0:
            case 1:
                r = rsin((t->timer * 3 << 12) / 360) * amp >> 12;
                ang = t->rotY + off;
                t->x = t->homeX - (rsin(ang) * r >> 12);
                t->z = t->homeZ - (rcos(ang) * r >> 12);
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                ang = t->rotY + off;
                t->x += rsin(ang) * 20 >> 12;
                t->z += rcos(ang) * 20 >> 12;
                break;
        }
        if ((t->x + gWorldShift[0] >= 0 ? t->x + gWorldShift[0] : -(t->x + gWorldShift[0])) < 2000) {
            e = t->z + gWorldShift[2];
            if ((e >= 0 ? e : -e) < 2000 && t->timer % 3 == 0) {
                v.vx = t->x + (rsin((t->rotY + 0x800) * (rand() % 10 * 10)) >> 12);
                v.vz = t->z + (rcos((t->rotY + 0x800) * (rand() % 10 * 10)) >> 12);
                v.vy = t->y;
                ScatterDust(&v);
            }
        }
        if (gSection - UpdatePathSegment(t) < 2) {
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->timer * 20 % 180 == 0) {
                PlaySe(0x4E);
            }
        }
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x36);
        }
    }
}

/* MATCHING: the engine dispatchers call this address inside the function
 * above (another stage's entry point in its own overlay). */
FUNC_ALIAS(func_800F74FC, StepHoppingTrap, 0x260);

#include "engine/swap_trap_model_on_approach.inc"

void StepMissSequence(void) {
    s32 unused[18];
    StageTrap *m;
    u16 i;

    m = &D_800AC6E0;
    if (D_800D3638 == 1) {
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
                gRefView[3] = gView[3];
                gRefView[4] = gView[4];
                gRefView[5] = gView[5];
                m->state++;
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, D_8007AD68[gStage].x, m->timer, 25);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, D_8007AD68[gStage].z, m->timer, 25);
                gView[1] = gRefView[1] + Lerp(0, 65, m->timer, 25);
                gView[4] = gRefView[4] + Lerp(0, 50, m->timer, 25);
                GsSetRefView2((GsRVIEW2 *)gView);
                if (++m->timer >= 26) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                gPlayer.motion[0] = 0;
                if (++m->timer != 0) {
                    D_800AC6B8 = 90;
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 3:
                if (++m->timer < 8 && m->timer % 2 == 0) {
                    PlaySe(0x38);
                }
                if (m->timer >= 9) {
                    m->timer = 0;
                    m->state++;
                    gOffsetVec[0] = gPlayer.x;
                    gOffsetVec[2] = gPlayer.z;
                    for (i = 0; i < 40; i++) {
                        gOffsetVec[1] = gPlayer.y - i * 10;
                        ScatterDust((VECTOR *)gOffsetVec);
                    }
                    gPlayer.rideMode = (gPlayer.rideMode & 0xF0) | 2;
                    gSpeedFixed = 800;
                    SpawnMissDebris();
                    gMissState = 0;
                    gFellOff = 0;
                    D_800D39D8[1].state = 0;
                    D_800D39D8[1].timer = 0;
                    gPlayer.control = 1;
                }
                gView[1] = gRefView[1] + 100;
                GsSetRefView2((GsRVIEW2 *)gView);
                break;
            case 4:
                DrawFrozenFrame();
                if (++m->timer == 30) {
                    m->state++;
                }
                gView[1] = gRefView[1] + 100;
                GsSetRefView2((GsRVIEW2 *)gView);
                break;
        }
    }
}

/* MATCHING: the engine dispatchers call these addresses inside the function
 * above (other stages' entry points in their own overlays). */
FUNC_ALIAS(func_800F7968, StepMissSequence, 0xB4);
FUNC_ALIAS(func_800F79A0, StepMissSequence, 0xEC);

extern s8 D_800D36F0;
/* MATCHING: main's view of the loader. */
void BeginLoadScreen(void);
s8 func_800F8960(StageTrap *t);

void UpdateGoalEventHook(void) {
    StageTrap *t;
    StageTrap *u;
    s16 n;
    s16 m;

    t = sGoalTraps;
    u = &sGoalTraps[1];
    if (D_800D36F0 == 1) {
        ResetRumble();
        switch (gStage) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                n = 35;
                m = 100;
                break;
            case 6:
            case 7:
            case 8:
                break;
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                n = 35;
                m = 100;
                break;
        }
        DrawFrozenFrame();
        switch (t->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = 0;
                gPlayer.motion[0] = 4;
                StopSpeed();
                gMissState = -2;
                ClipF = 1;
                t->homeX = gPlayer.x;
                t->homeY = gPlayer.y;
                t->homeZ = gPlayer.z;
                SetSoundFade(4);
                t->timer = 0;
                t->state++;
            case 1:
                gPlayer.x = Lerp(t->homeX, D_8007AE1C[gStage].x, t->timer, n);
                gPlayer.z = Lerp(t->homeZ, D_8007AE1C[gStage].z, t->timer, n);
                t->timer++;
                if (t->timer > n) {
                    t->timer = 0;
                    gPlayer.action = 0x17;
                    t->homeX = gPlayer.x;
                    t->homeY = gPlayer.y;
                    t->homeZ = gPlayer.z;
                    t->state++;
                }
                break;
            case 2:
                gPlayer.x = Lerp(gPlayer.x, D_8007AED0[gStage].x, t->timer, m);
                gPlayer.z = Lerp(gPlayer.z, D_8007AED0[gStage].z, t->timer, m);
                t->timer++;
                if (t->timer < (s16)(m / 3)) {
                    ScatterDust((VECTOR *)&gPlayer.x);
                }
                if (t->timer > m) {
                    gPlayer.action = 0;
                    gPlayer.motion[0] = 0x13;
                    gFellOff = 1;
                    gRefView[0] = rsin(gCourse.rot.vy + 0xCE3) * 300 >> 12;
                    gRefView[2] = rcos(gCourse.rot.vy + 0xCE3) * 300 >> 12;
                    gRefView[1] = gPlayer.y - 270;
                    gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                    gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
                    t->homeRotZ = 0;
                    t->timer = 0;
                    gRefView[4] = gPlayer.y - 100;
                    t->state++;
                }
                break;
            case 3:
                if (t->timer < 30) {
                    t->timer++;
                }
                gView[0] = Lerp(gView[0], gRefView[0], t->timer, 30);
                gView[1] = Lerp(gView[1], gRefView[1], t->timer, 30);
                gView[2] = Lerp(gView[2], gRefView[2], t->timer, 30);
                gView[3] = Lerp(gView[3], gRefView[3], t->timer, 30);
                gView[4] = Lerp(gView[4], gRefView[4], t->timer, 30);
                gView[5] = Lerp(gView[5], gRefView[5], t->timer, 30);
                GsSetRefView2((GsRVIEW2 *)gView);
                t->homeRotZ++;
                if (t->timer == 8) {
                    gPlayLive = 0;
                }
                if (t->homeRotZ >= 41) {
                    t->timer = 0;
                    gPlayer.action = 0x1B;
                    t->state++;
                }
                break;
            case 4:
                if (++t->timer >= 241) {
                    t->timer = 0;
                    t->state++;
                    StartActResults();
                }
                break;
            case 5:
                if (IsResultsActive() == 0) {
                    t->state++;
                }
                break;
            case 6:
                if (func_800F8960(u) && gInEditor != 1) {
                    t->state++;
                }
                break;
            case 7:
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
            case 8:
                if (++t->timer >= 9) {
                    gStage = GetNextStage(3);
                    BeginLoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

/* MATCHING: an unused 32-byte local gives retail's frame. */
void StepStageFrame(void) {
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
}

/* MATCHING: the engine dispatchers call these addresses inside the function
 * above (other stages' entry points in their own overlays). */
FUNC_ALIAS(func_800F82E0, StepStageFrame, 0x7C);
FUNC_ALIAS(func_800F8454, StepStageFrame, 0x1F0);

void func_800F87E0(StageTrap *t, s8 dir);

/* MATCHING: s32 with no return statement, which keeps retail's tail. */
s32 func_800F84A8(StageTrap *t) {
    s16 speed;

    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) < 2250000) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 2:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) <= 2250000) {
                speed = 40;
                if (t->timer % 20 == 1) {
                    func_800F87E0(t, t->homeRotZ % 3);
                    t->homeRotZ++;
                }
            } else {
                speed = 5;
            }
            SnapToFloor(t);
            t->timer++;
            t->y -= 20;
            t->rotY = GetPathDirection(t);
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 36000000) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            break;
        case 3:
            InterpRange(t->homeRotY, t->homeRotY - 0x400, t->timer, 30, gOffsetVec);
            t->rotY = gOffsetVec[1];
            StepTrapForwardLevel(t, 20);
            if (++t->timer >= 31) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 4:
            StepTrapForwardLevel(t, 20);
            if (++t->timer >= 31) {
                t->timer = 0;
                t->state++;
            }
            break;
    }
}

void func_800F87E0(StageTrap *t, s8 dir) {
    PlacedPos p;
    StageTrap *s;
    s16 n;

    n = FindFreeSlot48();
    if (n != -1) {
        s = &sSpawnedTraps[n];
        *s = *t;
        switch (dir) {
            case 2:
                p.lx = -80;
                break;
            case 1:
                p.lx = 80;
                break;
            case 3:
                p.lx = 0;
                break;
        }
        p.ly = 0;
        p.lz = 0;
        p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
        UpdatePlacedPos(&p);
        s->x = s->homeX = p.pos.vx;
        s->y = s->homeY = p.pos.vy;
        s->z = s->homeZ = p.pos.vz;
        s->model = 10;
        s->type = 0xFF;
        s->act = 100;
        s->state = 0;
        s->timer = 90;
    }
}

/* MATCHING: the engine dispatchers call this address inside the function
 * above (another stage's entry point in its own overlay). */
FUNC_ALIAS(func_800F87E4, func_800F87E0, 0x4);

s8 func_800F8960(StageTrap *t) {
    StageTrap *g;
    StageTrap *h;
    s8 ret = 0;

    switch (t->state) {
        case 0:
            t->homeX = gPlayer.x;
            t->homeY = gPlayer.y;
            t->homeZ = gPlayer.z;
            gRefView[0] = 0;
            gRefView[2] = 0;
            gRefView[1] = 0;
            gRefView[3] = 0;
            gRefView[5] = 0;
            gRefView[4] = 800;
            t->homeRotZ = 0;
            t->timer = 0;
            t->state++;
            PlaySe(0x23);
        case 1:
            g = (StageTrap *)gTraps;
            g[193].y += t->homeRotZ * 3;
            g[193].rotZ = Lerp(g[193].rotZ, 796, t->timer, 40);
            gPlayer.drawY += t->homeRotZ * 2;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (++t->homeRotZ >= 21) {
                t->homeRotZ = 0;
                t->timer = 0;
                t->state++;
                gPlayer.motion[0] = 0x36;
                PlaySe(0x1C);
                PlaySe(0x1A);
            }
            break;
        case 2:
            h = (StageTrap *)gTraps;
            h[193].y += t->homeRotZ * 4;
            gPlayer.drawY += t->homeRotZ * 3;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (t->timer < 60) {
                t->timer++;
            }
            gView[1] = Lerp(gView[1], gRefView[1], t->timer, 60);
            gView[3] = Lerp(gView[3], gRefView[3], t->timer, 60);
            gView[4] = Lerp(gView[4], gRefView[4], t->timer, 60);
            gView[5] = Lerp(gView[5], gRefView[5], t->timer, 60);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (++t->homeRotZ >= 66) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            ret = 1;
            break;
    }
    return ret;
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

#include "engine/run_trap_type04.inc"

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

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

void StepTrapAct25(StageTrap *t, s32 arg) {}

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

#include "engine/draw_trap_warning.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType2A(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

void StepTrapType27(StageTrap *t, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1D(StageTrap *t, u16 speed) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */

#include "engine_code.inc" /* BurstTrapStacked */

#include "engine/tumble_trap_act08.inc"

#include "engine/tumble_trap_act09.inc"

#include "engine/tumble_trap_act0b.inc"

#include "engine/knock_trap_away_act0c.inc"

#include "engine/tumble_trap_act12.inc"

void StepTrapAct1D(StageTrap *t) {}

#include "engine/draw_trap_while_linked.inc"

#include "engine/update_stage_event_hook.inc"

#include "engine_code.inc" /* UpdateGoalEvents */

void StepGoalSequence(void) {}

void CycleGoalLights(s8 arg) {}

#include "engine/reveal_start_cans.inc"

void func_800FA334(void) {}

#include "engine/emit_trap_particles.inc"

s32 LoadStageFiles(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&sFiles2Ovl06Loc[1];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl06Sectors[1];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)RAM_STAGE_LOAD);
                gVabLoadStep = 0;
                gSubStep++;
            }
            break;
        case 2:
            if (LoadVabBank(3) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl06Loc[2];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl06Sectors[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)RAM_STAGE_LOAD);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl06Loc[3];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadLoc[1] = (s32)&sFiles2Ovl06Loc[4];
            gCdLoadDest[1] = RAM_OBJECT_MODELS;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl06Sectors[3];
            gCdLoadSectors[1] = sFiles2Ovl06Sectors[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)RAM_STAGE_LOAD);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl06Loc[5];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl06Sectors[5];
            gSubStep++;
            break;
        case 8:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadSpritePack((void *)RAM_WORK_A, SPR_STAGE);
                gSubStep++;
            }
            break;
        case 9:
            InitEntryModels();
            gSubStep++;
            break;
        case 10:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl06Loc[6];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl06Sectors[6];
            gSubStep++;
            break;
        case 11:
            if (gCdLoadState == CD_LOAD_DONE) {
                SetHitBanks((void *)RAM_WORK_A, (void *)(RAM_WORK_A + 0x4744));
                RestoreStageObjects();
                LoadStageSprites();
                gSubStep++;
            }
            break;
        case 12:
            ResetStageObjects();
            StartStageOverlay();
            SetBootReady(1);
            ret = 1;
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}

void ResetToStageStart(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    /* MATCHING: stored as the s16 it is read as. */
    *(s16 *)&gCameraYaw = -2;
    sCamHeight = -150;
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
    gView[1] = (s16)(gCourse.y - 150);
    RevealStartCans();
}

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 PlaceTrap(s16 id, StageTrap *src, StageTrap *dst) {
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
    u8 shape[74]; /**< a shape per sprite id from SPR_STAGE on */
} StageShapes;

/** @brief The shapes as this overlay's rodata holds them: the link
 *         padded the block to a word. */
typedef struct {
    StageShapes shapes; /**< the shapes */
    u8 pad[2];          /**< MATCHING: the link's padding, whatever its buffer held */
} StageShapesRodata;

extern const StageShapesRodata sStageSpriteShapes;

void CopySpriteShapes(void) {
    StageShapes shapes;

    shapes = sStageSpriteShapes.shapes;
    *(StageShapes *)sSpriteShapes = shapes;
}

/* This stage's sprite shapes. */
const StageShapesRodata sStageSpriteShapes = {
    {{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    }},
    {133, 64},
};
