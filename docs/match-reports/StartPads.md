# StartPads — MATCHED (22/22), round 1, runner charlie

> Renamed from `func_8001552C` on 2026-10-10 (tools/rename.py). Address 0x8001552c.

Pad start-up: keeps two buffers in globals, clears 32 bytes of the second,
hands the first to `PadInitDirect` as the two controller-port buffers
(`a` and `a + 0x22`), then `PadStartCom()`.

```c
extern u8 *sPadBufPtr;

void StartPads(u8 *a, u8 *b) {
    s32 i;

    sPadBufPtr = a;
    sPadActPtr = b;
    for (i = 0; i < 32; i++) {
        sPadActPtr[i] = 0;
    }
    PadInitDirect(sPadBufPtr, sPadBufPtr + 0x22);
    PadStartCom();
}
```

First attempt. Both pointers are in `config/gp-symbols.txt`, so a plain
`extern u8 *` gives the `%gp_rel` accesses. The loop re-reads `sPadActPtr`
every iteration (a `u8` store may alias the pointer), which retail does too,
so the source indexes the global rather than a cached local. Prototypes come
from Sony's `libpad.h`, now included by the unit.
