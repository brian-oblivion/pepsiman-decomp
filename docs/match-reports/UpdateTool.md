# UpdateTool — MATCHED 198/198 (round 9, echo)

> Renamed from `func_80034070` on 2026-10-11 (tools/rename.py). Address 0x80034070.

Unit `src/hit_tool.c`. The tool's top-level step, by `gModeStep`:
state 0 resets the two record banks and the tool's globals (ResetTool's
body, plus the menu and error state) and advances; state 1 dispatches on the
error code `gToolMode` (jump table `jtbl_80011804`, cases 0..7), then runs
the per-frame helpers and draws the dispatch object and the cursor.

```c
void UpdateTool(void) {
    switch (gModeStep) {
        case 0:
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
            gToolRadius = 5;
            gToolViewPreset = 0;
            D_80095A22 = 0;
            gToolSection = 0;
            gToolKind = 0;
            gToolPointIndex = 0;
            D_80095A5C = 0;
            gToolQuadIndex = 0;
            D_80095A40 = 0;
            D_80095A58 = 0;
            D_80095A28 = 0;
            gToolMode = 0;
            D_8009574A = 0;
            D_80095748 = 0;
            gRepeatButtonsOn = 1;
            gModeStep++;
            break;
        case 1:
            switch ((u16)gToolMode) {
                case 0:
                    MoveToolCursor();
                    ToolPlace();
                    if (gToolKind == 1 && (gPadTrig & 0x40)) {
                        gToolQuadStage = 0;
                    }
                    break;
                case 1:
                    UpdateToolMenu();
                    break;
                case 2:
                    EditToolValues();
                    break;
                case 3:
                    EditHitPoints();
                    break;
                case 4:
                    gToolQuadStage = 0;
                    gToolMode = 0;
                    break;
                case 5:
                    EditHitQuads();
                    break;
                case 6:
                    break;
                case 7:
                    UpdateToolCard();
                    break;
            }
            PrintToolStatus();
            CycleToolStep();
            RefreshToolPreview();
            if ((u16)(gToolMode - 6) >= 2) {
                SelectToolView();
            }
            UpdateToolView();
            if (gToolKind == 1 && gToolQuadStage == 1) {
                DrawHitQuad(&gToolNewQuad);
                SetBoxAround(&gToolNewQuad, D_8009EEC0, gToolQuadW, gToolQuadH);
            }
            if (D_80095A58 == 0) {
                SelectToolSection();
                DrawToolPin((VECTOR *)D_8009EEC0);
                DrawToolPole((VECTOR *)D_8009EEC0);
                if (gToolKind == 0) {
                    DrawToolRadius((VECTOR *)D_8009EEC0, (s16)gToolRadius * 10);
                }
            }
            break;
    }
}
```

## Levers

| C | words |
| --- | --- |
| `switch (gToolMode)` (an `s16`) | 197/198: `lh` where retail has `lhu` |
| **`switch ((u16)gToolMode)`** | **198/198** |

- The constant 80 kept in `$s0` across the calls is cc1's own CSE; an
  `s32 n = 80;` local gives the same bytes, so the literal form stays.
- `(u16)(gToolMode - 6) >= 2` is the `addiu -6; sltiu 2` test
  (LEARNINGS' `(u32)(x - K) < N`, here negated).
- Every callee defined later in the unit is prototyped once at the unit's
  top (`DrawToolPin`/`DrawToolPole` moved there).
