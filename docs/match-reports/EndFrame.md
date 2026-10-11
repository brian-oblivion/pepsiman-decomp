# EndFrame — MATCHED 170/170 (round 8, echo)

> Renamed from `func_80014044` on 2026-10-10 (tools/rename.py). Address 0x80014044.

Unit `src/main.c`. The end-of-frame routine: sets the reference view (the
game's at `gRefView` unless state 14, or a fixed view 400 out), waits
for VSync (count kept in `gVSyncTime`), swaps buffers, clears to the fog
colour, adds the two `DR_STP` primitives around the 3D ordering table,
draws, switches the work buffer, re-clears both ordering tables, sorts the
sprite table into the 3D one, runs ReadPad and RemapButtons, and
counts the frame unless `gPauseState` is set.

Declaration changes: `gLoading`, `gPauseState` and `ClipF` moved from
`src/stage_flow.c` to `include/common.h` (declcheck MULTI); `gPauseState`
is now `s8` (was `u8`; stage_flow only stores 0 to it, no byte change).
The prototype of ReadPad moved up to this function.

```c
/* Sony's (libgs, carved as asm): GsSortClear by its arguments. */
void GsSortClear(u8 r, u8 g, u8 b, GsOT *ot);
void SetLetterbox(s32 clip);
void RemapButtons(void);
void ReadPad(s32 arg);

extern s32 gVSyncTime;
extern DR_STP sDrawStpOn;
extern DR_STP sDrawStpOff;

void EndFrame(void) {
    GsRVIEW2 view;

    switch (gLoading) {
        case 0:
            if (gGameMode != 14) {
                GsSetRefView2((GsRVIEW2 *)gRefView);
            }
            break;
        case 1:
            view.vpx = 0;
            view.vpy = 0;
            view.vpz = 400;
            view.rz = 0;
            view.vrx = 0;
            view.vry = 0;
            view.vrz = 0;
            GsSetRefView2(&view);
            break;
    }
    gVSyncTime = VSync(gVSyncMode);
    DrawSync(0);
    GsSwapDispBuff();
    GsSortClear(gClearR, gClearG, gClearB, &gOt[gActiveBuff]);
    SetDrawStp(&sDrawStpOn, 1);
    AddPrim(&gOt[gActiveBuff].org[0xFFF], &sDrawStpOn);
    SetLetterbox(ClipF);
    GsDrawOt(&gOt[gActiveBuff]);
    gActiveBuff = GsGetActiveBuff();
    GsSetWorkBase(gGpuWork[gActiveBuff]);
    GsClearOt(0, 300, &gOt[gActiveBuff]);
    GsClearOt(0, 0, &gSubOt[gActiveBuff]);
    GsSortOt(&gSubOt[gActiveBuff], &gOt[gActiveBuff]);
    gCurOt = &gOt[gActiveBuff];
    SetDrawStp(&sDrawStpOff, 0);
    AddPrim(gSubOt[gActiveBuff].org, &sDrawStpOff);
    ReadPad(0);
    RemapButtons();
    if (gPauseState == 0) {
        gFrameCount++;
    }
}
```

Second build. The switch on {0, 1} with an empty default and a shared
`GsSetRefView2` call came out as written (cross-jumping merges the two
calls). The `GsRVIEW2` members are assigned in retail's store order
(`vpx`, `vpy`, `vpz`, `rz`, `vrx`, `vry`, `vrz`; `super` is never set).
`func_80056B8C` is libgs's `GsSortClear`, carved as asm, declared by its
arguments like GsInitGraph. The only residue of the first build was
`lb` for `gPauseState`: an `(s8)` cast on a `u8` folds away under `== 0`,
so the global's declaration became `s8`.
