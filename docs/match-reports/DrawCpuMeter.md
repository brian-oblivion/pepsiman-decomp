# DrawCpuMeter — MATCHED 136/136 (round 9, bravo)

> Renamed from `func_8003BDF4` on 2026-10-10 (tools/rename.py). Address 0x8003bdf4.

Unit `src/stage.c`. Draws the frame-time meter: one flat quad per
recorded sample (`gCpuMarkTimes[i]`, the `VSync(1)` counts that the recorder
above stores, coloured from the three byte tables beside it), each running
from this sample's height down to the previous one's, then a white tick mark,
and clears the sample count. First build matched.

```c
void DrawCpuMeter(void) {
    POLY_F4 *p;
    s32 i;

    for (i = 0; i < gCpuMarkCount; i++) {
        if (gCpuMarkTimes[i] != 0xFF) {
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = -152;
            p->y0 = p->y1 = (gCpuMarkTimes[i] >> 1) - 120;
            if (i != 0) {
                p->y3 = (gCpuMarkTimes[i - 1] >> 1) - 120;
            } else {
                p->y3 = -120;
            }
            p->y2 = p->y3;
            p->r0 = sCpuMarkR[i];
            p->g0 = sCpuMarkG[i];
            p->b0 = sCpuMarkB[i];
            addPrim(gCurOt->org, p);
            p++;
            GsOUT_PACKET_P = (u8 *)p;
        }
    }
    p = (POLY_F4 *)GsOUT_PACKET_P;
    setPolyF4(p);
    p->x0 = p->x2 = -160;
    p->x1 = p->x3 = -144;
    p->y0 = p->y1 = 8;
    p->y2 = p->y3 = 9;
    setRGB0(p, 0xFF, 0xFF, 0xFF);
    gCpuMarkCount = 0;
    addPrim(gCurOt->org, p);
    p++;
    GsOUT_PACKET_P = (u8 *)p;
}
```

## Notes

- `sh y3; lhu y3; sh y2` after the if/else: `p->y2 = p->y3;` as its own
  statement after the branch. cc1 does not carry the stored value across the
  join, so the member is read back; no volatile needed (contrast
  SortTmdEnvMapGT3's volatile chained store, where both stores share a block).
- The final quad's colour is stored r, g, b from one register:
  `setRGB0`. A chained `r0 = g0 = b0` would store `b0` first.
- Existing learnings that held: chained assignment stores right to left;
  `addPrim(ot, p); p++; G = (u8 *)p;`.

### Proposed learning

- **A member stored in both arms of an if/else and read back right after
  the join (`sh X; lhu X; sh Y`): `Y = X;` as a statement after the branch**,
  not a volatile. (DrawCpuMeter)
