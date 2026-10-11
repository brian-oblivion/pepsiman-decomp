# UpdateMotionViewer — MATCHED (167/167), round 7, runner alpha

> Renamed from `func_80017B38` on 2026-10-10 (tools/rename.py). Address 0x80017b38.

Unit `src/code_7d74.c`. A model viewer's per-frame step, driven by the
state `gModeStep`. State 0 sets the fog colour and the reference view,
clears the model's rotation, translation and player state, applies the
rotation to the coordinate system `D_800D86E0[0]` (SetCoordRotation), starts
the animation player (StepAnimation) and moves to state 1. State 1 steps
the player when `gViewerReplay` asks for it, draws (DrawTodObjects), applies
the level keys (SelectViewerMotion) and the orbiting view (OrbitViewerCamera), and
moves the model with six button bits. Every frame ends with
`FntPrint("\n\n\tMOTION %3d", gViewerMotion)`.

```c
/* Player: model's view of the game state's head (gPlayer),
 * extended this round with unk340..unk384; sPlayer is its lvalue. */
void UpdateMotionViewer(void) {
    /* MATCHING: an unused local gives retail's 0x20-byte frame. */
    s32 unused[2];

    switch (gModeStep) {
        case 0:
            gClearR = 0x10;
            gClearG = 0x20;
            gClearB = 0x80;
            gRefView[0] = 0;
            gRefView[1] = 0;
            gRefView[2] = -1000;
            gRefView[3] = 0;
            gRefView[4] = 0;
            gRefView[5] = 0;
            gViewerAngle = -90;
            sPlayer.unk380 = 0;
            sPlayer.unk384 = 0;
            sPlayer.unk348 = 0;
            sPlayer.unk34C = 0;
            sPlayer.unk350 = 0;
            gWorldShift[0] = 0;
            D_80095918 = 0;
            gWorldShift[2] = 0;
            D_800D86E0[0].coord.t[0] = 0;
            D_800D86E0[0].coord.t[1] = 0;
            D_800D86E0[0].coord.t[2] = 0;
            D_8009EAB8[0] = 0;
            D_8009EAB8[1] = sPlayer.unk380;
            D_8009EAB8[2] = sPlayer.unk384;
            SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
            sPlayer.want[0] = 0;
            StepAnimation(&sPlayer, 0, 0);
            gModeStep++;
            break;
        case 1:
            if (gViewerReplay == 1) {
                gViewerReplay = StepAnimation(&sPlayer, 0, 0);
            } else {
                D_8009EB7E[0] = 0;
            }
            DrawTodObjects();
            SelectViewerMotion();
            OrbitViewerCamera();
            if (gPad & 0x2000) {
                D_800D86E0[0].coord.t[0]++;
            }
            if (gPad & 0x8000) {
                D_800D86E0[0].coord.t[0]--;
            }
            if (gPad & 0x4000) {
                D_800D86E0[0].coord.t[1]++;
            }
            if (gPad & 0x1000) {
                D_800D86E0[0].coord.t[1]--;
            }
            if (gPad & 1) {
                D_800D86E0[0].coord.t[2]--;
            }
            if (gPad & 2) {
                D_800D86E0[0].coord.t[2]++;
            }
            break;
    }
    FntPrint(gMotionFmt, gViewerMotion);
}
```

## Attempts (2)

1. The body above without `unused`: 161/167, every instruction right but
   the frame 0x18 instead of 0x20.
2. `s32 unused[2];` (LEARNINGS' unused-local frame lever): 167/167. It
   adds a NEW `-Wall` warning (`unused variable 'unused'` in model)
   for the head to baseline.

## Declarations

- `gModeStep` moved into `include/common.h` as `u16` (retail loads it
  `lhu`); stage_flow.c's local `extern s16` (a store only) was deleted.
- `D_800D86E0` (GsCOORDINATE2[]) moved from object.c into
  `include/object.h`, which now includes libgte/libgpu/libgs (all
  guarded); model includes that header.
- `gViewerReplay` is now `s8` here (retail loads it `lb`; SelectViewerMotion
  only stores it), and it and `gViewerMotion` moved to the top of the unit.
- The common.h views fit retail as they are: `D_8009EAB8` (`u16[]`, the
  rotation written as three halfwords and passed as an SVECTOR) and
  `D_8009EB7E` (the player's `want[0]`, which retail reaches through its
  own symbol in state 1 and through the `sPlayer` base in state 0).
