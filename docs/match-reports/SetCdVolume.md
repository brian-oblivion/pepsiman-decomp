# SetCdVolume — MATCHED 19/19 (round 2, bravo)

> Renamed from `func_80042968` on 2026-10-10 (tools/rename.py). Address 0x80042968.

Unit `src/sound.c`. Sets the CD audio mix to `vol`: both channels
straight through (`val0`, `val2`) and, unless the flag `gStereo` is
set, both cross-feeds too (`val1`, `val3`); with the flag set the
cross-feeds are 0.

```c
#include "libcd.h"

extern u8 gStereo;

void SetCdVolume(u8 vol) {
    CdlATV atv;

    if (gStereo) {
        atv.val2 = vol;
        atv.val0 = vol;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = vol;
        atv.val0 = vol;
        atv.val3 = vol;
        atv.val1 = vol;
    }
    CdMix(&atv);
}
```

`gStereo` is in `config/gp-symbols.txt` (`%gp_rel` `lbu`). First try,
writing the stores in retail's order (val2, val0, val3, val1), the lever
from StopXa: GCC 2.8.1 keeps local struct stores in source order.
