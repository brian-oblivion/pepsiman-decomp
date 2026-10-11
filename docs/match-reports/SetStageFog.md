# SetStageFog — MATCHED (24/24) after a header retype (round 2, alpha)

> Renamed from `func_80028650` on 2026-10-11 (tools/rename.py). Address 0x80028650.

**Resolved by the head after merge (round 2):** `gStageParams` is now `extern s32 gStageParams;`
in `include/common.h` (all 6 retail loads are the scalar `lui $X` / `lw $X` form), and the
body below, reading `gStageParams` instead of `gStageParams[0]`, is live and matches 24/24.
The text below is the stall analysis as filed.

Unit `src/player.c`. Sets the GTE fog range (`SetFogNearFar(3000,
8000, 250)`) and the far colour from a 4-byte RGB entry of the table
`gFogColors` points to, picked by bits 8..9 of `gStageParams`.

## Blocked on a shared declaration, not on the C

Retail loads `gStageParams` as `lui $s0` / `lw $s0,%lo(gStageParams)($s0)`: the
destination register carries the `%hi`, which is the assembler's expansion
of a plain symbolic load (a scalar extern). common.h declares it
`extern s32 gStageParams[];`, so cc1 splits the address itself, picks `$v0`
for the `%hi`, and schedules that `lui` above the `sw $s0`:

```
retail                         build (array)
sw   s0,0x10(sp)               lui  v0,0x8009
lui  s0,0x8009                 sw   s0,0x10(sp)
lw   s0,0x5864(s0)             lw   s0,0x5864(v0)
```

The other 23 words are the same. With a local experiment changing
common.h's line to `extern s32 gStageParams;` (and `(u32)gStageParams`) the
whole image matches: `OK: build matches retail`, 24/24. Every retail access
to `gStageParams` in all three units (UpdatePlayer twice, UpdateStageLights,
CheckForcedThrowBack, FadeInFog, this one) is a `lw` with the `%hi` in the
destination register, so the scalar declaration fits every one. No unit in
`src/` references it yet. The edit is not additive, so it is left to the
head: change common.h's array to `extern s32 gStageParams;` (move it from
the arrays block to the small-objects block), then this body goes live
with `gStageParams` in place of `gStageParams[0]`.

Shapes tried with the array declaration, all 21/24 with the same `$v0`:
`gStageParams[0]` direct, a `u32` temp loaded first, a local `s32 *p =
gStageParams;`.

- The shift is logical (`srl`): the word is cast `(u32)` (an `s32` gives
  `sra`, 23/24).
- `gFogColors` is a plain `extern u8 *` (`lui $v0` / `lw $v0`, loaded into
  its own destination).
- Indexing `gFogColors[i * 4 + k]` in the call arguments gives retail's
  `addu $s0,$s0,$v0` order; a `u8 *c = &gFogColors[i * 4];` temp puts the
  sum in `$v0` (`addu $v0,$v0,$s0`).

```c
#if 0
#include "common.h"
#include "libgte.h"

extern s32 gStageParams; /* scalar, not common.h's array */
extern u8 *gFogColors;

void SetStageFog(void) {
    s32 i;

    i = ((u32)gStageParams >> 8) & 3;
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(gFogColors[i * 4], gFogColors[i * 4 + 1], gFogColors[i * 4 + 2]);
}
#endif
```

The unit keeps the same body under `#ifdef NON_MATCHING` (with
`gStageParams[0]`).

### Proposed learning

A load whose `%hi` sits in the destination register (`lui $s0` / `lw
$s0,%lo(x)($s0)`) is a scalar extern even when only loads are seen; an
array declaration makes cc1 split the address into a scratch register and
schedule that `lui` separately.
