# LaunchMarkedEffect — MATCHED 36/36 (round 6, alpha)

> Renamed from `func_8002D230` on 2026-10-11 (tools/rename.py). Address 0x8002d230.

Unit `src/object.c`. On even frames (bit 0 of the frame counter
`gFrameCount` clear) finds the first block entry whose mark in `D_800A7550`
is 1, launches effect 4 at that entry's position (y raised by 50) through
`SpawnEffect`, and marks the entry 2.

```c
void LaunchMarkedEffect(void) {
    u32 i;
    Ent8 *e;

    if (gFrameCount & 1) {
        return;
    }
    for (i = 0; i < 200; i++) {
        if ((s8)D_800A7550[i] == 1) {
            e = (Ent8 *)(i * 8 + (u32)D_800959C4);
            SpawnEffect(4, e->unk0, e->unk2 - 50, (s16)e->unk4, 0);
            D_800A7550[i] = 2;
            return;
        }
    }
}
```

Third build.

- Build 1, `e = &((Ent8 *)D_800959C4)[i];`: 32/36, the entry address
  built `addu $v1, $v1, $v0` (base first) where retail has `addu $v0, $v0,
  $v1` (scaled index first); the next three loads follow the register.
- Build 2, `(Ent8 *)(i * 8 + D_800959C4)`: unchanged; pointer arithmetic
  still puts the pointer first.
- Build 3, `(Ent8 *)(i * 8 + (u32)D_800959C4)`: 36/36. The LEARNINGS entry
  for UpdateSection/UpdateStageLights again.

Declarations: `gFrameCount` is a scalar (`lui $v0` / `lw $v0, %lo` in one
register); it was a local extern in hit_tool.c, so it moved to
`include/common.h` (additive) and the local copy was deleted.
`SpawnEffect` gets this unit's `s32` view (the `lh` results and the
`addiu -50` go out unextended), as player already does.
