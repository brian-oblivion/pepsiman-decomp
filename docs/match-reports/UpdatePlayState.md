# UpdatePlayState — MATCHED 334/334 (round 10, bravo)

> Renamed from `func_80026D9C` on 2026-10-11 (tools/rename.py). Address 0x80026d9c.

Unit `src/player.c`. The stage-flow step, skipped on every third
stage. It switches on the flow state `gMissState`: -1 runs DrawHud;
0 is the race (rank clamp, the timers `gElapsed`/`gTimeLeft`, the end
of the stage when the timer runs out, then the switch to state 1 that
restarts the switch at once); 1 counts `gMissTimer` through the result
(resets the 200 SwitchBox records at 10, puts the player back at the saved
position from 10 on, moves to 100 after 160 frames); 2 and 3 are a short
pause into 100; 100 runs the fade-out `gRetries`; 110 the ending.

```c
extern s32 sSavedPos[];
extern s32 gElapsed;
extern s32 gTimeLeft;
extern u8 gInEditor;
extern SwitchBox gSwitches[];
extern u8 D_800959B0;
extern s16 gMissTimer;
extern s32 gRetries;
void DrawHud(void);
void FadeInFog(void);
void SetSoundFade(u8 value);
void DrawFrozenFrame(void);
void RestartStage(); /* stage_flow defines it with a u8 parameter */
void func_8003E360(s32 level);

typedef struct {
    s32 w[3];
} Words3;

/* MATCHING: the empty case -2 sets the switch's compare tree; non-void with no return keeps
 * the guards' delay slots nops; the unused words give retail's frame. */
s32 UpdatePlayState(void) {
    s32 unused[4];
    u16 i;
    s32 x;
    s16 t;

    if (gStage % 3 == 2) {
        goto end;
    }
top:
    switch (gMissState) {
        case -2:
            break;
        case -1:
            DrawHud();
            break;
        case 0:
            x = gBonusLevel;
            if (D_800958AC == 1) {
                if (x >= 2) {
                    gBonusLevel = 2;
                }
            } else if (x >= 3) {
                gBonusLevel = 3;
            }
            if (gBonusLevel <= 0) {
                gBonusLevel = 0;
            }
            if (D_8009EF4D[0] == 1) {
                gBonusLevel = 0;
            }
            if (gBonusLevel != 0 && D_8009EF4D[0] != 1 && (gStage != 4 || gSection != 0)) {
                gElapsed++;
                gTimeLeft--;
            }
            if (gStage % 3 == 2) {
                break;
            }
            FadeInFog();
            DrawHud();
            if (gInEditor == 1) {
                break;
            }
            if (gTimeLeft > 0) {
                break;
            }
            gTimeLeft = 0;
            sGame.unk0 = 0;
            sGame.unk2 = 0;
            sGame.unk3C8 = 500;
            sGame.unk38E = 0;
            sGame.unk38C = 0;
            if (sGame.unk3D0 & 0xF) {
                sGame.unk5 = 0;
                sGame.unk3D0 &= 0xF0;
                sGame.unk3D1 = 1;
                sGame.unk398 = 0;
                sGame.unk390 = 0;
            }
            if (gSwitches[196].unk0 == 1) {
                break;
            }
            if (gSwitches[198].unk0 == 1) {
                break;
            }
            if (sGame.unk5 == 0 || sGame.unk5 == 0x3A || sGame.unk5 == 0x3C) {
                if ((s16)gSpeedFixed >> 4 < 15) {
                    gSpeedFixed = 0xF0;
                }
                sGame.unk5 = 0x23;
                gMissState = 1;
                D_800959B0 = 0;
                gMissTimer = 0;
                sGame.unk3A8 = (s16)gSpeedFixed >> 4;
                SetSoundFade(4);
                goto top;
            }
            break;
        case 1:
            gMissTimer++;
            DrawFrozenFrame();
            if (gMissTimer == 10) {
                for (i = 0; i < 200; i++) {
                    gSwitches[i].unk0 = 2;
                }
                gFellOff = 0;
                D_800957D6 = 0;
                *(Words3 *)sSavedPos = *(Words3 *)D_8009EEC0;
            }
            if (gMissTimer >= 10) {
                sGame.unk3A8 = 0;
                gSpeedFixed = 0;
                *(Words3 *)&sGame.unk348 = *(Words3 *)sSavedPos;
            }
            if (gMissTimer > 100) {
                t = gMissTimer - 100;
                if (t > 60) {
                    gMissState = 100;
                }
            }
            break;
        case 2:
            StopCdTrack();
            PlaySe(0x1B);
            gMissTimer = 0;
            gMissState = 3;
            break;
        case 3:
            t = ++gMissTimer;
            if (t > 60) {
                gMissState = 100;
            }
            break;
        case 100:
            ClipF = 0;
            if (gRetries == 0) {
                D_8009EF4A[0] = 0;
                gGameMode = 6;
                gModeStep = 0xFFFF;
            } else {
                RestartStage();
                gRetries--;
            }
            break;
        case 110:
            t = ++gMissTimer;
            func_8003E360(t);
            if (t == 0x180) {
                gGameMode = 6;
            }
            break;
    }
end:;
}
```

`GameState` gained `unk3D1` (u8).

## Levers

Six builds: 17 -> 19 -> 53 -> 108 -> 334.

- **The compare tree: an empty `case -2: break;`.** Retail splits the
  switch at 1 (`beq 1; slti 2`, then `== -1; bltz; j case0` and `== 3;
  slt 3; == 100; == 110`). Seven cases split at 2. gcc 2.8 splits an
  n-node case list after node (n+1)/2, so the source had eight nodes
  with three below 1. The extra node, -2, shares the default label, which
  is why retail has a `bltz` to the exit and no `beq -2`. A `default:` in
  place of `case 2:` moved the split to 1 too, but broke the left side.
- The rank clamp at the top of state 0 reads `gBonusLevel` once into a
  local before testing `D_800958AC` (retail's delay slot holds the `x <
  3` test of the else arm). Spelt as in AddBonusLevel, with a reload in
  each arm, the blocks differ.
- States 1 and 3 share retail's tail through an `s16 t`: `t =
  gMissTimer - 100;` and `t = ++gMissTimer;` give the same
  `sll/sra; slti 61; li 100; sh` that cross-jumping merges. State 110
  keeps `t` across the call in `$s0`.
- `s32` with no return (and `goto end` for the first guard) leaves four
  delay slots as `nop`; `s32 unused[4]` gives the 0x28 frame. Both
  warnings baselined in config/typeviews-warnings.txt.
- The 12-byte copies are `Words3` struct copies; retail's 2+1 batch into
  the array and 3-word batch into `sGame.unk348` come out unchanged.

### Proposed learning

A switch whose compare tree splits one case lower than gcc 2.8 would
split the visible case list (it splits n nodes after node (n+1)/2): an
empty `case K: break;` that shares the default label, with K outside the
visible range. Retail shows no compare for K; the extra node shows only
in the split point and as a `bltz`/`bgez` to the exit. (UpdatePlayState)
