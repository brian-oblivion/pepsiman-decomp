# StartMovieRead — MATCHED 18/18 (round 2, bravo)

> Renamed from `func_80041EE0` on 2026-10-10 (tools/rename.py). Address 0x80041ee0.

Unit `src/sound.c`. Seeks to a CD position and starts a streaming
read (`CdlModeStream | CdlModeSpeed | CdlModeRT` = 0x1C0), retrying
the `CdlSetloc` until it is accepted and the whole sequence until
`CdRead2` is.

```c
#include "libcd.h"

void StartMovieRead(CdlLOC *loc) {
    do {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0) {
        }
    } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0);
}
```

First try. The `addiu $a0, 2` both at the outer loop head and in the
inner loop's back-branch delay slot is GCC filling the slot from the
loop target, not a source repetition.
