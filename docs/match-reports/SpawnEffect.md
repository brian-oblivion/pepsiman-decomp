# SpawnEffect — MATCHED (40/40), round 3, runner bravo

> Renamed from `func_8003F834` on 2026-10-10 (tools/rename.py). Address 0x8003f834.

Unit `src/stage.c`. Takes a record off the free list of the 128-entry
0x14-byte table `gEffects` and fills it. `gEffectFree` is the free-list
head (negative as `s8` when empty: return -1), `gEffectActive` the
most recently taken record; byte 0x13 of a record is the link that
`InitEffects` (unit `stage_flow`) initialises to `i + 1`. The taken record
is linked to the previous `gEffectActive`, gets `arg0` at 0x12, three halfwords
at 0xC..0x10, 0 at 0x0 and the fifth argument at 0x2. Returns 0.

```c
typedef struct {
    s16 unk0;
    s16 unk2;
    u8 unk4[8];
    s16 unkC;
    s16 unkE;
    s16 unk10;
    u8 unk12;
    u8 next;
} Slot;

#define sSlots ((Slot *)gEffects)

extern NumberedSlot gEffects[]; /* include/common.h */
extern u8 gEffectActive;             /* include/common.h (moved there) */
extern u8 gEffectFree;             /* include/common.h (moved there) */

s32 SpawnEffect(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4) {
    u8 prev;
    Slot *slot;

    if ((s8)gEffectFree < 0) {
        return -1;
    }
    prev = gEffectActive;
    gEffectActive = gEffectFree;
    gEffectFree = sSlots[gEffectActive].next;
    sSlots[gEffectActive].next = prev;
    sSlots[gEffectActive].unk12 = arg0;
    slot = &sSlots[gEffectActive];
    slot->unkC = arg1;
    slot->unkE = arg2;
    slot->unk10 = arg3;
    slot->unk0 = 0;
    slot->unk2 = arg4;
    return 0;
}
```

## Declarations

`common.h`'s `NumberedSlot` knows only byte 0x13, and it is shared, so the
unit views the records through a local `Slot` and a cast macro (the
EndJump pattern); no shared struct was edited.

`gEffectActive` and `gEffectFree` were declared `extern u8` in
`src/stage_flow.c`. Two units now reach them, both as `%gp_rel` byte
scalars, so declcheck wanted them shared: they MOVED to `include/common.h`
(an additive edit there) and the two lines left `src/stage_flow.c`. The
whole image stays OK.

## Levers

- **`arg0` is `s32`, not `u8`.** With `u8 arg0` the build was 34/40: the
  three argument copies got `$t0`/`$t1`/`$t2` in a different order (retail:
  `arg0` in `$t2`, `arg1` in `$t0`, `arg4` in `$t1`). With `s32 arg0` (still
  stored by `sb`) it is byte-exact. Making `arg1..arg4` `s32` as well (with `arg0`
  `u8` or `s32`) changed the function's length (funcdiff read 0/40).
- **`lb` then `lbu` of the same byte**: one read cast to `s8` for the empty
  test, a second, plain read for the value. cc1 does not merge the two
  extensions.
- Each `sSlots[gEffectActive]` re-reads the global after a store through the
  table (2.8.1 has no type-based alias analysis); the last five stores share
  one base, so the source held the record in a pointer.

### Proposed learning

- **Argument copies in temporaries allocated in an order that does not
  follow the argument order: try a wider type for the argument that lands
  out of place.** A `u8` parameter stored only by `sb` was `s32` in the
  source (SpawnEffect).
