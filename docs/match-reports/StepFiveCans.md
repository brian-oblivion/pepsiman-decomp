# StepFiveCans — MATCHED 207/207 (round 9, bravo)

> Renamed from `StepEffect8` on 2026-10-11 (tools/rename.py). Address 0x8003ec04.

> Renamed from `func_8003EC04` on 2026-10-10 (tools/rename.py). Address 0x8003ec04.

Unit `src/stage.c`. A slot step, the five-sprite sibling of
StepCanCollect right above it: after the wait, it projects the slot's point
once (first frame), then each frame draws sprite 0xFA five times, spaced 72
degrees apart on a circle whose radius shrinks as the group moves toward
(-136, -88), each rotated to face along the circle and fading out; after 8
frames it adds 5 to `gCans` and calls CheckCanBonus. First build
matched.

```c
s32 StepFiveCans(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];
    s32 i;

    if (p->unk2 == 0) {
        if (p->unk0 == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->unkC, (long *)q, NULL, NULL);
            p->unkC = q[0];
            p->unkE = q[1];
        }
        for (i = 0; i < 5; i++) {
            q[0] = p->unkC + ((-136 - p->unkC) * p->unk0 >> 3);
            q[1] = p->unkE + ((-88 - p->unkE) * p->unk0 >> 3);
            q[0] = q[2] = q[0] + rsin(((p->unk0 * 20 + i * 72) << 12) / 360) * (64 - p->unk0 * 8) / 4096;
            q[1] = q[3] = q[1] + rcos(((p->unk0 * 20 + i * 72) << 12) / 360) * (64 - p->unk0 * 8) / 4096;
            q[4] = q[5] = 0x1000 - (p->unk0 << 8);
            q[6] = -(((p->unk0 * 20 + i * 72 + 90) << 12) / 360);
            q[7] = 1;
            color.r = 0;
            color.g = color.b = color.cd = ~(p->unk0 << 4);
            DrawSpriteTransformed(0xFA, q, &color, 0, &gSubOt[gActiveBuff]);
        }
        if (++p->unk0 == 8) {
            gCans += 5;
            CheckCanBonus();
            return 1;
        }
        return 0;
    }
    p->unk2--;
    return 0;
}
```

## Notes

- Written straight from StepCanCollect's shape (the inlined `setLs`, the
  same frame layout with `s32 unused[2]`); the loop's `i * 72` became the
  strength-reduced `$s1`, and `/ 360` the multiply-high by 0xB60B60B7.
- `-Wall`'s unused-variable warning is baselined as a third stage
  line in `config/typeviews-warnings.txt`.
