/* Stage 1-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", ResetSpawnedTraps);

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapAct27);

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType2A);

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType0F);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapFFAct67);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapFFAct68);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepHoppingTrap);

#include "engine/swap_trap_model_on_approach.inc"
FUNC_ALIAS(func_800F74FC, SwapTrapModelOnApproach, 0x12C);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepMissSequence);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", UpdateGoalEventHook);

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

s32 func_800F8DF0(StageTrap *t) {
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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapAct23);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapAct24);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FA6C8);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FAD08);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F08FC);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F0960);
