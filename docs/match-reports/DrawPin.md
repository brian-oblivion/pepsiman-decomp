# DrawPin — MATCHED 74/74 (round 4, charlie)

> Renamed from `func_80033F48` on 2026-10-11 (tools/rename.py). Address 0x80033f48.

Unit `src/hit_tool.c`. Draws a bobbing marker above a world position: the
position plus the offset in `gWorldShift[0]`/`[2]` and, in y, a sine bob of
amplitude 10 around -200 (phase `gFrameCount * 10` degrees, converted to
libgte's 4096-per-turn units), projected by `ProjectToScreen`, then sprite
0x15E drawn at the screen point offset by (-7, -32) through
`DrawSprite` (handwritten), with the current `gSubOt` entry.

```c
void DrawPin(VECTOR *pos) {
    SVECTOR size;
    VECTOR world;
    SVECTOR screen;
    s32 bob;

    world.vx = pos->vx + gWorldShift[0];
    bob = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 10) >> 12) - 200;
    world.vy = pos->vy + bob;
    world.vz = pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    size.vx = screen.vx - 7;
    size.vy = screen.vy - 32;
    DrawSprite(0x15E, &size, 0, 5, &gSubOt[gActiveBuff]);
}
```

## Externs and types

- `gFrameCount`: `lui`/`lw` same register, a scalar; the `% 360` and
  `/ 360` are unsigned (`srl 3` then `multu` by 0x16C16C17), so `u32`.
- `gWorldShift`: `s32 []` in `common.h`, split `lui`/`%lo` as retail.
- Stack: `size` 0x18, `world` 0x20, `screen` 0x30, in declaration order.
- `ProjectToScreen` is defined in `player.c` with `VECTOR *`/`SVECTOR *`
  parameters. Its prototype cannot go into `include/common.h`, which is
  included before `libgte.h` (VECTOR undefined there: "parse error before
  `*'", tried); it sits in this unit's `.c`, which declcheck reports as
  LOCAL. Needs a header that sees libgte's types (head's call).
- `DrawSprite` is handwritten; a unit-local prototype (`u16` id: it
  masks `$a0`).

## Levers

| C for the y coordinate | result |
| --- | --- |
| `pos->vy + ((rsin(...) * 10) >> 12) - 200` | 70/74: -200 added to the sum last |
| `pos->vy + ((... >> 12) - 200)`, and `+ -200`, and the operands swapped | 69/74: -200 folded onto `pos->vy` |
| **the bob in its own statement, `bob = (... >> 12) - 200; world.vy = pos->vy + bob;`** | **74/74** |

Retail adds -200 to the shifted sine before adding the position; only a
separate statement keeps cc1 from reassociating the constant onto the load.

### Proposed learning

A constant added to a computed term BEFORE that term is added to a loaded
value, where parenthesising does not hold it: assign the term with its
constant to a local in its own statement, then add. (DrawPin; contrast
ScatterDust where parentheses sufficed)
