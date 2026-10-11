# ClearObjectHits — MATCHED 21/21 (round 4, alpha)

> Renamed from `func_8002C650` on 2026-10-11 (tools/rename.py). Address 0x8002c650.

Unit `src/object.c`. For all 200 `Rec48` records of `D_800A9008`, clears
`unk40` and `unk24` and keeps only bit 0 of `unk41`.

```c
s32 ClearObjectHits(void) {
    u16 i;
    Rec48 *r;

    for (i = 0; i < 200; i++) {
        r = &sRecs48[(s16)i];
        r->unk40 = 0;
        r->unk24 = 0;
        r->unk41 &= 1;
    }
}
```

`D_800A9008` is `s32[]` in common.h; the unit views it through
`#define sRecs48 ((Rec48 *)D_800A9008)` (no shared-header change). Retail
recomputes `&tbl[i]` every pass with `sll 16` / `sra 16` (the index as
`s16`) and `* 0x48` by shifts, and tests the counter with `andi 0xFFFF` /
`sltiu`: a `u16` counter indexed through an `(s16)` cast. Stores in source
order; the load of `unk41` rises above them on its own.

## Lever

As `void`: 19/21, reorg fills the back branch's slot with the loop top's
`sll $v0, $a0, 16` and retargets the branch. `s32` with no `return` (the
`ClearFlags128` lever) keeps the `nop`: 21/21. One more "control reaches end
of non-void function" warning to baseline.
