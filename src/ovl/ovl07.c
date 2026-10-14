/* Stage 2-2's overlay: the stage engine (engine.inc, engine_code.inc) with
 * this stage's own routines and the engine functions still in asm here. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

#include "engine/reset_spawned_traps.inc"

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
FUNC_ALIAS(func_800F74FC, StepHoppingTrap, 0x284);

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

/* MATCHING: the type dispatchers call these two addresses inside the
 * function above (other stages' entry points in their own overlays). */
FUNC_ALIAS(func_800F7968, StepMissSequence, 0xD8);
FUNC_ALIAS(func_800F79A0, StepMissSequence, 0x110);

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
                if (func_800F82CC(u)) {
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
                    gStage = GetNextStage(4);
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

void func_800F8184(u8 *dir, u16 id, u16 first, u16 count) {
    u16 i;
    s32 tp;
    DirEnt16 *e;
    TimInfo *t;

    /* MATCHING: the entry pointer set in two steps outranks the counter
     * for the first saved register. */
    e = (DirEnt16 *)dir;
    e += first;
    for (i = first; i < first + count; i++) {
        LoadTim(dir + e->offset);
        DrawSync(0);
        tp = GetTPage(0, 0, ((TimInfo *)gTimInfo)->pixRect.x, ((TimInfo *)gTimInfo)->pixRect.y);
        t = (TimInfo *)gTimInfo;
        DefineSprite(id, 0, (u8)t->width, (u8)t->pixRect.h, (u8)tp,
                     (u8)((t->pixRect.x - t->pixRect.x / 64 * 64) * t->perUnit),
                     (u8)(t->pixRect.y + 1 - t->pixRect.y / 256), t->clutRect.x, (u16)t->clutRect.y);
        id++;
        e++;
    }
}

s8 func_800F82CC(StageTrap *t) {
    s8 ret = 0;

    gPlayer.showShadow = 0;
    DrawFrozenFrame();
    switch (t->state) {
        case 0:
            StopCdTrack();
            gPlayer.action = 0;
            gPlayer.motion[0] = 0x36;
            gFellOff = 1;
            gRefView[0] = rsin(gCourse.rot.vy + 0x5C7) * 300 >> 12;
            gRefView[2] = rcos(gCourse.rot.vy + 0x5C7) * 300 >> 12;
            gRefView[1] = gPlayer.y - 150;
            gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 150 >> 12;
            gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 150 >> 12;
            D_800FF8DC = *(GsRVIEW2 *)gView;
            gRefView[4] = gPlayer.y - 100;
            t->homeRotZ = 0;
            t->timer = 0;
            t->state++;
            SetSoundFade(4);
        case 1:
            if (t->homeRotZ == 30) {
                PlaySe(0x22);
            }
            gPlayer.fallSpeed = 0;
            gPlayer.drawX += rsin(gCourse.rot.vy + 0x800) * 3 >> 12;
            gPlayer.drawZ += rcos(gCourse.rot.vy + 0x800) * 3 >> 12;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = t->homeY + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (t->timer < 60) {
                t->timer++;
            }
            gView[0] = Lerp(((s32 *)&D_800FF8DC)[0], gRefView[0], t->timer, 60);
            gView[1] = Lerp(((s32 *)&D_800FF8DC)[1], gRefView[1], t->timer, 60);
            gView[2] = Lerp(((s32 *)&D_800FF8DC)[2], gRefView[2], t->timer, 60);
            gView[3] = Lerp(((s32 *)&D_800FF8DC)[3], gRefView[3], t->timer, 60);
            gView[4] = Lerp(((s32 *)&D_800FF8DC)[4], gRefView[4], t->timer, 60);
            gView[5] = Lerp(((s32 *)&D_800FF8DC)[5], gRefView[5], t->timer, 60);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (++t->homeRotZ > 60) {
                gRefView[0] = rsin(gCourse.rot.vy + 0x471) * 800 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x471) * 800 >> 12;
                gRefView[1] = gPlayer.y - 450;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 700 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 700 >> 12;
                t->homeRotZ = 0;
                t->homeRotX = 0;
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            gPlayer.fallSpeed = 0;
            if (t->homeRotZ == 10) {
                PlaySe(0x1A);
            }
            t->homeRotZ++;
            t->homeRotX++;
            gPlayer.drawY += t->homeRotZ * 2;
            gPlayer.drawX += rsin(gCourse.rot.vy + 0x800) * 25 >> 12;
            gPlayer.drawZ += rcos(gCourse.rot.vy + 0x800) * 25 >> 12;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gPlayer.groundY = gTodCoords[0].coord.t[1] = t->homeY + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (t->timer < 60) {
                t->timer++;
            }
            gView[0] = Lerp(gView[0], gRefView[0], t->timer, 80);
            gView[1] = Lerp(gView[1], gRefView[1], t->timer, 80);
            gView[2] = Lerp(gView[2], gRefView[2], t->timer, 80);
            gView[3] = Lerp(gView[3], gRefView[3], t->timer, 80);
            gView[4] = Lerp(gView[4], gRefView[4], t->timer, 80);
            gView[5] = Lerp(gView[5], gRefView[5], t->timer, 80);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (t->homeRotX > 40) {
                t->timer = 0;
                t->state++;
                StartScreenWipe();
                sGoalLightsSet = 1;
            }
            break;
        case 3:
            if (++t->timer == 15) {
                PlaySe(0x33);
            }
            if (IsScreenWipeActive() == 0) {
                t->state++;
                gPlayer.x = 0x1EC0;
                gPlayer.z = 0x18AA;
                gView[0] = rsin(gCourse.rot.vy + 0x800) * 500 >> 12;
                gView[2] = rcos(gCourse.rot.vy + 0x800) * 500 >> 12;
                gView[1] = -0x992;
                gView[3] = rsin(gCourse.rot.vy + 0x800) * 700 >> 12;
                gView[5] = rcos(gCourse.rot.vy + 0x800) * 700 >> 12;
                gView[4] = -0x960;
                GsSetRefView2((GsRVIEW2 *)gView);
            }
            break;
        case 4:
            ret = 1;
            break;
    }
    return ret;
}

/* MATCHING: the type-34 dispatcher calls these addresses inside the
 * function above (other stages' entry points in their own overlays). */
FUNC_ALIAS(func_800F82E0, func_800F82CC, 0x14);
FUNC_ALIAS(func_800F8454, func_800F82CC, 0x188);
FUNC_ALIAS(func_800F84A8, func_800F82CC, 0x1DC);
FUNC_ALIAS(func_800F87E4, func_800F82CC, 0x518);
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

void UpdateStageEventHook(void) {}

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

#include "engine/load_tim_range.inc"

#include "engine/reveal_start_cans.inc"
#include "engine/emit_trap_particles.inc"

s32 func_800FA1C0(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800FB1A0[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[1];
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
            if (LoadVabBank(3) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&D_800FB1A0[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[3];
            gCdLoadSectors[1] = D_800FB1C0[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[5];
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
            gCdLoadLoc[0] = (s32)&D_800FB1A0[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[6];
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
            gCdLoadLoc[0] = (s32)&D_800FB1A0[6];
            gCdLoadDest[0] = 0x8018D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[7];
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
u16 func_800FA580(s16 id, StageTrap *src, StageTrap *dst) {
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

void func_800FA6C0(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    /* MATCHING: stored as the s16 it is read as. */
    *(s16 *)&gCameraYaw = -0x201;
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

void func_800FA800(void) {
    SpriteShapes shapes;

    shapes = D_800F08E8;
    *(SpriteShapes *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl07/nonmatchings/ovl07", D_800F08E8);
