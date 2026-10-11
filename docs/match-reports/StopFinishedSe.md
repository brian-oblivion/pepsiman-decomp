# StopFinishedSe — MATCHED 39/39 (round 4, echo)

> Renamed from `func_800429EC` on 2026-10-10 (tools/rename.py). Address 0x800429ec.

Unit `src/sound.c`. Walks sequence slots 2..9 of the access-number
table `gSeqIds` with the global index `gSeSlot`, and stops every
open sequence that has reached its end (`SsIsEos(seq, 0) == 1`).

```c
#include "libsnd.h"
extern s16 gSeqIds[];
extern u16 gSeSlot;

void StopFinishedSe(void) {
    s16 *seq;

    for (gSeSlot = 2; gSeSlot < 10; gSeSlot++) {
        seq = &gSeqIds[gSeSlot];
        if (*seq != 0) {
            if (SsIsEos(*seq, 0) == 1) {
                SsSeqStop(*seq);
            }
        }
    }
}
```

## Levers

- **The slot address kept in `$s0` across `SsIsEos`: a local pointer.**
  Indexing `gSeqIds[gSeSlot]` three times recomputes the address
  from the reloaded global after the call (4 words long, 2 saved
  registers short of retail's `$s0`/`$s1`). `seq = &gSeqIds[i];` then
  `*seq` gives 39/39.
- `gSeSlot` is `u16`: `lhu`, and `andi 0xFFFF` + `sltiu 10` on the
  loop test.

### Proposed learning

- **An element address held in a saved register across a call and
  dereferenced on both sides: the source took `&tbl[i]` into a local
  pointer.** Repeated `tbl[i]` with a global index reloads the index
  after the call. (StopFinishedSe)
