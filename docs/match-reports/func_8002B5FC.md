# func_8002B5FC — MATCHED 115/115 (round 9, charlie)

Unit `src/code_1a098.c`. Debug axes: projects (0, ±200, 0), (±200, 0, 0)
and (0, 0, ±200) through func_800230E0 and sorts a white `GsLINE` between
each pair into `D_800ACEA8[D_80095750]`, at priority 50, 50 and `world.vz
>> 2` (the third reads `vz` back from the stack after the call).

```c
/** @brief Sorts three white lines 400 units long through the origin, one
 *         along each axis, into the current ordering table. */
void func_8002B5FC(void) {
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
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 200;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
    world.vx = -200;
    world.vy = 0;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 200;
    world.vy = 0;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
    world.vx = 0;
    world.vy = 0;
    world.vz = -200;
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 0;
    world.vz = 200;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], world.vz >> 2);
}```

First build; the shape is code_1dc24's line drawers (func_80033388 and
its neighbour). The unit now includes `code_a0bc.h` for `D_800ACEA8`.
