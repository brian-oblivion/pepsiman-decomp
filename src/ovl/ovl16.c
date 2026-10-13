/* Stage 5-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", ResetSpawnedTraps);

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

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", ShatterTrap);

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", BurstRaisedTrap);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", ToppleTrapOnApproach);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F55A4);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", SwapTrapModelOnApproach);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepGoalSequence);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", CycleGoalLights);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F5E80);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F04D8);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F04E8);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F60CC);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F6FA0);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F70A8);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F729C);

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

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepTrapAct25);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F82E0);

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void RunTrapAhead(StageTrap *t, s16 speed) {}

void RunTrapAheadNonstop(StageTrap *t, s16 speed) {}

void RunTrapAlongPath(StageTrap *t, u16 speed, s8 turn) {}

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", DrawTrapWarning);

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

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", TumbleTrapAct08);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", TumbleTrapAct09);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", TumbleTrapAct0B);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", KnockTrapAwayAct0C);

void StepDashBoard(StageTrap *t) {}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", TumbleTrapAct12);

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepSmokingTumbleTrap(StageTrap *t) {}

void StepTrapAct1D(StageTrap *t) {}

void StepSmokingTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", DrawTrapWhileLinked);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", UpdateStageEventHook);

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F8B4C);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F8BD4);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F8C4C);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F8D34);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F90F4);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F922C);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F936C);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F08A0);

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F08F0);
