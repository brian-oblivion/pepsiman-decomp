# EraseSave1 — MATCHED 29/29 (round 3, echo)

> Renamed from `func_80038468` on 2026-10-10 (tools/rename.py). Address 0x80038468.

Unit `src/code_27bc8.c`. Erases the file `"bu10:BISLPS-12345PEPTOOL"` (rodata `D_80011998`) on the memory card with libapi's `erase`, up to ten tries. The name pointer goes through the `%gp_rel` word `gCardFileName`, stored once and reloaded every try. Returns 0 on success; after ten failures it stores 12 in `gCardError` and returns -1.

```c
s32 EraseSave1(void) {
    s32 ret;
    s16 i;

    ret = 0;
    gCardFileName = D_80011998;
    for (i = 0; i < 10; i++) {
        if (erase(gCardFileName) != 0) {
            return ret;
        }
    }
    gCardError = 12;
    return -1;
}
```

Same shape as `FormatCard` (an `s32` local holding the 0 that is returned, `s32` return type), which matched at once. Reloading `gCardFileName` inside the loop is the call clobbering memory, not a source detail.
