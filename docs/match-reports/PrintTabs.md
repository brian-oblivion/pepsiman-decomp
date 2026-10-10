# PrintTabs — MATCHED (26/26), round 1, runner charlie

> Renamed from `func_80014BF0` on 2026-10-10 (tools/rename.py). Address 0x80014bf0.

Prints `count` tabs to the debug font stream.

```c
extern char D_800954CC[];

void PrintTabs(s16 count) {
    s16 i;

    for (i = 0; i < count; i++) {
        FntPrint(D_800954CC);
    }
}
```

First attempt. `D_800954CC` is the string `"\t"` in `.sdata`, but retail
reaches it with `lui $s2` / `addiu %lo`, hoisted out of the loop: an
`extern char[]` of unknown size gives exactly that (it is not in
`config/gp-symbols.txt`). Same `s16` loop shape as `WaitFrames`.
