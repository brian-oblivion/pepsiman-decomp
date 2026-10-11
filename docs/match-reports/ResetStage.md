# ResetStage — MATCHED 206/206 (round 6, delta)

> Renamed from `func_80040998` on 2026-10-10 (tools/rename.py). Address 0x80040998.

Unit `src/stage_flow.c`. Stage reset. Calls the set-up routines, copies the
second display buffer's band down when `gGameMode` is not 0x29, resets
the game state's head (`gPlayer`) and a run of globals, then either
restores a saved start (`D_800D86B8`, when its first word is non-zero) or,
in mode 4, applies that mode's start; finally picks a per-mode value from
one of two `[][3]` tables (`D_800958AC == 1` picks the second set) times
30, and calls EndJump and StartRumble.

```c
/* Unit-local views: StageState (gPlayer's head, widened for this
 * function), SavedStart (D_800D86B8: s32 valid; Pos3 pos; s32 unk10,
 * unk14; u16 unk18, unk1A; pad; s32 unk20), Placement (D_800A7680).
 * Locals: extern s16 D_800E474C; u16 gGameMode, gCameraYaw; u8
 * gLoading, gPauseState; s32 D_800957A4, gTimeLeft, ClipF;
 * s32 gTimeLimitsAlt[][3], gTimeLimits[][3]. The rest from common.h. */
void ResetStage(void) {
    RECT r;
    s32 mode;
    s32 saved;

    D_80095900 = 1;
    StopCdTrack();
    if (gGameMode != 0x29) {
        r.x = 0;
        r.y = D_800E474C * 240;
        r.w = 320;
        r.h = 240;
        MoveImage(&r, 0x280, 0);
    }
    InitEffects();
    gLoading = 0;
    gVSyncMode = 2;
    ResetStageOverlay();
    if (D_800958AC == 1) {
        gBonusLevel = 2;
    } else {
        gBonusLevel = 3;
    }
    gMissState = 0;
    SetCurrentBlock((BlockHeader *)0x801FD000);
    D_80095768 = 0;
    sStage.unk6 = 3;
    sStage.unk3CA = 0;
    sStage.unk3 = 0;
    sStage.unk4 = 0;
    sStage.unk5 = 0;
    sStage.unk8 = 0xFF;
    StepAnimation(&sStage, 0, 0);
    sStage.unk3A8 = 50;
    sStage.unk38E = 0;
    sStage.unk38C = 0;
    sStage.unk3D2 = 0;
    sStage.unk354 = sStage.unk358 = sStage.unk35C = 0;
    sStage.unk360 = sStage.unk362 = sStage.unk364 = 0;
    sStage.unk3D1 = 0;
    sStage.unk3D3 = 1;
    sStage.unk3C8 = 0;
    sStage.unk2 = 1;
    D_8009599C = 0;
    sStage.unk3D4 = sStage.unk3D5 = 0;
    sStage.unk3A6 = 0;
    gView[6] = 0;
    sStage.unk0 = 1;
    D_800958EC = 0;
    D_800957D6 = 1;
    gSection = 0;
    gPauseState = 0;
    D_800957A4 = 0;
    ClipF = 0;
    gFogFade = 0;
    ResetSpeed();
    if (sSaved.valid != 0) {
        gSection = sSaved.unk10;
        sStage.unk348.vx = sSaved.pos.vx;
        sStage.unk348.vy = sSaved.pos.vy;
        sStage.unk348.vz = sSaved.pos.vz;
        D_800957F4 = sSaved.unk14;
        gCameraYaw = sSaved.unk18;
        gCans = sSaved.unk1A;
        gView[1] = sSaved.unk20;
    } else if (gStage == 4) {
        sStage.unk5 = 0x57;
        sStage.unk6 = 0x23;
        sStage.unk398 = 0;
        sStage.unk3A8 = 0;
        gSpeedFixed = 0;
        sStage.unk374 = sStage.unk376 = sStage.unk378 = sStage.unk37A = 0;
        sStage.unk0 = 0;
        sStage.unk348.vy = D_800A7680.pos.vy - 1000;
    }
    mode = gStage;
    saved = sSaved.valid;
    if (D_800958AC == 1) {
        gTimeLeft = gTimeLimitsAlt[mode][saved] * 30;
    } else {
        gTimeLeft = gTimeLimits[mode][saved] * 30;
    }
    D_8009EF48[0] = 0;
    EndJump();
    StartRumble(gRumblePatterns, 0);
}
```

## Levers

- **The two table indices in locals before the last `if`.** Retail loads
  `gStage` and the saved flag once, ahead of the `D_800958AC` test,
  and keeps the compare's 1 in `$a0`; in the taken arm the shifts by one
  then use `sllv` with that register. Indexing the globals in each arm
  gave a per-arm load and immediate shifts (25 differing lines); hoisting
  only the saved flag was 20. Both locals: byte-exact.
- **Chained assignment for each run of zeroed neighbours**
  (`a = b = c = 0` stores `c` first): 0x354..0x35C, 0x360..0x364,
  0x374..0x37A, 0x3D4/0x3D5.
- `sSaved` as a struct lvalue over common.h's `s32 D_800D86B8[]` gives the
  `addiu t1` base for the eight field loads; the restore is field by field
  (as RestartStage's position copy).
- The constant 2 that `$s0` carries across `ResetStageOverlay` into the
  `gBonusLevel = 2` arm needs no local; cc1 keeps it on its own.
- `ClipF` is written as a word: retail stores 0 at the address splat names
  after libgte's clipf object. `extern s32 ClipF;` in the unit, with a
  comment.

## Declarations

- `gVSyncMode` (from main.c), `D_800958AC`, `D_8009599C`, `gSpeedFixed`
  (from player.c) moved into `include/common.h` with the same types
  (additive), and `StopCdTrack` (model) was added there.
- `StepAnimation` takes `Stepper *` in main.c's view and `StageState *`
  here; both prototypes carry a `MATCHING:` note (main.c gained the note).
- `StartRumble` gets a unit-local `(u16 *, s32)` view with a
  `MATCHING:` note, as player has.
