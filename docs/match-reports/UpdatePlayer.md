# UpdatePlayer — MATCHED 335/335 (round 10, bravo)

> Renamed from `func_80023228` on 2026-10-11 (tools/rename.py). Address 0x80023228.

Unit `src/code_13068.c`. The player's per-frame step: runs the unit's
checks, dispatches the stage mode (low nibble of unk3D0) and the camera
mode (low nibble of `D_8009EF48[0]`) through three jump tables, sets the
speed display `gSpeedFixed` from unk3A8 (fixed at 70, 90 or 30 by flags in
`D_80095864`, which can also push the player along the heading), moves
the position by the speed along the heading, clamps unk380 to 0x155 of
the camera yaw and hands the result to the camera (`gBodyRot`,
`gTodCoords[0]`, `gWorldShift`).

```c
extern s8 D_80095962;
void func_800F8A58(void); /* and func_800F8B60, func_800F6FA0, func_800F70A8,
                             func_800F8D64, func_800F729C: overlay entries */

/* MATCHING: g keeps the sGame base in a register for the clamp's store at the join;
 * cases 1 and 2 of the second switch are separate arms, merged by cross-jumping. */
void UpdatePlayer(void) {
    s32 t;
    s32 x;
    s32 v;
    s32 hi;
    GameState *g;

    if (D_80095962 != 1) {
        CheckFall();
    }
    UpdateSpeedBand();
    ApplyGravity();
    if (sGame.unk0 == 1) {
        AttachModeObject();
    }
    AttachMode5Object();
    SpawnModeObject();
    switch (sGame.unk3D0 & 0xF) {
        case 0:
            UpdateRunSpeed();
            UpdateSlideDash();
            UpdateJump();
            TrySlide();
            break;
        case 1:
            func_800F8A58();
            func_800F8B60();
            break;
        case 4:
            func_800F6FA0();
            func_800F70A8();
            break;
        case 2:
            UpdateSpeedMode2();
            break;
        case 3:
            UpdateSpeedMode3();
            break;
    }
    sGame.unk344 = (40 - sGame.unk3A8) / 3 + 1;
    t = sGame.unk3A8;
    switch (((u32)D_80095864 >> 22) & 3) {
        case 1:
            if (t != 0) {
                t = 70;
            }
            break;
        case 2:
            if (t != 0) {
                t = 90;
            }
            break;
        case 3:
            if (sGame.unk34C >= sGame.unk3C0) {
                PolarOffset(gCourse->vy * 360 / 4096 - 180, 30, (VECTOR *)gOffsetVec);
                sGame.unk348 += gOffsetVec[0];
                sGame.unk350 += gOffsetVec[2];
            }
            break;
    }
    switch (((u32)D_80095864 >> 20) & 3) {
        case 1:
            if (D_8009EF20[0] != 0) {
                t = 30;
            }
            break;
        case 2:
            if (D_8009EF20[0] != 0) {
                t = 30;
            }
            break;
        case 3:
            if (sGame.unk34C >= sGame.unk3C0) {
                PolarOffset(gCourse->vy * 360 / 4096, 30, (VECTOR *)gOffsetVec);
                sGame.unk348 += gOffsetVec[0];
                sGame.unk350 += gOffsetVec[2];
            }
            break;
    }
    gSpeedFixed = t << 4;
    switch (D_8009EF48[0] & 0xF) {
        case 0:
        case 1:
        case 3:
        case 4:
            sGame.unk3AE = rsin(gCourse->vy) * sGame.unk3A8 >> 12;
            sGame.unk3B0 = rcos(gCourse->vy) * sGame.unk3A8 >> 12;
            sGame.unk348 -= sGame.unk3AE;
            sGame.unk350 -= sGame.unk3B0;
            break;
        case 2:
            sGame.unk3AE = rsin(sGame.unk380) * sGame.unk3A8 >> 12;
            sGame.unk3B0 = rcos(sGame.unk380) * sGame.unk3A8 >> 12;
            sGame.unk348 -= sGame.unk3AE;
            sGame.unk350 -= sGame.unk3B0;
            break;
    }
    switch (D_8009EF48[0] & 0xF) {
        case 0:
            SteerPlayer();
            break;
        case 1:
            func_800F8D64();
            break;
        case 4:
            func_800F729C();
            break;
        case 2:
            SteerMode2();
            break;
        case 3:
            SteerMode3();
            break;
    }
    g = &sGame;
    x = g->unk380;
    v = gCourse->vy - 0x155;
    if (x >= v) {
        hi = gCourse->vy + 0x155;
        if (x <= hi) {
            v = x;
        } else {
            v = hi;
        }
    }
    g->unk380 = v;
    gBodyRot[1] = sGame.unk380;
    gBodyRot[2] = sGame.unk384;
    gTodCoords[0].coord.t[0] = sGame.unk354;
    gTodCoords[0].coord.t[1] = sGame.unk34C + sGame.unk358;
    gTodCoords[0].coord.t[2] = sGame.unk35C;
    SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
    gWorldShift[1] = 0;
    gWorldShift[0] = -sGame.unk348;
    gWorldShift[2] = -sGame.unk350;
}
```

`GameState` gained `unk344`, `unk358` (s32) and `unk3AE`, `unk3B0`
(s16). The unit now includes `model.h` for SetCoordRotation, and
declares the prototypes of the unit's own later functions it calls.

## Levers

Four builds: 65 -> 269 -> 335 (plus one for a missing prototype).

- Case bodies follow source order (0, 1, 4, 2, 3 in both mode switches),
  as LEARNINGS says. The three jump tables in the unit's rodata come out
  at retail's addresses unchanged.
- **Two cases with identical bodies that retail tests separately
  (`beq 2; sltiu 3; beq 1` to one label): two arms, not `case 1: case
  2:`.** A shared label turns the tests into a range check.
  Cross-jumping merges the bodies afterwards.
- The camera-mode switch's shared rcos tail: each arm also does
  `unk348 -= unk3AE; unk350 -= unk3B0;`, and cross-jumping merges
  everything after the rcos argument.
- `sGame.unk344 = (40 - sGame.unk3A8) / 3 + 1; t = sGame.unk3A8;` gives
  retail's `move s3, v1` after the multiply; `t = unk3A8` first, then
  `(40 - t)`, loads straight into `$s3`.
- The yaw clamp is SteerMode3's shape: a `GameState *g` for the load
  and the store at the join, a scratch `hi`.

### Proposed learning

Two switch cases with identical bodies that retail tests one by one
(`beq K1; ...; beq K2` to the same label): write them as separate arms.
`case K1: case K2:` becomes a range test. (UpdatePlayer)
