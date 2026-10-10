# func_8001DAD4 — MATCHED (673/673), round 11, runner bravo

Unit `src/code_a0bc.c`. The flat-textured quad member of the subdividing
family (handler table row 6, column 7): func_8001D39C's FT3 body on
func_8001B9A4's quad skeleton. Depth-cued POLY_FT4 with the TMD's four
UV words one OT slot further in unless the transform overflowed; under
depth 250 a split into four quads at the edge midpoints and the centre,
with the midpoint and centre UVs averaged, each sub-quad depth-cued again
and linked at the parent's depth.

New unit-local type `TmdFT4` (header, four `TmdUV` words, a `CVECTOR`,
four vertex indices; 0x20 bytes). code_11dc4 has its own `TmdFT4` with
`u32`/`u16` UV members; this unit needs the bytes. The local prototype
changed from `PACKET *func_8001DAD4();` to the full one.

The source is in `src/code_a0bc.c`.

## How it went

1. Template body with `u32` midpoints and `u8 uc, vc` (retail masks the
   centre values: `andi 0xFF`, and `sll 6; andi 0xFF00` for `vc << 8`):
   one word long, frame 8 bytes too big.
2. `s16` on u01, u13 and u23 (retail has each as the first operand of
   its OR, func_8001E558's lever): one word short, 3097. A scan over the
   24 store orders in the UV switch (`timeout 300`, rc 0) kept the
   natural u0..u3 order.
3. Precompute order with every u before every v: 2852.
4. Type scan, u32/s16 on the five others (32 builds, rc 0): `s16 v01`
   gives exact length, ins/del 15/15 at 1646; the residue was u01, u13,
   u23, v13 and `v01 << 8` rotated through s0..s4. Declaration order
   and the order of those five assignments (120 builds, rc 0) did not
   move it.
5. u8/s16 scan: `u8 u01` gives 654/673, ins/del 0. Then a 324-build scan
   over u8/s16/u32 (rc 0) matched 86 combinations, all with `u8 u01`.
   All ten midpoints `u8` matches, and that is the source: the averages
   of two bytes are known to fit, so no mask appears.
6. With all `u8` the natural precompute order (u01, v01, u02, ...) is
   638/673; the every-u-before-v order stays.

### Proposed learning

- **Averages of TMD bytes in this family are `u8` locals**: `(u32)(a +
  b) >> 1` into a `u8` gives `srl` with no mask (the sum of two bytes
  halved fits), and the narrow pseudo puts the value first in its ORs.
  A `>> 2` centre of four bytes keeps its `andi 0xFF`. (func_8001DAD4,
  func_8001E558; func_8001D39C's `s16` pair is likely the same thing)
