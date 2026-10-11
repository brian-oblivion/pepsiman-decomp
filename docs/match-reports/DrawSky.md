# DrawSky — MATCHED 393/393 (round 12, bravo)

> Renamed from `func_800184BC` on 2026-10-10 (tools/rename.py). Address 0x800184bc.

Unit `src/model.c`. Draws the sky: two Gouraud quads (`POLY_G4`) from
the primitive buffer `GsOUT_PACKET_P`, both at the back of the ordering table
(`gOt[gActiveBuff]`, entry 0xFFF). The horizon height is `8 - y`,
where `y = rsin(ratan2(dy, horizontal distance)) * 240 >> 12` from the two
points in `gView` (the camera's pitch). The upper quad runs from the
top of the screen (-160) to the horizon, the lower from the horizon to 119.
Colours come from the table `gSkyColors`, entry `(arg >> 10) & 3` (four
`CVECTOR`s: top and bottom of each quad). While the fog fades in
(`gFogFade` below 100) and on two frames out of three, each channel is
scaled by `gFogFade / 32`; otherwise the colours are copied as words.

```c
/** @brief A sky band's four corner colours. */
typedef struct {
    CVECTOR c[4]; /**< top and bottom of the upper band, then of the lower */
} SkyColors;

extern SkyColors *gSkyColors;

void DrawSky(u32 arg) {
    /* MATCHING: volatile keeps every store in source order and in place. */
    volatile POLY_G4 *p;
    u32 *tag;
    s32 set;
    s32 dx;
    s32 dz;
    s32 y;

    dx = gView[3] - gView[0];
    dz = gView[5] - gView[2];
    /* MATCHING: two steps keep the shift and the mask in one register. */
    set = arg >> 10;
    set &= 3;
    y = gView[4] - gView[1];
    p = (volatile POLY_G4 *)GsOUT_PACKET_P;
    y = rsin(ratan2(y, SquareRoot0(dx * dx + dz * dz))) * 240 >> 12;
    if (gFogFade != 100 && gStage % 3 != 2) {
        p->r0 = p->r1 = gSkyColors[set].c[0].r * (gFogFade << 11) / 0x10000;
        p->g0 = p->g1 = gSkyColors[set].c[0].g * (gFogFade << 11) / 0x10000;
        p->b0 = p->b1 = gSkyColors[set].c[0].b * (gFogFade << 11) / 0x10000;
        p->r2 = p->r3 = gSkyColors[set].c[1].r * (gFogFade << 11) / 0x10000;
        p->g2 = p->g3 = gSkyColors[set].c[1].g * (gFogFade << 11) / 0x10000;
        p->b2 = p->b3 = gSkyColors[set].c[1].b * (gFogFade << 11) / 0x10000;
    } else {
        *(u32 *)&p->r0 = *(u32 *)&p->r1 = *(u32 *)&gSkyColors[set].c[0];
        *(u32 *)&p->r2 = *(u32 *)&p->r3 = *(u32 *)&gSkyColors[set].c[1];
    }
    p->x0 = -160;
    p->y0 = -160;
    p->x1 = 159;
    p->y1 = -160;
    p->x2 = -160;
    p->y2 = 8 - y;
    p->x3 = 159;
    p->y3 = 8 - y;
    setcode(p, 0x38);
    tag = (u32 *)gOt[gActiveBuff].org + 0xFFF;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x08000000;
    *tag = (u32)p & 0xFFFFFF;
    p++;
    if (gFogFade != 100 && gStage % 3 != 2) {
        p->r0 = p->r1 = gSkyColors[set].c[2].r * (gFogFade << 11) / 0x10000;
        p->g0 = p->g1 = gSkyColors[set].c[2].g * (gFogFade << 11) / 0x10000;
        p->b0 = p->b1 = gSkyColors[set].c[2].b * (gFogFade << 11) / 0x10000;
        p->r2 = p->r3 = gSkyColors[set].c[3].r * (gFogFade << 11) / 0x10000;
        p->g2 = p->g3 = gSkyColors[set].c[3].g * (gFogFade << 11) / 0x10000;
        p->b2 = p->b3 = gSkyColors[set].c[3].b * (gFogFade << 11) / 0x10000;
    } else {
        *(u32 *)&p->r0 = *(u32 *)&p->r1 = *(u32 *)&gSkyColors[set].c[2];
        *(u32 *)&p->r2 = *(u32 *)&p->r3 = *(u32 *)&gSkyColors[set].c[3];
    }
    p->x0 = -160;
    p->y0 = 8 - y;
    p->x1 = 159;
    p->y1 = 8 - y;
    p->x2 = -160;
    p->y2 = 119;
    p->x3 = 159;
    p->y3 = 119;
    setcode(p, 0x38);
    tag = (u32 *)gOt[gActiveBuff].org + 0xFFF;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x08000000;
    *tag = (u32)p & 0xFFFFFF;
    GsOUT_PACKET_P = (u8 *)(p + 1);
}```

## Levers

- **`volatile POLY_G4 *p`.** Retail writes every vertex in source order
  (the scheduler would interleave them) and each chained colour store
  reads its first target back (`sb r1; lbu r1; sb r0`). It also keeps the
  last colour store out of the `j`'s delay slot, so reorg fills that slot
  with the join's `li -160` instead. With the volatile only on the chained
  target (`((volatile POLY_G4 *)p)->r1`), 157/393 and 4 words short.
- **The tag word written whole**, `*(u32 *)p = (*tag & 0xFFFFFF) |
  0x08000000; *tag = (u32)p & 0xFFFFFF;` after `setcode(p, 0x38)`, as
  draw and draw_tmd do. libgpu's `setPolyG4` / `addPrim` go through
  the `P_TAG` bitfields and load the old tag.
- **The colour copy as `u32` words**, `*(u32 *)&p->r0 = *(u32 *)&p->r1 =
  *(u32 *)&tbl.c[0];`. A `CVECTOR` copy is `lwl`/`lwr` (alignment 1).
- **`set = arg >> 10; set &= 3;`**: retail shifts and masks in one saved
  register (`srl s1, a0; andi s1, s1`). `(arg >> 10) & 3` computes into
  `$v0` and moves. The two statements come after `dx`/`dz` and before `dy`.
- **`GsOUT_PACKET_P = (u8 *)(p + 1);`** for the last quad, not `p++` first:
  retail adds into `$v0` before the epilogue.

## Externs and types

- `DrawSky` takes a `u32` here (`srl` on `$a0`); editor, its
  caller, declares it `void (void)` and leaves whatever `$a0` holds. That
  declaration is unchanged.
- `gSkyColors` is a pointer (`lw`) to 16-byte records; declared in this
  unit only.
- `gView` is common.h's `s32[]` (cc1's split `lui`/`addiu` base).

### Proposed learning

- **A primitive whose every store comes out in source order, colour chains
  reading their first target back: the packet pointer is a `volatile T *`
  local.** (DrawSky)
- **`srl sN, a0; andi sN, sN` (one register): `v = a >> k; v &= m;`.**
  (DrawSky)
