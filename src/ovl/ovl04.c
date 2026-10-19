/* Stage 1-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

/* This overlay's data, in address order: the engine's (engine_data.inc)
 * around its own. */
#define ENGINE_SPEEDS_PAD 0

#include "engine_data.inc" /* sFiles1BootLoc to sFiles1LastSectors */

#include "engine_data.inc" /* sBurstDirs to sType1FSpeeds */

#include "engine_data.inc" /* sFiles2BootLoc to sFiles3LastSectors */

/* The camera view saved before a shot and restored after it. */
s32 sSavedView[8] = {0};

#include "engine_data.inc" /* sWalkPose to sGoalTraps */

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

#include "engine/reset_spawned_traps.inc"

#include "engine_code.inc" /* StepCheckpoint */

#include "engine/step_flung_piece.inc"

#include "engine/spawn_miss_debris.inc"

#include "engine/step_miss_debris.inc"

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

#include "engine/step_trap_act27.inc"

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

#include "engine/step_trap_type2a.inc"

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {
    StageTrap c;
    s32 unused[8];
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
                PlaySe(SE_TRUCK_HORN);
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
                    case 1:
                    case 3:
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
            c = *t;
            c.model++;
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
                if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 &&
                    gPlayer.hittable == 1) {
                    PlaySe(SE_TRAP_HIT);
                }
            }
            break;
    }
}

#include "engine/step_trap_type12.inc"

#include "engine/knock_trap_away.inc"

#include "engine/tumble_trap_away.inc"

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/step_trap_ff_act68.inc"

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

#include "engine/open_gate_on_approach.inc"

#include "engine/swing_gate_continuously.inc"

#include "engine/step_hopping_trap.inc"

#include "engine/swap_trap_model_on_approach.inc"

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
                gFellOff = 1;
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
                gPlayer.x = Lerp(gEventTraps[0].homeX, gMissTargets[gStage].x, m->timer, 25);
                gPlayer.z = Lerp(gEventTraps[0].homeZ, gMissTargets[gStage].z, m->timer, 25);
                gView[1] = gRefView[1] + Lerp(0, 65, m->timer, 25);
                gView[4] = gRefView[4] + Lerp(0, 50, m->timer, 25);
                GsSetRefView2((GsRVIEW2 *)gView);
                if (++m->timer >= 26) {
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
                    gPlayer.rideMode = (gPlayer.rideMode & RIDE_ITEM_MASK) | RIDE_BOARD;
                    gSpeedFixed = 800;
                    SpawnMissDebris();
                    gMissState = 0;
                    gFellOff = 0;
                    gEventTraps[1].state = 0;
                    gEventTraps[1].timer = 0;
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

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(StepStage4_1Dropper, StepMissSequence, 0x12C);

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
                    gStage = GetNextStage(1);
                    BeginAct3LoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

void LoadTimRange(u8 *dir, u16 first, u16 count);

#include "engine/cycle_goal_lights.inc"

/* MATCHING: the type dispatchers call these two addresses inside the
 * function above (other stages' entry points in their own overlays). */
FUNC_ALIAS(StepStage5_1Dropper, CycleGoalLights, 0x4);
FUNC_ALIAS(StepTrapFFAct65, CycleGoalLights, 0x3C);

#include "engine/step_stage_frame.inc"

void StepRideDismount(StageTrap *m) {
    if (gSwitches[190].state == SWITCH_ON) {
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                StopSpeed();
                EndJump();
                gEventTraps[0].homeRotZ = 0;
                gEventTraps[0].homeRotX = 0;
                gEventTraps[0].homeX = gPlayer.x;
                gEventTraps[0].homeY = gPlayer.y;
                gEventTraps[0].homeZ = gPlayer.z;
                gMissState = -1;
                m->timer = 0;
                m->state++;
                /* fall through */
            case 1:
                gPlayer.x = Lerp(gEventTraps[0].homeX, 0x6407, m->timer, 20);
                gPlayer.z = Lerp(gEventTraps[0].homeZ, -0x5854, m->timer, 20);
                m->timer++;
                if ((u8)(gPlayer.motion[0] - 2) < 3 && gEventTraps[0].homeRotX == 0) {
                    gEventTraps[0].homeRotX = 1;
                    gPlayer.dropRide = 1;
                    gPlayer.rideMode &= 0xF0;
                }
                if (m->timer > 20) {
                    m->timer = 0;
                    m->state++;
                    if (gEventTraps[0].homeRotX == 0) {
                        gPlayer.dropRide = 1;
                        gPlayer.rideMode &= 0xF0;
                    }
                }
                break;
            case 2:
                gPlayer.motion[0] = MOT_JUMP_LOW;
                gPlayer.action = ACT_RUN;
                gPlayer.control = 1;
                gPlayer.speed = 0x50;
                gSpeedFixed = 0x500;
                gPlayer.jumpPower = 60;
                gSwitches[192].state = SWITCH_OFF;
                gPlayer.rideMode &= 0xF0;
                gMissState = 0;
                m->timer = 0;
                m->state++;
                gPlayer.hittable = 0;
                gPlayer.invulnTimer = 70;
                break;
        }
    }
}

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
                sSavedView[0] = gView[0];
                sSavedView[1] = gView[1];
                sSavedView[2] = gView[2];
                sSavedView[3] = gView[3];
                sSavedView[4] = gView[4];
                sSavedView[5] = gView[5];
            }
            break;
        case 2:
            if (++t->timer > 30) {
                gPlayer.action = ACT_STOP_WAIT_START;
                gFellOff = 1;
                gRefView[0] = rsin(gCourse.rot.vy + 0x71D) * 200 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x71D) * 200 >> 12;
                gRefView[1] = gPlayer.y - 280;
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
                gView[0] = sSavedView[0];
                gView[1] = sSavedView[1];
                gView[2] = sSavedView[2];
                gView[3] = sSavedView[3];
                gView[4] = sSavedView[4] + 10;
                gView[5] = sSavedView[5];
                GsSetRefView2((GsRVIEW2 *)gView);
            }
            break;
        case 5:
            ret = 1;
            break;
    }
    return ret;
}

FUNC_ALIAS(StepStage1_1Dropper, StepGoalCutscene, 0x3E4);


void SpawnTrapFFAct68(StageTrap *t, s8 arg);

void StepStage1_2Dropper(StageTrap *t) {
    s16 speed;

    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) <= 0x22550F) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 2:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) > 0x225510) {
                speed = 5;
            } else {
                speed = 40;
                if (t->timer % 20 == 1) {
                    SpawnTrapFFAct68(t, (s16)t->homeRotZ % 3);
                    t->homeRotZ++;
                }
            }
            SnapToFloor(t);
            t->timer++;
            t->y -= 20;
            t->rotY = GetPathDirection(t);
            PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 0xC5C100) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            break;
        case 3:
            InterpRange(t->homeRotY, t->homeRotY + ANGLE_DEG(90), t->timer, 30, gOffsetVec);
            t->rotY = gOffsetVec[1];
            StepTrapForwardLevel(t, 20);
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 4:
            StepTrapForwardLevel(t, 20);
            break;
    }
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(StepStage2_1Dropper, StepStage1_2Dropper, 0x54);

void SpawnTrapFFAct68(StageTrap *t, s8 arg) {
    PlacedPos p;
    s16 models[9] = {0x17, 0x18, 0x19, 0x45, 0x46, 0x47, 0x49, 0x4A, 0x4B};
    StageTrap *s;
    s16 i;
    s16 m;
    s32 r;
    s32 d;

    i = FindFreeSlot48();
    if (i != -1) {
        s = &sSpawnedTraps[i];
        *s = *t;
        switch (arg) {
            case 2:
                p.lx = -80;
                break;
            case 1:
                p.lx = 80;
                break;
            case 0:
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
        m = models[rand() % 9];
        s->type = 0xFF;
        s->act = 0x68;
        s->state = 0;
        s->timer = 40;
        s->model = m;
        s->rotY = gCourse.rot.vy + ANGLE_DEG(180);
        d = (rand() % 3 - 1) * 227;
        s->homeRotX = 100;
        s->homeRotZ = -10;
        s->homeX = 1;
        s->aux = arg;
        s->homeRotY += d;
        r = rand() % 720 + 360;
        s->counter = 0;
        s->homeZ = r;
    }
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(StepStage1_1CreepingDropper, SpawnTrapFFAct68, 0x58);

void StepBoardSpeed(void) {
    s16 v;

    if (gPlayer.control != 0 && (u32)(gPlayer.motion[0] - MOT_RIDE) < 5) {
        v = gPlayer.speed;
        if (v < 40) {
            gSpeedFixed += 16;
        } else if (gPad & 0x1000) { /* PADLup */
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 80) {
                gSpeedFixed = 0x500;
            }
        } else if (gPad & 0x4000) { /* PADLdown */
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 40) {
                gSpeedFixed = 0x280;
            }
        } else if (v > 80) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        gPlayer.speed = (s16)gSpeedFixed >> 4;
    }
}

/* MATCHING: player's view of these, declared only where defined. */
void OnJumpStart(PlayerState *g);
void StartRumble(u16 *table, s32 index);

void StepBoardJump(void) {
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
            StartRumble(gRumblePatterns, RUMBLE_NONE);
            PlaySe(SE_JUMP);
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
void PushPlayer(s32 deg, s32 dist);

void SteerBoard(void) {
    if (gPad & 0x8000) { /* PADLleft */
        ApplySteer(3, 16, 5);
        sSteerDrift -= 10;
        gPlayer.action = ACT_RIDE_LEAN_A_START;
    } else if (gPad & 0x2000) { /* PADLright */
        ApplySteer(2, 16, 5);
        sSteerDrift += 10;
        gPlayer.action = ACT_RIDE_LEAN_B_START;
    } else {
        RecentreSteer();
        gPlayer.action = ACT_RIDE_CENTRE_START;
        if (sSteerDrift != 0) {
            if (sSteerDrift < 0) {
                sSteerDrift += 2;
            } else {
                sSteerDrift -= 2;
            }
        }
    }
    if (gPadType == 0) {
        RecentreSteer();
    }
    sSteerDrift = sSteerDrift < -100 ? -100 : sSteerDrift > 100 ? 100 : sSteerDrift;
    PushPlayer(90, (s16)(sSteerDrift / 10));
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

#include "engine/run_trap_type04.inc"

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

#include "engine/run_trap_type0b.inc"

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

void StepTrapType1F(StageTrap *t, u16 arg) {}

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapAct21(StageTrap *t) {}

void StepTrapAct22(StageTrap *t) {}

#include "engine/step_trap_act23.inc"

#include "engine/step_trap_act24.inc"

void StepTrapAct25(StageTrap *t, s32 arg) {}

#include "engine/step_trap_act28.inc"

#include "engine_code.inc" /* ArmTrapByRange */

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

#include "engine/draw_trap_warning.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 tiltSide, s32 unused) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType29(StageTrap *t, s32 startSide, s8 pauseMode) {}

void StepTrapType27(StageTrap *t, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1D(StageTrap *t, u16 speed) {}

void RunTrapAheadAtHeight(StageTrap *t, u16 speed) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */

#include "engine_code.inc" /* BurstTrapStacked */

#include "engine/tumble_trap_act08.inc"

#include "engine/tumble_trap_act09.inc"

#include "engine/tumble_trap_act0b.inc"

#include "engine/knock_trap_away_act0c.inc"

#include "engine/tumble_trap_act12.inc"

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepTrapAct1D(StageTrap *t) {}

void BurstRaisedTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void ToppleTrapOnApproach(StageTrap *t, s32 side, s32 speed, u8 kind) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void DrawTrapWhileLinked(StageTrap *t) {
    Query30 q;
    StageTrap *k;

    k = &((StageTrap *)gTraps)[195];
    if (k->state < 4) {
        if (k->state >= 0 && gSection - UpdatePathSegment(t) < 2) {
            DrawObjectModel(&gObjectModels[t->model], t);
        }
    }
}

void UpdateStageEventHook(void) {
    StepRideDismount(&gEventTraps[1]);
}

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
            gCdLoadLoc[0] = (s32)&sFiles2Ovl04Loc[1];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl04Sectors[1];
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
            if (LoadVabBank(2) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl04Loc[2];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl04Sectors[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)RAM_STAGE_LOAD);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl04Loc[3];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadLoc[1] = (s32)&sFiles2Ovl04Loc[4];
            gCdLoadDest[1] = RAM_OBJECT_MODELS;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl04Sectors[3];
            gCdLoadSectors[1] = sFiles2Ovl04Sectors[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)RAM_STAGE_LOAD);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&sFiles2Ovl04Loc[5];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl04Sectors[5];
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
            gCdLoadLoc[0] = (s32)&sFiles2Ovl04Loc[6];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl04Sectors[6];
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
            gCdLoadLoc[0] = (s32)&sFiles2Ovl04Loc[7];
            gCdLoadDest[0] = RAM_WORK_C;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles2Ovl04Sectors[7];
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

void ResetToStageStart(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    /* MATCHING: stored as the s16 it is read as. */
    *(s16 *)&gCameraYaw = -ANGLE_DEG(270);
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

extern const SpriteShapes sStageSpriteShapes;

void CopySpriteShapes(void) {
    SpriteShapes shapes;

    shapes = sStageSpriteShapes;
    *(SpriteShapes *)sSpriteShapes = shapes;
}

/* This stage's sprite shapes. */
const SpriteShapes sStageSpriteShapes = {{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
}};
