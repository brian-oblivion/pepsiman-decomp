/* Stage 1-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
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
        /* MATCHING: the model table in a local, so its base is built before
         * the trap table's. */
        m = gObjectModels;
        t = (StageTrap *)gTraps;
        a = (ModelSlot *)&m[t[194].model];
        b = (ModelSlot *)&m[t[195].model];
        b->scale.vy = a->scale.vy = b->scale.vx = a->scale.vx = 0x1000;
    }
}

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

void StepTrapAct27(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;

    /* MATCHING: the model in a local keeps it in a saved register. */
    model = 0x44;
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x39);
                }
                t->timer = 0;
                t->state++;
            }
            break;
        case 1:
            t->timer++;
            PolarToXZ(50, (gCourse.rot.vy + 0x800) * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                c.rotY = gCourse.rot.vy;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: timer * 4 + timer, not * 5: it loads the table first. */
                c.x += sBurstDirs[i][0] * (t->timer * 4 + t->timer);
                c.z += sBurstDirs[i][1] * (t->timer * 4 + t->timer);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                if (t->timer & 3) {
                    DrawObjectModel(&gObjectModels[c.model], &c);
                }
            }
            if (t->timer > 40) {
                t->model = -1;
                t->state++;
            }
            break;
    }
}

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

#ifdef NON_MATCHING
/* Matches with speed declared u16 (and -speed); that needs engine.inc's
 * prototype and the other overlays' stubs changed to u16. */
void StepTrapType2A(StageTrap *t, s32 speed, s32 arg) {
    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            SnapToFloor(t);
            t->y -= 20;
            PolarToXZ(-(u16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
            }
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(55);
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType2A);
#endif

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

#ifdef NON_MATCHING
void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {
    StageTrap c;
    s32 unused[8];

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
                    case 1:
                    case 3:
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
            c = *t;
            c.model++;
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
                if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 &&
                    gPlayer.hittable == 1) {
                    PlaySe(0x37);
                }
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType0F);
#endif

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

void StepTrapFFAct67(StageTrap *t) {
    s32 h;

    if (t->state == 0) {
        PolarToXZ(-10, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        t->timer += 10;
        h = rsin(t->timer * 4096 / 360) * 200 >> 12;
        t->y = t->homeY - (h >= 0 ? h : -h);
        if (t->timer >= 180) {
            PlaySe(0x49);
            t->state++;
        }
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
}

void StepTrapFFAct68(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;
    s32 h;
    u16 yaw;

    model = 12;
    switch (t->state) {
        case 0:
            PolarToXZ(t->homeRotZ, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->timer += 10;
            h = rsin((t->timer / t->homeX << 12) / 360) * t->homeRotX >> 12;
            /* MATCHING: h >= 0 ? h : -h folds to one abs insn, which the
             * scheduler moves loads across; an if gives a branch. */
            t->y = t->homeY - (h >= 0 ? h : -h);
            if (t->timer > 180) {
                t->homeX = 3;
                t->homeRotX = 10;
                t->homeRotZ = -5;
            }
            if (t->timer > 360) {
                t->homeX = 8;
                t->homeRotX = 2;
                t->homeRotZ = -2;
            }
            if (t->timer > 540) {
                t->homeRotX = 0;
                t->homeRotZ = 0;
            }
            if (t->timer == 180) {
                PlaySe(0x38);
            }
            if (*(s32 *)t->unk44 != 3) {
                t->homeZ -= ++*(s16 *)t->unk38;
                yaw = gCourse.rot.vy;
                if (t->homeZ < 0) {
                    t->homeZ = 0;
                }
                if (*(s32 *)t->unk44 == 2) {
                    yaw += (t->homeZ << 12) / 360;
                }
                if (*(s32 *)t->unk44 == 1) {
                    yaw += -(t->homeZ << 12) / 360;
                }
                t->rotY = yaw;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                PlaySe(0x38);
                t->timer = 0;
                t->state++;
            }
            break;
        case 1:
            t->timer++;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                c.rotY = gCourse.rot.vy;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                c.x += sBurstDirs[i][0] * (t->timer * 4 + t->timer);
                c.z += sBurstDirs[i][1] * (t->timer * 4 + t->timer);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                if (t->timer % 3) {
                    DrawObjectModel(&gObjectModels[c.model], &c);
                }
            }
            if (t->timer > 40) {
                t->model = -1;
                t->state++;
            }
            break;
    }
}

#include "engine/step_smoking_tumble_trap.inc"

#include "engine/step_smoking_trap.inc"

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine/slide_trap_away.inc"

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine/step_dash_board.inc"

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

#include "engine/open_gate_on_approach.inc"

#include "engine/swing_gate_continuously.inc"

void StepHoppingTrap(StageTrap *t, u8 act) {
    VECTOR v;
    s32 off;
    s32 amp;
    s16 r;
    s16 ang;
    s32 d;

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
        /* MATCHING: A >= 0 ? A : -A is cc1's one-insn abs; the spread is
         * (rand() % 10 * 10), not * 10 * (rand() % 10). */
        if ((t->x + gWorldShift[0] >= 0 ? t->x + gWorldShift[0] : -(t->x + gWorldShift[0])) <
            2000) {
            if ((t->z + gWorldShift[2] >= 0 ? t->z + gWorldShift[2] : -(t->z + gWorldShift[2])) <
                    2000 &&
                t->timer % 3 == 0) {
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

#include "engine/swap_trap_model_on_approach.inc"
FUNC_ALIAS(func_800F74FC, SwapTrapModelOnApproach, 0x12C);

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
                    gPlayer.action = 0x5E;
                    gPlayer.rideMode = (gPlayer.rideMode & 0xF0) | 3;
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

void UpdateStageEventHook(void) {
    StageTrap *m;

    m = &D_800D39D8[1];
    if (gSwitches[190].state == 1) {
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                gMissState = -1;
                D_800D39D8[0].homeRotZ = 0;
                m->timer = 0;
                D_800D39D8[0].homeX = gPlayer.x;
                D_800D39D8[0].homeY = gPlayer.y;
                D_800D39D8[0].homeZ = gPlayer.z;
                m->state++;
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, 0x1DCE, m->timer, 20);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, 0x4844, m->timer, 20);
                if (++m->timer > 20) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                gMissState = 0;
                gPlayer.action = 7;
                gSwitches[192].state = 2;
                gPlayer.dropRide = 1;
                m->timer = 0;
                m->state++;
                gPlayer.rideMode &= 0xF0;
                PlaySe(0x37);
                break;
        }
    }
}
FUNC_ALIAS(func_800F7968, UpdateStageEventHook, 0x18);
FUNC_ALIAS(func_800F79A0, UpdateStageEventHook, 0x50);

extern Vec3 D_8007AE1C[];
extern Vec3 D_8007AED0[];
extern s8 D_800D36F0;
s8 func_800F8DF0(StageTrap *t);
/* MATCHING: stage's, declared only where defined. */
void BeginLoadScreen(void);

void UpdateGoalEventHook(void) {
    StageTrap *t;
    StageTrap *u;
    s16 walk;
    s16 run;

    t = sGoalTraps;
    u = &sGoalTraps[1];
    if (D_800D36F0 == 1) {
        ResetRumble();
        /* MATCHING: a body per case, which cross-jumping merges after the
         * jump table is built; one shared body lets cse fold the constants. */
        switch (gStage) {
            case 0:
                walk = 35;
                run = 100;
                break;
            case 1:
                walk = 35;
                run = 100;
                break;
            case 2:
                walk = 35;
                run = 100;
                break;
            case 3:
                walk = 35;
                run = 100;
                break;
            case 4:
                walk = 35;
                run = 100;
                break;
            case 5:
                walk = 35;
                run = 100;
                break;
            case 9:
                walk = 35;
                run = 100;
                break;
            case 10:
                walk = 35;
                run = 100;
                break;
            case 11:
                walk = 35;
                run = 100;
                break;
            case 12:
                walk = 35;
                run = 100;
                break;
            case 13:
                walk = 35;
                run = 100;
                break;
            case 14:
                walk = 35;
                run = 100;
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
                gPlayer.x = Lerp(gPlayer.x, D_8007AED0[gStage].x, t->timer, run);
                gPlayer.z = Lerp(gPlayer.z, D_8007AED0[gStage].z, t->timer, run);
                t->timer++;
                if (t->timer < (s16)(run / 3)) {
                    ScatterDust((VECTOR *)&gPlayer.x);
                }
                if (t->timer > run) {
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
                if (func_800F8DF0(u) && gInEditor != 1) {
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
                if (++t->timer > 8) {
                    gStage = GetNextStage(0);
                    BeginLoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

/* MATCHING: engine/step_stage_frame.inc with a 120-byte unused local, which
 * gives this stage's frame. */
void func_800F809C(void) {
    s32 unused[30];
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

void func_800F85F0(StageTrap *t, s8 arg);

void func_800F82E0(StageTrap *t) {
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
            if (t->timer % 20 == 1) {
                func_800F85F0(t, (s16)t->homeRotZ % 3);
                t->homeRotZ++;
            }
            SnapToFloor(t);
            t->timer++;
            t->y -= 20;
            t->rotY = GetPathDirection(t);
            PolarToXZ(40, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 0xB06440) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            break;
        case 3:
            InterpRange(t->homeRotY, t->homeRotY - 0x400, t->timer, 30, gOffsetVec);
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
FUNC_ALIAS(func_800F8454, func_800F82E0, 0x174);
FUNC_ALIAS(func_800F84A8, func_800F82E0, 0x1C8);

void func_800F85F0(StageTrap *t, s8 arg) {
    PlacedPos p;
    StageTrap *s;
    s16 i;
    s32 r;

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
                p.lx = -20;
                break;
        }
        PlaySe(0x61);
        p.ly = 0;
        p.lz = 0;
        p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
        UpdatePlacedPos(&p);
        s->x = s->homeX = p.pos.vx;
        s->y = s->homeY = p.pos.vy;
        s->z = s->homeZ = p.pos.vz;
        r = rand();
        s->type = 0xFF;
        s->act = 0x67;
        s->rotY += 0x800;
        s->state = 0;
        s->timer = 20;
        s->model = r % 3 + 17;
        s->homeRotY += (rand() % 3 - 1) * 227;
        PlaySe(0x2A);
    }
}

void func_800F8B2C(StageTrap *t, s8 arg);

void func_800F87E4(StageTrap *t) {
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
                if (t->timer % 45 == 1) {
                    func_800F8B2C(t, (s16)t->homeRotZ % 3);
                    t->homeRotZ++;
                }
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
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 0x3715900) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            break;
        case 3:
            InterpRange(t->homeRotY, t->homeRotY + 0x400, t->timer, 30, gOffsetVec);
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

void func_800F8B2C(StageTrap *t, s8 arg) {
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
                p.lx = 20;
                break;
        }
        PlaySe(0x61);
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
        s->rotY += 0x800;
        s->model = m;
        d = (rand() % 3 - 1) * 227;
        s->homeRotX = 100;
        s->homeRotZ = -10;
        s->homeX = 1;
        *(s32 *)s->unk44 = arg;
        s->homeRotY += d;
        r = rand() % 720 + 360;
        *(s16 *)s->unk38 = 0;
        s->homeZ = r;
    }
}

s8 func_800F8DF0(StageTrap *t) {
    return t->state == 0;
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", RunTrapType0B);

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s16 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

void StepTrapType1F(StageTrap *t, u16 arg) {}

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapAct21(StageTrap *t) {}

void StepTrapAct22(StageTrap *t) {}

void StepTrapAct23(StageTrap *t) {
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x3B);
    }
}

void StepTrapAct24(StageTrap *t) {
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x3F);
    }
}

void StepTrapAct25(StageTrap *t, s32 arg) {}

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapAct28);

#include "engine_code.inc" /* ArmTrapByRange */

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

#include "engine/draw_trap_warning.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

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

#include "engine/tumble_trap_act0_b.inc"

#include "engine/knock_trap_away_act0_c.inc"

#include "engine/tumble_trap_act12.inc"

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepTrapAct1D(StageTrap *t) {}

void BurstRaisedTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void ToppleTrapOnApproach(StageTrap *t, s32 side, s32 speed, u8 kind) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

#include "engine/draw_trap_while_linked.inc"

#include "engine_code.inc" /* UpdateGoalEvents */

void StepGoalSequence(void) {}

void CycleGoalLights(s8 arg) {}

void func_800FA5D0(void) {
    ClearCanGroups();
    gCanGroupOn[1] = 1;
    gCanGroupOn[2] = 1;
    gCanGroupOn[4] = 1;
    gCanGroupOn[0x32] = 1;
    ShowRevealedCans();
}

void func_800FA610(void) {}

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", EmitTrapParticles);

extern s32 D_800FB55C[];
extern s32 D_800FB57C[];

s32 func_800FA6C8(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800FB55C[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB57C[1];
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
            if (LoadVabBank(2) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&D_800FB55C[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB57C[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&D_800FB55C[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&D_800FB55C[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB57C[3];
            gCdLoadSectors[1] = D_800FB57C[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&D_800FB55C[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB57C[5];
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
            gCdLoadLoc[0] = (s32)&D_800FB55C[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB57C[6];
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

#ifdef NON_MATCHING
/* MATCHING: model's, with this unit's view of the position. */
s32 FindFloor(s32 *pos);

void func_800FAA30(void) {
    s16 yaw;
    StartRec *st;

    st = gStageStart;
    gCameraYaw = 0;
    sCamHeight = -150;
    sViewSpeed = 100;
    gViewerAngle = 0xD8;
    gFellOff = 0;
    gPlayLive = 1;
    sCamOffset[0] = 0;
    gView[1] = 0;
    yaw = -st->yaw;
    sCamOffset[1] = 0;
    gPlayer.motion[0] = 2;
    gPlayer.control = 1;
    gPlayer.hittable = 1;
    gCourse.rot.vy = yaw;
    gCourse.rot.vz = yaw;
    gPlayer.startYaw = gPlayer.yaw = yaw;
    gFloorIndex = 0;
    gSteerYaw = yaw;
    gCourse.x = gPlayer.x = -st->x;
    gSteerYawHalf = yaw;
    gCourse.y = gPlayer.y = -st->y;
    gCourse.z = gPlayer.z = -st->z;
    gFloorIndex = FindFloor(&gPlayer.x);
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gSection = 0;
    gView[1] = (s16)(gCourse.y - 150);
    func_800FA5D0();
}
#else
INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FAA30);
#endif

extern const SpriteShapes D_800F08FC;

void func_800FAB9C(void) {
    SpriteShapes shapes;

    shapes = D_800F08FC;
    *(SpriteShapes *)sSpriteShapes = shapes;
}

/** @brief A table of 80 trap models, copied whole. */
typedef struct {
    u8 b[80]; /**< a model per entry */
} Models80;

extern const Models80 D_800F0960;

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register (as ovl07's func_800FA580). */
u16 func_800FAD08(s16 id, StageTrap *src, StageTrap *dst) {
    u16 ret = 1;
    Models80 models = D_800F0960;

    if (id < 80) {
        if (id >= 0) {
            dst->homeX = src->x;
            dst->homeY = src->y;
            dst->homeZ = src->z;
            dst->homeRotX = src->rotX;
            dst->homeRotY = src->rotY;
            dst->homeRotZ = src->rotZ;
            ResetObj34(dst);
            dst->model = models.b[id];
            ret = 1;
        }
    }
    return ret;
}

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F08FC);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F0960);
