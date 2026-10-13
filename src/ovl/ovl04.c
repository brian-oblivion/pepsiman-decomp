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

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", RunTrapAhead);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F29C4);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapType2A);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F2CE8);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", RunTrapAheadNonstop);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", RunTrapThenTurn);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", RunTrapAlongPath);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapType0F);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepTrapFFAct68);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepSmokingTumbleTrap);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepSmokingTrap);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", ShatterTrap);

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", SlideTrapAway);

#include "engine_code.inc" /* BurstTrapMedium */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepDashBoard);

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", OpenGateOnApproach);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", SwingGateContinuously);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepHoppingTrap);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", SwapTrapModelOnApproach);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F74FC);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", StepGoalSequence);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", CycleGoalLights);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F79A0);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800F7A9C);

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

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", DrawTrapWarning);

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

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", TumbleTrapAct08);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", TumbleTrapAct09);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", TumbleTrapAct0B);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", KnockTrapAwayAct0C);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", TumbleTrapAct12);

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

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FA71C);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FA804);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FABC4);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FAD04);

INCLUDE_ASM("asm/ovl/ovl04/nonmatchings/ovl04", func_800FAE44);

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F08C0);

INCLUDE_RODATA("asm/ovl/ovl04/nonmatchings/ovl04", D_800F0910);
