# PlayCdTrack — MATCHED (77/77), round 7, runner bravo

> Renamed from `func_80017440` on 2026-10-10 (tools/rename.py). Address 0x80017440.

Starts CD audio track `track`: stops any playing track, sets the CD mix
(stereo or mono by `gStereo`), reads the TOC, stores the track's start
and its end (next track's start minus 20 sectors), sets the serial
(CD) attribute and volume, sets CD mode 3, installs the ready callback
`OnCdTrackReady` and issues `CdlPlay` through `SendCdTrackCommand`.

```c
extern CdlLOC gCdToc[];
extern s32 gCdTrackEnd;
extern s32 gCdTrackLoop;
/* MATCHING: a known small size, so the byte store is `$gp`-relative. */
extern u8 D_80095AC0[4];

void PlayCdTrack(s32 track, s32 arg) {
    CdlATV atv;

    if (gCdTrackPlaying == 1) {
        StopCdTrack();
    }
    if (gStereo) {
        atv.val2 = 0x50;
        atv.val0 = 0x50;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = 0x50;
        atv.val0 = 0x50;
        atv.val3 = 0x50;
        atv.val1 = 0x50;
    }
    CdMix(&atv);
    gCdTrackLoop = arg;
    CdGetToc(gCdToc);
    gCdTrackStart = CdPosToInt(&gCdToc[track]);
    gCdTrackEnd = CdPosToInt(&gCdToc[track + 1]) - 20;
    gCdTrackPlays = 0;
    gCdTrackPos = gCdTrackStart;
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x72, 0x72);
    D_80095AC0[0] = 3;
    CdControlB(CdlSetmode, D_80095AC0, 0);
    VSync(3);
    CdReadyCallback((CdlCB)OnCdTrackReady);
    SendCdTrackCommand(CdlPlay);
    gCdTrackPlaying = 1;
}
```

Matched on the first build. The `CdlATV` stores follow source order
(LEARNINGS, "Independent stores to a local struct"): retail writes
`val2, val0, val3, val1`.

## Declarations

- `gCdTrackPlaying` (u8), `gStereo` (u8), `SendCdTrackCommand` and `OnCdTrackReady`
  moved to `include/common.h` (additive), since code_7d74 and code_31cec
  reach them too. The local `extern u8 gStereo;` in `src/code_31cec.c`
  was removed. `src/code_7d74.c` still declares `gCdTrackPlaying` (line 43) and
  `SendCdTrackCommand` (line 58) locally; code_7d74 is a unit of this round, so
  those two duplicates (declcheck MULTI/LOCAL) are left for the head.
- `D_80095AC0` is reached both `$gp` (`sb`) and as an address
  (`lui`/`addiu`): a known size at most 8.
