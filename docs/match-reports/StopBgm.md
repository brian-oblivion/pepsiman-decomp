# StopBgm — MATCHED 14/14 (round 2, bravo)

> Renamed from `func_800429B4` on 2026-10-10 (tools/rename.py). Address 0x800429b4.

Unit `src/code_31cec.c`. Stops and closes the sequence whose access
number is the first halfword of `gSeqIds`.

```c
#include "libsnd.h"

extern s16 gSeqIds[];

void StopBgm(void) {
    SsSeqStop(gSeqIds[0]);
    SsSeqClose(gSeqIds[0]);
}
```

## Externs

Retail keeps `%hi(gSeqIds)` in `$s0` across the first call and loads
the halfword twice with `lh %lo(gSeqIds)($s0)`: cc1 split the address,
so the object is an array of unknown size (`s16`, from the `lh` and
`SsSeqStop(short)`). Every other function in the unit reaches it the same
way (`lui` + `%lo`), so one unit-level declaration serves them.

First try, no levers.
