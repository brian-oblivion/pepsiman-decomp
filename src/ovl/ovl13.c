/* Stage 4-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

/* MATCHING: the engine's callers pass StepTrapType18 a word speed; this
 * stage's body takes it as s16, so the shared declaration is set aside. */
#define StepTrapType18 StepTrapType18_engine
#include "engine.inc"

/* This overlay's data, in address order: the engine's (engine_data.inc)
 * around its own. */
#define ENGINE_SPEEDS_PAD 0x660D

#include "engine_data.inc" /* sFiles1BootLoc to sFiles1LastSectors */

#include "engine_data.inc" /* sBurstDirs to sType1FSpeeds */

#include "engine_data.inc" /* sFiles2BootLoc to sFiles3LastSectors */

#include "engine_data.inc" /* sWalkPose to sGoalTraps */
#undef StepTrapType18
void StepTrapType18(StageTrap *t, s16 speed, s32 arg, s8 dir);

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

#include "engine/reset_spawned_traps.inc"

#include "engine_code.inc" /* StepCheckpoint */

#include "engine/step_flung_piece.inc"

#include "engine/spawn_miss_debris.inc"

#include "engine/step_miss_debris.inc"

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

void StepTrapAct22(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x45);
                }
                t->timer = 0;
                t->homeRotX = 0;
                t->state++;
                t->homeY = t->y;
            }
            break;
        case 1:
            t->timer = (t->timer + 10) % 360;
            d = rsin((t->timer << 12) / 360) * 150 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if (++t->homeRotX > 30) {
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2 && gFrameCount % 3) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_along_path.inc"

#include "engine/run_trap_then_turn_timed.inc"

void DrawTrapWarning(StageTrap *t) {
    SVECTOR pos;
    SVECTOR sv;
    s32 d;

    if (!(t->marks & 0x40)) {
        d = SquareRoot0(DistanceSquared((VECTOR *)t, (VECTOR *)&gPlayer.x));
        if (gSection - UpdatePathSegment(t) < 2) {
            t->marks |= 0x40;
        }
        if (d < 1200) {
            if (!(gFrameCount & 1)) {
                return;
            }
        } else if (d < 2400) {
            if ((gFrameCount & 3) < 2) {
                return;
            }
        } else if (d < 3200) {
            if ((gFrameCount & 7) < 4) {
                return;
            }
        }
        gOffsetVec[0] = t->x + (rsin(t->rotY) * d >> 12);
        gOffsetVec[2] = t->z + (rcos(t->rotY) * d >> 12);
        gOffsetVec[1] = gPlayer.groundY - 100;
        gOffsetVec[0] += gWorldShift[0];
        gOffsetVec[2] += gWorldShift[2];
        ProjectToScreen((VECTOR *)gOffsetVec, &sv);
        pos.vy = 50;
        pos.vx = sv.vx - 32;
        DrawSprite(SPR_DANGER, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    }
}

void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {
    StageTrap copy;
    s32 unused[4];
    s32 z;

    z = 0;
    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
            PlaySe(0x53);
            if (dir == 3) {
                PlaySe(SE_DANGER);
            }
        case 1:
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(0x55);
            }
            if (SnapToFloor(t) == -1) {
                t->y = t->homeY;
            }
            t->y -= 120;
            PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (speed != 0) {
                /* MATCHING: a test only combine can fold (after flow), guarding
                 * a read of z, leaves z's dead zero in a saved register. */
                if (((s32)speed >> 16) != 0) {
                    if (z) {
                        t->state = 0;
                    }
                }
                switch (dir) {
                    case 2:
                        t->rotY = GetPathDirection(t) + ANGLE_DEG(180);
                        break;
                    case 3:
                        DrawTrapWarning(t);
                    case 1:
                        t->rotY = GetPathDirection(t);
                        break;
                    case 4:
                        PolarToXZ(-(speed * 2), ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
                        t->x += gOffsetVec[0] >> 12;
                        t->z += gOffsetVec[2] >> 12;
                        if (arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                            t->state++;
                        }
                        break;
                    case 5:
                        if (gSection - UpdatePathSegment(t) >= 2) {
                            t->state += 2;
                        }
                        break;
                }
            }
        case 2:
            copy = *t;
            copy.model++;
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[copy.model], (Trap *)&copy);
                if ((t->touched != 0 || copy.touched != 0) && gPlayer.invulnTimer == 0 &&
                    gPlayer.hittable == 1) {
                    PlaySe(SE_TRAP_HIT);
                }
            }
            break;
    }
}

#include "engine/step_trap_type12.inc"

#include "engine/step_trap_type1_f.inc"

#include "engine/step_rolling_trap.inc"

#include "engine/step_trap_type1_d.inc"

void StepTrapType18(StageTrap *t, s16 speed, s32 arg, s8 dir) {
    VECTOR v;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (LatchProximity(t, 1000) != 0) {
                PlaySe(0x7F);
            }
            SnapToFloor(t);
            switch (dir) {
                case 0:
                    PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
                    t->x -= gOffsetVec[0] >> 12;
                    t->z -= gOffsetVec[2] >> 12;
                    if (arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                        t->timer = 0;
                        t->homeRotZ = 0;
                        t->state++;
                    }
                    break;
                case 1:
                    t->rotY = GetPathDirection(t) + ANGLE_DEG(180);
                    t->homeRotY = t->rotY + ANGLE_DEG(180);
                    PolarToXZ(speed, ANGLE_TO_DEG(t->homeRotY), (Vec3 *)gOffsetVec);
                    t->x += gOffsetVec[0] >> 12;
                    t->z += gOffsetVec[2] >> 12;
                    break;
            }
            break;
        case 2:
            PolarToXZ(Lerp(speed, 0, t->timer, 15), ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (++t->timer == 15) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            break;
        default:
            goto dust;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(SE_TRAP_HIT);
    }
dust:
    if (t->state < 3 &&
        (t->x + gWorldShift[0] >= 0 ? t->x + gWorldShift[0] : -(t->x + gWorldShift[0])) < 2000 &&
        (t->z + gWorldShift[2] >= 0 ? t->z + gWorldShift[2] : -(t->z + gWorldShift[2])) < 2000 &&
        (u16)(t->timer % 3) == 0) {
        v.vx = t->x + (rsin((t->rotY + ANGLE_DEG(180)) * (10 * (rand() % 20))) >> 12);
        v.vz = t->z + (rcos((t->rotY + ANGLE_DEG(180)) * (10 * (rand() % 20))) >> 12);
        v.vy = t->y;
        ScatterDust(&v);
    }
}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {
    s32 d;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->homeRotZ = t->rotZ;
            t->state++;
        case 1:
            if (LatchProximity(t, 1000) != 0) {
                PlaySe(0x80);
            }
            SnapToFloor(t);
            t->y -= 50;
            if (dir == 0) {
                PolarToXZ((s16)speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
                t->x += gOffsetVec[0] >> 12;
                t->z += gOffsetVec[2] >> 12;
            }
            t->rotX += ((s16)speed << 12) / 360;
            t->rotZ = t->homeRotZ + (rsin((t->timer << 12) / 360) * 20 >> 12);
            if (arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeY = t->y;
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 2:
            d = rsin((Lerp(0, 180, t->timer, 15) << 12) / 360) * 100 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d) + 50;
            PolarToXZ(Lerp((s16)speed, 0, t->timer, 15), ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (++t->timer == 15) {
                t->timer = 0;
                t->state++;
            }
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_CRASH);
            }
            break;
    }
}

#include "engine/step_trap_type1_c.inc"

void StepTrapType1E(StageTrap *t, u16 speed) {
    VECTOR v;
    s32 d;

    switch (t->state) {
        case 0:
            t->homeRotX = (rand() % 5 + 1) * 20;
            t->homeRotZ = (rand() % 4 + 1) * 20;
            t->state++;
        case 1:
            SnapToFloor(t);
            t->y -= 120;
            d = rsin((Lerp(0, 180, t->timer, t->homeRotZ) << 12) / 360 * t->homeRotX) >> 12;
            t->y -= d >= 0 ? d : -d;
            if ((t->x + gWorldShift[0] >= 0 ? t->x + gWorldShift[0] : -(t->x + gWorldShift[0])) < 2000 &&
                (t->z + gWorldShift[2] >= 0 ? t->z + gWorldShift[2] : -(t->z + gWorldShift[2])) < 2000 &&
                (u16)(t->timer % 3) == 0) {
                v.vx = t->x + (rsin((t->rotY + ANGLE_DEG(180)) * (10 * (rand() % 20))) >> 12);
                v.vz = t->z + (rcos((t->rotY + ANGLE_DEG(180)) * (10 * (rand() % 20))) >> 12);
                v.vy = t->y + 100;
                ScatterDust(&v);
            }
            PolarToXZ((s16)speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->rotX += ((s16)speed << 12) / 360;
            if (LatchProximity(t, 1500) != 0) {
                PlaySe(0x80);
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x1D);
            }
            t->timer++;
            if (t->timer > t->homeRotZ) {
                t->state--;
            }
            break;
    }
}

#include "engine/run_trap_ahead_at_height.inc"

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

#include "engine/spin_trap_away.inc"

#include "engine_code.inc" /* TipTrapAway */

#include "engine/step_trap_act1_d.inc"

#include "engine/burst_raised_trap.inc"

#include "engine/topple_trap_on_approach.inc"

#include "engine/scatter_trap_dust.inc"

void StepMissSequence(void) {
    s32 unused[18];
    StageTrap *m;
    u16 i;

    m = &gMissTrap;
    if (gMissSwitchState == 1) {
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = ACT_RUN;
                gPlayer.motion[0] = MOT_RUN_FAST;
                StopSpeed();
                EndJump();
                gMissState = -1;
                gEventTraps[0].homeRotZ = 0;
                gFellOff = 0;
                m->timer = 0;
                gEventTraps[0].homeX = gPlayer.x;
                gEventTraps[0].homeY = gPlayer.y;
                gEventTraps[0].homeZ = gPlayer.z;
                gRefView[0] = gView[0];
                gRefView[1] = gView[1];
                gRefView[2] = gView[2];
                gRefView[3] = gView[3];
                gRefView[4] = gView[4];
                gRefView[5] = gView[5];
                m->state++;
            case 1:
                gPlayer.x = Lerp(gEventTraps[0].homeX, gMissTargets[gStage].x, m->timer, 45);
                gPlayer.z = Lerp(gEventTraps[0].homeZ, gMissTargets[gStage].z, m->timer, 45);
                if (++m->timer >= 46) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                gPlayer.motion[0] = MOT_DEFAULT;
                if (++m->timer != 0) {
                    D_800AC6B8 = 90;
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 3:
                if (++m->timer < 8 && m->timer % 2 == 0) {
                    PlaySe(SE_CRASH);
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
                    gPlayer.rideMode = (gPlayer.rideMode & RIDE_ITEM_MASK) | RIDE_ROLLER;
                    gSpeedFixed = 800;
                    SpawnMissDebris();
                    gMissState = 0;
                    gFellOff = 0;
                    gEventTraps[1].state = 0;
                    gEventTraps[1].timer = 0;
                    gPlayer.control = 1;
                }
                break;
            case 4:
                DrawFrozenFrame();
                if (++m->timer == 30) {
                    m->state++;
                }
                break;
        }
    }
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(StepStage4_1Dropper, StepMissSequence, 0x2FC);

s8 StepGoalCutscene(StageTrap *t);

void StepGoalSequence(void) {
    StageTrap *t;
    StageTrap *u;

    t = sGoalTraps;
    u = &sGoalTraps[1];
    if (gAct2GoalSwitchState == 1) {
        ResetRumble();
        DrawFrozenFrame();
        switch (t->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = ACT_RUN;
                gPlayer.motion[0] = MOT_RUN_FAST;
                StopSpeed();
                gMissState = -2;
                ClipF = 1;
                t->timer = 0;
                u->homeX = gPlayer.x;
                u->homeY = gPlayer.y;
                u->homeZ = gPlayer.z;
                t->state++;
            case 1:
                if (StepGoalCutscene(u)) {
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
                    gStage = GetNextStage(10);
                    BeginAct3LoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

void LoadTimRange(u8 *dir, u16 first, u16 count);

#include "engine/cycle_goal_lights.inc"

#include "engine/step_stage_frame.inc"
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the function above. */
FUNC_ALIAS(StepStage5_1Dropper, StepStageFrame, 0x124);
FUNC_ALIAS(StepTrapFFAct65, StepStageFrame, 0x15C);

/* MATCHING: frame counts as s16 locals set at the top keep saved registers. */
s8 StepGoalCutscene(StageTrap *t) {
    s8 ret = 0;
    s16 n = 35;
    s16 m = 100;

    switch (t->state) {
        case 0:
            gPlayer.x = Lerp(t->homeX, gGoalWalkTargets[gStage].x, t->timer, n);
            gPlayer.z = Lerp(t->homeZ, gGoalWalkTargets[gStage].z, t->timer, n);
            if (++t->timer > n) {
                t->homeX = gPlayer.x;
                t->homeY = gPlayer.y;
                t->homeZ = gPlayer.z;
                t->timer = 0;
                t->state++;
                gPlayer.action = ACT_STOP_START;
                SetSoundFade(4);
            }
            break;
        case 1:
            gPlayer.x = Lerp(gPlayer.x, gGoalSlideTargets[gStage].x, t->timer, m);
            gPlayer.z = Lerp(gPlayer.z, gGoalSlideTargets[gStage].z, t->timer, m);
            if (++t->timer < (s16)(m / 3)) {
                ScatterDust((VECTOR *)&gPlayer.x);
            }
            if (t->timer > m) {
                gPlayer.action = ACT_RUN;
                gPlayer.motion[0] = MOT_STAND;
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            if (++t->timer > 30) {
                gPlayer.action = ACT_STOP_WAIT_START;
                gFellOff = 1;
                gRefView[0] = rsin(gCourse.rot.vy + 0x71D) * 200 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x71D) * 200 >> 12;
                gRefView[1] = gPlayer.y - 80;
                gRefView[3] = rsin(gCourse.rot.vy + ANGLE_DEG(180)) * 50 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + ANGLE_DEG(180)) * 50 >> 12;
                gRefView[4] = gPlayer.y - 130;
                t->homeRotZ = 0;
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            gView[0] = Lerp(gView[0], gRefView[0], t->timer, 20);
            gView[1] = Lerp(gView[1], gRefView[1], t->timer, 20);
            gView[2] = Lerp(gView[2], gRefView[2], t->timer, 20);
            gView[3] = Lerp(gView[3], gRefView[3], t->timer, 20);
            gView[4] = Lerp(gView[4], gRefView[4], t->timer, 20);
            gView[5] = Lerp(gView[5], gRefView[5], t->timer, 20);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (t->timer < 20) {
                t->timer++;
            }
            if (t->timer == 15) {
                PlaySe(SE_POWER_UP);
            }
            if (++t->homeRotZ > 40) {
                t->timer = 0;
                t->state++;
                StartScreenWipe();
                sGoalLightsSet = 1;
            }
            break;
        case 4:
            if (++t->timer == 15) {
                PlaySe(SE_GOAL);
            }
            if (IsScreenWipeActive() == 0) {
                t->state++;
                gView[0] = rsin(gCourse.rot.vy + ANGLE_DEG(180)) * 500 >> 12;
                gView[2] = rcos(gCourse.rot.vy + ANGLE_DEG(180)) * 500 >> 12;
                gView[1] = t->homeY - 150;
                gView[3] = rsin(gCourse.rot.vy + ANGLE_DEG(180)) * 700 >> 12;
                gView[5] = rcos(gCourse.rot.vy + ANGLE_DEG(180)) * 700 >> 12;
                GsSetRefView2((GsRVIEW2 *)gView);
            }
            break;
        case 5:
            ret = 1;
            break;
    }
    return ret;
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(StepStage1_1Dropper, RunTrapTypeFF, 0xF4);
FUNC_ALIAS(StepStage1_2Dropper, RunTrapType02, 0x7C);

void RunTrapType04(StageTrap *t) {
    Speeds sp = sChargeRanges;
    s16 range = sp.v[t->act / 4];

    StepChargingTrap(t, t->act * 10 + 40, range);
}

FUNC_ALIAS(StepStage2_1Dropper, RunTrapType04, 0x3C);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

#include "engine/run_trap_type0b.inc"

#include "engine_code.inc" /* RunTrapType32 */

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(StepStage1_1CreepingDropper, RunTrapType10, 0x48);

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapFFAct68(StageTrap *t) {}

#include "engine/step_trap_act21.inc"

void StepTrapAct23(StageTrap *t) {}

#include "engine/step_trap_act24.inc"

void StepTrapAct25(StageTrap *t, s32 arg) {}

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType2A(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType29(StageTrap *t, s32 startSide, s8 pauseMode) {}

void StepTrapType27(StageTrap *t, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */

void TumbleTrapAway(StageTrap *t, s32 height, s8 spawn) {}

void KnockTrapAway(StageTrap *t) {}

void ShatterTrap(StageTrap *t) {}

#include "engine_code.inc" /* BurstTrapStacked */

void SlideTrapAway(StageTrap *t, s32 arg) {}

void TumbleTrapAct08(StageTrap *t) {}

void TumbleTrapAct09(StageTrap *t) {}

void TumbleTrapAct0B(StageTrap *t) {}

void KnockTrapAwayAct0C(StageTrap *t) {}

void StepDashBoard(StageTrap *t) {}

void TumbleTrapAct12(StageTrap *t) {}

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepSmokingTumbleTrap(StageTrap *t) {}

void StepSmokingTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

void SwapTrapModelOnApproach(StageTrap *t, s32 speed, u8 arg) {}

#include "engine/draw_trap_while_linked.inc"

#include "engine/update_stage_event_hook.inc"

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

#include "engine/load_tim_range.inc"

#include "engine/reveal_start_cans.inc"

#include "engine/emit_trap_particles.inc"

s32 LoadStageFiles(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&sFiles2Ovl13Loc[1];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl13Sectors[1];
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
            if (LoadVabBank(5) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl13Loc[2];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl13Sectors[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)RAM_STAGE_LOAD);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl13Loc[3];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadLoc[1] = (s32)&sFiles2Ovl13Loc[4];
            gCdLoadDest[1] = RAM_OBJECT_MODELS;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl13Sectors[3];
            gCdLoadSectors[1] = sFiles2Ovl13Sectors[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)RAM_STAGE_LOAD);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl13Loc[5];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl13Sectors[5];
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
            gCdLoadLoc[0] = (s32)&sFiles2Ovl13Loc[6];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl13Sectors[6];
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
            gCdLoadLoc[0] = (s32)&sFiles2Ovl13Loc[7];
            gCdLoadDest[0] = RAM_WORK_C;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl13Sectors[7];
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

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 PlaceTrap(s16 id, StageTrap *src, StageTrap *dst) {
    u16 ret = 1;
    u8 models[76] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
                     19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 34, 37, 40, 43, 44, 45,
                     46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
                     65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};

    if (id < 35) {
        if (id >= 31) {
            goto triple;
        }
        if (id < 0) {
            goto out;
        }
    } else if (id >= 80) {
        goto out;
    }
    dst->homeX = src->x;
    dst->homeY = src->y;
    dst->homeZ = src->z;
    dst->homeRotX = src->rotX;
    dst->homeRotY = src->rotY;
    dst->homeRotZ = src->rotZ;
    ResetObj34(dst);
    dst->model = models[id];
    ret = 1;
    goto out;
triple:
    dst->homeX = src->x;
    dst->homeY = src->y;
    dst->homeZ = src->z;
    dst->homeRotX = src->rotX;
    dst->homeRotY = src->rotY;
    dst->homeRotZ = src->rotZ;
    ResetObj34(dst);
    dst->model = models[id];
    dst++;
    dst->homeX = 0;
    dst->homeY = -100;
    dst->homeZ = -50;
    dst->homeRotX = 0;
    dst->homeRotY = 0;
    dst->homeRotZ = 0;
    ResetObj34(dst);
    dst->model = models[id] + 1;
    dst++;
    dst->homeX = 0;
    dst->homeY = -100;
    dst->homeZ = 50;
    dst->homeRotX = 0;
    dst->homeRotY = 0;
    dst->homeRotZ = 0;
    ResetObj34(dst);
    dst->model = models[id] + 2;
    ret = 3;
out:
    return ret;
}

void ResetToStageStart(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    /* MATCHING: stored as the s16 it is read as. */
    *(s16 *)&gCameraYaw = ANGLE_DEG(90);
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
    gPlayer.motion[0] = MOT_RUN_SLOW;
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

/** @brief The shape of each of this stage's own sprites, as one block. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes41;

/** @brief The shapes as this overlay's rodata holds them: the link
 *         padded the block to a word. */
typedef struct {
    SpriteShapes41 shapes; /**< the shapes */
    u8 pad[3];             /**< MATCHING: the link's padding, whatever its buffer held */
} SpriteShapes41Rodata;

extern const SpriteShapes41Rodata sStageSpriteShapes;

void CopySpriteShapes(void) {
    SpriteShapes41 shapes;

    shapes = sStageSpriteShapes.shapes;
    *(SpriteShapes41 *)sSpriteShapes = shapes;
}

/* This stage's sprite shapes. */
const SpriteShapes41Rodata sStageSpriteShapes = {
    {{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    }},
    {16, 255, 253},
};
