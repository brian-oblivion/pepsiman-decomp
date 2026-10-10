# main — MATCHED 627/627 (round 11, delta)

Unit `src/main.c`. The game's entry, called from SN's crt0
(`__SN_ENTRY_POINT`). It initialises the system (func_80014AC8,
func_80020C14, func_80018DE8(1), func_80018CB4, func_80018094, fog),
sets the start-up globals, fills two four-entry tables (clears
`D_80096A58`, copies `D_80010454` into `D_800D8340`), then loops forever:
`FntPrint`, the 45-state top-level `switch (D_80095880)` (boot, title,
stage load and play, demo, movies), `FntFlush(-1)`, func_80015584,
func_80042208, func_80014044.

The definition is `int main(void)`; the name stays `main`.

## Levers

About twelve builds: link fail -> 111 -> 613 -> 621 -> 627.

- **`__main`.** cc1 emits `jal __main` as main's first call; retail's is
  a call of the empty function at 0x80042C50 (the 8 bytes right before
  `__SN_ENTRY_POINT`, most likely libsn's `__main`, carved into code_31cec
  as C). The C does not call __main. Round 11's head named that address
  `__main` (tools/rename.py), which replaced the runner's interim
  `.set __main, func_80042C50` line.
- **Case bodies in source order (LEARNINGS):** cases 38, 39 and 40 sit
  between case 8 and case 12 in retail, so the source lists them there.
- **Two identical arms kept apart.** Cases 0 and 3 of the world switch
  both call func_800F03BC (two overlays with an entry at one address);
  cross-jumping folds them. A bare `__asm__("")` AFTER the call in case 3
  keeps them apart; before the call the tails still merge.
- **The table copy is two `s32` locals through a destination pointer
  local**: `dst = &D_800D8340[i]; a = src[i].a; b = src[i].b; dst->a = a;
  dst->b = b;`. A struct copy is a BLKmode move after which cse reloads
  the global counter; an `s64` copy loads into a register pair (wrong
  order, and the zero stores go through `move t2, zero`); member-by-member
  assignment interleaves the loads and stores; locals without `dst` swap
  the hoisted `$a2`/`$a3` bases (621/627).
- The five movie names are string literals: splat migrated
  D_80010010..D_80010050 into main's own `.s`, so the literals are their
  definitions (as func_800384DC). Cases 2 and 3 share `"\\MOVIE3.STR;1"`.
- The loop keeps 1, 2, 6 and 0x26 in `$s0..$s3` by itself; no locals
  needed.
- Case 20 stores 23 and then 21 to `D_80095880`; retail has both stores.

## Declarations left for the head

declcheck MULTI: `D_800956D0` (code_7d74), `D_800958E2` (code_1a098),
`D_80095908` and `D_80095968` (code_29f54) are now declared in main.c as
well. LOCAL: prototypes for other units' functions (func_80017B38,
func_80020C14, func_800229A8, func_80022A74, func_8002B04C,
func_8002D424, func_80034070, func_80040CD0, func_8004121C,
func_80041BAC, func_80042208).

### Proposed learning

- **`jal` to an empty function as main's first call: cc1's `__main`.**
  Name that address `__main`. (main)
- **Identical switch arms retail keeps apart: `__asm__("")` after the
  call in one of them**, not before (cross-jumping matches tails).
  (main)
- **An 8-byte record copied as two loads then two stores, base registers
  in retail's order: two `s32` locals and a destination pointer local set
  first.** (main)
