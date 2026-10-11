# LoadStage — MATCHED 779/779 (round 11, delta)

> Renamed from `func_800160E8` on 2026-10-10 (tools/rename.py). Address 0x800160e8.

Unit `src/main.c`. The stage loader for the play states, stepped once per
frame on `gSubStep` (LoadAct3Stage's sibling for the other stages):
requests the stage's sound bank (LoadVabBank of GetStageVabBank's
answer), then in turn reads the stage's files 0, 1, 2+3, 4, 5 (and 6 on
stages 1, 4, 7, 10, 13) through the `gCdLoadLoc` / `gCdLoadSectors` /
`gCdLoadDest` request slots, handing each to its parser (LoadTimList,
LoadTimPack, LoadStageData, LoadSpritePack, SetHitBanks ...), and ends
with `gSubStep = 100`. Called from UpdateLoadScreenModel and UpdateLoadScreen.

The body is in `src/main.c` (first build matched).

## Levers

One build: 779/779.

- **LoadAct3Stage's lever carried over: one pair of extern arrays per
  stage, never a record table.** The stage directories are 0x48 bytes
  apart (`sStage00FileLoc + 0x48 * stage`).
- **The size array's base is 0x20 past the positions, and sizes are
  indexed one higher than their file** (`sStage00FileSectors[k + 1]` for file
  `k`). Retail's arm that reads two sizes builds the base
  `sStage00FileSectors` itself and loads at 0xC and 0x10; with a base at +0x24
  it would load at 8 and 0xC from another address. (LoadAct3Stage's
  stages 2, 5, 8, 11, 14 use the +0x24 names splat made, which give the
  same bytes for single loads.)
- Case 0 falls through into case 1 (no `break`).
- `(u8)(gStage % 3) == 1` gives retail's `andi 0xFF` on the
  remainder; the following switch reuses the loaded stage.

## Declarations left for the head

declcheck LOCAL: the new prototypes for other units' functions
(GetStageVabBank, LoadStageData, LoadSpritePack, InitEntryModels,
SetHitBanks, RestoreStageObjects, LoadStageSprites, ResetStageObjects,
StartStageOverlay). SetHitBanks is declared with `void *` parameters
(editor types them as its own `BankFile *`).

### Proposed learning

- **A base address built for two loads at +0xC/+0x10 names the array's
  real start.** When retail loads `X[3]` and `X[4]` off one `la`, the
  symbol in that `la` is where the source's array begins.
  (LoadStage)
