# ResetTool — MATCHED 64/64 (round 4, charlie)

> Renamed from `func_80036D50` on 2026-10-11 (tools/rename.py). Address 0x80036d50.

Unit `src/hit_tool.c`. Resets the editor's state: clears two words of
`gWorldShift`, sets the entry count to 80, clears the two record banks
(64 KiB each at 0x8016D000 and 0x8017D000; the first word of each is the
count 80, and `gHitPoints`/`gHitQuads` point just past it), resets the
per-entry halfwords of each (`ClearHitPoints`, `ClearHitQuads`), clears three
words of the game state and sets a run of `$gp` globals. First build
matched.

```c
void ResetTool(void) {
    gWorldShift[0] = 0;
    gWorldShift[2] = 0;
    gSectionCount = 80;
    bzero((u8 *)0x8016D000, 0x10000);
    *(s32 *)0x8016D000 = 80;
    gHitPointSections = 80;
    gHitPoints = 0x8016D004;
    ClearHitPoints();
    bzero((u8 *)0x8017D000, 0x10000);
    *(s32 *)0x8017D000 = 80;
    gHitQuadSections = 80;
    gHitQuads = 0x8017D004;
    ClearHitQuads();
    sGamePos.unk348 = 0;
    sGamePos.unk34C = 0;
    sGamePos.unk350 = 0;
    gToolZoom = 10;
    gToolViewStep = 2;
    gToolStep = 10;
    gToolViewPreset = 0;
    sToolModeSaved = 0;
    gToolSection = 0;
    gToolKind = 0;
    gToolPointIndex = 0;
    D_80095A5C = 0;
    gToolQuadIndex = 0;
    D_80095A40 = 0;
    gToolRadius = 5;
    sToolPickMode = 0;
    sToolInfoMode = 0;
}
```

## Externs and types

- splat's `D_8016D000` and `D_8017D000` are not objects: the stores are
  `lui $at` / `sw %lo` pairs that cc1 emits for a store to a constant
  address, `*(s32 *)0x8016D000 = 80;`, while the same constant as an
  argument is `lui`/`ori`. The bytes are identical; no extern is declared.
- `gHitPointSections`, `gHitQuadSections`: `lui $at` stores, plain `s32` outside `$gp`.
- `gPlayer` is `u8 []` in `common.h`; retail builds its address once and
  stores at 0x348/0x34C/0x350, so a struct lvalue through a unit-local view
  (`GameStatePos`, `#define sGamePos`). `player.c` has the full
  `GameState` with the same three fields; the two views could become one
  shared type when another unit needs it.
- The run of `$gp` globals: types from the store widths (`sb` `u8`, `sh`
  `s16`, `sw` `s32`); `gToolRadius` keeps the `u16` that `EditToolValues`'s
  `lhu` gave it.

The constant 80 kept in `$s0` across the calls came from cc1's CSE of the
literal, no local needed (contrast CheckCanBonus's across-blocks case).
