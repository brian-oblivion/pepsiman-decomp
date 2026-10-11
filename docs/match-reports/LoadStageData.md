# LoadStageData — MATCHED (round 13, alpha)

> Renamed from `func_800399A8` on 2026-10-10 (tools/rename.py). Address 0x800399a8.

Unit `src/code_29f54.c`. Loads a stage file from its header (`hdr`, twelve
byte offsets): maps and links the two model sets (as LinkStageTmd and
LinkPropsTmd do separately), clears `gSection` and `gFloorIndex`, then
turns each header offset into a pointer global (two of them, the run tables,
also store their leading counts and the byte list that follows them).

Everything up to the offset block matches (the two map/link loops, 120
words). The residue is the order and register use of the offset block:

- Retail reads `hdr[5]` once for both the run table (`+4`) and its count,
  copies the value (`move v1, v0`) and adds `hdr` to each copy; it reloads
  `hdr[5]` for the byte list after the stores. Same for `hdr[6]`.
- In a standalone cc1 harness that shape comes only from two locals set
  from one load and each advanced, `q = o = hdr[5]; o += (s32)hdr; q +=
  (s32)hdr;` (`o` for the count, `q` for the table). In the unit, that
  spelling (128/165) makes the scheduler hoist the `hdr[5]` load to the top
  of the block, where retail keeps it below the `gBoundaryEdges` store.
- Every plain spelling either reloads `hdr[5]` three times (121) or shares
  one sum (two words short).

## Round 9's best body (130/165, length exact)

```c
#if 0
/* MATCHING: this unit's views of the stage pointers; other units declare
 * their own (stage_flow, model, object, player). */
extern s32 gFloorIndex;
extern s32 gFloorData;
extern s32 gSectionEdges;
extern s32 gStageStart;
extern s32 gLightColors;
extern s32 sSectionCamHeights;
extern s32 gSkyColors;
extern s32 gStageRunCount;
extern s32 gPropRunCount;

void LoadStageData(s32 *hdr) {
    u32 i;
    GsDOBJ2 *obj;
    unsigned long *tmd;

    tmd = (unsigned long *)(hdr[0] + (s32)hdr);
    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
    tmd = (unsigned long *)(hdr[1] + (s32)hdr);
    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
    gSection = 0;
    gFloorIndex = 0;
    gFloorData = hdr[2] + (s32)hdr;
    gSectionEdges = hdr[3] + (s32)hdr + 4;
    gBoundaryEdges = (Edge *)(hdr[4] + (s32)hdr + 4);
    gStageRuns = (Run *)(hdr[5] + 4 + (s32)hdr);
    gStageRunCount = *(s32 *)(hdr[5] + (s32)hdr);
    gStageRunObjs = (s8 *)(hdr[5] + (s32)hdr + gStageRunCount * 4 + 4);
    gPropRuns = (Run *)(hdr[6] + (s32)hdr + 4);
    gPropRunCount = *(s32 *)(hdr[6] + (s32)hdr);
    gStageStart = hdr[7] + (s32)hdr;
    sSectionCamHeights = hdr[8] + (s32)hdr;
    gFogColors = (u8 *)(hdr[9] + (s32)hdr);
    gLightColors = hdr[10] + (s32)hdr;
    gSkyColors = hdr[11] + (s32)hdr;
    gPropRunObjs = (s8 *)(hdr[6] + (s32)hdr + gPropRunCount * 4 + 4);
}

#endif
```

Needs the unit's `Run` and `Edge` typedefs and their `extern`s
(`gStageRuns`, `gPropRuns`, `gStageRunObjs`, `gPropRunObjs`, `gBoundaryEdges`),
all already above it in `src/stage.c`; place it where the
`INCLUDE_ASM` stands.

## Levers tried (about 25 builds, one search)

| spelling | words |
| --- | --- |
| stores in retail's store order | 97 |
| stores in header-index order | 121 |
| `hdr` as a `StageHdr *` with `off[12]` (in-struct loads) | 121 |
| count read first into a local | two words short |
| one local for `hdr[5]`, two sums | two words short |
| `hdr[5] + 4 + (s32)hdr` for the table | **130** |
| `q = o = hdr[5]; o += h; q += h;` (copy + two adds as retail) | 128 |

Permuter on the 130 body: gate 3 AGREE (scaffold and in-tree objdump
identical); `timeout 900`, rc 124, 88,723 iterations, best 1110 from a base
of 2005, only through a `volatile` dummy local. Not closed.

## What would move it

Find where the `hdr[5]` load has to sit: retail has `lw [2]`, `lw [3]`
above the zero stores, then `lw [4]` after the `gFloorData` store and
`lw [5]` after the `gBoundaryEdges` store. A spelling that keeps the `q = o`
copy shape and stops the `hdr[5]` load from rising is the next thing to try.

### Proposed learning

- **One load, `move`, then two separate `addu` of the same base: two
  locals set from that load and each advanced** (`q = o = p[5]; o += b; q
  += b;`). A single local or a repeated expression shares the sum.
  (LoadStageData, harness-proven; not yet closed in-unit)

## Round 13 (alpha)

The round-9 body no longer compiled as given: the unit has since declared
`gSectionEdges` as `Edge *` and `sSectionCamHeights` as `s8 *` further down. With
those two externs retyped and their stores cast, the splice measured
130/165, 11 insertions / 11 deletions, as recorded.

Matched 165/165 by reading each **count before its table**:

```c
    gStageRunCount = *(s32 *)(hdr[5] + (s32)hdr);
    gStageRuns = (Run *)(hdr[5] + (s32)hdr + 4);
    gStageRunObjs = (s8 *)(hdr[5] + (s32)hdr + gStageRunCount * 4 + 4);
    gPropRunCount = *(s32 *)(hdr[6] + (s32)hdr);
    gPropRuns = (Run *)(hdr[6] + (s32)hdr + 4);
```

with every other store in round 9's order and every expression in its
plainest form. No locals: the `move` and the two `addu` of one `hdr[5]` load
come from cse giving the count's address and the table's sum separate
registers, and the reload of `hdr[5]` for the byte list from the store of
the count to an `s32` global (it may alias `hdr`).

Tried first in a cc1 harness (tail block only, scored against retail):
the same expressions with the table first (round 9's order, 3 of 57 tail
lines in place), a `s32 *p` record pointer (`p + 1` for the table; 4 words
short), `(Run *)((s32 *)(...) + 1)`, `(s32)hdr + hdr[5]` and `(u8 *)hdr +
hdr[5]` (all as the table-first form).

The definition takes `unsigned long *hdr`, main.c's prototype (identical
code). Declcheck then needed `MATCHING:` notes on the other units' views of
`gLightColors` (player), `gStageStart` (stage_flow), `gSkyColors`
(model) and on main.c's prototype; comments only.

### Proposed learning

- **One header word loaded once, copied, and both copies added to the same
  base: the source read the word through a pointer before computing the
  pointer beside it**, `n = *(s32 *)(o + b); t = o + b + 4;`. Table first
  shares one sum. The `q = o = x` two-local spelling of round 9 gave the
  same shape but let the scheduler hoist the load. (LoadStageData)
