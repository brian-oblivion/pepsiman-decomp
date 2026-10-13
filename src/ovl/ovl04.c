/* Stage 1-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", ResetSpawnedTraps);

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapAct27);

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapType2A);

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapType0F);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapFFAct68);

#include "engine/step_smoking_tumble_trap.inc"

#include "engine/step_smoking_trap.inc"

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine/slide_trap_away.inc"

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine/step_dash_board.inc"

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

#include "engine/open_gate_on_approach.inc"

#include "engine/swing_gate_continuously.inc"

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepHoppingTrap);

#include "engine/swap_trap_model_on_approach.inc"

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F74FC);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepGoalSequence);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", CycleGoalLights);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F79A0);

#include "engine/step_stage_frame.inc"

void func_800F7CE0(StageTrap *m) {
    if (gSwitches[190].state == SWITCH_ON) {
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                StopSpeed();
                EndJump();
                D_800D39D8[0].homeRotZ = 0;
                D_800D39D8[0].homeRotX = 0;
                D_800D39D8[0].homeX = gPlayer.x;
                D_800D39D8[0].homeY = gPlayer.y;
                D_800D39D8[0].homeZ = gPlayer.z;
                gMissState = -1;
                m->timer = 0;
                m->state++;
                /* fall through */
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, 0x6407, m->timer, 20);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, -0x5854, m->timer, 20);
                m->timer++;
                if ((u8)(gPlayer.motion[0] - 2) < 3 && D_800D39D8[0].homeRotX == 0) {
                    D_800D39D8[0].homeRotX = 1;
                    gPlayer.dropRide = 1;
                    gPlayer.rideMode &= 0xF0;
                }
                if (m->timer > 20) {
                    m->timer = 0;
                    m->state++;
                    if (D_800D39D8[0].homeRotX == 0) {
                        gPlayer.dropRide = 1;
                        gPlayer.rideMode &= 0xF0;
                    }
                }
                break;
            case 2:
                gPlayer.motion[0] = 11;
                gPlayer.action = 0;
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

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F04F0);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F7EFC);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F82E0);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F8454);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F84A8);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F878C);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F87E4);

void func_800F8A58(void) {
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

void StartRumble(u16 *table, s32 index);

void func_800F8B60(void) {
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
            PlaySe(0x17);
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

void ApplySteer(u8 mode, u16 a, s16 b);
void RecentreSteer(void);
void PushPlayer(s32 deg, s32 dist);

void func_800F8D64(void) {
    if (gPad & 0x8000) { /* PADLleft */
        ApplySteer(3, 16, 5);
        sSteerDrift -= 10;
        gPlayer.action = 0x35;
    } else if (gPad & 0x2000) { /* PADLright */
        ApplySteer(2, 16, 5);
        sSteerDrift += 10;
        gPlayer.action = 0x37;
    } else {
        RecentreSteer();
        gPlayer.action = 0x33;
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

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F052C);

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", RunTrapType0B);

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s16 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

void StepTrapType1F(StageTrap *t, u16 arg) {}

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapAct21(StageTrap *t) {}

void StepTrapAct22(StageTrap *t) {}

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapAct23);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapAct24);

void StepTrapAct25(StageTrap *t, s32 arg) {}

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapAct28);

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
    func_800F7CE0(&D_800D39D8[1]);
}

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FA694);

#include "engine/reveal_start_cans.inc"

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FA804);

#ifdef NON_MATCHING
void func_800FABC4(void) {
    s16 yaw;
    StartRec *st;

    /* MATCHING: stored as the s16 it is read as. */
    *(s16 *)&gCameraYaw = -0xC00;
    sCamHeight = -150;
    sViewSpeed = 100;
    gViewerAngle = 0xD8;
    gFellOff = 0;
    st = gStageStart;
    gPlayLive = 1;
    sCamOffset[0] = 0;
    gView[1] = 0;
    sCamOffset[1] = 0;
    yaw = -st->yaw;
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
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gCourse.z = gPlayer.z = -st->z;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gView[1] = (s16)(gCourse.y - 150);
    RevealStartCans();
}
#else
INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FABC4);
#endif

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 func_800FAD04(s16 id, StageTrap *src, StageTrap *dst) {
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

extern const SpriteShapes D_800F0910;

void func_800FAE44(void) {
    SpriteShapes shapes;

    shapes = D_800F0910;
    *(SpriteShapes *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F0910);
