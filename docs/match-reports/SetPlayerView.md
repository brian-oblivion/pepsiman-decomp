# SetPlayerView — MATCHED 132/132 (round 10, bravo)

> Renamed from `func_80026A60` on 2026-10-11 (tools/rename.py). Address 0x80026a60.

Unit `src/code_13068.c`. Fills the camera offsets in `gRefView`: the
orbit angle `gViewerAngle` (degrees) gives slots 1 and 2 at radii 500 and
600, slot 4 takes unk34C, and the yaw `D_800A7680->vy` gives slots 0/2 at
radius 400 and 3/5 at 900 from the opposite heading. It then steps
`D_8009587E` one towards unk3A8.

```c
extern u16 D_8009587E;

/* MATCHING: non-void with no return keeps the last branch's delay slot a nop. */
s32 SetPlayerView(void) {
    gRefView[0] = 0;
    gRefView[1] = rsin((gViewerAngle << 12) / 360) * 500 / 4096 + sGame.unk3C0;
    gRefView[2] = rcos((gViewerAngle << 12) / 360) * 600 / 4096;
    gRefView[4] = sGame.unk34C;
    gRefView[0] = rsin(D_800A7680->vy) * 400 / 4096;
    gRefView[2] = rcos(D_800A7680->vy) * 400 / 4096;
    gRefView[3] = rsin(D_800A7680->vy - 0x800) * 900 / 4096;
    gRefView[5] = rcos(D_800A7680->vy - 0x800) * 900 / 4096;
    if (D_8009587E < sGame.unk3A8) {
        D_8009587E++;
    }
    if (D_8009587E > sGame.unk3A8) {
        D_8009587E--;
    }
}
```

## Levers

- First build 120/132 (one word long): the second `beqz` filled its delay
  slot. `s32` with no return (LEARNINGS, "A near-miss that differs only
  in delay slots") closed it. Warning baselined in
  config/typeviews-warnings.txt.
- `(gViewerAngle << 12) / 360` and `* K / 4096` are code_7d74's spelling of
  the same orbit (func at code_7d74.c:344); they match unchanged.
- `D_8009587E` is reached `lui $at` for its stores: a plain extern, not in
  the unit's gp list.
