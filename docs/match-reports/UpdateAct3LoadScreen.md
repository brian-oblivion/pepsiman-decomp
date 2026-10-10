# UpdateAct3LoadScreen — MATCHED (83/83), round 7, runner bravo

> Renamed from `func_80017124` on 2026-10-10 (tools/rename.py). Address 0x80017124.

Draws three sprites into the current ordering table (a digit-like sprite
`0xFB + frame % 10` at (0x70, 0x48), then sprites 0x1FE and 0x1FF across
the top of the screen), calls `LoadAct3Stage`, and when the counter
`gSubStep` reaches 100 resets it and sets `gModeStep = 4`,
`gGameMode = 14`.

```c
void LoadAct3Stage(void);

extern u16 gSubStep;

void UpdateAct3LoadScreen(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    func_8001B354(gFrameCount % 10 + 0xFB, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    LoadAct3Stage();
    if (gSubStep == 100) {
        gModeStep = 4;
        gSubStep = 0;
        gGameMode = 14;
    }
}
```

Matched on the first build, modelled on code_29f54's `func_8003C014`.
cc1 keeps the repeated `-0x78` in `$s1` across the calls on its own.

## Declarations

- `src/main.c` now includes `code_a0bc.h` (for `func_8001B354` and
  `gSubOt`).
- `gModeStep` (s16) and `gGameMode` (u16) moved to `include/common.h`
  (additive); their local externs in `src/code_308ec.c` were removed.
