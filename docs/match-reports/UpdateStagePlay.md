# UpdateStagePlay — MATCHED 270/270 (round 12, alpha)

> Renamed from `func_800401F0` on 2026-10-10 (tools/rename.py). Address 0x800401f0.

Unit `src/stage_flow.c`. One frame of a stage, driven by the state in
`gModeStep`: 0 clears `gSubStep` and steps; 1 waits for the stage's
overlay (InitStageOverlay returns 1) and steps; 2 and 3 save the start position
and two words into `sSaved`, set `gRetries` to 50, reset the stage
(ResetStage) and go to 4; 4 runs the frame. In state 4, with no pause
(`gPauseState` 0) it runs the game's per-frame calls and the stage overlay's
own (a 15-way switch on `stage`); paused, it draws the pause screen (on the
first paused frame it copies the display to x 640 with MoveImage). Every
frame then checks button bit 0x800: pausing sets the volume to 30, unpausing
restores 80 and, with bit 0x100 also held, leaves to state 6 of
`gGameMode`. Called from main.c with the current stage.

Third build byte-exact.

```c
extern u16 sPauseFrames;
extern s32 gTmdSortMode;

void StepStageFrame3(void);
void StepChaseFrame4_3(void);
void StepChaseFrame5_3(void);
void StepChaseFrame3_3(void);
void StepChaseFrame2_3(void);
void StepChaseFrame1_3(void);
void StepStageFrame5_2(void);
void StepStageFrame4_1(void);
void StepStageFrame5_1(void);
void StepStageFrame4_2(void);
void StepStageFrame1_2(void);
void StepStageFrame2_2(void);
void StepStageFrame1_1(void);
void StepStageFrame2_1(void);
void UpdateActResults(void);
void DrawScreenWipe(void);
s32 UpdatePlayState(void);
s32 UpdateRumble(void);
void SetPadVibration(s32 offset, u8 a, u8 b);
void SetCdVolume(s32 vol);

void UpdateStagePlay(u8 stage) {
    RECT r;

    if (gStage % 3 != 2) {
        UpdateActResults();
        DrawScreenWipe();
    }
    switch (gModeStep) {
        case 0:
            gSubStep = 0;
            gModeStep++;
            break;
        case 1:
            if (InitStageOverlay(gStage) == 1) {
                gModeStep++;
            }
            break;
        case 2:
        case 3:
            sSaved.valid = 0;
            gRetries = 50;
            sSaved.unk10 = gSection;
            sSaved.pos.vx = gCourse.pos.vx;
            sSaved.pos.vy = gCourse.pos.vy;
            sSaved.pos.vz = gCourse.pos.vz;
            sSaved.unk14 = gFloorIndex;
            ResetStage();
            gSkipLandRumble = 1;
            gModeStep = 4;
            break;
        case 4:
            if (gPauseState == 0) {
                SetTmdSortMode(gTmdSortMode);
                UpdatePlayState();
                UpdateRumble();
                switch (stage) {
                    case 0:
                        StepStageFrame1_1();
                        break;
                    case 1:
                        StepStageFrame1_2();
                        break;
                    case 2:
                        StepChaseFrame1_3();
                        break;
                    case 3:
                        StepStageFrame2_1();
                        break;
                    case 4:
                        StepStageFrame2_2();
                        break;
                    case 5:
                        StepChaseFrame2_3();
                        break;
                    case 6:
                        StepStageFrame3();
                        break;
                    case 7:
                        /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
                        StepStageFrame3();
                        __asm__("");
                        break;
                    case 8:
                        StepChaseFrame3_3();
                        break;
                    case 9:
                        StepStageFrame4_1();
                        break;
                    case 10:
                        StepStageFrame4_2();
                        break;
                    case 11:
                        StepChaseFrame4_3();
                        break;
                    case 12:
                        StepStageFrame5_1();
                        break;
                    case 13:
                        StepStageFrame5_2();
                        break;
                    case 14:
                        StepChaseFrame5_3();
                        break;
                }
            } else {
                SetPadVibration(0, 0, 0);
                switch (gPauseState) {
                    case 1:
                        r.x = 0;
                        r.y = gActiveBuff * 240;
                        r.w = 320;
                        r.h = 240;
                        MoveImage(&r, 0x280, 0);
                        sPauseFrames = 0;
                        gPauseState++;
                    case 2:
                        sPauseFrames++;
                        DrawPausedFrame();
                        break;
                }
            }
            break;
    }
    gSkipLandRumble = 0;
    if (gPadTrig & 0x800) {
        if ((u16)(gMissState + 1) < 2) {
            if (gPauseState == 0) {
                SetCdVolume(30);
                gPauseState = 1;
            } else {
                if (gPad & 0x100) {
                    StartRumble(gRumblePatterns, 0);
                    gGameMode = 6;
                    gModeStep = 0;
                    StopCdTrack();
                    sStage.unk3D2 = 0;
                    sStage.unk3D0 = 0;
                }
                SetCdVolume(80);
                gPauseState = 0;
            }
        }
    }
}
```

`StageState` gained `u8 unk3D0` (split out of `pad3CC`); retail clears it
with `sb zero, 0x3D0` off the `gPlayer` base.

## Notes

- Build 1 was 4 words short: cases 6 and 7 of the overlay switch call the
  same function (StepStageFrame3) and cross-jumping merged them. A bare
  `__asm__("")` after the call in case 7 keeps them apart, as in
  StartStageOverlay and ResetStageOverlay.
- Build 2 was 268/270, then 270/270 after two reorders in states 2 and 3:
  `sSaved.unk10 = gSection;` written before the position copy loads
  `gSection` first, as retail does. `sSaved.valid = 0;` written before
  `gRetries = 50;` puts `sSaved`'s `lui` above the `li 50`, though the
  stores still come out in the other order.
- The parameter is copied to `$s0` at entry and masked (`andi 0xFF`) only at
  the switch: a `u8` parameter on 2.8.1 can look like this; it needed no
  `s32` view.
- `(u16)(gMissState + 1) < 2` gives retail's `addiu 1; andi 0xFFFF; sltiu 2`
  (gMissState is an `s16`: the test is "0 or -1").
- `gModeStep++` in state 1 reuses the 1 in `$v1` from the compare
  (`addu v0, v0, v1`); no special spelling needed.

## Declarations

- New prototypes in stage_flow.c for other units' functions
  (UpdateActResults, DrawScreenWipe, UpdatePlayState, UpdateRumble,
  SetPadVibration, SetCdVolume) and `extern s32 gTmdSortMode;` (main.c's);
  declcheck reports them LOCAL (SetPadVibration also MULTI with main.c's
  forward declaration). They stay in the `.c` this round, by the round's
  rule against shared-header prototypes for other units' functions.
- main.c's `void UpdateStagePlay(u8 stage);` moved to `include/common.h`.
