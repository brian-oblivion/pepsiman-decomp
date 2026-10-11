# InitSound — MATCHED 15/15 (round 2, bravo)

> Renamed from `func_800428B0` on 2026-10-10 (tools/rename.py). Address 0x800428b0.

Unit `src/sound.c`. Initialises libsnd: `SsInit`, a sequence table
of 9 sequences by 1 slot at `sSeqAttrTable`, and tick mode 1.

```c
#include "libsnd.h"

extern char sSeqAttrTable[];

void InitSound(void) {
    SsInit();
    SsSetTableSize(sSeqAttrTable, 9, 1);
    SsSetTickMode(1);
}
```

## Externs

`sSeqAttrTable` is reached `lui $a0` / `addiu $a0, %lo` (an address passed
as an argument), so it is declared as an array of unknown size. Its type
follows `SsSetTableSize`'s `char *` (it is the sequence attribute table
work area, `SS_SEQ_TABSIZ * 9 * 1` bytes). Sony's prototypes come from
`libsnd.h`.

First try, no levers.
