# EditorFreeMove — MATCHED 207/207 (round 9, alpha)

> Renamed from `func_8002FA78` on 2026-10-11 (tools/rename.py). Address 0x8002fa78.

Unit `src/editor.c`. Free movement in the tool: four pad bits and
the two analogue axes `gStickRawY`/`gStickRawX` (below 0x60 or above
0xA0) each move the game position with `MoveGamePos` by
`sTotals.unk4`; sets the camera offset `gWorldShift` to the negated
position; turns `gCourse[0].vy` by 11; unless `D_800958D8`, steps the
camera height `sTotals.unkC` by 5 (clamped to at most 0); steps the height
offset `sTotals.unk8` by `sTotals.unk4`; then finds the entry under the
position with `FindFloorNear`, stores it in `gCurBlock` (and
`gFloorIndex` when found), puts the position on that ground height plus the
offset, and on flag bit 5 with no entry sets error 7. Second build matched.

## Shapes read from retail

- Written straight from the disassembly; the first build was one word
  short: the flag test's `beqz` delay slot. Non-void with no return
  (LEARNINGS) closed it.

## Declarations

- `EditorState` (unit-local) splits `unk4[8]` into `s32 unk4` and
  `s32 unk8`; `GameSave` (unit-local) gains `s32 unk3C4`.
- New in the `.c`: a forward prototype of this unit's `MoveGamePos`,
  `extern u32 gStickRawY` and `extern u32 gStickRawX`.

```c
s32 EditorFreeMove(void) {
    s32 cur;

    /* MATCHING: non-void with no return keeps the flag test's delay slot
     * a nop. */
    if (gPadTrig & 0x1000) {
        MoveGamePos(0, sTotals.unk4);
    }
    if (gPadTrig & 0x4000) {
        MoveGamePos(180, sTotals.unk4);
    }
    if (gPadTrig & 0x8000) {
        MoveGamePos(-90, sTotals.unk4);
    }
    if (gPadTrig & 0x2000) {
        MoveGamePos(90, sTotals.unk4);
    }
    if (gStickRawY < 0x60) {
        MoveGamePos(0, sTotals.unk4);
    }
    if (gStickRawY > 0xA0) {
        MoveGamePos(180, sTotals.unk4);
    }
    if (gStickRawX < 0x60) {
        MoveGamePos(-90, sTotals.unk4);
    }
    if (gStickRawX > 0xA0) {
        MoveGamePos(90, sTotals.unk4);
    }
    gWorldShift[1] = 0;
    gWorldShift[0] = -sGameSave.unk348[0];
    gWorldShift[2] = -sGameSave.unk348[2];
    if (gPad & 4) {
        gCourse[0].vy -= 11;
    }
    if (gPad & 8) {
        gCourse[0].vy += 11;
    }
    if (D_800958D8 == 0) {
        if (gPad & 1) {
            sTotals.unkC -= 5;
        }
        if (gPad & 2) {
            sTotals.unkC += 5;
        }
        if (sTotals.unkC > 0) {
            sTotals.unkC = 0;
        }
    }
    if (gPadTrig & 0x80) {
        sTotals.unk8 -= sTotals.unk4;
    }
    if (gPadTrig & 0x40) {
        sTotals.unk8 += sTotals.unk4;
    }
    sGameSave.unk3C4 = sGameSave.unk348[1];
    cur = FindFloorNear(sGameSave.unk348, gFloorY, gFloorIndex);
    gCurBlock = cur;
    if (cur != -1) {
        gFloorIndex = cur;
    }
    sGameSave.unk348[1] = gFloorY[0] + sTotals.unk8;
    if (D_800958D8 == 0 && (gPadTrig & 0x20) && cur == -1) {
        gToolMode = 7;
    }
}
```
