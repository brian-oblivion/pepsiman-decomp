# CycleToolStep — MATCHED (41/41) after moving hit_tool's rodata start

> Renamed from `func_80036FE8` on 2026-10-11 (tools/rename.py). Address 0x80036fe8.

**Resolved by the head after round 7:** hit_tool's `.rodata` yaml line now
starts at 0x1CDC, taking the five strings splat had left at the end of
editor (only PrintToolStatus references them). The unit's jump tables are
then 8-aligned as cc1's `.align 3` expects, and the body is live. Original
title: CycleToolStep — STALL (blocked by segmentation): length exact, 40/41, first diff 0x80037024 (round 7, echo)

Unit `src/hit_tool.c`. When flag bit 8 (0x100) of `gPadTrig` is set,
steps `gToolStep` through the cycle 10 -> 20 -> 30 -> 40 -> 50 -> 10; any
other value is left alone. The switch compiles to retail's 41-entry jump
table `jtbl_800118F4`.

The body is kept under `#ifdef NON_MATCHING` in the unit. **The C is right**:
every text word matches except the `%lo` of the jump table, and with the
unit's rodata start corrected (below) the function builds 41/41 and the whole
image verifies OK.

```c
#if 0
extern s32 gPadTrig; /* common.h */
extern s16 gToolStep;

s32 CycleToolStep(void) {
    if (gPadTrig & 0x100) {
        switch (gToolStep) {
            case 10:
                gToolStep = 20;
                break;
            case 20:
                gToolStep = 30;
                break;
            case 30:
                gToolStep = 40;
                break;
            case 40:
                gToolStep = 50;
                break;
            case 50:
                gToolStep = 10;
                break;
        }
    }
}
#endif
```

## Attempts

1. `void`: text 38/41 once read past the 4-byte shift. The bound check's
   delay slot held the table's `lui $v0` instead of retail's `sll $v1, $v1, 2`
   (the SetTmdSortMode shape).
2. `s32` with no `return`: every text word right except
   `addiu $v0, $v0, %lo(jtbl)`, which points at 0x800118F8 instead of
   0x800118F4, and the whole image after the unit's rodata is shifted by 4.

## Why: the unit's rodata starts in the wrong place

cc1 emits `.rdata` / `.align 3` before each jump table. `pepsiman.ld` links
each unit's `.rodata` with `SUBALIGN(2)`, so a unit may start 4 mod 8, and
the `.align 3` pads relative to the unit's start. Retail's hit_tool jump
tables (0x80011804, 0x80011824, 0x8001183C, 0x8001188C, 0x800118AC,
0x800118F4) are all 4 mod 8 from splat's unit start 0x80011528, so cc1 pads
each one by 4.

The real start is 0x800114DC (file offset 0x1CDC): the five strings
`sHitSyncFmt`, `sHitTrapFmt`, `sHitGridFmt`, `sHitPosFmt` and `sHitItemText` that
splat placed at the end of editor (right after its last jump table,
`jtbl_80011494`, which ends at 0x800114DC) are referenced only by
hit_tool's `PrintToolStatus`. From 0x800114DC every hit_tool jump table is
0 mod 8. Every other unit checked has its jump tables 8-aligned from its own
start (stage, stage_flow, stage_overlay, sound).

Experiment: those five `INCLUDE_RODATA` lines (folder unchanged) moved from
the end of `src/editor.c` to the top of `src/hit_tool.c`'s rodata, with
the `s32` body above live: `OK: build matches retail`, CycleToolStep 41/41.
Reverted, since segmentation is the head's call: the clean fix is the yaml's
hit_tool `.rodata` boundary 0x1D28 -> 0x1CDC (or the source move above).
It unblocks every switch-table function in this unit.

### Proposed learning

- **A C jump table must sit 8-aligned from its unit's rodata start.** cc1
  puts `.align 3` before the table and the link script lets a unit begin 4
  mod 8. A table that lands 4 bytes late with otherwise matching code means
  the unit's rodata boundary is wrong; look for strings at the end of the
  previous unit that only this unit references. (CycleToolStep)
