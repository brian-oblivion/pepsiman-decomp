# UpdateTrapViewer — MATCHED 117/117 (round 9, charlie)

> Renamed from `func_8002B04C` on 2026-10-11 (tools/rename.py). Address 0x8002b04c.

Unit `src/code_1a098.c`. A trap-viewer frame: fixed reference view
(`gRefView` = 100, -200, 1000, 0, 0, 0), fog colour cleared, pad bits 2
and 1 step the selected Rec78 entry `D_800958E2`, clamped to `0 ..
gSectionCount - 1` with one nested-ternary store; then ResetCollision, a
zeroed Rec48 at `D_800A9008` turning with the frame counter, UpdateObjectEntry
on the selected entry, func_80023F80, func_80029838, the axes
(DrawAxes) and `FntPrint("TRAP NO  (%2d / %2d)", ...)`.

```c
extern s16 D_800958E2;    /**< the selected Rec78 entry */
extern char D_80010B34[]; /**< "TRAP NO  (%2d / %2d)" */
void func_80023F80(u8 *state);

/** @brief A trap viewer frame: fixed view, no fog, steps the selected
 *         Rec78 entry with the pad, places and draws it on a turning
 *         record, draws the axes and prints the entry number. */
void UpdateTrapViewer(void) {
    s32 flags;

    gWorldShift[0] = 0;
    gWorldShift[2] = 0;
    gRefView[0] = 100;
    gRefView[1] = -200;
    gRefView[2] = 1000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
    flags = gPadTrig;
    gClearB = 0;
    gClearG = 0;
    gClearR = 0;
    if (flags & 2) {
        D_800958E2++;
    }
    if (flags & 1) {
        D_800958E2--;
    }
    D_800958E2 = D_800958E2 < 0 ? 0 : D_800958E2 > gSectionCount - 1 ? gSectionCount - 1 : D_800958E2;
    ResetCollision();
    D_800A9008[0] = 0;
    D_800A9008[1] = 0;
    D_800A9008[2] = 0;
    ((s16 *)D_800A9008)[12] = 0;
    ((s16 *)D_800A9008)[13] = gFrameCount % 360 * 4096 / 360;
    ((s16 *)D_800A9008)[14] = 0;
    UpdateObjectEntry(&D_800D8D20[D_800958E2], (Rec48 *)D_800A9008);
    func_80023F80(D_8009EB78);
    func_80029838();
    DrawAxes();
    FntPrint(D_80010B34, D_800958E2, gSectionCount);
}```

First build. The clamp is SelectToolSection's (LEARNINGS, "a clamp whose one
store of the global sits at a join").

Declarations: `func_80023F80` (code_13068, still asm) is prototyped in this
unit as code_24748 prototypes it; declcheck lists both as MULTI/LOCAL
until a merge moves one copy to `include/code_13068.h`.

code_24748's RefreshToolPreview (NON_MATCHING) is this function's twin
without the pad and the drawing; the `D_800A9008` block above, three word
stores and three halfword stores through `(s16 *)`, is the form that
matched here.
