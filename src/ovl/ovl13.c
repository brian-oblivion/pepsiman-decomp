/* Stage 4-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", ResetSpawnedTraps);

#include "engine_code.inc" /* StepCheckpoint */

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepFlungPiece);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", SpawnMissDebris);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepMissDebris);

INCLUDE_RODATA("asm/ovl/ovl13/nonmatchings/ovl13", sDebrisModelInit);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapAct22);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapAhead);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F2840);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F2998);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapAheadNonstop);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapAlongPath);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapThenTurnTimed);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", DrawTrapWarning);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType0F);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType12);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType1F);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F3CC0);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType1D);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType18);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType1A);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType1C);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapType1E);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapAheadAtHeight);

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapAct1D);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", BurstRaisedTrap);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", ToppleTrapOnApproach);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F70DC);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepMissSequence);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F74FC);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepGoalSequence);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", CycleGoalLights);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F7844);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F7968);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F79A0);

INCLUDE_RODATA("asm/ovl/ovl13/nonmatchings/ovl13", D_800F0508);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F7A88);

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F82E0, RunTrapTypeFF, 0xF4);
FUNC_ALIAS(func_800F8454, RunTrapType02, 0x7C);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapType04);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F84A8);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", RunTrapType0B);

#include "engine_code.inc" /* RunTrapType32 */

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F87E4, RunTrapType10, 0x48);

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapFFAct68(StageTrap *t) {}

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapAct21);

void StepTrapAct23(StageTrap *t) {}

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", StepTrapAct24);

void StepTrapAct25(StageTrap *t, s32 arg) {}

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType2A(StageTrap *t, s32 speed, s32 arg) {}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {}

void StepTrapType27(StageTrap *t, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

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

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", DrawTrapWhileLinked);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", UpdateStageEventHook);

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F9774);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F97FC);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F98E4);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F9CA4);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800F9F18);

INCLUDE_ASM("asm/ovl/ovl13/nonmatchings/ovl13", func_800FA058);

INCLUDE_RODATA("asm/ovl/ovl13/nonmatchings/ovl13", D_800F08B0);

INCLUDE_RODATA("asm/ovl/ovl13/nonmatchings/ovl13", D_800F08FC);
