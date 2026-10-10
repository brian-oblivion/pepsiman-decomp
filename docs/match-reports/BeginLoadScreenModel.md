# BeginLoadScreenModel — MATCHED 147/147 (round 8, echo)

> Renamed from `func_800157DC` on 2026-10-10 (tools/rename.py). Address 0x800157dc.

Unit `src/main.c`. A mode setup in the family of BeginLoadScreen and
BeginAct3LoadScreen (same two sprite registrations, `MoveImage` and two sprite
draws), which then copies the 0x3D8-byte game state head into
`gLoadScreenModel` and resets fields of the copy, places `gRefView` at angle
0x800, radius 400, height -150, and switches to state 0x29.

```c
/**
 * @brief A 0x3D8-byte copy of the game state's head, reset for a demo or
 *        replay. Only the members this unit writes are named.
 */
typedef struct {
    u8 unk0;         /**< cleared */
    u8 unk1[4];      /**< not yet known */
    u8 unk5;         /**< the stepper kind; set to 0x61 */
    u8 unk6;         /**< set to 3 */
    u8 unk7[0x341];  /**< not yet known */
    s32 unk348;      /**< a position, x */
    s32 unk34C;      /**< a position, y */
    s32 unk350;      /**< a position, z */
    u8 unk354[0x2C]; /**< not yet known */
    s32 unk380;      /**< a rotation about y, as a word */
    u8 unk384[0x24]; /**< not yet known */
    s16 unk3A8;      /**< cleared */
    u8 unk3AA[0x2E]; /**< not yet known */
} StateCopy;

extern StateCopy gLoadScreenModel;

void BeginLoadScreenModel(void) {
    SVECTOR pos;
    RECT rect;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    gLoadScreenModel = *(StateCopy *)D_8009EB78;
    gLoadScreenModel.unk380 = 0x800;
    gLoadScreenModel.unk6 = 3;
    gLoadScreenModel.unk348 = 0;
    gLoadScreenModel.unk34C = 0;
    gLoadScreenModel.unk350 = 0;
    gLoadScreenModel.unk5 = 0x61;
    gLoadScreenModel.unk3A8 = 0;
    gLoadScreenModel.unk0 = 0;
    gRefView[0] = rsin(0x800) * 400 / 4096;
    gRefView[1] = -150;
    gRefView[2] = rcos(0x800) * 400 / 4096;
    gRefView[3] = 0;
    gRefView[4] = -150;
    gRefView[5] = 0;
    gSubStep = 0;
    gGameMode = 0x29;
}
```

First build. The four-`lw`/four-`sw` loop over 0x3D0 bytes plus an
eight-byte tail is a word-aligned struct copy; the copy type carries `s32`
members, so cc1 uses `lw`/`sw` rather than `lwl`/`lwr`. `gLoadScreenModel`'s
offset 0x380 is written as a word here (`sw` of 0x800), where DrawStepper
reads the head's 0x380 as a `u16` rotation, so the copy has its own view
(`StateCopy`) instead of reusing `Stepper`. The stores to the copy follow
retail's order; `rsin(0x800) * 400 / 4096` gives the `* 25 * 16` and the
`bgez`/`+0xFFF`/`sra 12` division as written.
