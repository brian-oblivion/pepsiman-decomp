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
  needs the unit split: code_1a098 split at func_8002D424 into code_1dc24,
  which matched both. (func_80033AB8, func_800337E4)
- **A word store through a pointer that keeps a later global reload below it:
  a plain `*(s32 *)p` store, not `p->member`.** A member store is assumed not
  to alias a fixed global, so the reload is dropped. The store-side twin of
  func_8002D0C4's byte-pointer read. (func_800337E4)
- **A callee result tested with no `sll`/`sra`: the callee is `s32` in this
  unit's view**; a definition that returns only constants can simply be
  `s32`. (func_800384DC, func_80039580)
- **A word written with `sw` and summed elsewhere with `lhu`: a union**
  (`{ s32 w; u16 lo; }`). (func_8003828C's table, code_27bc8)
- **A result kept in `$a3` that skips `$a1`/`$a2`, which the function never
  touches: extra arguments passed straight through to callees.**
  (func_80040E04)
- **`lbu` of a stack parameter with no `andi 0xFF`: an `s32` parameter read
  as `(u8)p`.** A `u8` parameter adds the `andi`. (func_8001B2F4)
- **`li -1` into a `u_char` member: `*(s8 *)&c.r = -1;`.** A plain
  assignment gives `li 0xFF`. (func_8003FFAC)
- **An `(s16)` cast on a `u16` global folds into `lh`**, no shifts.
  (func_80033E98)
- **A constant rebuilt (`li $a0, 1`) where the build copies it from a saved
  register: the local holding it is narrower than `s32`.** (func_80013B38)
- **A single-bit flag as `andi K` + `sltu $zero`: `c = x & K; f = c != 0;`.**
  One-expression spellings give `srl` + `andi 1`. (func_80017640)
- **An `s16` loaded with `lh` before a halfword store of a scale: `w * 4`,
  not `w << 2`** (the shift narrows the load to `lhu`). (func_80017640)
- **A chained store that reads its first target back (`sh X; lhu X; sh Y`):
  a volatile target.** (func_8001FBBC)
- **Two sign tests, one `bltz` and one `and`/`bnez` sharing a `lui 0x8000`:
  both written `v & 0x80000000`.** (code_a0bc, round 7)
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
- **A redundant `lbu; andi 0xFF` / `lhu; andi 0xFFFF` on a value: the narrow
  parameter of an inlined `static __inline__` helper.** Locals and casts
  never keep the mask. (func_8002CC24, func_80014DB0)
- **A global reloaded below pointer stores that the build hoists: read it as
  a member of a struct at a fixed address**, `((VECTOR *)D)->vy` or a unit
  view `#define sCur (*(BlockCur *)&D)`. (func_8002C894, func_8002FDB4)
- **An argument passed unextended though the callee narrows it: an `s32`
  parameter in the caller's view.** (func_8002BEC0, func_8002CB24)
- **An `(s8)` cast on a `u8` global under `== 0` folds to `lbu`; retail's
  `lb` needs the global declared `s8`.** (code main, round 8)
- **An `s8` -1 byte store retail keeps above a later load: a local struct
  whose first member really is `s8`**, passed as `(CVECTOR *)&c`; the
  `*(s8 *)&c.r` cast lets the store sink. (func_8003EF40)

- **A global reached `lui` in one function of a unit and `$gp` in the rest:
  read it by literal address there**, `*(u8 *)0x80095AEE`, with a
  `MATCHING:` note. cc1 splits a constant address itself and maspsx leaves
  it alone. Probably a source-file seam inside the unit. (func_80041BAC)
- **`swc2 $17..$19` (or `$16..$19`) off one base: `gte_stsz3c` /
  `gte_stsz4c`** (include/gte.h). (func_8001B4BC)

- **A `jal` to an empty function as main's first call: cc1's `__main`.**
  Name that address `__main`; the C does not call it. (main)
- **An address computed by indexing a real array, `D[i].v[j]`, adds the
  offsets before the symbol**; a cast view `((T *)D)[i]` adds `D + i*SIZE`
  first and CSE shares it with a record pointer. (func_8002AA58)
- **A stack area block-copied from a constant at entry: an aggregate
  initialiser** (cc1 `$LC`); when the constant sits outside the unit's
  rodata, copy from an extern, declared `[]` when 8 bytes or less.
  (func_80028984)
- **An indexed store with the index computed before the table's
  `lui`/`addiu`: an array of at most 8 bytes (`s32 X[2]`)**, not a scalar
  indexed through `&`. (func_8003D960)
- **A table base built 10 records before a symbol (`addiu -0x320`): the
  array starts earlier and needs a symbol plus a linker definition.**
  (func_8003F100, func_80039C3C, both still open)

- **A table's `lui` before the index's `lui`: the base went into a pointer
  local just before use** (`recs = (T *)D; recs[i].f`), or the symbol is a
  real array of `T`; the cast `((T *)D)[i]` loads the index first.
  (func_8002DC44)
- **An odd register that is an argument register holding a value at a
  call: check the callee's argument count first.** A base built in `$a1`
  was a second argument the old prototype lacked. (func_80037114)

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
  More than four registers per batch, the first word through `%lo(sym)`:
  separate `s32` assignments, not a struct copy. (func_800335E8)
- **A zero returned from a saved register (`addu $v0, $s2, $zero`): an `s32
  ret = 0;` local in an `s32` function.** An `s16` one adds `sra`.
  (func_800383F8, func_80038468, func_8003950C)
- **`andi 0xFFFF` / `sltiu` on the counter with `sll`/`sra 16` on the index:
  a `u16` counter indexed through `(s16)i`.** (func_8002C650)
- **A parameter masked once in the prologue: `mode &= 1;` at the top**, not
  at its use. (func_800153CC)
- **A test result copied to a saved register (`sltu a0; move s3, a0`): the
  result variable is `u8` or `s16`.** (func_8002BEC0)
- **`s16 % 64` gives `sll/sra 16` around a halfword remainder on 2.8.1;
  retail's code is `x - x / 64 * 64`.** (func_8002B7C8)
- **`srl` on a product stored to an `s16`: `(u32)(x * 25) >> 9`.**
  (code_29f54, round 8)
- **A block move with a dead `lwl`/`lwr` path jumped over: the source pointer
  is a local whose alignment cc1 cannot see**; plain `lw`/`sw` is an
  `s32 w[N/4]` wrapper. (func_8002C2B4)

- **A parameter copied to another register at entry, extended in place at
  its first use and re-extended from the copy at each later use: an `s16`
  parameter**, not `s32` plus casts. (func_80032C28, func_80032964)
- **`x *= -1` on an `s16` member loads it `lh`; `x = -x` loads it `lhu`.**
  (func_8002B8F8, round 9; the function is still a stall)
- **An `lhu` switch on an `s16` global: `switch ((u16)g)`.** (func_80034070)
- **An offset above 0x7FFF built whole (`lui`/`ori`, `addu`) comes from byte
  arithmetic or an offset in a local**, `(u8 *)p + 0x8294`, or `off =
  0x1DFFE; *(u16 *)(p + off) = v;`. A member access or a constant index is
  split into `%hi` plus the remainder. (func_80030278, func_80037CF0)
- **Block-move batches of three words through a base in its own register:
  the destination is an unknown-size array**, `*(T *)D_800DF630 = x;`. A
  struct member at a fixed offset moves four words per batch. (func_80037CF0)
- **A table base built by `addiu` from another symbol's address: the array
  starts before the symbol splat named.** This needs a symbol (and a linker
  definition if no instruction references it), not a C spelling.
  (func_8003F100, still open)

- **A `u8` field average compiled to `srl`: `(u32)(a + b) >> 1`.** `s32`
  locals give `sra`. (func_8001B4BC)
- **A spill-choice residue with identical code otherwise: declare values
  that need no extension `s16`**; only the allocation changes (permuter
  find). (func_8001D39C)
- **A redundant `andi 0xFFFF` on an `lhu`-loaded index: an `s16` local
  indexed through `(u16)i`.** (func_80042538)
- **A constant local copied before a `sb` (`li sN, 1; move v1, sN`): the
  local is `s16` or `u8`, not `s32`.** (func_80041534)
- **`lb` of a `char *` argument's first byte: `*(s8 *)name`;** `(s8)name[0]`
  folds to `lbu`. (func_80041534)
- **A constant narrowed to a halfword add (`li 0xFE0C`) where retail has
  `addiu -500`: give each offset its own `s32` local.** (func_8003146C)

- **A callee's return type can decide a register tie**: a `void` callee
  that leaves `$v0` set, declared `s32`, closed 8 register diffs in two
  functions (permuter finds). (func_8002AA58, func_80028F0C)
- **`bltz` on an `lbu`-loaded `u8`: copied into an `s32` local and tested
  `>= 0`.** (code_29f54, round 11)
- **An `andi 0xFF` on an argument the prototype types `s32`: a `(u8)` cast
  at the call.** (code_29f54, round 11)
- **Byte averages that need `srl` with no mask: `u8` locals assigned
  `(u32)(a + b) >> 1`**; tell: the value is the first operand of its OR.
  (func_8001E558)

- **`addiu 1; andi 0xFFFF; sltiu 2` on a halfword global:
  `(u16)(x + 1) < 2`.** (func_800401F0)
- **`srl sN, a0; andi sN, sN` in one register: `v = a >> k; v &= m;`.**
  (func_800184BC)

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
- **A record loop whose pointer stays at the record start and rebuilds a
  constant every pass: `r = &tbl[i];` inside the body.** `r++` biases the
  pointer and hoists the constant. (func_80033BF8, func_80031A48)
- **A parameter advanced in steps pairs saved registers like retail**:
  `tmd++; f(tmd); tmd += 2; g(tmd);`, not `tmd + 2`. (func_8002C20C)
- **`addPrim(ot, p); p++; G = (u8 *)p;`**, not `G = (u8 *)(p + 1);`, for a
  primitive-buffer bump. libgpu's macros match unchanged. (func_80040F14)
- **A loop that jumps to its test at the bottom with no guard: `goto test;
  do { ... test:; } while (k < n);`** with the bound in a local; every
  `for`/`while` is rotated into a guarded loop. (func_80034BCC, func_80034F38)
- **A store left on the loop variable beside a strength-reduced pointer: a
  volatile store**, `((volatile T *)p)->code = x;`. `*(volatile u8 *)&p->code`
  loses the volatile. (func_800210B4, func_80021434, func_80021240)
- **A loop pointer copied out of its argument register: the source looped on
  a local copy of the parameter.** (func_800210B4)
- **The order of stores in a strength-reduced loop sets the order of the
  pointer setup before it**, and `o++, i++` differs from `i++, o++`.
  (func_80013EE4, func_80017270)
- **A row offset built as `row*21` then plus the column: a flat index**,
  `tbl[row*21 + col]`; `T t[][21]` scales the row by the row size.
  (func_800229A8)
- **`addu d, shift, base` (index first): an integer sum**, `(T *)(i * 8 +
  (s32)D)`. Pointer arithmetic always puts the pointer first.
  (func_8003A3F4, func_80026C70)
- **A count-down `bgez` loop: write it counting down.** 2.8.1 did not
  reverse a short `s32` count-up here. (code_29f54, round 5)
- **A loop compared on a pointer with signed `slt`: an `s32` counter cc1
  replaced with the pointer.** Write the counter; bump the pointer in the
  `for` step after `k++`. (func_80015180)
- **One counter tested `sltiu` in one loop and `slt` in the next: one `u32`
  counter compared `(s32)i < g` in the second.** (func_80030984)
- **A loop bottom testing `next` with `i = next` in the delay slot:
  `if ((s8)next < 0) break; i = next;` inside `do { } while (1)`.**
  (code_29f54, round 8)

- **A parameter re-extended at every use inside a poll loop: the loop was a
  `goto` label.** A C loop hoists the extension. (func_80037440)
- **`addiu` of the counter in the back-branch delay slot: `if (n < K) { n++;
  goto top; }`.** `if (n++ < K)` copies the old value first. (func_80037440)
- **A constant store after a search loop that retail keeps as its own block,
  with the constant loaded in the preheader: the store is inside the loop**,
  `if (++i == N) { G = K; return; }`. After the loop, cross-jumping merges it
  with an identical store. (func_800317D0)
- **Two block-move pointers from one base: advance the base in place as the
  destination**, `dst = base; src = &dst[k + 1]; dst += k;`. (func_800350C8)
- **Two loop pointers in swapped registers, no instruction different: wrap
  one bump as `do { src++; } while (0);`** (permuter find). (func_800350C8,
  func_80035350)

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
- **Two identical blocks retail keeps apart: one call per branch**, not one
  after the chain; cross-jumping folds the calls and leaves the blocks.
  (func_800330D4, func_8003C2E8) Every switch arm jumping to one shared store
  before the exit: write the arm that falls into the exit last.
  (func_80015584)
- **Products recomputed per arm that CSE would merge: assign the copies in
  each arm** (`x0 = x2 = (...) >> 12;`), so the join loses the equality.
  (func_800198BC)
- **A function repeating another's body on a fixed object: one `static
  __inline__` helper, both callers wrappers.** (func_80016D14,
  func_800173E8, func_80017270)
- **A packet used straight from `$a3` at fixed offsets: a `volatile` packet
  parameter.** A volatile local copy when retail moves the pointer to
  another register. (func_8001FE5C, func_8001FBBC)

- **Retail setting -1 in a saved register in each `j end` delay slot and
  copying it to `$v0` once at the exit: a single-exit `ret` local with `goto
  end`**, not `return -1`. Separate fail tails were `ret = -1; G = K;`; one
  shared tail was `G = K; ret = -1;`. (func_80037AE4, func_800377E8)
- **`beqz r; blez r; beq r, 1` on a call result: `switch (r) { case 0: ...;
  case 1: ...; case -1: break; }`.** (func_800345C8)
- **`bgez r; nop; negu r` with the delay slot unfilled is cc1's abs: only `v
  >= 0 ? v : -v` gives it.** `v < 0 ? -v : v` and `if (v < 0) v = -v;` give
  a branch with a filled slot. (func_8003B780)
- **A body repeating another function of the unit: a `static __inline__`
  helper with both functions as callers.** (func_8002BD00, func_8002C894)

- **A compare tree split one case lower than expected: an empty `case -2:
  break;` outside the visible range.** gcc 2.8 splits n nodes after node
  (n+1)/2. Listing `case 0:` or not flips a small tree between `== 0` first
  and `< 3` first; identical arms retail tests separately are two arms, not
  `case 1: case 2:`. (func_80026D9C, func_80023F80, func_80023228)
- **`switch` with the default arm last, not `if/else`**, keeps the default's
  tail from cross-jumping into a sibling. (func_80027E14)
- **Menu answers whose case tails cross-jump: switch on an `s16`**; as `s32`
  the tails stay separate. (func_80031EF4)
- **`s32` with no return also frees a forward branch's delay slot** for a
  constant where `void` fills it with `lui`. (func_8002D424)

- **`beqz` to one block and `beq K` to another on one variable, then a `j`
  over both: `switch (s) { case 0: ...; case K: ...; }`.** (code_29f54,
  round 11)
- **A result's `sll`/`sra` above the epilogue's register loads: an early
  `return x;`**; the return label splits the block. (func_80028AE4)
- **Two identical arms retail keeps apart: a bare `__asm__("")` after the
  call in one of them** (before it, cc1 still merges them). (main)

- **Two identical case blocks with retail keeping the later one: write the
  deleted case FIRST.** jump2's cross-jumping redirects and deletes the
  block whose jump it visits first. (func_80035970)
- **`FntPrint(c ? A : B)` with `.sdata` strings: two calls, `if (c)
  FntPrint(A); else FntPrint(B);`**; the ternary shares one `%hi`, a word
  short per site. (func_8002DC44)
- **Failure exits whose constants and bases CSE rebuilds partway: one
  `{ close(fd); return -1; }` per site, not a shared `goto fail`**;
  cross-jumping merges the blocks afterwards. (func_80032EE4)

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
- **Retail rebuilding local addresses before every call, with those locals
  after the caller's own in the frame: an inlined helper**, `static
  __inline__ setLs(x, y, z)` holding the shared body. (func_8003F960,
  func_8003FA88, func_8003FBE0, func_8003FD0C, func_8003FE5C, func_8003FFAC)
- **A jump table whose bound check's delay slot holds the index `sll`, not
  the table's `lui`: non-void with no return.** (func_80020CF8)
- **`A[g] = (p = expr) + 1;` loads `g` before `expr`.** (func_8002C044)
- **Asm splat calls handwritten, with `cfc2 $t4, $31` / `mtc2 x, $8`:
  compiled C using `gte_stflg` / `gte_lddp`** (include/gte.h).
  (func_80020DD8, func_80020F24)
- **A near-miss that differs only in delay slots: try `s32` with no return
  first.** It closed four of round 7's code_13068 functions; the permuter
  finds it as `volatile int`. (func_8002670C, func_80027714)
- **A clamp bound computed into a scratch register in a delay slot, then
  copied: `if (x <= hi) v = x; else v = hi;`.** (code_13068, round 7)
- **An inlined search whose hit path moves the counter to `$v0` and whose
  join sign-extends it: an inlined `s16` helper.** (func_800278B0)
- **A load that must stay below a store to another global, with no C
  spelling found: a bare `__asm__("")` barrier**, noted `MATCHING:`.
  (func_80028008, func_80027A00)
- **The stack moved to the scratchpad around a call: `SetSpadStack()` /
  `ResetSpadStack()` from include/spad.h.** (func_80017270)
- **Jump tables 4 bytes off in a whole unit: its rodata starts earlier than
  the yaml says** (cc1 puts `.align 3` before each table). (func_80036FE8)
- **Two addresses computed into one register in turn: one local reused**,
  `p = a->data; G1 = (s32)p; p = b->data; G2 = (s32)p;`. (func_800373C8)
- **A register parameter spilled where retail spills a stack one: name an
  unrelated early subexpression** (`r = col & 0x1F;`) to flip global-alloc.
  (func_800286B0)
- **A store retail keeps above a branch that the build moves into the
  delay slot: a volatile store**, `((volatile T *)p)->code = K;`.
  (func_8001B004)
- **A frame address (`addiu $sN, $sp, K`) hoisted into the prologue: put an
  unrelated statement before the struct copy that uses it.** A pointer
  local fixes only the register. (func_8001ACB4, func_8001A950)
- **`*(u32 *)&volatile_p->x` drops the volatile**, and the order of such
  reads decides the register assignment. (func_800201BC)
- **Spill slots follow the declaration order of the spilled locals.**
  (func_800198BC)
- **`-fschedule-insns` is byte-inert on this cc1**: only the pass after
  register allocation schedules, so register reuse decides load and store
  order. (code_1dc24, round 8)
- **A value computed into `$a1` and moved to `$a0` in a call's delay slot:
  one local reused**, `h = b*b; h -= c; f(h); h = a >> 1;`. (func_8003B780)
- **A quotient in the wrong saved register: `d = n; d /= 10;`** rather than
  `d = n / 10` (permuter find). (func_8003B780, func_8003A4B4)
- **`sllv x, x, $sN` where a constant shift is expected: cse took the count
  from a variable already holding it.** (func_8003A4B4)
- **`sh X; lhu X; sh Y` after an if/else: `Y = X;` as its own statement
  after the join.** (func_8003BDF4)
- **A load at the top of a case whose store comes last: write that
  assignment as the case's first statement**; the scheduler sinks the store.
  (func_800345C8)
- **Retail builds a vertex address in the register that held its index:
  drop the per-vertex pointer locals and spell each read
  `((T *)(i * 8 + (s32)base))->x`.** (func_800299D8)
- **A load-delay `nop` missing right after inline asm: name the register in
  the asm the way cc1 does (`$sp`, not `$29`).** maspsx compares register
  names as text. (func_8003A008)
- **A struct load retail keeps below stores to unrelated globals, every
  statement order identical: make the first read volatile**, `*(volatile
  s32 *)&sGame.f`. A field re-loaded at every use after one test was read
  through `*(volatile u8 *)&p->f`. (func_80026848, func_80023F80)
- **`lw x; move y, x; sltu y` with x stored afterwards: the source read the
  member twice**, `G = p->f; if (p->f >= L)`. (func_80041A6C)
- **A dead `lhu` of a field just before a store to its neighbour: a
  self-assignment**, `x.f = x.f;`. (func_80041534)
- **A table index shifted before the table's `lui`: the table address went
  into a pointer local first.** (func_80042538, func_80030548, func_8002F270)
- **Two loop-invariant base copies in swapped registers: assign the pointer
  local as the loop body's first statement.** (func_8002D424)
- **A counter's `move $sN, $zero` above a global's `lui` in the prologue:
  `i = 0; G = 0; for (; i < N; i++)`.** (func_80031064)
- **Interleaved stores `A[0], B[0], A[1], B[1]` were written in that order.**
  (func_800426A4)
- **A pointer parameter in the wrong saved register: drop the typed local
  copy and cast the parameter at each use.** (func_80023F80)
- **The order of stores inside switch cases decides which ORs `loop.c`
  hoists**, and so the length: it hoists invariants in instruction order and
  lowers its threshold by 3 per move. `cc1 -dL` shows each candidate.
  (func_8001E558)
- **A swapped prologue store pair (`sw zero` for `i`, `sw aN` home): the
  `PACKET *packet` parameter with a typed local copy.** (func_8001C13C)
- **A stack store retail keeps that cc1 deletes as dead: `*(volatile s32
  *)&x.f = t;`, then use `t`.** (func_80028AE4)
- **Two counters in one loop: `for (...; i++, k++)` with `a[k] = f()`**;
  `a[k++]` swaps their registers. (func_80028F0C)
- **Two strength-reduced loop registers swapped: reverse the ternary that
  selects between them.** (code_29f54, round 11)
- **Two pointers stepping by one stride where one is only copied on a hit:
  one pointer plus `best = f;`**; cc1 makes the second. (func_80029E74)
- **Two saved registers swapped between a long-lived, much-used variable
  and a short-lived one: one more reference to the short one** (`n = fd;
  close(n);` in a block cross-jumping deletes). Global alloc ranks by
  `floor_log2(refs) * refs / live_length`; `cc1 -dg` prints the order.
  (func_80032EE4)
- **A primitive whose every store stays in source order, colour stores
  reading their first target back: a `volatile T *` packet local.**
  (func_800184BC)
