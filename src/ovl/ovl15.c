/* Stage 5-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
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

#include "engine/step_trap_type2a.inc"

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

#include "engine/step_charging_trap.inc"

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

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepTrapType0F);

#include "engine/step_trap_type12.inc"

#include "engine/step_trap_type27.inc"

#include "engine/knock_trap_away.inc"

#include "engine/tumble_trap_away.inc"

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/step_smoking_trap.inc"

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
                    gPlayer.action = ACT_RIDE_STRAIGHT;
                    gPlayer.rideMode = (gPlayer.rideMode & RIDE_ITEM_MASK) | RIDE_COVER;
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
    if (gSwitches[190].state == SWITCH_ON) {
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                gMissState = -1;
                D_800D39D8[0].homeRotZ = 0;
                m->timer = 0;
                m->state++;
                D_800D39D8[0].homeX = gPlayer.x;
                D_800D39D8[0].homeY = gPlayer.y;
                D_800D39D8[0].homeZ = gPlayer.z;
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, 0x4EFF, m->timer, 25);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, -0x5428, m->timer, 25);
                if (++m->timer >= 26) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                gMissState = 0;
                gPlayer.action = 7;
                gSwitches[192].state = SWITCH_OFF;
                gPlayer.dropRide = 1;
                m->timer = 0;
                m->state++;
                gPlayer.rideMode &= 0xF0;
                PlaySe(0x37);
                break;
        }
    }
}

extern s8 D_800D36F0;
extern Vec3 D_8007AE1C[];
extern Vec3 D_8007AED0[];
/* MATCHING: main's view of the loader. */
void BeginLoadScreen(void);
s8 func_800F7E34(StageTrap *t);

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
                if (func_800F7E34(u) && gInEditor != 1) {
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
                    gStage = GetNextStage(12);
                    BeginLoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(func_800F74FC, UpdateGoalEventHook, 0x3B8);

#include "engine/step_stage_frame.inc"

void func_800F7C9C(StageTrap *t, s8 kind);

s32 func_800F7968(StageTrap *t) {
    s16 speed;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->homeRotZ = 0;
            t->state++;
        case 1:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) < 1500 * 1500) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 2:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) > 1500 * 1500) {
                speed = 5;
            } else {
                speed = 40;
                if (t->timer % 30 == 1) {
                    func_800F7C9C(t, rand() % 3);
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
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 9200 * 9200) {
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
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            break;
    }
}

/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(func_800F79A0, func_800F7968, 0x38);

void func_800F7C9C(StageTrap *t, s8 kind) {
    PlacedPos p;
    StageTrap *s;
    s16 i;

    i = FindFreeSlot48();
    if (i != -1) {
        s = &sSpawnedTraps[i];
        *s = *t;
        switch (kind) {
            case 2:
                p.lx = -150;
                break;
            case 1:
                p.lx = 150;
                break;
            case 0:
                p.lx = 0;
                break;
        }
        if (t->timer > 90) {
            p.lx += 50;
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

extern s32 D_800FF3C0[];

s8 func_800F7E34(StageTrap *t) {
    s8 ret = 0;

    switch (t->state) {
        case 0:
            D_800FF3C0[0] = gView[0];
            D_800FF3C0[2] = gView[2];
            gRefView[0] = rsin(gPlayer.yaw + 0x238) * 300 >> 12;
            gRefView[2] = rcos(gPlayer.yaw + 0x238) * 300 >> 12;
            t->timer = 0;
            t->state++;
        case 1:
            t->timer++;
            gView[0] = Lerp(D_800FF3C0[0], gRefView[0], t->timer, 60);
            gView[2] = Lerp(D_800FF3C0[2], gRefView[2], t->timer, 60);
            GsSetRefView2((GsRVIEW2 *)gView);
            gPlayer.bodyYaw = Lerp(gCourse.rot.vy + 0x800, gCourse.rot.vy + 0xC00, t->timer, 60);
            gBodyRot[1] = gPlayer.yaw + gPlayer.bodyYaw;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (t->timer > 60) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            gPlayer.action = 0;
            gPlayer.motion[0] = 2;
            gPlayer.drawX += rsin(gCourse.rot.vy + 0xC00) * 20 >> 12;
            gPlayer.drawZ += rcos(gCourse.rot.vy + 0xC00) * 20 >> 12;
            gBodyRot[1] = gPlayer.yaw + gPlayer.bodyYaw;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (++t->timer > 60) {
                ret = 1;
            }
            break;
    }
    return ret;
}

/** @brief One of this stage's on-screen prompts, in a table ended by a
 *         section of -1: shown when the player reaches its section. */
typedef struct {
    s16 section; /**< the course section that starts it */
    s16 limit;   /**< frames it shows before it is dismissed */
    s16 kind;    /**< which prompt it draws */
    s16 timer;   /**< frames shown so far */
    s16 unk8;    /**< not yet known; cleared with timer */
    s16 state;   /**< 0 waiting, 1 shown, 2 answered, 100 done */
} StageHint;

extern s32 D_800FB464;
extern StageHint D_800FB468[];

s32 func_800F80F4(void) {
    SVECTOR pos;
    s32 n;

    switch (D_800FB468[D_800FB464].state) {
        case 0:
            if (D_800FB468[D_800FB464].section == gSection) {
                D_800FB468[D_800FB464].state = 1;
            }
            break;
        case 1:
            switch (D_800FB468[D_800FB464].kind) {
                case 0:
                    if (gPadTrig & 0x8000) {
                        D_800FB468[D_800FB464].state = 2;
                    }
                    if (D_800FB468[D_800FB464].timer < 9) {
                        n = D_800FB468[D_800FB464].timer;
                    } else {
                        n = 8;
                    }
                    pos.vx = 196 - n * 30;
                    pos.vy = -20;
                    DrawSprite(0x161, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
                case 1:
                    if (gPadTrig & 0x2000) {
                        D_800FB468[D_800FB464].state = 2;
                    }
                    if (D_800FB468[D_800FB464].timer < 9) {
                        n = D_800FB468[D_800FB464].timer;
                    } else {
                        n = 8;
                    }
                    pos.vx = n * 30 - 284;
                    pos.vy = -20;
                    DrawSprite(0x162, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
                case 2:
                    if (gPadTrig & 0x40) {
                        D_800FB468[D_800FB464].state = 2;
                    }
                    if (D_800FB468[D_800FB464].timer < 9) {
                        n = D_800FB468[D_800FB464].timer;
                    } else {
                        n = 8;
                    }
                    pos.vx = -44;
                    pos.vy = 220 - n * 30;
                    DrawSprite(0x163, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
                case 3:
                    if (gPadTrig & 0x20) {
                        D_800FB468[D_800FB464].state = 2;
                    }
                    if (D_800FB468[D_800FB464].timer < 9) {
                        n = D_800FB468[D_800FB464].timer;
                    } else {
                        n = 8;
                    }
                    pos.vx = -44;
                    pos.vy = n * 30 - 260;
                    DrawSprite(0x133, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
            }
            if (++D_800FB468[D_800FB464].timer > D_800FB468[D_800FB464].limit) {
                D_800FB468[D_800FB464].state = 100;
            }
            break;
        case 2:
            D_800FB464++;
            break;
        case 100:
            D_800FB464++;
            break;
    }
}

/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the function above. */
FUNC_ALIAS(func_800F82E0, func_800F80F4, 0x1EC);
FUNC_ALIAS(func_800F8454, func_800F80F4, 0x360);

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F84A8, PlaySpriteHitSe, 0xC);
FUNC_ALIAS(func_800F87E4, RunTrapType01, 0xB4);

#include "engine/run_trap_type04.inc"

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

#include "engine/run_trap_type0b.inc"

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

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

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1D(StageTrap *t, u16 speed) {}

void RunTrapAheadAtHeight(StageTrap *t, u16 speed) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */

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

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

#include "engine/draw_trap_while_linked.inc"

#include "engine_code.inc" /* UpdateGoalEvents */

void StepGoalSequence(void) {}

void CycleGoalLights(s8 arg) {}

#include "engine/reveal_start_cans.inc"

extern s32 D_800FB4F8;

void func_800F9AC8(void) {
    s32 i;

    for (i = 0; i < D_800FB4F8; i++) {
        D_800FB468[i].timer = 0;
        D_800FB468[i].unk8 = 0;
        D_800FB468[i].state = 0;
    }
    D_800FB464 = 0;
}

#include "engine/emit_trap_particles.inc"

extern s32 D_800FADA8[];
extern s32 D_800FADC8[];

s32 func_800F9BC0(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800FADA8[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FADC8[1];
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
            gCdLoadLoc[0] = (s32)&D_800FADA8[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FADC8[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&D_800FADA8[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&D_800FADA8[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FADC8[3];
            gCdLoadSectors[1] = D_800FADC8[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&D_800FADA8[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FADC8[5];
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
            gCdLoadLoc[0] = (s32)&D_800FADA8[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FADC8[6];
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

/* Resets the player, the camera and the course to the stage's start. */
void func_800F9F28(void) {
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
u16 func_800FA060(s16 id, StageTrap *src, StageTrap *dst) {
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

/** @brief This stage's sprite shapes, as one block: 41 of them where
 *         the engine's SpriteShapes has 100. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes41;

extern const SpriteShapes41 D_800F093C;

void func_800FA1A0(void) {
    SpriteShapes41 shapes;

    shapes = D_800F093C;
    *(SpriteShapes41 *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl15/nonmatchings/ovl15", D_800F093C);
