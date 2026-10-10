# OrbitViewerCamera — MATCHED (78/78), round 7, runner alpha

> Renamed from `func_80017DD4` on 2026-10-10 (tools/rename.py). Address 0x80017dd4.

Unit `src/code_7d74.c`. Sets the reference view `gRefView` (the
GsRVIEW2 that InitView hands to GsSetRefView2) to orbit the origin
at radius 400 and height -100, at the angle `gViewerAngle` degrees
(converted to 4096ths of a turn), looking at (0, -100, 0); bits 4 and 8
of `gPad` turn the angle by +2 and -2.

```c
extern s16 gViewerAngle;  /* this unit */
extern s32 gPad;  /* common.h, added this round */

void OrbitViewerCamera(void) {
    gRefView[0] = rcos((gViewerAngle << 12) / 360) * 400 / 4096;
    gRefView[1] = -100;
    gRefView[2] = rsin((gViewerAngle << 12) / 360) * 400 / 4096;
    gRefView[6] = 0;
    gRefView[3] = 0;
    gRefView[4] = -100;
    gRefView[5] = 0;
    if (gPad & 4) {
        gViewerAngle += 2;
    }
    if (gPad & 8) {
        gViewerAngle -= 2;
    }
}
```

## Attempts (1)

1. The body above, stores in retail's order: 78/78 on the first build.

## Declarations

`gPad` was declared only locally in `src/code_13068.c` (line 59).
It is now in `include/common.h` (an additive edit). code_13068 is a
round-7 unit, so its local line stays and declcheck reports MULTI/LOCAL
for it until the head deletes `src/code_13068.c`'s `extern s32
gPad;` at merge. Both units reach it the same way (`lui`/`lw`, a
plain `s32`).
