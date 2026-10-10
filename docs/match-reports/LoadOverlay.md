# LoadOverlay — MATCHED (197/197), round 10, runner alpha

> Renamed from `func_80022554` on 2026-10-10 (tools/rename.py). Address 0x80022554.

Unit `src/code_11dc4.c`. A selector code_31958's LoadStageOverlay calls with
an id from 3 to 17: a 23-way switch on the id's low byte (no case 1) that
stores one of 22 data addresses in gCdLoadLoc[0] and copies one of 22 words
into gCdLoadSectors[0], then sets gCdLoadDest[0] to 0x800F0000 and gCdLoadCount
and gCdLoadState to 1. code_31cec's LoadVabBank writes the same five
globals in the same order with SsVab calls nearby, so these are likely
sound-bank parameters. splat's instruction count (220) included the
jump table's 23 words.

```c
/* 44 one-word-or-larger data objects, each its own splat symbol, all
 * reached with lui/addiu (address) or lui/lw in different registers
 * (load): declared as arrays of unknown size. */
extern s32 D_800761AC[];
/* ... one `extern s32 D_800xxxxx[];` per symbol below ... */

void LoadOverlay(s32 id) {
    /* MATCHING: s32 in code_31958's view; the switch narrows it. */
    switch ((u8)id) {
        case 0:
            gCdLoadLoc[0] = (s32)D_800761AC;
            gCdLoadSectors[0] = D_800761FC[0];
            break;
        case 2:
            gCdLoadLoc[0] = (s32)D_800761B0;
            gCdLoadSectors[0] = D_80076200[0];
            break;
        /* cases 3 to 17: address D_8007624C + 0x48 * (k - 3), word
         * D_80076270 + 0x48 * (k - 3); cases 18 to 22: address D_800761D4
         * + 8 * (k - 18), word D_80076224 + 8 * (k - 18) (the full list
         * is in src/code_11dc4.c) */
    }
    gCdLoadDest[0] = 0x800F0000;
    gCdLoadCount = 1;
    gCdLoadState = 1;
}
```

## History

Fresh in round 10. Matched on the first build. The address loads are
`lui $v0` / `addiu $v0, $v0` (one register) and the word loads `lui $v0` /
`lw $v1` (split), and an unknown-size `extern s32 D[];` gives both here.
The `andi 0xFF` before the bound check is `switch ((u8)id)` on an `s32`
parameter, which keeps code_31958's existing `void func_80022554(s32 id);`
prototype correct. Every case jumps to one shared store of D_8009F248 with
its `lui` in the delay slot: cross-jumping merged the cases' tails, no
source spelling needed. Case 1 is absent (its table entry is the exit).
