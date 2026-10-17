# EditorArrangeRecords — MATCHED 177/177 (round 9, alpha)

> Renamed from `func_80030DA0` on 2026-10-11 (tools/rename.py). Address 0x80030da0.

Unit `src/editor.c`. A two-state placement step on `gSubStep`.
State 0 clears two halfwords of the first waiting record `gPlaceQueue[0]`,
sets its position (the game position) and heading
(`sTotals.unk20 * 4096 / 360`), fetches the waiting records through
`PlaceStageTrap` into `gPlaceCount`, and falls into state 1. State 1 sets
the first record's position and heading again, defaults the light vector
`gFloorY[1..3]` when no entry is current, stores two `ratan2` angles,
updates its ObjectModel via `UpdateObjectEntry`, then places every later record:
its offset (`unkC`) transformed through the first record's coordinate
(`TransformByCoord`) becomes its position, its heading is the first one's
plus its own `unk20`, it copies the two angles, and its ObjectModel is updated.
Sixth build matched.

## Shapes read from retail

- **Frame 0x78 with 0x48 bytes of locals nobody touches: an unused
  record-sized local** (`EditorTrap unused;`), a variant of the LEARNINGS
  "unused local" entries.
- State 0 stores through `gPlaceQueue[0]` directly (`a1`, not saved);
  a pointer local kept across the call takes a saved register (5/177).
- **The later record's address `addu s0, s0(index), s2(base)`: an integer
  sum**, `(EditorTrap *)(i * sizeof(EditorTrap) + (u32)w)` (LEARNINGS,
  StepFloorIndex). `&w[i]`, `i + w` and `r = w; r += i;` all put the base
  first (176/177); `&gPlaceQueue[i]` takes a seventh saved register.
- `bnez` with a `nop` slot at the loop bottom: non-void with no return.
- The `PlaceStageTrap` id is passed unextended (`lhu`): `s32` in this
  unit's view of the prototype; the two record pointers pass through.

## Declarations

- `EditorTrap` (unit-local) gains `unk18`, `unk1A`, `unk1C`, `unk20`,
  `unk2C`, `unk30`.
- New in the `.c`: prototypes of `PlaceStageTrap` (per-unit view, MATCHING
  note) and `TransformByCoord` (as player defines it).
- `gLocalPos` (`SVECTOR`) and `gWorldPos` (`VECTOR`) moved from
  object.c into include/object.h, types unchanged (declcheck LOCAL
  once a second unit uses them). Deletes two lines in object.c.

```c
s32 EditorArrangeRecords(void) {
    /* MATCHING: an unused record-sized local gives retail's 0x78 frame. */
    EditorTrap unused;
    EditorTrap *w;
    EditorTrap *r;
    u16 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    switch (gSubStep) {
        case 0:
            gPlaceQueue[0].unk18 = 0;
            gPlaceQueue[0].unk1C = 0;
            gPlaceQueue[0].unk0[0] = sGameSave.unk348[0];
            gPlaceQueue[0].unk0[1] = sGameSave.unk348[1];
            gPlaceQueue[0].unk0[2] = sGameSave.unk348[2];
            gPlaceQueue[0].unk1A = (sTotals.unk20 << 12) / 360;
            gPlaceCount = PlaceStageTrap(sTotals.unk1C, gPlaceQueue, gPlaceQueue);
            gSubStep++;
            /* fall through */
        case 1:
            w = gPlaceQueue;
            w->unk0[0] = sGameSave.unk348[0];
            w->unk0[1] = sGameSave.unk348[1];
            w->unk0[2] = sGameSave.unk348[2];
            w->unk1A = (sTotals.unk20 << 12) / 360;
            if (gCurBlock == -1) {
                gFloorY[1] = 0;
                gFloorY[2] = -0x1000;
                gFloorY[3] = 0;
            }
            w->unk2C = ratan2(-gFloorY[3], gFloorY[2]);
            w->unk30 = ratan2(-gFloorY[1], gFloorY[2]);
            UpdateObjectEntry(&gObjectModels[w->unk36], (Trap *)w);
            for (i = 1; i < gPlaceCount; i++) {
                /* MATCHING: an integer sum puts the index first in the addu. */
                r = (EditorTrap *)(i * sizeof(EditorTrap) + (u32)w);
                gLocalPos.vx = r->unkC[0];
                gLocalPos.vy = r->unkC[1];
                gLocalPos.vz = r->unkC[2];
                TransformByCoord((GsCOORDINATE2 *)gObjectModels[w->unk36].unk10, &gLocalPos, &gWorldPos);
                r->unk0[0] = gWorldPos.vx;
                r->unk0[1] = gWorldPos.vy;
                r->unk0[2] = gWorldPos.vz;
                r->unk1A = w->unk1A + r->unk20;
                r->unk2C = w->unk2C;
                r->unk30 = w->unk30;
                UpdateObjectEntry(&gObjectModels[r->unk36], (Trap *)r);
            }
            break;
    }
}
```
