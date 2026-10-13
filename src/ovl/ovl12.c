/* Stage 4-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", ResetSpawnedTraps);

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl12/nonmatchings/ovl12", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

#include "engine/run_trap_ahead.inc"

#include "engine/step_trap_forward.inc"

#include "engine/step_trap_forward_level.inc"

#include "engine/run_trap_ahead_nonstop.inc"

#include "engine/run_trap_along_path.inc"

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", RunTrapThenTurnTimed);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", DrawTrapWarning);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType0F);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType1F);

#include "engine/step_rolling_trap.inc"

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType0D);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType30);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType1B);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType1D);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType1C);

#include "engine/run_trap_ahead_at_height.inc"

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapAct1D);

#include "engine/burst_raised_trap.inc"

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", ToppleTrapOnApproach);

#include "engine/scatter_trap_dust.inc"

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", UpdateGoalEventHook);

#include "engine/step_stage_frame.inc"

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F74FC);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F77FC);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F79A0);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F7D24);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F7D30);

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F82E0, RunTrapType09, 0x64);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", RunTrapType0B);

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

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapAct21);

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

void StepTrapType2A(StageTrap *t, s32 speed, s32 arg) {}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

void StepTrapType27(StageTrap *t, s32 arg) {}

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", StepTrapType31);

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

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F9618);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F9980);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F9ABC);

INCLUDE_ASM("asm/ovl/ovl12/nonmatchings/ovl12", func_800F9BFC);

INCLUDE_RODATA("asm/ovl/ovl12/nonmatchings/ovl12", D_800F08D4);

INCLUDE_RODATA("asm/ovl/ovl12/nonmatchings/ovl12", D_800F0924);
