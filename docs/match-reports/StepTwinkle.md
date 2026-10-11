# StepTwinkle — MATCHED (106/106), round 8, runner charlie

> Renamed from `StepEffect1` on 2026-10-11 (tools/rename.py). Address 0x8003e550.

> Renamed from `func_8003E550` on 2026-10-10 (tools/rename.py). Address 0x8003e550.

Unit `src/stage.c`. One step of a slot animation, a sibling of
StepBurst..StepFloat: after the slot's start delay it sets the GS
local-screen matrix to the slot's position, draws sprite 0x11F (through
draw's DrawStandingBillboard) with a size that follows a cosine and a sine of the
frame, and reports the end at frame 15.

```c
/* DrawStandingBillboard: include/draw.h */
/* setLs: the unit's static __inline__ helper (see src/stage.c) */

s32 StepTwinkle(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = (u32)(rcos((p->unk0 * 3 << 13) / 360) * 25) >> 9;
        size.vy = (u32)(rsin((p->unk0 << 15) / 360) * 25) >> 8;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        DrawStandingBillboard(0x11F, &size, &color, 2, ot);
        return ++p->unk0 == 15;
    }
    p->unk2--;
    return 0;
}
```

## Levers

- **The inlined `setLs` helper** carries over unchanged from StepBurst;
  its definition moved up the file to sit before this function (an unused
  `static __inline__` emits nothing, so the move changes no bytes). Spelled
  out in the body, the build was far off (different register allocation and
  prologue order from the first instruction).
- **`srl` after the `*25` (`sll`/`addu` chain): an unsigned shift**,
  `(u32)(rcos(...) * 25) >> 9`. Signed gives `sra` (104/106).
- DrawStandingBillboard is defined in draw.c and had no header prototype; one
  was ADDED to `include/draw.h` (declcheck wants a defined function's
  prototype in its unit's header). Same signature as the definition.

### Proposed learning

- **`srl` on a value stored into an `s16`: a `(u32)` cast before the shift.**
  cc1 keeps `sra` for a signed product even when only 16 bits survive.
  (StepTwinkle)
