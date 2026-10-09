# Matching learnings for GCC 2.8.1

Source shapes that reproduce Pepsiman's code, each proved by a byte-exact
match. They are specific to the pinned compiler (GCC 2.8.1, -G8, aspsx 2.56);
`docs/lsd-reference/DECOMPILATION_LEARNINGS.md` was learned on 2.6.3 and is
only a list of things to try. One idiom per entry: what retail shows, what
the source was, and the function that proved it. The story belongs in that
function's match report, not here.

## Declarations

- **How a global is reached says how it was declared** (CLAUDE.md, "Facts
  worth knowing"). Confirmed across round 1: `%gp_rel` and `lui $at` are a
  plain extern; `lui <reg>` + `%lo(sym)(<reg>)` is an array of unknown size,
  including a `.sdata` string reached `lui`/`addiu` (`extern char
  D_800954CC[];`, func_80014BF0) and a halfword table head stored through
  `lui $v0` (`extern u16 D_80095B4C[];`, func_800338D8).
- **A LOAD whose `lui` and destination are the same register is a scalar**,
  `lui $v1` / `lbu $v1, %lo(sym)($v1)`: the assembler expanding a symbolic
  load. cc1's split of an array puts the address and the value in different
  registers (`lui $v0` / `lbu $v1`). Check every retail load before declaring.
  (func_800414EC, `u8 D_80095830`; func_80028650, `s32 D_80095864`)
  Only when the two are ADJACENT: a `lui` in a delay slot with another load
  before its `lw` is cc1's array split (`extern s32 D_8009EF44[];`,
  func_800281B8).
- **An ADDRESS built in one register, `lui $X` / `addiu $X, $X, %lo(sym)`:
  cc1's `la` of an object of known size at most 8**, e.g. `extern s8
  D_80095AA0[8];` or `extern CdlLOC D_80095728;`. Declared `[]`, cc1 splits
  it over two registers. (func_8003E444, func_800175AC)
- **A callee's prototype is a per-unit view.** A caller that stores an `s16`
  result with no re-extension saw it as `s32`; a caller passing arguments
  unextended saw `s32` parameters. Declare that view in the caller's `.c`
  with a `MATCHING:` note. (func_8002CAA4, func_8002CAE4; func_80028260's
  calls to func_8003F834)
- **A `u8` parameter that is only ever stored with `sb` may be `s32`**; the
  narrower type reorders the argument copies. (func_8003F834)
- **A large global's base in a register (`lui`/`addiu`) with fields at
  `off($reg)`: the source used a struct lvalue or a local pointer.** Byte
  arithmetic on the array, `*(s16 *)(D + 0x398)`, folds the offset into
  `%hi/%lo(sym+0x398)`. A unit-local typedef and `#define sX (*(T *)D)`
  works without a shared-header change. (func_800285B0, func_800283E4)
- **A trailing array's offset added to the base early, the scaled index
  last: the array base went into its own local**, `T *recs = p->recs; return
  &recs[i];`. `&p->recs[i]` folds the offset into the final add.
  (func_80036A50)
- **Sony's prototypes come from Sony's headers.** They include cleanly after
  `common.h`; `libgpu.h` needs `libgte.h` first (SVECTOR, MATRIX). A local
  prototype of a Sony function fails declcheck as soon as a header declares
  it. (func_80014D20, func_80014D6C)

## Types

- **`sll r, r, 24` + `bnez` testing a byte: an `s8`.** A `u8` gives
  `andi 0xFF`. (func_800173E8)
- **A byte store of `addiu $x, $zero, -1`: the value is `s8` -1.** A `u8` 0xFF
  gives `addiu ..., 0xFF`. (func_80033854)
- **A byte loaded twice, `lb` then `lbu`: read once through an `(s8)` cast
  for the test, once plainly for the value.** (func_8003F834)
- **An 8-byte local filled with `lwl`/`lwr` from a global: a struct copy**,
  `buf = *(Bytes8 *)D;` with a one-member `u8 b[8]` struct. (func_8003E444)
- **A zero returned from a saved register (`addu $v0, $s2, $zero`): an `s32
  ret = 0;` local in an `s32` function.** An `s16` one adds `sra`.
  (func_800383F8, func_80038468, func_8003950C)

## Loops

- **A loop counting up from 0 and tested with `sltiu`: a `u32` counter.** With
  `s32`, 2.8.1 reverses the loop into a count-down `bgez`. (func_80033854)
- **A pointer bumped before the loop's constants: the source advanced the
  parameter** as its own statement (`recs += 100;`). Indexing `recs[i]` from
  100 strength-reduces to the same loop with the `addiu` after the constants.
  (func_80033878)
- **A loop pointer biased to a field other than the first one written (stores
  at negative offsets): the source walked a separate struct pointer**, `r =
  &tbl[k]; ... r->f = ...; r++;`. Every `tbl[i].f` form reduces to the record
  start. (func_800338D8)
- **A compare loop that loads both bytes before either increment: the
  increments are separate statements after the test**, `if (*a != *b) return
  -1; a++; b++;`. In `*a++ != *b++`, 2.8.1 schedules `b++` before the load of
  `*a`. (func_8003828C)
- **A loop bound kept in a saved register across calls, never reloaded: the
  source copied it to a local** before the loop (`n = pack->count;`).
  Testing the member in the `for` reloads it every pass. (func_8001797C)
- **The index added into the table base's register (`addu $v0, $a0, $v0`):
  the base went into a local and was advanced**, `src = tbl; src += i;`.
  Every indexed spelling puts the sum in the index register. (func_8002C85C)

## Control flow and frames

- **A compare retail keeps that the `&&` chain would fold: a `switch` with an
  empty `case K: break;`.** (func_800173E8)
- **A leaf with `addiu $sp, -8` / `+8` and no stack access: an unused local**,
  `s32 unused[2];`. Without it 2.8.1 emits no frame. Triggers `-Wall`'s unused
  variable warning, which is baselined in `config/typeviews-warnings.txt`.
  (func_80014CF0)
- **A `nop` in a loop's back-branch delay slot where the build fills it from
  the loop target with an instruction writing `$v0`: the function is non-void
  with no `return`** (likely K&R implicit int). The live return register
  blocks the fill. Triggers "control reaches end of non-void function",
  baselined likewise. (func_8002D140; func_8002D16C and func_8002D1CC show the
  same slot) The same holds for a forward branch: the first `beqz` of an
  if/else-if chain keeps its `nop` slot only as `s32` with no return.
  (func_800283A0)
- **A clamp whose one store of the global sits at a join every path reaches,
  the unchanged path included: one assignment of a nested ternary**, `x = x <
  0 ? 0 : x > n - 1 ? n - 1 : x;`. An if/else-if clamp stores per branch.
  (func_8003708C)
- **A constant kept in one register across basic blocks: a local**, `s32 one
  = 1;`. Literals are rematerialised per block. A global reloaded in each arm
  with one shared branch is two compares that `goto` one label. (func_80028448)
- **The value in a branch's delay slot follows the early return**: `if (x >=
  4) return -1; return 0;` gives `beqz` with -1 in the slot; the inverted form
  gives `bnez` with 0. (func_80039618)
- **A non-leaf frame whose locals all sit N bytes high: an unused `s32
  unused[N/4];` declared at that point.** Locals are laid out from `sp+0x10`
  in declaration order. (func_80023194, 8 bytes; func_800230E0, 88)
- **`== 1`, then `slti < 2`, then `== 0` on a byte: nested `if`s on an `s32`
  copy.** A `u8` copy gives `sltu`; a `switch` on {0, 1} and an `&&` chain
  both fold to one `bnez`, so the empty-case lever above does not apply.
  (func_800414EC)

## Scheduling

- **A field loaded after earlier global stores: read it through a byte
  pointer**, `*(s32 *)((u8 *)p + 4)`. A member access (`p->offset`, and also
  `((s32 *)p)[1]`) lets the load rise above the stores. Once the load is in
  place, the order of the global stores decides the rest. Why the second form
  rises is not understood; treat the alias explanation as a hypothesis.
  (func_8002D0C4)
- **Independent stores to a local struct come out in source order.** Retail
  writing `val3`..`val0` of a `CdlATV` means the source assigned them in that
  order. (func_80042C14, func_80042968)
  The same holds for a field-by-field copy into a local struct.
  (func_8002D0F0)
- **A constant folded onto the wrong operand: parenthesise where retail adds
  it**, `a + (rand() % 160 - 80)`. `a - 80 + rand() % 160` moves -80 onto
  the `rand()` result. (func_80022F68)
