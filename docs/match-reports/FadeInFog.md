# FadeInFog — MATCHED 129/129 (round 6, delta)

> Renamed from `func_80040F14` on 2026-10-10 (tools/rename.py). Address 0x80040f14.

Unit `src/code_308ec.c`. Fog fade-in, one step a frame. While the step
`gFogFade` is below 100: on step 0 it adds a full-screen white
`POLY_F4` (from the primitive buffer `GsOUT_PACKET_P`) to the current ordering
table; every step it sets the fog near/far planes and the far colour
scaled by step/32 (the colour picked by bits 8-9 of `D_80095864`, as
SetStageFog does at full strength), then advances the step and parks it
at 100 after 32.

```c
/* libgte.h, libgpu.h, libgs.h; code_a0bc.h for gSubOt.
 * extern u8 *GsOUT_PACKET_P; common.h: gFogFade (u16), gFogColors (u8 *),
 * D_80095864, gActiveBuff. */
void FadeInFog(void) {
    POLY_F4 *p;
    s32 i;

    if (gFogFade != 100) {
        if (gFogFade == 0) {
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0xFF;
            setXY4(p, -160, -120, 160, -120, -160, 120, 160, 120);
            addPrim(gSubOt[gActiveBuff].org, p);
            p++;
            GsOUT_PACKET_P = (u8 *)p;
        }
        i = ((u32)D_80095864 >> 8) & 3;
        SetFogNearFar(gFogFade * 3000 * 2048 / 0x10000, gFogFade * 8000 * 2048 / 0x10000, 250);
        SetFarColor(gFogColors[i * 4] * (gFogFade << 11) / 0x10000,
                    gFogColors[i * 4 + 1] * (gFogFade << 11) / 0x10000,
                    gFogColors[i * 4 + 2] * (gFogFade << 11) / 0x10000);
        gFogFade++;
        if (gFogFade == 32) {
            gFogFade = 100;
        }
    }
}
```

## Levers

- **`p++; GsOUT_PACKET_P = (u8 *)p;` after `addPrim`.** With `GsOUT_PACKET_P =
  (u8 *)(p + 1);` (before or after `addPrim`, or between the two
  `setaddr`s) the build computes `p + 1` into a fresh register before
  `p & 0xFFFFFF` (116/129: registers only). Bumping the pointer variable
  itself gives retail's `and a2, a1, a2` then `addiu a1, a1, 0x18`.
  `GsOUT_PACKET_P += sizeof(POLY_F4)` reloads the global (worse).
- **Libgpu's `setPolyF4` and `addPrim` macros as they are.** `SetPolyF4`
  (the function) breaks it; a hand-written `addPrim` on `u32` words
  reloads the table index (the word store aliases the global) or, with a
  `GsOT *` local, swaps the two mask registers.
- **The colour as chained assignment, `p->r0 = p->g0 = p->b0 = 0xFF;`**:
  retail stores b, g, r. `setRGB0` stores r first.
- **The fog distances fold at tree level.** `gFogFade * 3000 * 2048 /
  0x10000` gives retail's `375 * D << 14` and a signed divide;
  `gFogFade * 8000 * 2048 / 0x10000` folds the whole divide into `D *
  250`. `(gFogFade << 11) * 3000` kept the shift in its own register.
- The colour index as `gFogColors[i * 4 + k]` (SetStageFog's form). A
  `u8 *c = &gFogColors[i * 4]` local swaps the operands of the `addu`.
- `100` lives in `$s1` across the calls with no local: cc1 kept the
  constant from the entry test for the final store on its own.

## Declarations

`gFogFade` (`u16`) and `gFogColors` (`u8 *`) move from
`src/code_13068.c` into `include/common.h` (additive; same types).

### Proposed learning

- **An allocation bump whose `addiu` reuses the pointer's register after
  its last other use: the source bumped the pointer variable**, `p++;
  G = (u8 *)p;`. `G = (u8 *)(p + 1);` computes the sum into a fresh
  register early. (FadeInFog)
