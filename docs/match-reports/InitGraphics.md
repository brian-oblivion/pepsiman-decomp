# InitGraphics — MATCHED (88/88), round 7, runner bravo

> Renamed from `func_80013EE4` on 2026-10-10 (tools/rename.py). Address 0x80013ee4.

Graphics init: `GsInitGraph` (Sony's, still carved as asm,
`GsInitGraph`), one display buffer pair, two pairs of 12-bit ordering
tables (`gSubOt` over tags `gSubOtTags`, `gOt` over tags
`gOtTags`), then the active buffer's work base and both tables cleared,
and `gCurOt` pointed at the active `gOt` table.

```c
/* Sony's (libgs, carved as asm): GsInitGraph by its arguments. */
void GsInitGraph(s32 w, s32 h, s32 intmode, s32 dith, s32 vram);

extern GsOT_TAG gOtTags[2][0x1000];
extern GsOT_TAG gSubOtTags[2][0x1000];
extern PACKET gGpuWork[2][67200];
extern GsOT *gCurOt;

void InitGraphics(void) {
    s32 i;

    GsInitGraph(320, 240, 4, 1, 0);
    GsDefDispBuff(0, 0, 0, 240);
    for (i = 0; i < 2; i++) {
        gSubOt[i].length = 12;
        gSubOt[i].org = gSubOtTags[i];
        gSubOt[i].point = 0;
        gSubOt[i].offset = 0;
        gOt[i].length = 12;
        gOt[i].org = gOtTags[i];
        /* MATCHING: retail stores this field twice. */
        gSubOt[i].point = 300;
        gOt[i].offset = 0;
    }
    gActiveBuff = GsGetActiveBuff();
    GsSetWorkBase(gGpuWork[gActiveBuff]);
    GsClearOt(0, 300, &gOt[gActiveBuff]);
    GsClearOt(0, 0, &gSubOt[gActiveBuff]);
    gCurOt = &gOt[gActiveBuff];
}
```

## Attempts (3)

1. Stores in retail's printed order (len, org, point, offset; org2, len2,
   point=300, offset2): 79/88. The loop's pointer inits came out with
   `gOt` before `gOtTags`, and the tag-pointer bump moved.
2. Each table's stores as length, org, offset, point: 82/88. The pointer
   inits and the bump now right; the two zero stores swapped and the
   `point = 300` store sank below `offset2`.
3. length, org, point, offset for the first table; length, org, then
   `point = 300`, then `offset` for the second: 88/88. The second table's
   `length` before its `org` is what orders the loop's pointer inits; retail
   prints `org` first only because the scheduler moved it.

The doubled `point` store is in retail (`sw $zero, 0xC($v1)` then `sw $t1,
0xC($v1)`); cc1 keeps both because the second table's stores could alias.

## Declarations

- `gOt` moved from `src/stage.c` to `include/draw.h`, next
  to its twin `gSubOt` (additive to the header).
- `GsInitGraph` sits in `asm/psyq_46F74.s` (Sony libgs, not yet named);
  its arguments are GsInitGraph's. A local prototype in `src/main.c`.

### Proposed learning

- **Stores in a strength-reduced loop: the source order of the stores sets
  the order of the pointer inits before the loop**, even when the scheduler
  later reorders the stores themselves. Read the `la` order as well as the
  store order. (InitGraphics)
