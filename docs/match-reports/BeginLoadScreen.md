# BeginLoadScreen — MATCHED (84/84), round 7, runner bravo

> Renamed from `func_80015A28` on 2026-10-10 (tools/rename.py). Address 0x80015a28.

Sets up and draws the two half-screen sprites 0x1FE and 0x1FF (the
framebuffer copied with `MoveImage` to x 640, then drawn across the top of
the screen), clears the counter `gSubStep` and sets `gGameMode = 0x2A`.
BeginLoadScreen and BeginAct3LoadScreen differ only in that last constant.

```c
extern u16 gSubStep;

void BeginLoadScreen(void) {
    SVECTOR pos;
    RECT rect;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    gSubStep = 0;
    gGameMode = 0x2A;
}
```

Matched on the first build: the body of code_29f54's `func_8003C014` with
a NULL colour. The frame puts `pos` at 0x28 and `rect` at 0x30, so `pos`
is declared first (locals laid out in declaration order).
