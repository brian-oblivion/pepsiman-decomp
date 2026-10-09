# func_8003FFAC — MATCHED (80/80), round 6, runner echo

Unit `src/code_29f54.c`. A rising sprite step on a `Slot` record: count
the delay (`unk2`) down, else set the local-screen matrix to the position
plus the record's offset, with `unk4 * frame` added to y, draw sprite 0x134
at size 100 in colour (0, 0x80, 0x80, 0x80), advance the frame and return 1
once `frame * unk4` reaches `unk6`.

```c
s32 func_8003FFAC(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE + p->unk4 * p->unk0, z + p->unk10);
        size.vy = size.vx = 100;
        color.r = 0;
        color.g = color.b = color.cd = 0x80;
        func_8001A3D4(0x134, &size, &color, 2, ot);
        return ++p->unk0 * p->unk4 >= p->unk6;
    }
    p->unk2--;
    return 0;
}
```

First build. `setLs` is the inlined matrix set-up (func_8003FA88's report).

The unit's `Slot` typedef changed: `u8 unk4[8]` became `s16 unk4`,
`s16 unk6`, `u8 unk8[4]` (retail reads 0x4 and 0x6 as halfwords here;
nothing else in the unit used the old array). The typedef is local to
`src/code_29f54.c`; the whole image stayed OK. The member comments for
`unk0` (frame), `unk2` (delay) and `unkC`/`unkE`/`unk10` (offset) now
say what these functions show them to be.
