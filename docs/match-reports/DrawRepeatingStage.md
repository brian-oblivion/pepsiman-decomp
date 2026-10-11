# DrawRepeatingStage — MATCHED (122/122), round 8, runner charlie

> Renamed from `func_8003A20C` on 2026-10-10 (tools/rename.py). Address 0x8003a20c.

Unit `src/stage.c`. Draws a scrolling strip of 5000-unit model
segments around a position: for the three segments i = -1, 0, 1 it picks a
model (`k`: 1 or 2 alternating by segment parity, 0 for the segment holding
the position, 3 for one just past `range`; segments outside are skipped),
places the shared coordinate system `gStageCoord` at the segment's offset
(x = `pos mod 5000` + (i-1)*5000, z = the third argument), and sorts both
object sets (`gStageObjs[k]`, `gPropObjs[k]`) into `gCurOt`. The second
parameter is never read.

```c
#include "model.h" /* SetCoordRotation */
/* gCurOt: include/draw.h */

void DrawRepeatingStage(s32 pos, s32 unused, s32 z, s32 range) {
    s32 i;
    s32 k;
    s32 d;
    s32 r;
    SVECTOR rot;
    MATRIX ls;

    for (i = -1; i < 2; i++) {
        k = (((pos + 10000000) / 5000 + i) & 1) + 1;
        d = pos - i * 5000;
        if (d >= 0) {
            if (d >= 5000) {
                continue;
            }
            k = 0;
        }
        if (d < -range) {
            if (d < -(range + 5000)) {
                continue;
            }
            k = 3;
        }
        if (pos >= 0) {
            r = pos % 5000;
        } else {
            r = (pos + 10000000) % 5000;
        }
        gStageCoord.coord.t[0] = r + (i - 1) * 5000;
        gStageCoord.coord.t[1] = 0;
        gStageCoord.coord.t[2] = z;
        rot.vx = rot.vy = rot.vz = 0;
        SetCoordRotation(&rot, &gStageCoord);
        GsGetLs(&gStageCoord, &ls);
        GsSetLsMatrix(&ls);
        GsSortObject4J(&gStageObjs[k], gCurOt, 2, (u_long *)0x1F800000);
        GsSortObject4J(&gPropObjs[k], gCurOt, 2, (u_long *)0x1F800000);
    }
}```

## Levers

- **`move s1, zero` in the `slti 5000` branch's delay slot is a real
  `k = 0;`**, for the `0 <= d < 5000` path: `if (d >= 0) { if (d >= 5000)
  continue; k = 0; }`, which also gives retail's `bltz` before the `slti`.
  A plain `if (d >= 5000) continue;` drops both.
- **Parity as `& 1`**: `% 2` adds the signed-remainder fixup.
- **The x offset as `(i - 1) * 5000`**: strength reduction then starts the
  induction register at -10000 and adds it directly; `i * 5000 - 5000`
  starts at -5000 and adds an `addiu -0x1388`.
- **Declarations moved:** `gCurOt` (`GsOT *`) was a local extern in
  main.c; with a second user it moved to `include/draw.h`, beside the
  ordering tables it points into (additive there; the one-line extern in
  main.c removed). common.h cannot hold it: it does not see libgs.
  This unit now includes `model.h` for SetCoordRotation.
