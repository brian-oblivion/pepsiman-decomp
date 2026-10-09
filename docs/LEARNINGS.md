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
  it over two registers. (func_8003E444, func_800175AC) This holds even for
  a loop base the loop writes past: `extern s32 D[2];` (func_8001534C).
- **A global every access reloads right after storing it: `volatile`**, in
  the unit, with a `MATCHING:` note. (D_80095AD0, code_31cec)
- **A global reached `$gp` in one stretch of a unit and `lui` in another
  marks a source-file seam**: maspsx applies one gp list to the whole build,
  so no C spelling closes it. Report it with the addresses; it is a
  segmentation or toolchain decision. (func_80033AB8, func_80014C58)
  Between units this is solved: each unit gets its own gp list,
  `config/gp/<unit>.txt` (func_80014C58, func_80018BD8). Inside one unit it
  still needs the unit split. (func_80033AB8, func_800337E4)
- **A word store through a pointer that keeps a later global reload below it:
  a plain `*(s32 *)p` store, not `p->member`.** A member store is assumed not
  to alias a fixed global, so the reload is dropped. The store-side twin of
  func_8002D0C4's byte-pointer read. (func_800337E4)
- **A callee result tested with no `sll`/`sra`: the callee is `s32` in this
  unit's view**; a definition that returns only constants can simply be
  `s32`. (func_800384DC, func_80039580)
- **A word written with `sw` and summed elsewhere with `lhu`: a union**
  (`{ s32 w; u16 lo; }`). (func_8003828C's table, code_27bc8)
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
- **Four `lw` then four `sw`, both pointers bumped by 16 to an end pointer:
  a block move**, `*(Big *)dst = *(Big *)src` with a one-member `u8 b[N]`
  struct; also for an odd, byte-aligned size. (func_80033930, func_800385E0)
- **A zero returned from a saved register (`addu $v0, $s2, $zero`): an `s32
  ret = 0;` local in an `s32` function.** An `s16` one adds `sra`.
  (func_800383F8, func_80038468, func_8003950C)
- **`andi 0xFFFF` / `sltiu` on the counter with `sll`/`sra 16` on the index:
  a `u16` counter indexed through `(s16)i`.** (func_8002C650)
- **A parameter masked once in the prologue: `mode &= 1;` at the top**, not
  at its use. (func_800153CC)

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
- **An element address kept in a saved register across a call: a local
  pointer**, `seq = &tbl[i];`. (func_800429EC)
- **Two one-pass loops whose registers pair with different variables: one
  counter per loop.** A shared counter costs a saved register. `f(fmt, *p);
  p++;` keeps the format's `addiu` first; `f(fmt, *p++)` does not.
  (func_800149D0)
- **A row offset built as `row*21` then plus the column: a flat index**,
  `tbl[row*21 + col]`; `T t[][21]` scales the row by the row size.
  (func_800229A8)
- **`addu d, shift, base` (index first): an integer sum**, `(T *)(i * 8 +
  (s32)D)`. Pointer arithmetic always puts the pointer first.
  (func_8003A3F4, func_80026C70)
- **A count-down `bgez` loop: write it counting down.** 2.8.1 did not
  reverse a short `s32` count-up here. (code_29f54, round 5)

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
  (func_8003708C) When the value is loaded straight into the stored register
  and the too-low block comes last, it is a local: `v = x; if (x > lo) { if
  (x > hi) v = hi; } else v = lo; x = v;`. (func_800355D8)
- **A call at a loop head, then `beqz` forward to a countdown that branches
  back**: `while ((x = f()) == NULL) { if (--cnt == 0) return -1; }`.
  Do/while lays the blocks out the other way round. (func_80041D88)
- **A redundant `li $v0, 1` while `$v0` already holds 1: `state++`** (the
  value known from the `switch` case), not `state = 1`. (func_80042A88)
- **Two `.sdata` strings sharing a `%hi`, each arm building its own `lui`
  before a call at the join: one call per arm**, `if (c) f(A); else f(B);`.
  A ternary or a `char *` local shares one `%hi`. (func_80037280)
- **Success falling through to its own `jr ra`, -1 set in the first test's
  delay slot and reused: separate guards**, `if (!A) return -1; if (!B)
  return -1; return 0;`. (func_80033680)
- **Case bodies are laid out in source order, not compare order.**
  (func_80023764)
- **`addiu -K; sltiu N`: `(u32)(x - K) < N`.** `x >= K && x < K+N` stays two
  compares. (func_80027BEC)
- **`beq K` / `slti K+1` / `beq K+1`: a switch with an empty `case K-1:
  break;`.** (func_80037700)
- **A NULL return as the last block: `if (found) { ...; return p; } return
  NULL;`.** (func_800197E4)
- **An `abs` that must not move a later branch target: write it inline**,
  `(d < 0 ? -d : d) > 56`, not as a statement. (func_80023B20)
- **A parameter copied out of its register at entry (`move a3, a1`): the
  source duplicated the tail that uses it**; cross-jumping merged the copies.
  (func_80019730)
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
  (func_8002D0C4) Confirmed: member reads through a pointer rise above
  stores to fixed scalar globals; byte-pointer reads stay. (func_80015450)
- **Independent stores to a local struct come out in source order.** Retail
  writing `val3`..`val0` of a `CdlATV` means the source assigned them in that
  order. (func_80042C14, func_80042968)
  The same holds for a field-by-field copy into a local struct.
  (func_8002D0F0)
- **A constant folded onto the wrong operand: parenthesise where retail adds
  it**, `a + (rand() % 160 - 80)`. `a - 80 + rand() % 160` moves -80 onto
  the `rand()` result. (func_80022F68) If cc1 still moves it, give it its own
  statement: `bob = (...) - 200; y = pos->vy + bob;`. (func_80033F48)
- **Operand order sets load order.** `a <= b` and `a - b` load `a` first; a
  `subu` whose subtrahend loads first was `-b + a`. (func_800281B8,
  func_80036478)
- **Chained assignment stores right to left**: `a.x = a.y = v` stores `y`
  first. (func_8002964C, func_80036AB8)
- **A store into a stack slot that held an earlier value: the source reused
  that local.** A fresh local loses the store. (func_8002971C)
- **One literal address in two registers (`lui`/`ori` twice): load through a
  pointer local CSE has lost track of** (set before the loop label, or
  modified), and keep the addend literal. (func_80042058, func_80042150)
- **Small local aggregates sit on 8-byte slots** from `sp+0x10` in
  declaration order. (func_80042B80)
- **A fixed-point chain kept in one register: one local through compound
  statements**, `v = (a - b) << 16; v /= n; v *= k; v /= 0x10000;`.
  (func_80018D04)
- **An induction expression cc1 reassociates: put the induction part in its
  own local**, `base = i*3 + 0x40; v = base - level;`. (func_8003E360)
- **Two addresses computed into one register in turn: one local reused**,
  `p = a->data; G1 = (s32)p; p = b->data; G2 = (s32)p;`. (func_800373C8)
- **A register parameter spilled where retail spills a stack one: name an
  unrelated early subexpression** (`r = col & 0x1F;`) to flip global-alloc.
  (func_800286B0)
