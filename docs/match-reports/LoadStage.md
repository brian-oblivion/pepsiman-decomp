# LoadStage — MATCHED 779/779 (round 11, delta)

> Renamed from `func_800160E8` on 2026-10-10 (tools/rename.py). Address 0x800160e8.

Unit `src/main.c`. The stage loader for the play states, stepped once per
frame on `gSubStep` (LoadAct3Stage's sibling for the other stages):
requests the stage's sound bank (LoadVabBank of GetStageVabBank's
answer), then in turn reads the stage's files 0, 1, 2+3, 4, 5 (and 6 on
stages 1, 4, 7, 10, 13) through the `gCdLoadLoc` / `gCdLoadSectors` /
`gCdLoadDest` request slots, handing each to its parser (LoadTimList,
LoadTimPack, LoadStageData, LoadSpritePack, func_800373C8 ...), and ends
with `gSubStep = 100`. Called from UpdateLoadScreenModel and UpdateLoadScreen.

The body is in `src/main.c` (first build matched).

## Levers

One build: 779/779.

- **LoadAct3Stage's lever carried over: one pair of extern arrays per
  stage, never a record table.** The stage directories are 0x48 bytes
  apart (`D_80073C70 + 0x48 * stage`).
- **The size array's base is 0x20 past the positions, and sizes are
  indexed one higher than their file** (`D_80073C90[k + 1]` for file
  `k`). Retail's arm that reads two sizes builds the base
  `D_80073C90` itself and loads at 0xC and 0x10; with a base at +0x24
  it would load at 8 and 0xC from another address. (LoadAct3Stage's
  stages 2, 5, 8, 11, 14 use the +0x24 names splat made, which give the
  same bytes for single loads.)
- Case 0 falls through into case 1 (no `break`).
- `(u8)(gStage % 3) == 1` gives retail's `andi 0xFF` on the
  remainder; the following switch reuses the loaded stage.

## Declarations left for the head

declcheck LOCAL: the new prototypes for other units' functions
(GetStageVabBank, LoadStageData, LoadSpritePack, func_8002CC24,
func_800373C8, func_8002C2B4, func_8002C044, func_8002C47C,
StartStageOverlay). func_800373C8 is declared with `void *` parameters
(code_1dc24 types them as its own `BankFile *`).

### Proposed learning

- **A base address built for two loads at +0xC/+0x10 names the array's
  real start.** When retail loads `X[3]` and `X[4]` off one `la`, the
  symbol in that `la` is where the source's array begins.
  (LoadStage)
