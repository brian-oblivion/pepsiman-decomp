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

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F7CE0);

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F04F0);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F7EFC);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F82E0);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F8454);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F84A8);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F878C);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F87E4);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F8A58);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F8B60);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F8D64);

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

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", DrawTrapWhileLinked);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", UpdateStageEventHook);

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FA694);

#include "engine/reveal_start_cans.inc"

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FA804);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FABC4);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FAD04);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FAE44);

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F08C0);

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F0910);
