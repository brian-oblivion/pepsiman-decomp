/* Stage 5-1's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", ResetSpawnedTraps);

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl15/nonmatchings/ovl15", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", RunTrapAhead);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F2674);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepTrapType2A);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F2998);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", RunTrapAheadNonstop);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", RunTrapThenTurn);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", RunTrapAlongPath);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepChargingTrap);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", DrawTrapWarning);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepTrapType0F);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepTrapType27);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepSmokingTrap);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", ShatterTrap);

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", BurstRaisedTrap);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", ToppleTrapOnApproach);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F6934);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", SwapTrapModelOnApproach);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", UpdateStageEventHook);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", UpdateGoalEventHook);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F74FC);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F7724);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F79A0);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F7C9C);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F7E34);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F80F4);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F82E0);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F8454);

INCLUDE_RODATA("asm/ovl/ovl15/nonmatchings/ovl15", D_800F0558);

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F84A8, PlaySpriteHitSe, 0xC);
FUNC_ALIAS(func_800F87E4, RunTrapType01, 0xB4);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", RunTrapType0B);

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s16 arg, s8 dir) {}

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

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", TumbleTrapAct08);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", TumbleTrapAct09);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", TumbleTrapAct0B);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", KnockTrapAwayAct0C);

void StepDashBoard(StageTrap *t) {}

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", TumbleTrapAct12);

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepSmokingTumbleTrap(StageTrap *t) {}

void StepTrapAct1D(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", DrawTrapWhileLinked);

#include "engine_code.inc" /* UpdateGoalEvents */

void StepGoalSequence(void) {}

void CycleGoalLights(s8 arg) {}

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F9A90);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F9AC8);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F9BC0);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800F9F28);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800FA060);

INCLUDE_ASM("asm/ovl/ovl15/nonmatchings/ovl15", func_800FA1A0);

INCLUDE_RODATA("asm/ovl/ovl15/nonmatchings/ovl15", D_800F08EC);

INCLUDE_RODATA("asm/ovl/ovl15/nonmatchings/ovl15", D_800F093C);
