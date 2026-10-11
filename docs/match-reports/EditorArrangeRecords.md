# EditorArrangeRecords — MATCHED 177/177 (round 9, alpha)

> Renamed from `func_80030DA0` on 2026-10-11 (tools/rename.py). Address 0x80030da0.

Unit `src/code_1dc24.c`. A two-state placement step on `gSubStep`.
State 0 clears two halfwords of the first waiting record `D_80096788[0]`,
sets its position (the game position) and heading
(`sTotals.unk20 * 4096 / 360`), fetches the waiting records through
`CallStageHook` into `D_8009596E`, and falls into state 1. State 1 sets
the first record's position and heading again, defaults the light vector
`gFloorY[1..3]` when no entry is current, stores two `ratan2` angles,
updates its Rec78 via `UpdateObjectEntry`, then places every later record:
its offset (`unkC`) transformed through the first record's coordinate
(`TransformByCoord`) becomes its position, its heading is the first one's
plus its own `unk20`, it copies the two angles, and its Rec78 is updated.
Sixth build matched.

## Shapes read from retail

- **Frame 0x78 with 0x48 bytes of locals nobody touches: an unused
  record-sized local** (`Obj48 unused;`), a variant of the LEARNINGS
  "unused local" entries.
- State 0 stores through `D_80096788[0]` directly (`a1`, not saved);
  a pointer local kept across the call takes a saved register (5/177).
- **The later record's address `addu s0, s0(index), s2(base)`: an integer
  sum**, `(Obj48 *)(i * sizeof(Obj48) + (u32)w)` (LEARNINGS,
  UpdateSection). `&w[i]`, `i + w` and `r = w; r += i;` all put the base
  first (176/177); `&D_80096788[i]` takes a seventh saved register.
- `bnez` with a `nop` slot at the loop bottom: non-void with no return.
- The `CallStageHook` id is passed unextended (`lhu`): `s32` in this
  unit's view of the prototype; the two record pointers pass through.

## Declarations

- `Obj48` (unit-local) gains `unk18`, `unk1A`, `unk1C`, `unk20`,
  `unk2C`, `unk30`.
- New in the `.c`: prototypes of `CallStageHook` (per-unit view, MATCHING
  note) and `TransformByCoord` (as code_13068 defines it).
- `D_800957E4` (`SVECTOR`) and `D_8009F268` (`VECTOR`) moved from
  code_1a098.c into include/code_1a098.h, types unchanged (declcheck LOCAL
  once a second unit uses them). Deletes two lines in code_1a098.c.

```c
s32 EditorArrangeRecords(void) {
    /* MATCHING: an unused record-sized local gives retail's 0x78 frame. */
    Obj48 unused;
    Obj48 *w;
    Obj48 *r;
    u16 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    switch (gSubStep) {
        case 0:
            D_80096788[0].unk18 = 0;
            D_80096788[0].unk1C = 0;
            D_80096788[0].unk0[0] = sGameSave.unk348[0];
            D_80096788[0].unk0[1] = sGameSave.unk348[1];
            D_80096788[0].unk0[2] = sGameSave.unk348[2];
            D_80096788[0].unk1A = (sTotals.unk20 << 12) / 360;
            D_8009596E = CallStageHook(sTotals.unk1C, D_80096788, D_80096788);
            gSubStep++;
            /* fall through */
        case 1:
            w = D_80096788;
            w->unk0[0] = sGameSave.unk348[0];
            w->unk0[1] = sGameSave.unk348[1];
            w->unk0[2] = sGameSave.unk348[2];
            w->unk1A = (sTotals.unk20 << 12) / 360;
            if (D_80095824 == -1) {
                gFloorY[1] = 0;
                gFloorY[2] = -0x1000;
                gFloorY[3] = 0;
            }
            w->unk2C = ratan2(-gFloorY[3], gFloorY[2]);
            w->unk30 = ratan2(-gFloorY[1], gFloorY[2]);
            UpdateObjectEntry(&D_800D8D20[w->unk36], (Rec48 *)w);
            for (i = 1; i < D_8009596E; i++) {
                /* MATCHING: an integer sum puts the index first in the addu. */
                r = (Obj48 *)(i * sizeof(Obj48) + (u32)w);
                D_800957E4.vx = r->unkC[0];
                D_800957E4.vy = r->unkC[1];
                D_800957E4.vz = r->unkC[2];
                TransformByCoord((GsCOORDINATE2 *)D_800D8D20[w->unk36].unk10, &D_800957E4, &D_8009F268);
                r->unk0[0] = D_8009F268.vx;
                r->unk0[1] = D_8009F268.vy;
                r->unk0[2] = D_8009F268.vz;
                r->unk1A = w->unk1A + r->unk20;
                r->unk2C = w->unk2C;
                r->unk30 = w->unk30;
                UpdateObjectEntry(&D_800D8D20[r->unk36], (Rec48 *)r);
            }
            break;
    }
}
```
