/* Stage 4-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

/* MATCHING: the engine's callers pass StepTrapType18 a word speed; this
 * stage's body takes it as s16, so the shared declaration is set aside. */
#define StepTrapType18 StepTrapType18_engine
#include "engine.inc"
#undef StepTrapType18
void StepTrapType18(StageTrap *t, s16 speed, s32 arg, s8 dir);

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

#ifdef NON_MATCHING
void ResetSpawnedTraps(void) {
    u32 i;
    StageTrap *t;
    ModelSlot *a;
    ModelSlot *b;

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
        t = (StageTrap *)gTraps;
        a = (ModelSlot *)&gObjectModels[t[194].model];
        b = (ModelSlot *)&gObjectModels[t[195].model];
        a->scale.vy = b->scale.vy = a->scale.vx = b->scale.vx = 0x1000;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", ResetSpawnedTraps);
#endif

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

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s32 arg, s8 dir) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
            PlaySe(0x11);
        case 1:
            StepTrapForward(t, speed);
            DrawTrapWarning(t);
            if (gSection - UpdatePathSegment(t) < 2) {
                if (++t->homeRotZ > arg) {
                    t->timer = 0;
                    t->state++;
                    t->homeRotY = t->rotY;
                }
            }
            break;
        case 2:
            switch (dir) {
                case 2:
                    InterpRange(t->homeRotY, t->homeRotY - 0x400, t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
                case 1:
                    InterpRange(t->homeRotY, t->homeRotY + 0x400, t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
            }
            StepTrapForward(t, speed);
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            StepTrapForwardLevel(t, speed);
            break;
    }
}

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

#ifdef NON_MATCHING
void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {
    StageTrap copy;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
            PlaySe(0x53);
            if (dir == 3) {
                PlaySe(0x11);
            }
        case 1:
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(0x55);
            }
            if (SnapToFloor(t) == -1) {
                t->y = t->homeY;
            }
            t->y -= 120;
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (speed != 0) {
                switch (dir) {
                    case 2:
                        t->rotY = GetPathDirection(t) + 0x800;
                        break;
                    case 3:
                        DrawTrapWarning(t);
                    case 1:
                        t->rotY = GetPathDirection(t);
                        break;
                    case 4:
                        PolarToXZ(-(speed * 2), t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
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
                    PlaySe(0x37);
                }
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType0F);
#endif


#include "engine/step_trap_type12.inc"

void StepTrapType1F(StageTrap *t, u16 arg) {
    s32 d;

    switch (t->state) {
        case 0:
            t->homeRotZ = (rand() % 3 + 1) * 30;
            t->timer = rand() % 360;
            t->homeX = t->y;
            t->state++;
        case 1:
            t->y = t->homeX - 30;
            d = rsin((t->timer * 20 << 12) / 360) * t->homeRotZ >> 12;
            t->homeY = -(d >= 0 ? d : -d);
            if (t->homeY > 0) {
                t->y = t->homeY;
            }
            PolarToXZ(arg, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->rotX += (arg << 12) / 360;
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x3F);
            }
            break;
    }
}

#include "engine/step_rolling_trap.inc"

void StepTrapType1D(StageTrap *t, u16 speed) {
    VECTOR v;
    StageTrap *base;
    u16 phase;

    base = t;
    switch (t->state) {
        case 0:
            SnapToFloor(t);
            t->homeY = t->y;
            t->state++;
        case 1:
            base->y = base->homeY;
            base->y -= (u16)(t->timer % 10) * 3;
            PolarToXZ(speed, base->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            base->x += gOffsetVec[0] >> 12;
            base->z += gOffsetVec[2] >> 12;
            t->timer++;
            phase = t->timer % 20;
            t++;
            if (phase < 15) {
                t->rotX = Lerp(0x2AA, -0x2AA, phase, 15);
            } else {
                t->rotX = Lerp(-0x2AA, 0x2AA, phase - 15, 5);
            }
            t->y = base->y - 100;
            t->rotY = base->rotY;
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->model = base->model + 1;
            t++;
            if (phase < 15) {
                t->rotX = Lerp(-0x2AA, 0x400, phase, 15);
            } else {
                t->rotX = Lerp(0x400, -0x2AA, phase - 15, 5);
            }
            t->y = base->y - 100;
            t->rotY = base->rotY;
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->model = base->model + 2;
            t = base;
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            t++;
            DrawObjectModel(&gObjectModels[t->model], t);
            t->model = -1;
            t++;
            DrawObjectModel(&gObjectModels[t->model], t);
            t->model = -1;
            if (base->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x1C);
            }
            v.vx = t->x - (25 - rand() % 50);
            v.vy = t->y + 70;
            v.vz = t->z - (25 - rand() % 50);
            ScatterDust(&v);
            if (LatchProximity(t, 1000) != 0) {
                PlaySe(0x79);
            }
            break;
    }
}

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
                    PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
                    t->x -= gOffsetVec[0] >> 12;
                    t->z -= gOffsetVec[2] >> 12;
                    if (arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                        t->timer = 0;
                        t->homeRotZ = 0;
                        t->state++;
                    }
                    break;
                case 1:
                    t->rotY = GetPathDirection(t) + 0x800;
                    t->homeRotY = t->rotY + 0x800;
                    PolarToXZ(speed, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
                    t->x += gOffsetVec[0] >> 12;
                    t->z += gOffsetVec[2] >> 12;
                    break;
            }
            break;
        case 2:
            PolarToXZ(Lerp(speed, 0, t->timer, 15), t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
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
        PlaySe(0x37);
    }
dust:
    if (t->state < 3 &&
        (t->x + gWorldShift[0] >= 0 ? t->x + gWorldShift[0] : -(t->x + gWorldShift[0])) < 2000 &&
        (t->z + gWorldShift[2] >= 0 ? t->z + gWorldShift[2] : -(t->z + gWorldShift[2])) < 2000 &&
        (u16)(t->timer % 3) == 0) {
        v.vx = t->x + (rsin((t->rotY + 0x800) * (10 * (rand() % 20))) >> 12);
        v.vz = t->z + (rcos((t->rotY + 0x800) * (10 * (rand() % 20))) >> 12);
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
                PolarToXZ((s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
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
            PolarToXZ(Lerp((s16)speed, 0, t->timer, 15), t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
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
                PlaySe(0x38);
            }
            break;
    }
}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            DrawObjectModel(&gObjectModels[t->model], t);
            break;
        case 1:
            t->timer++;
            switch ((s8)arg) {
                case 2:
                    t->rotZ = -Lerp(0, 0x400, t->timer, 10);
                    break;
                case 1:
                    t->rotZ = Lerp(0, 0x400, t->timer, 10);
                    break;
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
                PlaySe(0x3D);
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 2:
            t->timer++;
            t->y -= 10;
            if (t->timer == 2) {
                t->timer = 0;
                t->state++;
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 3:
            t->timer++;
            t->y += 10;
            if (t->timer == 4) {
                t->timer = 0;
                t->state++;
                PlaySe(0x3D);
            }
        case 4:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
    }
}

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
                v.vx = t->x + (rsin((t->rotY + 0x800) * (10 * (rand() % 20))) >> 12);
                v.vz = t->z + (rcos((t->rotY + 0x800) * (10 * (rand() % 20))) >> 12);
                v.vy = t->y + 100;
                ScatterDust(&v);
            }
            PolarToXZ((s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
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

void StepTrapAct1D(StageTrap *t) {
    VECTOR v;

    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x36);
                }
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x7D);
                }
            }
            break;
        case 1:
            t->timer++;
            t->rotX = Lerp(0, -0x400, t->timer, 5);
            if (t->timer == 5) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 2:
            t->timer++;
            if ((u16)(t->timer % 3) == 0) {
                v.vx = t->x + (rsin((t->rotY + 0x800) * (10 * (rand() % 10))) >> 12);
                v.vz = t->z + (rcos((t->rotY + 0x800) * (10 * (rand() % 10))) >> 12);
                v.vy = t->y;
                ScatterDust(&v);
            }
            if (t->timer == 20) {
                t->state++;
            }
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

#include "engine/burst_raised_trap.inc"

#include "engine/topple_trap_on_approach.inc"

#include "engine/scatter_trap_dust.inc"

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
                gFellOff = 0;
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
                gPlayer.x = Lerp(D_800D39D8[0].homeX, D_8007AD68[gStage].x, m->timer, 45);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, D_8007AD68[gStage].z, m->timer, 45);
                if (++m->timer >= 46) {
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
                    gPlayer.rideMode = (gPlayer.rideMode & RIDE_ITEM_MASK) | RIDE_ROLLER;
                    gSpeedFixed = 800;
                    SpawnMissDebris();
                    gMissState = 0;
                    gFellOff = 0;
                    D_800D39D8[1].state = 0;
                    D_800D39D8[1].timer = 0;
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
FUNC_ALIAS(func_800F74FC, StepMissSequence, 0x2FC);

s8 func_800F7A88(StageTrap *t);

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
                if (func_800F7A88(u)) {
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

void CycleGoalLights(s8 arg) {
    u16 periods[8] = {20, 40, 30, 40, 20, 30, 20, 35};
    s16 i;

    if (gSectionEdges[-1] - gSection < 20) {
        for (i = 0; i < 8; i++) {
            if (gFrameCount % periods[i] == 0) {
                LoadTimRange((u8 *)0x8018D000, i * 4 + arg * 2, 1);
            } else if (gFrameCount % periods[i] == periods[i] >> 1) {
                LoadTimRange((u8 *)0x8018D000, i * 4 + 1 + arg * 2, 1);
            }
        }
    }
}

/* The engine's StepStageFrame, under this overlay's name for it. */
#define StepStageFrame func_800F7844
#include "engine/step_stage_frame.inc"
#undef StepStageFrame
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the function above. */
FUNC_ALIAS(func_800F7968, func_800F7844, 0x124);
FUNC_ALIAS(func_800F79A0, func_800F7844, 0x15C);

extern Vec3 D_8007AE1C[];
extern Vec3 D_8007AED0[];

/* MATCHING: frame counts as s16 locals set at the top keep saved registers. */
s8 func_800F7A88(StageTrap *t) {
    s8 ret = 0;
    s16 n = 35;
    s16 m = 100;

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
            if (++t->timer > 30) {
                gPlayer.action = 0x19;
                gFellOff = 1;
                gRefView[0] = rsin(gCourse.rot.vy + 0x71D) * 200 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x71D) * 200 >> 12;
                gRefView[1] = gPlayer.y - 80;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 50 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 50 >> 12;
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
                PlaySe(0x22);
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
                PlaySe(0x33);
            }
            if (IsScreenWipeActive() == 0) {
                t->state++;
                gView[0] = rsin(gCourse.rot.vy + 0x800) * 500 >> 12;
                gView[2] = rcos(gCourse.rot.vy + 0x800) * 500 >> 12;
                gView[1] = t->homeY - 150;
                gView[3] = rsin(gCourse.rot.vy + 0x800) * 700 >> 12;
                gView[5] = rcos(gCourse.rot.vy + 0x800) * 700 >> 12;
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
FUNC_ALIAS(func_800F82E0, RunTrapTypeFF, 0xF4);
FUNC_ALIAS(func_800F8454, RunTrapType02, 0x7C);

void RunTrapType04(StageTrap *t) {
    Speeds sp = sChargeRanges;
    s16 range = sp.v[t->act / 4];

    StepChargingTrap(t, t->act * 10 + 40, range);
}

FUNC_ALIAS(func_800F84A8, RunTrapType04, 0x3C);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

#include "engine/run_trap_type0b.inc"

#include "engine_code.inc" /* RunTrapType32 */

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F87E4, RunTrapType10, 0x48);

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapFFAct68(StageTrap *t) {}

void StepTrapAct21(StageTrap *t) {
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x3D);
    }
}

void StepTrapAct23(StageTrap *t) {}

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapAct24);

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

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

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

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", LoadTimRange);

#include "engine/reveal_start_cans.inc"

#include "engine/emit_trap_particles.inc"

extern s32 D_800FABD0[];
extern s32 D_800FABF0[];

s32 func_800F98E4(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800FABD0[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FABF0[1];
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
            gCdLoadLoc[0] = (s32)&D_800FABD0[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FABF0[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&D_800FABD0[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&D_800FABD0[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FABF0[3];
            gCdLoadSectors[1] = D_800FABF0[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&D_800FABD0[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FABF0[5];
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
            gCdLoadLoc[0] = (s32)&D_800FABD0[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FABF0[6];
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
            gCdLoadLoc[0] = (s32)&D_800FABD0[6];
            gCdLoadDest[0] = 0x8018D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FABF0[7];
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
u16 func_800F9CA4(s16 id, StageTrap *src, StageTrap *dst) {
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

#ifdef NON_MATCHING
void func_800F9F18(void) {
    s16 yaw;

    gCameraYaw = 0x400;
    sCamHeight = -150;
    sViewSpeed = 100;
    gViewerAngle = 0xD8;
    gFellOff = 0;
    gPlayLive = 1;
    sCamOffset[0] = 0;
    gView[1] = 0;
    sCamOffset[1] = 0;
    yaw = -gStageStart->yaw;
    gPlayer.motion[0] = 2;
    gPlayer.control = 1;
    gPlayer.hittable = 1;
    gCourse.rot.vy = yaw;
    gCourse.rot.vz = yaw;
    gPlayer.startYaw = gPlayer.yaw = yaw;
    gFloorIndex = 0;
    gCourse.x = gPlayer.x = -gStageStart->x;
    gSteerYaw = yaw;
    gCourse.y = gPlayer.y = -gStageStart->y;
    gSteerYawHalf = yaw;
    gCourse.z = gPlayer.z = -gStageStart->z;
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gView[1] = (s16)(gCourse.y - 150);
    RevealStartCans();
}
#else
INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F9F18);
#endif

/** @brief The shape of each of this stage's own sprites, as one block. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes41;

extern const SpriteShapes41 D_800F08FC;

void func_800FA058(void) {
    SpriteShapes41 shapes;

    shapes = D_800F08FC;
    *(SpriteShapes41 *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl13/nonmatchings/ovl13", D_800F08FC);
