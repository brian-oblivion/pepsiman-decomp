# DrawToolPin — MATCHED 74/74 (round 4, charlie)

> Renamed from `func_80036478` on 2026-10-11 (tools/rename.py). Address 0x80036478.

Unit `src/hit_tool.c`. The mirrored twin of `DrawPin`: the same
bobbing marker (sprite 0x15E, offset (-7, -32)), but x and z are the offset
in `gWorldShift` MINUS the position.

```c
void DrawToolPin(VECTOR *pos) {
    SVECTOR size;
    VECTOR world;
    SVECTOR screen;
    s32 bob;

    world.vx = -pos->vx + gWorldShift[0];
    bob = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 10) >> 12) - 200;
    world.vy = pos->vy + bob;
    world.vz = -pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    size.vx = screen.vx - 7;
    size.vy = screen.vy - 32;
    DrawSprite(0x15E, &size, 0, 5, &gSubOt[gActiveBuff]);
}
```

Externs, types and the `bob` statement: see `DrawPin`.

## Levers

| C for x and z | result |
| --- | --- |
| `gWorldShift[0] - pos->vx` | 70/74: the global loaded before the parameter's word (both pairs) |
| `-(pos->vx - gWorldShift[0])` | 70/74, the same |
| **`-pos->vx + gWorldShift[0]`** | **74/74**: `pos->vx` loaded first, then `subu` |

The operand written first is loaded first, even though both forms compile
to the same `subu`.

### Proposed learning

A `subu` whose subtrahend is loaded BEFORE the minuend: the source wrote the
negated operand first, `-b + a`. `a - b` loads `a` first. (DrawToolPin)
