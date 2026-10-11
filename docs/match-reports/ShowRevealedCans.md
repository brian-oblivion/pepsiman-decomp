# ShowRevealedCans — MATCHED 24/24 (round 4, alpha)

> Renamed from `PromoteMarks` on 2026-10-11 (tools/rename.py). Address 0x8002d16c.

> Renamed from `func_8002D16C` on 2026-10-11 (tools/rename.py). Address 0x8002d16c.

Unit `src/object.c`. For each of the 200 eight-byte entries of the
current block (`gCanPoints`), if the byte flag `gCanGroupOn[entry.unk6]` is
1 (read signed), sets `gCanState[i]` to 2. First build matched.

```c
s32 ShowRevealedCans(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if ((s8)gCanGroupOn[((Ent8 *)gCanPoints)[i].unk6] == 1) {
            gCanState[i] = 2;
        }
    }
}
```

The non-void-no-return shape predicted by `ClearCanGroups`'s report was right
first time: the back branch's slot stays `nop`. `gCanPoints` is reloaded
`%gp_rel` every pass (it is a `u8 *` global, the cast does not hoist it).
`(s8)` gives the `lb`. `gCanState` is new in the unit as `extern u8 []`
(`lui`/`addiu` hoisted). Adds one "control reaches end of non-void function"
warning to baseline.
