# StopCdTrack — MATCHED (14/14), round 3, runner charlie

> Renamed from `func_80017574` on 2026-10-10 (tools/rename.py). Address 0x80017574.

Unit `src/model.c`. If the CD-busy flag `gCdTrackPlaying` is set, pauses the
drive with `CdControlF(CdlPause, 0)` and clears the flag.

```c
extern u8 gCdTrackPlaying;

void StopCdTrack(void) {
    if (gCdTrackPlaying != 0) {
        CdControlF(CdlPause, 0);
        gCdTrackPlaying = 0;
    }
}
```

## Attempts (1)

1. As above: 14/14 first build. `gCdTrackPlaying` is a scalar `u8` declared in
   the unit: every retail access (here, `main` and `PlayCdTrack`) is
   `lui $vN` / `lbu $vN` with the same register, or `lui $at` / `sb`, and it
   is not in `config/gp-symbols.txt`.
