# EraseSave2 — MATCHED 29/29 (round 3, echo)

> Renamed from `func_8003950C` on 2026-10-10 (tools/rename.py). Address 0x8003950c.

Unit `src/card.c`. `EraseSave1` for the second tool file, `"bu10:BISLPS-67890PEPTOOL"` (rodata `D_80011C24`): erases it with up to ten tries, 12 in `gCardError` and -1 on failure.

```c
s32 EraseSave2(void) {
    s32 ret;
    s16 i;

    ret = 0;
    gCardFileName = D_80011C24;
    for (i = 0; i < 10; i++) {
        if (erase(gCardFileName) != 0) {
            return ret;
        }
    }
    gCardError = 12;
    return -1;
}
```

Matched with `EraseSave1`'s body and a different string.
