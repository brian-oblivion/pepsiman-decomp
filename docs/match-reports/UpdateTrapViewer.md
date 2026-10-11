# UpdateTrapViewer — MATCHED 117/117 (round 9, charlie)

> Renamed from `func_8002B04C` on 2026-10-11 (tools/rename.py). Address 0x8002b04c.

Unit `src/object.c`. A trap-viewer frame: fixed reference view
(`gRefView` = 100, -200, 1000, 0, 0, 0), fog colour cleared, pad bits 2
and 1 step the selected ObjectModel entry `gSelectedSection`, clamped to `0 ..
gSectionCount - 1` with one nested-ternary store; then ResetCollision, a
zeroed Trap at `gTraps` turning with the frame counter, UpdateObjectEntry
on the selected entry, ReactToHits, LeaveQuadTop, the axes
(DrawAxes) and `FntPrint("TRAP NO  (%2d / %2d)", ...)`.

```c
extern s16 gSelectedSection;    /**< the selected ObjectModel entry */
extern char gTrapNoFmt[]; /**< "TRAP NO  (%2d / %2d)" */
void ReactToHits(u8 *state);

/** @brief A trap viewer frame: fixed view, no fog, steps the selected
 *         ObjectModel entry with the pad, places and draws it on a turning
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
        gSelectedSection++;
    }
    if (flags & 1) {
        gSelectedSection--;
    }
    gSelectedSection = gSelectedSection < 0 ? 0 : gSelectedSection > gSectionCount - 1 ? gSectionCount - 1 : gSelectedSection;
    ResetCollision();
    gTraps[0] = 0;
    gTraps[1] = 0;
    gTraps[2] = 0;
    ((s16 *)gTraps)[12] = 0;
    ((s16 *)gTraps)[13] = gFrameCount % 360 * 4096 / 360;
    ((s16 *)gTraps)[14] = 0;
    UpdateObjectEntry(&gObjectModels[gSelectedSection], (Trap *)gTraps);
    ReactToHits(gPlayer);
    LeaveQuadTop();
    DrawAxes();
    FntPrint(gTrapNoFmt, gSelectedSection, gSectionCount);
}```

First build. The clamp is SelectToolSection's (LEARNINGS, "a clamp whose one
store of the global sits at a join").

Declarations: `ReactToHits` (player, still asm) is prototyped in this
unit as hit_tool prototypes it; declcheck lists both as MULTI/LOCAL
until a merge moves one copy to `include/player.h`.

hit_tool's RefreshToolPreview (NON_MATCHING) is this function's twin
without the pad and the drawing; the `gTraps` block above, three word
stores and three halfword stores through `(s16 *)`, is the form that
matched here.
