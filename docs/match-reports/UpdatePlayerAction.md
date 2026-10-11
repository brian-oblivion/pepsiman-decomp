# UpdatePlayerAction — MATCHED (round 13, echo)

> Renamed from `func_80024450` on 2026-10-11 (tools/rename.py). Address 0x80024450.

Unit `src/code_13068.c`. The player's mode step, called through a pointer
with the state block (`gPlayer`, this unit's `GameState`). It counts
down unk3C8 (re-arming unk2), steps one or two animation tracks
(`StepAnimation` into unk340 / unk341), then runs two state machines:
a jump-table switch on unk5 (0x07..0x5E) that starts a mode (sets unk5 to
its "running" value, unk6 to the mode, clears the step counters, plays a
sound through `StartRumble(gRumblePatterns, n)`), and a jump-table switch on
unk6 (2..0x39) that runs it: the goal-cap approach (cases 8..0xB, 0x38,
0x39), the spin on the goal line (0x15..0x17), the camera shake and
zooms (0x23, 0x28, 0x30, using `gView` as a GsRVIEW2), the
continue/game-over countdowns that end in `gMissState = 100`, and the
reset to mode 2 through `ResetPlayerAction`.

First attempt this round. The m2c seed was already close in control flow;
2110 words, length exact after the levers below; two words differ.

## Residue (round 12, closed in round 13)

Case 8's second goal path (`else if (state->unk34C >= state->unk3C0)`):

```
retail                              build
lui   s0,%hi(gPlayer)  (slot)    lui   s0,%hi(gPlayer)  (slot)
lui   v0,%hi(gFloorY)            addiu s0,s0,%lo(gPlayer)
addiu s0,s0,%lo(gPlayer)         lui   v0,%hi(gFloorY)
lh    v0,%lo(gFloorY)(v0)        lh    v0,%lo(gFloorY)(v0)
```

Same instructions, same registers, one swap by sched2 inside one basic
block. The RTL before sched2 has the `gFloorY` load first (source
order). Taking the `lb 0x38E(s0)` after the call off `s0` moves
`addiu` to the bottom instead, so the order follows the priority of the
`&sGame` chain; retail's priority sits between the two. Nothing in the
block's source has been found that sets it there.


## Round 13 (echo): matched

The residue was not in sched2 but in sched1. `cc1 -dS` prints sched1's
ready list per block; for this block it read (bottom-up): call, the
`sw`, then the `$a1 <- d` copy before the `$a0 <- &sGame` copy (both
priority 1, so the higher LUID wins, and argument registers are always
loaded in argument order). That let the `lh` and the `%hi(gFloorY)`
become ready first, and `adjust_priority` boosts both to
`LAUNCH_PRIORITY` (they "birth" a single-set pseudo), so the `$a0` copy
and the `&sGame` pair sank below them; sched2 then breaks its ties by
LUID and gives the build's order.

Retail's second goal path passes ONE argument. `func_80018CAC` (empty in
model) is declared unprototyped in this unit, `void
func_80018CAC();`; the first path calls `func_80018CAC(&sGame, d)` (that
call is still what gives `d` its `$a1` preference) and the second
`func_80018CAC(&sGame)`. With no `$a1` copy the `$a0` copy is scheduled
right after the `sw`, and both schedulers reproduce retail. 2108 -> 2110,
build `OK`, in 4 builds.

Levers tried this round before it:

- One `GameState *g = &sGame;` for both paths, set again in the second:
  609/2110 (the second path's address joins the first's pseudo, `lui v0`
  / `addiu s0, v0`, and the first path's pair moves above the `lhu`).
- `g = &sGame;` only in the second path, before the `gFloorY` store:
  2108, same swap.
- The `gFloorY` value through the function's `s32 v` (a pseudo with
  several sets, so not "birthing"): 755/2110, global register shift.

### Proposed learning

- **A call's argument copies scheduled out of retail's order around a
  load: check the argument count.** sched1 picks the later argument copy
  first, and a load that births a pseudo then outranks the earlier copy.
  A callee whose calls pass different counts is declared unprototyped in
  the unit (`void f();`). (UpdatePlayerAction)
- `cc1 -dS` (and `-dR` for sched2) prints each block's ready list and
  choices; it shows which heuristic (priority, birth boost, LUID) decided
  a swap.

## Levers that took it from the m2c seed (length 21 short, ~600 words) to 2108

- **`s32` with no return**: the delay slots of branches to the exit are
  `nop`s in retail (`` live at the exit). Fixed most of the length.
- **Cross-jumping direction**: retail keeps case 0xD's `ResetPlayerAction()`
  call and cases 7, 0x1F and 0x26 branch into it. Each case written with
  its own call kept the LAST copy (case 7's). Cases 7/0x1F/0x26 `goto` a
  label in front of case 0xD's call. (+1000 words of drift.)
- **One step variable for four sites, typed `s32`**: the halfword step
  `state->unk398 -= 5 / 7` feeding `state->unk34C -= step` is
  sign-extended into `` at all four sites in retail. One local `d`
  (`state->unk398 -= 7; d = state->unk398; state->unk34C -= d;`) and
  case 8 passing it as `func_80018CAC(&sGame, d)` (an empty function in
  model; retail's `` is set there) gives global-alloc a hard-reg
  preference for `` for the whole pseudo. An `s16` local
  re-extends at the call; the `d = (s16)(x - 7); x = d;` order stores
  the extended register.
- **Byte-pointer read** for the second `Lerp` call's
  `state->unk374` keeps its `lh` below the store to `D_80095740`
  (CLAUDE.md's MEM_IN_STRUCT rule, LEARNINGS "A field loaded after
  earlier global stores").
- **`x - (5 - rand() % 10)`** keeps the `addiu -5` on the loaded
  `gView` word; `x - 5 + r`, `x + (r - 5)`, `x += r - 5`,
  `r + x - 5` all put it on the remainder.
- **abs on an expression of a local**: `v = rsin(...); gFloorY[0] -
  ((v * 100 >> 12) >= 0 ? ... : ...)` builds the table's `%hi` before
  the multiply, as retail; with `v` holding the product the `lui`
  lands after the abs.
- **`state->unk3A8 *= -1`** for the second `lh` (LEARNINGS).
- **Three nested `if`s** on the `s8` result of `LandPlayer`:
  `r != 0 && r >= 0 && r < 3` folds into `(u8)(r - 1) < 2`.
- **`switch ((s8)sGame.unk38E) { case 0: default: ...3; case 2: ...0x1A; }`**:
  `beqz` then `beq 2` with the 0x1A in the slot. `if (x == 0 || x != 2)`
  folds to `!= 2`; a separate default arm adds a `j`.
- **Case 0x1F written `if (unk3A8 != 0) {...} else {...}`** (its
  nonzero arm is the shared fall-through block).
- **A bare `__asm__("")`** after `state->unk398 = 300` in case 0x56
  keeps the call's argument setup below that store.

## Axes tried on the residue (about 20 builds, one permuter search)

- Store/call spellings: `*gFloorY`, `(GameState *)gPlayer`, a
  `GameState *g` local for the call and the switch, `func_80018CAC(&sGame,
  state->unk398)`, the load through `t`/`v`/`r` locals, volatile store,
  volatile load, barriers after the call and between store and call:
  2108 or worse.
- Diagnostic only: reading unk38E through `state` (no `s0` use after
  the call) moves `addiu s0` to the bottom of the block, so the swap is
  a sched2 priority tie between the `&sGame` and `gFloorY` chains.
- **Permuter**, PERM_RANDOMIZE around case 8 only; Gate 3: (1) scaffold
  builds, base 150 (90 of it the 18 relocation-name "register" diffs that
  never change); (2) scaffold 0 ins / 0 del, 1 reordering; (3) in-tree
  funcdiff 1/1 for the same swap: AGREE. `timeout 1800`, rc 124,
  85239 iterations (under load, other runners active). Best 100:
  `d = gFloorY[0]; state->unk34C = d; func_80018CAC(&sGame, d);`
  fixes the order but loads into `` (2108 in-tree, two other words).
  Not closed.

### Proposed learning

- A sign-extended value that retail keeps in an argument register at
  several sites with no call there: one local for all sites, passed as
  that argument at ONE site; global-alloc's preference spreads it.
- Cross-jumping keeps the later copy of a repeated call block; when
  retail keeps the earlier one, the later cases `goto` into it.
