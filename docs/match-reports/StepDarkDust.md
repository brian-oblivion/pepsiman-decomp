# StepDarkDust — MATCHED (84/84), round 6, runner echo

> Renamed from `StepEffect6` on 2026-10-11 (tools/rename.py). Address 0x8003fe5c.

> Renamed from `func_8003FE5C` on 2026-10-10 (tools/rename.py). Address 0x8003fe5c.

Unit `src/stage.c`. StepDust with colour mode 2 in `color.r`
(the same sprite 0x12D, size `frame * 10 + 100`, fade
`0x80 - frame * 16`, 8 frames).

```c
s32 StepDarkDust(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 * 10 + 100;
        size.vy = p->unk0 * 10 + 100;
        color.r = 2;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        DrawBillboard(0x12D, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}
```

`setLs` is the inlined matrix set-up (see StepSmoke's report). Built
alongside StepDust; the same one fix (`color.r` after both size
stores) closed both.
