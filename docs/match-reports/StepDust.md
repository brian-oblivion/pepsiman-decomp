# StepDust — MATCHED (84/84), round 6, runner echo

> Renamed from `StepEffect4` on 2026-10-11 (tools/rename.py). Address 0x8003fd0c.

> Renamed from `func_8003FD0C` on 2026-10-10 (tools/rename.py). Address 0x8003fd0c.

Unit `src/stage.c`. An 8-frame expanding, fading sprite step on a
`Slot` record, the same template as StepBurst: count the delay
(`unk2`) down, else set the local-screen matrix to (`x`, `y`, `z`) plus the
record's offset, draw sprite 0x12D at size `frame * 10 + 100` in colour
(1, c, c, c) with `c = 0x80 - frame * 16`, advance the frame and return 1 at
frame 8.

```c
s32 StepDust(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 * 10 + 100;
        size.vy = p->unk0 * 10 + 100;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        DrawBillboard(0x12D, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}
```

`setLs` is the unit's `static __inline__` copy of SetLsTranslation's body
(the lever from StepSmoke's report: the inlined locals' addresses are
rebuilt at every call, as retail does). Retail reloads the frame (`lh`)
for each size member, so they are two statements, not a chained
assignment. The one miss on the first build: `color.r = 1` written between
the two size stores puts its `li`/`sb` first; written after both, as in
StepSmoke, it matches. Two builds.
