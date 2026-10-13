/* Stage 4-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

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

    if (!(t->unk41 & 0x40)) {
        d = SquareRoot0(DistanceSquared((VECTOR *)t, (VECTOR *)&gPlayer.x));
        if (gSection - UpdatePathSegment(t) < 2) {
            t->unk41 |= 0x40;
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
        DrawSprite(0x133, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    }
}

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType0F);

#include "engine/step_trap_type12.inc"

#include "engine/step_trap_type1_f.inc"

#include "engine/step_rolling_trap.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {
    VECTOR v;
    u16 s;

    switch (t->state) {
        case 0:
            t->state++;
            PlaySe(0x11);
        case 1:
            StepRollingTrap(t, speed);
            DrawTrapWarning(t);
            t->timer++;
            if (LatchProximity(t, 1000) != 0) {
                PlaySe(0x60);
                PlaySe(0x78);
            }
            if (t->timer > arg) {
                t->state++;
                t->timer = 0;
                t->model += 2;
                t->homeRotY = t->rotY;
            }
            break;
        case 2:
            switch (dir) {
                case 1:
                    t->rotY += 0x38;
                    break;
                case 2:
                    t->rotY -= 0x38;
                    break;
            }
            SnapToFloor(t);
            t->timer++;
            s = speed - t->timer * 8;
            if (s < 10) {
                t->timer = 0;
                t->state++;
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                PlaySe(0x58);
                break;
            }
            PolarToXZ(s >> 2, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            if (!(t->timer & 1)) {
                v.vx = t->x - (50 - rand() % 100);
                v.vy = t->y;
                v.vz = t->z - (50 - rand() % 100);
                ScatterDust(&v);
            }
            break;
        case 3:
            StepRollingTrap(t, 0);
            break;
    }
}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {
    ModelSlot *m;
    ModelSlot *m2;
    s16 s;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
            PlaySe(0x11);
        case 1:
            StepTrapForward(t, speed);
            DrawTrapWarning(t);
            if (arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->homeRotZ = 0;
                t->state++;
                PlaySe(0x52);
            }
            break;
        case 2:
            s = Lerp(0x1000, 0, t->timer, 30);
            /* MATCHING: the table base in a local first, as in retail. */
            m = (ModelSlot *)gObjectModels;
            m[t->model].scale.vx = m[t->model].scale.vy = m[t->model].scale.vz = s;
            t->y -= 2;
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(0x54);
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            m2 = (ModelSlot *)gObjectModels;
            m2[t->model].scale.vx = m2[t->model].scale.vy = m2[t->model].scale.vz = 0x1000;
            break;
    }
}

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {
    StageTrap c;
    s32 near;

    near = gSection - UpdatePathSegment(t);
    switch (t->state) {
        case 0:
            t->timer = 0;
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (arg * arg > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->homeRotZ = 0;
                t->state++;
            }
            break;
        case 2:
            c = *t;
            SnapToFloor(&c);
            if (c.y - 100 < t->y) {
                t->state++;
            }
            t->homeRotZ++;
            t->y += t->homeRotZ;
            t->rotX += ((s16)((s16)speed / 3) << 12) / 360;
            if ((u16)(t->timer % 20) == 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x7C);
            }
            break;
        case 3:
            SnapToFloor(t);
            t->y -= 100;
            t->rotX += ((s16)((s16)speed / 3) << 12) / 360;
            PolarToXZ((s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (near < 2 && (u16)(t->timer % 20) == 0 && gPlayer.invulnTimer == 0 &&
                gPlayer.hittable == 1) {
                PlaySe(0x7C);
            }
            break;
    }
    t->timer++;
    if (near < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
    }
}

#include "engine/step_trap_type1_d.inc"

#include "engine/step_trap_type1_c.inc"

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

extern s8 D_800D36F0;
extern Vec3 D_8007AE1C[];
extern Vec3 D_8007AED0[];
s8 func_800F7D24(StageTrap *t);

/* MATCHING: the two stage groups set the counts separately; one set is
 * folded to a constant. */
void UpdateGoalEventHook(void) {
    StageTrap *t;
    StageTrap *u;
    s16 walk;
    s16 slide;

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
                walk = 35;
                slide = 100;
                break;
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                walk = 35;
                slide = 100;
                break;
            case 6:
            case 7:
            case 8:
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
                gPlayer.x = Lerp(t->homeX, D_8007AE1C[gStage].x, t->timer, walk);
                gPlayer.z = Lerp(t->homeZ, D_8007AE1C[gStage].z, t->timer, walk);
                t->timer++;
                if (t->timer > walk) {
                    t->timer = 0;
                    gPlayer.action = 0x17;
                    t->homeX = gPlayer.x;
                    t->homeY = gPlayer.y;
                    t->homeZ = gPlayer.z;
                    t->state++;
                }
                break;
            case 2:
                gPlayer.x = Lerp(gPlayer.x, D_8007AED0[gStage].x, t->timer, slide);
                gPlayer.z = Lerp(gPlayer.z, D_8007AED0[gStage].z, t->timer, slide);
                t->timer++;
                if (t->timer < (s16)(slide / 3)) {
                    ScatterDust((VECTOR *)&gPlayer.x);
                }
                if (t->timer > slide) {
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
                if (t->homeRotZ > 40) {
                    t->timer = 0;
                    gPlayer.action = 0x1B;
                    t->state++;
                }
                break;
            case 4:
                if (++t->timer > 240) {
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
                if (func_800F7D24(u) && gInEditor != 1) {
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
                    gStage = GetNextStage(9);
                    BeginLoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

#include "engine/step_stage_frame.inc"

void func_800F7D30(StageTrap *t, s16 period);

/* MATCHING: s32 with no return keeps the index shift in the jump table's
 * bound check delay slot. */
s32 func_800F74FC(StageTrap *t) {
    s32 home;
    s16 speed;

    switch (t->state) {
        case 0:
            t->homeRotX = 0;
            t->homeRotZ = 0;
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) < 2500 * 2500) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 2:
            home = DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t);
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) > 2500 * 2500) {
                speed = 5;
            } else {
                speed = 41;
                if (home < 10000 * 10000) {
                    func_800F7D30(t, 60);
                } else if (home < 20000 * 20000) {
                    func_800F7D30(t, 50);
                } else if (home < 30000 * 30000) {
                    func_800F7D30(t, 40);
                } else if (home < 40000 * 40000) {
                    func_800F7D30(t, 30);
                }
            }
            SnapToFloor(t);
            t->timer++;
            t->y -= 20;
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (home > 43000 * 43000) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            break;
        case 3:
            StepTrapForwardLevel(t, 0);
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 4:
            StepTrapForwardLevel(t, 0);
            break;
    }
}

s32 func_800F77FC(StageTrap *t, s8 kind) {
    PlacedPos p;
    StageTrap *s;
    s16 n;

    n = FindFreeSlot48();
    if (n == -1) {
        return -1;
    }
    s = &sSpawnedTraps[n];
    *s = *t;
    switch (kind) {
        case 0:
            s->homeRotZ = 50;
            break;
        case 1:
            s->homeRotZ = 150;
            break;
        case 2:
            s->homeRotZ = 250;
            break;
    }
    p.lx = 0;
    p.ly = 0;
    p.lz = 700;
    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
    UpdatePlacedPos(&p);
    s->x = s->homeX = p.pos.vx;
    s->y = s->homeY = p.pos.vy;
    s->z = s->homeZ = p.pos.vz;
    s->model = 0x24;
    s->type = 0xFF;
    s->act = rand() % 2 + 0x65;
    s->state = 0;
    s->timer = 90;
    return 0;
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(func_800F7968, func_800F77FC, 0x16C);

void func_800F79A0(StageTrap *t, s32 arg) {
    /* MATCHING: one local per case keeps each abs in its own register. */
    s32 d;
    s32 e;

    switch (t->state) {
        case 0:
            PolarToXZ(-10, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->timer += 10;
            t->rotX -= 0x71;
            d = rsin((t->timer << 12) / 360) * 150 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->timer > 180) {
                t->state++;
            }
            if ((u16)(t->timer % 180) == 0) {
                PlaySe(0x36);
            }
            break;
        case 1:
            PolarToXZ(-10, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->timer += 10;
            t->rotX -= 0x71;
            e = rsin((t->timer << 12) / 360) * arg >> 12;
            t->y = t->homeY - (e >= 0 ? e : -e);
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                if ((u16)(t->timer % 180) == 0) {
                    PlaySe(0x36);
                }
                if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
            } else {
                t->model = -1;
            }
            break;
    }
}

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F7D24);

void func_800F7D30(StageTrap *t, s16 period) {
    if (t->timer % period == 1) {
        func_800F77FC(t, t->homeRotZ % 3);
        t->homeRotZ++;
    }
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

#include "engine/run_trap_type04.inc"

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F82E0, RunTrapType09, 0x64);

#include "engine/run_trap_type0b.inc"

#include "engine_code.inc" /* RunTrapType32 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F8454, RunTrapType32, 0x30);
FUNC_ALIAS(func_800F84A8, RunTrapType32, 0x84);

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F87E4, RunTrapType30, 0x28);

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapFFAct68(StageTrap *t) {}

#include "engine/step_trap_act21.inc"

void StepTrapAct22(StageTrap *t) {}

void StepTrapAct23(StageTrap *t) {}

void StepTrapAct24(StageTrap *t) {}

void StepTrapAct25(StageTrap *t, s32 arg) {}

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType2A(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

void StepTrapType27(StageTrap *t, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {
    switch ((s8)arg) {
        case 0:
            t->rotY += 0xE3;
            break;
        case 1:
            t->rotX += 0xE3;
            break;
    }
    t->timer++;
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if ((u16)(t->timer % 20) == 0) {
            PlaySe(0x7B);
        }
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x5F);
    }
}

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */

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

void UpdateStageEventHook(void) {}

#include "engine_code.inc" /* UpdateGoalEvents */

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepMissSequence);

void StepGoalSequence(void) {}

void CycleGoalLights(s8 arg) {}

#include "engine/reveal_start_cans.inc"

#include "engine/emit_trap_particles.inc"

extern s32 D_800FA72C[];
extern s32 D_800FA74C[];

s32 func_800F9618(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800FA72C[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FA74C[1];
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
            if (LoadVabBank(5) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&D_800FA72C[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FA74C[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&D_800FA72C[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&D_800FA72C[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FA74C[3];
            gCdLoadSectors[1] = D_800FA74C[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&D_800FA72C[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FA74C[5];
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
            gCdLoadLoc[0] = (s32)&D_800FA72C[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FA74C[6];
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
            ResetStageObjects();
            StartStageOverlay();
            SetBootReady(1);
            ret = 1;
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}

void func_800F9980(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    gCameraYaw = 0;
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

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F9ABC);

/** @brief The shape of each of this stage's own sprites, as one block. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes41;

extern const SpriteShapes41 D_800F0924;

void func_800F9BFC(void) {
    SpriteShapes41 shapes;

    shapes = D_800F0924;
    *(SpriteShapes41 *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl12/nonmatchings/ovl12", D_800F08D4);

INCLUDE_RODATA("asm/ovl/ovl12/nonmatchings/ovl12", D_800F0924);
