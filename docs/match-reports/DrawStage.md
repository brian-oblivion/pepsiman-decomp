# DrawStage — MATCHED 129/129 (round 9, bravo)

> Renamed from `func_8003A008` on 2026-10-10 (tools/rename.py). Address 0x8003a008.

Unit `src/code_29f54.c`. Draws the stage pieces listed for the current
section: places `gStageCoord` at (x, y, z), builds its local-screen matrix,
then on the scratchpad stack sorts each object of the first model set whose
index is listed in `gStageRuns[gSection]` (a start/count run into the byte
list `gStageRunObjs`), then the same for the second set (`gPropRuns`,
`gPropRunObjs`), skipping an index of -1. Same shape as DrawRepeatingStage below it.

```c
void DrawStage(s16 x, s16 y, s16 z) {
    s32 i;
    s32 k;
    SVECTOR rot;
    MATRIX ls;

    gStageCoord.coord.t[0] = x;
    gStageCoord.coord.t[1] = y;
    gStageCoord.coord.t[2] = z;
    rot.vx = rot.vy = rot.vz = 0;
    SetCoordRotation(&rot, &gStageCoord);
    GsGetLs(&gStageCoord, &ls);
    GsSetLsMatrix(&ls);
    SetSpadStack();
    for (i = 0; i < gStageRuns[gSection].count; i++) {
        k = gStageRunObjs[gStageRuns[gSection].start + i];
        if ((u32)k < gStageObjCount) {
            GsSortObject4J(&gStageObjs[k], gCurOt, 2, (u_long *)0x1F800000);
        }
    }
    for (i = 0; i < gPropRuns[gSection].count; i++) {
        k = gPropRunObjs[gPropRuns[gSection].start + i];
        if ((u32)k < gPropObjCount && k != -1) {
            GsSortObject4J(&gPropObjs[k], gCurOt, 2, (u_long *)0x1F800000);
        }
    }
    ResetSpadStack();
}
```

## Externs and types

- `gStageRuns`/`gPropRuns`: pointers to a unit-local `Run { s16 start;
  s16 count; }` (the `lh 0x0`/`lh 0x2` after the `sll 2` index).
  `gStageRunObjs`/`gPropRunObjs`: `s8 *` (`lb`, compared unsigned against the
  `u32` counts, so `(u32)k`). All four are `lui` loads, plain externs.
- The loop bound and both pointers are reloaded every pass because the
  `GsSortObject4J` call may change them; no local copies.

## The one residue: the nop after ResetSpadStack

The first build was one word short: retail has `lw $sp, 0($sp); nop; lw $ra,
0x40($sp)`, the build had no `nop`. maspsx decides the load-delay nop by
comparing register NAMES as text; the macro loaded `$29`, the epilogue reads
`$sp`, so it saw no hazard. `include/spad.h`'s `ResetSpadStack` now spells
the load `lw $sp,0($sp)` (the same instruction); the whole build, main.c's
use included, stays byte-exact. This is a change to an existing macro.

### Proposed learning

- **A missing `nop` right after inline asm that loads a register: maspsx
  compares register names as text**, so a load of `$29` followed by a read
  of `$sp` gets no load-delay nop. Spell the inline asm's registers the way
  cc1 prints them (`$sp`). (DrawStage, include/spad.h)
