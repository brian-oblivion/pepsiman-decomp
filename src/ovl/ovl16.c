/* Stage 5-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
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

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepTrapType32);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepTrapType20);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepTrapType29);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepTrapType27);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

#include "engine/burst_raised_trap.inc"

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", ToppleTrapOnApproach);

#include "engine/scatter_trap_dust.inc"

#include "engine/swap_trap_model_on_approach.inc"

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepGoalSequence);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", CycleGoalLights);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F5E80);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F04D8);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F04E8);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F60CC);

void func_800F6FA0(void) {
    s16 v;

    if (gPlayer.control != 0 && (u32)(gPlayer.motion[0] - MOT_RIDE) < 5) {
        v = gPlayer.speed;
        if (v < 100) {
            gSpeedFixed += 16;
        } else if (gPad & 0x1000) { /* PADLup */
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 100) {
                gSpeedFixed = 0x640;
            }
        } else if (gPad & 0x4000) { /* PADLdown */
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 100) {
                gSpeedFixed = 0x640;
            }
        } else if (v > 100) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        gPlayer.speed = (s16)gSpeedFixed >> 4;
    }
}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F70A8);

extern s16 sSteerDrift;
/* MATCHING: player's, declared only where defined. */
void ApplySteer(u8 mode, u16 a, s16 b);
void RecentreSteer(void);

extern s16 sSteerDrift;
/* MATCHING: player's, declared only where defined. */
void ApplySteer(u8 mode, u16 a, s16 b);
void RecentreSteer(void);

void func_800F729C(void) {
    if (gPad & 0x8000) { /* PADLleft */
        ApplySteer(3, 16, 15);
        sSteerDrift -= 3;
        gPlayer.action = 0x35;
    } else if (gPad & 0x2000) { /* PADLright */
        ApplySteer(2, 16, 15);
        sSteerDrift += 3;
        gPlayer.action = 0x37;
    } else {
        RecentreSteer();
        gPlayer.action = 0x33;
        if (sSteerDrift != 0) {
            if (sSteerDrift < 0) {
                sSteerDrift++;
            } else {
                sSteerDrift--;
            }
        }
    }
    if (gPadType == 0) {
        RecentreSteer();
    }
    sSteerDrift = sSteerDrift < -50 ? -50 : sSteerDrift > 50 ? 50 : sSteerDrift;
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F74FC, StartSpriteWalk, 0x3C);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F7968, RunTrapType0A, 0x58);
FUNC_ALIAS(func_800F79A0, RunTrapType0A, 0x90);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", RunTrapType0B);

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

void StepTrapAct25(StageTrap *t, s32 arg) {
    u8 unused[0x78];

    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
    }
}
/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(func_800F82E0, StepTrapAct25, 0x78);

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void RunTrapAhead(StageTrap *t, s16 speed) {}

void RunTrapAheadNonstop(StageTrap *t, s16 speed) {}

void RunTrapAlongPath(StageTrap *t, u16 speed, s8 turn) {}

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

#include "engine/draw_trap_warning.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType2A(StageTrap *t, s32 speed, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F8454, RunTrapType13, 0x38);
FUNC_ALIAS(func_800F84A8, RunTrapType13, 0x8C);

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1D(StageTrap *t, u16 speed) {}

void RunTrapAheadAtHeight(StageTrap *t, u16 speed) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F87E4, RunTrapType2C, 0x24);

#include "engine_code.inc" /* BurstTrapStacked */

void SlideTrapAway(StageTrap *t, s32 arg) {}

#include "engine/tumble_trap_act08.inc"

#include "engine/tumble_trap_act09.inc"

#include "engine/tumble_trap_act0_b.inc"

#include "engine/knock_trap_away_act0_c.inc"

void StepDashBoard(StageTrap *t) {}

#include "engine/tumble_trap_act12.inc"

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepSmokingTumbleTrap(StageTrap *t) {}

void StepTrapAct1D(StageTrap *t) {}

void StepSmokingTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

#include "engine/draw_trap_while_linked.inc"

void UpdateStageEventHook(void) {
    StageTrap *m;

    m = &D_800D39D8[1];
    if (gSwitches[192].state == SWITCH_ON) {
        m->state = 0;
        gPlayer.motion[0] = 11;
        gPlayer.jumpPower = 60;
        gPlayer.action = 0;
        gSwitches[192].state = SWITCH_OFF;
        gPlayer.dropRide = 1;
        m->timer = 0;
        m->state = 1;
        gMissState = 0;
        gPlayer.hittable = 0;
        gPlayer.invulnTimer = 30;
        gPlayer.rideMode &= 0xF0;
        PlaySe(0x38);
    }
}

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

void func_800F8B4C(DirEnt16 *dir, u16 first, u16 count) {
    DirEnt16 *e;
    u16 i;

    e = dir;
    e += first;
    for (i = first; i < first + count; i++) {
        LoadTim((u8 *)dir + e->offset);
        e++;
        DrawSync(0);
    }
}

void func_800F8BD4(void) {
    RECT r;

    r.x = gFrameCount / 2 % 12 * 16;
    r.y = 0x1F3;
    r.w = 16;
    r.h = 1;
    MoveImage(&r, 0, 0x1E0);
}

#include "engine/reveal_start_cans.inc"

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F8D34);

#ifdef NON_MATCHING
void func_800F90F4(void) {
    s16 yaw;

    gCameraYaw = 0;
    sCamHeight = 0;
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
    gView[1] = (s16)(gCourse.y - 500);
    RevealStartCans();
}
#else
INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F90F4);
#endif

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 func_800F922C(s16 id, StageTrap *src, StageTrap *dst) {
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

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F936C);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F08F0);
