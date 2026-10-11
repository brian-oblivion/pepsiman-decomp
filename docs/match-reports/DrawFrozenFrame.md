# DrawFrozenFrame — MATCHED (90/90), round 6, runner echo

> Renamed from `func_8003C014` on 2026-10-10 (tools/rename.py). Address 0x8003c014.

Unit `src/stage.c`. Copies the displayed frame into VRAM at x 640
and draws it back as two full-screen sprites: registers sprites 0x1FE and
0x1FF (DefineSprite: 160x240 from texture pages 10 and 12, the second at
u 0x20), moves the 320x240 rectangle at y `PSDIDX * 240` to (640, 0) with
`MoveImage`, then draws both sprites at (-160, -120) and (0, -120) in colour
(0, 0x80, 0x80, 0x80) into the current ordering table.

```c
void DrawFrozenFrame(void) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, &color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, &color, 0, &gSubOt[gActiveBuff]);
}
```

First build. Frame: `RECT` at 0x28, `SVECTOR` at 0x30, `CVECTOR` at 0x38
(declaration order after 0x24 bytes of outgoing arguments). The repeated
`pos.vy = -120` is what keeps -120 in `s1` across the call.

Declarations added to the unit: `extern s16 D_800E474C;` (libgs's
`PSDIDX`, the double-buffer index: `GsInitGraph` clears it, and every
retail read is `lui`/`lh` into one register, a scalar);
`extern GsOT gOt[];` (DrawFrozenFrameFaded's table); and prototypes of
DefineSprite and DrawSprite, which draw defines. Those two are
stopgaps: this round, delta's branch puts DrawSprite in
`include/draw.h` (the same type) and DefineSprite in `common.h`,
and removes hit_tool.c's local DrawSprite. Until that merges,
declcheck reports DrawSprite as MULTI/LOCAL (hit_tool.c:103 and this
unit). Once it has merged, delete the block. Every argument here is a
constant, so the type of DefineSprite's `page` (u8 or s32, which charlie
and delta disagree on) does not affect these functions.
