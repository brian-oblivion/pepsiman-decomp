# InitMovieStream — MATCHED 28/28 (round 2, bravo)

> Renamed from `func_80041D18` on 2026-10-10 (tools/rename.py). Address 0x80041d18.

Unit `src/code_31cec.c`. Starts a movie stream: resets the MDEC,
clears `gMovieEnded`, installs the decode-out callback, sets a 32-sector
ring buffer at 0x80185000, sets streaming for all frames and seeks
(`StartMovieRead`).

```c
#include "libcd.h"
#include "libpress.h"

extern s32 gMovieEnded;
void StartMovieRead(CdlLOC *loc);

void InitMovieStream(CdlLOC *loc, void (*callback)()) {
    DecDCTReset(0);
    gMovieEnded = 0;
    DecDCToutCallback(callback);
    StSetRing((u_long *)0x80185000, 32);
    StSetStream(1, 1, -1, 0, 0);
    StartMovieRead(loc);
}
```

`gMovieEnded` is in `config/gp-symbols.txt` (`%gp_rel` store): a plain
scalar. The ring buffer is a fixed address (`lui`/`ori`), not a symbol.
First try.
