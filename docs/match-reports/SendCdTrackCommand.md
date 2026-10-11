# SendCdTrackCommand — MATCHED (26/26), round 3, runner charlie

> Renamed from `func_800175AC` on 2026-10-10 (tools/rename.py). Address 0x800175ac.

Unit `src/model.c`. Converts the sector `gCdTrackStart` into the `CdlLOC`
at `gCdTrackLoc`, retries `CdControl(com, &loc, 0)` until it succeeds, then
records the sector in `gCdTrackPos` and bumps the counter `gCdTrackPlays`.

```c
extern CdlLOC gCdTrackLoc;

s32 SendCdTrackCommand(u8 com) {
    CdIntToPos(gCdTrackStart, &gCdTrackLoc);
    while (CdControl(com, (u_char *)&gCdTrackLoc, 0) == 0) {
    }
    gCdTrackPos = gCdTrackStart;
    gCdTrackPlays++;
    return 0;
}
```

## Attempts (1)

1. As above: 26/26 first build. The three scalars are `%gp_rel` and are in
   `common.h` already; `gCdTrackLoc` is only ever reached by address
   (`lui`/`addiu`), so it is declared in the unit as a `CdlLOC`. The
   `andi 0xFF` both before the loop and in the back branch's delay slot is
   GCC's loop rotation of the `u8` command.
