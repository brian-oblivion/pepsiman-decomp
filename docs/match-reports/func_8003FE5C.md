# func_8003FE5C — MATCHED (84/84), round 6, runner echo

Unit `src/code_29f54.c`. func_8003FD0C with colour mode 2 in `color.r`
(the same sprite 0x12D, size `frame * 10 + 100`, fade
`0x80 - frame * 16`, 8 frames).

```c
s32 func_8003FE5C(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 * 10 + 100;
        size.vy = p->unk0 * 10 + 100;
        color.r = 2;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        func_8001A3D4(0x12D, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}
```

`setLs` is the inlined matrix set-up (see func_8003FA88's report). Built
alongside func_8003FD0C; the same one fix (`color.r` after both size
stores) closed both.
