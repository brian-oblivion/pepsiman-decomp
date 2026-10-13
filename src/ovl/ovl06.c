/* Stage 2-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", ResetSpawnedTraps);

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl06/nonmatchings/ovl06", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

#include "engine/run_trap_ahead.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepTrapForward);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepTrapForwardLevel);

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_then_turn.inc"

#include "engine/run_trap_along_path.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepChargingTrap);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800F3158);

#include "engine/run_trap_ahead_at_height.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/step_smoking_tumble_trap.inc"

#include "engine/step_smoking_trap.inc"

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine/slide_trap_away.inc"

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine/step_dash_board.inc"

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", StepSprayTrap);

#include "engine/burst_raised_trap.inc"

#include "engine/turn_trap_on_approach.inc"

#include "engine/open_gate_on_approach.inc"

#include "engine/swing_gate_continuously.inc"

#include "engine/topple_trap_on_approach.inc"

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", ScatterTrapDust);

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

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", RunTrapType0B);

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

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", DrawTrapWarning);

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

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA2FC);

void func_800FA334(void) {}

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA3EC);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA754);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA894);

INCLUDE_ASM("asm/ovl/ovl06/nonmatchings/ovl06", func_800FA9D4);

INCLUDE_RODATA("asm/ovl/ovl06/nonmatchings/ovl06", D_800F08D4);

INCLUDE_RODATA("asm/ovl/ovl06/nonmatchings/ovl06", D_800F0924);
