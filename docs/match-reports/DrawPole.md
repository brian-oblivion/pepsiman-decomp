# DrawPole — MATCHED 89/89 (round 8, bravo)

> Renamed from `func_80033224` on 2026-10-11 (tools/rename.py). Address 0x80033224.

Unit `src/code_1dc24.c`. Sorts a GsLINE from `pos` (offset by
`gWorldShift`) to 200 units above it, red 0xFF with green and blue pulsing
with the frame counter `gFrameCount`. It is the twin of code_24748's
`DrawToolPole` without that function's negated x and z; that body, with
`+ pos->vx` / `+ pos->vz`, matched on the first build.

The unit now includes `code_a0bc.h` for `gSubOt` (the ordering
tables); no header was changed.

```c
void DrawPole(VECTOR *pos) {
    s32 unused[2];
    VECTOR world;
    SVECTOR screen;
    GsLINE line;
    s16 g;

    g = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = pos->vx + gWorldShift[0];
    world.vy = pos->vy;
    world.vz = pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vy = pos->vy - 200;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
}
```
