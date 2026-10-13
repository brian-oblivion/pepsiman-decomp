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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapForward);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapType2A);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepTrapForwardLevel);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", SwapTrapModelOnApproach);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F74FC);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", UpdateStageEventHook);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F79A0);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", UpdateGoalEventHook);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F809C);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F82E0);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F8454);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F84A8);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F85F0);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F87E4);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F8B2C);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800F8DF0);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F056C);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", DrawTrapWarning);

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

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FA5D0);

void func_800FA610(void) {}

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FA6C8);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FAA30);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FAB9C);

INCLUDE_ASM("asm/ovl/ovl03/nonmatchings/ovl03", func_800FAD08);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F08FC);

INCLUDE_RODATA("asm/ovl/ovl03/nonmatchings/ovl03", D_800F0960);
