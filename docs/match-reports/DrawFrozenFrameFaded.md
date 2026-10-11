# DrawFrozenFrameFaded — MATCHED (91/91), round 6, runner echo

> Renamed from `func_8003C17C` on 2026-10-10 (tools/rename.py). Address 0x8003c17c.

Unit `src/stage.c`. DrawFrozenFrame with a brightness parameter:
the same two sprite registrations and `MoveImage`, then both sprites drawn
in colour (-1, level, level, level) with mode 0xFFF into the
`gOt` ordering tables instead of `gSubOt`.

```c
void DrawFrozenFrameFaded(u8 level) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = PSDIDX * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    *(s8 *)&color.r = -1;
    color.g = color.b = color.cd = level;
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, &color, 0xFFF, &gOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, &color, 0xFFF, &gOt[gActiveBuff]);
}
```

The first build missed one word: `color.r = -1` on the `u_char` member
gives `li v0, 0xFF`, while retail has `li v0, -1`. Storing through an `s8`
lvalue, `*(s8 *)&color.r = -1`, matches. This is LEARNINGS' "byte store of
-1 is an s8" with a Sony struct whose member type cannot change. The
parameter is only stored with `sb`, so `u8` and `s32` both fit; `u8` is the
type it has. Declarations: see DrawFrozenFrame's report (the stopgap
prototypes and their declcheck note).

### Proposed learning

- **A byte store of `li -1` into a `u_char` member of a Sony struct (e.g.
  `CVECTOR.r`): store through an `s8` lvalue**, `*(s8 *)&c.r = -1;`. The
  plain `c.r = -1` gives `li 0xFF`. (DrawFrozenFrameFaded)
