# DrawAxes — MATCHED 115/115 (round 9, charlie)

> Renamed from `func_8002B5FC` on 2026-10-11 (tools/rename.py). Address 0x8002b5fc.

Unit `src/object.c`. Debug axes: projects (0, ±200, 0), (±200, 0, 0)
and (0, 0, ±200) through ProjectToScreen and sorts a white `GsLINE` between
each pair into `gSubOt[gActiveBuff]`, at priority 50, 50 and `world.vz
>> 2` (the third reads `vz` back from the stack after the call).

```c
/** @brief Sorts three white lines 400 units long through the origin, one
 *         along each axis, into the current ordering table. */
void DrawAxes(void) {
    VECTOR world;
    SVECTOR screen;
    GsLINE line;

    line.attribute = 0;
    line.r = 0xFF;
    line.g = 0xFF;
    line.b = 0xFF;
    world.vx = 0;
    world.vy = -200;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 200;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
    world.vx = -200;
    world.vy = 0;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 200;
    world.vy = 0;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
    world.vx = 0;
    world.vy = 0;
    world.vz = -200;
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 0;
    world.vz = 200;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], world.vz >> 2);
}```

First build; the shape is editor's line drawers (DrawHeadingLine and
its neighbour). The unit now includes `draw.h` for `gSubOt`.
