# EndJump — MATCHED 6/6 (round 2, alpha)

> Renamed from `func_800285B0` on 2026-10-11 (tools/rename.py). Address 0x800285b0.

Unit `src/code_13068.c`. Clears one halfword of the game-state record
`gPlayer` and sets another to 2.

```c
void EndJump(void) {
    sGame.unk398 = 0;
    sGame.unk390 = 2;
}
```

`sGame` is the unit's `#define sGame (*(GameState *)gPlayer)`, a
unit-local struct view of the global (common.h declares it `u8[]`).

## The lever: a struct lvalue, not byte-array arithmetic

Retail loads the record's base with `lui`/`addiu` and stores at `0x398($v0)`
and `0x390($v0)`. Written as `*(s16 *)(gPlayer + 0x398) = 0;`, cc1 folds
each offset into the relocation: `lui %hi(gPlayer+920)`, `sh
%lo(gPlayer+920)`, then `addiu` and `sh -8($v2)` (0/6, the image drifts).
Same bytes as retail from either a struct lvalue (`((T *)gPlayer)->f`,
a struct-typed extern, the `sGame` macro) or a local `u8 *p =
gPlayer;` with byte offsets from `p`. A `s16 *` cast indexed by
`[0x398/2]` folds like the byte form.

### Proposed learning

A large global whose base retail materialises with `lui`/`addiu` and then
reaches as `off($reg)`: write it as a struct lvalue (or through a local
pointer). Byte-array arithmetic on the symbol, `*(s16 *)(D + off)`, folds
the offset into `%hi/%lo(D+off)`. (EndJump)
