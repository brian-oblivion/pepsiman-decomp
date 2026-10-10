# InitSound — MATCHED 15/15 (round 2, bravo)

> Renamed from `func_800428B0` on 2026-10-10 (tools/rename.py). Address 0x800428b0.

Unit `src/code_31cec.c`. Initialises libsnd: `SsInit`, a sequence table
of 9 sequences by 1 slot at `D_800E0588`, and tick mode 1.

```c
#include "libsnd.h"

extern char D_800E0588[];

void InitSound(void) {
    SsInit();
    SsSetTableSize(D_800E0588, 9, 1);
    SsSetTickMode(1);
}
```

## Externs

`D_800E0588` is reached `lui $a0` / `addiu $a0, %lo` (an address passed
as an argument), so it is declared as an array of unknown size. Its type
follows `SsSetTableSize`'s `char *` (it is the sequence attribute table
work area, `SS_SEQ_TABSIZ * 9 * 1` bytes). Sony's prototypes come from
`libsnd.h`.

First try, no levers.
