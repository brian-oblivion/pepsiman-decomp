/* Stage 2-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

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

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", SpawnMissDebris);

void StepMissDebris(StageTrap *t) {
    s32 d;

    if (t->state == 0) {
        PolarToXZ(t->homeRotZ, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        t->y = t->homeY;
        t->timer = (t->timer + t->homeZ) % 360;
        d = rsin((t->timer << 12) / 360) * t->homeX >> 12;
        t->homeRotX++;
        t->y = t->homeY - (d >= 0 ? d : -d);
        t->rotX -= (t->homeRotZ << 12) / 360;
        t->rotZ += (t->homeZ << 12) / 360;
        gModeObjectModels[t->model].coord.super = NULL;
        if (gFrameCount & 3) {
            DrawObjectModel((ObjectModel *)&gModeObjectModels[t->model], t);
        }
        if (t->homeRotX > 60) {
            t->model = -1;
            t->state++;
        }
    }
}

INCLUDE_RODATA("asm/ovl/ovl06/nonmatchings/ovl06", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepChargingTrap);

#include "engine/step_rolling_trap.inc"

#include "engine/run_trap_ahead_at_height.inc"

void KnockTrapAway(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(58);
                }
                t->homeX = 0;
                t->homeY = t->y;
                t->homeRotY = D_8009EEF8[0];
                t->state++;
                t->timer = 0;
            }
            t->y = t->homeY;
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer = (t->timer + 10) % 360;
            t->rotZ -= 11;
            d = rsin((t->timer << 12) / 360) * 150 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if (++t->homeX == 25) {
                t->state++;
            }
            if (gFrameCount % 3 != 0) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

#ifdef NON_MATCHING
void TumbleTrapAway(StageTrap *t, s32 height, s8 spawn) {
    s16 n;
    StageTrap *p;
    s32 d;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            t->y -= height;
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x37);
                }
                t->homeX = 0;
                t->homeY = t->y;
                t->homeRotY = gPlayer.yaw;
                t->homeRotZ = 0;
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x36);
            }
            t->y = t->homeY;
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer += 10;
            t->rotX -= 113;
            d = rsin((t->timer << 12) / 360) * 150 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if (++t->homeX == 30) {
                t->state++;
            }
            if (gFrameCount % 3 != 0) {
                DrawObjectModel(&gObjectModels[t->model], t);
                if ((u16)(t->timer % 360) == 180 || (u16)(t->timer % 360) == 0) {
                    PlaySe(0x36);
                }
            }
            if (spawn != 0 && t->homeRotZ < 5 && (t->timer == 180 || t->timer == 270)) {
                n = FindFreeSlot48();
                p = &sSpawnedTraps[n];
                *p = *t;
                sSpawnedTraps[n].rotZ = 0;
                sSpawnedTraps[n].rotX = 0;
                sSpawnedTraps[n].rotY = sSpawnedTraps[n].homeRotY = gCourse.rot.vy;
                sSpawnedTraps[n].x -= 25 - rand() % 50;
                sSpawnedTraps[n].z -= 25 - rand() % 50;
                sSpawnedTraps[n].state = 0;
                SnapToFloor(&sSpawnedTraps[n]);
                sSpawnedTraps[n].model = rand() % 3 + 17;
                sSpawnedTraps[n].type = 0;
                sSpawnedTraps[n].act = 1;
                t->homeRotZ++;
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", TumbleTrapAway);
#endif

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/step_smoking_tumble_trap.inc"

#include "engine/step_smoking_trap.inc"

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine/slide_trap_away.inc"

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine/step_dash_board.inc"

#include "engine_code.inc" /* StepFlipPanel */

void SpinTrapAway(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched == 1) {
                t->homeX = 0;
                t->homeY = t->y;
                /* MATCHING: read as a member at a fixed address, which keeps
                 * the load below the stores into t. */
                t->homeRotY = ((SVECTOR *)D_8009EEF8)->vx;
                t->state++;
                t->timer = 0;
            }
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer = (t->timer + 10) % 360;
            t->rotX += 113;
            t->rotZ += 113;
            d = rsin((t->timer << 12) / 360) * 200 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if (++t->homeX == 20) {
                t->state++;
            }
            if ((gFrameCount & 1) && gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

#include "engine_code.inc" /* TipTrapAway */

void StepSprayTrap(StageTrap *t, s8 dir) {
    PlacedPos p;
    StageTrap c;
    s32 d;

    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
        if (dir != 0) {
            switch (t->state) {
                case 0:
                    switch (dir) {
                        case 2:
                            p.lx = -140;
                            break;
                        case 1:
                            p.lx = 140;
                            break;
                        case 3:
                            p.lx = 0;
                            break;
                    }
                    p.ly = 50;
                    p.lz = 0;
                    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
                    UpdatePlacedPos(&p);
                    t->homeX = p.pos.vx;
                    t->homeY = p.pos.vy;
                    t->homeZ = p.pos.vz;
                    t->state++;
                case 1:
                    c = *t;
                    SnapToFloor(&c);
                    if (!(gFrameCount & 3)) {
                        SpawnEffectWithParams(9, t->homeX, t->homeY, t->homeZ, 0, 20, (d = c.y - t->homeY, d >= 0 ? d : -d), 0, 0);
                    }
                    if (gFrameCount % 10 == 0) {
                        PlaySe(0x6A);
                    }
                    if (!(gFrameCount & 1)) {
                        SpawnEffect(4, (s16)t->homeX - (50 - rand() % 100), (s16)c.y + (20 - rand() % 50), (s16)t->homeZ - (50 - rand() % 100), 0);
                    }
                    break;
            }
        }
    }
}

#include "engine/burst_raised_trap.inc"

#include "engine/turn_trap_on_approach.inc"

#include "engine/open_gate_on_approach.inc"

#include "engine/swing_gate_continuously.inc"

#include "engine/topple_trap_on_approach.inc"

#include "engine/scatter_trap_dust.inc"

#include "engine/shift_trap_on_approach.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepHoppingTrap);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F74FC);

#include "engine/swap_trap_model_on_approach.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F79A0);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", UpdateGoalEventHook);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F8264);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F82E0);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F8454);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F84A8);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F87E0);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F87E4);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F8960);

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

void RunTrapType04(StageTrap *t) {
    Speeds sp = sChargeRanges;
    s16 n;

    n = sp.v[t->act / 4];
    StepChargingTrap(t, t->act * 10 + 40, n);
}

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

void RunTrapType0B(StageTrap *t, s8 dir) {
    s16 v[2] = {60, 90};
    s16 n;

    n = v[t->act / 5];
    RunTrapThenTurnTimed(t, t->act * 10 + 60, n, dir);
}

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s16 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {}

void StepTrapType12(StageTrap *t, u16 speed, s32 arg) {}

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

void StepTrapType2A(StageTrap *t, s32 speed, s32 arg) {}

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

#include "engine/tumble_trap_act0_b.inc"

#include "engine/knock_trap_away_act0_c.inc"

#include "engine/tumble_trap_act12.inc"

void StepTrapAct1D(StageTrap *t) {}

#include "engine/draw_trap_while_linked.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", UpdateStageEventHook);

#include "engine_code.inc" /* UpdateGoalEvents */

void StepGoalSequence(void) {}

void CycleGoalLights(s8 arg) {}

#include "engine/reveal_start_cans.inc"

void func_800FA334(void) {}

void EmitTrapParticles(StageTrap *t) {
    s32 r;

    if ((gFrameCount & 3) == 0) {
        r = rand() % 10 + 15;
        SpawnEffectWithParams(12, t->x, t->y, t->z, 0, t->rotY + 0x800, r >= 0 ? r : -r, 20, 0);
    }
}

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA3EC);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA754);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA894);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA9D4);

INCLUDE_RODATA("asm/ovl/ovl06/nonmatchings/ovl06", D_800F08D4);

INCLUDE_RODATA("asm/ovl/ovl06/nonmatchings/ovl06", D_800F0924);
