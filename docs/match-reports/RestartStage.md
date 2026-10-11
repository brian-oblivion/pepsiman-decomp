# RestartStage — MATCHED 127/127 (round 6, delta)

> Renamed from `func_8004079C` on 2026-10-10 (tools/rename.py). Address 0x8004079c.

Unit `src/stage_flow.c`. Stage start. Raises `D_80095900`; when the
argument's remainder by 3 is 2 it only sets `gModeStep` to 2. Otherwise it
resets the head of the game state (`gPlayer`), copies the start position
from `gCourse`, looks up a per-stage value when the stage's flag in
`D_8007AD58` is 1, derives the camera height (`D_8009676C`) and the start
heading (from `D_80095840`), runs the stage set-up calls and clears a
halfword in each of the 200 Rec48 records.

```c
/* Unit-local views (all in src/stage_flow.c):
 *   Pos3       { s32 vx, vy, vz; }
 *   StageState  gPlayer's head: u8 unk0, unk2, unk6; Pos3 unk348 at
 *               0x348; s32 unk37C, unk380; s16 unk3C8, unk3CA
 *   Rec48Clear  object's Rec48 with s16 unk26
 *   Placement   { SVECTOR rot; Pos3 pos; }  (gCourse)
 *   StartRec    { u8 unk0[6]; s16 unk6; }   (*D_80095840)
 * extern u8 D_80095900, gInEditor; extern s16 gModeStep, gMissState,
 * D_800957B0; extern s8 D_8007AD58[]; extern s32 D_800957F4, gSection;
 * common.h: gBonusLevel (s32), D_800957BC (s16), D_8009676C[], gFellOff,
 * D_800957D6, gStage.
 * s32 FindFloor(Pos3 *pos); the rest void (void) except
 * PlayStageMusic(u8) and func_80041118(u8). */
void RestartStage(u8 arg0) {
    s32 unused[2];
    s16 yaw;
    u32 i;

    D_80095900 = 1;
    if (arg0 % 3 == 2) {
        gModeStep = 2;
        return;
    }
    sStage.unk3C8 = 0;
    sStage.unk2 = 1;
    gBonusLevel = 3;
    sStage.unk348.vx = gCourse.pos.vx;
    sStage.unk348.vy = gCourse.pos.vy;
    sStage.unk348.vz = gCourse.pos.vz;
    gMissState = 0;
    sStage.unk3CA = 0;
    if (D_8007AD58[gStage] == 1) {
        D_800957F4 = FindFloor(&sStage.unk348);
    } else {
        D_800957F4 = 0;
    }
    gSection = 0;
    yaw = -D_80095840->unk6;
    D_8009676C[0] = (s16)(gCourse.pos.vy - 500);
    gCourse.rot.vy = yaw;
    sStage.unk37C = sStage.unk380 = yaw;
    D_800957BC = yaw;
    D_800957B0 = yaw;
    if (gInEditor != 1) {
        RestoreStageObjects();
    } else {
        RestoreToolBuffer();
    }
    ResetStageObjects();
    func_80041118(gStage);
    sStage.unk6 = 2;
    sStage.unk0 = 1;
    gFellOff = 0;
    D_800957D6 = 1;
    for (i = 0; i < 200; i++) {
        sRecs48[i].unk26 = 0;
    }
    ResetStage();
    StartStageOverlay();
    PlayStageMusic(gStage);
}
```

## Levers

- **Frame 0x20 with nothing on the stack: `s32 unused[2];`** (LEARNINGS,
  ResetClearColor). Without it the frame is 0x18 and `ra` sits at 0x10.
- **The position is three field assignments, not a struct copy.** As a
  12-byte struct copy cc1 builds the destination address in a register
  first (`addiu a0, v1, 0x348`) and stores the words straight after the
  loads; retail loads the three words, interleaves the stores with the
  other resets and builds `&unk348` only in the `jal` delay slot. With a
  16-byte VECTOR it even copied a fourth word. Same family as bravo's
  round-6 lever on separate `s32` assignments.
- **Statement order around the heading.** `gSection = 0; yaw =
  -D_80095840->unk6; D_8009676C[0] = (s16)(gCourse.pos.vy - 500);
  gCourse.rot.vy = yaw;` matches. With the `D_8009676C` store before
  the heading read, the build stored it right after its `sra` and moved
  `gSection` down (107/127).
- `(s16)(pos.vy - 500)` comes out as retail's `lhu` / `addiu -0x1F4` /
  `sll`/`sra`: cc1 narrows the word load itself.
- `sStage.unk37C = sStage.unk380 = yaw;` stores 0x380 first (chained
  assignment stores right to left).
- The record loop clears `sRecs48[i].unk26` through a unit view with an
  `s16` member. A cast of the record address,
  `((s16 *)&sRecs48[i])[0x13]`, folds 0x26 into the loop pointer (`sh
  zero, 0(a1)`); `*(s16 *)sRecs48[i].unk26` on object's `u8 unk26[2]`
  did the same.
- `if (gInEditor != 1) RestoreStageObjects(); else RestoreToolBuffer();` gives
  retail's `beq` to the second call.

## Declarations

- `gBonusLevel` (`s32`) and `D_800957BC` (`s16`) are now in
  `include/common.h` (additive); their copies in `src/player.c` were
  deleted (same types).
- `gCourse` is an `SVECTOR[]` in player and a `Placement` here;
  both declarations carry a `MATCHING:` note.
- `FindFloor` takes a `Rec48 *` in object's view and a `Pos3 *`
  here (`&sStage.unk348`); this unit's prototype has a `MATCHING:` note.
- The `unused` local is a deliberate `-Wall` warning, NEW in
  `tools/typeviews.py --warnings`, for the head to baseline.
